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
public class MyTest extends TeamTalkTestCase {

    public TeamTalkBase newClientInstance() {
        TeamTalkBase ttclient = new TeamTalk5();
        ttclients.add(ttclient);
        return ttclient;
    }

    protected void setUp() throws Exception {
        super.setUp();

        this.ADMIN_USERNAME = "admin";
        this.ADMIN_PASSWORD = "admin";

        this.IPADDR = "192.168.0.68";
        this.TCPPORT = 10333;
        this.UDPPORT = 10333;
    }

    public void test_This() {
        TeamTalkBase ttclient = newClientInstance();
        assertTrue(ttclient != null);
        initSound(ttclient);
        connect(ttclient);
        login(ttclient, getCurrentMethod(), "guest", "guest");
        joinRoot(ttclient);
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
        TeamTalkBase ttclient3 = newClientInstance();
        TeamTalkBase ttclient4 = newClientInstance();

        int sndinputdevid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT | SoundDeviceConstants.TT_SOUNDDEVICE_SHARED_FLAG;
        int sndoutputdevid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT;

        for (TeamTalkBase ttclient : this.ttclients) {
            assertTrue("Init ttclient sound input device", ttclient.initSoundInputDevice(sndinputdevid));
            assertTrue("Init ttclient sound output device", ttclient.initSoundOutputDevice(sndoutputdevid));
        }

        int freq = 500;
        for (TeamTalkBase ttclient : this.ttclients) {
            connect(ttclient);
            login(ttclient, getCurrentMethod(), "guest", "guest");
            joinRoot(ttclient);
            ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq += 100);
        }

        // now we hear all clients transmitting at the same time
        for (TeamTalkBase ttclient : this.ttclients) {
            assertTrue("Transmit audio on ttclient", ttclient.enableVoiceTransmission(true));
        }
        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);
        for (TeamTalkBase ttclient : this.ttclients) {
            assertTrue("Stop transmit audio on ttclient", ttclient.enableVoiceTransmission(false));
        }
        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);

        // Create two separate channels, one for ttclient1, ttclient2 and one for ttclient3, ttclient4.
        Channel chan1 = buildDefaultChannel(ttclient1, "Opus Mono - 40 msec");
        assertEquals("opus default", chan1.audiocodec.nCodec, Codec.OPUS_CODEC);
        chan1.audiocodec.opus.nChannels = 1;
        chan1.audiocodec.opus.nTxIntervalMSec = 40;
        assertTrue("ttclient1 create channel", waitCmdSuccess(ttclient1, ttclient1.doJoinChannel(chan1), DEF_WAIT));
        assertTrue("ttclient2 join ttclient1's channel", waitCmdSuccess(ttclient2, ttclient2.doJoinChannelByID(ttclient1.getMyChannelID(), chan1.szPassword), DEF_WAIT));

        Channel chan2 = buildDefaultChannel(ttclient3, "Opus Stereo - 60 msec");
        assertEquals("opus default", chan2.audiocodec.nCodec, Codec.OPUS_CODEC);
        chan2.audiocodec.opus.nChannels = 2;
        chan2.audiocodec.opus.nTxIntervalMSec = 60;
        assertTrue("ttclient3 create channel", waitCmdSuccess(ttclient3, ttclient3.doJoinChannel(chan2), DEF_WAIT));
        assertTrue("ttclient4 join ttclient3's channel", waitCmdSuccess(ttclient4, ttclient4.doJoinChannelByID(ttclient3.getMyChannelID(), chan2.szPassword), DEF_WAIT));

        // now we should hear 5 second tone of each client on two different channels
        for (TeamTalkBase ttclient : this.ttclients) {
            assertTrue("Transmit audio on ttclient", ttclient.enableVoiceTransmission(true));
            waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);
            assertTrue("Stop transmit audio on ttclient", ttclient.enableVoiceTransmission(false));
            waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);
        }

        // put ttclient1,ttclient2 in 20 msec channel and redo test
        Channel chan3 = buildDefaultChannel(ttclient1, "Opus Stereo - 20 msec");
        assertEquals("opus default", chan3.audiocodec.nCodec, Codec.OPUS_CODEC);
        chan3.audiocodec.opus.nChannels = 1;
        chan3.audiocodec.opus.nTxIntervalMSec = 20;
        assertTrue("ttclient1 create channel", waitCmdSuccess(ttclient1, ttclient1.doJoinChannel(chan3), DEF_WAIT));
        assertTrue("ttclient2 join ttclient1's channel", waitCmdSuccess(ttclient2, ttclient2.doJoinChannelByID(ttclient1.getMyChannelID(), chan3.szPassword), DEF_WAIT));

        // now we should hear 5 second tone of each client on two different channels
        for (TeamTalkBase ttclient : this.ttclients) {
            assertTrue("Transmit audio on ttclient", ttclient.enableVoiceTransmission(true));
            waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);
            assertTrue("Stop transmit audio on ttclient", ttclient.enableVoiceTransmission(false));
            waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);
        }
    }

    public void test_SpeexDSP() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);

        // setup echo cancellation
        SpeexDSP spxdsp = new SpeexDSP(true);
        spxdsp.bEnableAGC = true;
        spxdsp.bEnableDenoise = true;
        spxdsp.nMaxNoiseSuppressDB = -30;
        assertTrue("SpeexDSP", ttclient.setSoundInputPreprocess(spxdsp));

        TTMessage msg = new TTMessage();

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        assertTrue("join root", ttclient.doJoinChannelByID(ttclient.getRootChannelID(), "") > 0);

        assertTrue("Wait for AGC error on ARMv7A", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_INTERNAL_ERROR, 1000));

        assertTrue("Leave channel", waitCmdSuccess(ttclient, ttclient.doLeaveChannel(), DEF_WAIT));

        spxdsp.bEnableAGC = false;
        assertTrue("SpeexDSP", ttclient.setSoundInputPreprocess(spxdsp));

        assertTrue("join root", ttclient.doJoinChannelByID(ttclient.getRootChannelID(), "") > 0);

        assertFalse("No AGC error on ARMv7A", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_INTERNAL_ERROR, 1000));
    }


    public void test_EnableAudioBlock() {
        super.test_AudioBlock();
    }

    public void test_LocalEnableAudioBlock() {
        super.test_LocalAudioBlock();
    }
}
