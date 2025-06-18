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
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.PopupMenu;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.view.ViewCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

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
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
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

public class ServerListActivity extends AppCompatActivity
        implements TeamTalkConnectionListener,
        Comparator<ServerEntry>,
        ClientEventListener.OnCmdMyselfLoggedInListener {

    private TeamTalkConnection mConnection;
    private TeamTalkService ttservice;
    private TeamTalkBase ttclient;
    private ServerEntry serverentry;

    private ServerListAdapter adapter;
    private RecyclerView recyclerView;
    private EditText searchEditText;
    private TextView emptyView;
    private ExecutorService executorService;
    
    private final Vector<ServerEntry> servers = new Vector<>();

    private static final String TAG = "bearware";
    private static final String SERVERLIST_NAME = "serverlist";
    private static final int REQUEST_EDITSERVER = 1;
    private static final int REQUEST_NEWSERVER = 2;
    private static final int REQUEST_IMPORT_SERVERLIST = 3;
    private static final String POSITION_NAME = "pos";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_server_list);
        initializeViews();
        setupRecyclerView();
        setupSearch();
        setTitle(R.string.title_activity_server_list);
        executorService = Executors.newFixedThreadPool(2);
    }

    private void initializeViews() {
        recyclerView = findViewById(R.id.servers_recycler_view);
        searchEditText = findViewById(R.id.search_edit_text);
        emptyView = findViewById(R.id.empty_view);
    }

    private void setupRecyclerView() {
        adapter = new ServerListAdapter();
        recyclerView.setLayoutManager(new LinearLayoutManager(this));
        recyclerView.setAdapter(adapter);
        updateEmptyView();
    }

    private void setupSearch() {
        searchEditText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                adapter.filter(s.toString());
            }

            @Override
            public void afterTextChanged(Editable s) {}
        });
    }

    private void updateEmptyView() {
        if (adapter == null) return;
        boolean isEmpty = adapter.getItemCount() == 0;
        emptyView.setVisibility(isEmpty ? View.VISIBLE : View.GONE);
        recyclerView.setVisibility(isEmpty ? View.GONE : View.VISIBLE);
    }

    @Override
    protected void onResume() {
        super.onResume();

        Intent intent = getIntent();
        Uri uri = intent.getData();
        if (uri != null) {
            loadServerFromUri(uri);
        }

        if (mConnection != null && mConnection.isBound()) {
            // reset state since we're creating a new connection
            ttservice.resetState();
            ttclient.closeSoundInputDevice();
            ttclient.closeSoundOutputDevice();
            ttservice.getEventHandler().registerOnCmdMyselfLoggedIn(this, true);

            // Connect to server if 'serverentry' is specified.
            // Connection to server is either started here or in onServiceConnected()
            if (this.serverentry != null) {
                ttservice.setServerEntry(this.serverentry);

                if (!ttservice.reconnect()) {
                    showToast(getString(R.string.err_connection));
                }
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
        if (mConnection != null && mConnection.isBound())
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

        if(isFinishing() && mConnection != null && mConnection.isBound()) {
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
        if(mConnection != null && mConnection.isBound()) {
            Log.d(TAG, "Unbinding TeamTalk service");
            onServiceDisconnected(ttservice);
            unbindService(mConnection);
            mConnection.setBound(false);
        }

        Log.d(TAG, "Activity destroyed " + this.hashCode());
    }


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
                        adapter.updateServers();
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
                        adapter.updateServers();
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
                        adapter.updateServers();
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

    private void onServerClick(ServerEntry entry) {
        if (ttservice == null) {
            showToast(getString(R.string.err_connection));
            return;
        }
        this.serverentry = entry;
        ttservice.setServerEntry(this.serverentry);

        if (!ttservice.reconnect()) {
            showToast(getString(R.string.err_connection));
        }
    }

    private void onServerLongClick(View view, ServerEntry entry, int position) {
        PopupMenu serverActions = new PopupMenu(this, view);
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
                    showRemoveServerDialog(entry);
                    return true;
                default:
                    return false;
            }
        });
        serverActions.show();
    }

    private void loadServerFromUri(Uri uri) {
        ServerEntry entry = new ServerEntry();
        String host = uri.getHost();
        
        if (host != null && !host.isEmpty()) {
            entry.ipaddr = host;
            entry.servername = host + ":" + getIntParameterOrDefault(uri, "tcpport", Constants.DEFAULT_TCP_PORT);
        }
        
        entry.tcpport = getIntParameterOrDefault(uri, "tcpport", Constants.DEFAULT_TCP_PORT);
        entry.udpport = getIntParameterOrDefault(uri, "udpport", Constants.DEFAULT_UDP_PORT);
        entry.username = getStringParameterOrDefault(uri, "username", "");
        entry.password = getStringParameterOrDefault(uri, "password", "");
        entry.channel = getStringParameterOrDefault(uri, "channel", entry.channel);
        entry.chanpasswd = getStringParameterOrDefault(uri, "chanpasswd", entry.chanpasswd);
        
        String encrypted = uri.getQueryParameter("encrypted");
        entry.encrypted = encrypted != null && (encrypted.equalsIgnoreCase("true") || encrypted.equals("1"));

        this.serverentry = entry;
        Log.i(TAG, "Connecting to " + entry.servername);
    }

    private int getIntParameterOrDefault(Uri uri, String parameter, int defaultValue) {
        String value = uri.getQueryParameter(parameter);
        return value != null ? Integer.parseInt(value) : defaultValue;
    }

    private String getStringParameterOrDefault(Uri uri, String parameter, String defaultValue) {
        String value = uri.getQueryParameter(parameter);
        return value != null ? value : defaultValue;
    }

    private class ServerListAdapter extends RecyclerView.Adapter<ServerListAdapter.ServerViewHolder> {
        private List<ServerEntry> filteredServers = new ArrayList<>();
        private String currentFilter = "";

        public ServerListAdapter() {
            updateFilteredList();
        }

        @NonNull
        @Override
        public ServerViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            View view = LayoutInflater.from(parent.getContext())
                    .inflate(R.layout.item_serverentry, parent, false);
            return new ServerViewHolder(view);
        }

        @Override
        public void onBindViewHolder(@NonNull ServerViewHolder holder, int position) {
            ServerEntry entry = filteredServers.get(position);
            holder.bind(entry, position);
        }

        @Override
        public int getItemCount() {
            return filteredServers.size();
        }

        public void filter(String query) {
            String newFilter = query.toLowerCase().trim();
            if (!newFilter.equals(currentFilter)) {
                currentFilter = newFilter;
                updateFilteredList();
            }
        }

        private void updateFilteredList() {
            List<ServerEntry> newFilteredList = new ArrayList<>();
            if (currentFilter.isEmpty()) {
                newFilteredList.addAll(servers);
            } else {
                for (ServerEntry server : servers) {
                    if (matchesFilter(server, currentFilter)) {
                        newFilteredList.add(server);
                    }
                }
            }
            
            filteredServers.clear();
            filteredServers.addAll(newFilteredList);
            notifyDataSetChanged();
            updateEmptyView();
        }

        private boolean matchesFilter(ServerEntry server, String filter) {
            return server.servername.toLowerCase().contains(filter) ||
                   server.ipaddr.toLowerCase().contains(filter);
        }

        public void updateServers() {
            updateFilteredList();
        }

        public void removeServer(ServerEntry entry) {
            int index = filteredServers.indexOf(entry);
            if (index != -1) {
                filteredServers.remove(index);
                notifyItemRemoved(index);
            }
        }

        private class ServerViewHolder extends RecyclerView.ViewHolder {
            private final ImageView serverIcon;
            private final TextView serverName;
            private final TextView serverSummary;

            public ServerViewHolder(@NonNull View itemView) {
                super(itemView);
                serverIcon = itemView.findViewById(R.id.servericon);
                serverName = itemView.findViewById(R.id.server_name);
                serverSummary = itemView.findViewById(R.id.server_summary);
            }

            public void bind(ServerEntry entry, int position) {
                serverName.setText(entry.servername);
                setServerIcon(entry);
                serverSummary.setText(getString(R.string.text_server_summary, 
                    entry.ipaddr, entry.tcpport, entry.stats_usercount, entry.stats_country));

                itemView.setOnClickListener(v -> onServerClick(entry));
                itemView.setOnLongClickListener(v -> {
                    onServerLongClick(v, entry, position);
                    return true;
                });
                
                setupAccessibilityActions(entry, position);
            }

            private void setupAccessibilityActions(ServerEntry entry, int position) {
                ViewCompat.addAccessibilityAction(itemView, 
                    getString(R.string.action_editsrv),
                    (view, arguments) -> {
                        Intent intent = new Intent(ServerListActivity.this, ServerEntryActivity.class);
                        startActivityForResult(Utils.putServerEntry(intent, entry)
                            .putExtra(POSITION_NAME, position), REQUEST_EDITSERVER);
                        return true;
                    });
                    
                ViewCompat.addAccessibilityAction(itemView,
                    getString(R.string.action_exportsrv), 
                    (view, arguments) -> {
                        if ((Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) || 
                            Permissions.WRITE_EXTERNAL_STORAGE.request(ServerListActivity.this)) {
                            exportServer(entry);
                        }
                        return true;
                    });
                    
                ViewCompat.addAccessibilityAction(itemView,
                    getString(R.string.action_removesrv),
                    (view, arguments) -> {
                        showRemoveServerDialog(entry);
                        return true;
                    });
            }

            private void setServerIcon(ServerEntry entry) {
                switch (entry.servertype) {
                    case LOCAL:
                        serverIcon.setImageResource(R.drawable.teamtalk_yellow);
                        serverIcon.setContentDescription(getString(R.string.text_localserver));
                        serverIcon.setImportantForAccessibility(View.IMPORTANT_FOR_ACCESSIBILITY_NO);
                        break;
                    case OFFICIAL:
                        serverIcon.setImageResource(R.drawable.teamtalk_blue);
                        serverIcon.setContentDescription(getString(R.string.text_officialserver));
                        serverIcon.setImportantForAccessibility(View.IMPORTANT_FOR_ACCESSIBILITY_YES);
                        break;
                    case PUBLIC:
                        serverIcon.setImageResource(R.drawable.teamtalk_green);
                        serverIcon.setContentDescription(getString(R.string.text_publicserver));
                        serverIcon.setImportantForAccessibility(View.IMPORTANT_FOR_ACCESSIBILITY_YES);
                        break;
                    case UNOFFICIAL:
                        serverIcon.setImageResource(R.drawable.teamtalk_orange);
                        serverIcon.setContentDescription(getString(R.string.text_unofficialserver));
                        serverIcon.setImportantForAccessibility(View.IMPORTANT_FOR_ACCESSIBILITY_YES);
                        break;
                }
            }
        }
    }

    private void saveServers() {
        SharedPreferences pref = getSharedPreferences(SERVERLIST_NAME, MODE_PRIVATE);
        SharedPreferences.Editor edit = pref.edit();

        clearExistingServerPreferences(pref, edit);
        saveLocalServersToPreferences(edit);
        edit.apply();
    }

    private void clearExistingServerPreferences(SharedPreferences pref, SharedPreferences.Editor edit) {
        int i = 0;
        while (!pref.getString(i + ServerEntry.KEY_SERVERNAME, "").isEmpty()) {
            removeServerPreferencesAtIndex(edit, i);
            i++;
        }
    }

    private void removeServerPreferencesAtIndex(SharedPreferences.Editor edit, int index) {
        String[] keys = {
            ServerEntry.KEY_SERVERNAME, ServerEntry.KEY_IPADDR, ServerEntry.KEY_TCPPORT,
            ServerEntry.KEY_UDPPORT, ServerEntry.KEY_ENCRYPTED, ServerEntry.KEY_USERNAME,
            ServerEntry.KEY_PASSWORD, ServerEntry.KEY_NICKNAME, ServerEntry.KEY_STATUSMSG, ServerEntry.KEY_REMEMBER_LAST_CHANNEL,
            ServerEntry.KEY_CHANNEL, ServerEntry.KEY_CHANPASSWD
        };
        
        for (String key : keys) {
            edit.remove(index + key);
        }
    }

    private void saveLocalServersToPreferences(SharedPreferences.Editor edit) {
        int localServerIndex = 0;
        for (ServerEntry server : servers) {
            if (server.servertype == ServerEntry.ServerType.LOCAL) {
                saveServerToPreferences(edit, server, localServerIndex);
                localServerIndex++;
            }
        }
    }

    private void saveServerToPreferences(SharedPreferences.Editor edit, ServerEntry server, int index) {
        edit.putString(index + ServerEntry.KEY_SERVERNAME, server.servername);
        edit.putString(index + ServerEntry.KEY_IPADDR, server.ipaddr);
        edit.putInt(index + ServerEntry.KEY_TCPPORT, server.tcpport);
        edit.putInt(index + ServerEntry.KEY_UDPPORT, server.udpport);
        edit.putBoolean(index + ServerEntry.KEY_ENCRYPTED, server.encrypted);
        edit.putString(index + ServerEntry.KEY_USERNAME, server.username);
        edit.putString(index + ServerEntry.KEY_PASSWORD, server.password);
        edit.putString(index + ServerEntry.KEY_NICKNAME, server.nickname);
        edit.putString(index + ServerEntry.KEY_STATUSMSG, server.statusmsg);
        edit.putBoolean(index + ServerEntry.KEY_REMEMBER_LAST_CHANNEL, server.rememberLastChannel);
        edit.putString(index + ServerEntry.KEY_CHANNEL, server.channel);
        edit.putString(index + ServerEntry.KEY_CHANPASSWD, server.chanpasswd);
    }

    private void loadLocalServers() {
        SharedPreferences pref = getSharedPreferences(SERVERLIST_NAME, MODE_PRIVATE);
        int i = 0;
        while (!pref.getString(i + ServerEntry.KEY_SERVERNAME, "").isEmpty()) {
            ServerEntry entry = loadServerFromPreferences(pref, i);
            servers.add(entry);
            i++;
        }

        Collections.sort(servers, this);
        adapter.updateServers();
    }

    private ServerEntry loadServerFromPreferences(SharedPreferences pref, int index) {
        ServerEntry entry = new ServerEntry();
        entry.servername = pref.getString(index + ServerEntry.KEY_SERVERNAME, "");
        entry.ipaddr = pref.getString(index + ServerEntry.KEY_IPADDR, "");
        entry.tcpport = pref.getInt(index + ServerEntry.KEY_TCPPORT, 0);
        entry.udpport = pref.getInt(index + ServerEntry.KEY_UDPPORT, 0);
        entry.encrypted = pref.getBoolean(index + ServerEntry.KEY_ENCRYPTED, false);
        entry.username = pref.getString(index + ServerEntry.KEY_USERNAME, "");
        entry.password = pref.getString(index + ServerEntry.KEY_PASSWORD, "");
        entry.nickname = pref.getString(index + ServerEntry.KEY_NICKNAME, "");
        entry.statusmsg = pref.getString(index + ServerEntry.KEY_STATUSMSG, "");
        entry.rememberLastChannel = pref.getBoolean(index + ServerEntry.KEY_REMEMBER_LAST_CHANNEL, true);
        entry.channel = pref.getString(index + ServerEntry.KEY_CHANNEL, "");
        entry.chanpasswd = pref.getString(index + ServerEntry.KEY_CHANPASSWD, "");
        return entry;
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
                    showToast(getString(R.string.err_retrieve_public_server_list));
                } else if (finalEntries.size() > 0) {
                    Collections.sort(finalEntries, ServerListActivity.this);
                    synchronized (servers) {
                        servers.addAll(finalEntries);
                    }
                    adapter.updateServers();
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
                    showToast(getString(R.string.version_update, finalLatestClient));
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

            if (!ttservice.reconnect()) {
                showToast(getString(R.string.err_connection));
            }
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

    private void showRemoveServerDialog(ServerEntry entry) {
        AlertDialog.Builder alert = new AlertDialog.Builder(this);
        alert.setMessage(getString(R.string.server_remove_confirmation, entry.servername));
        alert.setPositiveButton(android.R.string.yes, (dialog, whichButton) -> {
            servers.remove(entry);
            adapter.removeServer(entry);
            saveServers();
            updateEmptyView();
        });
        alert.setNegativeButton(android.R.string.no, null);
        alert.show();
    }
}
