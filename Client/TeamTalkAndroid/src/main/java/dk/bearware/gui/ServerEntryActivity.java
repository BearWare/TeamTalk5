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
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import java.util.Locale;

import com.google.android.material.textfield.TextInputEditText;

import dk.bearware.gui.databinding.ActivityServerEntryBinding;

import dk.bearware.TeamTalkBase;
import dk.bearware.UserAccount;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.data.AppInfo;
import dk.bearware.data.ServerEntry;
import dk.bearware.events.ClientEventListener;

public class ServerEntryActivity extends AppCompatActivity
        implements TeamTalkConnectionListener,
        ClientEventListener.OnCmdMyselfLoggedInListener {

    private static final String TAG = "bearware";
    private static final int MIN_PORT = 1;
    private static final int MAX_PORT = 65535;

    private TeamTalkConnection mConnection;
    private ServerEntry serverentry;
    private ActivityServerEntryBinding binding;

    TeamTalkService getService() {
        return mConnection.getService();
    }

    TeamTalkBase getClient() {
        return getService().getTTInstance();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mConnection = new TeamTalkConnection(this);
        binding = ActivityServerEntryBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        EdgeToEdgeHelper.enableEdgeToEdge(this);

        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        
        setupListeners();
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        ServerEntry entry = Utils.getServerEntry(this.getIntent());
        if(entry != null) {
            showServer(entry);
        }
        else {
            binding.serverStatusSection.setVisibility(View.GONE);
        }
    }


    private void setupListeners() {
        binding.webLoginCheckbox.setOnCheckedChangeListener((buttonView, isChecked) -> onWebLoginChanged(isChecked));
        binding.rememberLastChannelCheckbox.setOnCheckedChangeListener((buttonView, isChecked) -> setChannelViewsVisibility(!isChecked));
        
        binding.tcpPortEdit.addTextChangedListener(new PortTextWatcher(binding.tcpPortEdit));
        binding.udpPortEdit.addTextChangedListener(new PortTextWatcher(binding.udpPortEdit));
    }

    private void setChannelViewsVisibility(boolean visible) {
        int visibility = visible ? View.VISIBLE : View.GONE;
        binding.channelLabel.setVisibility(visibility);
        binding.channelLayout.setVisibility(visibility);
        binding.channelPasswordLabel.setVisibility(visibility);
        binding.channelPasswordLayout.setVisibility(visibility);
    }

    private record PortTextWatcher(TextInputEditText editText) implements TextWatcher {

        @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                String text = s.toString().trim();
                if (text.isEmpty()) {
                    editText.setError(null);
                    return;
                }

                try {
                    int port = Integer.parseInt(text);
                    if (port < MIN_PORT || port > MAX_PORT) {
                        editText.setError("Port must be between " + MIN_PORT + " and " + MAX_PORT);
                    } else {
                        editText.setError(null);
                    }
                } catch (NumberFormatException e) {
                    editText.setError("Invalid port number");
                }
            }
        }

    @Override
    protected void onResume() {
        super.onResume();
        if (mConnection.isBound()) {
            resetTeamTalkService();
            getService().getEventHandler().registerOnCmdMyselfLoggedIn(this, true);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mConnection.isBound()) {
            getService().getEventHandler().registerOnCmdMyselfLoggedIn(this, false);
        }
    }

    private void resetTeamTalkService() {
        getService().resetState();
        getClient().closeSoundInputDevice();
        getClient().closeSoundOutputDevice();
    }

    @Override
    protected void onStart() {
        super.onStart();
        if (serverentry != null) {
            showServer(serverentry);
            serverentry = null;
        }
        bindToTeamTalkService();
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (isFinishing()) {
            unbindFromTeamTalkService();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unbindFromTeamTalkService();
        binding = null;
        Log.d(TAG, "Activity destroyed " + this.hashCode());
    }

    private void bindToTeamTalkService() {
        if (!mConnection.isBound()) {
            Intent intent = new Intent(getApplicationContext(), TeamTalkService.class);
            if (!bindService(intent, mConnection, Context.BIND_AUTO_CREATE)) {
                Log.e(TAG, "Failed to bind to TeamTalk service");
            }
        }
    }

    private void unbindFromTeamTalkService() {
        if (mConnection.isBound()) {
            getService().resetState();
            onServiceDisconnected(getService());
            unbindService(mConnection);
            mConnection.setBound(false);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.server_entry, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_connect:
                connectToServer();
                break;
            case R.id.action_saveserver:
                saveServerAndFinish();
                break;
            case android.R.id.home:
                setResult(RESULT_CANCELED);
                finish();
                break;
            default:
                return super.onOptionsItemSelected(item);
        }
        return true;
    }

    private void connectToServer() {
        serverentry = getServerEntry();
        getService().setServerEntry(serverentry);
        if (!getService().reconnect()) {
            Toast.makeText(this, R.string.err_connection, Toast.LENGTH_LONG).show();
        }
    }

    private void saveServerAndFinish() {
        ServerEntry server = getServerEntry();
        server.servertype = ServerEntry.ServerType.LOCAL;
        Intent intent = Utils.putServerEntry(getIntent(), server);
        setResult(RESULT_OK, intent);
        finish();
    }

    private ServerEntry getServerEntry() {
        ServerEntry server = new ServerEntry();
        server.servername = getTextValue(binding.serverNameEdit);
        server.ipaddr = getTextValue(binding.ipAddressEdit);
        server.tcpport = parsePort(getTextValue(binding.tcpPortEdit));
        server.udpport = parsePort(getTextValue(binding.udpPortEdit));
        server.encrypted = binding.encryptedCheckbox.isChecked();
        server.username = getTextValue(binding.usernameEdit);
        server.password = getTextValue(binding.passwordEdit);
        server.nickname = getTextValue(binding.nicknameEdit);
        server.statusmsg = getTextValue(binding.statusmsgEdit);
        server.rememberLastChannel = binding.rememberLastChannelCheckbox.isChecked();
        server.channel = getTextValue(binding.channelEdit);
        server.chanpasswd = getTextValue(binding.channelPasswordEdit);
        return server;
    }

    private int parsePort(String portStr) {
        int defaultPort = getDefaultPort();
        if (portStr.isEmpty()) {
            return defaultPort;
        }
        try {
            int port = Integer.parseInt(portStr);
            return (port >= MIN_PORT && port <= MAX_PORT) ? port : defaultPort;
        } catch (NumberFormatException e) {
            return defaultPort;
        }
    }

    private int getDefaultPort() {
        try {
            return Integer.parseInt(getString(R.string.default_port));
        } catch (NumberFormatException e) {
            return 10333; // Fallback value
        }
    }


    private String getTextValue(TextInputEditText editText) {
        return editText.getText() != null ? editText.getText().toString().trim() : "";
    }
    
    private void showServer(ServerEntry entry) {
        populateServerInfo(entry);
        populateServerStatus(entry);
        populateConnectionSettings(entry);
        populateAuthenticationSettings(entry);
        populateChannelSettings(entry);
    }

    private void populateServerInfo(ServerEntry entry) {
        binding.serverNameEdit.setText(entry.servername);
    }

    private void populateServerStatus(ServerEntry entry) {
        boolean isLocal = (entry.servertype == ServerEntry.ServerType.LOCAL);
        binding.serverStatusSection.setVisibility(isLocal ? View.GONE : View.VISIBLE);
        
        if (!isLocal) {
            binding.userCountText.setText(formatServerInfo(R.string.pref_title_server_usercount, String.valueOf(entry.stats_usercount)));
            binding.motdText.setText(formatServerInfo(R.string.pref_title_server_motd, entry.stats_motd));
            binding.countryText.setText(formatServerInfo(R.string.pref_title_server_country, getCountryDisplayName(entry.stats_country)));
        }
    }

    private String getCountryDisplayName(String countryCode) {
        if (countryCode == null || countryCode.trim().isEmpty()) {
            return countryCode;
        }
        
        try {
            Locale locale = new Locale("", countryCode.toUpperCase(Locale.ROOT));
            String displayName = locale.getDisplayCountry();
            return displayName.isEmpty() ? countryCode : displayName;
        } catch (Exception e) {
            return countryCode;
        }
    }

    private void populateConnectionSettings(ServerEntry entry) {
        binding.ipAddressEdit.setText(entry.ipaddr);
        binding.tcpPortEdit.setText(String.valueOf(entry.tcpport));
        binding.udpPortEdit.setText(String.valueOf(entry.udpport));
        binding.encryptedCheckbox.setChecked(entry.encrypted);
    }

    private void populateAuthenticationSettings(ServerEntry entry) {
        boolean weblogin = Utils.isWebLogin(entry.username);
        binding.usernameEdit.setText(entry.username);
        binding.passwordEdit.setText(entry.password);
        setAuthFieldsEnabled(!weblogin);
        binding.webLoginCheckbox.setChecked(weblogin);
        binding.nicknameEdit.setText(entry.nickname);
        binding.statusmsgEdit.setText(entry.statusmsg);
    }

    private void populateChannelSettings(ServerEntry entry) {
        binding.rememberLastChannelCheckbox.setChecked(entry.rememberLastChannel);
        binding.channelEdit.setText(entry.channel);
        binding.channelPasswordEdit.setText(entry.chanpasswd);
        setChannelViewsVisibility(!entry.rememberLastChannel);
    }

    private String formatServerInfo(int titleResId, String value) {
        return getString(titleResId) + ": " + value;
    }

    private void setAuthFieldsEnabled(boolean enabled) {
        binding.usernameEdit.setEnabled(enabled);
        binding.passwordEdit.setEnabled(enabled);
    }

    private void onWebLoginChanged(boolean weblogin) {
        setAuthFieldsEnabled(!weblogin);
        
        if (weblogin) {
            binding.usernameEdit.setText(AppInfo.WEBLOGIN_BEARWARE_USERNAME);
            binding.passwordEdit.setText("");
        } else {
            ServerEntry entry = serverentry != null ? serverentry : Utils.getServerEntry(getIntent());
            if (entry != null) {
                binding.usernameEdit.setText(entry.username);
                binding.passwordEdit.setText(entry.password);
            }
        }
    }
    
    @Override
    public void onServiceConnected(TeamTalkService service) {
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
}
