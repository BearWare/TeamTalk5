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

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.text.InputType;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.preference.CheckBoxPreference;
import androidx.preference.EditTextPreference;
import androidx.preference.Preference;
import androidx.preference.Preference.OnPreferenceChangeListener;
import androidx.preference.PreferenceCategory;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceScreen;

import dk.bearware.TeamTalkBase;
import dk.bearware.UserAccount;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.data.AppInfo;
import dk.bearware.data.ServerEntry;
import dk.bearware.events.ClientEventListener;

public class ServerEntryActivity
extends AppCompatActivity
implements OnPreferenceChangeListener,
        TeamTalkConnectionListener,
        ClientEventListener.OnCmdMyselfLoggedInListener {

    public static final String TAG = "bearware";

    TeamTalkConnection mConnection;
    TeamTalkService ttservice;
    TeamTalkBase ttclient;
    ServerEntry serverentry;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getSupportFragmentManager().beginTransaction()
            .replace(android.R.id.content, new ServerPreferencesFragment())
            .commit();
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        ServerEntry entry = Utils.getServerEntry(this.getIntent());
        if(entry != null) {
            showServer(entry);
        }
        else {
            PreferenceScreen prefscreen = (PreferenceScreen)findPreference(ServerEntry.KEY_PREFSCREEN);
            prefscreen.removePreference(findPreference(ServerEntry.KEY_SRVSTATUS));
        }

        findPreference(ServerEntry.KEY_SERVERNAME).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_IPADDR).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_TCPPORT).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_UDPPORT).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_ENCRYPTED).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_USERNAME).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_PASSWORD).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_WEBLOGIN).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_NICKNAME).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_REMEMBER_LAST_CHANNEL).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_CHANNEL).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_CHANPASSWD).setOnPreferenceChangeListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (mConnection.isBound()) {
            // reset state since we're creating a new connection
            ttservice.resetState();
            ttclient.closeSoundInputDevice();
            ttclient.closeSoundOutputDevice();
            ttservice.getEventHandler().registerOnCmdMyselfLoggedIn(this, true);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mConnection.isBound())
            ttservice.getEventHandler().registerOnCmdMyselfLoggedIn(this, false);
    }

    @Override
    protected void onStart() {
        super.onStart();
                
        // Bind to LocalService if not already
        if (mConnection == null)
            mConnection = new TeamTalkConnection(this);
        if (!mConnection.isBound()) {
            Intent intent = new Intent(getApplicationContext(), TeamTalkService.class);
            if(!bindService(intent, mConnection, Context.BIND_AUTO_CREATE))
                Log.e(TAG, "Failed to bind to TeamTalk service");
        }
    }

    @Override
    protected void onStop() {
        super.onStop();

        if (isFinishing() && mConnection.isBound()) {
            // Unbind from the service
            ttservice.resetState();
            onServiceDisconnected(ttservice);
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

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.server_entry, menu);
        
        return true;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
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
                server.servertype = ServerEntry.ServerType.LOCAL;
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

    @Deprecated
    ServerEntry getServerEntry() {
        ServerEntry server = new ServerEntry();
        server.servername = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_SERVERNAME), "");
        server.ipaddr = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_IPADDR), "" );
        server.tcpport = Integer.parseInt(Utils.getEditTextPreference(findPreference(ServerEntry.KEY_TCPPORT), "10333"));
        server.udpport = Integer.parseInt(Utils.getEditTextPreference(findPreference(ServerEntry.KEY_UDPPORT), "10333"));
        CheckBoxPreference p = ((CheckBoxPreference)findPreference(ServerEntry.KEY_ENCRYPTED));
        server.encrypted = p.isChecked();
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

        // server info
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_SERVERNAME), entry.servername, entry.servername);

        // server status
        if (entry.servertype == ServerEntry.ServerType.LOCAL) {
            PreferenceScreen prefscreen = (PreferenceScreen)findPreference(ServerEntry.KEY_PREFSCREEN);
            prefscreen.removePreference(findPreference(ServerEntry.KEY_SRVSTATUS));
        }
        else {
            findPreference(ServerEntry.KEY_MOTD).setSummary(entry.stats_motd);
            findPreference(ServerEntry.KEY_USERCOUNT).setSummary(String.valueOf(entry.stats_usercount));
            findPreference(ServerEntry.KEY_COUNTRY).setSummary(entry.stats_country);
        }

        // connection
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_IPADDR), entry.ipaddr, entry.ipaddr);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_TCPPORT), String.valueOf(entry.tcpport), String.valueOf(entry.tcpport));        
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_UDPPORT), String.valueOf(entry.udpport), String.valueOf(entry.udpport));
        CheckBoxPreference p = (CheckBoxPreference)findPreference(ServerEntry.KEY_ENCRYPTED); 
        p.setChecked(entry.encrypted);

        // auth
        boolean weblogin = Utils.isWebLogin(entry.username);
        PreferenceCategory authcat = (PreferenceCategory)findPreference("auth_info");
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_USERNAME), entry.username, entry.username, weblogin);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_PASSWORD), entry.password, entry.password, weblogin);

        findPreference(ServerEntry.KEY_USERNAME).setEnabled(!weblogin);
        findPreference(ServerEntry.KEY_PASSWORD).setEnabled(!weblogin);
        ((CheckBoxPreference)findPreference(ServerEntry.KEY_WEBLOGIN)).setChecked(weblogin);

        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_NICKNAME), entry.nickname, entry.nickname);

        // join channel
        ((CheckBoxPreference)findPreference(ServerEntry.KEY_REMEMBER_LAST_CHANNEL)).setChecked(entry.rememberLastChannel);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_CHANNEL), entry.channel, entry.channel);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_CHANPASSWD), entry.chanpasswd, entry.chanpasswd);
    }

    @Override
    public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
        
        if(preference instanceof EditTextPreference) { 
            EditTextPreference editTextPreference =  (EditTextPreference)preference;
            editTextPreference.setSummary(newValue.toString());
        }

        if(findPreference(ServerEntry.KEY_WEBLOGIN) == preference) {
            boolean weblogin = (Boolean)newValue;
            CheckBoxPreference cbp = (CheckBoxPreference)preference;
            findPreference(ServerEntry.KEY_USERNAME).setEnabled(!weblogin);
            findPreference(ServerEntry.KEY_PASSWORD).setEnabled(!weblogin);

            ServerEntry entry = serverentry == null? Utils.getServerEntry(this.getIntent()) : serverentry;
            if(entry != null) {
                String username = weblogin? AppInfo.WEBLOGIN_BEARWARE_USERNAME : entry.username;
                String password = weblogin? "" : entry.password;
                Utils.setEditTextPreference(findPreference(ServerEntry.KEY_USERNAME), username, username, weblogin);
                Utils.setEditTextPreference(findPreference(ServerEntry.KEY_PASSWORD), password, password, weblogin);
            }
            else if(weblogin){
                Utils.setEditTextPreference(findPreference(ServerEntry.KEY_USERNAME), AppInfo.WEBLOGIN_BEARWARE_USERNAME, AppInfo.WEBLOGIN_BEARWARE_USERNAME);
                Utils.setEditTextPreference(findPreference(ServerEntry.KEY_PASSWORD), "", "", weblogin);
            }
        }

        return true;
    }
    
    @Override
    public void onServiceConnected(TeamTalkService service) {
        ttservice = service;
        ttclient = service.getTTInstance();
        service.getEventHandler().registerOnCmdMyselfLoggedIn(this, true);
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
        service.getEventHandler().unregisterListener(this);
    }

    @Override
    public void onCmdMyselfLoggedIn(int my_userid, UserAccount useraccount) {
        Intent intent = new Intent(getBaseContext(), MainActivity.class);
        startActivity(intent.putExtra(ServerEntry.KEY_SERVERNAME, serverentry.servername));
    }

    private Preference findPreference(CharSequence key) {
        return ((PreferenceFragmentCompat) getSupportFragmentManager().findFragmentById(android.R.id.content)).findPreference(key);
    }


    public static class ServerPreferencesFragment extends PreferenceFragmentCompat {

        private final androidx.preference.EditTextPreference.OnBindEditTextListener onBindEditTextListener = editText -> editText.setInputType(InputType.TYPE_CLASS_NUMBER);

        @Override
        public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
            setPreferencesFromResource(R.xml.pref_serverentry, rootKey);
            androidx.preference.EditTextPreference tcpPortPreference = getPreferenceManager().findPreference("tcpport");
            tcpPortPreference.setOnBindEditTextListener(onBindEditTextListener);
            androidx.preference.EditTextPreference udpPortPreference = getPreferenceManager().findPreference("udpport");
            udpPortPreference.setOnBindEditTextListener(onBindEditTextListener);
        }

    }

}
