/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

package dk.bearware.gui;

import java.util.Vector;

import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.Subscription;
import dk.bearware.TeamTalkBase;
import dk.bearware.TextMessage;
import dk.bearware.User;
import dk.bearware.UserAccount;
import dk.bearware.UserState;
import dk.bearware.gui.R;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.events.CommandListener;
import dk.bearware.events.ConnectionListener;
import dk.bearware.data.ServerEntry;

import android.os.AsyncTask;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.preference.PreferenceManager;
import android.app.ListActivity;
import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;
import android.util.SparseArray;
import android.view.View.OnClickListener;
import android.content.Intent;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class ServerListActivity
extends ListActivity
implements TeamTalkConnectionListener, ConnectionListener, CommandListener {

    private ServerListAdapter adapter;

    public static final String TAG = "bearware";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        adapter = new ServerListAdapter(this.getBaseContext());
        setListAdapter(adapter);

        setContentView(R.layout.activity_server_list);        
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    TeamTalkConnection mConnection = new TeamTalkConnection(this);
    TeamTalkService ttservice;
    TeamTalkBase ttclient;
    
    @Override
    protected void onStart() {
        super.onStart();
        
        // Bind to LocalService
        Intent intent = new Intent(getApplicationContext(), TeamTalkService.class);
        if(!bindService(intent, mConnection, Context.BIND_AUTO_CREATE))
            Log.e(TAG, "Failed to bind to TeamTalk service");
    }

    @Override
    protected void onStop() {
        super.onStop();
        // Unbind from the service
         unbindService(mConnection);
         
         if(ttservice != null) {
             ttservice.unregisterConnectionListener(this);
             ttservice.unregisterCommandListener(this);
         }
    }

    ServerEntry serverentry;

    static final int REQUEST_EDITSERVER = 1;
    static final int REQUEST_NEWSERVER = 2;
    static final String POSITION_NAME = "pos";

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch(requestCode) {
            case REQUEST_NEWSERVER : {
                if(resultCode == RESULT_OK) {
                    ServerEntry entry = Utils.getServerEntry(data);
                    if(entry != null) {
                        servers.add(entry);
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
                        if(pos >= 0)
                            servers.removeElementAt(pos);
                        servers.insertElementAt(entry, pos);
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
            case R.id.action_settings : {
                Intent intent = new Intent(ServerListActivity.this, PreferencesActivity.class);
                startActivity(intent);
                break;
            }
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onListItemClick(ListView l, View v, int position, long id) {
        Intent intent = new Intent(this, ServerEntryActivity.class);

        ServerEntry entry = servers.elementAt(position);

        startActivityForResult(Utils.putServerEntry(intent, entry).putExtra(POSITION_NAME, position),
            REQUEST_EDITSERVER);
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
                convertView = inflater.inflate(R.layout.item_serverentry, null);
            
            TextView name = (TextView) convertView.findViewById(R.id.server_name);
            TextView address = (TextView) convertView.findViewById(R.id.server_address);
            name.setText(servers.get(position).servername);
            if(servers.get(position).public_server)
                name.setTextColor(Color.parseColor("#85E58D"));
            address.setText(servers.get(position).ipaddr);
            Button connect = (Button) convertView.findViewById(R.id.server_connect);
            Button remove = (Button) convertView.findViewById(R.id.server_remove);
            OnClickListener listener = new OnClickListener() {
                @Override
                public void onClick(View v) {
                    switch(v.getId()) {
                        case R.id.server_connect :
                            serverentry = servers.get(position);

                            ttservice.setServerEntry(serverentry);
                            if (!ttservice.reconnect())
                                Toast.makeText(ServerListActivity.this, R.string.err_connection, Toast.LENGTH_LONG).show();
                        break;
                        case R.id.server_remove :
                            servers.remove(position);
                            notifyDataSetChanged();
                            saveServers();
                        break;
                    }
                }
            };
            connect.setOnClickListener(listener);
            remove.setOnClickListener(listener);
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
            edit.remove(i + ServerEntry.KEY_USERNAME);
            edit.remove(i + ServerEntry.KEY_PASSWORD);
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

            edit.putString(j + ServerEntry.KEY_USERNAME, servers.get(i).username);
            edit.putString(j + ServerEntry.KEY_PASSWORD, servers.get(i).password);

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
            entry.channel = pref.getString(i + ServerEntry.KEY_CHANNEL, "");
            entry.chanpasswd = pref.getString(i + ServerEntry.KEY_CHANPASSWD, "");
            servers.add(entry);
            i++;
        }
        
        adapter.notifyDataSetChanged();
    }
    
    class UrlAsyncTask extends AsyncTask<Void, Void, Void> {

        Vector<ServerEntry> entries;

        @Override
        protected Void doInBackground(Void... params) {
            final String APPNAME_SHORT = "TeamTalk5", APPVERSION_SHORT = "5.0", OSTYPE = "Android";
            final String TEAMTALK_VERSION = TeamTalkBase.getVersion();
            String urlToRead = "http://www.bearware.dk/teamtalk/tt5servers.php?client=" + APPNAME_SHORT + "&version="
                + APPVERSION_SHORT + "&dllversion=" + TEAMTALK_VERSION + "&os=" + OSTYPE;

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

        // Get public servers from from http. TeamTalk DLL must be loaded by 
        // service, otherwise static methods are unavailable (for getting DLL
        // version number).
        new UrlAsyncTask().execute();
    }
    
    @Override
    public void onServiceConnected(TeamTalkService service) {
        ttservice = service;
        ttclient = service.getTTInstance();

        ttservice.registerConnectionListener(this);
        ttservice.registerCommandListener(this);

        // reset state since we're creating a new connection
        ttservice.resetState();
        ttclient.closeSoundInputDevice();
        ttclient.closeSoundOutputDevice();
        
        refreshServerList();
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
    }

    @Override
    public void onConnectSuccess() {
    }

    @Override
    public void onConnectFailed() {
    }

    @Override
    public void onConnectionLost() {
    }

    @Override
    public void onMaxPayloadUpdate(int payload_size) {
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
        Intent intent = new Intent(getBaseContext(), MainActivity.class);
        startActivity(intent);
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
}
