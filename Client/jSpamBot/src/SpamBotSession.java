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

import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Vector;

import dk.bearware.*;
import dk.bearware.events.CommandListener;
import dk.bearware.events.ConnectionListener;
import dk.bearware.events.TeamTalkEventHandler;

public class SpamBotSession
implements ConnectionListener, CommandListener, AutoCloseable {

    TeamTalkBase ttclient = new TeamTalk5();
    TeamTalkEventHandler handler = new TeamTalkEventHandler();
    
    Map<Integer, Channel> channels = new HashMap<>();
    Map<Integer, User> users = new HashMap<>();

    TeamTalkServer server;
    WebLogin loginsession;
    Vector<String> badwords;
    Map<String, Vector<String>> langbadwords;

    int cmdid_completed = 0, cmdid_success = 0;

    SpamBotSession(TeamTalkServer srv, WebLogin loginsession, Map<String, Vector<String>> langbadwords) {
        this.server = srv;
        this.loginsession = loginsession;
        this.langbadwords = langbadwords;
        handler.addConnectionListener(this);
        handler.addCommandListener(this);
    }

    public void runConnectionEventLoop() {

        if ((ttclient.getFlags() & ClientFlag.CLIENT_CONNECTING) == ClientFlag.CLIENT_CONNECTING)
            return;

        // disconnect if no authentication has been made (afterwards try again)
        if ((ttclient.getFlags() & ClientFlag.CLIENT_CONNECTED) == ClientFlag.CLIENT_CONNECTED &&
                (ttclient.getFlags() & ClientFlag.CLIENT_AUTHORIZED) == ClientFlag.CLIENT_CLOSED) {
            System.err.printf("Authentication failed for %s:%d\n", server.ipaddr, server.tcpport);
            ttclient.disconnect();
        }

        if ((ttclient.getFlags() & ClientFlag.CLIENT_CONNECTION) == ClientFlag.CLIENT_CLOSED) {
            if (!ttclient.connect(server.ipaddr, server.tcpport, server.udpport, 0, 0, server.encrypted)) {
                System.err.printf("Failed to connect to %s:%d\n", server.ipaddr, server.tcpport);
                return;
            }
            else {
                System.out.printf("Connecting to %s:%d\n", server.ipaddr, server.tcpport);
            }
        }
    }

    public void runEventLoop(int timeoutMSec) {
        while (handler.processEvent(ttclient, timeoutMSec));
    }

    public boolean containsBadWord(String value) {
        value = value.toLowerCase();

        String[] words = value.split("\\W");
        
        for (String word : words) {
            if (word.isEmpty())
                continue;
            if (badwords.contains(word))
                return true;
        }
        return false;
    }

    public boolean cleanUser(User user) {
        if (containsBadWord(user.szNickname))
            return false;
        if (containsBadWord(user.szStatusMsg))
            return false;
        return true;
    }

    public boolean cleanChannel(Channel chan) {
        if (containsBadWord(chan.szName))
            return false;
        if (containsBadWord(chan.szTopic))
            return false;
        return true;
    }

    public boolean cleanTextMessage(TextMessage msg) {
        if (containsBadWord(msg.szMessage))
            return false;
        return true;
    }

    @Override
    public void onConnectFailed() {
        System.err.printf("Failed to connect to server %s:%d\n", server.ipaddr, server.tcpport);
        ttclient.disconnect();
    }

    @Override
    public void onConnectSuccess() {
        System.out.printf("Connected to server: %s:%d\n", server.ipaddr, server.tcpport);

        ServerProperties srvprop = new ServerProperties();
        ttclient.getServerProperties(srvprop);
        if (!loginsession.registerLogin(srvprop.szAccessToken))
            return;

        // perform login
        int cmdid = ttclient.doLogin("SpamBot", loginsession.getUsername(), "");
        System.out.printf("Issued login cmd #%d to %s:%d\n", cmdid, server.ipaddr, server.tcpport);
    }

    @Override
    public void onConnectionLost() {
        System.err.printf("Lost connection to server: %s:%d\n", server.ipaddr, server.tcpport);
        ttclient.disconnect();
    }

    @Override
    public void onMaxPayloadUpdate(int arg0) {
    }

    @Override
    public void onCmdBannedUser(BannedUser arg0) {
    }

    @Override
    public void onCmdChannelNew(Channel chan) {
        channels.put(chan.nChannelID, chan);

        if (!cleanChannel(chan))
            ttclient.doRemoveChannel(chan.nChannelID);
    }

    @Override
    public void onCmdChannelRemove(Channel chan) {
        channels.remove(chan.nChannelID);
    }

    @Override
    public void onCmdChannelUpdate(Channel chan) {
        channels.put(chan.nChannelID, chan);

        if (!cleanChannel(chan) && chan.nParentID != 0)
            ttclient.doRemoveChannel(chan.nChannelID);
    }

    @Override
    public void onCmdError(int cmdid, ClientErrorMsg err) {
        System.err.printf("Command #%d failed on %s:%d. %s\n", cmdid, server.ipaddr, server.tcpport, err.szErrorMsg);
    }

    @Override
    public void onCmdFileNew(RemoteFile arg0) {
    }

    @Override
    public void onCmdFileRemove(RemoteFile arg0) {
    }

    @Override
    public void onCmdMyselfKickedFromChannel() {
    }

    @Override
    public void onCmdMyselfKickedFromChannel(User user) {
    }

    @Override
    public void onCmdMyselfLoggedIn(int userid, UserAccount useraccount) {

        System.out.printf("Got userID #%d on %s:%d.\n", userid, server.ipaddr, server.tcpport);
        System.out.println("User rights assigned:");
        if((useraccount.uUserRights & UserRight.USERRIGHT_VIEW_ALL_USERS) != 0)
            System.out.println("\tSee all users");
        if((useraccount.uUserRights & UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL) != 0)
            System.out.println("\tCreate temporary channel");
        if((useraccount.uUserRights & UserRight.USERRIGHT_KICK_USERS) != 0)
            System.out.println("\tKick users");
        if((useraccount.uUserRights & UserRight.USERRIGHT_BAN_USERS) != 0)
            System.out.println("\tBan users");
        if((useraccount.uUserRights & UserRight.USERRIGHT_MODIFY_CHANNELS) != 0)
                System.out.println("\tModify all channels");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_VOICE) != 0)
                System.out.println("\tTransmit voice");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_VIDEOCAPTURE) != 0)
                System.out.println("\tTransmit video from webcam");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_DESKTOP) != 0)
                System.out.println("\tTransmit desktop");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_DESKTOPINPUT) != 0)
                System.out.println("\tControl desktops");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO) != 0)
                System.out.println("\tTransmit media files containing video");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO) != 0)
                System.out.println("\tTransmit media files containing audio");
        if((useraccount.uUserRights & UserRight.USERRIGHT_DOWNLOAD_FILES) != 0)
                System.out.println("\tDownload files");
        if((useraccount.uUserRights & UserRight.USERRIGHT_UPLOAD_FILES) != 0)
                System.out.println("\tUpload files");

        // see if spambot should join initial channel
        UserAccount account = new UserAccount();
        ttclient.getMyUserAccount(account);
        if (!account.szInitChannel.isEmpty()) {
            int chanid = ttclient.getChannelIDFromPath(account.szInitChannel);
            Channel chan = new Channel();
            if (chanid <= 0) {
                // TODO: handle where "szInitChannel" is a path
                chan.szName = account.szInitChannel;
                chan.nParentID = ttclient.getRootChannelID();
            }
            else {
                ttclient.getChannel(chanid, chan);
            }
            ttclient.doJoinChannel(chan);
        }

        badwords = new Vector<>();
        for (String lang : account.szNote.toLowerCase(Locale.ROOT).split(",")) {
            if (langbadwords.get(lang) != null) {
                badwords.addAll(langbadwords.get(lang));
                System.out.printf("Using language \"%s\" with %d bad words on %s:%d.\n", lang, langbadwords.get(lang).size(), server.ipaddr, server.tcpport);
            }
        }
    }

    @Override
    public void onCmdMyselfLoggedOut() {
        ttclient.disconnect();
    }

    @Override
    public void onCmdProcessing(int cmdid, boolean complete) {
        if(complete)
            cmdid_completed = cmdid;
    }

    @Override
    public void onCmdServerUpdate(ServerProperties srvprop) {
    }

    @Override
    public void onCmdSuccess(int cmdid) {
        cmdid_success = cmdid;
    }

    @Override
    public void onCmdUserAccount(UserAccount arg0) {
    }

    @Override
    public void onCmdUserJoinedChannel(User user) {
        users.put(user.nUserID, user);
        
        if (!cleanUser(user)) {
            ttclient.doKickUser(user.nUserID, 0);
            System.out.printf("Kicking %s from %s:%d\n", user.szNickname, server.ipaddr, server.tcpport);
        }

        if (user.nChannelID == ttclient.getMyChannelID()) {
            ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_VOICE | Subscription.SUBSCRIBE_DESKTOP | Subscription.SUBSCRIBE_VIDEOCAPTURE | Subscription.SUBSCRIBE_MEDIAFILE);
        }
    }

    @Override
    public void onCmdUserLeftChannel(int chanid, User user) {
        users.put(user.nUserID, user);
    }

    @Override
    public void onCmdUserLoggedIn(User user) {
        users.put(user.nUserID, user);
        
        if (!cleanUser(user)) {
            ttclient.doKickUser(user.nUserID, 0);
            System.out.printf("Kicking %s from %s:%d\n", user.szNickname, server.ipaddr, server.tcpport);
        }
    }

    @Override
    public void onCmdUserLoggedOut(User user) {
        users.remove(user.nUserID);
    }

    @Override
    public void onCmdUserTextMessage(TextMessage textmsg) {
        if (!cleanTextMessage(textmsg)) {
            ttclient.doKickUser(textmsg.nFromUserID, 0);
            System.out.printf("Kicking #%d from %s:%d\n", textmsg.nFromUserID, server.ipaddr, server.tcpport);
        }
    }

    @Override
    public void onCmdUserUpdate(User user) {
        users.put(user.nUserID, user);
        if (!cleanUser(user)) {
            ttclient.doKickUser(user.nUserID, 0);
            System.out.printf("Kicking %s from %s:%d\n", user.szNickname, server.ipaddr, server.tcpport);
        }
    }

    @Override
    public void close() {
        ttclient.disconnect();
    }
}
