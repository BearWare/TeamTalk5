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

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.time.Duration;
import java.time.Instant;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Vector;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

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
    IPBan bans;
    int ipv4banprefix, ipv6banprefix;
    BadWords badwords;
    Vector<String> langbadwords = new Vector<>();
    Abuse abuse;
    AbuseDB abusedb;
    enum CmdComplete {
        CMD_NONE,
        CMD_LISTBANS,
        CMD_ADDBAN,
        CMD_ABUSE_BAN,
        CMD_ABUSE_KICK,
        CMD_ABUSE_TEXTMSG,
        CMD_LOGIN,
        CMD_REMOVECHANNEL,
        CMD_JOINCHANNEL,
        CMD_UNSUBSCRIBE
    }
    Map<Integer, CmdComplete> activecommands = new HashMap<>();
    Logger logger;

    SpamBotSession(TeamTalkServer srv, WebLogin loginsession,
                   IPBan bans, BadWords badwords, Abuse abuse,
                   AbuseDB abusedb, int ipv4banprefix, int ipv6banprefix,
                   Logger log) {
        this.server = srv;
        this.loginsession = loginsession;
        this.bans = bans;
        this.badwords = badwords;
        this.abuse = abuse;
        this.abusedb = abusedb;
        this.ipv4banprefix = ipv4banprefix;
        this.ipv6banprefix = ipv6banprefix;
        this.logger = log;
        handler.addConnectionListener(this);
        handler.addCommandListener(this);
    }

    public void runConnectionEventLoop() {

        if ((ttclient.getFlags() & ClientFlag.CLIENT_CONNECTING) == ClientFlag.CLIENT_CONNECTING)
            return;

        // disconnect if no authentication has been made (afterwards try again)
        if ((ttclient.getFlags() & ClientFlag.CLIENT_CONNECTED) == ClientFlag.CLIENT_CONNECTED &&
                (ttclient.getFlags() & ClientFlag.CLIENT_AUTHORIZED) == ClientFlag.CLIENT_CLOSED) {
            logger.warning(String.format("Authentication failed for %s:%d", server.ipaddr, server.tcpport));
            ttclient.disconnect();
        }

        if ((ttclient.getFlags() & ClientFlag.CLIENT_CONNECTION) == ClientFlag.CLIENT_CLOSED) {
            if (!ttclient.connect(server.ipaddr, server.tcpport, server.udpport, 0, 0, server.encrypted)) {
                logger.warning(String.format("Failed to connect to %s:%d", server.ipaddr, server.tcpport));
                return;
            }
            else {
                logger.info(String.format("Connecting to %s:%d", server.ipaddr, server.tcpport));
            }
        }
    }

    public void runEventLoop(int timeoutMSec) {
        while (handler.processEvent(ttclient, timeoutMSec));
        runTimedEvents();
    }

    private final Duration SYNC_TIMEOUT = Duration.ofSeconds(10);
    private Instant synctimeout = Instant.now();

    private void runTimedEvents() {

        // wait until authorized
        if ((ttclient.getFlags() & ClientFlag.CLIENT_AUTHORIZED) == ClientFlag.CLIENT_CLOSED)
            return;

        // don't run timed events if commands are currently processing
        if (!this.activecommands.isEmpty())
            return;

        // run unban timer
        if (Instant.now().isAfter(synctimeout)) {
            int bancmdid = this.bans.syncBans(this.ttclient);
            if (bancmdid > 0) {
                activecommands.put(bancmdid, CmdComplete.CMD_ADDBAN);
            }
            else {
                synctimeout = Instant.now().plus(SYNC_TIMEOUT);
            }
        }
    }

    public boolean containsBadWord(String text) {
        for (String lang : langbadwords) {
            if (badwords.contains(lang, text)) {
                logger.info("Bad word in language: " + lang + ". Text: " + text);
                return true;
            }
        }
        return false;
    }

    public boolean cleanUser(User user) {
        if (containsBadWord(user.szNickname)) {
            logger.info("Bad word in nickname: " + user.szNickname);
            return false;
        }
        if (containsBadWord(user.szStatusMsg)) {
            logger.info("Bad word in status message: " + user.szStatusMsg);
            return false;
        }
        return true;
    }

    public boolean cleanChannel(Channel chan) {
        if (containsBadWord(chan.szName)) {
            logger.info("Bad word in channel name: " + chan.szName);
            return false;
        }
        if (containsBadWord(chan.szTopic)) {
            logger.info("Bad word in channel topic: " + chan.szTopic);
            return false;
        }
        return true;
    }

    public boolean cleanTextMessage(TextMessage msg) {
        if (containsBadWord(msg.szMessage)) {
            logger.info("Bad word in text message: " + msg.szMessage);
            return false;
        }
        return true;
    }

    @Override
    public void onEncryptionError(int opensslErrorNo, ClientErrorMsg errmsg) {
        logger.warning(String.format("Encryption error %s while connecting to server %s:%d", errmsg.szErrorMsg, server.ipaddr, server.tcpport));
    }

    @Override
    public void onConnectFailed() {
        logger.warning(String.format("Failed to connect to server %s:%d", server.ipaddr, server.tcpport));
        ttclient.disconnect();
    }

    @Override
    public void onConnectSuccess() {
        logger.info(String.format("Connected to server: %s:%d", server.ipaddr, server.tcpport));

        ServerProperties srvprop = new ServerProperties();
        ttclient.getServerProperties(srvprop);
        if (!loginsession.registerLogin(srvprop.szAccessToken))
            return;

        // perform login
        this.activecommands.put(ttclient.doLogin("SpamBot", loginsession.getUsername(), ""), CmdComplete.CMD_LOGIN);
        logger.info(String.format("Issued login cmd to %s:%d", server.ipaddr, server.tcpport));
    }

    @Override
    public void onConnectionLost() {
        logger.warning(String.format("Lost connection to server: %s:%d", server.ipaddr, server.tcpport));
        ttclient.disconnect();
    }

    @Override
    public void onMaxPayloadUpdate(int arg0) {
    }

    @Override
    public void onCmdBannedUser(BannedUser ban) {
        this.bans.addRemoteBan(ban);
    }

    @Override
    public void onCmdChannelNew(Channel chan) {
        channels.put(chan.nChannelID, chan);

        if (!cleanChannel(chan))
            this.activecommands.put(ttclient.doRemoveChannel(chan.nChannelID), CmdComplete.CMD_REMOVECHANNEL);
    }

    @Override
    public void onCmdChannelRemove(Channel chan) {
        channels.remove(chan.nChannelID);
    }

    @Override
    public void onCmdChannelUpdate(Channel chan) {
        channels.put(chan.nChannelID, chan);

        if (!cleanChannel(chan) && chan.nParentID != 0)
            this.activecommands.put(ttclient.doRemoveChannel(chan.nChannelID), CmdComplete.CMD_REMOVECHANNEL);
    }

    @Override
    public void onCmdError(int cmdid, ClientErrorMsg err) {
        logger.warning(String.format("Command #%d failed on %s:%d. %s", cmdid, server.ipaddr, server.tcpport, err.szErrorMsg));
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

        logger.info(String.format("Got userID #%d on %s:%d.", userid, server.ipaddr, server.tcpport));
        logger.info("User rights assigned:");
        if((useraccount.uUserRights & UserRight.USERRIGHT_VIEW_ALL_USERS) != 0)
            logger.info("See all users");
        if((useraccount.uUserRights & UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL) != 0)
            logger.info("Create temporary channel");
        if((useraccount.uUserRights & UserRight.USERRIGHT_KICK_USERS) != 0)
            logger.info("Kick users");
        if((useraccount.uUserRights & UserRight.USERRIGHT_BAN_USERS) != 0)
            logger.info("Ban users");
        if((useraccount.uUserRights & UserRight.USERRIGHT_MODIFY_CHANNELS) != 0)
            logger.info("Modify all channels");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_VOICE) != 0)
            logger.info("Transmit voice");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_VIDEOCAPTURE) != 0)
            logger.info("Transmit video from webcam");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_DESKTOP) != 0)
            logger.info("Transmit desktop");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_DESKTOPINPUT) != 0)
            logger.info("Control desktops");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO) != 0)
            logger.info("Transmit media files containing video");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO) != 0)
            logger.info("Transmit media files containing audio");
        if((useraccount.uUserRights & UserRight.USERRIGHT_DOWNLOAD_FILES) != 0)
            logger.info("Download files");
        if((useraccount.uUserRights & UserRight.USERRIGHT_UPLOAD_FILES) != 0)
            logger.info("Upload files");

        joinChannel(useraccount);
        setupBadWords(useraccount);
        syncBans(useraccount);
    }

    void joinChannel(UserAccount account) {
        // see if spambot should join initial channel
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
            this.activecommands.put(ttclient.doJoinChannel(chan), CmdComplete.CMD_JOINCHANNEL);
        }
    }

    void setupBadWords(UserAccount account) {
        for (String lang : account.szNote.toLowerCase(Locale.ROOT).split(",")) {
            langbadwords.add(lang);
            logger.info(String.format("Using language \"%s\" with %d bad words on %s:%d.", lang, badwords.getBadWords(lang).size(), server.ipaddr, server.tcpport));
        }

        if (langbadwords.isEmpty()) {
            langbadwords.add(""); // use default
        }
    }

    void syncBans(UserAccount account) {
        if ((account.uUserRights & UserRight.USERRIGHT_BAN_USERS) == UserRight.USERRIGHT_BAN_USERS) {
            if (versionSameOrLater("5.13")) {
                activecommands.put(ttclient.doListBans(0, 0, 1000000), CmdComplete.CMD_LISTBANS);
            }
            else {
                logger.info("Skipping ban sync due to version");
            }
        }
        else {
            logger.info("Skipping ban sync due to missing user right");
        }
    }

    void sendBadWordsNotify(int userid, String text) {
        TextMessage textmsg = new TextMessage();
        textmsg.nMsgType = TextMsgType.MSGTYPE_USER;
        textmsg.szMessage = text;
        textmsg.nToUserID = userid;
        activecommands.put(ttclient.doTextMessage(textmsg), CmdComplete.CMD_ABUSE_TEXTMSG);
    }

    int getBanPrefix(String ipaddr) {
        try {
            InetAddress ipv = InetAddress.getByName(ipaddr);
            if (ipv instanceof Inet6Address)
                return ipv6banprefix == 128 ? 0 : ipv6banprefix;
            else if (ipv instanceof Inet4Address)
                return ipv4banprefix == 32 ? 0 : ipv4banprefix;
        }
        catch(UnknownHostException e) {
        }
        return 0;
    }

    void abuseBan(User user) {
        BannedUser b = new BannedUser();
        b.uBanTypes = BanType.BANTYPE_IPADDR | BanType.BANTYPE_USERNAME;
        b.szNickname = user.szNickname;
        b.szUsername = user.szUsername;
        b.szIPAddress = user.szIPAddress;
        int prefix = getBanPrefix(user.szIPAddress);
        if (prefix > 0) {
            b.szIPAddress = String.format("%s/%d", b.szIPAddress, prefix);
        }
        int cmdid = ttclient.doBan(b);
        if (cmdid > 0) {
            activecommands.put(cmdid, CmdComplete.CMD_ABUSE_BAN);
            this.bans.addLocalBan(b, ttclient);
        }

        // inform user about ban
        TextMessage textmsg = new TextMessage();
        textmsg.nMsgType = TextMsgType.MSGTYPE_USER;
        textmsg.szMessage = "You have been banned due to abuse";
        textmsg.nToUserID = user.nUserID;
        cmdid = ttclient.doTextMessage(textmsg);
        if (cmdid > 0) {
            activecommands.put(cmdid, CmdComplete.CMD_ABUSE_TEXTMSG);
        }

        cmdid = ttclient.doKickUser(user.nUserID, 0);
        if (cmdid > 0) {
            activecommands.put(cmdid, CmdComplete.CMD_ABUSE_KICK);
        }

        abusedb.report(user.szIPAddress, "Spam");
    }

    @Override
    public void onCmdMyselfLoggedOut() {
        ttclient.disconnect();
    }

    @Override
    public void onCmdProcessing(int cmdid, boolean complete) {
        if(complete) {
            var v = activecommands.get(cmdid);
            switch (v == null ? CmdComplete.CMD_NONE : v) {
            case CMD_LISTBANS : {
                int bancmdid = this.bans.syncBans(this.ttclient);
                if (bancmdid > 0)
                    activecommands.put(bancmdid, CmdComplete.CMD_ADDBAN);
                break;
            }
            case CMD_ADDBAN : {
                int bancmdid = this.bans.syncBans(this.ttclient);
                if (bancmdid > 0)
                    activecommands.put(bancmdid, CmdComplete.CMD_ADDBAN);
            }
            }
            activecommands.remove(cmdid);
        }
    }

    @Override
    public void onCmdServerUpdate(ServerProperties srvprop) {
    }

    @Override
    public void onCmdSuccess(int cmdid) {
    }

    @Override
    public void onCmdUserAccount(UserAccount arg0) {
    }

    @Override
    public void onCmdUserAccountNew(UserAccount userAccount) {
    }

    @Override
    public void onCmdUserAccountRemove(UserAccount userAccount) {
    }

    @Override
    public void onCmdUserJoinedChannel(User user) {
        users.put(user.nUserID, user);

        if (!cleanUser(user)) {
            sendBadWordsNotify(user.nUserID, "Your nick name and/or status message contains foul language");
            this.activecommands.put(ttclient.doKickUser(user.nUserID, 0), CmdComplete.CMD_ABUSE_KICK);
            logger.info(String.format("Kicking %s from %s:%d", user.szNickname, server.ipaddr, server.tcpport));

            abuse.incKicks(user.szIPAddress);
        }

        if (user.nChannelID == ttclient.getMyChannelID()) {
            this.activecommands.put(ttclient.doUnsubscribe(user.nUserID, Subscription.SUBSCRIBE_VOICE | Subscription.SUBSCRIBE_DESKTOP | Subscription.SUBSCRIBE_VIDEOCAPTURE | Subscription.SUBSCRIBE_MEDIAFILE), CmdComplete.CMD_UNSUBSCRIBE);
        }

        abuse.incJoins(user.szIPAddress);
        if (abuse.checkJoinAbuse(user.szIPAddress)) {
            logger.info(String.format("Banning %s from %s:%d due to join abuse", user.szNickname, server.ipaddr, server.tcpport));
            abuseBan(user);
        }
    }

    @Override
    public void onCmdUserLeftChannel(int chanid, User user) {
        users.put(user.nUserID, user);
    }

    @Override
    public void onCmdUserLoggedIn(User user) {
        users.put(user.nUserID, user);

        if (user.nUserID == ttclient.getMyUserID())
            abusedb.addWhiteListIPAddr(user.szIPAddress);
        else if (user.szIPAddress.length() > 0 && abusedb.checkForReported(user.szIPAddress)) {
            sendBadWordsNotify(user.nUserID, "Your IP-address is listed as a spammer");
            this.activecommands.put(ttclient.doKickUser(user.nUserID, 0), CmdComplete.CMD_ABUSE_KICK);
            logger.info(String.format("Kicking %s from %s:%d because %s is listed as spammer", user.szNickname, server.ipaddr, server.tcpport, user.szIPAddress));
            abuse.incKicks(user.szIPAddress);
        }
        else if (!cleanUser(user)) {
            sendBadWordsNotify(user.nUserID, "Your nick name and/or status message contains foul language");
            this.activecommands.put(ttclient.doKickUser(user.nUserID, 0), CmdComplete.CMD_ABUSE_KICK);
            logger.info(String.format("Kicking %s from %s:%d due to bad words", user.szNickname, server.ipaddr, server.tcpport));
            abuse.incKicks(user.szIPAddress);
        }

        abuse.incLogin(user.szIPAddress);
        if (abuse.checkLoginAbuse(user.szIPAddress)) {
            logger.info(String.format("Banning %s from %s:%d due to login abuse", user.szNickname, server.ipaddr, server.tcpport));
            abuseBan(user);
        }
        if (abuse.checkKickAbuse(user.szIPAddress)) {
            logger.info(String.format("Banning %s from %s:%d due to badwords abuse", user.szNickname, server.ipaddr, server.tcpport));
            abuseBan(user);
        }
    }

    @Override
    public void onCmdUserLoggedOut(User user) {
        users.remove(user.nUserID);
    }

    @Override
    public void onCmdUserTextMessage(TextMessage textmsg) {
        if (!cleanTextMessage(textmsg)) {
            sendBadWordsNotify(textmsg.nFromUserID, "Your text message contains foul language");
            this.activecommands.put(ttclient.doKickUser(textmsg.nFromUserID, 0), CmdComplete.CMD_ABUSE_KICK);
            logger.info(String.format("Kicking #%d from %s:%d", textmsg.nFromUserID, server.ipaddr, server.tcpport));
            User user = users.get(textmsg.nFromUserID);
            if (user != null) {
                abuse.incKicks(user.szIPAddress);
            }
        }
    }

    @Override
    public void onCmdUserUpdate(User user) {
        users.put(user.nUserID, user);
        if (!cleanUser(user)) {
            sendBadWordsNotify(user.nUserID, "Your nick name and/or status message contains foul language");
            this.activecommands.put(ttclient.doKickUser(user.nUserID, 0), CmdComplete.CMD_ABUSE_KICK);
            logger.info(String.format("Kicking %s from %s:%d", user.szNickname, server.ipaddr, server.tcpport));
            abuse.incKicks(user.szIPAddress);
        }
    }

    @Override
    public void close() {
        ttclient.disconnect();
    }

    boolean versionSameOrLater(String version) {
        ServerProperties prop = new ServerProperties();
        if (ttclient.getServerProperties(prop)) {
            Pattern pattern = Pattern.compile("(\\d+)\\.(\\d+)");
            Matcher remotematcher = pattern.matcher(prop.szServerProtocolVersion);
            Matcher versionmatcher = pattern.matcher(version);
            if (remotematcher.find() && versionmatcher.find()) {
                return remotematcher.group(1).compareTo(versionmatcher.group(1)) >= 0 &&
                    remotematcher.group(2).compareTo(versionmatcher.group(2)) >= 0;
            }
        }
        return false;
    }
}
