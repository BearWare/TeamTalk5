package dk.bearware;

import junit.framework.TestCase;
import java.util.Vector;

public class TeamTalkServerTestCase extends TeamTalkTestCaseBase {

    Vector<TeamTalkSrv> servers = new Vector<TeamTalkSrv>();

    String FILESTORAGE_FOLDER = "/tmp";
    long MAX_DISKUSAGE = 1000000000, DEFAULT_CHANNEL_QUOTA = 10000000;

    protected void setUp() throws Exception {
        super.setUp();

        PROEDITION = true;

        // IPADDR = "127.0.0.1";
        // TCPPORT = 12000;
        // UDPPORT = 12000;

        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = ADMIN_USERNAME;
        useraccount.szPassword = ADMIN_PASSWORD;
        useraccount.uUserType = UserType.USERTYPE_ADMIN;
        useraccount.szNote = "An example administrator user account with all user-rights";
        useraccount.uUserRights = UserRight.USERRIGHT_ALL;
        
        useraccounts.add(useraccount);

    }

    protected void tearDown() throws Exception {
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
                String str = String.format("Login attempt from IP %s, username=%s, password=%s",
                                           lpUser.szIPAddress, lpUserAccount.szUsername, 
                                           lpUserAccount.szPassword);
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

    public void test_01_UserLogin() {

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

        assertTrue("Connect client", client1.connect(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED));

        while(server.runEventLoop(0));

        waitForEvent(client1, ClientEvent.CLIENTEVENT_CON_SUCCESS, 1000);

        int cmdid = client1.doLogin(getCurrentMethod(), useraccount.szUsername, useraccount.szPassword);
        assertTrue("Login client", cmdid > 0);

        while(server.runEventLoop(100));

        TTMessage msg = new TTMessage();
        //check that the client gets back the same user account we created in the server
        assertTrue("wait login", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDIN, DEF_WAIT, msg));
        assertEquals("Account identity", useraccount.szUsername, msg.useraccount.szUsername);
        assertEquals("Account type", useraccount.uUserType, msg.useraccount.uUserType);
        assertEquals("Account rights", useraccount.uUserRights, msg.useraccount.uUserRights);
        assertEquals("Account note", useraccount.szNote, msg.useraccount.szNote);

        assertTrue("wait success", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg));
        assertEquals("Login success", cmdid, msg.nSource);
    }

    public void test_02_CreateAccount() {

        TeamTalkSrv server = newServerInstance();
        TeamTalkBase client1 = newClientInstance();

        assertTrue("Connect client", client1.connect(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED));

        while(server.runEventLoop(0));

        waitForEvent(client1, ClientEvent.CLIENTEVENT_CON_SUCCESS, 1000);

        int cmdid = client1.doLogin(getCurrentMethod(), ADMIN_USERNAME, ADMIN_PASSWORD);
        assertTrue("Login client", cmdid > 0);

        while(server.runEventLoop(100));

        TTMessage msg = new TTMessage();
        assertTrue("wait success", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg));

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

        while(server.runEventLoop(100));

        assertEquals("One more account", useraccounts.size(), n_accounts + 1);

        waitCmdSuccess(client1, cmdid, 1000);

        UserAccount srv_ua = getUserAccount(useraccount.szUsername);
        assertEquals("Account identity", srv_ua.szUsername, useraccount.szUsername);
        assertEquals("Account type", srv_ua.uUserType, useraccount.uUserType);
        assertEquals("Account rights", srv_ua.uUserRights, useraccount.uUserRights);
        assertEquals("Account note", srv_ua.szNote, useraccount.szNote);
    }

    public void test_03_DeleteAccount() {

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

        assertTrue("Connect client1", client1.connect(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED));
        assertTrue("Connect client2", client2.connect(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED));

        while(server.runEventLoop(0));

        waitForEvent(client1, ClientEvent.CLIENTEVENT_CON_SUCCESS, 1000);
        waitForEvent(client2, ClientEvent.CLIENTEVENT_CON_SUCCESS, 1000);

        int cmdid = client1.doLogin(getCurrentMethod(), ADMIN_USERNAME, ADMIN_PASSWORD);
        assertTrue("Login client1", cmdid > 0);

        cmdid = client2.doLogin(getCurrentMethod(), useraccount.szUsername, useraccount.szPassword);
        assertTrue("Login client2", cmdid > 0);

        while(server.runEventLoop(100));

        TTMessage msg = new TTMessage();
        assertTrue("wait success1", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg));
        assertTrue("wait success2", waitForEvent(client2, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg));

        int n_accounts = useraccounts.size();
        cmdid = client1.doDeleteUserAccount(ADMIN_USERNAME);

        while(server.runEventLoop(100));

        assertEquals("One less account", useraccounts.size(), n_accounts - 1);

        waitCmdSuccess(client1, cmdid, 1000);

        cmdid = client2.doDeleteUserAccount(ADMIN_USERNAME);

        while(server.runEventLoop(100));

        waitCmdError(client2, cmdid, 1000);
        
    }

    public void test_04_banUser() {

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

        assertTrue("Connect client1", client1.connect(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED));
        assertTrue("Connect client2", client2.connect(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED));

        while(server.runEventLoop(0));

        waitForEvent(client1, ClientEvent.CLIENTEVENT_CON_SUCCESS, 1000);
        waitForEvent(client2, ClientEvent.CLIENTEVENT_CON_SUCCESS, 1000);

        int cmdid = client1.doLogin(getCurrentMethod(), ADMIN_USERNAME, ADMIN_PASSWORD);
        assertTrue("Login client1", cmdid > 0);

        cmdid = client2.doLogin(getCurrentMethod(), useraccount.szUsername, useraccount.szPassword);
        assertTrue("Login client2", cmdid > 0);

        while(server.runEventLoop(100));

        TTMessage msg = new TTMessage();
        assertTrue("wait success1", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg));
        assertTrue("wait success2", waitForEvent(client2, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg));

        int n_banned = banned_ipaddr.size();
        cmdid = client1.doBanUser(client2.getMyUserID(), 0);

        while(server.runEventLoop(100));

        assertTrue("wait success1", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg));

        assertEquals("Banned user", n_banned + 1, banned_ipaddr.size());

        cmdid = client1.doUnBanUser(banned_ipaddr.get(0), 0);

        while(server.runEventLoop(100));

        assertTrue("wait success1", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg));

        assertEquals("Banned user", n_banned, banned_ipaddr.size());

        cmdid = client1.doBanIPAddress("11.22.33.44", 0);

        while(server.runEventLoop(100));

        assertTrue("wait success1", waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, msg));

        assertEquals("Banned user", n_banned + 1, banned_ipaddr.size());
        
    }

    public void _test_05_runServer() {

        TeamTalkSrv server = newServerInstance();

        System.out.println("Running TeamTalk server forever");
        while(server.runEventLoop(-1));

    }

    public TeamTalkSrv newServerInstance() {

        TeamTalkSrv server = new TeamTalk5Srv(cmdcallback, logger);
        
        assertEquals("File storage", ClientError.CMDERR_SUCCESS,
                     server.setChannelFilesRoot(FILESTORAGE_FOLDER, MAX_DISKUSAGE, DEFAULT_CHANNEL_QUOTA));

        Channel chan = new Channel();
        chan.nChannelID = 1;
        chan.nParentID = 0;
        chan.nMaxUsers = 10;
        chan.audiocodec = new AudioCodec(true);
        chan.audiocfg = new AudioConfig(true);

        assertEquals("Make root channel", ClientError.CMDERR_SUCCESS, server.makeChannel(chan));

        assertTrue("Start server", server.startServer(IPADDR, TCPPORT, UDPPORT, ENCRYPTED));

        servers.add(server);

        return server;
    }
}
