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

import dk.bearware.gui.R;

import dk.bearware.data.ServerEntry;
import android.content.Intent;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.view.Menu;
import android.view.MenuItem;

public class ServerEntryActivity extends PreferenceActivity implements OnPreferenceChangeListener {

    @SuppressWarnings("deprecation")
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
        findPreference(ServerEntry.KEY_USERNAME).setOnPreferenceChangeListener(this);
        findPreference(ServerEntry.KEY_PASSWORD).setOnPreferenceChangeListener(this);
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
    protected void onStop() {
        super.onStop();
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
                Intent intent = new Intent(this, MainActivity.class);

                ServerEntry server = getServerEntry();
                startActivity(Utils.putServerEntry(intent, server));
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
    ServerEntry getServerEntry() {
        ServerEntry server = new ServerEntry();
        server.servername = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_SERVERNAME));
        server.ipaddr = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_IPADDR));
        server.tcpport = Integer.parseInt(Utils.getEditTextPreference(findPreference(ServerEntry.KEY_TCPPORT)));
        server.udpport = Integer.parseInt(Utils.getEditTextPreference(findPreference(ServerEntry.KEY_UDPPORT)));
        server.encrypted = ((CheckBoxPreference)findPreference(ServerEntry.KEY_ENCRYPTED)).isChecked();
        server.username = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_USERNAME));
        server.password = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_PASSWORD));
        server.rememberLastChannel = ((CheckBoxPreference)findPreference(ServerEntry.KEY_REMEMBER_LAST_CHANNEL)).isChecked();
        server.channel = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_CHANNEL));
        server.chanpasswd = Utils.getEditTextPreference(findPreference(ServerEntry.KEY_CHANPASSWD));
        return server;
    }
    
    @SuppressWarnings("deprecation")
    void showServer(ServerEntry entry) {
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_SERVERNAME), entry.servername, entry.servername);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_IPADDR), entry.ipaddr, entry.ipaddr);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_TCPPORT), String.valueOf(entry.tcpport), String.valueOf(entry.tcpport));        
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_UDPPORT), String.valueOf(entry.udpport), String.valueOf(entry.udpport));
        ((CheckBoxPreference)findPreference(ServerEntry.KEY_ENCRYPTED)).setChecked(entry.encrypted);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_USERNAME), entry.username, entry.username);
        Utils.setEditTextPreference(findPreference(ServerEntry.KEY_PASSWORD), entry.password, entry.password);
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
}
