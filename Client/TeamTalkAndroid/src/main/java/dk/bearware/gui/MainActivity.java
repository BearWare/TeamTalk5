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

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.SoundPool;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.Vibrator;
import android.preference.PreferenceManager;
import android.provider.OpenableColumns;
import android.text.InputType;
import android.util.Log;
import android.util.SparseArray;
import android.util.SparseIntArray;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
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
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.OptIn;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.NotificationCompat;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentPagerAdapter;
import androidx.fragment.app.ListFragment;
import androidx.viewpager.widget.ViewPager;

import com.google.android.material.tabs.TabLayout;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;
import java.util.Vector;

import dk.bearware.Channel;
import dk.bearware.ClientFlag;
import dk.bearware.ClientStatistics;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.SoundDeviceConstants;
import dk.bearware.SoundLevel;
import dk.bearware.Subscription;
import dk.bearware.TeamTalkBase;
import dk.bearware.TextMessage;
import dk.bearware.TextMsgType;
import dk.bearware.User;
import dk.bearware.UserAccount;
import dk.bearware.UserRight;
import dk.bearware.UserState;
import dk.bearware.backend.OnVoiceTransmissionToggleListener;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkConstants;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.data.FileListAdapter;
import dk.bearware.data.MediaAdapter;
import dk.bearware.data.MyTextMessage;
import dk.bearware.data.Permissions;
import dk.bearware.data.Preferences;
import dk.bearware.data.ServerEntry;
import dk.bearware.data.TTSWrapper;
import dk.bearware.data.TextMessageAdapter;
import dk.bearware.events.ClientEventListener;

public class MainActivity
extends AppCompatActivity
        implements TeamTalkConnectionListener,
        OnItemClickListener,
        OnItemLongClickListener,
        OnMenuItemClickListener,
        SensorEventListener,
        OnVoiceTransmissionToggleListener,
        ClientEventListener.OnConnectionLostListener,
        ClientEventListener.OnCmdProcessingListener,
        ClientEventListener.OnCmdMyselfLoggedInListener,
        ClientEventListener.OnCmdMyselfLoggedOutListener,
        ClientEventListener.OnCmdMyselfKickedFromChannelListener,
        ClientEventListener.OnCmdUserUpdateListener,
        ClientEventListener.OnCmdUserLeftChannelListener,
        ClientEventListener.OnCmdChannelNewListener,
        ClientEventListener.OnCmdUserTextMessageListener,
        ClientEventListener.OnCmdUserJoinedChannelListener,
        ClientEventListener.OnCmdChannelRemoveListener,
        ClientEventListener.OnCmdChannelUpdateListener,
        ClientEventListener.OnCmdUserLoggedOutListener,
        ClientEventListener.OnCmdUserLoggedInListener,
        ClientEventListener.OnCmdFileRemoveListener,
        ClientEventListener.OnUserStateChangeListener,
        ClientEventListener.OnVoiceActivationListener,
        ClientEventListener.OnCmdFileNewListener {

    SectionsPagerAdapter mSectionsPagerAdapter;

    /**
     * The {@link ViewPager} that will host the section contents.
     */
    ViewPager mViewPager;
    TabLayout mTabLayout;

    public static final String TAG = "bearware";

    private static final String MSG_NOTIFICATION_CHANNEL_ID = "TT_PM";

    public final int REQUEST_EDITCHANNEL = 1,
                     REQUEST_NEWCHANNEL = 2,
                     REQUEST_EDITUSER = 3,
                     REQUEST_SELECT_FILE = 4;

    // The channel currently being displayed
    Channel curchannel;
    // The channel we're currently in
    Channel mychannel;

    SparseArray<CmdComplete> activecmds = new SparseArray<>();

    ChannelListAdapter channelsAdapter;
    FileListAdapter filesAdapter;
    TextMessageAdapter textmsgAdapter;
    MediaAdapter mediaAdapter;
    TTSWrapper ttsWrapper = null;
    AccessibilityAssistant accessibilityAssistant;
    AudioManager audioManager;
    SoundPool audioIcons;
    NotificationManager notificationManager;
    WakeLock wakeLock, proximityWakeLock;
    boolean restarting;
    SensorManager mSensorManager;
    Sensor mSensor;
    Map<Integer, User> users = new HashMap<>();

    static final String MESSAGE_NOTIFICATION_TAG = "incoming_message";

    final int SOUND_VOICETXON = 1,
              SOUND_VOICETXOFF = 2,
              SOUND_USERMSG = 3,
              SOUND_CHANMSG = 4,
              SOUND_BCASTMSG = 5,
              SOUND_SERVERLOST = 6,
              SOUND_FILESUPDATE = 7,
              SOUND_VOXENABLE = 8,
              SOUND_VOXDISABLE = 9,
              SOUND_VOXON = 10,
              SOUND_VOXOFF = 11,
              SOUND_TXREADY = 12,
              SOUND_TXSTOP = 13,
              SOUND_USERJOIN = 14,
              SOUND_USERLEFT = 15,
              SOUND_USERLOGGEDIN = 16,
              SOUND_USERLOGGEDOFF = 17,
              SOUND_CHANMSGSENT = 18;
    
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
        ActionBar ab = getSupportActionBar();
        if (ab != null)
            ab.setDisplayHomeAsUpEnabled(true);

        mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);
        restarting = (savedInstanceState != null);
        accessibilityAssistant = new AccessibilityAssistant(this);
        audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        notificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        wakeLock = ((PowerManager)getSystemService(Context.POWER_SERVICE)).newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG + ":TeamTalk5");
         proximityWakeLock = ((PowerManager)getSystemService(Context.POWER_SERVICE)).newWakeLock(PowerManager.PROXIMITY_SCREEN_OFF_WAKE_LOCK, TAG + ":TeamTalk5");
        wakeLock.setReferenceCounted(false);
        proximityWakeLock.setReferenceCounted(false);

        channelsAdapter = new ChannelListAdapter(this.getBaseContext());
        filesAdapter = new FileListAdapter(this, this, accessibilityAssistant);
        textmsgAdapter = new TextMessageAdapter(this, accessibilityAssistant);
        mediaAdapter = new MediaAdapter(this.getBaseContext());
        
        // Create the adapter that will return a fragment for each of the five
        // primary sections of the app.
        mSectionsPagerAdapter = new SectionsPagerAdapter(getSupportFragmentManager());
        mTabLayout = findViewById(R.id.tab_layout);

        // Set up the ViewPager with the sections adapter.
        mViewPager = findViewById(R.id.pager);
        mViewPager.setAdapter(mSectionsPagerAdapter);
        mViewPager.addOnPageChangeListener(mSectionsPagerAdapter);
        mTabLayout.setupWithViewPager(mViewPager);

        setupButtons();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            final MediaPlayer mMediaPlayer;
            mMediaPlayer = MediaPlayer.create(getApplicationContext(), R.raw.silence);
            mMediaPlayer.setOnCompletionListener(mediaPlayer -> mMediaPlayer.release());
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
        UserAccount myuseraccount = new UserAccount();
        if (ttclient != null) {
            ttclient.getMyUserAccount(myuseraccount);
        }

        boolean uploadRight = (myuseraccount.uUserRights & UserRight.USERRIGHT_UPLOAD_FILES) != UserRight.USERRIGHT_NONE;
        boolean broadcastRight = (myuseraccount.uUserRights & UserRight.USERRIGHT_TEXTMESSAGE_BROADCAST) != UserRight.USERRIGHT_NONE;
        boolean isEditable = curchannel != null;
        boolean isJoinable = (ttclient != null) && (curchannel != null) && (ttclient.getMyChannelID() != curchannel.nChannelID) && (curchannel.nMaxUsers > 0);
        boolean isMyChannel = (ttclient != null) && (curchannel != null) && (ttclient.getMyChannelID() == curchannel.nChannelID);
        menu.findItem(R.id.action_edit).setEnabled(isEditable).setVisible(isEditable);
        menu.findItem(R.id.action_join).setEnabled(isJoinable).setVisible(isJoinable);
        menu.findItem(R.id.action_upload).setEnabled(uploadRight).setVisible(uploadRight);
        menu.findItem(R.id.action_broadcast).setEnabled(broadcastRight).setVisible(broadcastRight);
        menu.findItem(R.id.action_stream).setEnabled(isMyChannel).setVisible(isMyChannel);
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        AlertDialog.Builder alert = new AlertDialog.Builder(this);
        switch(item.getItemId()) {
            case R.id.action_join : {
                if (curchannel != null)
                    joinChannel(curchannel);
            }
            break;
            case R.id.action_upload : {
                if ((Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) ?
                    requestMediaPermissions() :
                    Permissions.READ_EXTERNAL_STORAGE.request(this)) {
                    fileSelectionStart();
                }
            }
            break;
            case R.id.action_broadcast:
                alert.setTitle(R.string.action_broadcast);
                alert.setMessage(R.string.text_broadcast_message);
                final EditText input = new EditText(this);
                input.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_MULTI_LINE);
                alert.setPositiveButton(android.R.string.yes, (dialog, whichButton) -> ttclient.doTextMessage(new TextMessage() {{ nMsgType = TextMsgType.MSGTYPE_BROADCAST; szMessage = input.getText().toString(); }}));
                alert.setNegativeButton(android.R.string.no, null);
                alert.setView(input);
                alert.show();
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
                int currentPage = mViewPager.getCurrentItem();
                Channel parentChannel = ((currentPage == SectionsPagerAdapter.CHANNELS_PAGE)
                                         && (curchannel != null)
                                         ) ?
                    ttservice.getChannels().get(curchannel.nParentID) :
                    null;
                if (currentPage != SectionsPagerAdapter.CHANNELS_PAGE) {
                    mViewPager.setCurrentItem(SectionsPagerAdapter.CHANNELS_PAGE);
                } else if ((curchannel != null)) {
                    setCurrentChannel(parentChannel);
                    channelsAdapter.notifyDataSetChanged();
                }
                else if (filesAdapter.getActiveTransfersCount() > 0) {
                    alert.setMessage(R.string.disconnect_alert);
                    alert.setPositiveButton(android.R.string.ok, (dialog, whichButton) -> {
                        filesAdapter.cancelAllTransfers();
                        finish();
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
            adjustSoundSystem(prefs);
            if (prefs.getBoolean(Preferences.PREF_SOUNDSYSTEM_BLUETOOTH_HEADSET, false)) {
                if (Permissions.BLUETOOTH.request(this))
                    ttservice.watchBluetoothHeadset();
            }
            else ttservice.unwatchBluetoothHeadset();

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

            adjustMuteButton(findViewById(R.id.speakerBtn));
            adjustVoxState(voxState, voxState ? voxlevel : gain);
            adjustTxState(txState);

            final SeekBar masterSeekBar = findViewById(R.id.master_volSeekBar);
            final SeekBar micSeekBar = findViewById(R.id.mic_gainSeekBar);
            masterSeekBar.setProgress(Utils.refVolumeToPercent(ttclient.getSoundOutputVolume()));
            if (ttservice.isVoiceActivationEnabled()) {
                micSeekBar.setProgress(ttclient.getVoiceActivationLevel());
            } else {
                micSeekBar.setProgress(Utils.refVolumeToPercent(ttclient.getSoundInputGainLevel()));
            }
            TextView volLevel = findViewById(R.id.vollevel_text);
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
        if (prefs.getBoolean("channel_message_sent_audio_icon", true)) {
            sounds.put(SOUND_CHANMSGSENT, audioIcons.load(getApplicationContext(), R.raw.channel_message_sent, 1));
        }
        if (prefs.getBoolean("broadcast_message_audio_icon", true)) {
            sounds.put(SOUND_BCASTMSG, audioIcons.load(getApplicationContext(), R.raw.broadcast_message, 1));
        }
        if (prefs.getBoolean("files_updated_audio_icon", true)) {
            sounds.put(SOUND_FILESUPDATE, audioIcons.load(getApplicationContext(), R.raw.fileupdate, 1));
        }
        if (prefs.getBoolean("voiceact_audio_icon", true)) {
            sounds.put(SOUND_VOXENABLE, audioIcons.load(getApplicationContext(), R.raw.voiceact_enable, 1));
            sounds.put(SOUND_VOXDISABLE, audioIcons.load(getApplicationContext(), R.raw.voiceact_disable, 1));
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
        if (prefs.getBoolean("userloggedin_icon", true)) {
            sounds.put(SOUND_USERLOGGEDIN, audioIcons.load(getApplicationContext(), R.raw.logged_on, 1));
        }
        if (prefs.getBoolean("userloggedoff_icon", true)) {
            sounds.put(SOUND_USERLOGGEDOFF, audioIcons.load(getApplicationContext(), R.raw.logged_off, 1));
        }

        getTextMessagesAdapter().showLogMessages(prefs.getBoolean("show_log_messages", true));

        getWindow().getDecorView().setKeepScreenOn(prefs.getBoolean("keep_screen_on_checkbox", false));

        createStatusTimer();
        ttsWrapper.useAnnouncements = prefs.getBoolean("pref_use_announcements", false);
        ttsWrapper.setAccessibilityStream(prefs.getBoolean("pref_a11y_volume", false));
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

            audioManager.setMode(AudioManager.MODE_NORMAL);

            // Unbind from the service
            if (mConnection.isBound()) {
                Log.d(TAG, "Unbinding TeamTalk service");
                onServiceDisconnected(ttservice);
                ttservice.disablePhoneCallReaction();
                ttservice.unwatchBluetoothHeadset();
                ttservice.resetState();
                unbindService(mConnection);
                mConnection.setBound(false);
            }
            notificationManager.cancelAll();
            mViewPager.removeOnPageChangeListener(mSectionsPagerAdapter);
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
            Uri uri = data.getData();
            String path = AbsolutePathHelper.getRealPath(this.getBaseContext(), uri);
            if (path != null) {
                File localFile = new File(path);
                if (localFile.canRead()) {
                    startFileUpload(path);
                } else {
                    Toast.makeText(this, getString(R.string.upload_failed, path), Toast.LENGTH_LONG).show();
                }
            } else {
                new FileCopyingTask().execute(uri);
            }
        } else {
            super.onActivityResult(requestCode, resultCode, data);
        }
    }

    private boolean startFileUpload(String path) {
        String remoteName = filesAdapter.getRemoteName(path);
        if (remoteName != null) {
            Toast.makeText(this, getString(R.string.remote_file_exists, remoteName), Toast.LENGTH_LONG).show();
        } else if (ttclient.doSendFile(curchannel.nChannelID, path) <= 0) {
            Toast.makeText(this, getString(R.string.upload_failed, path), Toast.LENGTH_LONG).show();
        } else {
            Toast.makeText(this, R.string.upload_started, Toast.LENGTH_SHORT).show();
            return true;
        }
        return false;
    }


    private class FileCopyingTask extends AsyncTask<Uri, Void, String> {

        @Override
        protected String doInBackground(Uri... uris) {
            Uri uri = uris[0];
            Cursor cursor = getContentResolver().query(uri, null, null, null, null);
            int columnIndex = ((cursor != null) && cursor.moveToFirst()) ? cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME) : -1;
            if (columnIndex >= 0) {
                File transitFile = new File(getCacheDir(), cursor.getString(columnIndex));
                cursor.close();
                try {
                    if (((!transitFile.exists()) || transitFile.delete()) && transitFile.createNewFile()) {
                        transitFile.deleteOnExit();
                    } else {
                        return null;
                    }
                } catch (Exception ex) {
                    return null;
                }
                try (InputStream src = getContentResolver().openInputStream(uri);
                     FileOutputStream dest = new FileOutputStream(transitFile)) {
                    byte[] buffer = new byte[1024];
                    int read;
                    while ((read = src.read(buffer)) > 0) {
                        dest.write(buffer, 0, read);
                    }
                } catch (Exception ex) {
                    return null;
                }
                return transitFile.getPath();
            } else if (cursor != null) {
                cursor.close();
            }
            return null;
        }

        @Override
        protected void onPostExecute(String path) {
            if ((path != null) && !startFileUpload(path)) {
                File transitFile = new File(path);
                transitFile.delete();
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
                proximityWakeLock.acquire();
                audioManager.setMode(AudioManager.MODE_IN_COMMUNICATION);
                audioManager.setSpeakerphoneOn(false);
                ttservice.enableVoiceTransmission(true);
            } else {
                proximityWakeLock.release();
                adjustSoundSystem(prefs);
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

    public class SectionsPagerAdapter extends FragmentPagerAdapter implements ViewPager.OnPageChangeListener {

        public static final int CHANNELS_PAGE   = 0,
                                CHAT_PAGE       = 1,
                                MEDIA_PAGE      = 2,
                                FILES_PAGE      = 3,

                                PAGE_COUNT      = 4;

        public SectionsPagerAdapter(FragmentManager fm) {
            super(fm, BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT);
        }

        @Override @NonNull
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


    private void fileSelectionStart() {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        Intent i = Intent.createChooser(intent, "File");
        startActivityForResult(i, REQUEST_SELECT_FILE);
    }

    private boolean requestMediaPermissions() {
        Permissions.READ_MEDIA_IMAGES.request(this, true);
        Permissions.READ_MEDIA_VIDEO.request(this, true);
        Permissions.READ_MEDIA_AUDIO.request(this, true);
        return areMediaPermissionsComplete();
    }

    private boolean areMediaPermissionsComplete() {
        return !(Permissions.READ_MEDIA_IMAGES.isPending() ||
                 Permissions.READ_MEDIA_VIDEO.isPending() ||
                 Permissions.READ_MEDIA_AUDIO.isPending());
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
            alert.setPositiveButton(android.R.string.ok, (dialog, whichButton) -> {
                filesAdapter.cancelAllTransfers();
                joinChannelUnsafe(channel, passwd);
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
            input.setInputType(InputType.TYPE_TEXT_VARIATION_PASSWORD|InputType.TYPE_CLASS_TEXT);
            input.setText(channel.szPassword);
            input.requestFocus();
            alert.setView(input);
            alert.setPositiveButton(android.R.string.ok, (dialog, whichButton) -> {
                InputMethodManager im = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                im.hideSoftInputFromWindow(input.getWindowToken(), 0);
                joinChannel(channel, input.getText().toString());
            });
            alert.setNegativeButton(android.R.string.cancel, (dialog, whichButton) -> {
                InputMethodManager im = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                im.hideSoftInputFromWindow(input.getWindowToken(), 0);
            });
			final AlertDialog dialog = alert.create();
            dialog.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
            dialog.show();
        }
        else {
            joinChannel(channel, "");
        }
    }

    // the channel currently being displayed
    private void setCurrentChannel(Channel channel) {
        curchannel = channel;
        ActionBar ab = getSupportActionBar();
        if (ab != null)
            ab.setSubtitle((channel != null) ? channel.szName : null);
        invalidateOptionsMenu();
    }

    // the channel we're currently in
    private void setMyChannel(Channel channel) {
        mychannel = channel;

        adjustVoiceGain();
    }

    private void subscriptionChange(User user) {
        User olduser = this.users.get(user.nUserID);
        if (olduser != null && this.ttsWrapper != null) {
            Utils.ttsSubscriptionChanged(getBaseContext(), olduser, user).ifPresent((text -> ttsWrapper.speak(text)));
        }
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
        public void onAttach(@NonNull Activity activity) {
            mainActivity = (MainActivity) activity;
            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_main_channels, container, false);
            mainActivity.accessibilityAssistant.registerPage(rootView, SectionsPagerAdapter.CHANNELS_PAGE);

            ListView channelsList = rootView.findViewById(R.id.listChannels);
            channelsList.setAdapter(mainActivity.getChannelsAdapter());
            channelsList.setOnItemClickListener(mainActivity);
            channelsList.setOnItemLongClickListener(mainActivity);

            return rootView;
        }
    }
    
    public static class ChatSectionFragment extends Fragment {
        MainActivity mainActivity;
        
private EditText newmsg;
        public ChatSectionFragment() {
        }
        
        @Override
        public void onAttach(@NonNull Activity activity) {
            mainActivity = (MainActivity) activity;
            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_main_chat, container, false);
            mainActivity.accessibilityAssistant.registerPage(rootView, SectionsPagerAdapter.CHAT_PAGE);
            newmsg = rootView.findViewById(R.id.channel_im_edittext);
            newmsg.setOnEditorActionListener((v, actionId, event) -> {
                if (actionId == EditorInfo.IME_ACTION_SEND || actionId == EditorInfo.IME_NULL) {
                    sendMsgToChannel();
                    return true;
                }
                return false;
            });
            ListView chatlog = rootView.findViewById(R.id.channel_im_listview);
            chatlog.setTranscriptMode(ListView.TRANSCRIPT_MODE_ALWAYS_SCROLL);
            chatlog.setAdapter(mainActivity.getTextMessagesAdapter());

            Button sendBtn = rootView.findViewById(R.id.channel_im_sendbtn);
            sendBtn.setOnClickListener(arg0 -> sendMsgToChannel());
            return rootView;
        }

        private void sendMsgToChannel() {
            String text = newmsg.getText().toString();
            if (text.isEmpty())
                return;

            MyTextMessage textmsg = new MyTextMessage();
            textmsg.nMsgType = TextMsgType.MSGTYPE_CHANNEL;
            textmsg.nChannelID = mainActivity.ttclient.getMyChannelID();
            textmsg.szMessage = text;

            int cmdid = 0;
            for (MyTextMessage m : textmsg.split()) {
                cmdid = mainActivity.ttclient.doTextMessage(m);
            }

            if (cmdid > 0) {
                mainActivity.activecmds.put(cmdid, CmdComplete.CMD_COMPLETE_TEXTMSG);
                newmsg.setText("");
            }
            else {
                Toast.makeText(mainActivity, getResources().getString(R.string.text_con_cmderr),
                Toast.LENGTH_LONG).show();
            }
        }

    }

    public static class VidcapSectionFragment extends Fragment {
        MainActivity mainActivity;

        public VidcapSectionFragment() {
        }

        @Override
        public void onAttach(@NonNull Activity activity) {
            mainActivity = (MainActivity) activity;
            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            return inflater.inflate(R.layout.fragment_main_vidcap, container, false);
        }
    }

    public static class MediaSectionFragment extends Fragment {
        MainActivity mainActivity;

        public MediaSectionFragment() {
        }

        @Override
        public void onAttach(@NonNull Activity activity) {
            mainActivity = (MainActivity) activity;
            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                                 Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_main_media, container, false);
            mainActivity.accessibilityAssistant.registerPage(rootView, SectionsPagerAdapter.MEDIA_PAGE);

            ExpandableListView mediaview = rootView.findViewById(R.id.media_elist_view);
            mediaview.setAdapter(mainActivity.getMediaAdapter());
            return rootView;
        }
    }

    public static class FilesSectionFragment extends ListFragment {

        @Override
        public void onViewCreated(@NonNull View view, Bundle savedInstanceState) {
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

        private final LayoutInflater inflater;

        Vector<Channel> subchannels = new Vector<>();
        Vector<Channel> stickychannels = new Vector<>();
        Vector<User> currentusers = new Vector<>();

        ChannelListAdapter(Context context) {
            inflater = LayoutInflater.from(context);
        }

        @Override
        public void notifyDataSetChanged() {
            subchannels.clear();
            stickychannels.clear();
            currentusers.clear();

            int chanid;
            if(curchannel != null) {
                chanid = curchannel.nChannelID;

                subchannels = Utils.getSubChannels(chanid, ttservice.getChannels());
                stickychannels = Utils.getStickyChannels(chanid, ttservice.getChannels());
                currentusers = Utils.getUsers(chanid, ttservice.getUsers());
            }
            else {
                chanid = ttclient.getRootChannelID();
                Channel root = ttservice.getChannels().get(chanid);
                if(root != null)
                    subchannels.add(root);
            }

            Collections.sort(subchannels, (c1, c2) -> c1.szName.compareToIgnoreCase(c2.szName));

            Collections.sort(stickychannels, (c1, c2) -> c1.szName.compareToIgnoreCase(c2.szName));

            Collections.sort(currentusers, (u1, u2) -> {
                if (((u1.uUserState & UserState.USERSTATE_VOICE) != 0) &&
                    ((u2.uUserState & UserState.USERSTATE_VOICE) == 0))
                    return -1;
                else if (((u1.uUserState & UserState.USERSTATE_VOICE) == 0) &&
                         ((u2.uUserState & UserState.USERSTATE_VOICE) != 0))
                    return 1;

                String name1 = Utils.getDisplayName(getBaseContext(), u1);
                String name2 = Utils.getDisplayName(getBaseContext(), u2);
                return name1.compareToIgnoreCase(name2);
            });

            super.notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            int count = currentusers.size() + subchannels.size() + stickychannels.size();
            if ((curchannel != null) && (curchannel.nParentID > 0)) {
                count++; // include parent channel shortcut
            }
            return count;
        }

        @Override
        public Object getItem(int position) {

            if (position < stickychannels.size()) {
                return stickychannels.get(position);
            }

            // sticky channels are first so subtract these
            position -= stickychannels.size();

            if (position < currentusers.size()) {
                return currentusers.get(position);
            }

            // users are first so subtract these
            position -= currentusers.size();

            if ((curchannel != null) && (curchannel.nParentID > 0)) {
                if(position == 0) {
                    Channel parent = ttservice.getChannels().get(curchannel.nParentID);

                    if(parent != null)
                        return parent;
                    return new Channel();
                }

                position--; // subtract parent channel shortcut
            }
            return subchannels.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public int getItemViewType(int position) {

            if (position < stickychannels.size())
                return INFO_VIEW_TYPE;

            // sticky channels are first so subtract these
            position -= stickychannels.size();

            if (position < currentusers.size())
                return USER_VIEW_TYPE;

            // users are first so subtract these
            position -= currentusers.size();

            if ((curchannel != null) && (curchannel.nParentID > 0)) {
                if (position == 0) {
                    return PARENT_CHANNEL_VIEW_TYPE;
                }

                position--; // subtract parent channel shortcut
            }

            return CHANNEL_VIEW_TYPE;
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

                switch (getItemViewType(position)) {
                    case PARENT_CHANNEL_VIEW_TYPE :
                        // show parent channel shortcut
                        if (convertView == null ||
                                convertView.findViewById(R.id.parentname) == null)
                            convertView = inflater.inflate(R.layout.item_channel_back, parent, false);
                        break;

                    case CHANNEL_VIEW_TYPE :
                        if (convertView == null ||
                                convertView.findViewById(R.id.channelname) == null)
                            convertView = inflater.inflate(R.layout.item_channel, parent, false);

                        ImageView chanicon = convertView.findViewById(R.id.channelicon);
                        TextView name = convertView.findViewById(R.id.channelname);
                        TextView topic = convertView.findViewById(R.id.chantopic);
                        Button join = convertView.findViewById(R.id.join_btn);
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

                        OnClickListener listener = v -> {
                            if (v.getId() == R.id.join_btn) {
                                joinChannel(channel);
                            }
                        };
                        join.setOnClickListener(listener);
                        join.setAccessibilityDelegate(accessibilityAssistant);
                        join.setEnabled(channel.nChannelID != ttclient.getMyChannelID());

                        if (channel.nMaxUsers > 0) {
                            int population = Utils.getUsers(channel.nChannelID, ttservice.getUsers()).size();
                            ((TextView)convertView.findViewById(R.id.population)).setText((population > 0) ? String.format("(%d)", population) : "");
                        }

                        break;

                    case INFO_VIEW_TYPE :
                        if (convertView == null ||
                                convertView.findViewById(R.id.titletext) == null)
                            convertView = inflater.inflate(R.layout.item_info, parent, false);
                        TextView title = convertView.findViewById(R.id.titletext);
                        TextView details = convertView.findViewById(R.id.infodetails);
                        title.setText(channel.szName);
                        details.setText(channel.szTopic);
                        break;
                }
            }
            else if(item instanceof User) {
                if (convertView == null ||
                    convertView.findViewById(R.id.nickname) == null)
                    convertView = inflater.inflate(R.layout.item_user, parent, false);
                ImageView usericon = convertView.findViewById(R.id.usericon);
                TextView nickname = convertView.findViewById(R.id.nickname);
                TextView status = convertView.findViewById(R.id.status);
                final User user = (User) item;
                String name = Utils.getDisplayName(getBaseContext(), user);
                nickname.setText(name);
                status.setText(user.szStatusMsg);
                
                boolean talking = (user.uUserState & UserState.USERSTATE_VOICE) != 0;
                boolean female = (user.nStatusMode & TeamTalkConstants.STATUSMODE_FEMALE) != 0;
                boolean neutral = (user.nStatusMode & TeamTalkConstants.STATUSMODE_NEUTRAL) != 0;
                boolean male = !female && !neutral;
                boolean away =  (user.nStatusMode & TeamTalkConstants.STATUSMODE_AWAY) != 0;
                int icon_resource;
                
                if(user.nUserID == ttservice.getTTInstance().getMyUserID()) {
                    talking = ttservice.isVoiceTransmitting();
                }
                if(talking) {
                    if(female) {
                        icon_resource = R.drawable.woman_green;
                        nickname.setContentDescription(getString(R.string.user_state_now_speaking, name) + " 👩");
                    }
                    else if(male) {
                        icon_resource = R.drawable.man_green;
                        nickname.setContentDescription(getString(R.string.user_state_now_speaking, name) + " 👨");
                    }
                    else {
                        icon_resource = R.drawable.man_green;
                        nickname.setContentDescription(getString(R.string.user_state_now_speaking, name));
                    }
                }
                else {
                    if(female) {
                        icon_resource = away? R.drawable.woman_orange : R.drawable.woman_blue;
                        nickname.setContentDescription(name + " 👩");
                    }
                    else if(male) {
                        icon_resource = away? R.drawable.man_orange : R.drawable.man_blue;
                        nickname.setContentDescription(name + " 👨");
                    }
                    else {
                        icon_resource = away? R.drawable.man_orange : R.drawable.man_blue;
                        nickname.setContentDescription(name);
                    }
                }
                status.setContentDescription(away ? getString(R.string.user_state_away) : null);

                usericon.setImageResource(icon_resource);
                usericon.setImportantForAccessibility(View.IMPORTANT_FOR_ACCESSIBILITY_NO);
                
                Button sndmsg = convertView.findViewById(R.id.msg_btn);
                OnClickListener listener = v -> {
                    if (v.getId() == R.id.msg_btn) {
                        Intent intent = new Intent(MainActivity.this, TextMessageActivity.class);
                        startActivity(intent.putExtra(TextMessageActivity.EXTRA_USERID, user.nUserID));
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
        
        final TextView connection = findViewById(R.id.connectionstat_textview);
        final TextView ping = findViewById(R.id.pingstat_textview);
        final TextView total = findViewById(R.id.totalstat_textview);
        final int defcolor = connection.getTextColors().getDefaultColor();
        
        if(stats_timer == null) {
            stats_timer = new CountDownTimer(10000, 1000) {
                
                ClientStatistics prev_stats;
                
                public void onTick(long millisUntilFinished) {
                
                    if (ttclient == null || accessibilityAssistant.isUiUpdateDiscouraged())
                        return;
                    filesAdapter.performPendingUpdate();

                    String con = getString(R.string.stat_offline);
                    int con_color = Color.RED;
                    int flags = ttclient.getFlags(); 
                    if ((flags & ClientFlag.CLIENT_CONNECTING) == ClientFlag.CLIENT_CONNECTING) {
                        con = getString(R.string.stat_connecting);
                    }
                    else if ((flags & ClientFlag.CLIENT_AUTHORIZED) == ClientFlag.CLIENT_CLOSED) {
                        // indicate 'offline' if not authorized
                        con = getString(R.string.stat_unauthorized);
                    }
                    else if ((flags & ClientFlag.CLIENT_AUTHORIZED) == ClientFlag.CLIENT_AUTHORIZED) {
                        con = getString(R.string.stat_online);
                        con_color = Color.GREEN;
                    }
                    else if((flags & ClientFlag.CLIENT_CONNECTING) == ClientFlag.CLIENT_CONNECTING) {
                        con = getString(R.string.stat_connecting);
                    }
                    
                    connection.setText(getString(R.string.label_connection) + " " + con);
                    connection.setTextColor(con_color);

                    ClientStatistics stats = new ClientStatistics();
                    if(!ttclient.getClientStatistics(stats))
                        return;
                    
                    if(prev_stats == null)
                        prev_stats = stats;
                    
                    long totalrx = stats.nUdpBytesRecv - prev_stats.nUdpBytesRecv;
                    long totaltx = stats.nUdpBytesSent - prev_stats.nUdpBytesSent;

                    String str;
                    if(stats.nUdpPingTimeMs >= 0) {
                        str = String.format("%1$d", stats.nUdpPingTimeMs); 
                        ping.setText(getString(R.string.label_ping) + " " + str);
                        
                        if(stats.nUdpPingTimeMs > 250) {
                            ping.setTextColor(Color.RED);
                        }
                        else {
                            ping.setTextColor(defcolor);
                        }
                    }                    
                    
                    str = String.format("%1$d/%2$d KB", totalrx/ 1024, totaltx / 1024);
                    total.setText(getString(R.string.label_rxtx) + " " + str);
                    
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
    }

    Channel selectedChannel;
    User selectedUser;
    List<Integer> userIDS = new ArrayList<>();

    @Override
    public boolean onItemLongClick(AdapterView< ? > l, View v, int position, long id) {
        Object item = channelsAdapter.getItem(position);
        if (item instanceof User) {
            selectedUser = (User) item;
            UserAccount myuseraccount = new UserAccount();
            ttclient.getMyUserAccount(myuseraccount);

            boolean banRight = (myuseraccount.uUserRights & UserRight.USERRIGHT_BAN_USERS) != UserRight.USERRIGHT_NONE;
            boolean moveRight = (myuseraccount.uUserRights & UserRight.USERRIGHT_MOVE_USERS) != UserRight.USERRIGHT_NONE;
            boolean kickRight = (myuseraccount.uUserRights & UserRight.USERRIGHT_KICK_USERS) != UserRight.USERRIGHT_NONE;
            // operator of a channel can also kick users
            int myuserid = ttclient.getMyUserID();
            boolean operatorRight = ttclient.isChannelOperator(myuserid, selectedUser.nChannelID);

            PopupMenu userActions = new PopupMenu(this, v);
            userActions.setOnMenuItemClickListener(this);
            userActions.inflate(R.menu.user_actions);
            userActions.getMenu().findItem(R.id.action_kickchan).setEnabled(kickRight | operatorRight).setVisible(kickRight | operatorRight);
            userActions.getMenu().findItem(R.id.action_kicksrv).setEnabled(kickRight).setVisible(kickRight);
            userActions.getMenu().findItem(R.id.action_banchan).setEnabled(banRight | operatorRight).setVisible(banRight | operatorRight);
            userActions.getMenu().findItem(R.id.action_bansrv).setEnabled(banRight).setVisible(banRight);
            userActions.getMenu().findItem(R.id.action_makeop).setTitle(ttclient.isChannelOperator(selectedUser.nUserID , selectedUser.nChannelID) ? R.string.action_revoke_operator : R.string.action_make_operator);
            userActions.getMenu().findItem(R.id.action_select).setTitle(userIDS.contains(selectedUser.nUserID) ? R.string.action_deselect : R.string.action_select);
            userActions.getMenu().findItem(R.id.action_select).setEnabled(moveRight).setVisible(moveRight);
            userActions.show();
            return true;
        }
        if (item instanceof Channel) {
            selectedChannel = (Channel) item;
            UserAccount myuseraccount = new UserAccount();
            ttclient.getMyUserAccount(myuseraccount);

            boolean moveRight = (myuseraccount.uUserRights & UserRight.USERRIGHT_MOVE_USERS) != UserRight.USERRIGHT_NONE;
            PopupMenu channelActions = new PopupMenu(this, v);
            channelActions.setOnMenuItemClickListener(this);
            channelActions.inflate(R.menu.channel_actions);
            channelActions.getMenu().findItem(R.id.action_move).setEnabled(moveRight && !userIDS.isEmpty()).setVisible(moveRight && !userIDS.isEmpty());
            channelActions.show();
            return true;
        }
        return false;
    }

    @Override
    public boolean onMenuItemClick(MenuItem item) {
        AlertDialog.Builder alert = new AlertDialog.Builder(this);
        switch (item.getItemId()) {
        case R.id.action_banchan:
            alert.setMessage(getString(R.string.ban_confirmation, selectedUser.szNickname));
            alert.setPositiveButton(android.R.string.yes, (dialog, whichButton) -> {
                ttclient.doBanUser(selectedUser.nUserID, selectedUser.nChannelID);
                ttclient.doKickUser(selectedUser.nUserID, selectedUser.nChannelID);
            });

            alert.setNegativeButton(android.R.string.no, null);
            alert.show();
            break;
        case R.id.action_bansrv:
            alert.setMessage(getString(R.string.ban_confirmation, selectedUser.szNickname));
            alert.setPositiveButton(android.R.string.yes, (dialog, whichButton) -> {
                ttclient.doBanUser(selectedUser.nUserID, 0);
                ttclient.doKickUser(selectedUser.nUserID, 0);
            });

            alert.setNegativeButton(android.R.string.no, null);
            alert.show();
            break;
        case R.id.action_edit:
            editChannelProperties(selectedChannel);
            break;
        case R.id.action_edituser: {
            Intent intent = new Intent(this, UserPropActivity.class);
            startActivityForResult(intent.putExtra(UserPropActivity.EXTRA_USERID, selectedUser.nUserID),
                                   REQUEST_EDITUSER);
        }
        break;
        case R.id.action_message: {
            Intent intent = new Intent(MainActivity.this, TextMessageActivity.class);
            startActivity(intent.putExtra(TextMessageActivity.EXTRA_USERID, selectedUser.nUserID));
        }
        break;
        case R.id.action_kickchan:
            alert.setMessage(getString(R.string.kick_confirmation, selectedUser.szNickname));
            alert.setPositiveButton(android.R.string.yes, (dialog, whichButton) -> ttclient.doKickUser(selectedUser.nUserID, selectedUser.nChannelID));

            alert.setNegativeButton(android.R.string.no, null);
            alert.show();
            break;
        case R.id.action_kicksrv:
            alert.setMessage(getString(R.string.kick_confirmation, selectedUser.szNickname));
            alert.setPositiveButton(android.R.string.yes, (dialog, whichButton) -> ttclient.doKickUser(selectedUser.nUserID, 0));

            alert.setNegativeButton(android.R.string.no, null);
            alert.show();
            break;
            case R.id.action_makeop:
                UserAccount myuseraccount = new UserAccount();
                ttclient.getMyUserAccount(myuseraccount);
                if ((myuseraccount.uUserRights & UserRight.USERRIGHT_OPERATOR_ENABLE) != UserRight.USERRIGHT_NONE) {
                    ttclient.doChannelOp(selectedUser.nUserID, selectedUser.nChannelID, ttclient.isChannelOperator(selectedUser.nUserID, selectedUser.nChannelID)? false: true);
                    break;
                }
                alert.setTitle(ttclient.isChannelOperator(selectedUser.nUserID , selectedUser.nChannelID) ? R.string.action_revoke_operator : R.string.action_make_operator);
                alert.setMessage(R.string.text_operator_password);
                final EditText input = new EditText(this);
                input.setInputType(InputType.TYPE_TEXT_VARIATION_PASSWORD | InputType.TYPE_CLASS_TEXT);
                alert.setPositiveButton(android.R.string.yes, ((dialog, whichButton) -> ttclient.doChannelOpEx(selectedUser.nUserID, selectedUser.nChannelID, input.getText().toString(), ttclient.isChannelOperator(selectedUser.nUserID, selectedUser.nChannelID)? false: true)));
                alert.setNegativeButton(android.R.string.no, null);
                alert.setView(input);
                alert.show();
                break;
        case R.id.action_move:
            for (Integer userID : userIDS) {
                ttclient.doMoveUser(userID, selectedChannel.nChannelID);
            }
            userIDS.clear();
            break;
        case R.id.action_select:
    if (userIDS.contains(selectedUser.nUserID)) {
        userIDS.remove((Integer) selectedUser.nUserID);
    } else {
        userIDS.add(selectedUser.nUserID);
    }
    break;
        case R.id.action_remove: {
            alert.setMessage(getString(R.string.channel_remove_confirmation, selectedChannel.szName));
            alert.setPositiveButton(android.R.string.yes, (dialog, whichButton) -> {
                if (ttclient.doRemoveChannel(selectedChannel.nChannelID) <= 0)
                    Toast.makeText(MainActivity.this,
                                   getString(R.string.err_channel_remove,
                                             selectedChannel.szName),
                                   Toast.LENGTH_LONG).show();
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

    @SuppressWarnings("deprecation")
    private void adjustSoundSystem(SharedPreferences prefs) {
        if (audioManager.isBluetoothA2dpOn())
            return;
        boolean voiceProcessing = prefs.getBoolean(Preferences.PREF_SOUNDSYSTEM_VOICEPROCESSING, false);
        audioManager.setMode(voiceProcessing ?
                AudioManager.MODE_IN_COMMUNICATION : AudioManager.MODE_NORMAL);
        if (voiceProcessing)
            audioManager.setSpeakerphoneOn(prefs.getBoolean(Preferences.PREF_SOUNDSYSTEM_SPEAKERPHONE, false) && !audioManager.isWiredHeadsetOn());
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
        ImageButton voxSwitch = findViewById(R.id.voxSwitch);
        TextView mikeLevel = findViewById(R.id.mikelevel_text);

        if (voiceActivationEnabled) {
            mikeLevel.setText(level + "%");
            mikeLevel.setContentDescription(getString(R.string.vox_level_description, mikeLevel.getText()));
            voxSwitch.setImageResource(R.drawable.microphone);
            voxSwitch.setContentDescription(getString(R.string.voice_activation_off));
            ((SeekBar) findViewById(R.id.mic_gainSeekBar)).setProgress(ttclient.getVoiceActivationLevel());
            findViewById(R.id.mic_gainSeekBar).setContentDescription(getString(R.string.voxlevel));
        }
        else {
            mikeLevel.setText(Utils.refVolumeToPercent(level) + "%");
            mikeLevel.setContentDescription(getString(R.string.mic_gain_description, mikeLevel.getText()));
            voxSwitch.setImageResource(R.drawable.mike_green);
            voxSwitch.setContentDescription(getString(R.string.voice_activation_on));
            ((SeekBar) findViewById(R.id.mic_gainSeekBar)).setProgress(Utils.refVolumeToPercent(ttclient.getSoundInputGainLevel()));
            findViewById(R.id.mic_gainSeekBar).setContentDescription(getString(R.string.micgain));
        }
    }

    private void adjustTxState(boolean txEnabled) {
        accessibilityAssistant.lockEvents();

        findViewById(R.id.transmit_voice).setBackgroundColor(txEnabled ? Color.GREEN : Color.RED);
        findViewById(R.id.transmit_voice).setContentDescription(txEnabled ? getString(R.string.tx_on) : getString(R.string.tx_off));

        if ((curchannel != null) && (ttclient.getMyChannelID() == curchannel.nChannelID))
            channelsAdapter.notifyDataSetChanged();

        accessibilityAssistant.unlockEvents();
    }

    private void adjustVoiceGain() {

        // if channel has audio configuration enabled then we should switch to AGC

        boolean showMicSeekBar = mychannel == null || !mychannel.audiocfg.bEnableAGC || ttservice == null || ttservice.isVoiceActivationEnabled();

        findViewById(R.id.mic_gainSeekBar).setVisibility(showMicSeekBar ? View.VISIBLE : View.GONE);
    }

    private interface OnButtonInteractionListener extends OnTouchListener, OnClickListener {
    }

    private void setupButtons() {

        final Button tx_btn = findViewById(R.id.transmit_voice);
        tx_btn.setAccessibilityDelegate(accessibilityAssistant);

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
        
        final SeekBar masterSeekBar = findViewById(R.id.master_volSeekBar);
        final SeekBar micSeekBar = findViewById(R.id.mic_gainSeekBar);
        final TextView mikeLevel = findViewById(R.id.mikelevel_text);
        final TextView volLevel = findViewById(R.id.vollevel_text);
        masterSeekBar.setMax(100);
        micSeekBar.setMax(100);

        SeekBar.OnSeekBarChangeListener volListener = new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
                if (seekBar == masterSeekBar) {
                    if (ttservice.isMute()) {
                        ttservice.setMute(false);
                        ImageButton speakerBtn = findViewById(R.id.speakerBtn);
                        adjustMuteButton(speakerBtn);
                    }
                    ttclient.setSoundOutputVolume(Utils.refVolume(progress));
                    volLevel.setText(progress + "%");
                    volLevel.setContentDescription(getString(R.string.speaker_volume_description, volLevel.getText()));
            }     else if (seekBar == micSeekBar) {
                    if (ttservice.isVoiceActivationEnabled()) {
                        ttclient.setVoiceActivationLevel(progress);
                        mikeLevel.setText(progress + "%");
                        mikeLevel.setContentDescription(getString(R.string.vox_level_description, mikeLevel.getText()));
                    } else {
                        ttclient.setSoundInputGainLevel(Utils.refGain(progress));
                        SharedPreferences.Editor editor = prefs.edit();
                        editor.putInt(Preferences.PREF_SOUNDSYSTEM_MICROPHONEGAIN, Utils.refGain(progress));
                        editor.apply();
                        mikeLevel.setText(progress + "%");
                        mikeLevel.setContentDescription(getString(R.string.mic_gain_description, mikeLevel.getText()));
                    }
                }
        }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        };

        masterSeekBar.setOnSeekBarChangeListener(volListener);
        micSeekBar.setOnSeekBarChangeListener(volListener);

        if ((Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) && accessibilityAssistant.isServiceActive()) {
            tx_btn.setOnClickListener(txButtonListener);
        }

        ImageButton speakerBtn = findViewById(R.id.speakerBtn);
        speakerBtn.setOnClickListener(v -> {
            if ((mConnection != null) && mConnection.isBound()) {
                ttservice.setMute(!ttservice.isMute());
                adjustMuteButton((ImageButton) v);

                int level = ttservice.isMute() ?
                    0 :
                    Utils.refVolumeToPercent(ttclient.getSoundOutputVolume());
                volLevel.setText(level + "%");
                volLevel.setContentDescription(getString(R.string.speaker_volume_description, volLevel.getText()));
            }
        });

        ImageButton voxSwitch = findViewById(R.id.voxSwitch);
        voxSwitch.setOnClickListener(v -> {
            if ((mConnection != null) && mConnection.isBound()) {
                if (ttservice.isVoiceTransmissionEnabled())
                    ttservice.enableVoiceTransmission(false);
                ttservice.enableVoiceActivation(!ttservice.isVoiceActivationEnabled());

                adjustVoiceGain();
            }
        });
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
        ttservice = service;
        ttclient = ttservice.getTTInstance();

        this.users = new HashMap<>(ttservice.getUsers());

        int mychanid = ttclient.getMyChannelID();
        if (mychanid > 0) {
            setCurrentChannel(ttservice.getChannels().get(mychanid));
        }

        setMyChannel(ttservice.getChannels().get(mychanid));

        mSectionsPagerAdapter.onPageSelected(mViewPager.getCurrentItem());

        channelsAdapter.notifyDataSetChanged();

        textmsgAdapter.setTextMessages(ttservice.getChatLogTextMsgs());
        textmsgAdapter.setMyUserID(ttclient.getMyUserID());
        textmsgAdapter.notifyDataSetChanged();

        mediaAdapter.setTeamTalkService(service);
        mediaAdapter.notifyDataSetChanged();

        filesAdapter.setTeamTalkService(service);
        filesAdapter.update(mychanid);

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
            if (Permissions.READ_PHONE_STATE.request(this))
                ttservice.enablePhoneCallReaction();
        }

        ttservice.getEventHandler().registerOnConnectionLostListener(this, true);
        ttservice.getEventHandler().registerOnCmdProcessing(this, true);
        ttservice.getEventHandler().registerOnCmdMyselfLoggedIn(this, true);
        ttservice.getEventHandler().registerOnCmdMyselfLoggedOut(this, true);
        ttservice.getEventHandler().registerOnCmdMyselfKickedFromChannel(this, true);
        ttservice.getEventHandler().registerOnCmdUserLoggedIn(this, true);
        ttservice.getEventHandler().registerOnCmdUserLoggedOut(this, true);
        ttservice.getEventHandler().registerOnCmdUserUpdate(this, true);
        ttservice.getEventHandler().registerOnCmdUserJoinedChannel(this, true);
        ttservice.getEventHandler().registerOnCmdUserLeftChannel(this, true);
        ttservice.getEventHandler().registerOnCmdUserTextMessage(this, true);
        ttservice.getEventHandler().registerOnCmdChannelNew(this, true);
        ttservice.getEventHandler().registerOnCmdChannelUpdate(this, true);
        ttservice.getEventHandler().registerOnCmdChannelRemove(this, true);
        ttservice.getEventHandler().registerOnCmdFileNew(this, true);
        ttservice.getEventHandler().registerOnCmdFileRemove(this, true);
        ttservice.getEventHandler().registerOnUserStateChange(this, true);
        ttservice.getEventHandler().registerOnVoiceActivation(this, true);

        ttservice.setOnVoiceTransmissionToggleListener(this);

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        adjustSoundSystem(prefs);

        if (prefs.getBoolean(Preferences.PREF_SOUNDSYSTEM_BLUETOOTH_HEADSET, false)
            && Permissions.BLUETOOTH.request(this))
            ttservice.watchBluetoothHeadset();

        if (Permissions.WAKE_LOCK.request(this))
            wakeLock.acquire();

        int mastervol = prefs.getInt(Preferences.PREF_SOUNDSYSTEM_MASTERVOLUME, SoundLevel.SOUND_VOLUME_DEFAULT);
        int voxlevel = prefs.getInt(Preferences.PREF_SOUNDSYSTEM_VOICEACTIVATION_LEVEL, 5);
        boolean voxState = ttservice.isVoiceActivationEnabled();
        boolean txState = ttservice.isVoiceTransmitting();

        // only set volume and gain if tt-instance hasn't already been configured
        if (ttclient.getSoundOutputVolume() != mastervol)
            ttclient.setSoundOutputVolume(mastervol);
        if (ttclient.getVoiceActivationLevel() != voxlevel)
            ttclient.setVoiceActivationLevel(voxlevel);

        adjustMuteButton(findViewById(R.id.speakerBtn));
        adjustVoxState(voxState, voxState ? voxlevel : ttclient.getSoundInputGainLevel());
        adjustTxState(txState);

        final SeekBar masterSeekBar = findViewById(R.id.master_volSeekBar);
        final SeekBar micSeekBar = findViewById(R.id.mic_gainSeekBar);
        masterSeekBar.setProgress(Utils.refVolumeToPercent(ttclient.getSoundOutputVolume()));
        if (ttservice.isVoiceActivationEnabled()) {
            micSeekBar.setProgress(ttclient.getVoiceActivationLevel());
        } else {
            micSeekBar.setProgress(Utils.refVolumeToPercent(ttclient.getSoundInputGainLevel()));
        }
        TextView volLevel = findViewById(R.id.vollevel_text);
        volLevel.setText(Utils.refVolumeToPercent(mastervol) + "%");
        volLevel.setContentDescription(getString(R.string.speaker_volume_description, volLevel.getText()));
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
        if (wakeLock.isHeld())
            wakeLock.release();
        service.setOnVoiceTransmissionToggleListener(null);

        ttservice.getEventHandler().unregisterListener(this);

        filesAdapter.setTeamTalkService(null);
        mediaAdapter.clearTeamTalkService(service);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        Permissions granted = Permissions.onRequestResult(this, requestCode, grantResults);
        if (granted == null) {
            granted = Permissions.fromRequestCode(requestCode);
            if ((granted != Permissions.READ_MEDIA_IMAGES) &&
                (granted != Permissions.READ_MEDIA_VIDEO) &&
                (granted != Permissions.READ_MEDIA_AUDIO))
                return;
        }
        switch (granted) {
            case READ_EXTERNAL_STORAGE:
            case READ_MEDIA_IMAGES:
            case READ_MEDIA_VIDEO:
            case READ_MEDIA_AUDIO:
                if ((Build.VERSION.SDK_INT < Build.VERSION_CODES.TIRAMISU) || areMediaPermissionsComplete())
                    fileSelectionStart();
                break;
            case WAKE_LOCK:
                wakeLock.acquire();
                break;
            case READ_PHONE_STATE:
                if ((mConnection != null) && mConnection.isBound())
                    ttservice.enablePhoneCallReaction();
                break;
            case BLUETOOTH:
                if ((mConnection != null) && mConnection.isBound())
                    ttservice.watchBluetoothHeadset();
                break;
            default:
                break;
        }
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
        users.put(user.nUserID, user);

        accessibilityAssistant.lockEvents();
        textmsgAdapter.notifyDataSetChanged();
        accessibilityAssistant.unlockEvents();

        if (sounds.get(SOUND_USERLOGGEDIN) != 0)
            audioIcons.play(sounds.get(SOUND_USERLOGGEDIN), 1.0f, 1.0f, 0, 0, 1.0f);
        if (ttsWrapper != null && PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("server_login_checkbox", false)) {
            String name = Utils.getDisplayName(getBaseContext(), user);
            ttsWrapper.speak(name + " " + getResources().getString(R.string.text_tts_loggedin));
        }
    }

    @Override
    public void onCmdUserLoggedOut(User user) {
        users.remove(user.nUserID);

        accessibilityAssistant.lockEvents();
        textmsgAdapter.notifyDataSetChanged();
        accessibilityAssistant.unlockEvents();

        if (sounds.get(SOUND_USERLOGGEDOFF) != 0)
            audioIcons.play(sounds.get(SOUND_USERLOGGEDOFF), 1.0f, 1.0f, 0, 0, 1.0f);
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

        subscriptionChange(user);

        users.put(user.nUserID, user);
    }

    @Override
    public void onCmdUserJoinedChannel(User user) {
        users.put(user.nUserID, user);
        
        if(user.nUserID == ttclient.getMyUserID()) {
            //myself joined channel
            Channel chan = ttservice.getChannels().get(user.nChannelID);
            setCurrentChannel(chan);
            filesAdapter.update(curchannel);

            setMyChannel(chan);

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
        users.put(user.nUserID, user);
        
        if(user.nUserID == ttclient.getMyUserID()) {
            //myself left current channel
            
            textmsgAdapter.notifyDataSetChanged();

            setCurrentChannel(null);
            setMyChannel(null);
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

            if (textmessage.nFromUserID != ttservice.getTTInstance().getMyUserID()) {
                if (sounds.get(SOUND_CHANMSG) != 0)
                    audioIcons.play(sounds.get(SOUND_CHANMSG), 1.0f, 1.0f, 0, 0, 1.0f);
                if (ttsWrapper != null && prefs.getBoolean("channel_message_checkbox", false)) {
                    User sender = ttservice.getUsers().get(textmessage.nFromUserID);
                    String name = Utils.getDisplayName(getBaseContext(), sender);
                    ttsWrapper.speak(getString(R.string.text_tts_channel_message, (sender != null) ? name : "", textmessage.szMessage));
                }
            }
            else if (textmessage.nFromUserID == ttservice.getTTInstance().getMyUserID()) {
                if (sounds.get(SOUND_CHANMSGSENT) != 0)
                    audioIcons.play(sounds.get(SOUND_CHANMSGSENT), 1.0f, 1.0f, 0, 0, 1.0f);
                if (ttsWrapper != null && prefs.getBoolean("channel_message_sent_checkbox", false)) {
                    ttsWrapper.speak(getString(R.string.text_tts_channel_message_sent, textmessage.szMessage));
                }
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
            Log.d(TAG, "Broadcast message in " + this.hashCode());
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
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                NotificationChannel mChannel = new NotificationChannel(MSG_NOTIFICATION_CHANNEL_ID, "Teamtalk incoming message", NotificationManager.IMPORTANCE_HIGH);
                mChannel.enableVibration(false);
                mChannel.setVibrationPattern(null);
                mChannel.enableLights(false);
                mChannel.setSound(null, null);
                notificationManager.createNotificationChannel(mChannel);
            }
            Notification notification = new NotificationCompat.Builder(this, MSG_NOTIFICATION_CHANNEL_ID)
                .setSmallIcon(R.drawable.message)
                .setContentTitle(getString(R.string.private_message_notification, senderName))
                .setContentText(getString(R.string.private_message_notification_hint))
                .setContentIntent(PendingIntent.getActivity(this, textmessage.nFromUserID, action.putExtra(TextMessageActivity.EXTRA_USERID, textmessage.nFromUserID), PendingIntent.FLAG_IMMUTABLE))
                .setAutoCancel(true)
                .build();
            notificationManager.notify(MESSAGE_NOTIFICATION_TAG, textmessage.nFromUserID, notification);
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

            if (ttsWrapper != null) {
                Utils.ttsTransmitUsersToggled(getBaseContext(), mychannel, channel, ttservice.getUsers()).ifPresent(text -> ttsWrapper.speak(text));
            }

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

            setMyChannel(channel);
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
    public void onCmdFileNew(RemoteFile remotefile) {
        filesAdapter.update();
        
        if(activecmds.size() == 0 && ttclient.getMyChannelID() == remotefile.nChannelID) {
            if(sounds.get(SOUND_FILESUPDATE) != 0) {
                audioIcons.play(sounds.get(SOUND_FILESUPDATE), 1.0f, 1.0f, 0, 0, 1.0f);
            }
        }
    }

    @Override
    public void onCmdFileRemove(RemoteFile remotefile) {
        filesAdapter.update();
        
        if(activecmds.size() == 0 && ttclient.getMyChannelID() == remotefile.nChannelID) {
            if(sounds.get(SOUND_FILESUPDATE) != 0) {
                audioIcons.play(sounds.get(SOUND_FILESUPDATE), 1.0f, 1.0f, 0, 0, 1.0f);
            }
        }
    }

    @Override
    public void onConnectionLost() {
        if(sounds.get(SOUND_SERVERLOST) != 0) {
            audioIcons.play(sounds.get(SOUND_SERVERLOST), 1.0f, 1.0f, 0, 0, 1.0f);
        }
    }

    @Override
    public void onUserStateChange(User user) {
        users.put(user.nUserID, user);
        
        if (curchannel != null && user.nChannelID == curchannel.nChannelID) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
        
    }

    @Override
    public void onVoiceTransmissionToggle(boolean voiceTransmissionEnabled, boolean isSuspended) {
        adjustTxState(voiceTransmissionEnabled);

        if (!isSuspended) {
            SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
            boolean ptt_vibrate = pref.getBoolean("vibrate_checkbox", true) &&
                Permissions.VIBRATE.request(this);
            if (voiceTransmissionEnabled) {
                accessibilityAssistant.shutUp();
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
                    long[] pattern = { 0, 20, 80, 20 };
                    vibrat.vibrate(pattern, -1);
                }
            }
        }
    }

    @Override
    public void onVoiceActivationToggle(boolean voiceActivationEnabled, boolean isSuspended) {
        adjustVoxState(voiceActivationEnabled, voiceActivationEnabled ? ttclient.getVoiceActivationLevel() : ttclient.getSoundInputGainLevel());
        if (voiceActivationEnabled) {
            if (sounds.get(SOUND_VOXENABLE) != 0) {
                audioIcons.play(sounds.get(SOUND_VOXENABLE), 1.0f, 1.0f, 0, 0, 1.0f);
            }
        } else {
            if (sounds.get(SOUND_VOXDISABLE) != 0) {
                audioIcons.play(sounds.get(SOUND_VOXDISABLE), 1.0f, 1.0f, 0, 0, 1.0f);
            }
        }
    }

    @Override
    public void onVoiceActivation(boolean bVoiceActive) {
        adjustTxState(bVoiceActive);

        int sound = sounds.get(bVoiceActive ? SOUND_VOXON : SOUND_VOXOFF);
        if (sound != 0)
            audioIcons.play(sound, 1.0f, 1.0f, 0, 0, 1.0f);
    }
}
