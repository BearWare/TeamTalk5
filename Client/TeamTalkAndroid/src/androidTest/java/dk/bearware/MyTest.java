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

import junit.framework.Assert;

import java.util.Vector;

/**
 * Created by bdr on 3-06-16.
 */
public class MyTest extends TeamTalkTestCaseBase {

    public TeamTalkBase newClientInstance() {
        TeamTalkBase ttclient = new TeamTalk5();
        ttclients.add(ttclient);
        return ttclient;
    }

    protected void setUp() throws Exception {
        super.setUp();

        this.ADMIN_USERNAME = "admin";
        this.ADMIN_PASSWORD = "admin";

        this.IPADDR = "tt5us.bearware.dk";
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

    public void test_StreamMedia() {
        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);
        connect(ttclient);
        login(ttclient, getCurrentMethod(), "guest", "guest");
        joinRoot(ttclient);

        assertTrue("Stream media file", ttclient.startStreamingMediaFileToChannel("http://hi5.streamingsoundtracks.com", new VideoCodec()));

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 20000);

        assertTrue("Stop media stream", ttclient.stopStreamingMediaFileToChannel());
    }

    public void test_SharedAudioDevice() {
        TeamTalkBase ttclient1 = newClientInstance();
        TeamTalkBase ttclient2 = newClientInstance();

        Vector<SoundDevice> devs = new Vector<>();
        TeamTalkBase.getSoundDevices(devs);
        for(SoundDevice d : devs) {
            System.out.println("Sound Device #" + d.nDeviceID + " name: " + d.szDeviceName);
        }

        long sndloop1 = ttclient1.startSoundLoopbackTest(1, 0, 48000, 1, false, null);
        assertTrue("Start client 1 sound loop", sndloop1 != 0);
        long sndloop2 = ttclient2.startSoundLoopbackTest(1, 0, 48000, 1, false, null);
        assertTrue("Start client 2 sound loop", sndloop2 != 0);
        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue("Close sndloop1", ttclient1.closeSoundLoopbackTest(sndloop1));
        assertTrue("Close sndloop2", ttclient2.closeSoundLoopbackTest(sndloop2));
    }

}
