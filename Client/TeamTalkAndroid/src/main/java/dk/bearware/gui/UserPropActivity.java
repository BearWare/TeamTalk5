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

import dk.bearware.StreamType;
import dk.bearware.Subscription;
import dk.bearware.TeamTalkBase;
import dk.bearware.User;
import dk.bearware.SoundLevel;
import dk.bearware.UserState;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkService;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;

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
        TextView nickname = (TextView) findViewById(R.id.user_nickname);
        TextView username = (TextView) findViewById(R.id.user_username);
        TextView clientname = (TextView) findViewById(R.id.user_clientname);
        final SeekBar voiceVol = (SeekBar) findViewById(R.id.user_vol_voiceSeekBar);
        final Button defVoiceBtn = (Button) findViewById(R.id.defVoiceVolBtn);
        final Switch voiceMute = (Switch) findViewById(R.id.user_mutevoiceSwitch);
        final SeekBar mediaVol = (SeekBar) findViewById(R.id.user_vol_mediaSeekBar);
        final Button defMfBtn = (Button) findViewById(R.id.defMfVolBtn);
        final Switch mediaMute = (Switch) findViewById(R.id.user_mutemediaSwitch);
        final Switch subscribeTxtmsg = (Switch) findViewById(R.id.user_subscribetxtmsgSwitch);
        final Switch subscribeChanmsg = (Switch) findViewById(R.id.user_subscribechanmsgSwitch);
        final Switch subscribeBcastmsg = (Switch) findViewById(R.id.user_subscribebcastmsgSwitch);
        final Switch subscribeVoice = (Switch) findViewById(R.id.user_subscribevoiceSwitch);
        final Switch subscribeVid = (Switch) findViewById(R.id.user_subscribevidSwitch);
        final Switch subscribeDesk = (Switch) findViewById(R.id.user_subscribedeskSwitch);
        final Switch subscribeMedia = (Switch) findViewById(R.id.user_subscribemediaSwitch);

        nickname.setText(user.szNickname);
        username.setText(user.szUsername);
        clientname.setText(user.szClientName);
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
        
        OnClickListener defListener = new OnClickListener() {
            @Override
            public void onClick(View v) {
                if(v == defVoiceBtn) {
                    voiceVol.setProgress(Utils.refVolumeToPercent(SoundLevel.SOUND_VOLUME_DEFAULT));
                }
                else if(v == defMfBtn) {
                    mediaVol.setProgress(Utils.refVolumeToPercent(SoundLevel.SOUND_VOLUME_DEFAULT));
                }
            }
        };
        
        defVoiceBtn.setOnClickListener(defListener);
        defMfBtn.setOnClickListener(defListener);

        CompoundButton.OnCheckedChangeListener muteListener = new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton btn, boolean checked) {
                if(btn == voiceMute)
                    ttclient.setUserMute(user.nUserID, StreamType.STREAMTYPE_VOICE, checked);
                else if(btn == mediaMute)
                    ttclient.setUserMute(user.nUserID, StreamType.STREAMTYPE_MEDIAFILE_AUDIO, checked);
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
