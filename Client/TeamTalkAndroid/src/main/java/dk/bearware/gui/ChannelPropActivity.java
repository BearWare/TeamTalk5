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
import dk.bearware.SoundLevel;
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
import android.widget.Spinner;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.EditText;
import android.widget.Toast;
import dk.bearware.AudioCodec;
import dk.bearware.Codec;
import dk.bearware.OpusCodec;
import dk.bearware.OpusConstants;
import dk.bearware.SpeexCodec;
import dk.bearware.SpeexConstants;
import dk.bearware.SpeexVBRCodec;
import dk.bearware.backend.TeamTalkConstants;
import dk.bearware.data.MapAdapter;
import android.widget.AdapterView;
import android.view.View;
import android.widget.CompoundButton;

public class ChannelPropActivity
        extends AppCompatActivity
        implements TeamTalkConnectionListener, ClientEventListener.OnCmdErrorListener, ClientEventListener.OnCmdSuccessListener {

    public static final String TAG = "bearware";

    public static final String EXTRA_CHANNELID = "channelid",   //edit existing channel
            EXTRA_PARENTID = "parentid";     //create new channel

    public static final int REQUEST_AUDIOCODEC = 1,
            REQUEST_AUDIOCONFIG = 2;

    TeamTalkConnection mConnection;
    TeamTalkService ttservice;
    TeamTalkBase ttclient;
    Channel channel;

    EditText chanName;
    EditText chanTopic;
    EditText chanPasswd;
    EditText chanOpPasswd;
    EditText chanMaxUsers;
    EditText chanDiskQuota;
    TextView opusappLabel;
    TextView opussrLabel;
    TextView opusacLabel;
    TextView opusbrLabel;
    TextView opustxintervalLabel;
    TextView opusfsmsecLabel;
    TextView speexsrLabel;
    TextView speexqualityLabel;
    TextView speextxintervalLabel;
    TextView speexvbrsrLabel;
    TextView speexvbrqualityLabel;
    TextView speexvbrtxintervalLabel;
    TextView fixvolLabel;
    TextView opusbitrateText;
    TextView opustxintervalText;
    TextView speextxintervalText;
    TextView speexvbrtxintervalText;
    TextView voicemaxText;
    TextView mediamaxText;
    CheckBox chanPermanent;
    CheckBox chanNoInterrupt;
    CheckBox chanClassroom;
    CheckBox chanOpRecvOnly;
    CheckBox chanNoVoiceAct;
    CheckBox chanNoAudioRec;
    CheckBox chanHidden;
    CheckBox opusdtx;
    CheckBox opusvbr;
    CheckBox speexvbrdtx;
    CheckBox fixvol;
    CheckBox joinexit;
    Spinner codectype;
    Spinner opusapp;
    Spinner opussr;
    Spinner opusaudchan;
    Spinner opusframesize;
    Spinner speexsr;
    Spinner speexvbrsr;
    SeekBar opusbitrate;
    SeekBar opustxinterval;
    SeekBar speexquality;
    SeekBar speextxinterval;
    SeekBar speexvbrquality;
    SeekBar speexvbrtxinterval;
    SeekBar fixvolume;
    SeekBar voicemax;
    SeekBar mediamax;

    AudioCodec audiocodec;

    MapAdapter codecTypeMap;
    MapAdapter opus_appMap;
    MapAdapter opus_srMap;
    MapAdapter opus_audMap;
    MapAdapter opus_fsMap;

    MapAdapter speexvbr_srMap;
    MapAdapter speex_srMap;

    private Boolean isControlsInitialized = false;
    private void ensureControlsInitialized() {
        if(isControlsInitialized) return;
        isControlsInitialized = true;
        chanName = findViewById(R.id.channame);
        chanTopic = findViewById(R.id.chantopic);
        chanPasswd = findViewById(R.id.chanpasswd);
        chanOpPasswd = findViewById(R.id.chanoppasswd);
        chanMaxUsers = findViewById(R.id.chanmaxusers);
        chanDiskQuota = findViewById(R.id.chandiskquota);
        opusappLabel = findViewById(R.id.opus_appLabel);
        opussrLabel = findViewById(R.id.opus_samplerateLabel);
        opusacLabel = findViewById(R.id.opus_audchanLabel);
        opusbrLabel = findViewById(R.id.opus_bitrateLabel);
        opustxintervalLabel = findViewById(R.id.opus_txintervalLabel);
        opusfsmsecLabel = findViewById(R.id.opus_fsmsecLabel);
        speexsrLabel = findViewById(R.id.speex_bandmodeLabel);
        speexqualityLabel = findViewById(R.id.speex_qualityLabel);
        speextxintervalLabel = findViewById(R.id.speex_txintervalLabel);
        speexvbrsrLabel = findViewById(R.id.speexvbr_bandmodeLabel);
        speexvbrqualityLabel = findViewById(R.id.speexvbr_qualityLabel);
        speexvbrtxintervalLabel = findViewById(R.id.speexvbr_txintervalLabel);
        fixvolLabel = findViewById(R.id.fixvolumeLabel);
        opusbitrateText = findViewById(R.id.opus_brTextView);
        opustxintervalText = findViewById(R.id.opus_txintervalTextView);
        speextxintervalText = findViewById(R.id.speex_txintervalTextView);
        speexvbrtxintervalText = findViewById(R.id.speexvbr_txintervalTextView);
        voicemaxText = findViewById(R.id.voice_maxTextView);
        mediamaxText = findViewById(R.id.media_maxTextView);
        chanPermanent = findViewById(R.id.chan_permanent);
        chanNoInterrupt = findViewById(R.id.chan_nointerrupt);
        chanClassroom = findViewById(R.id.chan_classroom);
        chanOpRecvOnly = findViewById(R.id.chan_oprecvonly);
        chanNoVoiceAct = findViewById(R.id.chan_novoiceact);
        chanNoAudioRec = findViewById(R.id.chan_noaudiorecord);
        chanHidden = findViewById(R.id.chan_hidden);
        opusdtx = findViewById(R.id.opus_dtxCheckBox);
        opusvbr = findViewById(R.id.opus_vbrCheckBox);
        speexvbrdtx = findViewById(R.id.speexvbr_dtxCheckBox);
        fixvol = findViewById(R.id.fixvolumeCheckBox);
        joinexit = findViewById(R.id.joinchan_exitCheckBox);
        codectype = findViewById(R.id.codec_typeSpin);
        opusapp = findViewById(R.id.opus_appSpin);
        opussr = findViewById(R.id.opus_samplerateSpin);
        opusaudchan = findViewById(R.id.opus_audchanSpin);
        opusframesize = findViewById(R.id.opus_fsmsecSpin);
        speexsr = findViewById(R.id.speex_bandmodeSpin);
        speexvbrsr = findViewById(R.id.speexvbr_bandmodeSpin);
        opusbitrate = findViewById(R.id.opus_bitrateSeekBar);
        opustxinterval = findViewById(R.id.opus_txintervalSeekBar);
        speexquality = findViewById(R.id.speex_qualitySeekBar);
        speextxinterval = findViewById(R.id.speex_txintervalSeekBar);
        speexvbrquality = findViewById(R.id.speexvbr_qualitySeekBar);
        speexvbrtxinterval = findViewById(R.id.speexvbr_txintervalSeekBar);
        fixvolume = findViewById(R.id.fixvolumeSeekBar);
        voicemax = findViewById(R.id.voice_maxSeekBar);
        mediamax = findViewById(R.id.media_maxSeekBar);

        codecTypeMap = new MapAdapter(this, R.layout.item_spinner, R.id.spinTextView);
        codecTypeMap.addPair("No Audio", 0);
        codecTypeMap.addPair("Speex", 1);
        codecTypeMap.addPair("Speex Variable Bitrate", 2);
        codecTypeMap.addPair("OPUS", 3);
        codectype.setAdapter(codecTypeMap);

        opus_appMap = new MapAdapter(this, R.layout.item_spinner, R.id.spinTextView);
        opus_srMap = new MapAdapter(this, R.layout.item_spinner, R.id.spinTextView);
        opus_audMap = new MapAdapter(this, R.layout.item_spinner, R.id.spinTextView);
        opus_fsMap = new MapAdapter(this, R.layout.item_spinner, R.id.spinTextView);

        opus_appMap.addPair("VoIP", OpusConstants.OPUS_APPLICATION_VOIP);
        opus_appMap.addPair("Music", OpusConstants.OPUS_APPLICATION_AUDIO);

        opus_srMap.addPair("8 KHz", 8000);
        opus_srMap.addPair("12 KHz", 12000);
        opus_srMap.addPair("16 KHz", 16000);
        opus_srMap.addPair("24 KHz", 24000);
        opus_srMap.addPair("48 KHz", 48000);

        opus_audMap.addPair("Mono", 1);
        opus_audMap.addPair("Stereo", 2);

        opus_fsMap.addPair("Default", TeamTalkConstants.OPUS_DEFAULT_FRAMESIZEMSEC);
        opus_fsMap.addPair("2.5 msec", OpusConstants.OPUS_MIN_FRAMESIZE);
        opus_fsMap.addPair("5 msec", 5);
        opus_fsMap.addPair("10 msec", 10);
        opus_fsMap.addPair("20 msec", 20);
        opus_fsMap.addPair("40 msec", 40);
        opus_fsMap.addPair("60 msec", OpusConstants.OPUS_MAX_FRAMESIZE);
        opus_fsMap.addPair("80 msec", 80);
        opus_fsMap.addPair("100 msec", 100);
        opus_fsMap.addPair("120 msec", OpusConstants.OPUS_REALMAX_FRAMESIZE);

        speexvbr_srMap = new MapAdapter(this, R.layout.item_spinner, R.id.spinTextView);
        speexvbr_srMap.addPair("8 KHz", SpeexConstants.SPEEX_BANDMODE_NARROW);
        speexvbr_srMap.addPair("16 KHz", SpeexConstants.SPEEX_BANDMODE_WIDE);
        speexvbr_srMap.addPair("32 KHz", SpeexConstants.SPEEX_BANDMODE_UWIDE);

        speex_srMap = new MapAdapter(this, R.layout.item_spinner, R.id.spinTextView);
        speex_srMap.addPair("8 KHz", SpeexConstants.SPEEX_BANDMODE_NARROW);
        speex_srMap.addPair("16 KHz", SpeexConstants.SPEEX_BANDMODE_WIDE);
        speex_srMap.addPair("32 KHz", SpeexConstants.SPEEX_BANDMODE_UWIDE);

        codectype.setAdapter(codecTypeMap);
        opusapp.setAdapter(opus_appMap);
        opussr.setAdapter(opus_srMap);
        opusaudchan.setAdapter(opus_audMap);
        opusframesize.setAdapter(opus_fsMap);
        speexsr.setAdapter(speex_srMap);
        speexvbrsr.setAdapter(speexvbr_srMap);

        codectype.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                speexsrLabel.setVisibility(position==1 ? View.VISIBLE : View.GONE);
                speexqualityLabel.setVisibility(position==1 ? View.VISIBLE : View.GONE);
                speextxintervalLabel.setVisibility(position==1 ? View.VISIBLE : View.GONE);
                speexsr.setEnabled(position==1); speexsr.setVisibility(position==1 ? View.VISIBLE : View.GONE);
                speexquality.setEnabled(position==1); speexquality.setVisibility(position==1 ? View.VISIBLE : View.GONE);
                speextxintervalText.setVisibility(position==1 ? View.VISIBLE : View.GONE);
                speextxinterval.setEnabled(position==1); speextxinterval.setVisibility(position==1 ? View.VISIBLE : View.GONE);
                speexvbrsrLabel.setVisibility(position==2 ? View.VISIBLE : View.GONE);
                speexvbrqualityLabel.setVisibility(position==2 ? View.VISIBLE : View.GONE);
                speexvbrtxintervalLabel.setVisibility(position==2 ? View.VISIBLE : View.GONE);
                speexvbrdtx.setEnabled(position==2); speexvbrdtx.setVisibility(position==2 ? View.VISIBLE : View.GONE);
                speexvbrsr.setEnabled(position==2); speexvbrsr.setVisibility(position==2 ? View.VISIBLE : View.GONE);
                speexvbrquality.setEnabled(position==2); speexvbrquality.setVisibility(position==2 ? View.VISIBLE : View.GONE);
                speexvbrtxintervalText.setVisibility(position==2 ? View.VISIBLE : View.GONE);
                speexvbrtxinterval.setEnabled(position==2); speexvbrtxinterval.setVisibility(position==2 ? View.VISIBLE : View.GONE);
                opusappLabel.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opussrLabel.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opusacLabel.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opusbrLabel.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opustxintervalLabel.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opusfsmsecLabel.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opusdtx.setEnabled(position==3); opusdtx.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opusvbr.setEnabled(position==3); opusvbr.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opusapp.setEnabled(position==3); opusapp.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opussr.setEnabled(position==3); opussr.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opusaudchan.setEnabled(position==3); opusaudchan.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opusframesize.setEnabled(position==3); opusframesize.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opusbitrateText.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opusbitrate.setEnabled(position==3); opusbitrate.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opustxintervalText.setVisibility(position==3 ? View.VISIBLE : View.GONE);
                opustxinterval.setEnabled(position==3); opustxinterval.setVisibility(position==3 ? View.VISIBLE : View.GONE);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });
        
        fixvol.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
fixvolLabel.setVisibility(isChecked ? View.VISIBLE : View.GONE);
                fixvolume.setEnabled(isChecked); fixvolume.setVisibility(isChecked ? View.VISIBLE : View.GONE);
            }
        });

        int maxbr = OpusConstants.OPUS_MAX_BITRATE - OpusConstants.OPUS_MIN_BITRATE;
        opusbitrate.setMax(maxbr / 1000);
        opusbitrate.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                int br = progress + (OpusConstants.OPUS_MIN_BITRATE / 1000);
                opusbitrateText.setText(br + " kbit/s");
            }
        });

        int maxtxinterval = TeamTalkConstants.OPUS_MAX_TXINTERVALMSEC - TeamTalkConstants.OPUS_MIN_TXINTERVALMSEC;
        opustxinterval.setMax(maxtxinterval);

        opustxinterval.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                int interval = progress + TeamTalkConstants.OPUS_MIN_TXINTERVALMSEC;
                opustxintervalText.setText(interval + " msec");

                int selFramesize = opus_fsMap.getValue(opusframesize.getSelectedItemPosition(), TeamTalkConstants.OPUS_DEFAULT_FRAMESIZEMSEC);
                if (interval > OpusConstants.OPUS_REALMAX_FRAMESIZE && selFramesize == TeamTalkConstants.OPUS_DEFAULT_FRAMESIZEMSEC)
                opusframesize.setSelection(opus_fsMap.getIndex(OpusConstants.OPUS_REALMAX_FRAMESIZE, 0));
                else if (selFramesize > interval)
                opusframesize.setSelection(opus_fsMap.getIndex(TeamTalkConstants.OPUS_DEFAULT_FRAMESIZEMSEC, 0));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        speexquality.setMax(SpeexConstants.SPEEX_QUALITY_MAX - SpeexConstants.SPEEX_QUALITY_MIN);

        int speexmaxtxinterval = TeamTalkConstants.SPEEX_MAX_TXINTERVALMSEC - TeamTalkConstants.OPUS_MIN_TXINTERVALMSEC;
        speextxinterval.setMax(speexmaxtxinterval);

        speextxinterval.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                int interval = progress + TeamTalkConstants.SPEEX_MIN_TXINTERVALMSEC;
                speextxintervalText.setText(interval + " msec");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        speexvbrquality.setMax(SpeexConstants.SPEEX_QUALITY_MAX - SpeexConstants.SPEEX_QUALITY_MIN);

        int speexvbrmaxtxinterval = TeamTalkConstants.SPEEX_MAX_TXINTERVALMSEC - TeamTalkConstants.OPUS_MIN_TXINTERVALMSEC;
        speexvbrtxinterval.setMax(speexvbrmaxtxinterval);
        speexvbrtxinterval.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                int interval = progress + TeamTalkConstants.SPEEX_MIN_TXINTERVALMSEC;
                speexvbrtxintervalText.setText(interval + " msec");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        fixvolume.setMax((SoundLevel.SOUND_GAIN_MAX - SoundLevel.SOUND_GAIN_MIN) / 1000);
        
        voicemax.setMax(100);
        mediamax.setMax(100);
        voicemax.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                voicemaxText.setText(progress + " " + getString(R.string.text_timeseconds));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        mediamax.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mediamaxText.setText(progress + " " + getString(R.string.text_timeseconds));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });
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
        setContentView(R.layout.activity_channel_prop);
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

                    updateCmdId = ttclient.doUpdateChannel(channel);
                    if(updateCmdId < 0) {
                        Toast.makeText(this, getResources().getString(R.string.text_con_cmderr),
                                Toast.LENGTH_LONG).show();
                    }
                }
                else {
                    exchangeChannel(true);

                    if(joinexit.isChecked()) {
                        updateCmdId = ttclient.doJoinChannel(channel);
                        if(updateCmdId > 0)
                            ttservice.setJoinChannel(channel);
                        else {
                            Toast.makeText(this, getResources().getString(R.string.text_con_cmderr),
                                    Toast.LENGTH_LONG).show();
                        }
                    } else {
                        updateCmdId = ttclient.doMakeChannel(channel);
                        if(updateCmdId ==-1)
                            Toast.makeText(this, getResources().getString(R.string.text_con_cmderr), Toast.LENGTH_LONG).show();
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

        // Unbind from the service
        if(mConnection.isBound()) {
            onServiceDisconnected(ttservice);
            unbindService(mConnection);
            mConnection.setBound(false);
        }
    }

    void exchangeChannel(boolean store) {
        ensureControlsInitialized();

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

            audiocodec.opus.nApplication = opus_appMap.getValue(opusapp.getSelectedItemPosition(), OpusConstants.DEFAULT_OPUS_APPLICATION);
            audiocodec.opus.nSampleRate = opus_srMap.getValue(opussr.getSelectedItemPosition(), OpusConstants.DEFAULT_OPUS_SAMPLERATE);
            audiocodec.opus.nChannels = opus_audMap.getValue(opusaudchan.getSelectedItemPosition(),OpusConstants.DEFAULT_OPUS_CHANNELS);
            audiocodec.opus.nComplexity = OpusConstants.DEFAULT_OPUS_COMPLEXITY;
            audiocodec.opus.bFEC = OpusConstants.DEFAULT_OPUS_FEC;
            audiocodec.opus.bDTX = opusdtx.isChecked();
            audiocodec.opus.bVBR = opusvbr.isChecked();
            audiocodec.opus.bVBRConstraint = OpusConstants.DEFAULT_OPUS_VBRCONSTRAINT;
            audiocodec.opus.nBitRate = opusbitrate.getProgress() * 1000 + OpusConstants.OPUS_MIN_BITRATE;
            audiocodec.opus.nTxIntervalMSec = opustxinterval.getProgress() + TeamTalkConstants.OPUS_MIN_TXINTERVALMSEC;
            audiocodec.opus.nFrameSizeMSec = opus_fsMap.getValue(opusframesize.getSelectedItemPosition(), TeamTalkConstants.OPUS_DEFAULT_FRAMESIZEMSEC);

            audiocodec.speex.nBandmode = speex_srMap.getValue(speexsr.getSelectedItemPosition(), SpeexConstants.DEFAULT_SPEEX_BANDMODE);
            audiocodec.speex.nQuality = speexquality.getProgress() + SpeexConstants.SPEEX_QUALITY_MIN;
            audiocodec.speex.nTxIntervalMSec = speextxinterval.getProgress() + TeamTalkConstants.SPEEX_MIN_TXINTERVALMSEC;

            audiocodec.speex_vbr.nBandmode = speexvbr_srMap.getValue(speexvbrsr.getSelectedItemPosition(), SpeexConstants.DEFAULT_SPEEX_BANDMODE);
            audiocodec.speex_vbr.nQuality = speexvbrquality.getProgress() + SpeexConstants.SPEEX_QUALITY_MIN;
            audiocodec.speex_vbr.bDTX = speexvbrdtx.isChecked();
            audiocodec.speex_vbr.nTxIntervalMSec = speexvbrtxinterval.getProgress() + TeamTalkConstants.SPEEX_MIN_TXINTERVALMSEC;

            audiocodec.nCodec = codecTypeMap.getValue(codectype.getSelectedItemPosition(), 3);

            channel.audiocodec = audiocodec;
            
            channel.audiocfg.bEnableAGC = fixvol.isChecked();
            channel.audiocfg.nGainLevel = (fixvolume.getProgress() * 1000) + SoundLevel.SOUND_GAIN_MIN;
            channel.nTimeOutTimerVoiceMSec = voicemax.getProgress() * 1000;
            channel.nTimeOutTimerMediaFileMSec = mediamax.getProgress() * 1000;
        }
        else {
            audiocodec = channel.audiocodec;
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
            opusapp.setSelection(opus_appMap.getIndex(audiocodec.opus.nApplication, 0));
            opussr.setSelection(opus_srMap.getIndex(audiocodec.opus.nSampleRate, 0));
            opusaudchan.setSelection(opus_audMap.getIndex(audiocodec.opus.nChannels, 0));
            opusframesize.setSelection(opus_fsMap.getIndex(audiocodec.opus.nFrameSizeMSec, 0));
            speexsr.setSelection(speex_srMap.getIndex(audiocodec.speex.nBandmode, 1));
            speexvbrsr.setSelection(speexvbr_srMap.getIndex(audiocodec.speex_vbr.nBandmode, 1));

            opusdtx.setChecked(audiocodec.opus.bDTX);
            opusvbr.setChecked(audiocodec.opus.bVBR);

            opusbitrate.setProgress((audiocodec.opus.nBitRate / 1000) - (OpusConstants.OPUS_MIN_BITRATE / 1000));
            opustxinterval.setProgress(audiocodec.opus.nTxIntervalMSec - TeamTalkConstants.OPUS_MIN_TXINTERVALMSEC);
            speexquality.setProgress(audiocodec.speex.nQuality + SpeexConstants.SPEEX_QUALITY_MIN);
            speextxinterval.setProgress(audiocodec.speex.nTxIntervalMSec - TeamTalkConstants.SPEEX_MIN_TXINTERVALMSEC);
            speexvbrquality.setProgress(audiocodec.speex_vbr.nQuality - SpeexConstants.SPEEX_QUALITY_MIN);

            speexvbrdtx.setChecked(audiocodec.speex_vbr.bDTX);

            speexvbrtxinterval.setProgress(audiocodec.speex_vbr.nTxIntervalMSec - TeamTalkConstants.SPEEX_MIN_TXINTERVALMSEC);

            codectype.setSelection(codecTypeMap.getIndex(audiocodec.nCodec, 3));

            fixvol.setChecked(channel.audiocfg.bEnableAGC);
            fixvolume.setProgress((channel.audiocfg.nGainLevel - SoundLevel.SOUND_GAIN_MIN)/1000);
fixvolLabel.setVisibility(channel.audiocfg.bEnableAGC ? View.VISIBLE : View.GONE);
            fixvolume.setEnabled(channel.audiocfg.bEnableAGC); fixvolume.setVisibility(channel.audiocfg.bEnableAGC ? View.VISIBLE : View.GONE);
            voicemax.setProgress(channel.nTimeOutTimerVoiceMSec / 1000);
            voicemaxText.setText(channel.nTimeOutTimerVoiceMSec / 1000 + " " + getString(R.string.text_timeseconds));
            mediamax.setProgress(channel.nTimeOutTimerMediaFileMSec / 1000);
            mediamaxText.setText(channel.nTimeOutTimerMediaFileMSec / 1000 + " " + getString(R.string.text_timeseconds));
        }
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
        ttservice = service;
        ttclient = ttservice.getTTInstance();

        service.getEventHandler().registerOnCmdError(this, true);
        service.getEventHandler().registerOnCmdSuccess(this, true);

        if (channel == null) {
            int channelid = getIntent().getExtras().getInt(EXTRA_CHANNELID);
            int parentid = getIntent().getExtras().getInt(EXTRA_PARENTID);
            if(channelid > 0) {
                //existing channel
                channel = ttservice.getChannels().get(channelid);
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
                if(ttservice.getTTInstance().getServerProperties(prop)) {
                    channel.nMaxUsers = prop.nMaxUsers;
                }
            }
        }
        audiocodec = channel.audiocodec;

        exchangeChannel(false);

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
