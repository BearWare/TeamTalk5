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
        TeamTalkBase ttclient3 = newClientInstance();

        int sounddeviceid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT | SoundDeviceConstants.TT_SOUNDDEVICE_SHARED_FLAG;

        SoundDevice shareddev = null;
        Vector<SoundDevice> devs = new Vector<>();
        TeamTalkBase.getSoundDevices(devs);
        for(SoundDevice d : devs) {
            System.out.println("Sound Device #" + d.nDeviceID + " name: " + d.szDeviceName);
            if (d.nDeviceID == sounddeviceid)
                shareddev = d;
        }

        assertTrue("shared device exists", shareddev != null);

        // test two instances with same sample settings as original and one instance which requires resampling
        long sndloop1 = ttclient1.startSoundLoopbackTest(shareddev.nDeviceID, 0, 48000, 2, false, null);
        assertTrue("Start sound loop 1", sndloop1 != 0);
        long sndloop2 = ttclient2.startSoundLoopbackTest(shareddev.nDeviceID, 0, 48000, 2, false, null);
        assertTrue("Start sound loop 2", sndloop2 != 0);
        long sndloop3 = ttclient3.startSoundLoopbackTest(shareddev.nDeviceID, 0, 48000, 1, false, null);
        assertTrue("Start sound loop 3", sndloop3 != 0);

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue("Close sndloop1", ttclient1.closeSoundLoopbackTest(sndloop1));
        assertTrue("Close sndloop2", ttclient2.closeSoundLoopbackTest(sndloop2));
        assertTrue("Close sndloop3", ttclient3.closeSoundLoopbackTest(sndloop3));

        // test two instances which require resampling
        long sndloop4 = ttclient1.startSoundLoopbackTest(shareddev.nDeviceID, 0, 32000, 1, false, null);
        assertTrue("Start sound loop 4", sndloop4 != 0);
        long sndloop5 = ttclient2.startSoundLoopbackTest(shareddev.nDeviceID, 0, 44100, 2, false, null);
        assertTrue("Start sound loop 5", sndloop5 != 0);

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue("Close sndloop4", ttclient1.closeSoundLoopbackTest(sndloop4));
        assertTrue("Close sndloop5", ttclient2.closeSoundLoopbackTest(sndloop5));

        Vector<Long> sndloops = new Vector<>();
        // now go through all sample rates
        for(int samplerate : shareddev.inputSampleRates) {
            if (samplerate <= 0)
                continue;
            long sndloop = ttclient1.startSoundLoopbackTest(shareddev.nDeviceID, 0, samplerate, 1, false, null);
            assertTrue("Start sound loop at " + samplerate + " channels " + 1, sndloop != 0);
            sndloops.add(sndloop);
        }

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);

        for(long sndloop : sndloops) {
            assertTrue("Close sndloop", ttclient1.closeSoundLoopbackTest(sndloop));
        }
    }

    public void test_MultiClientOnSharedAudioDevice() {
        TeamTalkBase ttclient1 = newClientInstance();
        TeamTalkBase ttclient2 = newClientInstance();

        int sndinputdevid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT | SoundDeviceConstants.TT_SOUNDDEVICE_SHARED_FLAG;
        int sndoutputdevid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT;
        assertTrue("Init ttclient1 sound input device", ttclient1.initSoundInputDevice(sndinputdevid));
        assertTrue("Init ttclient1 sound output device", ttclient1.initSoundOutputDevice(sndoutputdevid));
        assertTrue("Init ttclient2 sound input device", ttclient2.initSoundInputDevice(sndinputdevid));
        assertTrue("Init ttclient2 sound output device", ttclient2.initSoundOutputDevice(sndoutputdevid));

        connect(ttclient1);
        login(ttclient1, getCurrentMethod(), "guest", "guest");
        joinRoot(ttclient1);
        ttclient1.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 600);

        connect(ttclient2);
        login(ttclient2, getCurrentMethod(), "guest", "guest");
        joinRoot(ttclient2);
        ttclient2.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 900);

        assertTrue("Transmit audio on ttclient1", ttclient1.enableVoiceTransmission(true));
        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);
        assertTrue("Stop transmit audio on ttclient1", ttclient1.enableVoiceTransmission(false));
        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue("Transmit audio on ttclient2", ttclient2.enableVoiceTransmission(true));
        waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 5000);
        assertTrue("Stop transmit audio on ttclient2", ttclient2.enableVoiceTransmission(false));
        waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue("Transmit audio on ttclient1", ttclient1.enableVoiceTransmission(true));
        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);
        assertTrue("Stop transmit audio on ttclient1", ttclient1.enableVoiceTransmission(false));
        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue("Transmit audio on ttclient2", ttclient2.enableVoiceTransmission(true));
        waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 5000);
        assertTrue("Stop transmit audio on ttclient2", ttclient2.enableVoiceTransmission(false));
        waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 1000);
    }
}
