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
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;

import dk.bearware.ClientEvent;
import dk.bearware.SoundLevel;
import dk.bearware.StreamType;
import dk.bearware.Subscription;
import dk.bearware.TeamTalkBase;
import dk.bearware.Channel;
import dk.bearware.User;
import dk.bearware.UserState;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkService;

public class UserPropActivity extends AppCompatActivity implements TeamTalkConnectionListener {

    public final static String EXTRA_USERID = "userid";

    public static final String TAG = "bearware";
    
    TeamTalkConnection mConnection;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_user_prop);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.user_prop, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
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
        else {
            int userid = getIntent().getExtras().getInt(EXTRA_USERID);
            if(!ttclient.getUser(userid, user)) {
                setResult(RESULT_CANCELED);
                finish();
            }
            else
                showUser();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        
        // Unbind from the service
        if(mConnection.isBound()) {
            unbindService(mConnection);
            mConnection.setBound(false);
        }
    }

    TeamTalkService ttservice;
    TeamTalkBase ttclient;
    User user = new User();

    void showUser() {
        TextView nickname = findViewById(R.id.user_nickname);
        TextView username = findViewById(R.id.user_username);
        TextView userid = findViewById(R.id.user_userid);
        TextView statusmsg = findViewById(R.id.user_statusmsg);
        TextView clientname = findViewById(R.id.user_clientname);
        TextView ipaddress = findViewById(R.id.user_ipaddress);
        final SeekBar voiceVol = findViewById(R.id.user_vol_voiceSeekBar);
        final Button defVoiceBtn = findViewById(R.id.defVoiceVolBtn);
        final SwitchCompat voiceMute = findViewById(R.id.user_mutevoiceSwitch);
        final SeekBar mediaVol = findViewById(R.id.user_vol_mediaSeekBar);
        final Button defMfBtn = findViewById(R.id.defMfVolBtn);
        final SwitchCompat mediaMute = findViewById(R.id.user_mutemediaSwitch);
        final SwitchCompat subscribeTxtmsg = findViewById(R.id.user_subscribetxtmsgSwitch);
        final SwitchCompat subscribeChanmsg = findViewById(R.id.user_subscribechanmsgSwitch);
        final SwitchCompat subscribeBcastmsg = findViewById(R.id.user_subscribebcastmsgSwitch);
        final SwitchCompat subscribeVoice = findViewById(R.id.user_subscribevoiceSwitch);
        final SwitchCompat subscribeVid = findViewById(R.id.user_subscribevidSwitch);
        final SwitchCompat subscribeDesk = findViewById(R.id.user_subscribedeskSwitch);
        final SwitchCompat subscribeMedia = findViewById(R.id.user_subscribemediaSwitch);
        final SwitchCompat subscribeIntercepttxtmsg = findViewById(R.id.user_subscribeintercepttxtmsgSwitch);
        final SwitchCompat subscribeInterceptchanmsg = findViewById(R.id.user_subscribeinterceptchanmsgSwitch);
        final SwitchCompat subscribeInterceptvoice = findViewById(R.id.user_subscribeinterceptvoiceSwitch);
        final SwitchCompat subscribeInterceptvid = findViewById(R.id.user_subscribeinterceptvidSwitch);
        final SwitchCompat subscribeInterceptdesk = findViewById(R.id.user_subscribeinterceptdeskSwitch);
        final SwitchCompat subscribeInterceptmedia = findViewById(R.id.user_subscribeinterceptmediaSwitch);
        final SwitchCompat transmitVoice = findViewById(R.id.user_transmitvoiceSwitch);
        final SwitchCompat transmitVid = findViewById(R.id.user_transmitvidSwitch);
        final SwitchCompat transmitDesk = findViewById(R.id.user_transmitdeskSwitch);
        final SwitchCompat transmitMedia = findViewById(R.id.user_transmitmediaSwitch);
        final SwitchCompat transmitChanmsg = findViewById(R.id.user_transmitchanmsgSwitch);

        nickname.setText(getString(R.string.user_prop_title_nickname) + " " + user.szNickname);
        username.setText(getString(R.string.user_prop_title_username) + " " + user.szUsername);
        userid.setText(getString(R.string.user_prop_title_userid) + " " + user.nUserID);
        statusmsg.setText(getString(R.string.user_prop_title_statusmsg) + " " + user.szStatusMsg);
        clientname.setText(getString(R.string.user_prop_title_clientname) + " " + user.szClientName + " " + getString(R.string.user_prop_clientversion) + " " + ((user.uVersion >> 16) & 0xFF) + "." + ((user.uVersion >> 8) & 0xFF) + "." + (user.uVersion & 0xFF));
        ipaddress.setText(getString(R.string.user_prop_title_ipaddress) + " " + user.szIPAddress);
        voiceVol.setMax(100);
        voiceVol.setProgress(Utils.refVolumeToPercent(user.nVolumeVoice));
        mediaVol.setMax(100);
        mediaVol.setProgress(Utils.refVolumeToPercent(user.nVolumeMediaFile));
        voiceMute.setChecked((user.uUserState & UserState.USERSTATE_MUTE_VOICE) != 0);
        mediaMute.setChecked((user.uUserState & UserState.USERSTATE_MUTE_MEDIAFILE) != 0);
        subscribeTxtmsg.setChecked((user.uLocalSubscriptions & Subscription.SUBSCRIBE_USER_MSG) != 0);
        subscribeChanmsg.setChecked((user.uLocalSubscriptions & Subscription.SUBSCRIBE_CHANNEL_MSG) != 0);
        subscribeBcastmsg.setChecked((user.uLocalSubscriptions & Subscription.SUBSCRIBE_BROADCAST_MSG) != 0);
        subscribeVoice.setChecked((user.uLocalSubscriptions & Subscription.SUBSCRIBE_VOICE) != 0);
        subscribeVid.setChecked((user.uLocalSubscriptions & Subscription.SUBSCRIBE_VIDEOCAPTURE) != 0);
        subscribeDesk.setChecked((user.uLocalSubscriptions & Subscription.SUBSCRIBE_DESKTOP) != 0);
        subscribeMedia.setChecked((user.uLocalSubscriptions & Subscription.SUBSCRIBE_MEDIAFILE) != 0);
        subscribeIntercepttxtmsg.setChecked((user.uLocalSubscriptions & Subscription.SUBSCRIBE_INTERCEPT_USER_MSG) != 0);
        subscribeInterceptchanmsg.setChecked((user.uLocalSubscriptions & Subscription.SUBSCRIBE_INTERCEPT_CHANNEL_MSG) != 0);
        subscribeInterceptvoice.setChecked((user.uLocalSubscriptions & Subscription.SUBSCRIBE_INTERCEPT_VOICE) != 0);
        subscribeInterceptvid.setChecked((user.uLocalSubscriptions & Subscription.SUBSCRIBE_INTERCEPT_VIDEOCAPTURE) != 0);
        subscribeInterceptdesk.setChecked((user.uLocalSubscriptions & Subscription.SUBSCRIBE_INTERCEPT_DESKTOP) != 0);
        subscribeInterceptmedia.setChecked((user.uLocalSubscriptions & Subscription.SUBSCRIBE_INTERCEPT_MEDIAFILE) != 0);

        Channel chan = this.ttservice.getChannels().get(user.nChannelID);
        if (chan != null) {
            transmitVoice.setChecked(Utils.isTransmitAllowed(user, chan, StreamType.STREAMTYPE_VOICE));
            transmitVid.setChecked(Utils.isTransmitAllowed(user, chan, StreamType.STREAMTYPE_VIDEOCAPTURE));
            transmitDesk.setChecked(Utils.isTransmitAllowed(user, chan, StreamType.STREAMTYPE_DESKTOP));
            transmitMedia.setChecked(Utils.isTransmitAllowed(user, chan, StreamType.STREAMTYPE_MEDIAFILE));
            transmitChanmsg.setChecked(Utils.isTransmitAllowed(user, chan, StreamType.STREAMTYPE_CHANNELMSG));
        }

        SeekBar.OnSeekBarChangeListener volListener = new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,
                boolean fromUser) {
                if(seekBar == voiceVol) {
                    ttclient.setUserVolume(user.nUserID,
                        StreamType.STREAMTYPE_VOICE, Utils.refVolume(progress));
                }
                else if(seekBar == mediaVol) {
                    ttclient.setUserVolume(user.nUserID,
                        StreamType.STREAMTYPE_MEDIAFILE_AUDIO,
                        Utils.refVolume(progress));
                }
                ttclient.pumpMessage(ClientEvent.CLIENTEVENT_USER_STATECHANGE, user.nUserID);
            }

            @Override
            public void onStartTrackingTouch(SeekBar arg0) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar arg0) {
            }
        };
        voiceVol.setOnSeekBarChangeListener(volListener);
        mediaVol.setOnSeekBarChangeListener(volListener);
        
        OnClickListener defListener = v -> {
            if(v == defVoiceBtn) {
                voiceVol.setProgress(Utils.refVolumeToPercent(SoundLevel.SOUND_VOLUME_DEFAULT));
            }
            else if(v == defMfBtn) {
                mediaVol.setProgress(Utils.refVolumeToPercent(SoundLevel.SOUND_VOLUME_DEFAULT));
            }
        };
        
        defVoiceBtn.setOnClickListener(defListener);
        defMfBtn.setOnClickListener(defListener);

        CompoundButton.OnCheckedChangeListener muteListener = (btn, checked) -> {
            if(btn == voiceMute) {
                ttclient.setUserMute(user.nUserID, StreamType.STREAMTYPE_VOICE, checked);
                ttclient.pumpMessage(ClientEvent.CLIENTEVENT_USER_STATECHANGE, user.nUserID);
            }
            else if(btn == mediaMute) {
                ttclient.setUserMute(user.nUserID, StreamType.STREAMTYPE_MEDIAFILE_AUDIO, checked);
                ttclient.pumpMessage(ClientEvent.CLIENTEVENT_USER_STATECHANGE, user.nUserID);
            }
            else if(btn == subscribeTxtmsg)
                if (checked) {
                    ttclient.doSubscribe(user.nUserID, Subscription.SUBSCRIBE_USER_MSG);
                } else {
                    ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_USER_MSG);
                }
            else if(btn == subscribeChanmsg)
                if (checked) {
                    ttclient.doSubscribe(user.nUserID, Subscription.SUBSCRIBE_CHANNEL_MSG);
                } else {
                    ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_CHANNEL_MSG);
                }
            else if(btn == subscribeBcastmsg)
                if (checked) {
                    ttclient.doSubscribe(user.nUserID, Subscription.SUBSCRIBE_BROADCAST_MSG);
                } else {
                    ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_BROADCAST_MSG);
                }
            else if(btn == subscribeVoice)
                if (checked) {
                    ttclient.doSubscribe(user.nUserID, Subscription.SUBSCRIBE_VOICE);
                } else {
                    ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_VOICE);
                }
            else if(btn == subscribeVid)
                if (checked) {
                    ttclient.doSubscribe(user.nUserID, Subscription.SUBSCRIBE_VIDEOCAPTURE);
                } else {
                    ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_VIDEOCAPTURE);
                }
            else if(btn == subscribeDesk)
                if (checked) {
                    ttclient.doSubscribe(user.nUserID, Subscription.SUBSCRIBE_DESKTOP);
                } else {
                    ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_DESKTOP);
                }
            else if(btn == subscribeMedia)
                if (checked) {
                    ttclient.doSubscribe(user.nUserID, Subscription.SUBSCRIBE_MEDIAFILE);
                } else {
                    ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_MEDIAFILE);
                }
            else if(btn == subscribeIntercepttxtmsg)
                if (checked) {
                    ttclient.doSubscribe(user.nUserID, Subscription.SUBSCRIBE_INTERCEPT_USER_MSG);
                } else {
                    ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_INTERCEPT_USER_MSG);
                }
            else if(btn == subscribeInterceptchanmsg)
                if (checked) {
                    ttclient.doSubscribe(user.nUserID, Subscription.SUBSCRIBE_INTERCEPT_CHANNEL_MSG);
                } else {
                    ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_INTERCEPT_CHANNEL_MSG);
                }
            else if(btn == subscribeInterceptvoice)
                if (checked) {
                    ttclient.doSubscribe(user.nUserID, Subscription.SUBSCRIBE_INTERCEPT_VOICE);
                } else {
                    ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_INTERCEPT_VOICE);
                }
            else if(btn == subscribeInterceptvid)
                if (checked) {
                    ttclient.doSubscribe(user.nUserID, Subscription.SUBSCRIBE_INTERCEPT_VIDEOCAPTURE);
                } else {
                    ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_INTERCEPT_VIDEOCAPTURE);
                }
            else if(btn == subscribeInterceptdesk)
                if (checked) {
                    ttclient.doSubscribe(user.nUserID, Subscription.SUBSCRIBE_INTERCEPT_DESKTOP);
                } else {
                    ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_INTERCEPT_DESKTOP);
                }
            else if(btn == subscribeInterceptmedia)
                if (checked) {
                    ttclient.doSubscribe(user.nUserID, Subscription.SUBSCRIBE_INTERCEPT_MEDIAFILE);
                } else {
                    ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_INTERCEPT_MEDIAFILE);
                }
            else if(btn == transmitVoice) {
                Utils.toggleTransmitUsers(user, chan, StreamType.STREAMTYPE_VOICE, checked);
                ttclient.doUpdateChannel(chan);
            }
            else if(btn == transmitVid) {
                Utils.toggleTransmitUsers(user, chan, StreamType.STREAMTYPE_VIDEOCAPTURE, checked);
                ttclient.doUpdateChannel(chan);
            }
            else if(btn == transmitDesk) {
                Utils.toggleTransmitUsers(user, chan, StreamType.STREAMTYPE_DESKTOP, checked);
                ttclient.doUpdateChannel(chan);
            }
            else if(btn == transmitMedia) {
                Utils.toggleTransmitUsers(user, chan, StreamType.STREAMTYPE_MEDIAFILE, checked);
                ttclient.doUpdateChannel(chan);
            }
            else if(btn == transmitChanmsg) {
                Utils.toggleTransmitUsers(user, chan, StreamType.STREAMTYPE_CHANNELMSG, checked);
                ttclient.doUpdateChannel(chan);
            }
        };
        voiceMute.setOnCheckedChangeListener(muteListener);
        mediaMute.setOnCheckedChangeListener(muteListener);
        subscribeTxtmsg.setOnCheckedChangeListener(muteListener);
        subscribeChanmsg.setOnCheckedChangeListener(muteListener);
        subscribeBcastmsg.setOnCheckedChangeListener(muteListener);
        subscribeVoice.setOnCheckedChangeListener(muteListener);
        subscribeVid.setOnCheckedChangeListener(muteListener);
        subscribeDesk.setOnCheckedChangeListener(muteListener);
        subscribeMedia.setOnCheckedChangeListener(muteListener);
        subscribeIntercepttxtmsg.setOnCheckedChangeListener(muteListener);
        subscribeInterceptchanmsg.setOnCheckedChangeListener(muteListener);
        subscribeInterceptvoice.setOnCheckedChangeListener(muteListener);
        subscribeInterceptvid.setOnCheckedChangeListener(muteListener);
        subscribeInterceptdesk.setOnCheckedChangeListener(muteListener);
        subscribeInterceptmedia.setOnCheckedChangeListener(muteListener);
        transmitVoice.setOnCheckedChangeListener(muteListener);
        transmitVid.setOnCheckedChangeListener(muteListener);
        transmitDesk.setOnCheckedChangeListener(muteListener);
        transmitMedia.setOnCheckedChangeListener(muteListener);
        transmitChanmsg.setOnCheckedChangeListener(muteListener);
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
        ttservice = service;
        ttclient = ttservice.getTTInstance();

        int userid = getIntent().getExtras().getInt(EXTRA_USERID);
        if(!ttclient.getUser(userid, user)) {
            setResult(RESULT_CANCELED);
            finish();
        }
        else
            showUser();
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
    }
}
