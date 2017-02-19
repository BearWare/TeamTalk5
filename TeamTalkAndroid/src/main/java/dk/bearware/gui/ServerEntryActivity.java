/*
 * Copyright (c) 2005-2017, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

package dk.bearware.gui;

import dk.bearware.BannedUser;
import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.TeamTalkBase;
import dk.bearware.TextMessage;
import dk.bearware.User;
import dk.bearware.UserAccount;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.data.ServerEntry;
import dk.bearware.events.CommandListener;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

public class ServerEntryActivity
extends PreferenceActivity
implements OnPreferenceChangeListener, TeamTalkConnectionListener, CommandListener {

    public static final String TAG = "bearware";

    TeamTalkConnection mConnection;
    TeamTalkService ttservice;
    TeamTalkBase ttclient;
    ServerEntry serverentry;
    
    @SuppressWarnings("deprecation")
    @Deprecated
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.pref_serverentry);
        getActionBar().setDisplayHomeAsUpEnabled(true);

        ServerEntry entry = Utils.getServerEntry(this.getIntent());
        if(entry != null) {
            showServer(entry);
        }
        
        findPreference(ServerEntry.KEY_SERVERNAME).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_IPADDR).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_TCPPORT).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_UDPPORT).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_ENCRYPTED).setOnPreferenceChangeListener(this);
        PreferenceCategory category = (PreferenceCategory)findPreference("srv_info");
        if(category != null)category.removePreference(findPreference(ServerEntry.KEY_ENCRYPTED));
        findPreference(ServerEntry.KEY_USERNAME).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_PASSWORD).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_NICKNAME).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_REMEMBER_LAST_CHANNEL).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_CHANNEL).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_CHANPASSWD).setOnPreferenceChangeListener(this);
    }
    
    @Override
    protected void onResume() {
        super.onResume();
    }
    
    @Override
    protected void onPause() {
        super.onPause();
    }
    
    @Override
    protected void onStart() {
        super.onStart();
        
        if ((serverentry != null) && serverentry.rememberLastChannel) {
            showServer(serverentry);
            serverentry = null;
        }
        
        // Bind to LocalService
        Intent intent = new Intent(getApplicationContext(), TeamTalkService.class);
        mConnection = new TeamTalkConnection(this);
        if(!bindService(intent, mConnection, Context.BIND_AUTO_CREATE))
            Log.e(TAG, "Failed to bind to TeamTalk service");
        else
            mConnection.setBound(true);
    }

    @Override
    protected void onStop() {
        super.onStop();

        if (ttservice != null) {
            if (isFinishing() && ttservice != null)
                ttservice.resetState();
            ttservice.unregisterCommandListener(this);
        }
        // Unbind from the service
        if(mConnection.isBound()) {
            unbindService(mConnection);
            mConnection.setBound(false);
        }
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.server_entry, menu);
        
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_connect : {
                serverentry = getServerEntry();
                ttservice.setServerEntry(serverentry);
                if (!ttservice.reconnect())
                    Toast.makeText(this, R.string.err_connection, Toast.LENGTH_LONG).show();
            }
            break;
            case R.id.action_saveserver : {
                Intent intent = this.getIntent();
                ServerEntry server = getServerEntry();
                server.public_server = false;
                setResult(RESULT_OK, Utils.putServerEntry(intent, server)); 
                finish();
            }
            break;
            case android.R.id.home : {
                setResult(RESULT_CANCELED);
                finish();
            }
            break;
            default :
                return super.onOptionsItemSelected(item);
        }
        return true;
    }
    
    @SuppressWarnings("deprecation")
    @Deprecated
    ServerEntry getServerEntry() {
        ServerEntry server = new ServerEntry();
        server.servername = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_SERVERNAME), "");
        server.ipaddr = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_IPADDR), "" );
        server.tcpport = Integer.parseInt(Utils.getEditTextPreference(findPreference(ServerEntry.KEY_TCPPORT), "10333"));
        server.udpport = Integer.parseInt(Utils.getEditTextPreference(findPreference(ServerEntry.KEY_UDPPORT), "10333"));
        CheckBoxPreference p = ((CheckBoxPreference)findPreference(ServerEntry.KEY_ENCRYPTED));
        server.encrypted = (p != null)? p.isChecked() : false;
        server.username = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_USERNAME), "");
        server.password = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_PASSWORD), "");
        server.nickname = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_NICKNAME), "");
        server.rememberLastChannel = ((CheckBoxPreference)findPreference(ServerEntry.KEY_REMEMBER_LAST_CHANNEL)).isChecked();
        server.channel = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_CHANNEL), "");
        server.chanpasswd = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_CHANPASSWD), "");
        return server;
    }
    
    @SuppressWarnings("deprecation")
    @Deprecated
    void showServer(ServerEntry entry) {
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_SERVERNAME), entry.servername, entry.servername);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_IPADDR), entry.ipaddr, entry.ipaddr);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_TCPPORT), String.valueOf(entry.tcpport), String.valueOf(entry.tcpport));        
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_UDPPORT), String.valueOf(entry.udpport), String.valueOf(entry.udpport));
        CheckBoxPreference p = (CheckBoxPreference)findPreference(ServerEntry.KEY_ENCRYPTED); 
        if(p != null)p.setChecked(entry.encrypted);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_USERNAME), entry.username, entry.username);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_PASSWORD), entry.password, entry.password);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_NICKNAME), entry.nickname, entry.nickname);
        ((CheckBoxPreference)findPreference(ServerEntry.KEY_REMEMBER_LAST_CHANNEL)).setChecked(entry.rememberLastChannel);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_CHANNEL), entry.channel, entry.channel);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_CHANPASSWD), entry.chanpasswd, entry.chanpasswd);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        
        if(preference instanceof EditTextPreference) { 
            EditTextPreference editTextPreference =  (EditTextPreference)preference;
            editTextPreference.setSummary(newValue.toString());
        }
        return true;
    }
    
    @Override
    public void onServiceConnected(TeamTalkService service) {
        ttservice = service;
        ttclient = service.getTTInstance();

        ttservice.registerCommandListener(this);

        // reset state since we're creating a new connection
        ttservice.resetState();
        ttclient.closeSoundInputDevice();
        ttclient.closeSoundOutputDevice();
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
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
        startActivity(intent.putExtra(ServerEntry.KEY_SERVERNAME, serverentry.servername));
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

}
