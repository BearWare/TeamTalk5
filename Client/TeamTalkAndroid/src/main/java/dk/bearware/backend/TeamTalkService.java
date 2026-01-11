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

package dk.bearware.backend;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ServiceInfo;
import android.media.AudioManager;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.Build;
import android.os.CountDownTimer;
import android.os.Handler;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.support.v4.media.session.MediaSessionCompat;
import android.support.v4.media.session.PlaybackStateCompat;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.util.SparseArray;
import android.view.KeyEvent;
import android.widget.Toast;

import androidx.core.app.NotificationCompat;
import androidx.core.app.ServiceCompat;

import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringReader;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

import dk.bearware.AudioPreprocessor;
import dk.bearware.AudioPreprocessorType;
import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.ClientEvent;
import dk.bearware.ClientFlag;
import dk.bearware.EncryptionContext;
import dk.bearware.FileTransfer;
import dk.bearware.FileTransferStatus;
import dk.bearware.MediaFileInfo;
import dk.bearware.MediaFileStatus;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.SoundDeviceConstants;
import dk.bearware.SoundLevel;
import dk.bearware.StreamType;
import dk.bearware.Subscription;
import dk.bearware.TeamTalk5;
import dk.bearware.TeamTalkBase;
import dk.bearware.TextMessage;
import dk.bearware.TextMsgType;
import dk.bearware.User;
import dk.bearware.UserAccount;
import dk.bearware.UserRight;
import dk.bearware.WebRTCConstants;
import dk.bearware.data.AppInfo;
import dk.bearware.data.License;
import dk.bearware.data.MyTextMessage;
import dk.bearware.data.Preferences;
import dk.bearware.data.ServerEntry;
import dk.bearware.data.UserCached;
import dk.bearware.events.ClientEventListener;
import dk.bearware.events.TeamTalkEventHandler;
import dk.bearware.gui.CmdComplete;
import dk.bearware.gui.MainActivity;
import dk.bearware.gui.MediaButtonEventReceiver;
import dk.bearware.gui.R;
import dk.bearware.gui.Utils;
import dk.bearware.utils.DnsUtils;

import static dk.bearware.gui.CmdComplete.CMD_COMPLETE_NONE;

public class TeamTalkService extends Service
        implements BluetoothHeadsetHelper.HeadsetConnectionListener,
        ClientEventListener.OnConnectSuccessListener,
        ClientEventListener.OnConnectFailedListener,
        ClientEventListener.OnConnectionLostListener,
        ClientEventListener.OnEncryptionErrorListener,
        ClientEventListener.OnCmdSuccessListener,
        ClientEventListener.OnCmdProcessingListener,
        ClientEventListener.OnCmdMyselfLoggedInListener,
        ClientEventListener.OnCmdMyselfKickedFromChannelListener,
        ClientEventListener.OnCmdErrorListener,
        ClientEventListener.OnCmdUserLoggedInListener,
        ClientEventListener.OnCmdUserLoggedOutListener,
        ClientEventListener.OnCmdUserUpdateListener,
        ClientEventListener.OnCmdUserJoinedChannelListener,
        ClientEventListener.OnCmdUserLeftChannelListener,
        ClientEventListener.OnCmdUserTextMessageListener,
        ClientEventListener.OnCmdChannelNewListener,
        ClientEventListener.OnCmdChannelRemoveListener,
        ClientEventListener.OnCmdServerUpdateListener,
        ClientEventListener.OnCmdChannelUpdateListener,
        ClientEventListener.OnCmdFileNewListener,
        ClientEventListener.OnCmdFileRemoveListener,
        ClientEventListener.OnUserStateChangeListener,
        ClientEventListener.OnVoiceActivationListener,
        ClientEventListener.OnFileTransferListener,
        ClientEventListener.OnStreamMediaFileListener {

    public static final String CANCEL_TRANSFER = "cancel_transfer";

    public static final String TAG = "bearware";

    private static final int UI_WIDGET_ID = 1;
    private static final String UI_CHANNEL_ID = "TeamtalkConnection";

    // Binder given to clients
    private final IBinder mBinder = new LocalBinder();

    private BluetoothHeadsetHelper bluetoothHeadsetHelper;
    private TelephonyManager telephonyManager;
    OnVoiceTransmissionToggleListener onVoiceTransmissionToggleListener;
    private boolean listeningPhoneStateChanges;
    private boolean txSuspended;
    private boolean voxSuspended;
    private boolean permanentMuteState;
    private boolean currentMuteState;
    private Notification widget = null;
    private NotificationManager notificationManager;
    private volatile boolean inPhoneCall;
    private MediaSessionCompat mediaSession;
    Handler reconnectHandler = new Handler();
    Runnable reconnectTimer = this::reconnect;

    TeamTalkBase ttclient;
    ServerEntry ttserver;
    Channel joinchannel, /* the channel to join after login */
            mychannel; /* the channel 'ttclient' is currently in */
    private final TeamTalkEventHandler mEventHandler = new TeamTalkEventHandler();
    CountDownTimer eventTimer;
    SparseArray<CmdComplete> activecmds = new SparseArray<>();

    Map<Integer, Channel> channels = new HashMap<>();
    Map<Integer, RemoteFile> remoteFiles = new HashMap<>();
    Map<Integer, FileTransfer> fileTransfers = new HashMap<>();
    Map<Integer, User> users = new HashMap<>();
    Map<Integer, Vector<MyTextMessage>> usertxtmsgs = new HashMap<>();
    Vector<MyTextMessage> chatlogtxtmsgs = new Vector<>();
    Map<String, UserCached> usercache = new HashMap<>();

    public void resetState() {
        reconnectHandler.removeCallbacks(reconnectTimer);
        disablePhoneCallReaction();

        syncToUserCache();

        if(ttclient != null)
            ttclient.disconnect();

        displayNotification(false);
        joinchannel = null;
        setMyChannel(null);
        activecmds.clear();
        channels.clear();
        remoteFiles.clear();
        fileTransfers.clear();
        users.clear();
        usertxtmsgs.clear();
        chatlogtxtmsgs.clear();
    }

    public Map<Integer, Channel> getChannels() {
        return channels;
    }
    public Map<Integer, RemoteFile> getRemoteFiles() {
        return remoteFiles;
    }
    public Map<Integer, FileTransfer> getFileTransfers() {
        return fileTransfers;
    }
    public Map<Integer, User> getUsers() {
        return users;
    }

    private final MediaSessionCompat.Callback mMediaSessionCallback = new MediaSessionCompat.Callback() {

        @Override
        public boolean onMediaButtonEvent(Intent mediaButtonEvent) {
            super.onMediaButtonEvent(mediaButtonEvent);
            final String intentAction = mediaButtonEvent.getAction();
            if (Intent.ACTION_MEDIA_BUTTON.equals(intentAction)) {
                final KeyEvent event = mediaButtonEvent.getParcelableExtra(Intent.EXTRA_KEY_EVENT);
                if (event == null) {
                    return false;
                }
                final int keycode = event.getKeyCode();
                final int action = event.getAction();
                if (event.getRepeatCount() == 0 && action == KeyEvent.ACTION_DOWN) {
                    switch (keycode) {
                        case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
                        case KeyEvent.KEYCODE_MEDIA_PAUSE:
                        case KeyEvent.KEYCODE_MEDIA_PLAY:
                        case KeyEvent.KEYCODE_HEADSETHOOK:
                            if (isVoiceActivationEnabled())
                                enableVoiceActivation(false);
                            else
                                enableVoiceTransmission(!isVoiceTransmissionEnabled());
                            break;
                    }
                    return true;
                }
            }
            return false;
        }
    };


    public class LocalBinder extends Binder {
        public TeamTalkService getService() {
            // Return this instance of LocalService so clients can call public methods
            return TeamTalkService.this;
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();

        // make sure DLL is loaded 
        TeamTalk5.loadLibrary();

        TeamTalk5.setLicenseInformation(License.REGISTRATION_NAME, License.REGISTRATION_KEY);

        ttclient = new TeamTalk5();
        telephonyManager = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        listeningPhoneStateChanges = false;
        txSuspended = false;
        voxSuspended = false;
        permanentMuteState = false;
        currentMuteState = false;
        inPhoneCall = false;

        //register self as event handler so 'users' and 'channels' can be updated
        mEventHandler.registerOnConnectSuccessListener(this, true);
        mEventHandler.registerOnConnectFailedListener(this, true);
        mEventHandler.registerOnConnectionLostListener(this, true);
        mEventHandler.registerOnEncryptionErrorListener(this, true);

        mEventHandler.registerOnCmdError(this, true);
        mEventHandler.registerOnCmdSuccess(this, true);
        mEventHandler.registerOnCmdProcessing(this, true);
        mEventHandler.registerOnCmdMyselfLoggedIn(this, true);
        mEventHandler.registerOnCmdMyselfKickedFromChannel(this, true);
        mEventHandler.registerOnCmdUserLoggedIn(this,true);
        mEventHandler.registerOnCmdUserLoggedOut(this, true);
        mEventHandler.registerOnCmdUserUpdate(this, true);
        mEventHandler.registerOnCmdUserJoinedChannel(this, true);
        mEventHandler.registerOnCmdUserLeftChannel(this, true);
        mEventHandler.registerOnCmdUserTextMessage(this, true);
        mEventHandler.registerOnCmdChannelNew(this, true);
        mEventHandler.registerOnCmdChannelUpdate(this, true);
        mEventHandler.registerOnCmdChannelRemove(this, true);
        mEventHandler.registerOnCmdServerUpdate(this, true);
        mEventHandler.registerOnCmdFileNew(this, true);
        mEventHandler.registerOnCmdFileRemove(this, true);

        mEventHandler.registerOnUserStateChange(this, true);

        mEventHandler.registerOnVoiceActivation(this, true);
        mEventHandler.registerOnFileTransfer(this, true);
        mEventHandler.registerOnStreamMediaFile(this, true);

        //create timer to process 'mEventHandler'
        createEventTimer();

        bluetoothHeadsetHelper = new BluetoothHeadsetHelper(this);

        ComponentName receiver = new ComponentName(getPackageName(), MediaButtonEventReceiver.class.getName());
        //mediaSession = new MediaSessionCompat(this, "MediaService", receiver, null);
        mediaSession = new MediaSessionCompat(this, "TeamTalkService");
        mediaSession.setFlags(MediaSessionCompat.FLAG_HANDLES_MEDIA_BUTTONS |
                MediaSessionCompat.FLAG_HANDLES_TRANSPORT_CONTROLS);
        mediaSession.setPlaybackState(new PlaybackStateCompat.Builder()
                .setState(PlaybackStateCompat.STATE_PAUSED, 0, 0)
                .setActions(PlaybackStateCompat.ACTION_PLAY_PAUSE)
                .build());
        mediaSession.setCallback(mMediaSessionCallback);

        AudioManager audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        audioManager.requestAudioFocus(focusChange -> {
            // Ignore
        }, AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN);
        mediaSession.setActive(true);
        Log.d(TAG, "Created TeamTalk 5 service");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if ((intent != null) && intent.hasExtra(CANCEL_TRANSFER)) {
            int transferId = intent.getIntExtra(CANCEL_TRANSFER, 0);
            if ((ttclient != null) && ttclient.cancelFileTransfer(transferId)) {
                fileTransfers.remove(transferId);
                Toast.makeText(this, R.string.transfer_stopped, Toast.LENGTH_LONG).show();
            }
        }
        if (mediaSession.getController().getPlaybackState().getState() == PlaybackStateCompat.STATE_PLAYING) {
            mediaSession.setPlaybackState(new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_PAUSED, 0, 0.0f)
                    .setActions(PlaybackStateCompat.ACTION_PLAY_PAUSE).build());
        } else {
            mediaSession.setPlaybackState(new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_PLAYING, 0, 1.0f)
                    .setActions(PlaybackStateCompat.ACTION_PLAY_PAUSE).build());
        }
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public void onDestroy() {
        eventTimer.cancel();

        mEventHandler.unregisterListener(this);
        disablePhoneCallReaction();
        unwatchBluetoothHeadset();

        if (ttclient != null)
            ttclient.closeTeamTalk();

        super.onDestroy();
        mediaSession.release();

        Log.d(TAG, "Destroyed TeamTalk 5 service");
    }

    private String getNotificationText() {
        return (mychannel != null) ?
            String.format("%s / %s", ttserver.servername, mychannel.szName) :
            ttserver.servername;
    }

    @SuppressLint("NewApi")
    private void displayNotification(boolean enabled) {
        if (enabled) {
            if (widget == null) {
                notificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
                Intent ui = new Intent(this, MainActivity.class);
                ui.setFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                    NotificationChannel mChannel = new NotificationChannel(UI_CHANNEL_ID, "Teamtalk connection", NotificationManager.IMPORTANCE_DEFAULT);
                    mChannel.enableVibration(false);
                    mChannel.setVibrationPattern(null);
                    mChannel.enableLights(false);
                    mChannel.setSound(null, null);
                    notificationManager.createNotificationChannel(mChannel);
                }
                int intentFlags = PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE;
                widget = new NotificationCompat.Builder(this, UI_CHANNEL_ID)
                    .setSmallIcon(R.drawable.teamtalk_green)
                    .setContentTitle(getString(R.string.app_name))
                    .setContentIntent(PendingIntent.getActivity(this, 0, ui, intentFlags))
                    .setOngoing(true)
                    .setAutoCancel(false)
                    .setContentText(getNotificationText())
                    .setShowWhen(false)
                    .build();
                ServiceCompat.startForeground(this, UI_WIDGET_ID, widget, ServiceInfo.FOREGROUND_SERVICE_TYPE_MANIFEST);
            } else {
                widget = new NotificationCompat.Builder(this, widget)
                    .setContentText(getNotificationText())
                    .build();
                notificationManager.notify(UI_WIDGET_ID, widget);
            }
        } else if (widget != null) {
            ServiceCompat.stopForeground(this, ServiceCompat.STOP_FOREGROUND_REMOVE);
            widget = null;
        }
    }

    private void adjustMuteOnTx(boolean txEnabled) {
        if (PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getBoolean(Preferences.PREF_SOUNDSYSTEM_MUTE_ON_TRANSMISSION, false)) {
            boolean isMuted = isMute();
            if ((txEnabled && !isMuted) || (isMuted && !txEnabled && !permanentMuteState))
                ttclient.setSoundOutputMute(txEnabled);
        }
    }

    private final PhoneStateListener phoneStateListener = new PhoneStateListener() {
            int myStatus = 0;

            @Override
            public void onCallStateChanged(int state, String incomingNumber) {
                User myself = users.get(ttclient.getMyUserID());
                if (myself == null) // event may have been generated before ttclient.disconnect() was called
                    return;

                switch (state) {
                case TelephonyManager.CALL_STATE_IDLE:
                    if (voxSuspended)
                        enableVoiceActivation(true);
                    else if (txSuspended)
                        enableVoiceTransmission(true);
                    setMute(permanentMuteState);
                    if ((myself != null) && ((myStatus & TeamTalkConstants.STATUSMODE_AWAY) == 0))
                        ttclient.doChangeStatus(myself.nStatusMode & ~TeamTalkConstants.STATUSMODE_AWAY, myself.szStatusMsg);
                    inPhoneCall = false;
                    break;
                case TelephonyManager.CALL_STATE_RINGING:
                    inPhoneCall = true;
                    if (!isMute()) {
                        ttclient.setSoundOutputMute(true);
                        currentMuteState = true;
                    }
                    if (isVoiceActivationEnabled()) {
                        voxSuspended = true;
                        enableVoiceActivation(false);
                    }
                    else if (isVoiceTransmissionEnabled()) {
                        txSuspended = true;
                        enableVoiceTransmission(false);
                    }
                    myStatus = myself.nStatusMode;
                    if ((myStatus & TeamTalkConstants.STATUSMODE_AWAY) == 0)
                        ttclient.doChangeStatus(myStatus | TeamTalkConstants.STATUSMODE_AWAY, myself.szStatusMsg);
                    break;
                default:
                    break;
                }
            }
        };

    public void enablePhoneCallReaction() {
        txSuspended = false;
        voxSuspended = false;
        inPhoneCall = false;
        telephonyManager.listen(phoneStateListener, PhoneStateListener.LISTEN_CALL_STATE);
        listeningPhoneStateChanges = true;
    }

    public void disablePhoneCallReaction() {
        if (listeningPhoneStateChanges) {
            telephonyManager.listen(phoneStateListener, PhoneStateListener.LISTEN_NONE);
            listeningPhoneStateChanges = false;
        }
        txSuspended = false;
        voxSuspended = false;
        inPhoneCall = false;
    }

    public boolean isInPhoneCall() {
        return inPhoneCall;
    }

    public void watchBluetoothHeadset() {
        if (bluetoothHeadsetHelper.start()) {
            if (bluetoothHeadsetHelper.isHeadsetConnected())
                bluetoothHeadsetHelper.scoAudioConnect();
            bluetoothHeadsetHelper.registerHeadsetConnectionListener(this);
        }
    }

    public void unwatchBluetoothHeadset() {
        bluetoothHeadsetHelper.unregisterHeadsetConnectionListener(this);
        bluetoothHeadsetHelper.stop();
    }

    private void setMyChannel(Channel chan) {
        this.mychannel = chan;

        setupAudioPreprocessor();
    }

    public TeamTalkBase getTTInstance() {
        return ttclient;
    }

    public TeamTalkEventHandler getEventHandler() { return mEventHandler; }

    public ServerEntry getServerEntry() {
        return ttserver;
    }

    // set TT server which service should connect to
    public void setServerEntry(ServerEntry entry) {
        ttserver = entry;
    }

    // set channel which service should join
    public void setJoinChannel(Channel channel) {
        joinchannel = channel;
    }

    public void setOnVoiceTransmissionToggleListener(OnVoiceTransmissionToggleListener listener) {
        onVoiceTransmissionToggleListener = listener;
    }

    public boolean getCurrentMuteState() {
        return currentMuteState;
    }

    public boolean isMute() {
        return ((ttclient.getFlags() & ClientFlag.CLIENT_SNDOUTPUT_MUTE) != 0);
    }

    public boolean isVoiceTransmissionEnabled() {
        return (ttclient.getFlags() & ClientFlag.CLIENT_TX_VOICE) != 0;
    }

    public boolean isVoiceTransmitting() {
        final int voiceActivationMask = ClientFlag.CLIENT_SNDINPUT_VOICEACTIVATED | ClientFlag.CLIENT_SNDINPUT_VOICEACTIVE;
        int flags = ttclient.getFlags();
        return ((flags & ClientFlag.CLIENT_TX_VOICE) != 0) ||
            ((flags & voiceActivationMask) == voiceActivationMask);
    }

    public boolean isVoiceActivationEnabled() {
        return (ttclient.getFlags() & (ClientFlag.CLIENT_SNDINPUT_VOICEACTIVATED | ClientFlag.CLIENT_SNDINPUT_VOICEACTIVE)) != 0;
    }

    public void setMute(boolean state) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());

        permanentMuteState = state;
        currentMuteState = state;
        if ((isMute() != permanentMuteState) &&
            !(prefs.getBoolean(Preferences.PREF_SOUNDSYSTEM_MUTE_ON_TRANSMISSION, false) && isVoiceTransmitting()))
            ttclient.setSoundOutputMute(permanentMuteState);
    }

    public void enableVoiceTransmission(boolean enable) {
        if (enable) {
            txSuspended = false;
            voxSuspended = false;
            int indevid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT;
            if (((ttclient.getFlags() & ClientFlag.CLIENT_SNDINPUT_READY) != 0) || ttclient.initSoundInputDevice(indevid))
                ttclient.enableVoiceTransmission(true);
        }
        else {
            ttclient.enableVoiceTransmission(false);
            ttclient.closeSoundInputDevice();
        }
        adjustMuteOnTx(enable);
    }

    public void enableVoiceActivation(boolean enable) {
        if (enable) {
            txSuspended = false;
            voxSuspended = false;
            int indevid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT;
            if (((ttclient.getFlags() & ClientFlag.CLIENT_SNDINPUT_READY) != 0) || ttclient.initSoundInputDevice(indevid))
                ttclient.enableVoiceActivation(true);
        }
        else {
            ttclient.enableVoiceActivation(false);
            ttclient.closeSoundInputDevice();
        }
        adjustMuteOnTx(enable);
    }

    public void syncToUserCache(User user) {
        String cacheid = UserCached.getCacheID(user);
        if (!cacheid.isEmpty()) {
            usercache.put(cacheid, new UserCached(user));
        }
    }

    public void syncToUserCache() {
        // sync user settings to cache
        for (Map.Entry<Integer, User> entry : users.entrySet()) {
            syncToUserCache(entry.getValue());
        }
    }

    public void syncFromUserCache(User user) {
        String cacheid = UserCached.getCacheID(user);
        if (cacheid.isEmpty())
            return;

        UserCached userprop = usercache.get(cacheid);
        if (userprop != null) {
            userprop.sync(ttclient, user);
        }
    }

    public boolean reconnect() {
        if(ttserver == null || ttclient == null)
            return false;

        syncToUserCache();

        ttclient.disconnect();

        if (!setupEncryption())
            return false;

        String[] connectHosts = DnsUtils.resolveHostCandidatesForConnect(this, ttserver.ipaddr);
        boolean ok = false;
        String chosen = (connectHosts.length > 0) ? connectHosts[0] : ttserver.ipaddr;
        for (String host : connectHosts) {
            chosen = host;
            ok = ttclient.connect(host, ttserver.tcpport,
                                  ttserver.udpport, 0, 0, ttserver.encrypted);
            if (ok)
                break;
        }
        Log.i(TAG, "connect() to " + chosen + ":" + ttserver.tcpport +
                   " udp " + ttserver.udpport + " enc=" + ttserver.encrypted +
                   " -> " + ok);
        if(!ok) {
            ttclient.disconnect();
            return false;
        }
        
        return true;
    }

    private boolean setupEncryption() {
        if (!this.ttserver.encrypted)
            return true;

        File outputDir = getBaseContext().getCacheDir();
        try {
            File cacertfile = File.createTempFile("cacert", "pem", outputDir);
            File clientcertfile = File.createTempFile("clientcert", "pem", outputDir);
            File clientkeyfile = File.createTempFile("clientkey", "pem", outputDir);
            try (FileWriter cawriter = new FileWriter(cacertfile);
                 FileWriter certwriter = new FileWriter(clientcertfile);
                 FileWriter keywriter = new FileWriter(clientkeyfile)) {
                cawriter.write(this.ttserver.cacert);
                certwriter.write(this.ttserver.clientcert);
                keywriter.write(this.ttserver.clientcertkey);
            }
            EncryptionContext context = new EncryptionContext();
            if (!this.ttserver.cacert.isEmpty())
                context.szCAFile = cacertfile.getAbsolutePath();
            if (!this.ttserver.clientcert.isEmpty())
                context.szCertificateFile = clientcertfile.getAbsolutePath();
            if (!this.ttserver.clientcertkey.isEmpty())
                context.szPrivateKeyFile = clientkeyfile.getAbsolutePath();
            context.bVerifyPeer = ttserver.verifypeer;
            if (!context.bVerifyPeer) {
                context.nVerifyDepth = -1;
            }
            return ttclient.setEncryptionContext(context);
        } catch (IOException e) {
            return false;
        }
    }

    public int HISTORY_CHATLOG_MSG_MAX = 100;
    public int HISTORY_USER_MSG_MAX = 100;

    public Vector<MyTextMessage> getUserTextMsgs(int userid) {
        Vector<MyTextMessage> msgs;
        if(usertxtmsgs.get(userid) == null) {
            msgs = new Vector<>();
            usertxtmsgs.put(userid, msgs);
        }
        msgs = usertxtmsgs.get(userid);
        if(msgs.size() > HISTORY_USER_MSG_MAX)
            msgs.remove(0);
        return msgs;
    }

    public Vector<MyTextMessage> getChatLogTextMsgs() {
        if(chatlogtxtmsgs.size()>HISTORY_CHATLOG_MSG_MAX)
            chatlogtxtmsgs.remove(0);

        return chatlogtxtmsgs;
    }

    void createEventTimer() {
        eventTimer = new CountDownTimer(10000, 100) {
            private boolean prevVoiceTransmissionState = isVoiceTransmissionEnabled();
            private boolean prevVoiceActivationState = isVoiceActivationEnabled();

            public void onTick(long millisUntilFinished) {
                while(mEventHandler.processEvent(ttclient, 0));
                boolean newVoiceTransmissionState = isVoiceTransmissionEnabled();
                boolean newVoiceActivationState = isVoiceActivationEnabled();
                if (onVoiceTransmissionToggleListener != null) {
                    if (newVoiceTransmissionState != prevVoiceTransmissionState) {
                        onVoiceTransmissionToggleListener.onVoiceTransmissionToggle(newVoiceTransmissionState, txSuspended);
                        prevVoiceTransmissionState = newVoiceTransmissionState;
                    }
                    if (newVoiceActivationState != prevVoiceActivationState) {
                        onVoiceTransmissionToggleListener.onVoiceActivationToggle(newVoiceActivationState, voxSuspended);
                        prevVoiceActivationState = newVoiceActivationState;
                    }
                }
            }

            public void onFinish() {
                start();
            }
        };
        eventTimer.start();
    }

    void createReconnectTimer(long delayMsec) {
        
        reconnectHandler.removeCallbacks(reconnectTimer);
        reconnectHandler.postDelayed(reconnectTimer, delayMsec);
    }

    private void login() {

        String nickname = ttserver.nickname;
        if (TextUtils.isEmpty(nickname)) {
            nickname = PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getString(Preferences.PREF_GENERAL_NICKNAME, "");
        }

        int loginCmdId = ttclient.doLoginEx(nickname, ttserver.username, ttserver.password, AppInfo.APPNAME_SHORT);
        if(loginCmdId<0) {
            Toast.makeText(this, getResources().getString(R.string.text_cmderr_login),
                    Toast.LENGTH_LONG).show();
        }
        else {
            activecmds.put(loginCmdId, CmdComplete.CMD_COMPLETE_LOGIN);
        }

        MyTextMessage msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
                getResources().getString(R.string.text_con_success));
        getChatLogTextMsgs().add(msg);
    }

    private void loginComplete() {
        if (joinchannel == null) {

            // join channel specified in ServerEntry
            if (ttserver.channel != null && !ttserver.channel.isEmpty()) {
                int chanid = ttclient.getChannelIDFromPath(ttserver.channel);
                joinchannel = getChannels().get(chanid);
                if (joinchannel != null) {
                    joinchannel.szPassword = ttserver.chanpasswd;
                }
            }

            // if last channel is not set then join initial channel
            UserAccount useraccount = new UserAccount();
            ttclient.getMyUserAccount(useraccount);
            if (joinchannel == null && !useraccount.szInitChannel.isEmpty()) {
                int chanid = ttclient.getChannelIDFromPath(useraccount.szInitChannel);
                joinchannel = getChannels().get(chanid);
            }

            // otherwise join root channel
            boolean joinroot = PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getBoolean(Preferences.PREF_JOIN_ROOT_CHAN, true);
            if (joinroot && joinchannel == null) {
                joinchannel = getChannels().get(ttclient.getRootChannelID());
                if (joinchannel != null) {
                    joinchannel.szPassword = ttserver.chanpasswd;
                }
            }
        }

        if(joinchannel != null) {
            int cmdid = ttclient.doJoinChannel(joinchannel);
            activecmds.put(cmdid, CmdComplete.CMD_COMPLETE_JOIN);
        }
    }

    private void setupAudioPreprocessor() {
        if (mychannel != null && mychannel.audiocfg.bEnableAGC) {
            AudioPreprocessor ap = new AudioPreprocessor(AudioPreprocessorType.WEBRTC_AUDIOPREPROCESSOR, true);
            ap.webrtc.gaincontroller2.bEnable = true;
            float gainPercent = mychannel.audiocfg.nGainLevel / (float)TeamTalkConstants.CHANNEL_AUDIOCONFIG_MAX;
            ap.webrtc.gaincontroller2.fixeddigital.fGainDB = WebRTCConstants.WEBRTC_GAINCONTROLLER2_FIXEDGAIN_MAX * gainPercent;
            ttclient.setSoundInputPreprocess(ap);
            ttclient.setSoundInputGainLevel(SoundLevel.SOUND_GAIN_DEFAULT);
        }
        else {
            ttclient.setSoundInputPreprocess(new AudioPreprocessor());
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
            int gain = prefs.getInt(Preferences.PREF_SOUNDSYSTEM_MICROPHONEGAIN, SoundLevel.SOUND_GAIN_DEFAULT);
            ttclient.setSoundInputGainLevel(gain);
        }
    }

    @Override
    public void onConnectSuccess() {
        
        assert (ttserver != null);

        if (Utils.isWebLogin(ttserver.username)) {
            new WebLoginAccessToken().execute();
        }
        else {
            login();
        }
    }

    @Override
    public void onEncryptionError(int opensslErrorNo, ClientErrorMsg errmsg) {
        Log.i(TAG, "Encryption error: " + errmsg.szErrorMsg + " connecting to " + ttserver.ipaddr + ":" + ttserver.tcpport);
        Toast.makeText(this, getResources().getString(R.string.text_con_encryption_error, errmsg.szErrorMsg),
                       Toast.LENGTH_LONG).show();
    }    

    @Override
    public void onConnectFailed() {
        
        Log.i(TAG, "Failed to connect " + ttserver.ipaddr + ":" + ttserver.tcpport);
        
        Toast.makeText(this, getResources().getString(R.string.text_con_failed),
                       Toast.LENGTH_SHORT).show();
        
        createReconnectTimer(5000);
    }

    @Override
    public void onConnectionLost() {
        
        Log.i(TAG, "Connection lost to " + ttserver.ipaddr + ":" + ttserver.tcpport);
        
        activecmds.clear();
        
        Toast.makeText(this, getResources().getString(R.string.text_con_lost),
                       Toast.LENGTH_LONG).show();

        createReconnectTimer(5000);
        
        MyTextMessage msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_ERROR,
            getResources().getString(R.string.text_con_lost));
        getChatLogTextMsgs().add(msg);
    }

    @Override
    public void onCmdError(int cmdId, ClientErrorMsg errmsg) {
        
        Utils.notifyError(this, errmsg);
        
        if(activecmds.get(cmdId) == CmdComplete.CMD_COMPLETE_LOGIN) {
            
            //don't try to reconnect if we get a server login error
            reconnectHandler.removeCallbacks(reconnectTimer);
        }
    }

    @Override
    public void onCmdSuccess(int cmdId) {
        if(activecmds.get(cmdId) == CmdComplete.CMD_COMPLETE_LOGIN) {
            
            //stop reconnect timer since we're now connected and logged in
            reconnectHandler.removeCallbacks(reconnectTimer);

            //update status bar widget
            displayNotification(true);
        }
    }

    @Override
    public void onCmdProcessing(int cmdId, boolean complete) {

        if (!complete) {
            switch (activecmds.get(cmdId, CMD_COMPLETE_NONE)) {
                case CMD_COMPLETE_LOGIN:
                    //new users and channels will be posted for new login, so delete old ones
                    users.clear();
                    remoteFiles.clear();
                    fileTransfers.clear();
                    channels.clear();
                    break;
            }
        }
        else {
            switch (activecmds.get(cmdId, CMD_COMPLETE_NONE)) {
                case CMD_COMPLETE_LOGIN : {
                    loginComplete();
                }
                break;
            }
            activecmds.delete(cmdId);
        }
    }

    @Override
    public void onCmdMyselfLoggedIn(int my_userid, UserAccount useraccount) {
        MyTextMessage msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
            getResources().getString(R.string.text_cmd_loggedin));
        getChatLogTextMsgs().add(msg);

        // check whether to switch to female icon and put status message per server
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        int statusmode = TeamTalkConstants.STATUSMODE_AVAILABLE;
        String statusmsg = ttserver.statusmsg;

        if (TextUtils.isEmpty(statusmsg))
        {
            statusmsg = prefs.getString(Preferences.PREF_GENERAL_STATUSMSG, "");
        }

        if (prefs.getBoolean(Preferences.PREF_GENERAL_GENDER, false))
            statusmode |= TeamTalkConstants.STATUSMODE_FEMALE;

        ttclient.doChangeStatus(statusmode, statusmsg);
    }

    @Override
    public void onCmdMyselfKickedFromChannel() {
    }

    @Override
    public void onCmdMyselfKickedFromChannel(User kicker) {
        users.put(kicker.nUserID, kicker);
    }

    @Override
    public void onCmdUserLoggedIn(User user) {
        users.put(user.nUserID, user);

        SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        int def_unsub = Subscription.SUBSCRIBE_NONE;
        if(!pref.getBoolean(Preferences.PREF_SUB_TEXTMESSAGE, true))
            def_unsub |= Subscription.SUBSCRIBE_USER_MSG;
        if(!pref.getBoolean(Preferences.PREF_SUB_CHANMESSAGE, true))
            def_unsub |= Subscription.SUBSCRIBE_CHANNEL_MSG;
        if(!pref.getBoolean(Preferences.PREF_SUB_BCAST_MESSAGES, true))
            def_unsub |= Subscription.SUBSCRIBE_BROADCAST_MSG;
        if(!pref.getBoolean(Preferences.PREF_SUB_VOICE, true))
            def_unsub |= Subscription.SUBSCRIBE_VOICE;
        if(!pref.getBoolean(Preferences.PREF_SUB_VIDCAP, true))
            def_unsub |= Subscription.SUBSCRIBE_VIDEOCAPTURE;
        if(!pref.getBoolean(Preferences.PREF_SUB_DESKTOP, true))
            def_unsub |= Subscription.SUBSCRIBE_DESKTOP;
        if(!pref.getBoolean(Preferences.PREF_SUB_MEDIAFILE, true))
            def_unsub |= Subscription.SUBSCRIBE_MEDIAFILE;

        if((user.uLocalSubscriptions & def_unsub) != 0) {
            int cmdid = ttclient.doUnsubscribe(user.nUserID, def_unsub);
            if(cmdid > 0)
                activecmds.put(cmdid, CmdComplete.CMD_COMPLETE_UNSUBSCRIBE);
        }

        String name = Utils.getDisplayName(getBaseContext(), user);
        MyTextMessage msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
            name + " " + getResources().getString(R.string.text_cmd_userloggedin));
        getChatLogTextMsgs().add(msg);

        // sync weblogin user settings from cache
        syncFromUserCache(user);
    }

    @Override
    public void onCmdUserLoggedOut(User user) {
        users.remove(user.nUserID);

        String name = Utils.getDisplayName(getBaseContext(), user);
        MyTextMessage msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
            name + " " + getResources().getString(R.string.text_cmd_userloggedout));
        getChatLogTextMsgs().add(msg);

        // sync user settings to cache
        syncToUserCache(user);
    }

    @Override
    public void onCmdUserUpdate(User user) {
        users.put(user.nUserID, user);
    }

    @Override
    public void onCmdUserJoinedChannel(User user) {
        users.put(user.nUserID, user);        
        if (ttserver.rememberLastChannel && (user.nUserID == ttclient.getMyUserID()) && (joinchannel != null)) {
            ttserver.channel = ttclient.getChannelPath(joinchannel.nChannelID);
            ttserver.chanpasswd = joinchannel.szPassword;
        }
        
        if(user.nUserID == ttclient.getMyUserID()) {
            //myself joined channel
            setMyChannel(getChannels().get(user.nChannelID));
            displayNotification(true);

            MyTextMessage msg;
            if (mychannel.nParentID == 0) {
                msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
                    getResources().getString(R.string.text_cmd_joinroot));
            }
            else {
                msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
                    getResources().getString(R.string.text_cmd_joinchan) + " " + mychannel.szName);
            }
            getChatLogTextMsgs().add(msg);
        }
        else if (mychannel != null && mychannel.nChannelID == user.nChannelID) {
            //other user joined current channel
            
            String name = Utils.getDisplayName(getBaseContext(), user);
            MyTextMessage msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
                name + " " + getResources().getString(R.string.text_cmd_userjoinchan));
            getChatLogTextMsgs().add(msg);
        }
        
        // set media file volume
        SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        int mf_volume = pref.getInt(Preferences.PREF_SOUNDSYSTEM_MEDIAFILE_VOLUME, 50);
        mf_volume = Utils.refVolume(mf_volume);
        ttclient.setUserVolume(user.nUserID, StreamType.STREAMTYPE_MEDIAFILE_AUDIO, mf_volume);
        ttclient.pumpMessage(ClientEvent.CLIENTEVENT_USER_STATECHANGE, user.nUserID);

        // sync user settings from cache
        if (!UserCached.getCacheID(user).isEmpty()) {
            UserAccount myaccount = new UserAccount();
            if (ttclient.getMyUserAccount(myaccount) && (myaccount.uUserRights & UserRight.USERRIGHT_VIEW_ALL_USERS) == UserRight.USERRIGHT_NONE) {
                // sync weblogin user settings from cache
                syncFromUserCache(user);
            }
        }
    }

    @Override
    public void onCmdUserLeftChannel(int channelid, User user) {
        users.put(user.nUserID, user);
        
        if (mychannel != null && mychannel.nChannelID == channelid) {
            
            Channel chan = getChannels().get(channelid);
            MyTextMessage msg;
            if(user.nUserID == ttclient.getMyUserID()) {
                // myself left channel
                if(chan.nParentID == 0) {
                    msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
                        getResources().getString(R.string.text_cmd_leftroot));
                }
                else {
                    msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
                        getResources().getString(R.string.text_cmd_leftchan) + " " + chan.szName);
                }
            }
            else {
                // other user left current channel
                String name = Utils.getDisplayName(getBaseContext(), user);
                msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
                    name + " " + getResources().getString(R.string.text_cmd_userleftchan));                
            }
            getChatLogTextMsgs().add(msg);
        }
        
        if(user.nUserID == ttclient.getMyUserID()) {
            setMyChannel(null);
        }

        // sync user settings to cache
        String cacheid = UserCached.getCacheID(user);
        if (!cacheid.isEmpty()) {
            UserAccount myaccount = new UserAccount();
            if (ttclient.getMyUserAccount(myaccount) && (myaccount.uUserRights & UserRight.USERRIGHT_VIEW_ALL_USERS) == UserRight.USERRIGHT_NONE) {
                syncToUserCache(user);
            }
        }
    }

    @Override
    public void onCmdUserTextMessage(TextMessage textmessage) {

        User user = getUsers().get(textmessage.nFromUserID);
        MyTextMessage newmsg = new MyTextMessage(textmessage, 
                                                 user == null? "" : Utils.getDisplayName(getBaseContext(), user));

        switch(textmessage.nMsgType) {
            case TextMsgType.MSGTYPE_USER : {
                getUserTextMsgs(textmessage.nFromUserID).add(newmsg);
                break;
            }
            case TextMsgType.MSGTYPE_BROADCAST : {
                getChatLogTextMsgs().add(newmsg);
                break;
            }
            case TextMsgType.MSGTYPE_CHANNEL : {
                getChatLogTextMsgs().add(newmsg);
                break;
            }
        }
    }

    @Override
    public void onCmdChannelNew(Channel channel) {
        channels.put(channel.nChannelID, channel);
    }

    @Override
    public void onCmdChannelUpdate(Channel channel) {
        channels.put(channel.nChannelID, channel);

        if (mychannel != null && mychannel.nChannelID == channel.nChannelID) {
            setMyChannel(channel);
        }
    }

    @Override
    public void onCmdChannelRemove(Channel channel) {
        channels.remove(channel.nChannelID);
    }

    @Override
    public void onCmdServerUpdate(ServerProperties serverproperties) {
        MyTextMessage msg;
        msg = MyTextMessage.createUserDefMsg(MyTextMessage.MSGTYPE_SERVERPROP,
                                             serverproperties);
        getChatLogTextMsgs().add(msg);
    }

    @Override
    public void onCmdFileNew(RemoteFile remotefile) {
        remoteFiles.put(remotefile.nFileID, remotefile);
    }

    @Override
    public void onCmdFileRemove(RemoteFile remotefile) {
        remoteFiles.remove(remotefile.nFileID);
    }
    
    @Override
    public void onUserStateChange(User user) {
        users.put(user.nUserID, user);
    }


    @Override
    public void onVoiceActivation(boolean bVoiceActive) {
        adjustMuteOnTx(bVoiceActive);
    }

    @Override
    public void onFileTransfer(FileTransfer transfer) {
        if (transfer.nStatus == FileTransferStatus.FILETRANSFER_ACTIVE) {
            fileTransfers.put(transfer.nTransferID, transfer);
        }
        else {
            fileTransfers.remove(transfer.nTransferID);
        }
    }

    @Override
    public void onStreamMediaFile(MediaFileInfo mediafileinfo) {
        User myself = users.get(ttclient.getMyUserID());
        if (myself == null) // event may have been generated before ttclient.disconnect() was called
            return;

        switch (mediafileinfo.nStatus) {
            case MediaFileStatus.MFS_STARTED :
                ttclient.doChangeStatus(myself.nStatusMode | TeamTalkConstants.STATUSMODE_STREAM_MEDIAFILE, myself.szStatusMsg);
                break;
            case MediaFileStatus.MFS_ERROR:
            case MediaFileStatus.MFS_ABORTED:
            case MediaFileStatus.MFS_FINISHED:
                ttclient.doChangeStatus(myself.nStatusMode & ~TeamTalkConstants.STATUSMODE_STREAM_MEDIAFILE, myself.szStatusMsg);
                break;
            case MediaFileStatus.MFS_PLAYING :
            case MediaFileStatus.MFS_PAUSED :
            case MediaFileStatus.MFS_CLOSED :
            default :
                break;
        }
    }

    @Override
    public void onHeadsetConnected() {
        bluetoothHeadsetHelper.scoAudioConnect();
    }

    @Override
    public void onHeadsetDisconnected() {
        bluetoothHeadsetHelper.scoAudioDisconnect();
    }


    class WebLoginAccessToken extends AsyncTask<Void, Void, Void> {

        String username = "", token = "", accesstoken = "";

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());

            this.username = prefs.getString(Preferences.PREF_GENERAL_BEARWARE_USERNAME, "");
            this.token = prefs.getString(Preferences.PREF_GENERAL_BEARWARE_TOKEN, "");

            ServerProperties srvprop = new ServerProperties();
            if (ttclient.getServerProperties(srvprop))
                accesstoken = srvprop.szAccessToken;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            if (username.length() > 0) {
                ttserver.username = this.username;
                login();
            }
            else {
                Toast.makeText(TeamTalkService.this, getResources().getString(R.string.text_weblogin_authfailure),
                        Toast.LENGTH_LONG).show();
            }
        }

        @Override
        protected Void doInBackground(Void... voids) {
            String xml = Utils.getURL(AppInfo.getBearWareAccessTokenUrl(getBaseContext(),
                    this.username, this.token, accesstoken));
            Log.d(AppInfo.TAG, xml);

            try {
                InputSource src = new InputSource(new StringReader(xml));
                DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
                DocumentBuilder db = dbf.newDocumentBuilder();
                Document document = db.parse(src);
                XPathFactory factory = XPathFactory.newInstance();
                XPath xPath = factory.newXPath();

                this.username = (String)xPath.evaluate("/teamtalk/bearware/username", document, XPathConstants.STRING);

            } catch (XPathExpressionException e) {
                Log.e(AppInfo.TAG, "XPath failed: " + e);
            } catch (ParserConfigurationException e) {
                Log.e(AppInfo.TAG, "Parser cfg failed: " + e);
            } catch (IOException e) {
                Log.e(AppInfo.TAG, "XML IOException: " + e);
            } catch (SAXException e) {
                Log.e(AppInfo.TAG, "XML SAXException: " + e);
            }

            return null;
        }
    }
}
