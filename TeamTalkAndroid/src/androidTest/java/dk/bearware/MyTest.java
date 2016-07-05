package dk.bearware;

import junit.framework.Assert;

/**
 * Created by bdr on 3-06-16.
 */
public class MyTest extends TeamTalkTestCaseBase {

    protected void setUp() throws Exception {
        super.setUp();

        this.ADMIN_USERNAME = "admin";
        this.ADMIN_PASSWORD = "admin";

        this.IPADDR = "tt5eu.bearware.dk";
        this.TCPPORT = 10335;
        this.UDPPORT = 10335;
    }

    public void test_This() {
        TeamTalkBase ttclient = newClientInstance();
        assertTrue(ttclient != null);
        connect(ttclient);
    }

    public void test_RestartSnd() {
        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);
        connect(ttclient);
        login(ttclient, getCurrentMethod(), "guest", "guest");
        joinRoot(ttclient);
        Assert.assertTrue("sub voice", ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE)>0);
        ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 600);
        Assert.assertTrue("tx voice", ttclient.enableVoiceTransmission(true));
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT);
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);
        Assert.assertTrue("close input devs", ttclient.closeSoundInputDevice());
        Assert.assertTrue("close output devs", ttclient.closeSoundOutputDevice());
        Assert.assertTrue("restart sound system", ttclient.restartSoundSystem());
        initSound(ttclient);
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT);
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);
    }
}
