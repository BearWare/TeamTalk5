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

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.StringReader;
import java.util.Collections;
import java.util.Comparator;
import java.util.Vector;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;

import dk.bearware.BannedUser;
import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.Constants;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.TeamTalkBase;
import dk.bearware.TextMessage;
import dk.bearware.User;
import dk.bearware.UserAccount;
import dk.bearware.backend.TeamTalkConstants;
import dk.bearware.data.Permissions;
import dk.bearware.data.Preferences;
import dk.bearware.gui.R;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.events.CommandListener;
import dk.bearware.data.AppInfo;
import dk.bearware.data.ServerEntry;

import android.Manifest;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import androidx.fragment.app.ListFragment;
import androidx.appcompat.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.facebook.CallbackManager;
import com.facebook.FacebookSdk;
import com.facebook.login.LoginManager;

public class ServerListActivity
extends AppCompatActivity
implements AdapterView.OnItemClickListener, AdapterView.OnItemLongClickListener, TeamTalkConnectionListener, CommandListener, Comparator<ServerEntry> {

    TeamTalkConnection mConnection;
    TeamTalkService ttservice;
    TeamTalkBase ttclient;
    CallbackManager callbackManager;
    ServerEntry serverentry;

    private ServerListAdapter adapter;

    public static final String TAG = "bearware";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        callbackManager = CallbackManager.Factory.create();
        adapter = new ServerListAdapter(this.getBaseContext());

        setContentView(R.layout.activity_server_list);
        getListFragment().setListAdapter(adapter);
        getListFragment().setEmptyText(getString(R.string.server_list_empty));
        getListFragment().getListView().setOnItemClickListener(this);
        getListFragment().getListView().setOnItemLongClickListener(this);

        setTitle(R.string.title_activity_server_list);
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (mConnection.isBound()) {
            // reset state since we're creating a new connection
            ttservice.resetState();
            ttclient.closeSoundInputDevice();
            ttclient.closeSoundOutputDevice();
            ttservice.registerCommandListener(this);
        }

        Intent intent = getIntent();
        if (Intent.ACTION_VIEW.equals(intent.getAction())) {
            Uri uri = intent.getData();
            ServerEntry entry = new ServerEntry();
            String host = uri.getHost();
            if (host != "") {
                entry.servername = host;
                entry.ipaddr = host;
            }
            String tcpPort = uri.getQueryParameter("tcpport");
            if (tcpPort != "") {
                entry.tcpport = Integer.parseInt(tcpPort);
            } else {
                entry.tcpport = Constants.DEFAULT_TCP_PORT;
            }
            String udpPort = uri.getQueryParameter("udpport");
            if (udpPort != "") {
                entry.udpport = Integer.parseInt(udpPort);
            } else {
                entry.udpport = Constants.DEFAULT_UDP_PORT;
            }
            String userName = uri.getQueryParameter("username");
            if (userName != "") {
                entry.username = userName;
            }
            String password = uri.getQueryParameter("password");
            if (password != "") {
                entry.password = password;
            }
            this.serverentry = entry;
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mConnection.isBound())
            ttservice.unregisterCommandListener(this);
    }


    @Override
    protected void onStart() {
        super.onStart();

        if ((serverentry != null) && serverentry.rememberLastChannel) {
            saveServers();
            serverentry = null;
        }

        Permissions.setupPermission(getBaseContext(), this, Permissions.MY_PERMISSIONS_REQUEST_INTERNET);
        Permissions.setupPermission(getBaseContext(), this, Permissions.MY_PERMISSIONS_REQUEST_RECORD_AUDIO);
        Permissions.setupPermission(getBaseContext(), this, Permissions.MY_PERMISSIONS_REQUEST_MODIFY_AUDIO_SETTINGS);

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
        callbackManager.onActivityResult(requestCode, resultCode, data);

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
                    String xml = "";
                    try {
                        String line;
                        BufferedReader source = new BufferedReader(new FileReader(data.getStringExtra(FilePickerActivity.SELECTED_FILE)));
                        while ((line = source.readLine()) != null) {
                            xml += line;
                        }
                        source.close();
                    }
                    catch (Exception ex) {
                    }
                    Vector<ServerEntry> entries = Utils.getXmlServerEntries(xml);
                    if (entries != null) {
                        for (ServerEntry entry : entries) {
                            entry.public_server = false;
                            entry.rememberLastChannel = true;
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
                if (Permissions.setupPermission(getBaseContext(), this, Permissions.MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE)) {
                    Intent filepicker = new Intent(this, FilePickerActivity.class);
                    startActivityForResult(filepicker.putExtra(FilePickerActivity.FILTER_EXTENSION, ".tt"), REQUEST_IMPORT_SERVERLIST);
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
        serverentry = servers.get(position);

        //unregister so delayed facebook login will not cancel new login session
        LoginManager.getInstance().unregisterCallback(callbackManager);

        if(serverentry.isFacebookLogin()) {
            LoginManager.getInstance().registerCallback(callbackManager,
                    Utils.createFacebookLogin(this, ttservice, serverentry));

            Utils.facebookLogin(this);
        }
        else {
            ttservice.setServerEntry(serverentry);

            if (!ttservice.reconnect())
                Toast.makeText(this, R.string.err_connection, Toast.LENGTH_LONG).show();
        }
    }

    @Override
    public boolean onItemLongClick(AdapterView< ? > l, View v, int position, long id) {
        Intent intent = new Intent(this, ServerEntryActivity.class);

        ServerEntry entry = servers.elementAt(position);

        startActivityForResult(Utils.putServerEntry(intent, entry).putExtra(POSITION_NAME, position),
            REQUEST_EDITSERVER);
        return true;
    }

    Vector<ServerEntry> servers = new Vector<ServerEntry>();

    class ServerListAdapter extends BaseAdapter {

        private LayoutInflater inflater;

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
            
            ImageView img = (ImageView) convertView.findViewById(R.id.servericon);
            TextView name = (TextView) convertView.findViewById(R.id.server_name);
            TextView address = (TextView) convertView.findViewById(R.id.server_address);
            name.setText(servers.get(position).servername);
            if (servers.get(position).public_server) {
                img.setImageResource(R.drawable.teamtalk_green);
                img.setContentDescription(getString(R.string.text_publicserver));
                img.setImportantForAccessibility(View.IMPORTANT_FOR_ACCESSIBILITY_YES);
            }
            else {
                img.setImageResource(R.drawable.teamtalk_yellow);
                img.setContentDescription(null);
                img.setImportantForAccessibility(View.IMPORTANT_FOR_ACCESSIBILITY_NO);
            }
            address.setText(servers.get(position).ipaddr);
            View editButton = convertView.findViewById(R.id.server_edit);
            if (editButton != null)
                editButton.setOnClickListener(new View.OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            onItemLongClick(getListFragment().getListView(), v, position, v.getId());
                        }
                    });
            convertView.findViewById(R.id.server_remove).setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    AlertDialog.Builder alert = new AlertDialog.Builder(ServerListActivity.this);
                    alert.setMessage(getString(R.string.server_remove_confirmation, servers.get(position).servername));
                    alert.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {

                            @Override
                                public void onClick(DialogInterface dialog, int whichButton) {
                                servers.remove(position);
                                notifyDataSetChanged();
                                saveServers();
                            }
                        });
                    alert.setNegativeButton(android.R.string.no, null);
                    alert.show();
                }
            });

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
            if(servers.get(i).public_server)
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
    
    class ServerListAsyncTask extends AsyncTask<Void, Void, Void> {

        Vector<ServerEntry> entries;

        @Override
        protected Void doInBackground(Void... params) {
            String urlToRead = AppInfo.getServerListURL(ServerListActivity.this);

            String xml = Utils.getURL(urlToRead);
            if(!xml.isEmpty())
                entries = Utils.getXmlServerEntries(xml);
            return null;
        }

        protected void onPostExecute(Void result) {
            if(entries == null)
                Toast.makeText(ServerListActivity.this,
                               R.string.err_retrieve_public_server_list,
                               Toast.LENGTH_LONG).show();
            else if(entries.size() > 0) {
                Collections.sort(entries, ServerListActivity.this);
                synchronized(servers) {
                    servers.addAll(entries);
                }
                adapter.notifyDataSetChanged();
            }
        }
    }
    
    void refreshServerList() {
        synchronized(servers) {
            servers.clear();
            loadLocalServers();        
        }

        SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        if(pref.getBoolean(Preferences.PREF_GENERAL_PUBLICSERVERS, true)) {
            // Get public servers from http. TeamTalk DLL must be loaded by
            // service, otherwise static methods are unavailable (for getting DLL
            // version number).
            new ServerListAsyncTask().execute();
        }
    }
    
    class VersionCheckAsyncTask extends AsyncTask<Void, Void, Void> {
        
        String latestclient = "", versionmsg = "";

        @Override
        protected Void doInBackground(Void... params) {
            String urlToRead = AppInfo.getUpdateURL(ServerListActivity.this);

            String xml = Utils.getURL(urlToRead);
            if(!xml.isEmpty()) {
                DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
                DocumentBuilder dBuilder;
                Document doc;
                try {
                    dBuilder = dbFactory.newDocumentBuilder();
                    doc = dBuilder.parse(new InputSource(new StringReader(xml)));
                }
                catch(Exception e) {
                    return null;
                }
                
                doc.getDocumentElement().normalize();
                
                NodeList nList = doc.getElementsByTagName("teamtalk");
                for (int i = 0; i < nList.getLength(); i++) {
                    Node nNode = nList.item(i);
                    if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                        Element eElement = (Element) nNode;
                        NodeList nName = eElement.getElementsByTagName("name");
                        if(nName.getLength()>0)
                            latestclient = nName.item(0).getTextContent();
                    }
                }
            }
            return null;
        }

        protected void onPostExecute(Void result) {
            if(versionmsg.length()>0) {
                Toast.makeText(ServerListActivity.this,
                               getString(R.string.version_update, latestclient),
                               Toast.LENGTH_LONG).show();
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[], int[] grantResults) {
        boolean granted = grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED;
        switch (requestCode) {
            case Permissions.MY_PERMISSIONS_REQUEST_INTERNET :
                break;
            case Permissions.MY_PERMISSIONS_REQUEST_RECORD_AUDIO :
                break;
            case Permissions.MY_PERMISSIONS_REQUEST_MODIFY_AUDIO_SETTINGS :
                break;
            case Permissions.MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE :
                Intent filepicker = new Intent(this, FilePickerActivity.class);
                startActivityForResult(filepicker.putExtra(FilePickerActivity.FILTER_EXTENSION, ".tt"), REQUEST_IMPORT_SERVERLIST);
                break;
        }
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
        ttservice = service;
        ttclient = service.getTTInstance();

        service.registerCommandListener(this);

        if (serverentry != null) {
            ttservice.setServerEntry(serverentry);

            if (!ttservice.reconnect())
                Toast.makeText(this, R.string.err_connection, Toast.LENGTH_LONG).show();

            this.serverentry = null;
        }

        refreshServerList();

        String version = AppInfo.getVersion(this);
                
        TextView tv_version = (TextView)findViewById(R.id.version_textview);
        TextView tv_dllversion = (TextView)findViewById(R.id.dllversion_textview);
        tv_version.setText(getString(R.string.ttversion) + version + AppInfo.APPVERSION_POSTFIX);
        tv_dllversion.setText(getString(R.string.ttdllversion) + TeamTalkBase.getVersion());

        new VersionCheckAsyncTask().execute();
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
        service.unregisterCommandListener(this);
    }

    @Override
    public void onCmdError(int cmdId, ClientErrorMsg errmsg) {
    }

    @Override
    public void onCmdSuccess(int cmdId) {
    }

    @Override
    public void onCmdProcessing(int cmdId, boolean complete) {
    }

    @Override
    public void onCmdMyselfLoggedIn(int my_userid, UserAccount useraccount) {
        if (serverentry != null) {
            Intent intent = new Intent(getBaseContext(), MainActivity.class);
            startActivity(intent.putExtra(ServerEntry.KEY_SERVERNAME, serverentry.servername));
        }
    }

    @Override
    public void onCmdMyselfLoggedOut() {
    }

    @Override
    public void onCmdMyselfKickedFromChannel() {
    }

    @Override
    public void onCmdMyselfKickedFromChannel(User kicker) {
    }

    @Override
    public void onCmdUserLoggedIn(User user) {
    }

    @Override
    public void onCmdUserLoggedOut(User user) {
    }

    @Override
    public void onCmdUserUpdate(User user) {
    }

    @Override
    public void onCmdUserJoinedChannel(User user) {
    }

    @Override
    public void onCmdUserLeftChannel(int channelid, User user) {
    }

    @Override
    public void onCmdUserTextMessage(TextMessage textmessage) {
    }

    @Override
    public void onCmdChannelNew(Channel channel) {
    }

    @Override
    public void onCmdChannelUpdate(Channel channel) {
    }

    @Override
    public void onCmdChannelRemove(Channel channel) {
    }

    @Override
    public void onCmdServerUpdate(ServerProperties serverproperties) {
    }

    @Override
    public void onCmdFileNew(RemoteFile remotefile) {
    }

    @Override
    public void onCmdFileRemove(RemoteFile remotefile) {
    }

    @Override
    public void onCmdUserAccount(UserAccount useraccount) {
    }

    @Override
    public void onCmdBannedUser(BannedUser banneduser) {
    }

    @Override
    public int compare(ServerEntry s1, ServerEntry s2) {
        if (s1.public_server && !s2.public_server)
            return 1;
        else if (s2.public_server && !s1.public_server)
            return -1;
        if(!s1.public_server && !s2.public_server)
            return s1.servername.compareToIgnoreCase(s2.servername);
        // order of public servers are determined by xml-reply
        return 0;
    }

    private ListFragment getListFragment() {
        return (ListFragment) getSupportFragmentManager().findFragmentById(R.id.list_fragment);
    }

}
