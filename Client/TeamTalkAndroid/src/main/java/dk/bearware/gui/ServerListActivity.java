/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

package dk.bearware.gui;

import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.PopupMenu;
import android.widget.PopupMenu.OnMenuItemClickListener;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.ListFragment;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.Collections;
import java.util.Comparator;
import java.util.Vector;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import dk.bearware.Constants;
import dk.bearware.TeamTalkBase;
import dk.bearware.UserAccount;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.data.AppInfo;
import dk.bearware.data.Permissions;
import dk.bearware.data.Preferences;
import dk.bearware.data.ServerEntry;
import dk.bearware.events.ClientEventListener;

public class ServerListActivity
extends AppCompatActivity
        implements AdapterView.OnItemClickListener,
        AdapterView.OnItemLongClickListener,
        TeamTalkConnectionListener,
        Comparator<ServerEntry>,
        ClientEventListener.OnCmdMyselfLoggedInListener {

    TeamTalkConnection mConnection;
    TeamTalkService ttservice;
    TeamTalkBase ttclient;
    ServerEntry serverentry;

    private ServerListAdapter adapter;
    private ExecutorService executorService;

    public static final String TAG = "bearware";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        adapter = new ServerListAdapter(this.getBaseContext());

        setContentView(R.layout.activity_server_list);
        getListFragment().setListAdapter(adapter);
        getListFragment().setEmptyText(getString(R.string.server_list_empty));
        getListFragment().getListView().setOnItemClickListener(this);
        getListFragment().getListView().setOnItemLongClickListener(this);

        setTitle(R.string.title_activity_server_list);
        executorService = Executors.newFixedThreadPool(2);
    }

    @Override
    protected void onResume() {
        super.onResume();

        Intent intent = getIntent();
        Uri uri = intent.getData();
        if (uri != null) {
            loadServerFromUri(uri);
        }

        if (mConnection.isBound()) {
            // reset state since we're creating a new connection
            ttservice.resetState();
            ttclient.closeSoundInputDevice();
            ttclient.closeSoundOutputDevice();
            ttservice.getEventHandler().registerOnCmdMyselfLoggedIn(this, true);

            // Connect to server if 'serverentry' is specified.
            // Connection to server is either started here or in onServiceConnected()
            if (this.serverentry != null) {
                ttservice.setServerEntry(this.serverentry);

                if (!ttservice.reconnect())
                    Toast.makeText(this, R.string.err_connection, Toast.LENGTH_LONG).show();
            }
        }
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);

        if (intent.getData() != null) {
            loadServerFromUri(intent.getData());
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mConnection.isBound())
            ttservice.getEventHandler().unregisterListener(this);
    }


    @Override
    protected void onStart() {
        super.onStart();

        if (serverentry != null) {
            saveServers();
        }

        Permissions.POST_NOTIFICATIONS.request(this);
        Permissions.INTERNET.request(this);
        Permissions.RECORD_AUDIO.request(this);
        Permissions.MODIFY_AUDIO_SETTINGS.request(this);

        // Bind to LocalService if not already
        if (mConnection == null)
            mConnection = new TeamTalkConnection(this);

        if (!mConnection.isBound()) {
            Intent intent = new Intent(getApplicationContext(), TeamTalkService.class);
            if(!bindService(intent, mConnection, Context.BIND_AUTO_CREATE))
                Log.e(TAG, "Failed to bind to TeamTalk service");
            else
                startService(intent);
        }
    }

    @Override
    protected void onStop() {
        super.onStop();

        if(isFinishing() && mConnection.isBound()) {
            // Unbind from the service.
            ttservice.resetState();
            onServiceDisconnected(ttservice);
            stopService(new Intent(getApplicationContext(), TeamTalkService.class));
            unbindService(mConnection);
            mConnection.setBound(false);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (executorService != null) {
            executorService.shutdown();
        }

        // Unbind from the service
        if(mConnection.isBound()) {
            Log.d(TAG, "Unbinding TeamTalk service");
            onServiceDisconnected(ttservice);
            unbindService(mConnection);
            mConnection.setBound(false);
        }

        Log.d(TAG, "Activity destroyed " + this.hashCode());
    }

    static final int REQUEST_EDITSERVER = 1;
    static final int REQUEST_NEWSERVER = 2;
    static final int REQUEST_IMPORT_SERVERLIST = 3;
    static final String POSITION_NAME = "pos";

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        switch(requestCode) {
            case REQUEST_NEWSERVER : {
                if(resultCode == RESULT_OK) {
                    ServerEntry entry = Utils.getServerEntry(data);
                    if(entry != null) {
                        servers.add(entry);
                        Collections.sort(servers, this);
                        adapter.notifyDataSetChanged();
                        saveServers();
                    }
                }
                break;
            }
            case REQUEST_EDITSERVER : {
                if(resultCode == RESULT_OK) {
                    ServerEntry entry = Utils.getServerEntry(data);
                    if(entry != null) {
                        int pos = data.getIntExtra(POSITION_NAME, -1);
                        if ((pos >= 0) && (pos < servers.size())) {
                            servers.removeElementAt(pos);
                            servers.insertElementAt(entry, pos);
                        }
                        else {
                            servers.add(entry);
                        }
                        Collections.sort(servers, this);
                        adapter.notifyDataSetChanged();
                        saveServers();
                    }
                }
                break;
            }
            case REQUEST_IMPORT_SERVERLIST : {
                if(resultCode == RESULT_OK) {
                    StringBuilder xml = new StringBuilder();
                    try (InputStream inputStream = this.getContentResolver().openInputStream(data.getData())) {
                        String line;
                        if (inputStream != null) {
                            BufferedReader source = new BufferedReader(new InputStreamReader(inputStream));
                            while ((line = source.readLine()) != null) {
                                xml.append(line);
                            }
                            source.close();
                        }
                    }
                    catch (Exception ex) {
                    }
                    Vector<ServerEntry> entries = Utils.getXmlServerEntries(xml.toString());
                    if (entries != null) {
                        for (ServerEntry entry : entries) {
                            entry.servertype = ServerEntry.ServerType.LOCAL;
                        }
                        servers.addAll(entries);
                        Collections.sort(servers, this);
                        adapter.notifyDataSetChanged();
                        saveServers();
                    }
                }
                break;
            }
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.server_list, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
            case R.id.action_newserverentry :
                Intent edit = new Intent(this, ServerEntryActivity.class);
                startActivityForResult(edit, REQUEST_NEWSERVER);
            break;
            case R.id.action_refreshserverlist :
                refreshServerList();
            break;
            case R.id.action_import_serverlist :
                if ((Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) || Permissions.READ_EXTERNAL_STORAGE.request(this)) {
                    fileSelectionStart();
                }
            break;
            case R.id.action_export_serverlist :
                if ((Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) || Permissions.WRITE_EXTERNAL_STORAGE.request(this)) {
                    exportServers();
                }
            break;
            case R.id.action_settings : {
                Intent intent = new Intent(ServerListActivity.this, PreferencesActivity.class);
                startActivity(intent);
                break;
            }
            case R.id.action_exit :
                finish();
            break;
            default :
                return super.onOptionsItemSelected(item);
        }
        return true;
    }

    @Override
    public void onItemClick(AdapterView<?> l, View v, int position, long id) {

        this.serverentry = servers.get(position);
        ttservice.setServerEntry(this.serverentry);

        if (!ttservice.reconnect())
            Toast.makeText(this, R.string.err_connection, Toast.LENGTH_LONG).show();
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> l, View v, int position, long id) {
        ServerEntry entry = servers.elementAt(position);

        PopupMenu serverActions = new PopupMenu(this, v);
        serverActions.inflate(R.menu.server_actions);
        serverActions.setOnMenuItemClickListener(item -> {
            switch (item.getItemId()) {
                case R.id.action_exportsrv:
                    if ((Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) || Permissions.WRITE_EXTERNAL_STORAGE.request(this)) {
                        exportServer(entry);
                    }
                    return true;
                case R.id.action_editsrv:
                    Intent intent = new Intent(this, ServerEntryActivity.class);
                    startActivityForResult(Utils.putServerEntry(intent, entry).putExtra(POSITION_NAME, position), REQUEST_EDITSERVER);
                    return true;
                case R.id.action_removesrv:
                    showRemoveServerDialog(entry, position);
                    return true;
                default:
                    return false;
            }
        });
        serverActions.show();
        return true;
    }

    void loadServerFromUri(Uri uri) {
        ServerEntry entry = new ServerEntry();
        String host = uri.getHost();
        if (host != null && !host.equals("")) {
            entry.ipaddr = host;
        }
        String tcpport = uri.getQueryParameter("tcpport");
        entry.tcpport = tcpport != null ? Integer.parseInt(tcpport) : Constants.DEFAULT_TCP_PORT;
        String udpport = uri.getQueryParameter("udpport");
        entry.udpport = udpport != null ? Integer.parseInt(udpport) : Constants.DEFAULT_UDP_PORT;
        String username = uri.getQueryParameter("username");
        entry.username = username != null ? username : "";
        String password = uri.getQueryParameter("password");
        entry.password = password != null ? password : "";
        String encrypted = uri.getQueryParameter("encrypted");
        entry.encrypted = encrypted != null ? encrypted.equalsIgnoreCase("true") || encrypted.equals("1") : entry.encrypted;
        String channel = uri.getQueryParameter("channel");
        entry.channel = channel != null ? channel : entry.channel;
        String chpasswd = uri.getQueryParameter("chanpasswd");
        entry.chanpasswd = chpasswd != null ? chpasswd : entry.chanpasswd;

        entry.servername = host + ":" + entry.tcpport;

        this.serverentry = entry;

        Log.i(TAG, "Connecting to " + entry.servername);
    }

    final Vector<ServerEntry> servers = new Vector<>();

    class ServerListAdapter extends BaseAdapter {

        private final LayoutInflater inflater;

        ServerListAdapter(Context context) {
            inflater = LayoutInflater.from(context);
        }

        @Override
        public int getCount() {
            return servers.size();
        }

        @Override
        public Object getItem(int position) {
            return servers.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(final int position, View convertView, ViewGroup parent) {

            if(convertView == null)
                convertView = inflater.inflate(R.layout.item_serverentry, parent, false);

            ImageView img = convertView.findViewById(R.id.servericon);
            TextView name = convertView.findViewById(R.id.server_name);
            TextView summary = convertView.findViewById(R.id.server_summary);
            name.setText(servers.get(position).servername);
            switch (servers.get(position).servertype) {
                case LOCAL :
                    img.setImageResource(R.drawable.teamtalk_yellow);
                    img.setContentDescription(getString(R.string.text_localserver));
                    img.setImportantForAccessibility(View.IMPORTANT_FOR_ACCESSIBILITY_NO);
                    break;
                case OFFICIAL :
                    img.setImageResource(R.drawable.teamtalk_blue);
                    img.setContentDescription(getString(R.string.text_officialserver));
                    img.setImportantForAccessibility(View.IMPORTANT_FOR_ACCESSIBILITY_YES);
                    break;
                case PUBLIC :
                    img.setImageResource(R.drawable.teamtalk_green);
                    img.setContentDescription(getString(R.string.text_publicserver));
                    img.setImportantForAccessibility(View.IMPORTANT_FOR_ACCESSIBILITY_YES);
                    break;
                case UNOFFICIAL:
                    img.setImageResource(R.drawable.teamtalk_orange);
                    img.setContentDescription(getString(R.string.text_unofficialserver));
                    img.setImportantForAccessibility(View.IMPORTANT_FOR_ACCESSIBILITY_YES);
                    break;
            }
            ServerEntry entry = servers.get(position);
            summary.setText(getString(R.string.text_server_summary, entry.ipaddr, entry.tcpport, entry.stats_usercount, entry.stats_country));

            return convertView;
        }
    }

    static final String SERVERLIST_NAME = "serverlist";

    void saveServers() {

        // SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(this);
        // SharedPreferences.Editor edit = PreferenceManager.getDefaultSharedPreferences(this).edit();
        SharedPreferences pref = this.getSharedPreferences(SERVERLIST_NAME, MODE_PRIVATE);
        SharedPreferences.Editor edit = pref.edit();

        int i = 0;
        while(!pref.getString(i + ServerEntry.KEY_SERVERNAME, "").isEmpty()) {
            edit.remove(i + ServerEntry.KEY_SERVERNAME);
            edit.remove(i + ServerEntry.KEY_IPADDR);
            edit.remove(i + ServerEntry.KEY_TCPPORT);
            edit.remove(i + ServerEntry.KEY_UDPPORT);
            edit.remove(i + ServerEntry.KEY_ENCRYPTED);
            edit.remove(i + ServerEntry.KEY_USERNAME);
            edit.remove(i + ServerEntry.KEY_PASSWORD);
            edit.remove(i + ServerEntry.KEY_NICKNAME);
            edit.remove(i + ServerEntry.KEY_REMEMBER_LAST_CHANNEL);
            edit.remove(i + ServerEntry.KEY_CHANNEL);
            edit.remove(i + ServerEntry.KEY_CHANPASSWD);
            i++;
        }

        int j=0;
        for(i = 0;i < servers.size();i++) {
            if(servers.get(i).servertype != ServerEntry.ServerType.LOCAL)
                continue;
            edit.putString(j + ServerEntry.KEY_SERVERNAME, servers.get(i).servername);
            edit.putString(j + ServerEntry.KEY_IPADDR, servers.get(i).ipaddr);
            edit.putInt(j + ServerEntry.KEY_TCPPORT, servers.get(i).tcpport);
            edit.putInt(j + ServerEntry.KEY_UDPPORT, servers.get(i).udpport);
            edit.putBoolean(j + ServerEntry.KEY_ENCRYPTED, servers.get(i).encrypted);

            edit.putString(j + ServerEntry.KEY_USERNAME, servers.get(i).username);
            edit.putString(j + ServerEntry.KEY_PASSWORD, servers.get(i).password);
            edit.putString(j + ServerEntry.KEY_NICKNAME, servers.get(i).nickname);

            edit.putBoolean(j + ServerEntry.KEY_REMEMBER_LAST_CHANNEL, servers.get(i).rememberLastChannel);
            edit.putString(j + ServerEntry.KEY_CHANNEL, servers.get(i).channel);
            edit.putString(j + ServerEntry.KEY_CHANPASSWD, servers.get(i).chanpasswd);
            j++;
        }

        edit.apply();
    }

    void loadLocalServers() {
        //load from file
        SharedPreferences pref = this.getSharedPreferences(SERVERLIST_NAME, MODE_PRIVATE);
        int i = 0;
        while(!pref.getString(i + ServerEntry.KEY_SERVERNAME, "").isEmpty()) {
            ServerEntry entry = new ServerEntry();
            entry.servername = pref.getString(i + ServerEntry.KEY_SERVERNAME, "");
            entry.ipaddr = pref.getString(i + ServerEntry.KEY_IPADDR, "");
            entry.tcpport = pref.getInt(i + ServerEntry.KEY_TCPPORT, 0);
            entry.udpport = pref.getInt(i + ServerEntry.KEY_UDPPORT, 0);
            entry.encrypted = pref.getBoolean(i + ServerEntry.KEY_ENCRYPTED, false);
            entry.username = pref.getString(i + ServerEntry.KEY_USERNAME, "");
            entry.password = pref.getString(i + ServerEntry.KEY_PASSWORD, "");
            entry.nickname = pref.getString(i + ServerEntry.KEY_NICKNAME, "");
            entry.rememberLastChannel = pref.getBoolean(i + ServerEntry.KEY_REMEMBER_LAST_CHANNEL, true);
            entry.channel = pref.getString(i + ServerEntry.KEY_CHANNEL, "");
            entry.chanpasswd = pref.getString(i + ServerEntry.KEY_CHANPASSWD, "");
            servers.add(entry);
            i++;
        }

        Collections.sort(servers, this);
        adapter.notifyDataSetChanged();
    }

    private void loadServerListAsync() {
        if (executorService == null) return;
        
        executorService.execute(() -> {
            SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(getBaseContext());

            String urlToRead = AppInfo.getServerListURL(ServerListActivity.this,
                    pref.getBoolean(Preferences.PREF_GENERAL_OFFICIALSERVERS, true),
                    pref.getBoolean(Preferences.PREF_GENERAL_UNOFFICIALSERVERS, false));

            String xml = Utils.getURL(urlToRead);
            Vector<ServerEntry> entries = null;
            if (!xml.isEmpty()) {
                entries = Utils.getXmlServerEntries(xml);
            }

            final Vector<ServerEntry> finalEntries = entries;
            runOnUiThread(() -> {
                if (finalEntries == null) {
                    Toast.makeText(ServerListActivity.this,
                            R.string.err_retrieve_public_server_list,
                            Toast.LENGTH_LONG).show();
                } else if (finalEntries.size() > 0) {
                    Collections.sort(finalEntries, ServerListActivity.this);
                    synchronized (servers) {
                        servers.addAll(finalEntries);
                    }
                    adapter.notifyDataSetChanged();
                }
            });
        });
    }

    private void refreshServerList() {
        synchronized(servers) {
            servers.clear();
            loadLocalServers();        
        }

        // Get public servers from http. TeamTalk DLL must be loaded by
        // service, otherwise static methods are unavailable (for getting DLL
        // version number).
        loadServerListAsync();
    }

    private void checkVersionAsync() {
        if (executorService == null) return;
        
        executorService.execute(() -> {
            String urlToRead = AppInfo.getUpdateURL(ServerListActivity.this);
            String xml = Utils.getURL(urlToRead);
            String latestClient = "";
            String versionMsg = "";

            if (!xml.isEmpty()) {
                try {
                    DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
                    DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
                    Document doc = dBuilder.parse(new InputSource(new StringReader(xml)));
                    doc.getDocumentElement().normalize();

                    NodeList nList = doc.getElementsByTagName("teamtalk");
                    for (int i = 0; i < nList.getLength(); i++) {
                        Node nNode = nList.item(i);
                        if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                            Element eElement = (Element) nNode;
                            NodeList nName = eElement.getElementsByTagName("name");
                            if (nName.getLength() > 0) {
                                latestClient = nName.item(0).getTextContent();
                            }
                        }
                    }
                } catch (Exception e) {
                    Log.e(TAG, "Error parsing version XML", e);
                }
            }

            final String finalLatestClient = latestClient;
            final String finalVersionMsg = versionMsg;
            runOnUiThread(() -> {
                if (finalVersionMsg.length() > 0) {
                    Toast.makeText(ServerListActivity.this,
                            getString(R.string.version_update, finalLatestClient),
                            Toast.LENGTH_LONG).show();
                }
            });
        });
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        Permissions granted = Permissions.onRequestResult(this, requestCode, grantResults);
        if (granted == null)
            return;
        switch (granted) {
            case READ_EXTERNAL_STORAGE:
                fileSelectionStart();
                break;
            case WRITE_EXTERNAL_STORAGE:
                exportServers();
                break;
        }
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
        ttservice = service;
        ttclient = service.getTTInstance();

        service.getEventHandler().registerOnCmdMyselfLoggedIn(this, true);

        // Connect to server if 'serverentry' is specified.
        // Connection to server is either started here or in onResume()
        if (serverentry != null) {
            ttservice.setServerEntry(serverentry);

            if (!ttservice.reconnect())
                Toast.makeText(this, R.string.err_connection, Toast.LENGTH_LONG).show();
        }

        refreshServerList();

        String version = AppInfo.getVersion(this);

        TextView tv_version = findViewById(R.id.version_textview);
        TextView tv_dllversion = findViewById(R.id.dllversion_textview);
        tv_version.setText(String.format("%s%s%s Build %d", getString(R.string.ttversion), version, AppInfo.APPVERSION_POSTFIX, BuildConfig.VERSION_CODE));
        tv_dllversion.setText(getString(R.string.ttdllversion) + TeamTalkBase.getVersion());

        checkVersionAsync();
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
        service.getEventHandler().unregisterListener(this);
    }


    @Override
    public void onCmdMyselfLoggedIn(int my_userid, UserAccount useraccount) {
        if (serverentry != null) {
            Intent intent = new Intent(getBaseContext(), MainActivity.class);
            startActivity(intent.putExtra(ServerEntry.KEY_SERVERNAME, serverentry.servername));

            serverentry = null;
        }
    }

    @Override
    public int compare(ServerEntry s1, ServerEntry s2) {
        switch (s1.servertype) {
            case LOCAL :
                switch (s2.servertype) {
                    case LOCAL :
                        return s1.servername.compareToIgnoreCase(s2.servername);
                    case OFFICIAL :
                    case PUBLIC :
                    case UNOFFICIAL :
                        return -1;
                }
                break;

            case OFFICIAL:
                switch (s2.servertype) {
                    case LOCAL :
                        return 1;
                    case OFFICIAL :
                        return 0; // order determined by xml-reply (from web-request)
                    case PUBLIC :
                    case UNOFFICIAL :
                        return -1;
                }
                break;

            case PUBLIC :
                switch (s2.servertype) {
                    case LOCAL :
                    case OFFICIAL :
                        return 1;
                    case PUBLIC :
                        return 0; // order determined by xml-reply (from web-request)
                    case UNOFFICIAL :
                        return -1;
                }
                break;

            case UNOFFICIAL:
                switch (s2.servertype) {
                    case LOCAL :
                    case OFFICIAL :
                    case PUBLIC :
                        return 1;
                    case UNOFFICIAL :
                        return s1.servername.compareToIgnoreCase(s2.servername);
                }
                break;
        }
        return 0;
    }

    private void fileSelectionStart() {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        Intent i = Intent.createChooser(intent, "File");
        startActivityForResult(i, REQUEST_IMPORT_SERVERLIST);
    }

    private ListFragment getListFragment() {
        return (ListFragment) getSupportFragmentManager().findFragmentById(R.id.list_fragment);
    }

    private void exportServers() {
        Vector<ServerEntry> localServers = getLocalServers();
        File ttFile = createExportFile("tt5servers.tt");
        if (ttFile != null) {
            exportToFile(localServers, ttFile, R.string.serverlist_export_confirmation);
        }
    }

    private void exportServer(ServerEntry entry) {
        Vector<ServerEntry> singleServer = new Vector<>(Collections.singletonList(entry));
        File ttFile = createExportFile(entry.servername + "_server.tt");
        if (ttFile != null) {
            exportToFile(singleServer, ttFile, R.string.server_export_confirmation);
        }
    }

    private Vector<ServerEntry> getLocalServers() {
        Vector<ServerEntry> localServers = new Vector<>();
        synchronized(servers) {
            for (ServerEntry entry : servers) {
                if (entry.servertype == ServerEntry.ServerType.LOCAL) {
                    localServers.add(entry);
                }
            }
        }
        return localServers;
    }

    private File createExportFile(String fileName) {
        File dirPath = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
        if (dirPath.mkdirs() || dirPath.isDirectory()) {
            return new File(dirPath, fileName);
        }
        return null;
    }

    private void exportToFile(Vector<ServerEntry> entries, File ttFile, int successMsgId) {
        final String filePath = ttFile.getAbsolutePath();
        
        if (ttFile.exists()) {
            showFileOverrideDialog(entries, ttFile, filePath, successMsgId);
        } else {
            performExport(entries, filePath, successMsgId);
        }
    }

    private void showFileOverrideDialog(Vector<ServerEntry> entries, File ttFile, String filePath, int successMsgId) {
        AlertDialog.Builder alert = new AlertDialog.Builder(this);
        alert.setMessage(getString(R.string.alert_file_override, filePath));
        alert.setPositiveButton(android.R.string.yes, (dialog, whichButton) -> {
            if (ttFile.delete()) {
                performExport(entries, filePath, successMsgId);
            } else {
                showToast(getString(R.string.err_file_delete, filePath));
            }
        });
        alert.setNegativeButton(android.R.string.no, null);
        alert.show();
    }

    private void performExport(Vector<ServerEntry> entries, String filePath, int successMsgId) {
        boolean success = Utils.saveServers(entries, filePath);
        int msgId = success ? successMsgId : R.string.err_file_write;
        showToast(getString(msgId, filePath));
    }

    private void showToast(String message) {
        Toast.makeText(this, message, Toast.LENGTH_LONG).show();
    }

    private void showRemoveServerDialog(ServerEntry entry, int position) {
        AlertDialog.Builder alert = new AlertDialog.Builder(this);
        alert.setMessage(getString(R.string.server_remove_confirmation, entry.servername));
        alert.setPositiveButton(android.R.string.yes, (dialog, whichButton) -> {
            servers.remove(position);
            adapter.notifyDataSetChanged();
            saveServers();
        });
        alert.setNegativeButton(android.R.string.no, null);
        alert.show();
    }
}
