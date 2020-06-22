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

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.Locale;
import java.util.List;
import java.util.Vector;

import dk.bearware.BannedUser;
import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.ClientFlag;
import dk.bearware.ClientStatistics;
import dk.bearware.DesktopInput;
import dk.bearware.FileTransfer;
import dk.bearware.MediaFileInfo;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.SoundDeviceConstants;
import dk.bearware.SoundLevel;
import dk.bearware.TeamTalkBase;
import dk.bearware.TextMessage;
import dk.bearware.TextMsgType;
import dk.bearware.User;
import dk.bearware.UserAccount;
import dk.bearware.UserRight;
import dk.bearware.UserState;
import dk.bearware.data.Permissions;
import dk.bearware.events.ClientListener;
import dk.bearware.events.CommandListener;
import dk.bearware.events.ConnectionListener;
import dk.bearware.events.UserListener;

import dk.bearware.backend.OnVoiceTransmissionToggleListener;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkConstants;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.data.FileListAdapter;
import dk.bearware.data.MediaAdapter;
import dk.bearware.data.Preferences;
import dk.bearware.data.ServerEntry;
import dk.bearware.data.TextMessageAdapter;
import dk.bearware.data.TTSWrapper;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.SoundPool;
import android.os.Build;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.Handler;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.Vibrator;
import android.preference.PreferenceManager;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.app.ListFragment;
import android.support.v4.view.ViewPager;
import android.text.method.SingleLineTransformationMethod;
import android.util.Log;
import android.util.SparseArray;
import android.util.SparseIntArray;
import android.view.inputmethod.InputMethodManager;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ExpandableListView;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.PopupMenu.OnMenuItemClickListener;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity
extends FragmentActivity
implements TeamTalkConnectionListener, 
        ConnectionListener, 
        CommandListener, 
        UserListener, 
        ClientListener,
        OnItemClickListener, 
        OnItemLongClickListener, 
        OnMenuItemClickListener, 
        SensorEventListener, 
        OnVoiceTransmissionToggleListener {

    /**
     * The {@link android.support.v4.view.PagerAdapter} that will provide fragments for each of the sections. We use a
     * {@link android.support.v4.app.FragmentPagerAdapter} derivative, which will keep every loaded fragment in memory.
     * If this becomes too memory intensive, it may be best to switch to a
     * {@link android.support.v4.app.FragmentStatePagerAdapter}.
     */
    SectionsPagerAdapter mSectionsPagerAdapter;

    /**
     * The {@link ViewPager} that will host the section contents.
     */
    ViewPager mViewPager;
    
    public static final String TAG = "bearware";

    public final int REQUEST_EDITCHANNEL = 1,
                     REQUEST_NEWCHANNEL = 2,
                     REQUEST_EDITUSER = 3,
                     REQUEST_SELECT_FILE = 4;

    // The channel currently being displayed
    Channel curchannel;
    // The channel we're currenlty in
    Channel mychannel;

    SparseArray<CmdComplete> activecmds = new SparseArray<CmdComplete>();

    ChannelListAdapter channelsAdapter;
    FileListAdapter filesAdapter;
    TextMessageAdapter textmsgAdapter;
    MediaAdapter mediaAdapter;
    TTSWrapper ttsWrapper = null;
    AccessibilityAssistant accessibilityAssistant;
    AudioManager audioManager;
    SoundPool audioIcons;
    ComponentName mediaButtonEventReceiver;
    NotificationManager notificationManager;
    WakeLock wakeLock;
    boolean restarting;
    SensorManager mSensorManager;
    Sensor mSensor;

    static final String MESSAGE_NOTIFICATION_TAG = "incoming_message";

    final int SOUND_VOICETXON = 1,
              SOUND_VOICETXOFF = 2,
              SOUND_USERMSG = 3,
              SOUND_CHANMSG = 4,
              SOUND_BCASTMSG = 5,
              SOUND_SERVERLOST = 6,
              SOUND_FILESUPDATE = 7,
              SOUND_VOXON = 8,
              SOUND_VOXOFF = 9,
              SOUND_TXREADY = 10,
              SOUND_TXSTOP = 11,
              SOUND_USERJOIN = 12,
              SOUND_USERLEFT = 13;
    
    SparseIntArray sounds = new SparseIntArray();

    public ChannelListAdapter getChannelsAdapter() {
        return channelsAdapter;
    }
    
    public FileListAdapter getFilesAdapter() {
        return filesAdapter;
    }

    public TextMessageAdapter getTextMessagesAdapter() {
        return textmsgAdapter;
    }
    
    public MediaAdapter getMediaAdapter() {
        return mediaAdapter;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        String serverName = getIntent().getStringExtra(ServerEntry.KEY_SERVERNAME);
        if ((serverName != null) && !serverName.isEmpty())
            setTitle(serverName);
        getActionBar().setDisplayHomeAsUpEnabled(true);

        mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);
        restarting = (savedInstanceState != null);
        accessibilityAssistant = new AccessibilityAssistant(this);
        audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        mediaButtonEventReceiver = new ComponentName(getPackageName(), MediaButtonEventReceiver.class.getName());
        notificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        wakeLock = ((PowerManager)getSystemService(Context.POWER_SERVICE)).newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
        wakeLock.setReferenceCounted(false);

        channelsAdapter = new ChannelListAdapter(this.getBaseContext());
        filesAdapter = new FileListAdapter(this, this, accessibilityAssistant);
        textmsgAdapter = new TextMessageAdapter(this.getBaseContext(), accessibilityAssistant);
        mediaAdapter = new MediaAdapter(this.getBaseContext());
        
        // Create the adapter that will return a fragment for each of the five
        // primary sections of the app.
        mSectionsPagerAdapter = new SectionsPagerAdapter(getSupportFragmentManager());

        // Set up the ViewPager with the sections adapter.
        mViewPager = (ViewPager) findViewById(R.id.pager);
        mViewPager.setAdapter(mSectionsPagerAdapter);
        mViewPager.setOnPageChangeListener(mSectionsPagerAdapter);

        setupButtons();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            final MediaPlayer mMediaPlayer;
            mMediaPlayer = MediaPlayer.create(getApplicationContext(), R.raw.silence);
            mMediaPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
                @Override
                public void onCompletion(MediaPlayer mediaPlayer) {
                    mMediaPlayer.release();
                }
            });
            mMediaPlayer.start();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        boolean isEditable = curchannel != null;
        boolean isJoinable = (ttclient != null) && (curchannel != null) && (ttclient.getMyChannelID() != curchannel.nChannelID) && (curchannel.nMaxUsers > 0);
        boolean isMyChannel = (ttclient != null) && (curchannel != null) && (ttclient.getMyChannelID() == curchannel.nChannelID);
        menu.findItem(R.id.action_edit).setEnabled(isEditable).setVisible(isEditable);
        menu.findItem(R.id.action_join).setEnabled(isJoinable).setVisible(isJoinable);
        menu.findItem(R.id.action_upload).setEnabled(isMyChannel).setVisible(isMyChannel);
        menu.findItem(R.id.action_stream).setEnabled(isMyChannel).setVisible(isMyChannel);
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
            case R.id.action_join : {
                if (curchannel != null)
                    joinChannel(curchannel);
            }
            break;
            case R.id.action_upload : {
                if (Permissions.setupPermission(getBaseContext(), this, Permissions.MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE)) {
                    startActivityForResult(new Intent(this, FilePickerActivity.class), REQUEST_SELECT_FILE);
                }
            }
            break;
            case R.id.action_stream : {
                int flags = ttclient.getFlags();
                if ((flags & ClientFlag.CLIENT_STREAM_AUDIO) == ClientFlag.CLIENT_STREAM_AUDIO || (flags & ClientFlag.CLIENT_STREAM_VIDEO) == ClientFlag.CLIENT_STREAM_VIDEO) {
                    ttclient.stopStreamingMediaFileToChannel();
                } else {
                    Intent intent = new Intent(MainActivity.this, StreamMediaActivity.class);
                    startActivity(intent);
                }
            }
            break;
            case R.id.action_edit : {
                if (curchannel != null)
                    editChannelProperties(curchannel);
            }
            break;

            case R.id.action_newchannel : {
                Intent intent = new Intent(MainActivity.this, ChannelPropActivity.class);
                
                int parent_chan_id = ttclient.getRootChannelID();
                if(curchannel != null)
                    parent_chan_id = curchannel.nChannelID;
                intent = intent.putExtra(ChannelPropActivity.EXTRA_PARENTID, parent_chan_id);
                
                startActivityForResult(intent, REQUEST_NEWCHANNEL);
            }
            break;
            case R.id.action_settings : {
                Intent intent = new Intent(MainActivity.this, PreferencesActivity.class);
                startActivity(intent);
                break;
            }
            case android.R.id.home : {
                if (filesAdapter.getActiveTransfersCount() > 0) {
                    AlertDialog.Builder alert = new AlertDialog.Builder(this);
                    alert.setMessage(R.string.disconnect_alert);
                    alert.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int whichButton) {
                                filesAdapter.cancelAllTransfers();
                                finish();
                            }
                        });
                    alert.setNegativeButton(android.R.string.cancel, null);
                    alert.show();
                }
                else {
                    finish();
                }
                break;
            }
            default :
                return super.onOptionsItemSelected(item);
        }
        return true;
    }

    CountDownTimer stats_timer = null;
    
    TeamTalkConnection mConnection;
    TeamTalkService ttservice;
    TeamTalkBase ttclient;

    @Override
    protected void onStart() {
        super.onStart();

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        if (ttsWrapper == null)
            ttsWrapper = new TTSWrapper(this, prefs.getString("pref_speech_engine", TTSWrapper.defaultEngineName));
        if (mConnection == null)
            mConnection = new TeamTalkConnection(this);

        if (!mConnection.isBound()) {
            // Bind to LocalService
            Intent intent = new Intent(getApplicationContext(), TeamTalkService.class);
            Log.d(TAG, "Binding TeamTalk service");
            if(!bindService(intent, mConnection, Context.BIND_AUTO_CREATE))
                Log.e(TAG, "Failed to bind to TeamTalk service");
        }
        else {

            int mastervol = prefs.getInt(Preferences.PREF_SOUNDSYSTEM_MASTERVOLUME, SoundLevel.SOUND_VOLUME_DEFAULT);
            int gain = prefs.getInt(Preferences.PREF_SOUNDSYSTEM_MICROPHONEGAIN, SoundLevel.SOUND_GAIN_DEFAULT);
            int voxlevel = prefs.getInt(Preferences.PREF_SOUNDSYSTEM_VOICEACTIVATION_LEVEL, 5);
            boolean voxState = ttservice.isVoiceActivationEnabled();
            boolean txState = ttservice.isVoiceTransmitting();

            // only set volume and gain if tt-instance hasn't already been configured
            if (ttclient.getSoundOutputVolume() != mastervol)
                ttclient.setSoundOutputVolume(mastervol);
            if (ttclient.getSoundInputGainLevel() != gain)
                ttclient.setSoundInputGainLevel(gain);
            if (ttclient.getVoiceActivationLevel() != voxlevel)
                ttclient.setVoiceActivationLevel(voxlevel);

            adjustMuteButton((ImageButton) findViewById(R.id.speakerBtn));
            adjustVoxState(voxState, voxState ? voxlevel : gain);
            adjustTxState(txState);

            TextView volLevel = (TextView) findViewById(R.id.vollevel_text);
            volLevel.setText(Utils.refVolumeToPercent(mastervol) + "%");
            volLevel.setContentDescription(getString(R.string.speaker_volume_description, volLevel.getText()));
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        mSensorManager.registerListener(this, mSensor, SensorManager.SENSOR_DELAY_NORMAL);

        if (audioIcons != null)
            audioIcons.release();
        sounds.clear();

        audioIcons = new SoundPool(1, AudioManager.STREAM_MUSIC, 0);

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());

        audioManager.setMode(prefs.getBoolean(Preferences.PREF_SOUNDSYSTEM_VOICEPROCESSING, false)?
                AudioManager.MODE_IN_COMMUNICATION : AudioManager.MODE_NORMAL);
        audioManager.setSpeakerphoneOn(prefs.getBoolean(Preferences.PREF_SOUNDSYSTEM_SPEAKERPHONE, false));

        if (prefs.getBoolean("server_lost_audio_icon", true)) {
            sounds.put(SOUND_SERVERLOST, audioIcons.load(getApplicationContext(), R.raw.serverlost, 1));
        }
        if (prefs.getBoolean("rx_tx_audio_icon", true)) {
            sounds.put(SOUND_VOICETXON, audioIcons.load(getApplicationContext(), R.raw.on, 1));
            sounds.put(SOUND_VOICETXOFF, audioIcons.load(getApplicationContext(), R.raw.off, 1));
        }
        if (prefs.getBoolean("private_message_audio_icon", true)) {
            sounds.put(SOUND_USERMSG, audioIcons.load(getApplicationContext(), R.raw.user_message, 1));
        }
        if (prefs.getBoolean("channel_message_audio_icon", true)) {
            sounds.put(SOUND_CHANMSG, audioIcons.load(getApplicationContext(), R.raw.channel_message, 1));
        }
        if (prefs.getBoolean("broadcast_message_audio_icon", true)) {
            sounds.put(SOUND_BCASTMSG, audioIcons.load(getApplicationContext(), R.raw.broadcast_message, 1));
        }
        if (prefs.getBoolean("files_updated_audio_icon", true)) {
            sounds.put(SOUND_FILESUPDATE, audioIcons.load(getApplicationContext(), R.raw.fileupdate, 1));
        }
        if (prefs.getBoolean("voiceact_triggered_icon", true)) {
            sounds.put(SOUND_VOXON, audioIcons.load(getApplicationContext(), R.raw.voiceact_on, 1));
            sounds.put(SOUND_VOXOFF, audioIcons.load(getApplicationContext(), R.raw.voiceact_off, 1));
        }
        if (prefs.getBoolean("transmitready_icon", true)) {
            sounds.put(SOUND_TXREADY, audioIcons.load(getApplicationContext(), R.raw.txqueue_start, 1));
            sounds.put(SOUND_TXSTOP, audioIcons.load(getApplicationContext(), R.raw.txqueue_stop, 1));
        }
        if (prefs.getBoolean("userjoin_icon", true)) {
            sounds.put(SOUND_USERJOIN, audioIcons.load(getApplicationContext(), R.raw.user_join, 1));
        }
        if (prefs.getBoolean("userleft_icon", true)) {
            sounds.put(SOUND_USERLEFT, audioIcons.load(getApplicationContext(), R.raw.user_left, 1));
        }

        getTextMessagesAdapter().showLogMessages(prefs.getBoolean("show_log_messages", true));

        getWindow().getDecorView().setKeepScreenOn(prefs.getBoolean("keep_screen_on_checkbox", false));

        createStatusTimer();
        ttsWrapper.switchEngine(prefs.getString("pref_speech_engine", TTSWrapper.defaultEngineName));
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (stats_timer != null) {
            stats_timer.cancel();
            stats_timer = null;
        }
    }

    @Override
    protected void onStop() {
        super.onStop();

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());

        if (mConnection.isBound()) {
            SharedPreferences.Editor editor = prefs.edit();
            editor.putInt(Preferences.PREF_SOUNDSYSTEM_MASTERVOLUME, ttclient.getSoundOutputVolume());
            editor.putInt(Preferences.PREF_SOUNDSYSTEM_MICROPHONEGAIN, ttclient.getSoundInputGainLevel());
            editor.putInt(Preferences.PREF_SOUNDSYSTEM_VOICEACTIVATION_LEVEL, ttclient.getVoiceActivationLevel());
            editor.apply();
        }

        // Cleanup resources
        if (isFinishing()) {
            if (audioIcons != null) {
                audioIcons.release();
                audioIcons = null;
            }
            if (ttsWrapper != null) {
                ttsWrapper.shutdown();
                ttsWrapper = null;
            }

            // Unbind from the service
            if (mConnection.isBound()) {
                Log.d(TAG, "Unbinding TeamTalk service");
                onServiceDisconnected(ttservice);
                ttservice.disablePhoneCallReaction();
                ttservice.resetState();
                unbindService(mConnection);
                mConnection.setBound(false);
            }
            notificationManager.cancelAll();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        mSensorManager.unregisterListener(this);
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
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if ((requestCode == REQUEST_SELECT_FILE) && (resultCode == RESULT_OK)) {
            String path = data.getStringExtra(FilePickerActivity.SELECTED_FILE);
            String remoteName = filesAdapter.getRemoteName(path);
            if (remoteName != null) {
                Toast.makeText(this, getString(R.string.remote_file_exists, remoteName), Toast.LENGTH_LONG).show();
            } else if (ttclient.doSendFile(curchannel.nChannelID, path) <= 0) {
                Toast.makeText(this, getString(R.string.upload_failed, path), Toast.LENGTH_LONG).show();
            }
            else {
                Toast.makeText(this, R.string.upload_started, Toast.LENGTH_SHORT).show();
            }
        }
    }

    public void onAccuracyChanged(Sensor sensor, int accuracy) {
    }

    public void onSensorChanged(SensorEvent event) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        boolean proximity_sensor = prefs.getBoolean("proximity_sensor_checkbox", false);
        if (proximity_sensor && (mConnection != null) && mConnection.isBound() && !ttservice.isInPhoneCall()) {
            if (event.values[0] == 0) {
                audioManager.setMode(AudioManager.MODE_IN_COMMUNICATION);
                audioManager.setSpeakerphoneOn(false);
                ttservice.enableVoiceTransmission(true);
            } else {
                audioManager.setMode(prefs.getBoolean(Preferences.PREF_SOUNDSYSTEM_VOICEPROCESSING, false)?
                        AudioManager.MODE_IN_COMMUNICATION : AudioManager.MODE_NORMAL);
                audioManager.setSpeakerphoneOn(prefs.getBoolean(Preferences.PREF_SOUNDSYSTEM_SPEAKERPHONE, false));
                if (ttservice.isVoiceTransmissionEnabled())
                    ttservice.enableVoiceTransmission(false);
            }
        }
    }

    ChannelsSectionFragment channelsFragment;
    ChatSectionFragment chatFragment;
    VidcapSectionFragment vidcapFragment;
    MediaSectionFragment mediaFragment;
    FilesSectionFragment filesFragment;

    /**
     * A {@link FragmentPagerAdapter} that returns a fragment corresponding to one of the sections/tabs/pages.
     */
    public class SectionsPagerAdapter extends FragmentPagerAdapter implements ViewPager.OnPageChangeListener {
        
        public static final int CHANNELS_PAGE   = 0,
                                CHAT_PAGE       = 1,
                                MEDIA_PAGE      = 2,
                                FILES_PAGE      = 3,
                                
                                PAGE_COUNT      = 4;

        public SectionsPagerAdapter(FragmentManager fm) {
            super(fm);
        }

        @Override
        public Fragment getItem(int position) {

            // getItem is called to instantiate the fragment for the given page.

            switch(position) {
                default :
                case CHANNELS_PAGE : {
                    channelsFragment = new ChannelsSectionFragment();
                    return channelsFragment;
                }
                case CHAT_PAGE : {
                    chatFragment = new ChatSectionFragment();
                    return chatFragment;
                }
                case MEDIA_PAGE : {
                    mediaFragment = new MediaSectionFragment();
                    return mediaFragment;
                }
                case FILES_PAGE : {
                    filesFragment = new FilesSectionFragment();
                    return filesFragment;
                }
            }
        }

        @Override
        public int getCount() {
            return PAGE_COUNT;
        }

        @Override
        public CharSequence getPageTitle(int position) {
            Locale l = Locale.getDefault();
            switch(position) {
                case CHANNELS_PAGE :
                    return getString(R.string.title_section_channels).toUpperCase(l);
                case CHAT_PAGE :
                    return getString(R.string.title_section_chat).toUpperCase(l);
                case MEDIA_PAGE :
                    return getString(R.string.title_section_media).toUpperCase(l);
                case FILES_PAGE :
                    return getString(R.string.title_section_files).toUpperCase(l);
            }
            return null;
        }

        @Override
        public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
        }

        @Override
        public void onPageSelected(int position) {
            InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
            View v = getCurrentFocus();
            if (v != null)
                imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
            accessibilityAssistant.setVisiblePage(position);
        }

        @Override
        public void onPageScrollStateChanged(int state) {
        }
    }

    private void editChannelProperties(Channel channel) {
        Intent intent = new Intent(this, ChannelPropActivity.class);
        startActivityForResult(intent.putExtra(ChannelPropActivity.EXTRA_CHANNELID, channel.nChannelID), REQUEST_EDITCHANNEL);
    }

    private void joinChannelUnsafe(Channel channel, String passwd) {
        int cmdid = ttclient.doJoinChannelByID(channel.nChannelID, passwd);
        if(cmdid>0) {
            activecmds.put(cmdid, CmdComplete.CMD_COMPLETE_JOIN);
            channel.szPassword = passwd;
            ttservice.setJoinChannel(channel);
        }
        else {
            Toast.makeText(this, R.string.text_con_cmderr, Toast.LENGTH_LONG).show();
        }
    }

    private void joinChannel(final Channel channel, final String passwd) {
        if (filesAdapter.getActiveTransfersCount() > 0) {
            AlertDialog.Builder alert = new AlertDialog.Builder(this);
            alert.setMessage(R.string.channel_change_alert);
            alert.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int whichButton) {
                        filesAdapter.cancelAllTransfers();
                        joinChannelUnsafe(channel, passwd);
                    }
                });
            alert.setNegativeButton(android.R.string.cancel, null);
            alert.show();
        }

        else {
            joinChannelUnsafe(channel, passwd);
        }
    }

    private void joinChannel(final Channel channel) {
        if(channel.bPassword) {
            AlertDialog.Builder alert = new AlertDialog.Builder(this);
            alert.setTitle(R.string.pref_title_join_channel);
            alert.setMessage(R.string.channel_password_prompt);
            final EditText input = new EditText(this);
            input.setTransformationMethod(SingleLineTransformationMethod.getInstance());
            input.setText(channel.szPassword);
            alert.setView(input);
            alert.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int whichButton) {
                        joinChannel(channel, input.getText().toString());
                    }
                });
            alert.setNegativeButton(android.R.string.cancel, null);
            alert.show();
        }
        else {
            joinChannel(channel, "");
        }
    }

    private void setCurrentChannel(Channel channel) {
        curchannel = channel;
        getActionBar().setSubtitle((channel != null) ? channel.szName : null);
        invalidateOptionsMenu();
    }

    private boolean isVisibleChannel(int chanid) {
        if (curchannel != null) {
            if (curchannel.nParentID == chanid)
                return true;
            Channel channel = ttservice.getChannels().get(chanid);
            if (channel != null)
                return curchannel.nChannelID == channel.nParentID;
        }
        else {
            return chanid == ttclient.getRootChannelID();
        }
        return false;
    }

    public static class ChannelsSectionFragment extends Fragment {
        MainActivity mainActivity;

        public ChannelsSectionFragment() {
        }

        @Override
        public void onAttach(Activity activity) {
            mainActivity = (MainActivity) activity;
            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_main_channels, container, false);
            mainActivity.accessibilityAssistant.registerPage(rootView, SectionsPagerAdapter.CHANNELS_PAGE);

            ListView channelsList = (ListView) rootView.findViewById(R.id.listChannels);
            channelsList.setAdapter(mainActivity.getChannelsAdapter());
            channelsList.setOnItemClickListener(mainActivity);
            channelsList.setOnItemLongClickListener(mainActivity);

            return rootView;
        }
    }
    
    public static class ChatSectionFragment extends Fragment {
        MainActivity mainActivity;
        
        public ChatSectionFragment() {
        }
        
        @Override
        public void onAttach(Activity activity) {
            mainActivity = (MainActivity) activity;
            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_main_chat, container, false);
            mainActivity.accessibilityAssistant.registerPage(rootView, SectionsPagerAdapter.CHAT_PAGE);
            final EditText newmsg = (EditText) rootView.findViewById(R.id.channel_im_edittext);
            ListView chatlog = (ListView)rootView.findViewById(R.id.channel_im_listview);
            chatlog.setTranscriptMode(ListView.TRANSCRIPT_MODE_ALWAYS_SCROLL);
            chatlog.setAdapter(mainActivity.getTextMessagesAdapter());

            Button sendBtn = (Button) rootView.findViewById(R.id.channel_im_sendbtn);
            sendBtn.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View arg0) {
                    TextMessage textmsg = new TextMessage();
                    textmsg.nMsgType = TextMsgType.MSGTYPE_CHANNEL;
                    textmsg.nChannelID = mainActivity.ttclient.getMyChannelID();
                    textmsg.szMessage = newmsg.getText().toString();
                    int cmdid = mainActivity.ttclient.doTextMessage(textmsg);
                    if(cmdid>0) {
                        mainActivity.activecmds.put(cmdid, CmdComplete.CMD_COMPLETE_TEXTMSG);
                        newmsg.setText("");
                    }
                    else {
                        Toast.makeText(mainActivity, getResources().getString(R.string.text_con_cmderr),
                            Toast.LENGTH_LONG).show();
                    }
                }
            });
            return rootView;
        }
    }

    public static class VidcapSectionFragment extends Fragment {
        MainActivity mainActivity;

        public VidcapSectionFragment() {
        }

        @Override
        public void onAttach(Activity activity) {
            mainActivity = (MainActivity) activity;
            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_main_vidcap, container, false);
//            mainActivity.accessibilityAssistant.registerPage(rootView, SectionsPagerAdapter.VIDCAP_PAGE);

            return rootView;
        }
    }

    public static class MediaSectionFragment extends Fragment {
        MainActivity mainActivity;

        public MediaSectionFragment() {
        }

        @Override
        public void onAttach(Activity activity) {
            mainActivity = (MainActivity) activity;
            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                                 Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_main_media, container, false);
            mainActivity.accessibilityAssistant.registerPage(rootView, SectionsPagerAdapter.MEDIA_PAGE);

            ExpandableListView mediaview = (ExpandableListView) rootView.findViewById(R.id.media_elist_view);
            mediaview.setAdapter(mainActivity.getMediaAdapter());
            return rootView;
        }
    }

    public static class FilesSectionFragment extends ListFragment {

        @Override
        public void onViewCreated(View view, Bundle savedInstanceState) {
            MainActivity mainActivity = (MainActivity) getActivity();
            mainActivity.accessibilityAssistant.registerPage(view, SectionsPagerAdapter.FILES_PAGE);
            setListAdapter(mainActivity.getFilesAdapter());
            super.onViewCreated(view, savedInstanceState);
        }
    }

    class ChannelListAdapter extends BaseAdapter {

        private static final int PARENT_CHANNEL_VIEW_TYPE = 0,
            CHANNEL_VIEW_TYPE = 1,
            USER_VIEW_TYPE = 2,
            INFO_VIEW_TYPE = 3,

            VIEW_TYPE_COUNT = 4;

        private LayoutInflater inflater;

        Vector<Channel> subchannels = new Vector<Channel>();
        Vector<User> currentusers = new Vector<User>();

        ChannelListAdapter(Context context) {
            inflater = LayoutInflater.from(context);
        }

        @Override
        public void notifyDataSetChanged() {
            int chanid = 0;

            subchannels.clear();
            currentusers.clear();

            if(curchannel != null) {
                chanid = curchannel.nChannelID;

                subchannels = Utils.getSubChannels(chanid, ttservice.getChannels());
                currentusers = Utils.getUsers(chanid, ttservice.getUsers());
            }
            else {
                chanid = ttclient.getRootChannelID();
                Channel root = ttservice.getChannels().get(chanid);
                if(root != null)
                    subchannels.add(root);
            }

            Collections.sort(subchannels, new Comparator<Channel>() {
                    @Override
                    public int compare(Channel c1, Channel c2) {
                        if ((c1.nMaxUsers <= 0) && (c2.nMaxUsers > 0))
                            return -1;
                        else if ((c1.nMaxUsers > 0) && (c2.nMaxUsers <= 0))
                            return 1;
                        return c1.szName.compareToIgnoreCase(c2.szName);
                    }
                });

            Collections.sort(currentusers, new Comparator<User>() {
                    @Override
                    public int compare(User u1, User u2) {
                        if (((u1.uUserState & UserState.USERSTATE_VOICE) != 0) &&
                            ((u2.uUserState & UserState.USERSTATE_VOICE) == 0))
                            return -1;
                        else if (((u1.uUserState & UserState.USERSTATE_VOICE) == 0) &&
                                 ((u2.uUserState & UserState.USERSTATE_VOICE) != 0))
                            return 1;
                        
                        String name1 = Utils.getDisplayName(getBaseContext(), u1);
                        String name2 = Utils.getDisplayName(getBaseContext(), u2);
                        return name1.compareToIgnoreCase(name2);
                    }
                });

            super.notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            int count = subchannels.size() + currentusers.size();
            if ((curchannel != null) && (curchannel.nParentID > 0)) {
                count++; // include parent channel shortcut
            }
            return count;
        }

        @Override
        public Object getItem(int position) {
            if ((curchannel != null) && (curchannel.nParentID > 0)) {
                if(position == 0) {
                    Channel parent = ttservice.getChannels().get(curchannel.nParentID);

                    if(parent != null)
                        return parent;
                    return new Channel();
                }

                position--; // substract parent channel shortcut
            }
            if(position < subchannels.size()) {
                return subchannels.get(position);
            }
            else {
                position -= subchannels.size();
                return currentusers.get(position);
            }
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public int getItemViewType(int position) {
            if ((curchannel != null) && (curchannel.nParentID > 0)) {
                if (position == 0) {
                    return PARENT_CHANNEL_VIEW_TYPE;
                }
                else if (position <= subchannels.size()) {
                    return (subchannels.get(position - 1).nMaxUsers > 0) ? CHANNEL_VIEW_TYPE : INFO_VIEW_TYPE;
                }
                return USER_VIEW_TYPE;
            }
            else if (position < subchannels.size()) {
                return (subchannels.get(position).nMaxUsers > 0) ? CHANNEL_VIEW_TYPE : INFO_VIEW_TYPE;
            }
            return USER_VIEW_TYPE;
        }

        @Override
        public int getViewTypeCount() {
            return VIEW_TYPE_COUNT;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {

            Object item = getItem(position);

            if(item instanceof Channel) {

                final Channel channel = (Channel) item;

                if ((curchannel != null) && (curchannel.nParentID > 0) && (position == 0)) {
                    // show parent channel shortcut
                    if (convertView == null ||
                        convertView.findViewById(R.id.parentname) == null)
                        convertView = inflater.inflate(R.layout.item_channel_back, parent, false);
                }
                else if (channel.nMaxUsers == 0) {
                    if (convertView == null ||
                        convertView.findViewById(R.id.titletext) == null)
                        convertView = inflater.inflate(R.layout.item_info, parent, false);
                    TextView title = (TextView) convertView.findViewById(R.id.titletext);
                    TextView details = (TextView) convertView.findViewById(R.id.infodetails);
                    title.setText(channel.szName);
                    details.setText(channel.szTopic);
                }
                else {

                    if (convertView == null ||
                        convertView.findViewById(R.id.channelname) == null)
                        convertView = inflater.inflate(R.layout.item_channel, parent, false);

                    ImageView chanicon = (ImageView) convertView.findViewById(R.id.channelicon);
                    TextView name = (TextView) convertView.findViewById(R.id.channelname);
                    TextView topic = (TextView) convertView.findViewById(R.id.chantopic);
                    Button join = (Button) convertView.findViewById(R.id.join_btn);
                    int icon_resource = R.drawable.channel_orange;
                    if(channel.bPassword) {
                        icon_resource = R.drawable.channel_pink;
                        chanicon.setContentDescription(getString(R.string.text_passwdprot));
                        chanicon.setImportantForAccessibility(View.IMPORTANT_FOR_ACCESSIBILITY_YES);
                    }
                    else {
                        chanicon.setContentDescription(null);
                        chanicon.setImportantForAccessibility(View.IMPORTANT_FOR_ACCESSIBILITY_NO);
                    }
                    chanicon.setImageResource(icon_resource);
                    
                    if(channel.nParentID == 0) {
                        // show server name as channel name for root channel
                        ServerProperties srvprop = new ServerProperties();
                        ttclient.getServerProperties(srvprop);
                        name.setText(srvprop.szServerName);
                    }
                    else {
                        name.setText(channel.szName);
                    }
                    topic.setText(channel.szTopic);

                    OnClickListener listener = new OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            switch(v.getId()) {
                                case R.id.join_btn : {
                                    joinChannel(channel);
                                }
                                break;
                            }
                        }
                    };
                    join.setOnClickListener(listener);
                    join.setAccessibilityDelegate(accessibilityAssistant);
                    join.setEnabled(channel.nChannelID != ttclient.getMyChannelID());
                }
                if (channel.nMaxUsers > 0) {
                    int population = Utils.getUsers(channel.nChannelID, ttservice.getUsers()).size();
                    ((TextView)convertView.findViewById(R.id.population)).setText((population > 0) ? String.format("(%d)", population) : "");
                }
            }
            else if(item instanceof User) {
                if (convertView == null ||
                    convertView.findViewById(R.id.nickname) == null)
                    convertView = inflater.inflate(R.layout.item_user, parent, false);
                ImageView usericon = (ImageView) convertView.findViewById(R.id.usericon);
                TextView nickname = (TextView) convertView.findViewById(R.id.nickname);
                TextView status = (TextView) convertView.findViewById(R.id.status);
                final User user = (User) item;
                String name = Utils.getDisplayName(getBaseContext(), user);
                nickname.setText(name);
                status.setText(user.szStatusMsg);
                
                boolean talking = (user.uUserState & UserState.USERSTATE_VOICE) != 0;
                boolean female = (user.nStatusMode & TeamTalkConstants.STATUSMODE_FEMALE) != 0;
                boolean away =  (user.nStatusMode & TeamTalkConstants.STATUSMODE_AWAY) != 0;
                int icon_resource = R.drawable.man_blue;
                
                if(user.nUserID == ttservice.getTTInstance().getMyUserID()) {
                    talking = ttservice.isVoiceTransmitting();
                }
                
                if(talking) {
                    String name1 = Utils.getDisplayName(getBaseContext(), user);
                    nickname.setContentDescription(getString(R.string.user_state_now_speaking, name1));
                    if(female) {
                        icon_resource = R.drawable.woman_green;
                    }
                    else {
                        icon_resource = R.drawable.man_green;
                    }
                }
                else {
                    nickname.setContentDescription(null);
                    if(female) {
                        icon_resource = away? R.drawable.woman_orange : R.drawable.woman_blue;
                    }
                    else {
                        icon_resource = away? R.drawable.man_orange : R.drawable.man_blue;
                    }
                }
                usericon.setImageResource(icon_resource);
                usericon.setImportantForAccessibility(View.IMPORTANT_FOR_ACCESSIBILITY_NO);
                
                Button sndmsg = (Button) convertView.findViewById(R.id.msg_btn);
                OnClickListener listener = new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        switch(v.getId()) {
                            case R.id.msg_btn : {
                                Intent intent = new Intent(MainActivity.this, TextMessageActivity.class);
                                startActivity(intent.putExtra(TextMessageActivity.EXTRA_USERID, user.nUserID));
                                break;
                            }
                        }
                    }
                };
                sndmsg.setOnClickListener(listener);
                sndmsg.setAccessibilityDelegate(accessibilityAssistant);
            }
            convertView.setAccessibilityDelegate(accessibilityAssistant);
            return convertView;
        }
    }
    
    void createStatusTimer() {
        
        final TextView connection = (TextView) findViewById(R.id.connectionstat_textview);
        final TextView ping = (TextView) findViewById(R.id.pingstat_textview);
        final TextView total = (TextView) findViewById(R.id.totalstat_textview);
        final int defcolor = connection.getTextColors().getDefaultColor();
        
        if(stats_timer == null) {
            stats_timer = new CountDownTimer(10000, 1000) {
                
                ClientStatistics prev_stats;
                
                public void onTick(long millisUntilFinished) {
                
                    if (ttclient == null || accessibilityAssistant.isUiUpdateDiscouraged())
                        return;
                    filesAdapter.performPendingUpdate();

                    int con = R.string.stat_offline;
                    int con_color = Color.RED;
                    int flags = ttclient.getFlags(); 
                    if((flags & ClientFlag.CLIENT_CONNECTED) == ClientFlag.CLIENT_CONNECTED) {
                        con = R.string.stat_online;
                        con_color = Color.GREEN;
                    }
                    else if((flags & ClientFlag.CLIENT_CONNECTING) == ClientFlag.CLIENT_CONNECTING) {
                        con = R.string.stat_connecting;
                    }
                    
                    connection.setText(con);
                    connection.setTextColor(con_color);

                    ClientStatistics stats = new ClientStatistics();
                    if(!ttclient.getClientStatistics(stats))
                        return;
                    
                    if(prev_stats == null)
                        prev_stats = stats;
                    
                    long totalrx = stats.nUdpBytesRecv - prev_stats.nUdpBytesRecv;
                    long totaltx = stats.nUdpBytesSent - prev_stats.nUdpBytesSent;
                    long voicerx = stats.nVoiceBytesRecv - prev_stats.nVoiceBytesRecv;
                    long voicetx = stats.nVoiceBytesSent - prev_stats.nVoiceBytesSent;
                    long deskrx = stats.nDesktopBytesRecv - prev_stats.nDesktopBytesRecv;
                    long desktx = stats.nDesktopBytesSent - prev_stats.nDesktopBytesSent;
                    long mfrx = (stats.nMediaFileAudioBytesRecv + stats.nMediaFileVideoBytesRecv) - 
                                (prev_stats.nMediaFileAudioBytesRecv + prev_stats.nMediaFileVideoBytesRecv);
                    long mftx = (stats.nMediaFileAudioBytesSent + stats.nMediaFileVideoBytesSent) - 
                                (prev_stats.nMediaFileAudioBytesSent + prev_stats.nMediaFileVideoBytesSent);

                    String str;
                    if(stats.nUdpPingTimeMs >= 0) {
                        str = String.format("%1$d", stats.nUdpPingTimeMs); 
                        ping.setText(str);
                        
                        if(stats.nUdpPingTimeMs > 250) {
                            ping.setTextColor(Color.RED);
                        }
                        else {
                            ping.setTextColor(defcolor);
                        }
                    }                    
                    
                    str = String.format("%1$d/%2$d KB", totalrx/ 1024, totaltx / 1024);
                    total.setText(str);
                    
                    prev_stats = stats;
                }

                public void onFinish() {
                    start();
                }
            }.start();
        }

    }

    @Override
    public void onItemClick(AdapterView< ? > l, View v, int position, long id) {

        Object item = channelsAdapter.getItem(position);
        if(item instanceof User) {
            User user = (User)item;
            Intent intent = new Intent(this, UserPropActivity.class);
            // TODO: check 'curchannel' for null
            startActivityForResult(intent.putExtra(UserPropActivity.EXTRA_USERID, user.nUserID),
                                   REQUEST_EDITUSER);
        }
        else if(item instanceof Channel) {
            Channel channel = (Channel) item;
            setCurrentChannel((channel.nChannelID > 0) ? channel : null);
            channelsAdapter.notifyDataSetChanged();
        }
        else {
        }
    }

    Channel selectedChannel;
    User selectedUser;
    List<Integer> userIDS = new ArrayList<Integer>();

    @Override
    public boolean onItemLongClick(AdapterView< ? > l, View v, int position, long id) {
        Object item = channelsAdapter.getItem(position);
        if (item instanceof User) {
            selectedUser = (User) item;
            UserAccount myuseraccount = new UserAccount();
            ttclient.getMyUserAccount(myuseraccount);

            boolean banRight = (myuseraccount.uUserRights & UserRight.USERRIGHT_BAN_USERS) !=0;
            boolean moveRight = (myuseraccount.uUserRights & UserRight.USERRIGHT_MOVE_USERS) !=0;
            boolean kickRight = (myuseraccount.uUserRights & UserRight.USERRIGHT_KICK_USERS) !=0;
            // operator of a channel can also kick users
            int myuserid = ttclient.getMyUserID();
            kickRight |= ttclient.isChannelOperator(myuserid, selectedUser.nChannelID);

            PopupMenu userActions = new PopupMenu(this, v);
            userActions.setOnMenuItemClickListener(this);
            userActions.inflate(R.menu.user_actions);
            userActions.getMenu().findItem(R.id.action_kickchan).setEnabled(kickRight).setVisible(kickRight);
            userActions.getMenu().findItem(R.id.action_kicksrv).setEnabled(kickRight).setVisible(kickRight);
            userActions.getMenu().findItem(R.id.action_banchan).setEnabled(banRight).setVisible(banRight);
            userActions.getMenu().findItem(R.id.action_bansrv).setEnabled(banRight).setVisible(banRight);
            userActions.getMenu().findItem(R.id.action_select).setEnabled(moveRight).setVisible(moveRight);
            userActions.show();
            return true;
        }
        if (item instanceof Channel) {
            selectedChannel = (Channel) item;
            if ((curchannel != null) && (curchannel.nParentID != selectedChannel.nChannelID)) {
                boolean isRemovable = (ttclient != null) && (selectedChannel.nChannelID != ttclient.getMyChannelID());
                PopupMenu channelActions = new PopupMenu(this, v);
                channelActions.setOnMenuItemClickListener(this);
                channelActions.inflate(R.menu.channel_actions);
                channelActions.getMenu().findItem(R.id.action_remove).setEnabled(isRemovable).setVisible(isRemovable);
                channelActions.show();
                return true;
            }
        }
        return false;
    }

    @Override
    public boolean onMenuItemClick(MenuItem item) {
        AlertDialog.Builder alert = new AlertDialog.Builder(this);
        switch (item.getItemId()) {
        case R.id.action_banchan:
            alert.setMessage(getString(R.string.ban_confirmation, selectedUser.szNickname));
            alert.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {

                @Override
                public void onClick(DialogInterface dialog, int whichButton) {
                    ttclient.doBanUser(selectedUser.nUserID, selectedUser.nChannelID);
                    ttclient.doKickUser(selectedUser.nUserID, selectedUser.nChannelID);
                }
            });

            alert.setNegativeButton(android.R.string.no, null);
            alert.show();
            break;
        case R.id.action_bansrv:
            alert.setMessage(getString(R.string.ban_confirmation, selectedUser.szNickname));
            alert.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {

                @Override
                public void onClick(DialogInterface dialog, int whichButton) {
                    ttclient.doBanUser(selectedUser.nUserID, 0);
                    ttclient.doKickUser(selectedUser.nUserID, 0);
                }
            });

            alert.setNegativeButton(android.R.string.no, null);
            alert.show();
            break;
        case R.id.action_edit:
            editChannelProperties(selectedChannel);
            break;
        case R.id.action_kickchan:
            alert.setMessage(getString(R.string.kick_confirmation, selectedUser.szNickname));
            alert.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {

                @Override
                public void onClick(DialogInterface dialog, int whichButton) {
                    ttclient.doKickUser(selectedUser.nUserID, selectedUser.nChannelID);
                }
            });

            alert.setNegativeButton(android.R.string.no, null);
            alert.show();
            break;
        case R.id.action_kicksrv:
            alert.setMessage(getString(R.string.kick_confirmation, selectedUser.szNickname));
            alert.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {

                @Override
                public void onClick(DialogInterface dialog, int whichButton) {
                    ttclient.doKickUser(selectedUser.nUserID, 0);
                }
            });

            alert.setNegativeButton(android.R.string.no, null);
            alert.show();
            break;
        case R.id.action_move:
            Iterator<Integer> userIDSIterator = userIDS.iterator(); 
            while (userIDSIterator.hasNext()) {
                ttclient.doMoveUser(userIDSIterator.next(), selectedChannel.nChannelID);
            }
            userIDS.clear();
            break;
        case R.id.action_select:
            userIDS.add(selectedUser.nUserID);
            break;
        case R.id.action_remove: {
            alert.setMessage(getString(R.string.channel_remove_confirmation, selectedChannel.szName));
            alert.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int whichButton) {
                        if (ttclient.doRemoveChannel(selectedChannel.nChannelID) <= 0)
                            Toast.makeText(MainActivity.this,
                                           getString(R.string.err_channel_remove,
                                                     selectedChannel.szName),
                                           Toast.LENGTH_LONG).show();
                    }
                });

            alert.setNegativeButton(android.R.string.no, null);
            alert.show();
            break;
        }

        default:
            return false;
        }
        return true;
    }

    private void adjustMuteButton(ImageButton btn) {
        if (ttservice.getCurrentMuteState()) {
            btn.setImageResource(R.drawable.mute_blue);
            btn.setContentDescription(getString(R.string.speaker_unmute));
        }
        else {
            btn.setImageResource(R.drawable.speaker_blue);
            btn.setContentDescription(getString(R.string.speaker_mute));
        }
    }

    private void adjustVoxState(boolean voiceActivationEnabled, int level) {
        ImageButton voxSwitch = (ImageButton) findViewById(R.id.voxSwitch);
        TextView mikeLevel = (TextView) findViewById(R.id.mikelevel_text);

        if (voiceActivationEnabled) {
            mikeLevel.setText(level + "%");
            mikeLevel.setContentDescription(getString(R.string.vox_level_description, mikeLevel.getText()));
            voxSwitch.setImageResource(R.drawable.microphone);
            voxSwitch.setContentDescription(getString(R.string.voice_activation_off));
            findViewById(R.id.mikeDec).setContentDescription(getString(R.string.decvoxlevel));
            findViewById(R.id.mikeInc).setContentDescription(getString(R.string.incvoxlevel));
        }
        else {
            mikeLevel.setText(Utils.refVolumeToPercent(level) + "%");
            mikeLevel.setContentDescription(getString(R.string.mic_gain_description, mikeLevel.getText()));
            voxSwitch.setImageResource(R.drawable.mike_green);
            voxSwitch.setContentDescription(getString(R.string.voice_activation_on));
            findViewById(R.id.mikeDec).setContentDescription(getString(R.string.decgain));
            findViewById(R.id.mikeInc).setContentDescription(getString(R.string.incgain));
        }
    }

    private void adjustTxState(boolean txEnabled) {
        findViewById(R.id.transmit_voice).setBackgroundColor(txEnabled ? Color.GREEN : Color.RED);
        findViewById(R.id.transmit_voice).setContentDescription(txEnabled ? getString(R.string.tx_on) : getString(R.string.tx_off));

        if ((curchannel != null) && (ttclient.getMyChannelID() == curchannel.nChannelID)) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
    }

    private interface OnButtonInteractionListener extends OnTouchListener, OnClickListener {
    }

    private void setupButtons() {
        
        final Button tx_btn = (Button) findViewById(R.id.transmit_voice);

        OnButtonInteractionListener txButtonListener = new OnButtonInteractionListener() {

            boolean tx_state = false;
            long tx_down_start = 0;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                boolean tx = event.getAction() != MotionEvent.ACTION_UP;

                if(tx != tx_state) {

                    if(!tx) {
                        if(System.currentTimeMillis() - tx_down_start < 800) {
                            tx = true;
                            tx_down_start = 0;
                        }
                        else {
                            tx_down_start = System.currentTimeMillis();
                        }

                        //Log.i(TAG, "TX is now: " + tx + " diff " + (System.currentTimeMillis() - tx_down_start));
                    }

                    if (ttservice.isVoiceActivationEnabled())
                        ttservice.enableVoiceActivation(false);
                    ttservice.enableVoiceTransmission(tx);
                }
                tx_state = tx;
                return true;
            }

            @Override
            public void onClick(View v) {
                if(System.currentTimeMillis() - tx_down_start < 800) {
                    tx_state = true;
                    tx_down_start = 0;
                }
                else {
                    tx_state = false;
                    tx_down_start = System.currentTimeMillis();
                }
                if (ttservice.isVoiceActivationEnabled())
                    ttservice.enableVoiceActivation(false);
                ttservice.enableVoiceTransmission(tx_state);
            }
        };

        tx_btn.setOnTouchListener(txButtonListener);
        
        final ImageButton decVol = (ImageButton) findViewById(R.id.volDec);
        final ImageButton incVol = (ImageButton) findViewById(R.id.volInc);
        final ImageButton decMike = (ImageButton) findViewById(R.id.mikeDec);
        final ImageButton incMike = (ImageButton) findViewById(R.id.mikeInc);
        final TextView mikeLevel = (TextView) findViewById(R.id.mikelevel_text);
        final TextView volLevel = (TextView) findViewById(R.id.vollevel_text);
        
        OnButtonInteractionListener tuningButtonListener = new OnButtonInteractionListener() {
            Handler handler = new Handler();
            Runnable runnable;

            @Override
            public boolean onTouch(final View v, MotionEvent event) {
                if(ttclient == null)
                    return false;

                if(event.getAction() == MotionEvent.ACTION_DOWN) {
                    adjustLevel(v);
                    
                    runnable = new Runnable() {

                        @Override
                        public void run() {
                            boolean done = adjustLevel(v);
                            if(!done)
                                handler.postDelayed(this, 100);
                        }
                    };
                    handler.postDelayed(runnable, 100);
                }
                else if(event.getAction() == MotionEvent.ACTION_UP) {
                    if(runnable != null)
                        handler.removeCallbacks(runnable);
                }
                return false;
            }

            @Override
            public void onClick(View v) {
                if(ttclient != null)
                    adjustLevel(v);
            }

            boolean adjustLevel(View view) {
                if(view == decVol) {

                    // pressing +/- aborts mute state
                    if(ttservice.isMute()) {
                        ttservice.setMute(false);
                        ImageButton speakerBtn = (ImageButton) findViewById(R.id.speakerBtn);
                        adjustMuteButton(speakerBtn);
                    }

                    int v = ttclient.getSoundOutputVolume();
                    v = Utils.refVolumeToPercent(v);
                    v = Utils.refVolume(v-1);
                    if(v >= SoundLevel.SOUND_VOLUME_MIN) {
                        ttclient.setSoundOutputVolume(v);
                        volLevel.setText(Utils.refVolumeToPercent(v) + "%");
                        volLevel.setContentDescription(getString(R.string.speaker_volume_description, volLevel.getText()));
                        if(v == SoundLevel.SOUND_VOLUME_DEFAULT)
                            return true;
                    }
                    else
                        return true;
                }
                else if(view == incVol) {

                    // pressing +/- aborts mute state
                    if(ttservice.isMute()) {
                        ttservice.setMute(false);
                        ImageButton speakerBtn = (ImageButton) findViewById(R.id.speakerBtn);
                        adjustMuteButton(speakerBtn);
                    }

                    int v = ttclient.getSoundOutputVolume();
                    v = Utils.refVolumeToPercent(v);
                    v = Utils.refVolume(v+1);
                    if(v <= SoundLevel.SOUND_VOLUME_MAX) {
                        ttclient.setSoundOutputVolume(v);
                        volLevel.setText(Utils.refVolumeToPercent(v) + "%");
                        volLevel.setContentDescription(getString(R.string.speaker_volume_description, volLevel.getText()));
                        if(v == SoundLevel.SOUND_VOLUME_DEFAULT)
                            return true;
                    }
                    else
                        return true;
                }
                else if(view == decMike) {
                    if (ttservice.isVoiceActivationEnabled()) {
                        int x = ttclient.getVoiceActivationLevel() - 1;
                        if (x >= SoundLevel.SOUND_VU_MIN) {
                            ttclient.setVoiceActivationLevel(x);
                            mikeLevel.setText(x + "%");
                            mikeLevel.setContentDescription(getString(R.string.vox_level_description, mikeLevel.getText()));
                        }
                        else
                            return true;
                    }
                    else {
                        int g = ttclient.getSoundInputGainLevel();
                        g = Utils.refGainToPercent(g);
                        g = Utils.refGain(g-1);
                        if(g >= SoundLevel.SOUND_GAIN_MIN) {
                            ttclient.setSoundInputGainLevel(g);
                            mikeLevel.setText(Utils.refVolumeToPercent(g) + "%");
                            mikeLevel.setContentDescription(getString(R.string.mic_gain_description, mikeLevel.getText()));
                            if(g == SoundLevel.SOUND_GAIN_DEFAULT)
                                return true;
                        }
                        else
                            return true;
                    }
                }
                else if(view == incMike) {
                    if (ttservice.isVoiceActivationEnabled()) {
                        int x = ttclient.getVoiceActivationLevel() + 1;
                        if (x <= SoundLevel.SOUND_VU_MAX) {
                            ttclient.setVoiceActivationLevel(x);
                            mikeLevel.setText(x + "%");
                            mikeLevel.setContentDescription(getString(R.string.vox_level_description, mikeLevel.getText()));
                        }
                        else
                            return true;
                    }
                    else {
                        int g = ttclient.getSoundInputGainLevel();
                        g = Utils.refGainToPercent(g);
                        g = Utils.refGain(g+1);
                        if(g <= SoundLevel.SOUND_GAIN_MAX) {
                            ttclient.setSoundInputGainLevel(g);
                            mikeLevel.setText(Utils.refVolumeToPercent(g) + "%");
                            mikeLevel.setContentDescription(getString(R.string.mic_gain_description, mikeLevel.getText()));
                            if(g == SoundLevel.SOUND_VOLUME_DEFAULT)
                                return true;
                        }
                        else
                            return true;
                    }
                }
                return false;
            }
        };

        decVol.setOnTouchListener(tuningButtonListener);
        incVol.setOnTouchListener(tuningButtonListener);
        decMike.setOnTouchListener(tuningButtonListener);
        incMike.setOnTouchListener(tuningButtonListener);

        if ((Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) && accessibilityAssistant.isServiceActive()) {
            tx_btn.setOnClickListener(txButtonListener);
            decVol.setOnClickListener(tuningButtonListener);
            incVol.setOnClickListener(tuningButtonListener);
            decMike.setOnClickListener(tuningButtonListener);
            incMike.setOnClickListener(tuningButtonListener);
        }

        ImageButton speakerBtn = (ImageButton) findViewById(R.id.speakerBtn);
        speakerBtn.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    if ((mConnection != null) && mConnection.isBound()) {
                        ttservice.setMute(!ttservice.isMute());
                        adjustMuteButton((ImageButton) v);

                        int level = ttservice.isMute() ?
                            0 :
                            Utils.refVolumeToPercent(ttclient.getSoundOutputVolume());
                        volLevel.setText(level + "%");
                        volLevel.setContentDescription(getString(R.string.speaker_volume_description, volLevel.getText()));
                    }
                }
            });

        ImageButton voxSwitch = (ImageButton) findViewById(R.id.voxSwitch);
        voxSwitch.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    if ((mConnection != null) && mConnection.isBound()) {
                        if (ttservice.isVoiceTransmissionEnabled())
                            ttservice.enableVoiceTransmission(false);
                        ttservice.enableVoiceActivation(!ttservice.isVoiceActivationEnabled());
                    }
                }
            });
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
        ttservice = service;
        ttclient = ttservice.getTTInstance();

        //ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 440);

        int mychannel = ttclient.getMyChannelID();
        if(mychannel > 0) {
            setCurrentChannel(ttservice.getChannels().get(mychannel));
        }

        this.mychannel = ttservice.getChannels().get(mychannel);

        mSectionsPagerAdapter.onPageSelected(mViewPager.getCurrentItem());

        channelsAdapter.notifyDataSetChanged();

        textmsgAdapter.setTextMessages(ttservice.getChatLogTextMsgs());
        textmsgAdapter.setMyUserID(ttclient.getMyUserID());
        textmsgAdapter.notifyDataSetChanged();

        mediaAdapter.setTeamTalkService(service);
        mediaAdapter.notifyDataSetChanged();

        filesAdapter.setTeamTalkService(service);
        filesAdapter.update(mychannel);

        int outsndid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT;
        // outsndid |= SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;

        int flags = ttclient.getFlags();
        if (((flags & ClientFlag.CLIENT_SNDOUTPUT_READY) == 0) &&
            !ttclient.initSoundOutputDevice(outsndid))
            Toast.makeText(this, R.string.err_init_sound_output, Toast.LENGTH_LONG).show();

        if (!restarting) {
            ttservice.setMute(false);
            ttservice.enableVoiceTransmission(false);
            ttservice.enableVoiceActivation(false);
            if (Permissions.setupPermission(getBaseContext(), this, Permissions.MY_PERMISSIONS_REQUEST_READ_PHONE_STATE))
                ttservice.enablePhoneCallReaction();
        }

        ttservice.registerConnectionListener(this);
        ttservice.registerCommandListener(this);
        ttservice.registerUserListener(this);
        ttservice.registerClientListener(this);
        ttservice.setOnVoiceTransmissionToggleListener(this);
        audioManager.registerMediaButtonEventReceiver(mediaButtonEventReceiver);

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());

        if (Permissions.setupPermission(getBaseContext(), this, Permissions.MY_PERMISSIONS_REQUEST_WAKE_LOCK))
            wakeLock.acquire();

        int mastervol = prefs.getInt(Preferences.PREF_SOUNDSYSTEM_MASTERVOLUME, SoundLevel.SOUND_VOLUME_DEFAULT);
        int gain = prefs.getInt(Preferences.PREF_SOUNDSYSTEM_MICROPHONEGAIN, SoundLevel.SOUND_GAIN_DEFAULT);
        int voxlevel = prefs.getInt(Preferences.PREF_SOUNDSYSTEM_VOICEACTIVATION_LEVEL, 5);
        boolean voxState = ttservice.isVoiceActivationEnabled();
        boolean txState = ttservice.isVoiceTransmitting();

        // only set volume and gain if tt-instance hasn't already been configured
        if (ttclient.getSoundOutputVolume() != mastervol)
            ttclient.setSoundOutputVolume(mastervol);
        if (ttclient.getSoundInputGainLevel() != gain)
            ttclient.setSoundInputGainLevel(gain);
        if (ttclient.getVoiceActivationLevel() != voxlevel)
            ttclient.setVoiceActivationLevel(voxlevel);

        adjustMuteButton((ImageButton) findViewById(R.id.speakerBtn));
        adjustVoxState(voxState, voxState ? voxlevel : gain);
        adjustTxState(txState);

        TextView volLevel = (TextView) findViewById(R.id.vollevel_text);
        volLevel.setText(Utils.refVolumeToPercent(mastervol) + "%");
        volLevel.setContentDescription(getString(R.string.speaker_volume_description, volLevel.getText()));
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
        if (wakeLock.isHeld())
            wakeLock.release();
        audioManager.unregisterMediaButtonEventReceiver(mediaButtonEventReceiver);
        service.setOnVoiceTransmissionToggleListener(null);
        service.unregisterConnectionListener(this);
        service.unregisterCommandListener(this);
        service.unregisterUserListener(this);
        service.unregisterClientListener(this);
        filesAdapter.setTeamTalkService(null);
        mediaAdapter.clearTeamTalkService(service);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[], int[] grantResults) {
        boolean granted = grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED;
        switch (requestCode) {
            case Permissions.MY_PERMISSIONS_REQUEST_VIBRATE :
                break;
            case Permissions.MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE :
                startActivityForResult(new Intent(this, FilePickerActivity.class), REQUEST_SELECT_FILE);
                break;
            case Permissions.MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE :
                break;
            case Permissions.MY_PERMISSIONS_REQUEST_WAKE_LOCK:
                wakeLock.acquire();
                break;
            case Permissions.MY_PERMISSIONS_REQUEST_READ_PHONE_STATE:
                if ((mConnection != null) && mConnection.isBound())
                    ttservice.enablePhoneCallReaction();
                break;
        }
    }

    @Override
    public void onCmdError(int cmdId, ClientErrorMsg errmsg) {
        // error is notified in service
    }

    @Override
    public void onCmdSuccess(int cmdId) {
    }

    @Override
    public void onCmdProcessing(int cmdId, boolean complete) {
        if(complete) {
            activecmds.remove(cmdId);
        }
    }

    @Override
    public void onCmdMyselfLoggedIn(int my_userid, UserAccount useraccount) {
        textmsgAdapter.setMyUserID(my_userid);
    }

    @Override
    public void onCmdMyselfLoggedOut() {
        accessibilityAssistant.lockEvents();
        channelsAdapter.notifyDataSetChanged();
        accessibilityAssistant.unlockEvents();
    }

    @Override
    public void onCmdMyselfKickedFromChannel() {
        accessibilityAssistant.lockEvents();
        channelsAdapter.notifyDataSetChanged();
        accessibilityAssistant.unlockEvents();
    }

    @Override
    public void onCmdMyselfKickedFromChannel(User kicker) {
        accessibilityAssistant.lockEvents();
        channelsAdapter.notifyDataSetChanged();
        accessibilityAssistant.unlockEvents();
    }

    @Override
    public void onCmdUserLoggedIn(User user) {
        if (ttsWrapper != null && PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("server_login_checkbox", false)) {
            String name = Utils.getDisplayName(getBaseContext(), user);
            ttsWrapper.speak(name + " " + getResources().getString(R.string.text_tts_loggedin));
        }
    }

    @Override
    public void onCmdUserLoggedOut(User user) {
        if (ttsWrapper != null && PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("server_logout_checkbox", false)) {
            String name = Utils.getDisplayName(getBaseContext(), user);
            ttsWrapper.speak(name + " " + getResources().getString(R.string.text_tts_loggedout));
        }
    }

    @Override
    public void onCmdUserUpdate(User user) {
        if(curchannel != null && curchannel.nChannelID == user.nChannelID) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
    }

    @Override
    public void onCmdUserJoinedChannel(User user) {
        
        if(user.nUserID == ttclient.getMyUserID()) {
            //myself joined channel
            Channel chan = ttservice.getChannels().get(user.nChannelID);
            setCurrentChannel(chan);
            filesAdapter.update(curchannel);
            //store copy of channel
            mychannel = chan;

            //update the displayed channel to the one we're currently in
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
        else if(curchannel != null && curchannel.nChannelID == user.nChannelID) {
            //other user joined current channel
        }
        
        if(curchannel != null && curchannel.nChannelID == user.nChannelID) {
            //event took place in current channel
            
            if(user.nUserID != ttclient.getMyUserID()) {
                accessibilityAssistant.lockEvents();
                textmsgAdapter.notifyDataSetChanged();
                channelsAdapter.notifyDataSetChanged();
                if (ttclient.getMyChannelID() == user.nChannelID) {
                    if (sounds.get(SOUND_USERJOIN) != 0)
                        audioIcons.play(sounds.get(SOUND_USERJOIN), 1.0f, 1.0f, 0, 0, 1.0f);
                    if (ttsWrapper != null && PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("channel_join_checkbox", false)) {
                        String name = Utils.getDisplayName(getBaseContext(), user);
                        ttsWrapper.speak(name + " " + getResources().getString(R.string.text_tts_joined_chan));
                    }
                }
                accessibilityAssistant.unlockEvents();
            }
            else {
                textmsgAdapter.notifyDataSetChanged();
                channelsAdapter.notifyDataSetChanged();
            }
        }
        else if (isVisibleChannel(user.nChannelID)) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
    }

    @Override
    public void onCmdUserLeftChannel(int channelid, User user) {
        
        if(user.nUserID == ttclient.getMyUserID()) {
            //myself left current channel
            
            Channel chan = ttservice.getChannels().get(channelid);
            textmsgAdapter.notifyDataSetChanged();
            
            setCurrentChannel(null);
            mychannel = null;
        }
        else if(curchannel != null && channelid == curchannel.nChannelID){
            //other user left current channel
            
            accessibilityAssistant.lockEvents();
            textmsgAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
        
        if(curchannel != null && curchannel.nChannelID == channelid) {
            //event took place in current channel
            
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            if (ttclient.getMyChannelID() == channelid) {
                    if (sounds.get(SOUND_USERLEFT) != 0)
                        audioIcons.play(sounds.get(SOUND_USERLEFT), 1.0f, 1.0f, 0, 0, 1.0f);
                if (ttsWrapper != null && PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("channel_leave_checkbox", false)) {
                    String name = Utils.getDisplayName(getBaseContext(), user);
                    ttsWrapper.speak(name + " " + getResources().getString(R.string.text_tts_left_chan));
                }
            }
            accessibilityAssistant.unlockEvents();
        }
        else if (isVisibleChannel(channelid)) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
    }

    @Override
    public void onCmdUserTextMessage(TextMessage textmessage) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        switch (textmessage.nMsgType) {
        case TextMsgType.MSGTYPE_CHANNEL :
            accessibilityAssistant.lockEvents();
            textmsgAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
            
            if (sounds.get(SOUND_CHANMSG) != 0)
                audioIcons.play(sounds.get(SOUND_CHANMSG), 1.0f, 1.0f, 0, 0, 1.0f);
            if (ttsWrapper != null && prefs.getBoolean("channel_message_checkbox", false)) {
                User sender = ttservice.getUsers().get(textmessage.nFromUserID);
                String name = Utils.getDisplayName(getBaseContext(), sender);
                ttsWrapper.speak(getString(R.string.text_tts_channel_message, (sender != null) ? name : "", textmessage.szMessage));
            }
            Log.d(TAG, "Channel message in " + this.hashCode());
            break;
        case TextMsgType.MSGTYPE_BROADCAST :
            accessibilityAssistant.lockEvents();
            textmsgAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
            
            if (sounds.get(SOUND_BCASTMSG) != 0)
                audioIcons.play(sounds.get(SOUND_BCASTMSG), 1.0f, 1.0f, 0, 0, 1.0f);
            if (ttsWrapper != null && prefs.getBoolean("broadcast_message_checkbox", false)) {
                User sender = ttservice.getUsers().get(textmessage.nFromUserID);
                String name = Utils.getDisplayName(getBaseContext(), sender);
                ttsWrapper.speak(getString(R.string.text_tts_broadcast_message, (sender != null) ? name : "", textmessage.szMessage));
            }
            break;
        case TextMsgType.MSGTYPE_USER :
            if (sounds.get(SOUND_USERMSG) != 0)
                audioIcons.play(sounds.get(SOUND_USERMSG), 1.0f, 1.0f, 0, 0, 1.0f);
            
            User sender = ttservice.getUsers().get(textmessage.nFromUserID);
            String name = Utils.getDisplayName(getBaseContext(), sender);
            String senderName = (sender != null) ? name : "";
            if (ttsWrapper != null && prefs.getBoolean("private_message_checkbox", false))
                ttsWrapper.speak(getString(R.string.text_tts_private_message, senderName, textmessage.szMessage));
            Intent action = new Intent(this, TextMessageActivity.class);
            Notification.Builder notification = new Notification.Builder(this);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                NotificationChannel mChannel = new NotificationChannel("TT_PM", "Teamtalk incoming message", NotificationManager.IMPORTANCE_HIGH);
                mChannel.enableVibration(false);
                mChannel.setVibrationPattern(null);
                mChannel.enableLights(false);
                mChannel.setSound(null, null);
                notificationManager.createNotificationChannel(mChannel);
            }
            notification.setSmallIcon(R.drawable.message)
                .setContentTitle(getString(R.string.private_message_notification, senderName))
                .setContentText(getString(R.string.private_message_notification_hint))
                .setContentIntent(PendingIntent.getActivity(this, textmessage.nFromUserID, action.putExtra(TextMessageActivity.EXTRA_USERID, textmessage.nFromUserID), 0))
                .setAutoCancel(true);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                notification.setChannelId("TT_PM");
			}
            notificationManager.notify(MESSAGE_NOTIFICATION_TAG, textmessage.nFromUserID, notification.build());
            break;
        case TextMsgType.MSGTYPE_CUSTOM:
        default:
            break;
        }
    }

    @Override
    public void onCmdChannelNew(Channel channel) {
        if (curchannel != null && curchannel.nChannelID == channel.nParentID) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
    }

    @Override
    public void onCmdChannelUpdate(Channel channel) {
        if (curchannel != null && curchannel.nChannelID == channel.nParentID) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }

        if(mychannel != null && mychannel.nChannelID == channel.nChannelID) {

            int myuserid = ttclient.getMyUserID();

            if(channel.transmitUsersQueue[0] == myuserid && mychannel.transmitUsersQueue[0] != myuserid) {
                if(sounds.get(SOUND_TXREADY) != 0) {
                    audioIcons.play(sounds.get(SOUND_TXREADY), 1.0f, 1.0f, 0, 0, 1.0f);
                }
            }
            if(mychannel.transmitUsersQueue[0] == myuserid && channel.transmitUsersQueue[0] != myuserid) {
                if(sounds.get(SOUND_TXSTOP) != 0) {
                    audioIcons.play(sounds.get(SOUND_TXSTOP), 1.0f, 1.0f, 0, 0, 1.0f);
                }
            }

            mychannel = channel;
        }
    }

    @Override
    public void onCmdChannelRemove(Channel channel) {
        if (curchannel != null && curchannel.nChannelID == channel.nParentID) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
    }

    @Override
    public void onCmdServerUpdate(ServerProperties serverproperties) {
    }

    @Override
    public void onCmdFileNew(RemoteFile remotefile) {
        filesAdapter.update();
        
        if(activecmds.size() == 0) {
            if(sounds.get(SOUND_FILESUPDATE) != 0) {
                audioIcons.play(sounds.get(SOUND_FILESUPDATE), 1.0f, 1.0f, 0, 0, 1.0f);
            }
        }
    }

    @Override
    public void onCmdFileRemove(RemoteFile remotefile) {
        filesAdapter.update();
        
        if(activecmds.size() == 0) {
            if(sounds.get(SOUND_FILESUPDATE) != 0) {
                audioIcons.play(sounds.get(SOUND_FILESUPDATE), 1.0f, 1.0f, 0, 0, 1.0f);
            }
        }
    }

    @Override
    public void onCmdUserAccount(UserAccount useraccount) {
    }

    @Override
    public void onCmdBannedUser(BannedUser banneduser) {
    }

    @Override
    public void onConnectSuccess() {
    }

    @Override
    public void onConnectFailed() {
    }

    @Override
    public void onConnectionLost() {
        if(sounds.get(SOUND_SERVERLOST) != 0) {
            audioIcons.play(sounds.get(SOUND_SERVERLOST), 1.0f, 1.0f, 0, 0, 1.0f);
        }
    }

    @Override
    public void onMaxPayloadUpdate(int payload_size) {
    }

    @Override
    public void onUserStateChange(User user) {
        
        if (curchannel != null && user.nChannelID == curchannel.nChannelID) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
        
    }

    @Override
    public void onUserVideoCapture(int nUserID, int nStreamID) {
    }

    @Override
    public void onUserMediaFileVideo(int nUserID, int nStreamID) {
    }

    @Override
    public void onUserDesktopWindow(int nUserID, int nStreamID) {

    }

    @Override
    public void onUserDesktopCursor(int nUserID, DesktopInput desktopinput) {
    }

    @Override
    public void onUserDesktopInput(int i, DesktopInput desktopInput) {

    }

    @Override
    public void onUserRecordMediaFile(int nUserID, MediaFileInfo mediafileinfo) {
    }

    @Override
    public void onUserAudioBlock(int nUserID, int nStreamType) {
    }

    @Override
    public void onVoiceTransmissionToggle(boolean voiceTransmissionEnabled, boolean isSuspended) {
        adjustTxState(voiceTransmissionEnabled);

        if (!isSuspended) {
            SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
            boolean ptt_vibrate = pref.getBoolean("vibrate_checkbox", true) &&
                Permissions.setupPermission(getBaseContext(), this, Permissions.MY_PERMISSIONS_REQUEST_VIBRATE);
            if (voiceTransmissionEnabled) {
                if (sounds.get(SOUND_VOICETXON) != 0) {
                    audioIcons.play(sounds.get(SOUND_VOICETXON), 1.0f, 1.0f, 0, 0, 1.0f);
                }
                if (ptt_vibrate) {
                    Vibrator vibrat = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
                    vibrat.vibrate(50);
                }
            } else {
                if (sounds.get(SOUND_VOICETXOFF) != 0) {
                    audioIcons.play(sounds.get(SOUND_VOICETXOFF), 1.0f, 1.0f, 0, 0, 1.0f);
                }
                if (ptt_vibrate) {
                    Vibrator vibrat = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
                    long pattern[] = { 0, 20, 80, 20 };
                    vibrat.vibrate(pattern, -1);
                }
            }
        }
    }

    @Override
    public void onVoiceActivationToggle(boolean voiceActivationEnabled, boolean isSuspended) {
        adjustVoxState(voiceActivationEnabled, voiceActivationEnabled ? ttclient.getVoiceActivationLevel() : ttclient.getSoundInputGainLevel());
    }

    @Override
    public void onInternalError(ClientErrorMsg clienterrormsg) {
    }

    @Override
    public void onVoiceActivation(boolean bVoiceActive) {
        adjustTxState(bVoiceActive);

        int sound = sounds.get(bVoiceActive ? SOUND_VOXON : SOUND_VOXOFF);
        if (sound != 0)
            audioIcons.play(sound, 1.0f, 1.0f, 0, 0, 1.0f);
    }

    @Override
    public void onHotKeyToggle(int nHotKeyID, boolean bActive) {
    }

    @Override
    public void onHotKeyTest(int nVkCode, boolean bActive) {
    }

    @Override
    public void onFileTransfer(FileTransfer filetransfer) {
    }

    @Override
    public void onDesktopWindowTransfer(int nSessionID, int nTransferRemaining) {
    }

    @Override
    public void onLocalMediaFile(MediaFileInfo mediaFileInfo) {

    }

    @Override
    public void onStreamMediaFile(MediaFileInfo mediafileinfo) {
    }

}
