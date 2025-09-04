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
import dk.bearware.events.ClientEventListener;
import dk.bearware.events.CommandListener;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Toast;

public class ChannelPropActivity
extends AppCompatActivity
implements TeamTalkConnectionListener, ClientEventListener.OnCmdErrorListener, ClientEventListener.OnCmdSuccessListener {

    public static final String TAG = "bearware";

    public static final String EXTRA_CHANNELID = "channelid",   //edit existing channel
                               EXTRA_PARENTID = "parentid";     //create new channel

    public static final int REQUEST_AUDIOCODEC = 1,
                            REQUEST_AUDIOCONFIG = 2;
    
    TeamTalkConnection mConnection;
    Channel channel;

    TeamTalkService getService() {
        return mConnection.getService();
    }

    TeamTalkBase getClient() {
        return getService().getTTInstance();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if(resultCode == RESULT_OK) {
            channel = Utils.getChannel(data);
            channel.audiocodec = Utils.getAudioCodec(data);
            exchangeChannel(false);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mConnection = new TeamTalkConnection(this);
        setContentView(R.layout.activity_channel_prop);
        EdgeToEdgeHelper.enableEdgeToEdge(this);

        getSupportActionBar().setDisplayHomeAsUpEnabled(true);        
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
                    
                    updateCmdId = getClient().doUpdateChannel(channel);
                    if(updateCmdId < 0) {
                        Toast.makeText(this, getResources().getString(R.string.text_con_cmderr),
                                       Toast.LENGTH_LONG).show();
                    }
                }
                else {
                    exchangeChannel(true);
                    
                    updateCmdId = getClient().doJoinChannel(channel);
                    if(updateCmdId > 0)
                        getService().setJoinChannel(channel);
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

    @Override
    protected void onStart() {
        super.onStart();
        
        // Bind to LocalService if not already
        if (!mConnection.isBound()) {
            Intent intent = new Intent(getApplicationContext(), TeamTalkService.class);
            if(!bindService(intent, mConnection, Context.BIND_AUTO_CREATE))
                Log.e(TAG, "Failed to bind to TeamTalk service");
        }
    }

    @Override
    protected void onStop() {
        super.onStop();

        // Unbind from the service
        if(mConnection.isBound()) {
            onServiceDisconnected(getService());
            unbindService(mConnection);
            mConnection.setBound(false);
        }
    }

    void exchangeChannel(boolean store) {

        EditText chanName = findViewById(R.id.channame);
        EditText chanTopic = findViewById(R.id.chantopic);
        EditText chanPasswd = findViewById(R.id.chanpasswd);
        EditText chanOpPasswd = findViewById(R.id.chanoppasswd);
        EditText chanMaxUsers = findViewById(R.id.chanmaxusers);
        EditText chanDiskQuota = findViewById(R.id.chandiskquota);
        CheckBox chanPermanent = findViewById(R.id.chan_permanent);
        CheckBox chanNoInterrupt = findViewById(R.id.chan_nointerrupt);
        CheckBox chanClassroom = findViewById(R.id.chan_classroom);
        CheckBox chanOpRecvOnly = findViewById(R.id.chan_oprecvonly);
        CheckBox chanNoVoiceAct = findViewById(R.id.chan_novoiceact);
        CheckBox chanNoAudioRec = findViewById(R.id.chan_noaudiorecord);
        CheckBox chanHidden = findViewById(R.id.chan_hidden);

        if (store) {
            channel.szName = chanName.getText().toString();
            channel.szTopic = chanTopic.getText().toString();
            channel.szPassword = chanPasswd.getText().toString();
            channel.szOpPassword = chanOpPasswd.getText().toString();
            try {
                channel.nMaxUsers = Integer.parseInt(chanMaxUsers.getText().toString());
            } catch (NumberFormatException e) {
                Log.e(TAG, "Invalid input for channel's max users");
            }
            try {
                channel.nDiskQuota = Long.parseLong(chanDiskQuota.getText().toString());
            } catch (NumberFormatException e) {
                Log.e(TAG, "Invalid input for channel's disk quota");
            }
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
            if(chanHidden.isChecked())
                channel.uChannelType |= ChannelType.CHANNEL_HIDDEN;
            else
                channel.uChannelType &= ~ChannelType.CHANNEL_HIDDEN;
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
            chanHidden.setChecked((channel.uChannelType & ChannelType.CHANNEL_HIDDEN) != 0);
        }
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
        service.getEventHandler().registerOnCmdError(this, true);
        service.getEventHandler().registerOnCmdSuccess(this, true);

        if (channel == null) {
            int channelid = getIntent().getExtras().getInt(EXTRA_CHANNELID);
            int parentid = getIntent().getExtras().getInt(EXTRA_PARENTID);
            if(channelid > 0) {
                //existing channel
                channel = service.getChannels().get(channelid);
                if (channel == null) {
                    setResult(RESULT_CANCELED);
                    finish();
                    return;
                }
            }
            else if(parentid > 0) {
                //create new channel
                channel = new Channel(true, true);
                channel.nParentID = parentid;
                ServerProperties prop = new ServerProperties();
                if (service.getTTInstance().getServerProperties(prop)) {
                    channel.nMaxUsers = prop.nMaxUsers;
                }
            }
        }

        exchangeChannel(false);

        Button codec_btn = findViewById(R.id.setup_audcodec_btn);
//        Button audcfg_btn = (Button) findViewById(R.id.setup_audcfg_btn);
        
        OnClickListener listener = v -> {
            if (v.getId() == R.id.setup_audcodec_btn) {
                Intent edit = new Intent(ChannelPropActivity.this, AudioCodecActivity.class);
                edit = Utils.putAudioCodec(edit, channel.audiocodec);
                exchangeChannel(true);
                edit = Utils.putChannel(edit, channel);
                startActivityForResult(edit, REQUEST_AUDIOCODEC);
            }
        };
        codec_btn.setOnClickListener(listener);
//        audcfg_btn.setOnClickListener(listener);

    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
        service.getEventHandler().unregisterListener(this);
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
}
