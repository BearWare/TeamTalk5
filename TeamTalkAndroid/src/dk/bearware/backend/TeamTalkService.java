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

import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.DesktopInput;
import dk.bearware.MediaFileInfo;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.TeamTalk5;
import dk.bearware.TeamTalkBase;
import dk.bearware.TextMessage;
import dk.bearware.TextMsgType;
import dk.bearware.User;
import dk.bearware.UserAccount;
import dk.bearware.data.MyTextMessage;
import dk.bearware.events.TeamTalkEventHandler;
import dk.bearware.events.ClientListener;
import dk.bearware.events.ConnectionListener;
import dk.bearware.events.CommandListener;
import dk.bearware.events.UserListener;
import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.CountDownTimer;
import android.os.IBinder;

public class TeamTalkService extends Service implements CommandListener, UserListener {

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
        mEventHandler.addCommandListener(this);
        mEventHandler.addUserListener(this);
        
        //create timer to process 'mEventHandler'
        createTimer();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    TeamTalkBase ttclient;

    public TeamTalkBase getTTInstance() {
        return ttclient;
    }

    Map<Integer, Channel> channels = new HashMap<Integer, Channel>();
    Map<Integer, User> users = new HashMap<Integer, User>();
    Map<Integer, Vector<MyTextMessage>> usertxtmsgs = new HashMap<Integer, Vector<MyTextMessage>>();
    Map<Integer, Vector<MyTextMessage>> chantxtmsgs = new HashMap<Integer, Vector<MyTextMessage>>();

    public Map<Integer, Channel> getChannels() {
        return channels;
    }

    public Map<Integer, User> getUsers() {
        return users;
    }
    
    public int HISTORY_CHANNEL_MSG_MAX = 100;
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

    public Vector<MyTextMessage> getChannelTextMsgs(int channelid) {
        Vector<MyTextMessage> msgs;
        if(chantxtmsgs.get(channelid) == null) {
            msgs = new Vector<MyTextMessage>();
            chantxtmsgs.put(channelid, msgs);
        }
        msgs = chantxtmsgs.get(channelid);
        if(msgs.size() > HISTORY_CHANNEL_MSG_MAX)
            msgs.remove(0);
        return msgs;
    }

    public void resetState() {
        channels.clear();
        users.clear();
        usertxtmsgs.clear();
        chantxtmsgs.clear();
    }

    void createTimer() {
        new CountDownTimer(100, 100) {
            public void onTick(long millisUntilFinished) {
                while(mEventHandler.processEvent(ttclient, 0));
            }

            public void onFinish() {
                createTimer();
            }
        }.start();
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
    public void onCmdError(int cmdId, ClientErrorMsg errmsg) {
    }

    @Override
    public void onCmdSuccess(int cmdId) {
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
        users.put(kicker.nUserID, kicker);
    }

    @Override
    public void onCmdUserLoggedIn(User user) {
        users.put(user.nUserID, user);
    }

    @Override
    public void onCmdUserLoggedOut(User user) {
        users.remove(user.nUserID);
    }

    @Override
    public void onCmdUserUpdate(User user) {
        users.put(user.nUserID, user);
    }

    @Override
    public void onCmdUserJoinedChannel(User user) {
        users.put(user.nUserID, user);
    }

    @Override
    public void onCmdUserLeftChannel(int channelid, User user) {
        users.put(user.nUserID, user);
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
                getChannelTextMsgs(textmessage.nChannelID).add(newmsg);
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
    }

    @Override
    public void onCmdFileNew(RemoteFile remotefile) {
    }

    @Override
    public void onCmdFileRemove(RemoteFile remotefile) {
    }
}
