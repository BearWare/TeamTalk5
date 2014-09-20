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

import dk.bearware.Channel;
import dk.bearware.ChannelType;
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
import dk.bearware.events.CommandListener;
import dk.bearware.events.ConnectionListener;
import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Toast;

public class ChannelPropActivity
extends Activity
implements TeamTalkConnectionListener, ConnectionListener, CommandListener {

    public static final String TAG = "bearware";
    
    public static final String EXTRA_CHANNELID = "channelid",   //edit existing channel
                               EXTRA_PARENTID = "parentid";     //create new channel

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_channel_prop);
        
        mConnection = new TeamTalkConnection(this);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.channel_prop, menu);
        
        if(getIntent().getExtras().getInt(EXTRA_CHANNELID) == 0) {
            MenuItem item = menu.findItem(R.id.action_updatechannel);
            item.setTitle(getResources().getString(R.string.action_createchannel));
        }
        
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
            case R.id.action_updatechannel : {
                setgetChannel(true);
                if(channel.nChannelID > 0)
                    updateCmdId = ttclient.doUpdateChannel(channel);
                else {
                    setgetChannel(true);
                    updateCmdId = ttclient.doJoinChannel(channel);
                }
            }
            break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    TeamTalkConnection mConnection;
    TeamTalkService ttservice;
    TeamTalkBase ttclient;
    Channel channel;

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
    }

    void setgetChannel(boolean set) {

        EditText chanName = (EditText) findViewById(R.id.channame);
        EditText chanTopic = (EditText) findViewById(R.id.chantopic);
        EditText chanPasswd = (EditText) findViewById(R.id.chanpasswd);
        EditText chanOpPasswd = (EditText) findViewById(R.id.chanoppasswd);
        CheckBox chanPermanent = (CheckBox) findViewById(R.id.chan_permanent);
        CheckBox chanNoInterrupt = (CheckBox) findViewById(R.id.chan_nointerrupt);

        if (set) {
            channel.szName = chanName.getText().toString();
            channel.szTopic = chanTopic.getText().toString();
            channel.szPassword = chanPasswd.getText().toString();
            channel.szOpPassword = chanPasswd.getText().toString();
            
            if(chanPermanent.isChecked())
                channel.uChannelType |= ChannelType.CHANNEL_PERMANENT;
            else
                channel.uChannelType &= ~ChannelType.CHANNEL_PERMANENT;
            if(chanNoInterrupt.isChecked())
                channel.uChannelType |= ChannelType.CHANNEL_SOLO_TRANSMIT;
            else
                channel.uChannelType &= ~ChannelType.CHANNEL_SOLO_TRANSMIT;
        }
        else {
            chanName.setFocusable(channel.nParentID > 0);
            chanName.setText(channel.szName);
            chanTopic.setText(channel.szTopic);
            chanPasswd.setText(channel.szPassword);
            chanOpPasswd.setText(channel.szOpPassword);
            
            chanPermanent.setChecked((channel.uChannelType & ChannelType.CHANNEL_PERMANENT) != 0);
            chanNoInterrupt.setChecked((channel.uChannelType & ChannelType.CHANNEL_SOLO_TRANSMIT) != 0);
        }
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
        ttservice = service;
        ttclient = ttservice.getTTInstance();

        ttservice.registerConnectionListener(ChannelPropActivity.this);
        ttservice.registerCommandListener(ChannelPropActivity.this);

        int channelid = ChannelPropActivity.this.getIntent().getExtras().getInt(EXTRA_CHANNELID);
        int parentid = ChannelPropActivity.this.getIntent().getExtras().getInt(EXTRA_PARENTID);
        if(channelid > 0) {
            channel = ttservice.getChannels().get(channelid);

            setgetChannel(false);
        }
        else if(parentid > 0) {
            channel = new Channel();
            channel.nParentID = parentid;
        }
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
        ttservice.unregisterConnectionListener(ChannelPropActivity.this);
        ttservice.unregisterCommandListener(ChannelPropActivity.this);
        ttservice = null;
    }

    @Override
    public void onConnectSuccess() {
        // TODO Auto-generated method stub

    }

    @Override
    public void onConnectFailed() {
        // TODO Auto-generated method stub

    }

    @Override
    public void onConnectionLost() {
        // TODO Auto-generated method stub

    }

    @Override
    public void onMaxPayloadUpdate(int payload_size) {
        // TODO Auto-generated method stub

    }

    int updateCmdId = 0;

    @Override
    public void onCmdError(int cmdId, ClientErrorMsg errmsg) {
        if (updateCmdId == cmdId) {
            updateCmdId = 0;
            Toast.makeText(this, errmsg.szErrorMsg, Toast.LENGTH_LONG).show();
        }
    }

    @Override
    public void onCmdSuccess(int cmdId) {
        setResult(RESULT_OK);
        finish();
    }

    @Override
    public void onCmdProcessing(int cmdId, boolean complete) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdMyselfLoggedIn(int my_userid, UserAccount useraccount) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdMyselfLoggedOut() {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdMyselfKickedFromChannel() {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdMyselfKickedFromChannel(User kicker) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdUserLoggedIn(User user) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdUserLoggedOut(User user) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdUserUpdate(User user) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdUserJoinedChannel(User user) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdUserLeftChannel(int channelid, User user) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdUserTextMessage(TextMessage textmessage) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdChannelNew(Channel channel) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdChannelUpdate(Channel channel) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdChannelRemove(Channel channel) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdServerUpdate(ServerProperties serverproperties) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdFileNew(RemoteFile remotefile) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdFileRemove(RemoteFile remotefile) {
        // TODO Auto-generated method stub

    }
}
