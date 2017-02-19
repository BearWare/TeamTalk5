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
import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Toast;

public class ChannelPropActivity
extends Activity
implements TeamTalkConnectionListener, CommandListener {

    public static final String TAG = "bearware";
    
    public static final String EXTRA_CHANNELID = "channelid",   //edit existing channel
                               EXTRA_PARENTID = "parentid";     //create new channel

    public static final int REQUEST_AUDIOCODEC = 1,
                            REQUEST_AUDIOCONFIG = 2;
    
    TeamTalkConnection mConnection;
    TeamTalkService ttservice;
    TeamTalkBase ttclient;

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if(resultCode == RESULT_OK) {
            channel.audiocodec = Utils.getAudioCodec(data);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_channel_prop);
        getActionBar().setDisplayHomeAsUpEnabled(true);        
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
                exchangeChannel(true);
                if(channel.nChannelID > 0) {
                    
                    updateCmdId = ttclient.doUpdateChannel(channel);
                    if(updateCmdId < 0) {
                        Toast.makeText(this, getResources().getString(R.string.text_con_cmderr),
                                       Toast.LENGTH_LONG).show();
                    }
                }
                else {
                    exchangeChannel(true);
                    
                    updateCmdId = ttclient.doJoinChannel(channel);
                    if(updateCmdId > 0)
                        ttservice.setJoinChannel(channel);
                    else {
                        Toast.makeText(this, getResources().getString(R.string.text_con_cmderr),
                                       Toast.LENGTH_LONG).show();
                    }
                }
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

    @Override
    public void onResume() {
        super.onResume();
    }

    Channel channel;

    @Override
    protected void onStart() {
        super.onStart();
        
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

        if(ttservice != null) {
            ttservice.unregisterCommandListener(ChannelPropActivity.this);            
        }
        // Unbind from the service
        if(mConnection.isBound()) {
            unbindService(mConnection);
            mConnection.setBound(false);
        }
    }

    void exchangeChannel(boolean store) {

        EditText chanName = (EditText) findViewById(R.id.channame);
        EditText chanTopic = (EditText) findViewById(R.id.chantopic);
        EditText chanPasswd = (EditText) findViewById(R.id.chanpasswd);
        EditText chanOpPasswd = (EditText) findViewById(R.id.chanoppasswd);
        EditText chanMaxUsers = (EditText) findViewById(R.id.chanmaxusers);
        EditText chanDiskQuota = (EditText) findViewById(R.id.chandiskquota);
        CheckBox chanPermanent = (CheckBox) findViewById(R.id.chan_permanent);
        CheckBox chanNoInterrupt = (CheckBox) findViewById(R.id.chan_nointerrupt);
        CheckBox chanClassroom = (CheckBox)findViewById(R.id.chan_classroom);
        CheckBox chanOpRecvOnly = (CheckBox)findViewById(R.id.chan_oprecvonly);
        CheckBox chanNoVoiceAct = (CheckBox)findViewById(R.id.chan_novoiceact);
        CheckBox chanNoAudioRec = (CheckBox)findViewById(R.id.chan_noaudiorecord);

        if (store) {
            channel.szName = chanName.getText().toString();
            channel.szTopic = chanTopic.getText().toString();
            channel.szPassword = chanPasswd.getText().toString();
            channel.szOpPassword = chanOpPasswd.getText().toString();
            channel.nMaxUsers = Integer.parseInt(chanMaxUsers.getText().toString());
            channel.nDiskQuota = Long.parseLong(chanDiskQuota.getText().toString());
            channel.nDiskQuota *= 1024;
            
            if(chanPermanent.isChecked())
                channel.uChannelType |= ChannelType.CHANNEL_PERMANENT;
            else
                channel.uChannelType &= ~ChannelType.CHANNEL_PERMANENT;
            if(chanNoInterrupt.isChecked())
                channel.uChannelType |= ChannelType.CHANNEL_SOLO_TRANSMIT;
            else
                channel.uChannelType &= ~ChannelType.CHANNEL_SOLO_TRANSMIT;
            if(chanClassroom.isChecked())
                channel.uChannelType |= ChannelType.CHANNEL_CLASSROOM;
            else
                channel.uChannelType &= ~ChannelType.CHANNEL_CLASSROOM;
            if(chanOpRecvOnly.isChecked())
                channel.uChannelType |= ChannelType.CHANNEL_OPERATOR_RECVONLY;
            else
                channel.uChannelType &= ~ChannelType.CHANNEL_OPERATOR_RECVONLY;
            if(chanNoVoiceAct.isChecked())
                channel.uChannelType |= ChannelType.CHANNEL_NO_VOICEACTIVATION;
            else
                channel.uChannelType &= ~ChannelType.CHANNEL_NO_VOICEACTIVATION;
            if(chanNoAudioRec.isChecked())
                channel.uChannelType |= ChannelType.CHANNEL_NO_RECORDING;
            else
                channel.uChannelType &= ~ChannelType.CHANNEL_NO_RECORDING;
        }
        else {
            chanName.setFocusable(channel.nParentID > 0);
            chanName.setText(channel.szName);
            chanTopic.setText(channel.szTopic);
            chanPasswd.setText(channel.szPassword);
            chanOpPasswd.setText(channel.szOpPassword);
            chanMaxUsers.setText(Integer.toString(channel.nMaxUsers));
            chanDiskQuota.setText(Long.toString(channel.nDiskQuota / 1024));
            
            chanPermanent.setChecked((channel.uChannelType & ChannelType.CHANNEL_PERMANENT) != 0);
            chanNoInterrupt.setChecked((channel.uChannelType & ChannelType.CHANNEL_SOLO_TRANSMIT) != 0);
            chanClassroom.setChecked((channel.uChannelType & ChannelType.CHANNEL_CLASSROOM) != 0);
            chanOpRecvOnly.setChecked((channel.uChannelType & ChannelType.CHANNEL_OPERATOR_RECVONLY) != 0);
            chanNoVoiceAct.setChecked((channel.uChannelType & ChannelType.CHANNEL_NO_VOICEACTIVATION) != 0);
            chanNoAudioRec.setChecked((channel.uChannelType & ChannelType.CHANNEL_NO_RECORDING) != 0);
        }
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
        ttservice = service;
        ttclient = ttservice.getTTInstance();

        ttservice.registerCommandListener(ChannelPropActivity.this);

        int channelid = getIntent().getExtras().getInt(EXTRA_CHANNELID);
        int parentid = getIntent().getExtras().getInt(EXTRA_PARENTID);
        if(channelid > 0) {
            //existing channel
            channel = ttservice.getChannels().get(channelid);
        }
        else if(parentid > 0) {
            //create new channel
            channel = new Channel(true, true);
            channel.nParentID = parentid;
            ServerProperties prop = new ServerProperties();
            if(ttservice.getTTInstance().getServerProperties(prop)) {
                channel.nMaxUsers = prop.nMaxUsers;
            }
        }

        exchangeChannel(false);

        Button codec_btn = (Button) findViewById(R.id.setup_audcodec_btn);
//        Button audcfg_btn = (Button) findViewById(R.id.setup_audcfg_btn);
        
        OnClickListener listener = new OnClickListener() {
            @Override
            public void onClick(View v) {
                switch(v.getId()) {
                    case R.id.setup_audcodec_btn :
                        Intent edit = new Intent(ChannelPropActivity.this, AudioCodecActivity.class);
                        edit = Utils.putAudioCodec(edit, channel.audiocodec);
                        startActivityForResult(edit, REQUEST_AUDIOCODEC);
                        break;
                }
            }
        };
        codec_btn.setOnClickListener(listener);
//        audcfg_btn.setOnClickListener(listener);

    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
    }

    int updateCmdId = 0;

    @Override
    public void onCmdError(int cmdId, ClientErrorMsg errmsg) {
        if (updateCmdId == cmdId) {
            updateCmdId = 0;
        }
    }

    @Override
    public void onCmdSuccess(int cmdId) {
        setResult(RESULT_OK);
        finish();
    }

    @Override
    public void onCmdProcessing(int cmdId, boolean complete) {
    }

    @Override
    public void onCmdMyselfLoggedIn(int my_userid, UserAccount useraccount) {
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
