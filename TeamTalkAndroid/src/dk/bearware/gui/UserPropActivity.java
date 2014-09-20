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

import dk.bearware.StreamType;
import dk.bearware.TeamTalkBase;
import dk.bearware.User;
import dk.bearware.SoundLevel;
import dk.bearware.UserState;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkService;
import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.view.Menu;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;

public class UserPropActivity extends Activity implements TeamTalkConnectionListener {

    public final static String EXTRA_USERID = "userid";

    public static final String TAG = "bearware";
    
    TeamTalkConnection mConnection;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_user_prop);

        mConnection = new TeamTalkConnection(this);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.user_prop, menu);
        return true;
    }

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

    TeamTalkService ttservice;
    TeamTalkBase ttclient;
    User user = new User();

    void showUser() {
        TextView nickname = (TextView) findViewById(R.id.user_nickname);
        TextView username = (TextView) findViewById(R.id.user_username);
        final SeekBar voiceVol = (SeekBar) findViewById(R.id.user_vol_voiceSeekBar);
        final Switch voiceMute = (Switch) findViewById(R.id.user_mutevoiceSwitch);
        final SeekBar mediaVol = (SeekBar) findViewById(R.id.user_vol_mediaSeekBar);
        final Switch mediaMute = (Switch) findViewById(R.id.user_mutemediaSwitch);

        nickname.setText(user.szNickname);
        username.setText(user.szUsername);
        voiceVol.setMax(SoundLevel.SOUND_VOLUME_MAX);
        voiceVol.setProgress(user.nVolumeVoice);
        mediaVol.setMax(SoundLevel.SOUND_VOLUME_MAX);
        mediaVol.setProgress(user.nVolumeMediaFile);
        voiceMute.setChecked((user.uUserState & UserState.USERSTATE_MUTE_VOICE) != 0);
        mediaMute.setChecked((user.uUserState & UserState.USERSTATE_MUTE_MEDIAFILE) != 0);

        SeekBar.OnSeekBarChangeListener volListener = new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if(fromUser) {
                    if(seekBar == voiceVol)
                        ttclient.setUserVolume(user.nUserID, StreamType.STREAMTYPE_VOICE, progress);
                    else if(seekBar == mediaVol)
                        ttclient.setUserVolume(user.nUserID, StreamType.STREAMTYPE_MEDIAFILE_AUDIO, progress);
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

        CompoundButton.OnCheckedChangeListener muteListener = new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton btn, boolean checked) {
                if(btn == voiceMute)
                    ttclient.setUserMute(user.nUserID, StreamType.STREAMTYPE_VOICE, checked);
                else if(btn == mediaMute)
                    ttclient.setUserMute(user.nUserID, StreamType.STREAMTYPE_MEDIAFILE_AUDIO, checked);
            }
        };
        voiceMute.setOnCheckedChangeListener(muteListener);
        mediaMute.setOnCheckedChangeListener(muteListener);
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
        ttservice = service;
        ttclient = ttservice.getTTInstance();

        int userid = UserPropActivity.this.getIntent().getExtras().getInt(EXTRA_USERID);
        if(!ttclient.getUser(userid, user)) {
            UserPropActivity.this.setResult(RESULT_CANCELED);
            finish();
        }
        else
            showUser();
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
        // TODO Auto-generated method stub

    }
}
