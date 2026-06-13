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

package dk.bearware;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.MethodOrderer;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.TestMethodOrder;
import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.util.Vector;
import java.io.DataOutputStream;
import java.io.FileOutputStream;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.File;
import java.util.Arrays;
import java.net.InetAddress;

@TestMethodOrder(MethodOrderer.MethodName.class)
public class TeamTalkServerTestCase extends TeamTalkTestCaseBase {

    public static final String CRYPTO_SERVER_CERT_FILE = "ttservercert.pem", CRYPTO_SERVER_KEY_FILE = "ttserverkey.pem";
    public static final String CRYPTO_SERVER_CERT_EXPIRED_FILE = "ttservercert-expired.pem";
    public static final String CRYPTO_CLIENT_CERT_EXPIRED_FILE = "ttclientcert-expired.pem";
    public static final String CRYPTO_CA2_FILE = "ca2.cer";
    public static final String CRYPTO_SERVER_CERT2_FILE = "ttservercert2.pem", CRYPTO_SERVER_KEY2_FILE = "ttserverkey2.pem";


    Vector<TeamTalkSrv> servers = new Vector<TeamTalkSrv>();

    String FILESTORAGE_FOLDER = "./filestorage";
    long MAX_DISKUSAGE = 100000000000l, DEFAULT_CHANNEL_QUOTA = 10000000000l;
    String SERVERBINDIP;

    public TeamTalkBase newClientInstance() {
        TeamTalkBase ttclient = new TeamTalk5Pro();
        ttclients.add(ttclient);
        // if (ENCRYPTED) {
        //     assertTrue(ttclient.setEncryptionContext(new EncryptionContext()), "Set encryption context");
        // }
        return ttclient;
    }

    @BeforeEach
    public void setUp() throws Exception {
        super.setUp();

        String prop = System.getProperty("dk.bearware.serverbindip");
        if(prop != null && !prop.isEmpty())
            this.SERVERBINDIP = prop;
        else
            this.SERVERBINDIP = IPADDR;

        File storagedir = new File(FILESTORAGE_FOLDER);
        if (!storagedir.exists())
            storagedir.mkdir();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = ADMIN_USERNAME;
        useraccount.szPassword = ADMIN_PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_ADMIN;
        useraccount.szNote = "An example administrator user account with all user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_ALL;

        useraccounts.add(useraccount);

    }

    @AfterEach
    public void tearDown() throws Exception {
        super.tearDown();

        for(TeamTalkSrv s : servers) {
            s.stopServer();
        }
        servers.clear();
    }

    Vector<UserAccount> useraccounts = new Vector<UserAccount>();

    Vector<String> banned_ipaddr = new Vector<String>();

    UserAccount getUserAccount(String username) {
        for(UserAccount u : useraccounts) {
            if(u.szUsername.equals(username))
                return u;
        }
        return null;
    }

    ServerCallback cmdcallback = new ServerCallback() {

            public void userLogin(ClientErrorMsg lpClientErrorMsg,
                                  User lpUser, UserAccount lpUserAccount) {
                String str = String.format("Login attempt from IP %s, username=%s, password=%s, nickname=%s, clientname=%s",
                                           lpUser.szIPAddress, lpUserAccount.szUsername,
                                           lpUserAccount.szPassword, lpUser.szNickname, lpUser.szClientName);
                System.out.println(str);

                for(UserAccount u : useraccounts) {
                    // validate user account
                    if(u.szUsername.equals(lpUserAccount.szUsername) &&
                       u.szPassword.equals(lpUserAccount.szPassword)) {
                        // manually copy every field
                        lpUserAccount.copy(u);
                        lpClientErrorMsg.nErrorNo = ClientError.CMDERR_SUCCESS;
                        return;
                    }
                }

                // login rejected
                lpClientErrorMsg.nErrorNo = ClientError.CMDERR_INVALID_ACCOUNT;
                lpClientErrorMsg.szErrorMsg = "Invalid username or password";
            }

            public void userChangeNickname(ClientErrorMsg lpClientErrorMsg,
                                           User lpUser, String szNewNickname) {

                String str = String.format("User %s is requesting to change nickname to %s",
                                           lpUser.szNickname, szNewNickname);
                System.out.println(str);

                if(szNewNickname.indexOf("crap")>=0) {
                    lpClientErrorMsg.nErrorNo = 4567;
                    lpClientErrorMsg.szErrorMsg = "Nickname not allowed";
                }
                else {
                    lpClientErrorMsg.nErrorNo = ClientError.CMDERR_SUCCESS;
                }
            }

            public void userChangeStatus(ClientErrorMsg lpClientErrorMsg,
                                         User lpUser, int nNewStatusMode, String szNewStatusMsg) {

                String str = String.format("User %s is requesting to change status message to %s",
                                           lpUser.szNickname, szNewStatusMsg);
                System.out.println(str);

                if(szNewStatusMsg.indexOf("crap")>=0) {
                    lpClientErrorMsg.nErrorNo = 4568;
                    lpClientErrorMsg.szErrorMsg = "Status not allowed";
                }
                else {
                    lpClientErrorMsg.nErrorNo = ClientError.CMDERR_SUCCESS;
                }
            }

            public void userCreateUserAccount(ClientErrorMsg lpClientErrorMsg,
                                              User lpUser, UserAccount lpUserAccount) {

                String str = String.format("User %s is creating useraccount %s",
                                           lpUser.szUsername, lpUserAccount.szUsername);
                System.out.println(str);

                switch(lpUser.uUserType) {
                case UserType.USERTYPE_ADMIN :

                    // remove existing user account
                    UserAccount ua = null;
                    for(UserAccount u : useraccounts) {
                        if(u.szUsername.equals(lpUserAccount.szUsername))
                            ua = u;
                    }
                    useraccounts.remove(ua);

                    // add user account to list
                    useraccounts.add(lpUserAccount);

                    lpClientErrorMsg.nErrorNo = ClientError.CMDERR_SUCCESS;
                    break;
                case UserType.USERTYPE_DEFAULT :
                    lpClientErrorMsg.nErrorNo = ClientError.CMDERR_NOT_AUTHORIZED;
                    lpClientErrorMsg.szErrorMsg = "Hell no!";
                    break;
                default :
                    assertTrue(false, "User type not set");
                    break;
                }

            }

            public void userDeleteUserAccount(ClientErrorMsg lpClientErrorMsg,
                                              User lpUser, String szUsername) {

                String str = String.format("User %s is deleting useraccount %s",
                                           lpUser.szUsername, szUsername);
                System.out.println(str);

                switch(lpUser.uUserType) {
                case UserType.USERTYPE_ADMIN :
                    UserAccount ua = null;
                    for(UserAccount u : useraccounts) {
                        if(u.szUsername.equals(szUsername))
                            ua = u;
                    }
                    useraccounts.remove(ua);
                    lpClientErrorMsg.nErrorNo = ClientError.CMDERR_SUCCESS;
                    break;
                case UserType.USERTYPE_DEFAULT :
                    lpClientErrorMsg.nErrorNo = ClientError.CMDERR_NOT_AUTHORIZED;
                    lpClientErrorMsg.szErrorMsg = "Hell no!";
                    break;
                default :
                    assertTrue(false, "User type not set");
                    break;
                }
            }

            public void userAddServerBan(ClientErrorMsg lpClientErrorMsg,
                                         User lpBanner, User lpBanee) {

                UserAccount ua = getUserAccount(lpBanner.szUsername);

                if((ua.uUserRights & UserRight.USERRIGHT_BAN_USERS) != 0) {
                    lpClientErrorMsg.nErrorNo = ClientError.CMDERR_SUCCESS;
                    banned_ipaddr.add(lpBanee.szIPAddress);
                }
                else {
                    lpClientErrorMsg.nErrorNo = ClientError.CMDERR_NOT_AUTHORIZED;
                    lpClientErrorMsg.szErrorMsg = "Hell no!";
                }
            }

            public void userAddServerBanIPAddress(ClientErrorMsg lpClientErrorMsg,
                                                  User lpBanner, String szIPAddress) {

                UserAccount ua = getUserAccount(lpBanner.szUsername);

                if((ua.uUserRights & UserRight.USERRIGHT_BAN_USERS) != 0) {
                    lpClientErrorMsg.nErrorNo = ClientError.CMDERR_SUCCESS;
                    banned_ipaddr.add(szIPAddress);
                }
                else {
                    lpClientErrorMsg.nErrorNo = ClientError.CMDERR_NOT_AUTHORIZED;
                    lpClientErrorMsg.szErrorMsg = "Hell no!";
                }
            }

            public void userDeleteServerBan(ClientErrorMsg lpClientErrorMsg,
                                            User lpUser, String szIPAddress) {

                UserAccount ua = getUserAccount(lpUser.szUsername);

                if((ua.uUserRights & UserRight.USERRIGHT_BAN_USERS) != 0) {
                    lpClientErrorMsg.nErrorNo = ClientError.CMDERR_SUCCESS;
                    banned_ipaddr.remove(szIPAddress);
                }
                else {
                    lpClientErrorMsg.nErrorNo = ClientError.CMDERR_NOT_AUTHORIZED;
                    lpClientErrorMsg.szErrorMsg = "Hell no!";
                }
            }


        };

    Vector<Integer> logevents = new Vector<>();

    ServerLogger logger = new ServerLogger() {

            public void userConnected(User lpUser) {
                String str = String.format("User with IP-address %s connected",
                                           lpUser.szIPAddress);
                System.out.println(str);
                logevents.add(ServerLogEvent.SERVERLOGEVENT_USER_CONNECTED);
            }

            public void userLoggedIn(User lpUser) {
                String str = String.format("User %s logged in with nickname %s",
                                           lpUser.szUsername, lpUser.szNickname);
                System.out.println(str);
            }

            public void userLoggedOut(User lpUser) {
                String str = String.format("User %s logged out",
                                           lpUser.szUsername);
                System.out.println(str);
            }

            public void userDisconnected(User lpUser) {
                String str = String.format("User %s disconnected",
                                           lpUser.szUsername);
                System.out.println(str);
            }

            public void userTimedout(User lpUser) {
                String str = String.format("User %s timed out",
                                           lpUser.szUsername);
                System.out.println(str);
            }

            public void userKicked(User lpKicker, User lpKickee, Channel lpChannel) {
                String str = String.format("User %s kicked by %s",
                                           lpKicker != null? lpKicker.szUsername : "unknown", lpKickee.szUsername);
                System.out.println(str);
            }

            public void userBanned(User lpBanner, User lpBanee, Channel lpChannel) {
                String str = String.format("User %s banned by %s",
                                           lpBanner.szUsername, lpBanee.szUsername);
                System.out.println(str);
            }

            public void userUnbanned(User lpUnbanner, String szIPAddress) {
                String str = String.format("User %s unbanned IP-address %s",
                                           lpUnbanner.szUsername, szIPAddress);
                System.out.println(str);
            }

            public void userUpdated(User lpUser) {
                String str = String.format("User %s updated properties",
                                           lpUser.szUsername);
                System.out.println(str);
            }

            public void userJoinedChannel(User lpUser, Channel lpChannel) {
                String str = String.format("User %s join channel #%d",
                                           lpUser.szUsername, lpChannel.nChannelID);
                System.out.println(str);
            }

            public void userLeftChannel(User lpUser, Channel lpChannel) {
                String str = String.format("User %s left channel #%d",
                                           lpUser.szUsername, lpChannel.nChannelID);
                System.out.println(str);
            }

            public void userMoved(User lpMover, User lpMovee) {
                String str = String.format("User %s moved %s to channel #%d",
                                           lpMover.szUsername, lpMovee.szUsername,
                                           lpMovee.nChannelID);
                System.out.println(str);
            }

            public void userTextMessage(User lpUser, TextMessage lpTextMessage) {
                String str = String.format("User %s sent text message",
                                           lpUser.szUsername);
                System.out.println(str);
            }

            public void channelCreated(Channel lpChannel, User lpUser) {
                String str = String.format("Channel #%d created",
                                           lpChannel.nChannelID);
                System.out.println(str);
            }

            public void channelUpdated(Channel lpChannel, User lpUser) {
                String str = String.format("Channel #%d updated",
                                           lpChannel.nChannelID);
                System.out.println(str);
            }

            public void channelRemoved(Channel lpChannel, User lpUser) {
                String str = String.format("Channel #%d removed",
                                           lpChannel.nChannelID);
                System.out.println(str);
            }

            public void fileUploaded(RemoteFile lpRemoteFile, User lpUser) {
                String str = String.format("File %s uploaded to channel #%d",
                                           lpRemoteFile.szFileName, lpRemoteFile.nChannelID);
                System.out.println(str);
            }

            public void fileDownloaded(RemoteFile lpRemoteFile, User lpUser) {
                String str = String.format("File %s downloaded to channel #%d",
                                           lpRemoteFile.szFileName, lpRemoteFile.nChannelID);
                System.out.println(str);
            }

            public void fileDelete(RemoteFile lpRemoteFile, User lpUser) {
                String str = String.format("File %s deleted from channel #%d",
                                           lpRemoteFile.szFileName, lpRemoteFile.nChannelID);
                System.out.println(str);
            }

            public void serverUpdated(ServerProperties lpServerProperties, User lpUser) {
                String str = String.format("Server properties updated, name is now %s",
                                           lpServerProperties.szServerName);
                System.out.println(str);
            }

            public void serverSavedConfig(User lpUser) {
                String str = String.format("Server settings saved");
                System.out.println(str);
            }
        };

    @Test
    public void testUserLogin() {

        UserAccount useraccount = new UserAccount();

        useraccount.szUsername = "guest";
        useraccount.szPassword = "guest";
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_TRANSMIT_VOICE;

        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        TeamTalkBase client1 = newClientInstance();

        connect(server, client1);

        ServerInterleave interleave = new RunServer(server);

        int cmdid = client1.doLogin(getTestMethodName(), useraccount.szUsername, useraccount.szPassword);
        assertTrue(cmdid > 0, "Login client");

        TTMessage msg = new TTMessage();
        //check that the client gets back the same user account we created in the server
        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDIN, DEF_WAIT, msg, interleave), "wait login");
        assertEquals(useraccount.szUsername, msg.useraccount.szUsername, "Account identity");
        assertEquals(useraccount.uUserType, msg.useraccount.uUserType, "Account type");
        assertEquals(useraccount.uUserRights, msg.useraccount.uUserRights, "Account rights");
        assertEquals(useraccount.szNote, msg.useraccount.szNote, "Account note");

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave), "wait success");
        assertEquals(cmdid, msg.nSource, "Login success");
    }

    @Test
    public void testCreateAccount() {

        TeamTalkSrv server = newServerInstance();
        TeamTalkBase client1 = newClientInstance();

        ServerInterleave interleave = new RunServer(server);

        connect(server, client1);
        login(server, client1, getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        int cmdid;
        TTMessage msg = new TTMessage();

        UserAccount useraccount = new UserAccount();

        useraccount.szUsername = "guest";
        useraccount.szPassword = "guest";
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_TRANSMIT_VOICE;

        cmdid = client1.doNewUserAccount(useraccount);
        assertTrue(cmdid > 0, "New account cmd");

        int n_accounts = useraccounts.size();

        waitCmdSuccess(client1, cmdid, 1000, interleave);

        assertEquals(useraccounts.size(), n_accounts + 1, "One more account");

        UserAccount srv_ua = getUserAccount(useraccount.szUsername);
        assertEquals(srv_ua.szUsername, useraccount.szUsername, "Account identity");
        assertEquals(srv_ua.uUserType, useraccount.uUserType, "Account type");
        assertEquals(srv_ua.uUserRights, useraccount.uUserRights, "Account rights");
        assertEquals(srv_ua.szNote, useraccount.szNote, "Account note");
    }

    @Test
    public void testDeleteAccount() {

        UserAccount useraccount = new UserAccount();

        useraccount.szUsername = "guest";
        useraccount.szPassword = "guest";
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_TRANSMIT_VOICE;

        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        TeamTalkBase client1 = newClientInstance();
        TeamTalkBase client2 = newClientInstance();

        ServerInterleave interleave = new RunServer(server);

        connect(server, client1);
        connect(server, client2);

        int cmdid = client1.doLogin(getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);
        assertTrue(cmdid > 0, "Login client1");

        cmdid = client2.doLogin(getTestMethodName(), useraccount.szUsername, useraccount.szPassword);
        assertTrue(cmdid > 0, "Login client2");

        TTMessage msg = new TTMessage();
        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave), "wait success1");
        assertTrue(waitForEvent(client2, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave), "wait success2");

        int n_accounts = useraccounts.size();
        cmdid = client1.doDeleteUserAccount(ADMIN_USERNAME);

        waitCmdSuccess(client1, cmdid, 1000, interleave);

        assertEquals(useraccounts.size(), n_accounts - 1, "One less account");

        cmdid = client2.doDeleteUserAccount(ADMIN_USERNAME);

        waitCmdError(client2, cmdid, 1000, interleave);
    }

    @Test
    public void testBanUser() {

        UserAccount useraccount = new UserAccount();

        useraccount.szUsername = "guest";
        useraccount.szPassword = "guest";
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_TRANSMIT_VOICE;

        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        TeamTalkBase client1 = newClientInstance();
        TeamTalkBase client2 = newClientInstance();

        ServerInterleave interleave = new RunServer(server);
        connect(server, client1);
        connect(server, client2);

        int cmdid = client1.doLogin(getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);
        assertTrue(cmdid > 0, "Login client1");

        cmdid = client2.doLogin(getTestMethodName(), useraccount.szUsername, useraccount.szPassword);
        assertTrue(cmdid > 0, "Login client2");

        TTMessage msg = new TTMessage();
        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave), "wait success1");
        assertTrue(waitForEvent(client2, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave), "wait success2");

        int n_banned = banned_ipaddr.size();
        cmdid = client1.doBanUser(client2.getMyUserID(), 0);

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave), "wait success1");

        assertEquals(n_banned + 1, banned_ipaddr.size(), "Banned user");

        cmdid = client1.doUnBanUser(banned_ipaddr.get(0), 0);

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave), "wait success1");

        assertEquals(n_banned, banned_ipaddr.size(), "Banned user");

        cmdid = client1.doBanIPAddress("11.22.33.44", 0);

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave), "wait success1");

        assertEquals(n_banned + 1, banned_ipaddr.size(), "Banned user");

    }

    @Test
    public void testSystemID() {
        TeamTalkSrv server = newServerInstance("foobar");

        ServerInterleave interleave = new RunServer(server);

        TeamTalkBase client = newClientInstance();

        connect(server, client, "foobar");

        int cmdid = client.doLoginEx(getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD, "myclientname");
        assertTrue(cmdid > 0, "Login client");

        TTMessage msg = new TTMessage();
        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave), "wait success");

        User user = new User();
        assertTrue(client.getUser(client.getMyUserID(), user), "Get user");

        assertEquals("myclientname", user.szClientName, "clientname set");

    }

    @Test
    public void testWrongSystemID() {
        TeamTalkSrv server = newServerInstance("foobar");

        final ServerInterleave interleave = new RunServer(server);

        final TeamTalkBase client = newClientInstance();

        assertTrue(client.connect(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED), "Connect");

        waitForEvent(client, ClientEvent.CLIENTEVENT_CMD_ERROR, 1000, interleave);
    }

    @Test
    public void testMoveUser() {
        UserAccount useraccount = new UserAccount();

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_MULTI_LOGIN |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
            UserRight.USERRIGHT_MOVE_USERS;

        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        TeamTalkBase client1 = newClientInstance();
        TeamTalkBase client2 = newClientInstance();

        ServerInterleave interleave = new RunServer(server);

        connect(server, client1);
        connect(server, client2);
        login(server, client1, NICKNAME, USERNAME, PASSWORD);
        login(server, client2, NICKNAME, USERNAME, PASSWORD);

        Channel chan = new Channel();
        chan.nChannelID = 0;
        chan.nParentID = client1.getRootChannelID();
        chan.nMaxUsers = 10;
        chan.szName = "foo";
        chan.audiocodec = new AudioCodec(true);
        chan.audiocfg = new AudioConfig(true);

        int cmdid = client1.doJoinChannel(chan);

        assertTrue(waitCmdSuccess(client1, cmdid, DEF_WAIT, interleave), "join channel");

        cmdid = client2.doJoinChannelByID(client2.getRootChannelID(), "");

        assertTrue(waitCmdSuccess(client2, cmdid, DEF_WAIT, interleave), "join channel");

        cmdid = client1.doPing();

        assertTrue(waitCmdComplete(client1, cmdid, DEF_WAIT, interleave), "drain client1");

        cmdid = client2.doMoveUser(client1.getMyUserID(), client2.getMyChannelID());

        assertTrue(waitCmdSuccess(client2, cmdid, DEF_WAIT, interleave), "move user");

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_USER_JOINED, DEF_WAIT, interleave), "wait move");

        assertEquals(client1.getMyChannelID(), client2.getMyChannelID(), "same channel");
    }

    @Test
    public void testMoveUserNoLogin() {
        UserAccount useraccount = new UserAccount();

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_MULTI_LOGIN |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
            UserRight.USERRIGHT_MOVE_USERS;

        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        TeamTalkBase client1 = newClientInstance();
        TeamTalkBase client2 = newClientInstance();

        ServerInterleave interleave = new RunServer(server);

        connect(server, client1);
        login(server, client1, NICKNAME, USERNAME, PASSWORD);
        joinRoot(server, client1);

        connect(server, client2);
        int cmdid = client1.doMoveUser(client2.getMyUserID(), client1.getMyChannelID());
        assertTrue(waitCmdError(client1, cmdid, DEF_WAIT, interleave), "move user");
    }

    @Test
    public void testChannelUpdates() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS;
        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        TeamTalkBase client1 = newClientInstance();

        ServerInterleave interleave = new RunServer(server);

        connect(server, client1);
        login(server, client1, NICKNAME, USERNAME, PASSWORD);

        Channel chan = new Channel();
        chan.nChannelID = 2;
        chan.nParentID = 1;
        chan.nMaxUsers = 10;
        chan.szName = "foo";
        chan.audiocodec = new AudioCodec(true);
        chan.audiocfg = new AudioConfig(true);

        assertEquals(ClientError.CMDERR_SUCCESS, server.makeChannel(chan), "Make sub channel");

        interleave.interleave();

        chan.szName = "foo2";
        assertEquals(ClientError.CMDERR_SUCCESS, server.updateChannel(chan), "Update sub channel");

        interleave.interleave();

        assertEquals(ClientError.CMDERR_SUCCESS, server.removeChannel(chan.nChannelID), "Remove sub channel");

        interleave.interleave();
    }

    void compareChannels(Channel chan1, Channel chan2, boolean joincheck) {
        assertEquals(chan1.nParentID, chan2.nParentID, "parent");
        assertEquals(chan1.szName, chan2.szName, "name");
        assertEquals(chan1.uChannelType, chan2.uChannelType, "chan type");
        assertEquals(chan1.szPassword, chan2.szPassword, "password");
        assertEquals(chan1.szOpPassword, chan2.szOpPassword, "opassword");
        assertEquals(chan1.szTopic, chan2.szTopic, "topic");
        assertEquals(chan1.nUserData, chan2.nUserData, "userdata");
        if (joincheck) {
            assertEquals(chan1.nDiskQuota, chan2.nDiskQuota, "diskquota");
            assertEquals(chan1.nMaxUsers, chan2.nMaxUsers, "maxusers");
        }
        assertEquals(chan1.nTransmitUsersQueueDelayMSec, chan2.nTransmitUsersQueueDelayMSec, "transmit queue delay");
        assertArrayEquals(chan1.transmitUsers, chan2.transmitUsers, "transmitUsers");
        assertEquals(chan1.nTimeOutTimerVoiceMSec, chan2.nTimeOutTimerVoiceMSec, "tot voice");
        assertEquals(chan1.nTimeOutTimerMediaFileMSec, chan2.nTimeOutTimerMediaFileMSec, "tot mf");
        assertEquals(chan1.audiocfg.bEnableAGC, chan2.audiocfg.bEnableAGC, "agc");
        assertEquals(chan1.audiocfg.nGainLevel, chan2.audiocfg.nGainLevel, "gain");
        assertEquals(chan1.audiocodec.nCodec, chan2.audiocodec.nCodec, "codec");
        assertEquals(chan1.audiocodec.opus.nSampleRate, chan2.audiocodec.opus.nSampleRate, "samplerate");
        assertEquals(chan1.audiocodec.opus.nChannels, chan2.audiocodec.opus.nChannels, "channels");
        assertEquals(chan1.audiocodec.opus.nApplication, chan2.audiocodec.opus.nApplication, "app");
        assertEquals(chan1.audiocodec.opus.nComplexity, chan2.audiocodec.opus.nComplexity, "complex");
        assertEquals(chan1.audiocodec.opus.bFEC, chan2.audiocodec.opus.bFEC, "fec");
        assertEquals(chan1.audiocodec.opus.bDTX, chan2.audiocodec.opus.bDTX, "dtx");
        assertEquals(chan1.audiocodec.opus.nBitRate, chan2.audiocodec.opus.nBitRate, "br");
        assertEquals(chan1.audiocodec.opus.bVBR, chan2.audiocodec.opus.bVBR, "vbr");
        assertEquals(chan1.audiocodec.opus.bVBRConstraint, chan2.audiocodec.opus.bVBRConstraint, "constrai");
        assertEquals(chan1.audiocodec.opus.nTxIntervalMSec, chan2.audiocodec.opus.nTxIntervalMSec, "txinterval");
        assertEquals(chan1.audiocodec.opus.nFrameSizeMSec, chan2.audiocodec.opus.nFrameSizeMSec, "framesize");
    }

    @Test
    public void testChannelProperties() {

        TeamTalkSrv server = newServerInstance();
        ServerInterleave interleave = new RunServer(server);

        TeamTalkBase admin = newClientInstance();

        connect(server, admin);
        login(server, admin, getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        Channel chan = buildDefaultChannel(admin, getTestMethodName());
        chan.uChannelType |= ChannelType.CHANNEL_PERMANENT;
        chan.uChannelType |= ChannelType.CHANNEL_SOLO_TRANSMIT;
        chan.uChannelType |= ChannelType.CHANNEL_CLASSROOM;
        chan.uChannelType |= ChannelType.CHANNEL_OPERATOR_RECVONLY;
        chan.uChannelType |= ChannelType.CHANNEL_NO_VOICEACTIVATION;
        chan.uChannelType |= ChannelType.CHANNEL_NO_RECORDING;
        chan.uChannelType |= ChannelType.CHANNEL_HIDDEN;
        chan.szPassword = "password";
        chan.szOpPassword = "oppassword";
        chan.szTopic = "topic";
        chan.nUserData = 123;
        chan.nDiskQuota = 334455;
        chan.nMaxUsers = 234;
        chan.nTransmitUsersQueueDelayMSec = 676;
        chan.transmitUsers[0][0] = admin.getMyUserID();
        chan.transmitUsers[0][1] = StreamType.STREAMTYPE_VOICE |
            StreamType.STREAMTYPE_MEDIAFILE |
            StreamType.STREAMTYPE_CHANNELMSG |
            StreamType.STREAMTYPE_VIDEOCAPTURE |
            StreamType.STREAMTYPE_DESKTOP;
        chan.nTimeOutTimerVoiceMSec = 51;
        chan.nTimeOutTimerMediaFileMSec = 52;

        TTMessage msg = new TTMessage();
        int cmdid = admin.doMakeChannel(chan);
        assertTrue(waitForEvent(admin, ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW, DEF_WAIT, msg, interleave), "new channel");
        compareChannels(chan, msg.channel, true);
        assertTrue(waitCmdComplete(admin, cmdid, DEF_WAIT, interleave), "done");

        assertTrue(waitCmdSuccess(admin, admin.doRemoveChannel(msg.channel.nChannelID), DEF_WAIT, interleave), "Remove channel");

        cmdid = admin.doJoinChannel(chan);
        assertTrue(waitForEvent(admin, ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW, DEF_WAIT, msg, interleave), "new join channel");
        compareChannels(chan, msg.channel, false);
        assertTrue(waitCmdComplete(admin, cmdid, DEF_WAIT, interleave), "done join");

        assertTrue(waitCmdSuccess(admin, admin.doRemoveChannel(msg.channel.nChannelID), DEF_WAIT, interleave), "Remove channel");

        Channel chan2 = buildDefaultChannel(admin, getTestMethodName()+"123", Codec.SPEEX_CODEC);
        cmdid = admin.doMakeChannel(chan2);
        assertTrue(waitForEvent(admin, ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW, DEF_WAIT, msg, interleave), "new channel");
        assertTrue(waitCmdComplete(admin, cmdid, DEF_WAIT, interleave), "done");
        chan.nChannelID = msg.channel.nChannelID;
        chan.uChannelType &= ~ChannelType.CHANNEL_HIDDEN;
        cmdid = admin.doUpdateChannel(chan);
        assertTrue(waitForEvent(admin, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg, interleave), "update channel");
        compareChannels(chan, msg.channel, true);

        assertTrue(waitCmdSuccess(admin, admin.doRemoveChannel(chan.nChannelID), DEF_WAIT, interleave), "Remove chan2");
    }

    @Test
    public void testKickUser() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS;
        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        ServerInterleave interleave = new RunServer(server);

        TeamTalkBase client1 = newClientInstance();
        connect(server, client1);
        login(server, client1, NICKNAME, USERNAME, PASSWORD);
        joinRoot(server, client1);

        TeamTalkBase client2 = newClientInstance();
        connect(server, client2);
        login(server, client2, NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);

        int cmdid = client2.doKickUser(client1.getMyUserID(), client1.getMyChannelID());
        assertTrue(waitCmdSuccess(client2, cmdid, DEF_WAIT, interleave), "kick success");

        Channel chan = new Channel();
        chan.nChannelID = 2;
        chan.nParentID = 1;
        chan.nMaxUsers = 10;
        chan.szName = "foo";
        chan.audiocodec = new AudioCodec(true);
        chan.audiocfg = new AudioConfig(true);

        assertEquals(ClientError.CMDERR_SUCCESS, server.makeChannel(chan), "Make sub channel");

        waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW, DEF_WAIT, interleave);

        cmdid = client1.doJoinChannelByID(chan.nChannelID, "");
        assertTrue(cmdid>0, "Join new channel");

        assertTrue(waitCmdSuccess(client1, cmdid, DEF_WAIT, interleave), "join channel");

        assertEquals(ClientError.CMDERR_SUCCESS, server.removeChannel(chan.nChannelID), "Remove sub channel");

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_CHANNEL_REMOVE,
                                DEF_WAIT, new TTMessage(), interleave), "remove channel event");

        assertEquals(0, client1.getMyChannelID(), "No channel");

        assertTrue(client2.doKickUser(client1.getMyUserID(), 0)>0, "Kick cmd");

        interleave.interleave();
    }

    @Test
    public void testSendMessage() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS;
        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        ServerInterleave interleave = new RunServer(server);

        TeamTalkBase client1 = newClientInstance();
        connect(server, client1);
        login(server, client1, NICKNAME, USERNAME, PASSWORD);
        joinRoot(server, client1);

        TextMessage textmsg = new TextMessage();

        // user 2 user message
        textmsg.nMsgType = TextMsgType.MSGTYPE_USER;
        textmsg.nFromUserID = 0;
        textmsg.szFromUsername = "hest";
        textmsg.nToUserID = client1.getMyUserID();
        textmsg.nChannelID = 0;
        textmsg.szMessage = "this is my message";

        assertEquals(0, server.sendTextMessage(textmsg), "send message");

        TTMessage msg = new TTMessage();
        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_USER_TEXTMSG, DEF_WAIT, msg, interleave), "wait text msg");

        assertEquals(textmsg.nFromUserID, msg.textmessage.nFromUserID, "from id");
        assertEquals(textmsg.szMessage, msg.textmessage.szMessage, "msg content");

        // custom message

        textmsg.nMsgType = TextMsgType.MSGTYPE_CUSTOM;
        textmsg.nToUserID = client1.getMyUserID();

        assertEquals(0, server.sendTextMessage(textmsg), "send message");

        // channel message

        textmsg.nMsgType = TextMsgType.MSGTYPE_CHANNEL;
        textmsg.nChannelID = client1.getMyChannelID();
        textmsg.nToUserID = 0;

        assertEquals(0, server.sendTextMessage(textmsg), "send message");

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_USER_TEXTMSG, DEF_WAIT, msg, interleave), "wait text msg");

        assertEquals(textmsg.nFromUserID, msg.textmessage.nFromUserID, "from id");
        assertEquals(textmsg.szMessage, msg.textmessage.szMessage, "msg content");

        // broadcast mesage
        textmsg.nMsgType = TextMsgType.MSGTYPE_BROADCAST;
        textmsg.nChannelID = 0;
        textmsg.nToUserID = 0;

        assertEquals(0, server.sendTextMessage(textmsg), "send message");

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_USER_TEXTMSG, DEF_WAIT, msg, interleave));

        assertEquals(textmsg.nFromUserID, msg.textmessage.nFromUserID, "from id");
        assertEquals(textmsg.szMessage, msg.textmessage.szMessage, "msg content");
    }

    @Test
    public void testChangeNicknameStatus() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS;
        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        ServerInterleave interleave = new RunServer(server);

        TeamTalkBase client1 = newClientInstance();
        connect(server, client1);
        login(server, client1, NICKNAME, USERNAME, PASSWORD);
        joinRoot(server, client1);

        int cmdid = client1.doChangeNickname("This is crap");
        assertTrue(cmdid>0, "Issue change nickname");

        TTMessage msg = new TTMessage();
        assertTrue(waitCmdError(client1, cmdid, DEF_WAIT, msg, interleave), "Change nick error");

        assertEquals(4567, msg.clienterrormsg.nErrorNo, "Error message");

        cmdid = client1.doChangeStatus(45, "This is also crap");
        assertTrue(cmdid>0, "Issue change status");

        msg = new TTMessage();
        assertTrue(waitCmdError(client1, cmdid, DEF_WAIT, msg, interleave), "Change status error");
        assertEquals(4568, msg.clienterrormsg.nErrorNo, "Error message");
    }

    private void uploadDownloadTest(TeamTalkSrv server, UserAccount ua,
                                    String NICKNAME, int filesize) {

        ServerInterleave interleave = new RunServer(server);

        TeamTalkBase client1 = newClientInstance();
        connect(server, client1);
        login(server, client1, NICKNAME, ua.szUsername, ua.szPassword);
        joinRoot(server, client1);

        String uploadfilename = "uploadfile.txt";
        String downloadfilename = "downloadfile.txt";

        // writing string to a file encoded as modified UTF-8
        try {
            int size = filesize, written = 0;
            DataOutputStream dataOut = new DataOutputStream(new FileOutputStream(uploadfilename));
            byte c = 'A';
            byte[] buff = new byte[1024];
            while(written < size) {
                int todo = Math.min(buff.length, size - written);
                for(int i=0;i<todo;i++) {
                    buff[i] = c++;
                    if(c > 'z') {
                        c = 'A';
                    }
                }
                dataOut.write(buff, 0, todo);
                written += todo;
            }
            dataOut.close();
        }
        catch(IOException e) {
            assertTrue(false, "Failed to create file.txt: " + e);
        }

        int cmdid = client1.doSendFile(client1.getMyChannelID(), uploadfilename);
        assertTrue(cmdid>0, "upload issued");

        TTMessage msg = new TTMessage();
        assertTrue(waitCmdSuccess(client1, cmdid, DEF_WAIT, interleave), "Send success");

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_FILE_NEW, DEF_WAIT, msg, interleave), "file upload done");

        RemoteFile fileinfo = msg.remotefile;

        cmdid = client1.doRecvFile(client1.getMyChannelID(), fileinfo.nFileID, downloadfilename);
        assertTrue(cmdid>0, "download issued");

        assertTrue(waitCmdSuccess(client1, cmdid, DEF_WAIT, interleave), "download success");

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_FILETRANSFER, DEF_WAIT, msg, interleave), "file download begin event");
        assertEquals(FileTransferStatus.FILETRANSFER_ACTIVE, msg.filetransfer.nStatus, "file download begin");

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_FILETRANSFER, DEF_WAIT, msg, interleave), "file download end event");

        assertEquals(FileTransferStatus.FILETRANSFER_FINISHED, msg.filetransfer.nStatus, "file download finished");
        cmdid = client1.doDeleteFile(client1.getMyChannelID(), fileinfo.nFileID);
        assertTrue(cmdid>0, "delete issued");

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_FILE_REMOVE, DEF_WAIT, msg, interleave), "file rm");

        try {
            DataInputStream uploaded = new DataInputStream(new FileInputStream(uploadfilename));
            DataInputStream downloaded = new DataInputStream(new FileInputStream(downloadfilename));
            byte[] upbuff = new byte[1024], downbuff = new byte[1024];
            int inup = 1, indown = 1;
            while(inup > 0 || indown > 0) {
                inup = uploaded.read(upbuff);
                indown = downloaded.read(downbuff);
                assertTrue(Arrays.equals(upbuff, downbuff), "uploaded same as downloaded");
            }
        }
        catch(IOException e) {
            assertTrue(false, "Failed to compare file" + e);
        }
    }

    @Test
    public void testFileUpload() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_UPLOAD_FILES | UserRight.USERRIGHT_DOWNLOAD_FILES | UserRight.USERRIGHT_MULTI_LOGIN;
        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();

        uploadDownloadTest(server, useraccount, NICKNAME, 7);
        uploadDownloadTest(server, useraccount, NICKNAME, 77);
        uploadDownloadTest(server, useraccount, NICKNAME, 777);
        uploadDownloadTest(server, useraccount, NICKNAME, 7777);
        uploadDownloadTest(server, useraccount, NICKNAME, 777777);
        uploadDownloadTest(server, useraccount, NICKNAME, 7777777);
        uploadDownloadTest(server, useraccount, NICKNAME, 77777777);
    }

    @Test
    public void testFileAccessOutsideChannel() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount ua = new UserAccount();
        ua.szUsername = USERNAME;
        ua.szPassword = PASSWORD;
        ua.uUserType = UserType.USERTYPE_DEFAULT;
        ua.szNote = "An example user account with limited user-rights";
        ua.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_UPLOAD_FILES | UserRight.USERRIGHT_DOWNLOAD_FILES | UserRight.USERRIGHT_MULTI_LOGIN | UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        useraccounts.add(ua);

        TeamTalkSrv server = newServerInstance();
        ServerInterleave interleave = new RunServer(server);

        TeamTalkBase client1 = newClientInstance();
        connect(server, client1);
        login(server, client1, NICKNAME, ua.szUsername, ua.szPassword);
        Channel chan = buildDefaultChannel(client1, getTestMethodName() + client1.getMyUserID());
        assertTrue(waitCmdSuccess(client1, client1.doJoinChannel(chan), DEF_WAIT, interleave), "client1 join channel");

        TeamTalkBase client2 = newClientInstance();
        connect(server, client2);
        login(server, client2, NICKNAME, ua.szUsername, ua.szPassword);
        chan = buildDefaultChannel(client2, getTestMethodName() + client2.getMyUserID());
        assertTrue(waitCmdSuccess(client2, client2.doJoinChannel(chan), DEF_WAIT, interleave), "client2 join channel");

        String uploadfilename = getTestMethodName() + ".txt";
        try {
            DataOutputStream dataOut = new DataOutputStream(new FileOutputStream(uploadfilename));
            byte[] buff = new byte[1024*777];
            dataOut.write(buff);
            dataOut.close();
        }
        catch(IOException e) {
            assertTrue(false, "Failed to create file.txt: " + e);
        }

        int cmdid = client1.doSendFile(client2.getMyChannelID(), uploadfilename);
        assertTrue(cmdid>0, "upload issued to client2 channel");

        TTMessage msg = new TTMessage();
        assertTrue(waitCmdError(client1, cmdid, DEF_WAIT, interleave), "Send outside channel failed");

        cmdid = client2.doSendFile(client2.getMyChannelID(), uploadfilename);
        assertTrue(cmdid>0, "upload issued to client2 channel");

        assertTrue(waitForEvent(client2, ClientEvent.CLIENTEVENT_CMD_FILE_NEW, DEF_WAIT, msg, interleave), "file upload done");

        RemoteFile fileinfo = msg.remotefile;

        assertFalse(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_FILE_NEW, 0, msg, interleave), "file upload not available to client1");

        cmdid = client1.doRecvFile(client2.getMyChannelID(), fileinfo.nFileID, uploadfilename);
        assertEquals(-1, cmdid, "file not found");
    }

    @Test
    public void testDnsResolve() throws Exception {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_MULTI_LOGIN;
        useraccounts.add(useraccount);

        for (InetAddress a : InetAddress.getAllByName("localhost"))
        {
            System.out.println(a + " Binding to " + a.getHostAddress());
            if (a.isLinkLocalAddress())
                continue; // e.g. cannot bind to fe80:0:0:0:0:0:0:1%1

            TeamTalkSrv s = newServerInstance(SYSTEMID, a.getHostAddress());
            TeamTalkBase c = newClientInstance();
            connect(s, c, SYSTEMID, "localhost", TCPPORT, UDPPORT);
            login(s, c, NICKNAME, USERNAME, PASSWORD);
            joinRoot(s, c);
            c.disconnect();
            s.stopServer();
        }
    }

    @Test
    public void testIPv6Addr() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_MULTI_LOGIN;
        useraccounts.add(useraccount);

        TeamTalkSrv s = newServerInstance(SYSTEMID, "::1");
        TeamTalkBase c = newClientInstance();
        connect(s, c, SYSTEMID, "::1", TCPPORT, UDPPORT);
        login(s, c, NICKNAME, USERNAME, PASSWORD);
        joinRoot(s, c);
        c.disconnect();
        s.stopServer();
    }

    @Test
    public void testIPv6Hostname() throws Exception {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_MULTI_LOGIN;
        useraccounts.add(useraccount);

        InetAddress inetAddress = InetAddress.getByName("::1");
        String hostname = inetAddress.getHostName();

        TeamTalkSrv s = newServerInstance(SYSTEMID, "::1");
        TeamTalkBase c = newClientInstance();
        connect(s, c, SYSTEMID, hostname, TCPPORT, UDPPORT);
        login(s, c, NICKNAME, USERNAME, PASSWORD);
        joinRoot(s, c);
        c.disconnect();
        s.stopServer();
    }

    @Test
    public void testIPv4v6HostnameResolveMultiple() throws Exception {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_MULTI_LOGIN;
        useraccounts.add(useraccount);

        {
            TeamTalkSrv srvIPv6 = newServerInstance(SYSTEMID, "::1");
            TeamTalkBase c = newClientInstance();
            connect(srvIPv6, c, SYSTEMID, "localhost", TCPPORT, UDPPORT);
            login(srvIPv6, c, NICKNAME, USERNAME, PASSWORD);
            joinRoot(srvIPv6, c);
            c.disconnect();
            srvIPv6.stopServer();
        }
        {
            TeamTalkSrv srvIPv4 = newServerInstance(SYSTEMID, "127.0.0.1");
            TeamTalkBase c = newClientInstance();
            connect(srvIPv4, c, SYSTEMID, "localhost", TCPPORT, UDPPORT);
            login(srvIPv4, c, NICKNAME, USERNAME, PASSWORD);
            joinRoot(srvIPv4, c);
            c.disconnect();
            srvIPv4.stopServer();
        }
    }

    @Test
    public void testInterceptVoice() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_TRANSMIT_VOICE;
        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        TeamTalkBase admin = newClientInstance();

        ServerInterleave interleave = new RunServer(server);

        initSound(admin);
        connect(server, admin);
        login(server, admin, "admin", ADMIN_USERNAME, ADMIN_PASSWORD);
        joinRoot(server, admin);

        TeamTalkBase client = newClientInstance();
        initSound(client);
        connect(server, client);
        login(server, client, NICKNAME, USERNAME, PASSWORD);

        Channel chan = buildDefaultChannel(client, "Some channel");
        assertTrue(waitCmdSuccess(client, client.doJoinChannel(chan), DEF_WAIT, interleave), "join channel");

        assertTrue(client.enableVoiceTransmission(true), "vox");

        assertTrue(admin.enableAudioBlockEvent(client.getMyUserID(), StreamType.STREAMTYPE_VOICE, true), "enable aud cb");

        assertFalse(waitForEvent(admin, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000, interleave), "no voice audioblock");

        assertTrue(waitCmdSuccess(admin, admin.doSubscribe(client.getMyUserID(), Subscription.SUBSCRIBE_INTERCEPT_VOICE), DEF_WAIT, interleave));

        assertTrue(waitForEvent(admin, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, interleave), "voice audioblock");

    }

    @Test
    public void testTransmitUsers() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                                  UserRight.USERRIGHT_VIEW_ALL_USERS |
                                  UserRight.USERRIGHT_TEXTMESSAGE_CHANNEL |
                                  UserRight.USERRIGHT_TRANSMIT_VOICE;
        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        ServerInterleave interleave = new RunServer(server);

        TeamTalkBase client = newClientInstance();
        initSound(client);
        connect(server, client);
        login(server, client, NICKNAME, USERNAME, PASSWORD);

        Channel chan = buildDefaultChannel(client, "Classroom channel");
        chan.uChannelType |= ChannelType.CHANNEL_CLASSROOM;
        assertTrue(waitCmdSuccess(client, client.doJoinChannel(chan), DEF_WAIT, interleave), "join channel");

        assertTrue(client.getChannel(client.getMyChannelID(), chan), "get new chan");

        assertTrue(waitCmdSuccess(client, client.doSubscribe(client.getMyUserID(),
                                            Subscription.SUBSCRIBE_VOICE), DEF_WAIT, interleave), "subscribe voice");

        assertTrue(client.enableVoiceTransmission(true));

        assertTrue(client.enableAudioBlockEvent(client.getMyUserID(), StreamType.STREAMTYPE_VOICE, true), "enable aud cb");

        assertFalse(waitForEvent(client, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000, interleave), "no voice audioblock");

        chan.transmitUsers[0][Constants.TT_TRANSMITUSERS_USERID_INDEX] = client.getMyUserID();
        chan.transmitUsers[0][Constants.TT_TRANSMITUSERS_STREAMTYPE_INDEX] = StreamType.STREAMTYPE_VOICE;

        assertTrue(waitCmdSuccess(client, client.doUpdateChannel(chan), DEF_WAIT, interleave), "update channel");

        assertEquals(client.getMyUserID(), chan.transmitUsers[0][Constants.TT_TRANSMITUSERS_USERID_INDEX], "Tx user ID set");
        assertEquals(StreamType.STREAMTYPE_VOICE, chan.transmitUsers[0][Constants.TT_TRANSMITUSERS_STREAMTYPE_INDEX], "Tx streamtype set");

        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, interleave), "voice audioblock");

        // check that cannot send channel-text message in classroom channel
        TextMessage textmsg = new TextMessage();
        textmsg.nMsgType = TextMsgType.MSGTYPE_CHANNEL;
        textmsg.nChannelID = client.getMyChannelID();
        textmsg.nToUserID = 0;
        textmsg.szMessage = "Some message";

        assertTrue(waitCmdError(client, client.doTextMessage(textmsg), DEF_WAIT, interleave), "cannot send chanmsg");

        // unblock classroom channel for channel message
        assertTrue(client.getChannel(client.getMyChannelID(), chan), "get updated chan");
        chan.transmitUsers[0][Constants.TT_TRANSMITUSERS_USERID_INDEX] = client.getMyUserID();
        chan.transmitUsers[0][Constants.TT_TRANSMITUSERS_STREAMTYPE_INDEX] |= StreamType.STREAMTYPE_CHANNELMSG;
        assertTrue(waitCmdSuccess(client, client.doUpdateChannel(chan), DEF_WAIT, interleave), "update channel");

        assertTrue(waitCmdSuccess(client, client.doTextMessage(textmsg), DEF_WAIT, interleave), "can send chanmsg");

        // check that we can block channel message in channel
        chan = buildDefaultChannel(client, "Default channel");
        assertTrue(waitCmdSuccess(client, client.doJoinChannel(chan), DEF_WAIT, interleave), "join channel");
        textmsg.nChannelID = client.getMyChannelID();

        assertTrue(client.getChannel(client.getMyChannelID(), chan), "get chan");
        assertTrue(waitCmdSuccess(client, client.doTextMessage(textmsg), DEF_WAIT, interleave), "can send chanmsg");
        chan.transmitUsers[0][Constants.TT_TRANSMITUSERS_USERID_INDEX] = client.getMyUserID();
        chan.transmitUsers[0][Constants.TT_TRANSMITUSERS_STREAMTYPE_INDEX] |= StreamType.STREAMTYPE_CHANNELMSG;
        assertTrue(waitCmdSuccess(client, client.doUpdateChannel(chan), DEF_WAIT, interleave), "block chanmsg");
        assertTrue(waitCmdError(client, client.doTextMessage(textmsg), DEF_WAIT, interleave), "cannot send chanmsg");

        // unblock channel message type
        chan.transmitUsers[0][Constants.TT_TRANSMITUSERS_USERID_INDEX] = 0;
        chan.transmitUsers[0][Constants.TT_TRANSMITUSERS_STREAMTYPE_INDEX] = StreamType.STREAMTYPE_NONE;
        assertTrue(waitCmdSuccess(client, client.doUpdateChannel(chan), DEF_WAIT, interleave), "unblock chanmsg");
        assertTrue(waitCmdSuccess(client, client.doTextMessage(textmsg), DEF_WAIT, interleave), "can send chanmsg");
    }

    @Test
    public void testClientKeepAlive() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_UPDATE_SERVERPROPERTIES;
        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        ServerInterleave interleave = new RunServer(server);

        TeamTalkBase client = newClientInstance();
        connect(server, client);
        login(server, client, NICKNAME, USERNAME, PASSWORD);

        ClientKeepAlive ka = new ClientKeepAlive();
        assertTrue(client.getClientKeepAlive(ka), "get keepalive");

        ServerProperties srvprop = new ServerProperties();
        assertTrue(client.getServerProperties(srvprop));

        assertEquals(srvprop.nUserTimeout * 1000 / 2, ka.nTcpKeepAliveIntervalMSec, "tcp ping is half of user timeout (default)");

        srvprop.nUserTimeout = 4;
        assertTrue(waitCmdSuccess(client, client.doUpdateServer(srvprop), DEF_WAIT, interleave));

        assertTrue(client.getServerProperties(srvprop));

        assertTrue(client.getClientKeepAlive(ka), "get keepalive");

        assertEquals(srvprop.nUserTimeout * 1000 / 2, ka.nTcpKeepAliveIntervalMSec, "tcp ping is half of user timeout, 4 sec");

        ka.nUdpKeepAliveIntervalMSec = 1;

        assertTrue(client.setClientKeepAlive(ka), "set UDP keepalive");

        assertFalse(waitForEvent(client, ClientEvent.CLIENTEVENT_NONE, 100, interleave));

        ka.nTcpKeepAliveIntervalMSec = 1;

        assertTrue(client.setClientKeepAlive(ka), "set TCP keepalive");

        assertTrue(client.getClientKeepAlive(ka), "get keepalive");

        assertEquals(srvprop.nUserTimeout * 1000 / 2, ka.nTcpKeepAliveIntervalMSec, "ka.nTcpKeepAliveIntervalMSec is read-only");
    }

    @Test
    public void testPeerVerification() {

        if (!ENCRYPTED) {
            System.out.println("Skipped test. Requires encryption");
            return;
        }

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_TRANSMIT_VOICE;

        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        ServerInterleave interleave = new RunServer(server);

        // setup client which will reject server due to invalid server
        // certificate
        final TeamTalkBase client = newClientInstance();

        EncryptionContext context = new EncryptionContext();
        context.szCAFile = CRYPTO_CA_FILE;
        context.bVerifyPeer = true;
        context.nVerifyDepth = 0;
        assertTrue(client.setEncryptionContext(context), "Set client encryption context");

        assertTrue(client.connectSysID(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED, SYSTEMID), "connect call");
        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_CON_FAILED, DEF_WAIT, interleave), "connect failed");

        assertTrue(server.stopServer(), "Stop server");
        assertTrue(client.disconnect(), "Disconnect client");

        // now run server with peer verification, i.e. client has
        // certificate and private key

        EncryptionContext srvcontext = new EncryptionContext();
        srvcontext.szCertificateFile = CRYPTO_SERVER_CERT_FILE;
        srvcontext.szPrivateKeyFile = CRYPTO_SERVER_KEY_FILE;
        srvcontext.szCAFile = CRYPTO_CA_FILE;
        srvcontext.bVerifyPeer = true;
        srvcontext.bVerifyClientOnce = true;
        srvcontext.nVerifyDepth = 0;
        assertTrue(server.setEncryptionContext(srvcontext), "set server encryption context");

        // here we specify client's private key and certificate (for
        // server verification)
        context.szCertificateFile = CRYPTO_CLIENT_CERT_FILE;
        context.szPrivateKeyFile = CRYPTO_CLIENT_KEY_FILE;
        assertTrue(client.setEncryptionContext(context), "Set client encryption context");

        assertTrue(server.startServer(SERVERBINDIP, TCPPORT, UDPPORT, ENCRYPTED), "Start server");

        connect(server, client);
        login(server, client, NICKNAME, USERNAME, PASSWORD);

        assertTrue(server.stopServer(), "Stop server");
        assertTrue(client.disconnect(), "Disconnect client");

        // Now disable server's peer verification but make client
        // verify server's certificate
        srvcontext = new EncryptionContext();
        srvcontext.szCertificateFile = CRYPTO_SERVER_CERT_FILE;
        srvcontext.szPrivateKeyFile = CRYPTO_SERVER_KEY_FILE;
        srvcontext.bVerifyPeer = false;
        srvcontext.bVerifyClientOnce = false;
        srvcontext.nVerifyDepth = 0;
        assertTrue(server.setEncryptionContext(srvcontext), "set server encryption context");

        context = new EncryptionContext();
        context.szCAFile = CRYPTO_CA_FILE;
        context.bVerifyPeer = true;
        context.nVerifyDepth = 0;
        assertTrue(client.setEncryptionContext(context), "Set client encryption context");

        assertTrue(server.startServer(SERVERBINDIP, TCPPORT, UDPPORT, ENCRYPTED), "Start server");

        connect(server, client);
        login(server, client, NICKNAME, USERNAME, PASSWORD);
    }

    @Test
    public void testServerCertExpired() {

        if (!ENCRYPTED) {
            System.out.println("Skipped test. Requires encryption");
            return;
        }

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS;

        useraccounts.add(useraccount);

        EncryptionContext srvcontext = new EncryptionContext();
        srvcontext.szCertificateFile = CRYPTO_SERVER_CERT_EXPIRED_FILE;
        srvcontext.szPrivateKeyFile = CRYPTO_SERVER_KEY_FILE;
        srvcontext.szCAFile = CRYPTO_CA_FILE;
        srvcontext.bVerifyPeer = true;
        srvcontext.bVerifyClientOnce = true;
        srvcontext.nVerifyDepth = 0;

        TeamTalkSrv server = newServerInstance("", "", srvcontext);
        ServerInterleave interleave = new RunServer(server);

        // setup client which will reject server due to invalid server
        // certificate
        final TeamTalkBase client = newClientInstance();

        EncryptionContext context = new EncryptionContext();
        context.szCAFile = CRYPTO_CA_FILE;
        context.szCertificateFile = CRYPTO_CLIENT_CERT_FILE;
        context.szPrivateKeyFile = CRYPTO_CLIENT_KEY_FILE;
        context.bVerifyPeer = true;
        context.nVerifyDepth = 0;
        assertTrue(client.setEncryptionContext(context), "Set client encryption context");

        assertTrue(client.connectSysID(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED, SYSTEMID), "connect call");
        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_CON_CRYPT_ERROR, DEF_WAIT, interleave), "wait crypt error");
        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_CON_FAILED, DEF_WAIT, interleave), "connect failed");
    }

    @Test
    public void testSSLClientCA() {

        if (!ENCRYPTED) {
            System.out.println("Skipped test. Requires encryption");
            return;
        }

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS;

        useraccounts.add(useraccount);

        EncryptionContext srvcontext = new EncryptionContext();
        srvcontext.szCertificateFile = CRYPTO_SERVER_CERT_FILE;
        srvcontext.szPrivateKeyFile = CRYPTO_SERVER_KEY_FILE;
        srvcontext.bVerifyPeer = false;
        srvcontext.bVerifyClientOnce = true;
        srvcontext.nVerifyDepth = 0;

        TeamTalkSrv server = newServerInstance("", "", srvcontext);
        ServerInterleave interleave = new RunServer(server);

        final TeamTalkBase client = newClientInstance();

        EncryptionContext context = new EncryptionContext();
        context.szCAFile = CRYPTO_CA_FILE;
        context.bVerifyPeer = true;
        context.nVerifyDepth = 1;

        assertTrue(client.setEncryptionContext(context), "Set client encryption context");
        connect(server, client);
        login(server, client, NICKNAME, USERNAME, PASSWORD);
    }

    @Test
    public void testSSLClientCAInvalid() {
        if (!ENCRYPTED) {
            System.out.println("Skipped test. Requires encryption");
            return;
        }

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS;

        useraccounts.add(useraccount);

        EncryptionContext srvcontext = new EncryptionContext();
        srvcontext.szCertificateFile = CRYPTO_SERVER_CERT_FILE;
        srvcontext.szPrivateKeyFile = CRYPTO_SERVER_KEY_FILE;
        srvcontext.bVerifyPeer = false;
        srvcontext.bVerifyClientOnce = true;
        srvcontext.nVerifyDepth = 0;

        TeamTalkSrv server = newServerInstance("", "", srvcontext);
        ServerInterleave interleave = new RunServer(server);

        final TeamTalkBase client = newClientInstance();

        EncryptionContext context = new EncryptionContext();
        context.szCAFile = CRYPTO_CA2_FILE;
        context.bVerifyPeer = true;
        context.nVerifyDepth = 1;

        assertTrue(client.setEncryptionContext(context), "set context");
        assertTrue(client.connectSysID(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED, SYSTEMID), "connect call");
        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_CON_CRYPT_ERROR, DEF_WAIT, interleave), "wait crypt error");
        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_CON_FAILED, DEF_WAIT, interleave), "connection failed");
    }

    @Test
    public void testSSLClientCert() {
        if (!ENCRYPTED) {
            System.out.println("Skipped test. Requires encryption");
            return;
        }

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS;

        useraccounts.add(useraccount);

        EncryptionContext srvcontext = new EncryptionContext();
        srvcontext.szCertificateFile = CRYPTO_SERVER_CERT_FILE;
        srvcontext.szPrivateKeyFile = CRYPTO_SERVER_KEY_FILE;
        srvcontext.szCAFile = CRYPTO_CA_FILE;
        srvcontext.bVerifyPeer = false;
        srvcontext.bVerifyClientOnce = true;
        srvcontext.nVerifyDepth = 0;

        TeamTalkSrv server = newServerInstance("", "", srvcontext);
        ServerInterleave interleave = new RunServer(server);

        final TeamTalkBase client = newClientInstance();

        EncryptionContext context = new EncryptionContext();
        context.szCertificateFile = CRYPTO_CLIENT_CERT_FILE;
        context.szPrivateKeyFile = CRYPTO_CLIENT_KEY_FILE;
        context.szCAFile = CRYPTO_CA_FILE;
        context.bVerifyPeer = true;
        context.nVerifyDepth = 1;

        assertTrue(client.setEncryptionContext(context), "set context");
        connect(server, client);
        login(server, client, NICKNAME, USERNAME, PASSWORD);
    }

    @Test
    public void testSSLClientCertExpired() {

        if (!ENCRYPTED) {
            System.out.println("Skipped test. Requires encryption");
            return;
        }

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS;

        useraccounts.add(useraccount);

        EncryptionContext srvcontext = new EncryptionContext();
        srvcontext.szCertificateFile = CRYPTO_SERVER_CERT_FILE;
        srvcontext.szPrivateKeyFile = CRYPTO_SERVER_KEY_FILE;
        srvcontext.szCAFile = CRYPTO_CA_FILE;
        srvcontext.bVerifyPeer = false;
        srvcontext.bVerifyClientOnce = true;
        srvcontext.nVerifyDepth = 0;

        TeamTalkSrv server = newServerInstance("", "", srvcontext);
        ServerInterleave interleave = new RunServer(server);

        final TeamTalkBase client = newClientInstance();

        EncryptionContext context = new EncryptionContext();
        context.szCertificateFile = CRYPTO_CLIENT_CERT_EXPIRED_FILE;
        context.szPrivateKeyFile = CRYPTO_CLIENT_KEY_FILE;
        context.szCAFile = CRYPTO_CA_FILE;
        context.bVerifyPeer = true;
        context.nVerifyDepth = 1;

        assertTrue(client.setEncryptionContext(context), "set context");
        assertTrue(client.connectSysID(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED, SYSTEMID), "connect call");
        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_CON_CRYPT_ERROR, DEF_WAIT, interleave), "wait crypt error");
        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_CON_FAILED, DEF_WAIT, interleave), "connection failed");
    }

    @Test
    public void testHiddenChannel() {

        final String ADMIN = "admin";
        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = ADMIN;
        useraccount.szPassword = ADMIN;
        useraccount.uUserType = UserType.USERTYPE_ADMIN;
        useraccounts.add(useraccount);

        final String VIEW_NONE = "view_none";
        useraccount = new UserAccount();
        useraccount.szUsername = VIEW_NONE;
        useraccount.szPassword = VIEW_NONE;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;;
        useraccount.uUserRights = UserRight.USERRIGHT_TRANSMIT_VOICE;
        useraccounts.add(useraccount);

        final String VIEW_ALL_USERS = "view_all_users";
        useraccount = new UserAccount();
        useraccount.szUsername = VIEW_ALL_USERS;
        useraccount.szPassword = VIEW_ALL_USERS;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;;
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_TRANSMIT_VOICE;
        useraccounts.add(useraccount);

        final String VIEW_HIDDEN_CHANNELS = "view_hidden_channels";
        useraccount = new UserAccount();
        useraccount.szUsername = VIEW_HIDDEN_CHANNELS;
        useraccount.szPassword = VIEW_HIDDEN_CHANNELS;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;;
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_HIDDEN_CHANNELS | UserRight.USERRIGHT_TRANSMIT_VOICE;
        useraccounts.add(useraccount);

        final String VIEW_ALL_USERS_HIDDEN_CHANNELS = "view_all_users_hidden_channels";
        useraccount = new UserAccount();
        useraccount.szUsername = VIEW_ALL_USERS_HIDDEN_CHANNELS;
        useraccount.szPassword = VIEW_ALL_USERS_HIDDEN_CHANNELS;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;;
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_HIDDEN_CHANNELS | UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_TRANSMIT_VOICE;
        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        ServerInterleave interleave = new RunServer(server);

        TeamTalkBase admin = newClientInstance();
        connect(server, admin);
        login(server, admin, ADMIN + getTestMethodName(), ADMIN, ADMIN);

        UserAccount adminaccount = new UserAccount();
        assertTrue(admin.getMyUserAccount(adminaccount), "get account");
        assertEquals(UserRight.USERRIGHT_VIEW_HIDDEN_CHANNELS, adminaccount.uUserRights & UserRight.USERRIGHT_VIEW_HIDDEN_CHANNELS, "admin has hidden chan rights");

        TeamTalkBase view_none = newClientInstance();
        connect(server, view_none);
        login(server, view_none, VIEW_NONE + getTestMethodName(), VIEW_NONE, VIEW_NONE);

        TeamTalkBase view_all_users = newClientInstance();
        connect(server, view_all_users);
        login(server, view_all_users, VIEW_ALL_USERS + getTestMethodName(), VIEW_ALL_USERS, VIEW_ALL_USERS);

        TeamTalkBase view_hidden_channels = newClientInstance();
        connect(server, view_hidden_channels);
        login(server, view_hidden_channels, VIEW_HIDDEN_CHANNELS + getTestMethodName(), VIEW_HIDDEN_CHANNELS, VIEW_HIDDEN_CHANNELS);

        TeamTalkBase view_all_users_hidden_channels = newClientInstance();
        connect(server, view_all_users_hidden_channels);
        login(server, view_all_users_hidden_channels, VIEW_ALL_USERS_HIDDEN_CHANNELS + getTestMethodName(), VIEW_ALL_USERS_HIDDEN_CHANNELS, VIEW_ALL_USERS_HIDDEN_CHANNELS);

        Vector<TeamTalkBase> clients = new Vector<TeamTalkBase>();
        clients.add(admin);
        clients.add(view_none);
        clients.add(view_all_users);
        clients.add(view_hidden_channels);
        clients.add(view_all_users_hidden_channels);

        Channel hidden = buildDefaultChannel(admin, "Hidden channel");
        hidden.uChannelType |= (ChannelType.CHANNEL_HIDDEN | ChannelType.CHANNEL_PERMANENT);
        assertTrue(waitCmdSuccess(admin, admin.doMakeChannel(hidden), DEF_WAIT, interleave), "admin make hidden channel");

        for (TeamTalkBase client : clients)
            assertTrue(waitCmdComplete(client, client.doPing(), DEF_WAIT, interleave), "sync " + client.getMyUserID());

        int hidden_id = admin.getChannelIDFromPath(hidden.szName);
        assertTrue(hidden_id > 0, "Got hidden channel in admin");

        // check hidden channel visibility
        Channel chan = new Channel();
        assertTrue(admin.getChannel(hidden_id, chan), ADMIN + " can see hidden channel");
        assertFalse(view_none.getChannel(hidden_id, chan), VIEW_NONE + " cannot see hidden channel");
        assertFalse(view_all_users.getChannel(hidden_id, chan), VIEW_ALL_USERS + " cannot see hidden channel");
        assertTrue(view_hidden_channels.getChannel(hidden_id, chan), VIEW_HIDDEN_CHANNELS + " can see hidden channel");
        assertTrue(view_all_users_hidden_channels.getChannel(hidden_id, chan), VIEW_ALL_USERS_HIDDEN_CHANNELS + " can see hidden channel");

        // check initial login with hidden channels
        for (TeamTalkBase client : clients) {
            assertTrue(waitCmdComplete(client, client.doLogout(), DEF_WAIT, interleave), "logout/login");
        }
        login(server, admin, ADMIN + getTestMethodName(), ADMIN, ADMIN);
        login(server, view_none, VIEW_NONE + getTestMethodName(), VIEW_NONE, VIEW_NONE);
        login(server, view_all_users, VIEW_ALL_USERS + getTestMethodName(), VIEW_ALL_USERS, VIEW_ALL_USERS);
        login(server, view_hidden_channels, VIEW_HIDDEN_CHANNELS + getTestMethodName(), VIEW_HIDDEN_CHANNELS, VIEW_HIDDEN_CHANNELS);
        login(server, view_all_users_hidden_channels, VIEW_ALL_USERS_HIDDEN_CHANNELS + getTestMethodName(), VIEW_ALL_USERS_HIDDEN_CHANNELS, VIEW_ALL_USERS_HIDDEN_CHANNELS);

        // check user in hidden channel visibility
        assertTrue(waitCmdSuccess(admin, admin.doJoinChannelByID(hidden_id, ""), DEF_WAIT, interleave), "admin join hidden");
        for (TeamTalkBase client : clients)
            assertTrue(waitCmdComplete(client, client.doPing(), DEF_WAIT, interleave), "sync " + client.getMyUserID());
        User user = new User();
        assertTrue(admin.getUser(admin.getMyUserID(), user) && user.nChannelID == hidden_id, ADMIN + " can see user in channel");
        assertFalse(view_none.getUser(admin.getMyUserID(), user), VIEW_NONE + " cannot see user inside/outside channel");
        assertTrue(view_all_users.getUser(admin.getMyUserID(), user) && user.nChannelID == 0, VIEW_ALL_USERS + " cannot see user in hidden channel");
        assertFalse(view_hidden_channels.getUser(admin.getMyUserID(), user), VIEW_HIDDEN_CHANNELS + " cannot see user in hidden channel");
        assertTrue(view_all_users_hidden_channels.getUser(admin.getMyUserID(), user) && user.nChannelID == hidden_id, VIEW_ALL_USERS_HIDDEN_CHANNELS + " can see user in hidden channel");

        // check user leave hidden channel visibility
        assertTrue(waitCmdSuccess(admin, admin.doLeaveChannel(), DEF_WAIT, interleave), "admin leave hidden channel");
        for (TeamTalkBase client : clients)
            assertTrue(waitCmdComplete(client, client.doPing(), DEF_WAIT, interleave), "sync " + client.getMyUserID());
        assertTrue(admin.getUser(admin.getMyUserID(), user) && user.nChannelID == 0, ADMIN + " can see user");
        assertFalse(view_none.getUser(admin.getMyUserID(), user), VIEW_NONE + " cannot see user");
        assertTrue(view_all_users.getUser(admin.getMyUserID(), user) && user.nChannelID == 0, VIEW_ALL_USERS + " see user");
        assertFalse(view_hidden_channels.getUser(admin.getMyUserID(), user), VIEW_HIDDEN_CHANNELS + " cannot see user");
        assertTrue(view_all_users_hidden_channels.getUser(admin.getMyUserID(), user) && user.nChannelID == 0, VIEW_ALL_USERS_HIDDEN_CHANNELS + " can see user");

        // check USERRIGHT_VIEW_HIDDEN_CHANNELS when joining hidden channel
        assertTrue(waitCmdSuccess(view_hidden_channels, view_hidden_channels.doJoinChannelByID(hidden_id, ""), DEF_WAIT, interleave), VIEW_HIDDEN_CHANNELS + " join hidden channel");
        assertTrue(view_hidden_channels.getUser(view_hidden_channels.getMyUserID(), user) && user.nChannelID == hidden_id, VIEW_HIDDEN_CHANNELS + " can see self");
        assertTrue(waitCmdSuccess(admin, admin.doJoinChannelByID(hidden_id, ""), DEF_WAIT, interleave), "admin join hidden channel");
        for (TeamTalkBase client : clients)
            assertTrue(waitCmdComplete(client, client.doPing(), DEF_WAIT, interleave), "sync " + client.getMyUserID());
        assertTrue(view_hidden_channels.getUser(admin.getMyUserID(), user) && user.nChannelID == hidden_id, VIEW_HIDDEN_CHANNELS + " can see admin in hidden channel");
        assertTrue(waitCmdSuccess(admin, admin.doLeaveChannel(), DEF_WAIT, interleave), "admin leave hidden channel");
        for (TeamTalkBase client : clients)
            assertTrue(waitCmdComplete(client, client.doPing(), DEF_WAIT, interleave), "sync " + client.getMyUserID());
        assertFalse(view_hidden_channels.getUser(admin.getMyUserID(), user), VIEW_HIDDEN_CHANNELS + " cannot see admin");
        assertTrue(waitCmdSuccess(view_hidden_channels, view_hidden_channels.doLeaveChannel(), DEF_WAIT, interleave), VIEW_HIDDEN_CHANNELS + "leave hidden channel");
        assertTrue(waitCmdSuccess(view_hidden_channels, view_hidden_channels.doJoinChannelByID(hidden_id, ""), DEF_WAIT, interleave), VIEW_HIDDEN_CHANNELS + " join hidden channel again");

        // check USERRIGHT_NONE can join hidden channel
        assertTrue(waitCmdSuccess(view_none, view_none.doJoinChannel(hidden), DEF_WAIT, interleave), VIEW_NONE + " join hidden");
        for (TeamTalkBase client : clients)
            assertTrue(waitCmdComplete(client, client.doPing(), DEF_WAIT, interleave), "sync " + client.getMyUserID());
        assertFalse(view_none.getUser(admin.getMyUserID(), user), VIEW_NONE + " cannot see admin");
        assertTrue(view_none.getUser(view_none.getMyUserID(), user) && user.nChannelID == hidden_id, VIEW_NONE + " can see self");
        assertTrue(view_none.getUser(view_hidden_channels.getMyUserID(), user) && user.nChannelID == hidden_id, VIEW_NONE + " can see " + view_hidden_channels);
        assertTrue(waitCmdSuccess(view_none, view_none.doLeaveChannel(), DEF_WAIT, interleave), VIEW_NONE + "leave hidden channel");
        assertFalse(view_none.getChannel(hidden_id, chan), VIEW_NONE + " cannot see hidden channel");
        assertFalse(view_none.getUser(view_hidden_channels.getMyUserID(), user), VIEW_NONE + " cannot see " + view_hidden_channels);

        // check removal of hidden channel
        assertTrue(waitCmdSuccess(view_none, view_none.doJoinChannel(hidden), DEF_WAIT, interleave), VIEW_NONE + " join hidden");
        assertTrue(waitCmdSuccess(admin, admin.doRemoveChannel(hidden_id), DEF_WAIT, interleave), "admin remove hidden channel");
        for (TeamTalkBase client : clients)
            assertTrue(waitCmdComplete(client, client.doPing(), DEF_WAIT, interleave), "sync " + client.getMyUserID());
        assertTrue(view_none.getMyChannelID() == 0, VIEW_NONE + " kicked from hidden channel");

        // check CHANNEL_HIDDEN cannot be toggled
        assertTrue(admin.getChannel(admin.getRootChannelID(), chan), "get root chan");
        chan.uChannelType |= ChannelType.CHANNEL_HIDDEN;
        assertTrue(waitCmdError(admin, admin.doUpdateChannel(chan), DEF_WAIT, interleave), "cannot set hidden on existing channel");
        chan = buildDefaultChannel(admin, "Foo");
        assertTrue(waitCmdSuccess(admin, admin.doMakeChannel(chan), DEF_WAIT, interleave), "admin make new channel");
        int foo_id = admin.getChannelIDFromPath(chan.szName);
        assertTrue(admin.getChannel(foo_id, chan), "Got new channel in admin");
        chan.uChannelType |= ChannelType.CHANNEL_HIDDEN;
        assertTrue(waitCmdError(admin, admin.doUpdateChannel(chan), DEF_WAIT, interleave), "cannot set hidden on existing channel");

        // check CHANNEL_HIDDEN cannot contain subchannel
        hidden = buildDefaultChannel(admin, "Hidden channel");
        hidden.uChannelType |= (ChannelType.CHANNEL_HIDDEN | ChannelType.CHANNEL_PERMANENT);
        assertTrue(waitCmdSuccess(admin, admin.doMakeChannel(hidden), DEF_WAIT, interleave), "admin make hidden channel");
        hidden_id = admin.getChannelIDFromPath(hidden.szName);
        assertTrue(hidden_id > 0, "Got hidden channel in admin");
        hidden.nParentID = hidden_id;
        assertTrue(waitCmdError(admin, admin.doMakeChannel(chan), DEF_WAIT, interleave), "cannot create hidden subchannel in hidden channel");
        hidden.uChannelType = ChannelType.CHANNEL_DEFAULT;
        assertTrue(waitCmdError(admin, admin.doMakeChannel(chan), DEF_WAIT, interleave), "cannot create subchannel in hidden channel");

        // test login after hidden channel has been created
        hidden = buildDefaultChannel(admin, "Hidden channel 2");
        hidden.uChannelType |= (ChannelType.CHANNEL_HIDDEN);
        assertTrue(waitCmdSuccess(admin, admin.doJoinChannel(hidden), DEF_WAIT, interleave), "admin make hidden channel");
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS;
        useraccounts.add(useraccount);
        TeamTalkBase ttclient = newClientInstance();
        connect(server, ttclient);
        login(server, ttclient, USERNAME + getTestMethodName(), USERNAME, PASSWORD);
        joinRoot(server, ttclient);
        user = new User();
        assertTrue(ttclient.getUser(admin.getMyUserID(), user), "get admin");
        assertEquals(0, user.nChannelID, "no chan specified");
        hidden.uChannelType = ChannelType.CHANNEL_DEFAULT;
        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(hidden), DEF_WAIT, interleave), "join hidden");
        user = new User();
        assertTrue(ttclient.getUser(admin.getMyUserID(), user), "get admin");
        assertEquals(ttclient.getMyChannelID(), user.nChannelID, "chan specified");
        assertTrue(waitCmdSuccess(admin, admin.doChangeStatus(9, "hest"), DEF_WAIT, interleave), "admin change status");
        assertTrue(waitCmdComplete(ttclient, ttclient.doPing(), DEF_WAIT, interleave), "sync");
        user = new User();
        assertTrue(ttclient.getUser(admin.getMyUserID(), user), "get admin");
        assertEquals(9, user.nStatusMode, "admin state update for ttclient");
        assertTrue(waitCmdSuccess(admin, admin.doLeaveChannel(), DEF_WAIT, interleave), "admin leave");
        assertTrue(waitCmdComplete(ttclient, ttclient.doPing(), DEF_WAIT, interleave), "sync");
        user = new User();
        assertTrue(ttclient.getUser(admin.getMyUserID(), user), "get admin");
        assertEquals(0, user.nChannelID, "no chan specified");
    }

    @Test
    public void testServerLogging() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_UPDATE_SERVERPROPERTIES;
        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        ServerInterleave interleave = new RunServer(server);

        logevents.clear();

        TeamTalkBase client = newClientInstance();
        connect(server, client);
        ServerProperties srvprop = new ServerProperties();
        assertTrue(client.getServerProperties(srvprop), "get serverprop");
        assertEquals(ServerLogEvent.SERVERLOGEVENT_NONE, srvprop.uServerLogEvents, "log mask none");
        login(server, client, NICKNAME, USERNAME, PASSWORD);

        assertEquals(1, logevents.size(), "one connected log event");

        assertTrue(client.getServerProperties(srvprop), "get serverprop after login");
        srvprop.uServerLogEvents = ServerLogEvent.SERVERLOGEVENT_NONE;
        assertTrue(waitCmdSuccess(client, client.doUpdateServer(srvprop), DEF_WAIT, interleave), "update server");

        assertTrue(client.getServerProperties(srvprop), "get server properties again");
        assertEquals(ServerLogEvent.SERVERLOGEVENT_NONE, srvprop.uServerLogEvents, "log mask none");

        connect(server, newClientInstance());
        assertEquals(1, logevents.size(), "still one log event");

        srvprop.uServerLogEvents = ServerLogEvent.SERVERLOGEVENT_USER_CONNECTED;
        assertTrue(waitCmdSuccess(client, client.doUpdateServer(srvprop), DEF_WAIT, interleave), "update server");

        connect(server, newClientInstance());
        assertEquals(2, logevents.size(), "now two log events");
    }

    @Test
    public void testTextMessageIndex() {
        UserAccount useraccount = new UserAccount();

        useraccount.szUsername = "guest";
        useraccount.szPassword = "guest";
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_MULTI_LOGIN | UserRight.USERRIGHT_TEXTMESSAGE_USER;

        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        TeamTalkBase client1 = newClientInstance();
        TeamTalkBase client2 = newClientInstance();

        connect(server, client1);
        connect(server, client2);

        ServerInterleave interleave = new RunServer(server);

        login(server, client1, getTestMethodName(), useraccount.szUsername, useraccount.szPassword);
        login(server, client2, getTestMethodName(), useraccount.szUsername, useraccount.szPassword);

        for (int i=0;i<57;++i) {
            TextMessage txtmsg = new TextMessage();
            txtmsg.nMsgType = TextMsgType.MSGTYPE_USER;
            txtmsg.nToUserID = client2.getMyUserID();
            txtmsg.szMessage = "My text message";
            txtmsg.bMore = i < 56;
            assertTrue(waitCmdSuccess(client1, client1.doTextMessage(txtmsg), DEF_WAIT, interleave), "send text message #" + i);
        }

        TTMessage msg = new TTMessage();
        for (int i=0;i<57;++i) {
            assertTrue(waitForEvent(client2, ClientEvent.CLIENTEVENT_CMD_USER_TEXTMSG, DEF_WAIT, msg, interleave), "message event");
            assertEquals(i < 56, msg.textmessage.bMore, "message more");
        }
    }

    @Test
    public void testTextMessagePrivate() {
        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = "guest";
        useraccount.szPassword = "guest";
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_MULTI_LOGIN | UserRight.USERRIGHT_TEXTMESSAGE_USER;
        useraccounts.add(useraccount);

        UserAccount useraccount2 = new UserAccount();
        useraccount2.szUsername = "guest2";
        useraccount2.szPassword = "guest";
        useraccount2.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount2.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_MULTI_LOGIN;
        useraccounts.add(useraccount2);

        TeamTalkSrv server = newServerInstance();
        TeamTalkBase client1 = newClientInstance();
        TeamTalkBase client2 = newClientInstance();

        connect(server, client1);
        connect(server, client2);

        ServerInterleave interleave = new RunServer(server);

        login(server, client1, getTestMethodName(), useraccount.szUsername, useraccount.szPassword);
        login(server, client2, getTestMethodName(), useraccount2.szUsername, useraccount2.szPassword);

        TextMessage txtmsg = new TextMessage();
        txtmsg.nMsgType = TextMsgType.MSGTYPE_USER;
        txtmsg.nToUserID = client2.getMyUserID();
        txtmsg.szMessage = "My text message";

        assertTrue(waitCmdSuccess(client1, client1.doTextMessage(txtmsg), DEF_WAIT, interleave), "send text message with rights");

        txtmsg.nToUserID = client1.getMyUserID();

        assertTrue(waitCmdError(client2, client2.doTextMessage(txtmsg), DEF_WAIT, interleave), "send text message without rights");
    }

    @Test
    public void testTextMessageChannel() {
        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = "guest";
        useraccount.szPassword = "guest";
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_MULTI_LOGIN | UserRight.USERRIGHT_TEXTMESSAGE_CHANNEL;
        useraccounts.add(useraccount);

        UserAccount useraccount2 = new UserAccount();
        useraccount2.szUsername = "guest2";
        useraccount2.szPassword = "guest";
        useraccount2.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount2.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_MULTI_LOGIN;
        useraccounts.add(useraccount2);

        TeamTalkSrv server = newServerInstance();
        TeamTalkBase client1 = newClientInstance();
        TeamTalkBase client2 = newClientInstance();

        connect(server, client1);
        connect(server, client2);

        ServerInterleave interleave = new RunServer(server);

        login(server, client1, getTestMethodName(), useraccount.szUsername, useraccount.szPassword);
        login(server, client2, getTestMethodName(), useraccount2.szUsername, useraccount2.szPassword);
        joinRoot(server, client1);
        joinRoot(server, client2);

        TextMessage txtmsg = new TextMessage();
        txtmsg.nMsgType = TextMsgType.MSGTYPE_CHANNEL;
        txtmsg.nChannelID = client1.getMyChannelID();
        txtmsg.szMessage = "My text message";

        assertTrue(waitCmdSuccess(client1, client1.doTextMessage(txtmsg), DEF_WAIT, interleave), "send text message with rights");
        assertTrue(waitCmdError(client2, client2.doTextMessage(txtmsg), DEF_WAIT, interleave), "send text message without rights");
    }

    @Test
    public void testAutoOperatorChannelPassword() {

        TeamTalkSrv server = newServerInstance();
        ServerInterleave interleave = new RunServer(server);

        TeamTalkBase admin = newClientInstance();
        TeamTalkBase client = newClientInstance();

        connect(server, admin);
        login(server, admin, getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        Channel chan = buildDefaultChannel(admin, getTestMethodName());
        chan.uChannelType |= ChannelType.CHANNEL_PERMANENT;
        chan.szPassword = "password";
        chan.szOpPassword = "oppassword";

        assertTrue(waitCmdSuccess(admin, admin.doMakeChannel(chan), DEF_WAIT, interleave), "perm chan");

        int permid = admin.getChannelIDFromPath(getTestMethodName());

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = "guest";
        useraccount.szPassword = "guest";
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_MULTI_LOGIN;
        useraccounts.add(useraccount);

        connect(server, client);
        login(server, client, getTestMethodName(), useraccount.szUsername, useraccount.szPassword);

        Channel tmp = new Channel();
        assertTrue(client.getChannel(permid, tmp), "get chan");
        assertEquals("", tmp.szPassword, "No chan password");
        assertEquals("", tmp.szOpPassword, "No chan oppassword");

        assertTrue(client.disconnect(), "disconnect");

        useraccount.autoOperatorChannels[0] = permid;

        connect(server, client);
        login(server, client, getTestMethodName(), useraccount.szUsername, useraccount.szPassword);
        assertTrue(client.getChannel(permid, tmp), "get chan");
        assertEquals("password", tmp.szPassword, "Chan password");
        assertEquals("oppassword", tmp.szOpPassword, "Chan oppassword");

        assertTrue(admin.getChannel(permid, chan), "get admin chan");
        chan.szPassword = "foo";
        chan.szOpPassword = "foo2";
        assertTrue(waitCmdSuccess(admin, admin.doUpdateChannel(chan), DEF_WAIT, interleave), "updateperm chan");

        assertTrue(waitCmdComplete(client, client.doPing(), DEF_WAIT, interleave), "update client");
        assertTrue(client.getChannel(permid, tmp), "get chan");
        assertEquals("foo", tmp.szPassword, "Client see chan password");
        assertEquals("foo2", tmp.szOpPassword, "Client see oppassword");
    }

    @Test
    public void testAutoOperatorChannelUpdate() {

        TeamTalkSrv server = newServerInstance();
        ServerInterleave interleave = new RunServer(server);

        TeamTalkBase admin = newClientInstance();
        TeamTalkBase client = newClientInstance();
        TeamTalkBase client2 = newClientInstance();

        connect(server, admin);
        login(server, admin, getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        Channel chan = buildDefaultChannel(admin, getTestMethodName());
        chan.uChannelType |= ChannelType.CHANNEL_PERMANENT;
        chan.szPassword = "password";
        chan.szOpPassword = "oppassword";

        assertTrue(waitCmdSuccess(admin, admin.doMakeChannel(chan), DEF_WAIT, interleave), "perm chan");

        int permid = admin.getChannelIDFromPath(getTestMethodName());

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = "guest";
        useraccount.szPassword = "guest";
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_MULTI_LOGIN;
        useraccount.autoOperatorChannels[0] = permid;
        useraccounts.add(useraccount);

        UserAccount noopuseraccount = new UserAccount();
        noopuseraccount.szUsername = "guest";
        noopuseraccount.szPassword = "guest";
        noopuseraccount.uUserType = UserType.USERTYPE_DEFAULT;
        noopuseraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_MULTI_LOGIN;
        useraccounts.add(noopuseraccount);

        connect(server, client);
        login(server, client, getTestMethodName(), useraccount.szUsername, useraccount.szPassword);

        connect(server, client2);
        login(server, client2, getTestMethodName(), noopuseraccount.szUsername, noopuseraccount.szPassword);

        Channel tmp = new Channel();
        assertTrue(client.getChannel(permid, tmp), "get chan");

        // test auto-op can update channels not joined
        tmp.szPassword = "foo";
        assertTrue(waitCmdSuccess(client, client.doUpdateChannel(tmp), DEF_WAIT, interleave), "updateperm chan");
        assertTrue(waitCmdComplete(client, client.doPing(), DEF_WAIT, interleave), "update client");

        tmp = new Channel();
        assertTrue(client.getChannel(permid, tmp), "get chan");

        assertEquals("foo", tmp.szPassword, "Client updated chan password");

        // test auto-op can kick from channels not joined
        assertTrue(waitCmdSuccess(admin, admin.doJoinChannelByID(permid, "foo"), DEF_WAIT, interleave), "admin join");
        assertTrue(waitCmdComplete(client, client.doPing(), DEF_WAIT, interleave), "update client");
        assertTrue(waitCmdComplete(client2, client2.doPing(), DEF_WAIT, interleave), "update client2");
        assertTrue(waitCmdError(client2, client2.doKickUser(admin.getMyUserID(), admin.getMyChannelID()), DEF_WAIT, interleave), "cannot kick");
        assertTrue(waitCmdSuccess(client, client.doKickUser(admin.getMyUserID(), admin.getMyChannelID()), DEF_WAIT, interleave), "kick from chan");

        // test auto-op can ban from channels not joined
        assertTrue(waitCmdSuccess(admin, admin.doJoinChannelByID(permid, "foo"), DEF_WAIT, interleave), "admin join");
        assertTrue(waitCmdComplete(client, client.doPing(), DEF_WAIT, interleave), "update client");
        assertTrue(waitCmdComplete(client2, client2.doPing(), DEF_WAIT, interleave), "update client2");
        assertTrue(waitCmdError(client2, client2.doBanUser(admin.getMyUserID(), admin.getMyChannelID()), DEF_WAIT, interleave), "cannot ban from chan");
        // ServerCallback in this test class overrides ban outcome
        // assertTrue(waitCmdSuccess(client, client.doBanUser(admin.getMyUserID(), admin.getMyChannelID()), DEF_WAIT, interleave), "ban from chan");

        // test auto-op can list bans
        assertTrue(waitCmdSuccess(client, client.doListBans(permid, 0, 100), DEF_WAIT, interleave), "update client");
        assertTrue(waitCmdError(client2, client2.doListBans(permid, 0, 100), DEF_WAIT, interleave), "update client2");
    }

    @Test
    public void testTimeOutTimer() {
        TeamTalkSrv server = newServerInstance();
        ServerInterleave interleave = new RunServer(server);

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = "guest";
        useraccount.szPassword = "guest";
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE | UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        useraccounts.add(useraccount);

        TeamTalkBase client = newClientInstance();
        initSound(client);
        connect(server, client);
        login(server, client, getTestMethodName(), useraccount.szUsername, useraccount.szPassword);

        Channel chan = buildDefaultChannel(client, "New channel", Codec.OPUS_CODEC);
        chan.audiocodec.opus.nFrameSizeMSec = 5;
        chan.audiocodec.opus.nTxIntervalMSec = 10;
        chan.nTimeOutTimerVoiceMSec = 50;
        chan.nTimeOutTimerMediaFileMSec = 50;
        assertTrue(waitCmdSuccess(client, client.doJoinChannel(chan), DEF_WAIT, interleave), "join channel");

        assertTrue(client.getChannel(client.getMyChannelID(), chan), "get new chan");

        assertTrue(waitCmdSuccess(client, client.doSubscribe(client.getMyUserID(),
                                                                                Subscription.SUBSCRIBE_VOICE), DEF_WAIT, interleave), "subscribe voice");
        assertTrue(waitCmdSuccess(client, client.doSubscribe(client.getMyUserID(),
                                                                             Subscription.SUBSCRIBE_MEDIAFILE), DEF_WAIT, interleave), "subscribe mf");

        TTMessage msg = new TTMessage();

        assertTrue(client.enableVoiceTransmission(true), "vox");

        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "User state changed to voice");
        assertEquals(UserState.USERSTATE_VOICE, msg.user.uUserState, "User is talking");

        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "User state changed to not voice");
        assertEquals(UserState.USERSTATE_NONE, msg.user.uUserState, "User is not talking");

        assertTrue(client.enableVoiceTransmission(false), "vox disable");

        assertTrue(client.enableVoiceTransmission(true), "vox new stream");

        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "User state changed to voice on new stream");
        assertEquals(UserState.USERSTATE_VOICE, msg.user.uUserState, "User is talking on new stream");

        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "User state changed to not voice on new stream");
        assertEquals(UserState.USERSTATE_NONE, msg.user.uUserState, "User is not talking on new stream");

        MediaFileInfo mfi = new MediaFileInfo();
        mfi.szFileName = STORAGEFOLDER + File.separator + "tot.wav";
        mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 2);
        mfi.uDurationMSec = 3 * 1000;

        assertTrue(TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600), "Write media file");

        assertTrue(client.startStreamingMediaFileToChannel(mfi.szFileName, new VideoCodec()), "Start stream file");

        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "User state changed to media file");
        assertEquals(UserState.USERSTATE_MEDIAFILE_AUDIO, msg.user.uUserState, "User is streaming");

        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "User state changed to not streaming");
        assertEquals(UserState.USERSTATE_NONE, msg.user.uUserState, "User is not streaming");

        client.stopStreamingMediaFileToChannel();
    }

    @Test
    public void testServerUpdatedLogEvent() {
        class Log extends ServerLogger {
            public boolean srvupdateevent = false;
            public void serverUpdated(ServerProperties lpServerProperties, User lpUser) {
                srvupdateevent = true;
            }
        };
        Log log = new Log();

        TeamTalkSrv server = newServerInstance("", "", null, log);

        ServerInterleave interleave = new RunServer(server);

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = "guest";
        useraccount.szPassword = "guest";
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_NONE;
        useraccounts.add(useraccount);

        TeamTalkBase client = newClientInstance();
        initSound(client);
        connect(server, client);
        login(server, client, getTestMethodName(), useraccount.szUsername, useraccount.szPassword);

        ServerProperties properties = new ServerProperties();
        assertTrue(client.getServerProperties(properties), "get props");
        properties.szServerName = properties.szServerName + "some more";
        assertEquals(ClientError.CMDERR_SUCCESS, server.updateServer(properties), "update properties");
        TTMessage msg = new TTMessage();
        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_CMD_SERVER_UPDATE, DEF_WAIT, msg, interleave), "update event");
        assertEquals(properties.szServerName, msg.serverproperties.szServerName, "update properties client");
        assertTrue(log.srvupdateevent, "log event");
    }

    // @Test
    public void _testRunServer() {

        UserAccount useraccount = new UserAccount();

        useraccount.szUsername = "guest";
        useraccount.szPassword = "guest";
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.szNote = "An example user account with limited user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_TRANSMIT_VOICE;

        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();

        System.out.println("Running TeamTalk server forever");
        while(server.runEventLoop(-1));

    }

    public TeamTalkSrv newServerInstance() {
        return newServerInstance("");
    }

    public TeamTalkSrv newServerInstance(String systemid) {
        return newServerInstance(systemid, "");
    }

    public TeamTalkSrv newServerInstance(String systemid, String bindip) {
        return newServerInstance(systemid, bindip, null);
    }

    public TeamTalkSrv newServerInstance(String systemid, String bindip, EncryptionContext srvcontext) {
        return newServerInstance(systemid, bindip, srvcontext, this.logger);
    }

    public TeamTalkSrv newServerInstance(String systemid, String bindip, EncryptionContext srvcontext, ServerLogger logger) {

        TeamTalkSrv server = new TeamTalk5Srv(cmdcallback, logger);
        if (ENCRYPTED) {
            if (srvcontext == null)
                assertTrue(server.setEncryptionContext(CRYPTO_SERVER_CERT2_FILE, CRYPTO_SERVER_KEY2_FILE), "Set context");
            else
                assertTrue(server.setEncryptionContext(srvcontext), "set server encryption context");
        }
        assertEquals(ClientError.CMDERR_SUCCESS,
                 server.setChannelFilesRoot(FILESTORAGE_FOLDER, MAX_DISKUSAGE, DEFAULT_CHANNEL_QUOTA), "File storage");

        Channel chan = new Channel();
        chan.nChannelID = 1;
        chan.nParentID = 0;
        chan.nMaxUsers = 10;
        chan.audiocodec = new AudioCodec(true);
        chan.audiocfg = new AudioConfig(true);
        chan.nDiskQuota = DEFAULT_CHANNEL_QUOTA;

        assertEquals(ClientError.CMDERR_SUCCESS, server.makeChannel(chan), "Make root channel");

        if (bindip.isEmpty())
            bindip = SERVERBINDIP;

        if(systemid.isEmpty())
            assertTrue(server.startServer(bindip, TCPPORT, UDPPORT, ENCRYPTED), "Start server");
        else
            assertTrue(server.startServerSysID(bindip, TCPPORT, UDPPORT, ENCRYPTED, systemid), "Start server");

        servers.add(server);

        return server;
    }

    static class RunServer implements ServerInterleave {
        TeamTalkSrv server;

        public RunServer(TeamTalkSrv server) {
            this.server = server;
        }

        public void interleave() {
            while(server.runEventLoop(0)) {
            }
        }
    }

    protected static void connect(TeamTalkSrv server, TeamTalkBase ttclient) {
        connect(server, ttclient, SYSTEMID);
    }

    protected static void connect(TeamTalkSrv server, TeamTalkBase ttclient, String systemID) {
        connect(server, ttclient, systemID, IPADDR, TCPPORT, UDPPORT);
    }

    protected static void connect(TeamTalkSrv server, final TeamTalkBase ttclient, final String systemID,
                                  final String hostaddr, final int tcpport, final int udpport) {

        connect(ttclient, systemID, hostaddr, tcpport, udpport, new RunServer(server));
    }

    protected static void login(TeamTalkSrv server, TeamTalkBase ttclient,
                                String nick, String username, String passwd) {
        login(server, ttclient, nick, username, passwd, "");
    }

    protected static void login(TeamTalkSrv server, TeamTalkBase ttclient,
                                String nick, String username, String passwd,
                                String clientname) {
        login(ttclient, nick, username, passwd, clientname, new RunServer(server));
    }

    protected static void joinRoot(TeamTalkSrv server, TeamTalkBase ttclient) {
        joinRoot(ttclient, new RunServer(server));
    }

}
