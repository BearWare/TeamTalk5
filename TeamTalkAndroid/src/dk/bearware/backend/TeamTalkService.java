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

package dk.bearware.backend;

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import dk.bearware.BannedUser;
import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.ClientFlag;
import dk.bearware.DesktopInput;
import dk.bearware.FileTransfer;
import dk.bearware.FileTransferStatus;
import dk.bearware.MediaFileInfo;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.Subscription;
import dk.bearware.TeamTalk5;
import dk.bearware.TeamTalkBase;
import dk.bearware.TextMessage;
import dk.bearware.TextMsgType;
import dk.bearware.User;
import dk.bearware.UserAccount;
import dk.bearware.data.MyTextMessage;
import dk.bearware.data.ServerEntry;
import dk.bearware.events.ClientListener;
import dk.bearware.events.CommandListener;
import dk.bearware.events.ConnectionListener;
import dk.bearware.events.TeamTalkEventHandler;
import dk.bearware.events.UserListener;
import dk.bearware.gui.CmdComplete;
import dk.bearware.gui.R;
import dk.bearware.gui.Utils;
import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Binder;
import android.os.CountDownTimer;
import android.os.Handler;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.util.Log;
import android.util.SparseArray;
import android.widget.Toast;

public class TeamTalkService extends Service
implements CommandListener, UserListener, ConnectionListener, ClientListener {

    public static final String CANCEL_TRANSFER = "cancel_transfer";

    public static final String TAG = "bearware";
    
    // Binder given to clients
    private final IBinder mBinder = new LocalBinder();

    TeamTalkEventHandler mEventHandler = new TeamTalkEventHandler();

    public class LocalBinder extends Binder {
        public TeamTalkService getService() {
            // Return this instance of LocalService so clients can call public methods
            return TeamTalkService.this;
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();

        ttclient = new TeamTalk5();
        
        //register self as event handler so 'users' and 'channels' can be updated
        mEventHandler.addConnectionListener(this);
        mEventHandler.addClientListener(this);
        mEventHandler.addCommandListener(this);
        mEventHandler.addUserListener(this);
        
        //create timer to process 'mEventHandler'
        createEventTimer();
        
        Log.d(TAG, "Created TeamTalk 5 service");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent.hasExtra(CANCEL_TRANSFER)) {
            int transferId = intent.getIntExtra(CANCEL_TRANSFER, 0);
            if ((ttclient != null) && ttclient.cancelFileTransfer(transferId)) {
                fileTransfers.remove(transferId);
                Toast.makeText(this, R.string.transfer_stopped, Toast.LENGTH_LONG).show();
            }
        }
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public void onDestroy() {
        eventTimer.cancel();
        mEventHandler.removeConnectionListener(this);
        mEventHandler.removeClientListener(this);
        mEventHandler.removeCommandListener(this);
        mEventHandler.removeUserListener(this);

        if (ttclient != null)
            ttclient.closeTeamTalk();

        super.onDestroy();
        
        Log.d(TAG, "Destroyed TeamTalk 5 service");
    }

    TeamTalkBase ttclient;
    ServerEntry ttserver;
    Channel joinchannel, /* the channel to join after login */
            curchannel; /* the channel 'ttclient' is currently in */
    OnVoiceTransmissionToggleListener onVoiceTransmissionToggleListener;
    CountDownTimer eventTimer;
    
    SparseArray<CmdComplete> activecmds = new SparseArray<CmdComplete>();

    public TeamTalkBase getTTInstance() {
        return ttclient;
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

    public boolean isVoiceTransmissionEnabled() {
        return (ttclient.getFlags() & ClientFlag.CLIENT_TX_VOICE) != 0;
    }

    public boolean enableVoiceTransmission(boolean enable) {
        if (enable)
            return (((ttclient.getFlags() & ClientFlag.CLIENT_SNDINPUT_READY) != 0) || ttclient.initSoundInputDevice(0)) && ttclient.enableVoiceTransmission(true);
        else if (ttclient.enableVoiceTransmission(false)) {
            ttclient.closeSoundInputDevice();
            return true;
        }
        return false;
    }

    public boolean reconnect() {
        if(ttserver == null || ttclient == null)
            return false;
        
        ttclient.disconnect();
        
        if(!ttclient.connect(ttserver.ipaddr, ttserver.tcpport,
                             ttserver.udpport, 0, 0, ttserver.encrypted)) {
            ttclient.disconnect();
            return false;
        }
        
        return true;
    }

    Map<Integer, Channel> channels = new HashMap<Integer, Channel>();
    Map<Integer, RemoteFile> remoteFiles = new HashMap<Integer, RemoteFile>();
    Map<Integer, FileTransfer> fileTransfers = new HashMap<Integer, FileTransfer>();
    Map<Integer, User> users = new HashMap<Integer, User>();
    Map<Integer, Vector<MyTextMessage>> usertxtmsgs = new HashMap<Integer, Vector<MyTextMessage>>();
    Vector<MyTextMessage> chatlogtxtmsgs = new Vector<MyTextMessage>();

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
    
    public int HISTORY_CHATLOG_MSG_MAX = 100;
    public int HISTORY_USER_MSG_MAX = 100;

    public Vector<MyTextMessage> getUserTextMsgs(int userid) {
        Vector<MyTextMessage> msgs;
        if(usertxtmsgs.get(userid) == null) {
            msgs = new Vector<MyTextMessage>();
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
    
    public void resetState() {
        reconnectHandler.removeCallbacks(reconnectTimer);
        
        if(ttclient != null)
            ttclient.disconnect();
        
        joinchannel = null;
        curchannel = null;
        channels.clear();
        remoteFiles.clear();
        fileTransfers.clear();
        users.clear();
        usertxtmsgs.clear();
        chatlogtxtmsgs.clear();
    }
    
    void createEventTimer() {
        eventTimer = new CountDownTimer(10000, 100) {
            private boolean prevVoiceTransmissionState = isVoiceTransmissionEnabled();

            public void onTick(long millisUntilFinished) {
                while(mEventHandler.processEvent(ttclient, 0));
                boolean newVoiceTransmissionState = isVoiceTransmissionEnabled();
                if ((onVoiceTransmissionToggleListener != null) &&
                    (newVoiceTransmissionState != prevVoiceTransmissionState))
                    onVoiceTransmissionToggleListener.onVoiceTransmissionToggle(newVoiceTransmissionState);
                prevVoiceTransmissionState = newVoiceTransmissionState;
            }

            public void onFinish() {
                start();
            }
        };
        eventTimer.start();
    }

    Handler reconnectHandler = new Handler();
    Runnable reconnectTimer = new Runnable() {
        @Override
        public void run() {
            reconnect();
        }
    };
    
    void createReconnectTimer(long delayMsec) {
        
        reconnectHandler.removeCallbacks(reconnectTimer);
        reconnectHandler.postDelayed(reconnectTimer, delayMsec);
    }
    
    public void registerConnectionListener(ConnectionListener l) {
        mEventHandler.addConnectionListener(l);
    }

    public void registerCommandListener(CommandListener l) {
        mEventHandler.addCommandListener(l);
    }

    public void registerUserListener(UserListener l) {
        mEventHandler.addUserListener(l);
    }

    public void registerClientListener(ClientListener l) {
        mEventHandler.addClientListener(l);
    }
    
    public void unregisterConnectionListener(ConnectionListener l) {
        mEventHandler.removeConnectionListener(l);
    }

    public void unregisterCommandListener(CommandListener l) {
        mEventHandler.removeCommandListener(l);
    }

    public void unregisterUserListener(UserListener l) {
        mEventHandler.removeUserListener(l);
    }

    public void unregisterClientListener(ClientListener l) {
        mEventHandler.removeClientListener(l);
    }

    @Override
    public void onConnectSuccess() {
        
        assert (ttserver != null);
        
        String def_nick = getResources().getString(R.string.pref_default_nickname);
        String nickname = PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getString("nickname_text", def_nick);

        int loginCmdId = ttclient.doLogin(nickname, ttserver.username, ttserver.password);
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
    public void onMaxPayloadUpdate(int payload_size) {
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
        }
    }

    @Override
    public void onCmdProcessing(int cmdId, boolean complete) {

        if(!complete && activecmds.get(cmdId) == CmdComplete.CMD_COMPLETE_LOGIN) {
            //new users and channels will be posted for new login, so delete old ones
            users.clear();
            remoteFiles.clear();
            fileTransfers.clear();
            channels.clear();
        }
        
        if(!complete)
            return;

        if(activecmds.get(cmdId) == CmdComplete.CMD_COMPLETE_LOGIN) {

        }

        activecmds.delete(cmdId);
    }

    @Override
    public void onCmdMyselfLoggedIn(int my_userid, UserAccount useraccount) {
        if (joinchannel == null) {
            joinchannel = new Channel();
            int rootchanid = ttclient.getRootChannelID();
            int chanid = ((ttserver.channel == null) || ttserver.channel.isEmpty()) ? rootchanid : ttclient.getChannelIDFromPath(ttserver.channel);
            if (ttclient.getChannel(chanid, joinchannel)) {
                joinchannel.szPassword = ttserver.chanpasswd;
            }
            else if ((chanid == rootchanid) || !ttclient.getChannel(rootchanid, joinchannel)) {
                joinchannel = null;
            }
        }

        if(joinchannel != null) {
            int cmdid = ttclient.doJoinChannel(joinchannel);
            activecmds.put(cmdid, CmdComplete.CMD_COMPLETE_JOIN);
        }
        
        MyTextMessage msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
            getResources().getString(R.string.text_cmd_loggedin));
        getChatLogTextMsgs().add(msg);
    }

    @Override
    public void onCmdMyselfLoggedOut() {
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
        if(!pref.getBoolean("sub_txtmsg_checkbox", true))
            def_unsub |= Subscription.SUBSCRIBE_USER_MSG;
        if(!pref.getBoolean("sub_chanmsg_checkbox", true))
            def_unsub |= Subscription.SUBSCRIBE_CHANNEL_MSG;
        if(!pref.getBoolean("sub_bcastmsg_checkbox", true))
            def_unsub |= Subscription.SUBSCRIBE_BROADCAST_MSG;
        if(!pref.getBoolean("sub_voice_checkbox", true))
            def_unsub |= Subscription.SUBSCRIBE_VOICE;
        if(!pref.getBoolean("sub_video_checkbox", true))
            def_unsub |= Subscription.SUBSCRIBE_VIDEOCAPTURE;
        if(!pref.getBoolean("sub_desktop_checkbox", true))
            def_unsub |= Subscription.SUBSCRIBE_MEDIAFILE;
        if(!pref.getBoolean("sub_mediafile_checkbox", true))
            def_unsub |= Subscription.SUBSCRIBE_DESKTOP;

        if((user.uLocalSubscriptions & def_unsub) != 0) {
            int cmdid = ttclient.doUnsubscribe(user.nUserID, def_unsub);
            if(cmdid > 0)
                activecmds.put(cmdid, CmdComplete.CMD_COMPLETE_UNSUBSCRIBE);
        }
    }

    @Override
    public void onCmdUserLoggedOut(User user) {
        users.remove(user.nUserID);
        
        if(usertxtmsgs.containsKey(user.nUserID)) {
            MyTextMessage msg;
            msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
                user.szNickname + " " + getResources().getString(R.string.text_cmd_userleftchan));
            getUserTextMsgs(user.nUserID).add(msg);
        }
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
            curchannel = getChannels().get(user.nChannelID);
            
            MyTextMessage msg;
            if(curchannel.nParentID == 0) {
                msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
                    getResources().getString(R.string.text_cmd_joinroot));
            }
            else {
                msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
                    getResources().getString(R.string.text_cmd_joinchan) + " " + curchannel.szName);
            }
            getChatLogTextMsgs().add(msg);
        }
        else if(curchannel != null && curchannel.nChannelID == user.nChannelID) {
            //other user joined current channel
            
            MyTextMessage msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
                user.szNickname + " " + getResources().getString(R.string.text_cmd_userjoinchan));
            getChatLogTextMsgs().add(msg);
        }
        
        // Video is currently not supported on Android
        if((user.uLocalSubscriptions & Subscription.SUBSCRIBE_VIDEOCAPTURE) != 0) {
            ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_VIDEOCAPTURE);
        }
    }

    @Override
    public void onCmdUserLeftChannel(int channelid, User user) {
        users.put(user.nUserID, user);
        
        if(curchannel != null && curchannel.nChannelID == channelid) {
            
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
                msg = MyTextMessage.createLogMsg(MyTextMessage.MSGTYPE_LOG_INFO,
                    user.szNickname + " " + getResources().getString(R.string.text_cmd_userleftchan));                
            }
            getChatLogTextMsgs().add(msg);
        }
        
        if(user.nUserID == ttclient.getMyUserID()) {
            curchannel = null;
        }
    }

    @Override
    public void onCmdUserTextMessage(TextMessage textmessage) {

        User user = getUsers().get(textmessage.nFromUserID);
        MyTextMessage newmsg = new MyTextMessage(textmessage, 
                                                 user == null? "" : user.szNickname);

        switch(textmessage.nMsgType) {
            case TextMsgType.MSGTYPE_USER : {
                getUserTextMsgs(textmessage.nFromUserID).add(newmsg);
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
    public void onUserRecordMediaFile(int nUserID, MediaFileInfo mediafileinfo) {
    }

    @Override
    public void onUserAudioBlock(int nUserID, int nStreamType) {
        // TODO Auto-generated method stub
        
    }

    @Override
    public void onCmdUserAccount(UserAccount useraccount) {
    }

    @Override
    public void onCmdBannedUser(BannedUser banneduser) {
    }

    @Override
    public void onInternalError(ClientErrorMsg clienterrormsg) {
    }

    @Override
    public void onVoiceActivation(boolean bVoiceActive) {
    }

    @Override
    public void onHotKeyToggle(int nHotKeyID, boolean bActive) {
    }

    @Override
    public void onHotKeyTest(int nVkCode, boolean bActive) {
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
    public void onDesktopWindowTransfer(int nSessionID, int nTransferRemaining) {
    }

    @Override
    public void onStreamMediaFile(MediaFileInfo mediafileinfo) {
    }

}
