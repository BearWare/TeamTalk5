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

import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.FixMethodOrder;
import org.junit.runners.MethodSorters;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertEquals;

import java.util.Vector;
import java.io.DataOutputStream;
import java.io.FileOutputStream;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.File;
import java.util.Arrays;
import java.net.InetAddress;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TeamTalkServerTestCase extends TeamTalkTestCaseBase {

    Vector<TeamTalkSrv> servers = new Vector<TeamTalkSrv>();

    String FILESTORAGE_FOLDER = "./filestorage";
    long MAX_DISKUSAGE = 100000000000l, DEFAULT_CHANNEL_QUOTA = 10000000000l;
    String SERVERBINDIP;

    public TeamTalkBase newClientInstance() {
        TeamTalkBase ttclient = new TeamTalk5Pro();
        ttclients.add(ttclient);
        return ttclient;
    }

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
                    assertTrue("User type not set", false);
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
                    assertTrue("User type not set", false);
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

    ServerLogger logger = new ServerLogger() {
            
            public void userConnected(User lpUser) {
                String str = String.format("User with IP-address %s connected",
                                           lpUser.szIPAddress);
                System.out.println(str);
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
        assertTrue("Login client", cmdid > 0);

        TTMessage msg = new TTMessage();
        //check that the client gets back the same user account we created in the server
        assertTrue("wait login", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDIN, DEF_WAIT, msg, interleave));
        assertEquals("Account identity", useraccount.szUsername, msg.useraccount.szUsername);
        assertEquals("Account type", useraccount.uUserType, msg.useraccount.uUserType);
        assertEquals("Account rights", useraccount.uUserRights, msg.useraccount.uUserRights);
        assertEquals("Account note", useraccount.szNote, msg.useraccount.szNote);

        assertTrue("wait success", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave));
        assertEquals("Login success", cmdid, msg.nSource);
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
        assertTrue("New account cmd", cmdid > 0);

        int n_accounts = useraccounts.size();

        waitCmdSuccess(client1, cmdid, 1000, interleave);

        assertEquals("One more account", useraccounts.size(), n_accounts + 1);

        UserAccount srv_ua = getUserAccount(useraccount.szUsername);
        assertEquals("Account identity", srv_ua.szUsername, useraccount.szUsername);
        assertEquals("Account type", srv_ua.uUserType, useraccount.uUserType);
        assertEquals("Account rights", srv_ua.uUserRights, useraccount.uUserRights);
        assertEquals("Account note", srv_ua.szNote, useraccount.szNote);
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
        assertTrue("Login client1", cmdid > 0);

        cmdid = client2.doLogin(getTestMethodName(), useraccount.szUsername, useraccount.szPassword);
        assertTrue("Login client2", cmdid > 0);

        TTMessage msg = new TTMessage();
        assertTrue("wait success1", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave));
        assertTrue("wait success2", waitForEvent(client2, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave));

        int n_accounts = useraccounts.size();
        cmdid = client1.doDeleteUserAccount(ADMIN_USERNAME);

        waitCmdSuccess(client1, cmdid, 1000, interleave);

        assertEquals("One less account", useraccounts.size(), n_accounts - 1);

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
        assertTrue("Login client1", cmdid > 0);

        cmdid = client2.doLogin(getTestMethodName(), useraccount.szUsername, useraccount.szPassword);
        assertTrue("Login client2", cmdid > 0);

        TTMessage msg = new TTMessage();
        assertTrue("wait success1", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave));
        assertTrue("wait success2", waitForEvent(client2, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave));

        int n_banned = banned_ipaddr.size();
        cmdid = client1.doBanUser(client2.getMyUserID(), 0);

        assertTrue("wait success1", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave));

        assertEquals("Banned user", n_banned + 1, banned_ipaddr.size());

        cmdid = client1.doUnBanUser(banned_ipaddr.get(0), 0);

        assertTrue("wait success1", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave));

        assertEquals("Banned user", n_banned, banned_ipaddr.size());

        cmdid = client1.doBanIPAddress("11.22.33.44", 0);

        assertTrue("wait success1", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave));

        assertEquals("Banned user", n_banned + 1, banned_ipaddr.size());
        
    }

    @Test
    public void testSystemID() {
        TeamTalkSrv server = newServerInstance("foobar");

        ServerInterleave interleave = new RunServer(server);

        TeamTalkBase client = newClientInstance();

        connect(server, client, "foobar");

        int cmdid = client.doLoginEx(getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD, "myclientname");
        assertTrue("Login client", cmdid > 0);

        TTMessage msg = new TTMessage();
        assertTrue("wait success", waitForEvent(client, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg, interleave));

        User user = new User();
        assertTrue("Get user", client.getUser(client.getMyUserID(), user));

        assertEquals("clientname set", "myclientname", user.szClientName);

    }

    @Test
    public void testWrongSystemID() {
        TeamTalkSrv server = newServerInstance("foobar");

        final ServerInterleave interleave = new RunServer(server);

        final TeamTalkBase client = newClientInstance();
        
        if (ENCRYPTED) {
            // SSL client uses blocking connect, so we have to connect
            // in separate thread otherwise we cannot run the client
            // and server event-loop at the same time
            Thread t = new Thread(new Runnable(){
                    public void run() {
                        assertTrue("Connect", client.connect(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED));

                        waitForEvent(client, ClientEvent.CLIENTEVENT_CMD_ERROR, 1000);
                    }
                });
            t.start();

            while (t.isAlive()) {
                server.runEventLoop(0);
            }
        }
        else {
            assertTrue("Connect", client.connect(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED));

            waitForEvent(client, ClientEvent.CLIENTEVENT_CMD_ERROR, 1000, interleave);
        }
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

        assertTrue("join channel", waitCmdSuccess(client1, cmdid, DEF_WAIT, interleave));

        cmdid = client2.doJoinChannelByID(client2.getRootChannelID(), "");

        assertTrue("join channel", waitCmdSuccess(client2, cmdid, DEF_WAIT, interleave));

        cmdid = client1.doPing();
        
        assertTrue("drain client1", waitCmdComplete(client1, cmdid, DEF_WAIT, interleave));

        cmdid = client2.doMoveUser(client1.getMyUserID(), client2.getMyChannelID());

        assertTrue("move user", waitCmdSuccess(client2, cmdid, DEF_WAIT, interleave));

        assertTrue("wait move", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_USER_JOINED, DEF_WAIT, interleave));

        assertEquals("same channel", client1.getMyChannelID(), client2.getMyChannelID());
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

        assertEquals("Make sub channel", ClientError.CMDERR_SUCCESS, server.makeChannel(chan));

        interleave.interleave();

        chan.szName = "foo2";
        assertEquals("Update sub channel", ClientError.CMDERR_SUCCESS, server.updateChannel(chan));
        
        interleave.interleave();

        assertEquals("Remove sub channel", ClientError.CMDERR_SUCCESS, server.removeChannel(chan.nChannelID));
        
        interleave.interleave();
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
        assertTrue("kick success", waitCmdSuccess(client2, cmdid, DEF_WAIT, interleave));

        Channel chan = new Channel();
        chan.nChannelID = 2;
        chan.nParentID = 1;
        chan.nMaxUsers = 10;
        chan.szName = "foo";
        chan.audiocodec = new AudioCodec(true);
        chan.audiocfg = new AudioConfig(true);

        assertEquals("Make sub channel", ClientError.CMDERR_SUCCESS, server.makeChannel(chan));

        waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW, DEF_WAIT, interleave);

        cmdid = client1.doJoinChannelByID(chan.nChannelID, "");
        assertTrue("Join new channel", cmdid>0);

        assertTrue("join channel", waitCmdSuccess(client1, cmdid, DEF_WAIT, interleave));

        assertEquals("Remove sub channel", ClientError.CMDERR_SUCCESS, server.removeChannel(chan.nChannelID));

        assertTrue("remove channel event", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_CHANNEL_REMOVE,
                                                        DEF_WAIT, new TTMessage(), interleave));
    
        assertEquals("No channel", 0, client1.getMyChannelID());

        assertTrue("Kick cmd", client2.doKickUser(client1.getMyUserID(), 0)>0);

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

        assertEquals("send message", 0, server.sendTextMessage(textmsg));

        TTMessage msg = new TTMessage();
        assertTrue("wait text msg", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_USER_TEXTMSG, DEF_WAIT, msg, interleave));

        assertEquals("from id", textmsg.nFromUserID, msg.textmessage.nFromUserID);
        assertEquals("msg content", textmsg.szMessage, msg.textmessage.szMessage);

        // custom message

        textmsg.nMsgType = TextMsgType.MSGTYPE_CUSTOM;
        textmsg.nToUserID = client1.getMyUserID();

        assertEquals("send message", 0, server.sendTextMessage(textmsg));

        // channel message

        textmsg.nMsgType = TextMsgType.MSGTYPE_CHANNEL;
        textmsg.nChannelID = client1.getMyChannelID();
        textmsg.nToUserID = 0;

        assertEquals("send message", 0, server.sendTextMessage(textmsg));

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_USER_TEXTMSG, DEF_WAIT, msg, interleave));

        assertEquals("from id", textmsg.nFromUserID, msg.textmessage.nFromUserID);
        assertEquals("msg content", textmsg.szMessage, msg.textmessage.szMessage);

        // broadcast mesage
        textmsg.nMsgType = TextMsgType.MSGTYPE_BROADCAST;
        textmsg.nChannelID = 0;
        textmsg.nToUserID = 0;

        assertEquals("send message", 0, server.sendTextMessage(textmsg));

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_USER_TEXTMSG, DEF_WAIT, msg, interleave));

        assertEquals("from id", textmsg.nFromUserID, msg.textmessage.nFromUserID);
        assertEquals("msg content", textmsg.szMessage, msg.textmessage.szMessage);
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
        assertTrue("Issue change nickname", cmdid>0);

        TTMessage msg = new TTMessage();
        assertTrue("Change nick error", waitCmdError(client1, cmdid, DEF_WAIT, msg, interleave));

        assertEquals("Error message", 4567, msg.clienterrormsg.nErrorNo);

        cmdid = client1.doChangeStatus(45, "This is also crap");
        assertTrue("Issue change status", cmdid>0);

        msg = new TTMessage();
        assertTrue("Change status error", waitCmdError(client1, cmdid, DEF_WAIT, msg, interleave));
        assertEquals("Error message", 4568, msg.clienterrormsg.nErrorNo);
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
            assertTrue("Failed to create file.txt: " + e, false);
        }

        int cmdid = client1.doSendFile(client1.getMyChannelID(), uploadfilename);
        assertTrue("upload issued", cmdid>0);

        TTMessage msg = new TTMessage();
        assertTrue("Send success", waitCmdSuccess(client1, cmdid, DEF_WAIT, interleave));

        assertTrue("file upload done", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_FILE_NEW, DEF_WAIT, msg, interleave));

        RemoteFile fileinfo = msg.remotefile;

        cmdid = client1.doRecvFile(client1.getMyChannelID(), fileinfo.nFileID, downloadfilename);
        assertTrue("download issued", cmdid>0);

        assertTrue("download success", waitCmdSuccess(client1, cmdid, DEF_WAIT, interleave));

        assertTrue("file download begin event", waitForEvent(client1, ClientEvent.CLIENTEVENT_FILETRANSFER, DEF_WAIT, msg, interleave));
        assertEquals("file download begin", FileTransferStatus.FILETRANSFER_ACTIVE, msg.filetransfer.nStatus);

        assertTrue("file download end event", waitForEvent(client1, ClientEvent.CLIENTEVENT_FILETRANSFER, DEF_WAIT, msg, interleave));

        assertEquals("file download finished", FileTransferStatus.FILETRANSFER_FINISHED, msg.filetransfer.nStatus);
        cmdid = client1.doDeleteFile(client1.getMyChannelID(), fileinfo.nFileID);
        assertTrue("delete issued", cmdid>0);

        assertTrue("file rm", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_FILE_REMOVE, DEF_WAIT, msg, interleave));

        try {
            DataInputStream uploaded = new DataInputStream(new FileInputStream(uploadfilename));
            DataInputStream downloaded = new DataInputStream(new FileInputStream(downloadfilename));
            byte[] upbuff = new byte[1024], downbuff = new byte[1024];
            int inup = 1, indown = 1;
            while(inup > 0 || indown > 0) {
                inup = uploaded.read(upbuff);
                indown = downloaded.read(downbuff);
                assertTrue("uploaded same as downloaded", Arrays.equals(upbuff, downbuff));
            }
        }
        catch(IOException e) {
            assertTrue("Failed to compare file" + e, false);
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
    public void testDnsResolve() throws Exception {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        if (ENCRYPTED) {
            System.out.println("Skipping test_DnsResolve in encrypted mode");
            return;
        }
        
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
        assertTrue("join channel", waitCmdSuccess(client, client.doJoinChannel(chan), DEF_WAIT, interleave));

        assertTrue("vox", client.enableVoiceTransmission(true));

        assertTrue("enable aud cb", admin.enableAudioBlockEvent(client.getMyUserID(), StreamType.STREAMTYPE_VOICE, true));

        assertFalse("no voice audioblock", waitForEvent(admin, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000, interleave));

        assertTrue(waitCmdSuccess(admin, admin.doSubscribe(client.getMyUserID(), Subscription.SUBSCRIBE_INTERCEPT_VOICE), DEF_WAIT, interleave));

        assertTrue("voice audioblock", waitForEvent(admin, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, interleave));

    }

    @Test
    public void testTransmitUsers() {
        
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.uUserRights = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_TRANSMIT_VOICE;
        useraccounts.add(useraccount);

        TeamTalkSrv server = newServerInstance();
        ServerInterleave interleave = new RunServer(server);

        TeamTalkBase client = newClientInstance();
        initSound(client);
        connect(server, client);
        login(server, client, NICKNAME, USERNAME, PASSWORD);

        Channel chan = buildDefaultChannel(client, "Some channel");
        chan.uChannelType |= ChannelType.CHANNEL_CLASSROOM;
        assertTrue("join channel", waitCmdSuccess(client, client.doJoinChannel(chan), DEF_WAIT, interleave));

        assertTrue("get new chan", client.getChannel(client.getMyChannelID(), chan));

        assertTrue("subscribe voice", waitCmdSuccess(client, client.doSubscribe(client.getMyUserID(),
                                                                                Subscription.SUBSCRIBE_VOICE), DEF_WAIT, interleave));

        assertTrue(client.enableVoiceTransmission(true));
        
        assertTrue("enable aud cb", client.enableAudioBlockEvent(client.getMyUserID(), StreamType.STREAMTYPE_VOICE, true));

        assertFalse("no voice audioblock", waitForEvent(client, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000, interleave));
        
        chan.transmitUsers[0][Constants.TT_TRANSMITUSERS_USERID_INDEX] = client.getMyUserID();
        chan.transmitUsers[0][Constants.TT_TRANSMITUSERS_STREAMTYPE_INDEX] = StreamType.STREAMTYPE_VOICE;

        assertTrue("update channel", waitCmdSuccess(client, client.doUpdateChannel(chan), DEF_WAIT, interleave));
        
        assertEquals("Tx user ID set", client.getMyUserID(), chan.transmitUsers[0][Constants.TT_TRANSMITUSERS_USERID_INDEX]);
        assertEquals("Tx streamtype set", StreamType.STREAMTYPE_VOICE, chan.transmitUsers[0][Constants.TT_TRANSMITUSERS_STREAMTYPE_INDEX]);

        assertTrue("voice audioblock", waitForEvent(client, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, interleave));
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
        assertTrue("get keepalive", client.getClientKeepAlive(ka));

        ServerProperties srvprop = new ServerProperties();
        assertTrue(client.getServerProperties(srvprop));

        assertEquals("tcp ping is half of user timeout (default)", srvprop.nUserTimeout * 1000 / 2, ka.nTcpKeepAliveIntervalMSec);
        
        srvprop.nUserTimeout = 4;
        assertTrue(waitCmdSuccess(client, client.doUpdateServer(srvprop), DEF_WAIT, interleave));

        assertTrue(client.getServerProperties(srvprop));
        
        assertTrue("get keepalive", client.getClientKeepAlive(ka));

        assertEquals("tcp ping is half of user timeout, 4 sec", srvprop.nUserTimeout * 1000 / 2, ka.nTcpKeepAliveIntervalMSec);

        ka.nUdpKeepAliveIntervalMSec = 1;

        assertTrue("set UDP keepalive", client.setClientKeepAlive(ka));

        assertFalse(waitForEvent(client, ClientEvent.CLIENTEVENT_NONE, 100, interleave));

        ka.nTcpKeepAliveIntervalMSec = 1;

        assertTrue("set TCP keepalive", client.setClientKeepAlive(ka));

        assertTrue("get keepalive", client.getClientKeepAlive(ka));

        assertEquals("ka.nTcpKeepAliveIntervalMSec is read-only", srvprop.nUserTimeout * 1000 / 2, ka.nTcpKeepAliveIntervalMSec);
    }
    
    public void _test_runServer() {

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
        return newServerInstance(systemid, SERVERBINDIP);
    }
    
    public TeamTalkSrv newServerInstance(String systemid, String bindip) {

        TeamTalkSrv server = new TeamTalk5Srv(cmdcallback, logger);
        if(ENCRYPTED)
            assertTrue("Set context", server.setEncryptionContext(CRYPTO_CERT_FILE, CRYPTO_KEY_FILE));

        assertEquals("File storage", ClientError.CMDERR_SUCCESS,
                     server.setChannelFilesRoot(FILESTORAGE_FOLDER, MAX_DISKUSAGE, DEFAULT_CHANNEL_QUOTA));

        Channel chan = new Channel();
        chan.nChannelID = 1;
        chan.nParentID = 0;
        chan.nMaxUsers = 10;
        chan.audiocodec = new AudioCodec(true);
        chan.audiocfg = new AudioConfig(true);
        chan.nDiskQuota = DEFAULT_CHANNEL_QUOTA;

        assertEquals("Make root channel", ClientError.CMDERR_SUCCESS, server.makeChannel(chan));

        if(systemid.isEmpty())
            assertTrue("Start server", server.startServer(bindip, TCPPORT, UDPPORT, ENCRYPTED));
        else
            assertTrue("Start server", server.startServerSysID(bindip, TCPPORT, UDPPORT, ENCRYPTED, systemid));

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

        if (ENCRYPTED) {
            
            // SSL client uses blocking connect, so we have to connect
            // in separate thread otherwise we cannot run the client
            // and server event-loop at the same time
            Thread t = new Thread(new Runnable(){
                    public void run() {
                        connect(ttclient, systemID, hostaddr, tcpport, udpport, nop);
                    }
                });
            t.start();

            while (t.isAlive()) {
                server.runEventLoop(0);
            }
        }
        else {
            connect(ttclient, systemID, hostaddr, tcpport, udpport, new RunServer(server));
        }
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
