package dk.bearware;

import junit.framework.TestCase;

public class TeamTalkServerTestCase extends TeamTalkTestCaseBase {

    protected void setUp() throws Exception {
        super.setUp();

        IPADDR = "192.168.1.110";
        TCPPORT = 12000;
        UDPPORT = 12000;
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

    boolean login_complete = false;

    ServerCallback cb = new ServerCallback() {

            public void userLogin(ClientErrorMsg lpClientErrorMsg,
                                  User lpUser, UserAccount lpUserAccount) {
                String str = String.format("Login attempt from IP %s, username=%s, password=%s",
                                           lpUser.szIPAddress, lpUserAccount.szUsername, 
                                           lpUserAccount.szPassword);
                System.out.println(str);

                //just accept user
                lpClientErrorMsg.nErrorNo = ClientError.CMDERR_SUCCESS;

                //setup UserAccount, i.e. user-type and user-rights, and return it to TeamTalk server
                lpUserAccount.uUserType = UserType.USERTYPE_DEFAULT;
                lpUserAccount.szNote = "Hello there";
                lpUserAccount.uUserRights = UserRight.USERRIGHT_VIEW_ALL_USERS |
                    UserRight.USERRIGHT_TRANSMIT_VOICE;

                login_complete = true;
            }
            
        };

    public void test_01_This() {

        TeamTalkSrv srv = new TeamTalkSrv(cb);

        Channel chan = new Channel();
        chan.nChannelID = 1;
        chan.nParentID = 0;
        chan.nMaxUsers = 10;
        chan.audiocodec = new AudioCodec(true);
        chan.audiocfg = new AudioConfig(true);

        assertEquals("Make root channel", ClientError.CMDERR_SUCCESS, srv.makeChannel(chan));

        assertTrue("Start server", srv.startServer(IPADDR, TCPPORT, UDPPORT, false));
        
        while(!login_complete) {
            if(srv.runEventLoop(1000))
                System.out.println("Event");
            else
                System.out.println("No Event");
        }
    }
}