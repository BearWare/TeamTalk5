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

import android.Manifest;
import android.os.Environment;
import androidx.test.rule.GrantPermissionRule;

import org.junit.Rule;
import org.junit.Test;
import org.junit.Before;
import org.junit.After;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;

import java.io.File;
import java.util.Vector;

/**
 * Created by bdr on 3-06-16.
 */
public class TeamTalkAndroidOnlyTest extends TeamTalkTestCaseBase {

    public TeamTalkBase newClientInstance() {
        TeamTalkBase ttclient = new TeamTalk5();
        ttclients.add(ttclient);
        return ttclient;
    }

    @Rule
    public GrantPermissionRule permissionRule1 = GrantPermissionRule.grant(Manifest.permission.RECORD_AUDIO,
            Manifest.permission.MODIFY_AUDIO_SETTINGS,
            Manifest.permission.INTERNET,
            Manifest.permission.VIBRATE,
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.WAKE_LOCK,
            Manifest.permission.READ_PHONE_STATE);

    @Before
    public void setUp() throws Exception {
        ADMIN_USERNAME = "admin";
        ADMIN_PASSWORD = "admin";

        IPADDR = "192.168.0.51";
        TCPPORT = 10333;
        UDPPORT = 10333;

        super.setUp();

        INPUTDEVICEID = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT | SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;
        OUTPUTDEVICEID = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT;

        File filepath = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
        STORAGEFOLDER = filepath.toString();
    }

    @After
    public void tearDown() throws Exception {
        super.tearDown();
    }

    @Test
    public void testRestartSnd() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_TRANSMIT_VOICE;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);
        assertTrue(ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE) > 0, "sub voice");
        ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 600);
        assertTrue(ttclient.enableVoiceTransmission(true), "tx voice");
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT);
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);
        assertTrue(ttclient.closeSoundInputDevice(), "close input devs");
        assertTrue(ttclient.closeSoundOutputDevice(), "close output devs");
        assertTrue(TeamTalkBase.restartSoundSystem(), "restart sound system");
        initSound(ttclient);
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT);
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);
    }

    @Test
    public void testStreamMedia() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        assertTrue(ttclient.startStreamingMediaFileToChannel("http://hi5.streamingsoundtracks.com", new VideoCodec()), "Stream media file");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 20000);

        assertTrue(ttclient.stopStreamingMediaFileToChannel(), "Stop media stream");
    }

    @Test
    public void testSharedAudioDevice() {
        TeamTalkBase ttclient1 = newClientInstance();
        TeamTalkBase ttclient2 = newClientInstance();
        TeamTalkBase ttclient3 = newClientInstance();

        SoundDevice shareddev = null;
        Vector<SoundDevice> devs = new Vector<>();
        TeamTalkBase.getSoundDevices(devs);
        for (SoundDevice d : devs) {
            System.out.println("Sound Device #" + d.nDeviceID + " name: " + d.szDeviceName);
            if (d.nDeviceID == SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT)
                shareddev = d;
        }

        assertNotNull(shareddev, "shared device exists");

        shareddev.nDeviceID = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT | SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;

        // test two instances with same sample settings as original and one instance which requires resampling
        long sndloop1 = TeamTalkBase.startSoundLoopbackTest(shareddev.nDeviceID, shareddev.nDeviceID, 48000, 2, false, null);
        assertTrue(sndloop1 != 0, "Start sound loop 1");
        long sndloop2 = TeamTalkBase.startSoundLoopbackTest(shareddev.nDeviceID, shareddev.nDeviceID, 48000, 2, false, null);
        assertTrue(sndloop2 != 0, "Start sound loop 2");
        long sndloop3 = TeamTalkBase.startSoundLoopbackTest(shareddev.nDeviceID, shareddev.nDeviceID, 48000, 1, false, null);
        assertTrue(sndloop3 != 0, "Start sound loop 3");

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue(TeamTalkBase.closeSoundLoopbackTest(sndloop1), "Close sndloop1");
        assertTrue(TeamTalkBase.closeSoundLoopbackTest(sndloop2), "Close sndloop2");
        assertTrue(TeamTalkBase.closeSoundLoopbackTest(sndloop3), "Close sndloop3");

        // test two instances which require resampling
        long sndloop4 = TeamTalkBase.startSoundLoopbackTest(shareddev.nDeviceID, shareddev.nDeviceID, 32000, 1, false, null);
        assertTrue(sndloop4 != 0, "Start sound loop 4");
        long sndloop5 = TeamTalkBase.startSoundLoopbackTest(shareddev.nDeviceID, shareddev.nDeviceID, 44100, 2, false, null);
        assertTrue(sndloop5 != 0, "Start sound loop 5");

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue(TeamTalkBase.closeSoundLoopbackTest(sndloop4), "Close sndloop4");
        assertTrue(TeamTalkBase.closeSoundLoopbackTest(sndloop5), "Close sndloop5");

        Vector<Long> sndloops = new Vector<>();
        // now go through all sample rates
        for (int samplerate : shareddev.inputSampleRates) {
            if (samplerate <= 0)
                continue;
            long sndloop = TeamTalkBase.startSoundLoopbackTest(shareddev.nDeviceID, shareddev.nDeviceID, samplerate, 1, false, null);
            assertTrue(sndloop != 0, "Start sound loop at " + samplerate + " channels " + 1);
            sndloops.add(sndloop);
        }

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);

        for (long sndloop : sndloops) {
            assertTrue(TeamTalkBase.closeSoundLoopbackTest(sndloop), "Close sndloop");
        }
    }

    @Test
    public void testMultiClientOnSharedAudioDevice() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_TRANSMIT_VOICE |
                UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_MULTI_LOGIN;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        Vector<TeamTalkBase> clients = new Vector<>();
        TeamTalkBase ttclient1 = newClientInstance();
        TeamTalkBase ttclient2 = newClientInstance();
        TeamTalkBase ttclient3 = newClientInstance();
        TeamTalkBase ttclient4 = newClientInstance();

        clients.add(ttclient1);
        clients.add(ttclient2);
        clients.add(ttclient3);
        clients.add(ttclient4);

        int sndinputdevid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT | SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;
        int sndoutputdevid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT;

        for (TeamTalkBase ttclient : clients) {
            assertTrue(ttclient.initSoundInputDevice(sndinputdevid), "Init ttclient sound input device");
            assertTrue(ttclient.initSoundOutputDevice(sndoutputdevid), "Init ttclient sound output device");
        }

        int freq = 500;
        for (TeamTalkBase ttclient : clients) {
            connect(ttclient);
            login(ttclient, NICKNAME, USERNAME, PASSWORD);
            joinRoot(ttclient);
            ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq += 100);
        }

        // now we hear all clients transmitting at the same time
        for (TeamTalkBase ttclient : clients) {
            assertTrue(ttclient.enableVoiceTransmission(true), "Transmit audio on ttclient");
        }
        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);
        for (TeamTalkBase ttclient : clients) {
            assertTrue(ttclient.enableVoiceTransmission(false), "Stop transmit audio on ttclient");
        }
        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);

        // Create two separate channels, one for ttclient1, ttclient2 and one for ttclient3, ttclient4.
        Channel chan1 = buildDefaultChannel(ttclient1, "Opus Mono - 40 msec");
        assertEquals(chan1.audiocodec.nCodec, Codec.OPUS_CODEC, "opus default");
        chan1.audiocodec.opus.nChannels = 1;
        chan1.audiocodec.opus.nTxIntervalMSec = 40;
        assertTrue(waitCmdSuccess(ttclient1, ttclient1.doJoinChannel(chan1), DEF_WAIT), "ttclient1 create channel");
        assertTrue(waitCmdSuccess(ttclient2, ttclient2.doJoinChannelByID(ttclient1.getMyChannelID(), chan1.szPassword), DEF_WAIT), "ttclient2 join ttclient1's channel");

        Channel chan2 = buildDefaultChannel(ttclient3, "Opus Stereo - 60 msec");
        assertEquals(chan2.audiocodec.nCodec, Codec.OPUS_CODEC, "opus default");
        chan2.audiocodec.opus.nChannels = 2;
        chan2.audiocodec.opus.nTxIntervalMSec = 60;
        assertTrue(waitCmdSuccess(ttclient3, ttclient3.doJoinChannel(chan2), DEF_WAIT), "ttclient3 create channel");
        assertTrue(waitCmdSuccess(ttclient4, ttclient4.doJoinChannelByID(ttclient3.getMyChannelID(), chan2.szPassword), DEF_WAIT), "ttclient4 join ttclient3's channel");

        // now we should hear 5 second tone of each client on two different channels
        for (TeamTalkBase ttclient : clients) {
            assertTrue(ttclient.enableVoiceTransmission(true), "Transmit audio on ttclient");
            waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);
            assertTrue(ttclient.enableVoiceTransmission(false), "Stop transmit audio on ttclient");
            waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);
        }

        // put ttclient1,ttclient2 in 20 msec channel and redo test
        Channel chan3 = buildDefaultChannel(ttclient1, "Opus Stereo - 20 msec");
        assertEquals(chan3.audiocodec.nCodec, Codec.OPUS_CODEC, "opus default");
        chan3.audiocodec.opus.nChannels = 1;
        chan3.audiocodec.opus.nFrameSizeMSec = 20;
        chan3.audiocodec.opus.nTxIntervalMSec = 20;
        assertTrue(waitCmdSuccess(ttclient1, ttclient1.doJoinChannel(chan3), DEF_WAIT), "ttclient1 create channel");
        assertTrue(waitCmdSuccess(ttclient2, ttclient2.doJoinChannelByID(ttclient1.getMyChannelID(), chan3.szPassword), DEF_WAIT), "ttclient2 join ttclient1's channel");

        // now we should hear 5 second tone of each client on two different channels
        for (TeamTalkBase ttclient : clients) {
            assertTrue(ttclient.enableVoiceTransmission(true), "Transmit audio on ttclient");
            waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);
            assertTrue(ttclient.enableVoiceTransmission(false), "Stop transmit audio on ttclient");
            waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);
        }
    }

    @Test
    public void testSpeexDSP() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);

        // setup echo cancellation
        SpeexDSP spxdsp = new SpeexDSP(true);
        spxdsp.bEnableAGC = true;
        spxdsp.bEnableDenoise = true;
        spxdsp.nMaxNoiseSuppressDB = -30;
        assertTrue(ttclient.setSoundInputPreprocess(spxdsp), "SpeexDSP");

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        assertTrue(ttclient.doJoinChannelByID(ttclient.getRootChannelID(), "") > 0, "join root");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_INTERNAL_ERROR, 1000), "Wait for AGC error on ARMv7A");

        assertTrue(waitCmdSuccess(ttclient, ttclient.doLeaveChannel(), DEF_WAIT), "Leave channel");

        spxdsp.bEnableAGC = false;
        assertTrue(ttclient.setSoundInputPreprocess(spxdsp), "SpeexDSP");

        assertTrue(ttclient.doJoinChannelByID(ttclient.getRootChannelID(), "") > 0, "join root");

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_INTERNAL_ERROR, 1000), "No AGC error on ARMv7A");
    }

    @Test
    public void testSoundDeviceEffects() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        int[] inputdevices = {
                SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT,
                SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_VOICECOM,
                SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT | SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG,
                SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_VOICECOM | SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG};

        int outputdeviceid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT | SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;

        for (int inputdeviceid : inputdevices) {
            TeamTalkBase ttclient = newClientInstance();
            initSound(ttclient, false, inputdeviceid, outputdeviceid);
            connect(ttclient);
            login(ttclient, NICKNAME, USERNAME, PASSWORD);

            // set AudioEffect prior to having sound device initialized
            SoundDeviceEffects effects = new SoundDeviceEffects();
            effects.bEnableAGC = true;
            effects.bEnableDenoise = true;
            effects.bEnableEchoCancellation = true;

            assertTrue(ttclient.setSoundDeviceEffects(effects), String.format("set android preprocessor, dev %x", inputdeviceid));

            joinRoot(ttclient);

            SoundDeviceEffects updatedEffects = new SoundDeviceEffects();
            assertTrue(ttclient.getSoundDeviceEffects(updatedEffects), String.format("get android preprocessor, dev %x", inputdeviceid));
            // cannot check that values have actually been set since only what is supported has been changed
            // assertEquals("AGC enabled", effects.bEnableAGC, updatedEffects.bEnableAGC);
            // assertEquals("AEC enabled", effects.bEnableEchoCancellation, updatedEffects.bEnableEchoCancellation);
            // assertEquals("denoise enabled", effects.bEnableDenoise, updatedEffects.bEnableDenoise);

            System.out.println("Testing sound device #" + inputdeviceid);
            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

            assertTrue(waitCmdSuccess(ttclient, ttclient.doLeaveChannel(), DEF_WAIT), String.format("leave channel, dev %x", inputdeviceid));
            assertTrue(ttclient.closeSoundInputDevice(), String.format("Close sound, dev %x", inputdeviceid));
        }
    }

    @Test
    public void testSndInputFailure() {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_MULTI_LOGIN |
                UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        int sndinputdevid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT | SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;
        int sndoutputdevid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT;

        // create 4 clients which will toggle PTT and open/close sound device
        Vector<TeamTalkBase> clients = new Vector<>();
        for (int i = 0; i < 4; ++i) {
            TeamTalkBase ttclient = newClientInstance();
            assertTrue(ttclient.initSoundOutputDevice(sndoutputdevid), "Init ttclient sound output device");

            // disable audio preprocessing
            SpeexDSP spxdsp = new SpeexDSP(true);
            spxdsp.bEnableAGC = false;
            spxdsp.bEnableDenoise = false;
            spxdsp.bEnableEchoCancellation = false;
            assertTrue(ttclient.setSoundInputPreprocess(spxdsp), "SpeexDSP");

            connect(ttclient);
            login(ttclient, NICKNAME, USERNAME, PASSWORD);
            Channel chan = buildDefaultChannel(ttclient, String.valueOf(ttclient.getMyUserID()), Codec.OPUS_CODEC);
            chan.audiocodec.opus.nFrameSizeMSec = 120;
            chan.audiocodec.opus.nTxIntervalMSec = 240;
            assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join channel");

            clients.add(ttclient);
        }

        // create 4 simulator clients which join in each of the clients's
        // channels
        Vector<TeamTalkBase> simclients = new Vector<>();
        for (int i = 0; i < 4; ++i) {
            TeamTalkBase sclient = newClientInstance();
            assertTrue(sclient.initSoundInputDevice(sndinputdevid), "Init sclient sound input device");
            assertTrue(sclient.initSoundOutputDevice(sndoutputdevid), "Init sclient sound output device");

            // disable audio preprocessing
            SpeexDSP spxdsp = new SpeexDSP(true);
            spxdsp.bEnableAGC = false;
            spxdsp.bEnableDenoise = false;
            spxdsp.bEnableEchoCancellation = false;
            assertTrue(sclient.setSoundInputPreprocess(spxdsp), "SpeexDSP");

            connect(sclient);
            login(sclient, NICKNAME, USERNAME, PASSWORD);
            assertTrue(waitCmdSuccess(sclient,
                    sclient.doJoinChannelByID(clients.elementAt(i).getMyChannelID(), ""),
                    DEF_WAIT), "join channel");

            simclients.add(sclient);
        }

        // now loop where the simulator clients are transmitting all
        // the time and the clients are PTT'ing every 15 seconds
        for (int x = 0; x < 3; x++) {
            for (TeamTalkBase sclient : simclients) {
                assertTrue(sclient.enableVoiceTransmission(true), "enable sim voice tx");
            }

            for (TeamTalkBase ttclient : clients) {
                assertTrue(ttclient.initSoundInputDevice(sndinputdevid), "Init ttclient sound input device");
                assertTrue(ttclient.enableVoiceTransmission(true), "Init voice TX");
            }

            waitForEvent(simclients.elementAt(0), ClientEvent.CLIENTEVENT_NONE, 15000);

            for (TeamTalkBase ttclient : clients) {
                assertTrue(ttclient.enableVoiceTransmission(false), "Stop voice TX");
                assertTrue(ttclient.closeSoundInputDevice(), "close ttclient sound input device");
            }

            for (TeamTalkBase sclient : simclients) {
                assertTrue(sclient.enableVoiceTransmission(false), "disable sim voice tx");
            }

            waitForEvent(simclients.elementAt(0), ClientEvent.CLIENTEVENT_NONE, 5000);

            for (TeamTalkBase ttclient : clients) {
                assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_INTERNAL_ERROR, 0), "No snd input error");
            }
        }
    }

    // force 'E/libOpenSLES: Too many objects' error
    @Test
    public void testMaxSoundOutputStreams() {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_MULTI_LOGIN |
                UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        int sndinputdevid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT | SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;
        int sndoutputdevid = SoundDeviceConstants.TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT;

        TeamTalkBase ttclient = newClientInstance();
        assertTrue(ttclient.initSoundInputDevice(sndinputdevid), "Init ttclient sound input device");
        assertTrue(ttclient.initSoundOutputDevice(sndoutputdevid), "Init ttclient sound output device");

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        Channel chan = buildDefaultChannel(ttclient, String.valueOf(ttclient.getMyUserID()), Codec.OPUS_CODEC);
        chan.audiocodec.opus.nFrameSizeMSec = 120;
        chan.audiocodec.opus.nTxIntervalMSec = 240;
        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join channel");

        boolean outputfailed = false;
        int outputs = 0;
        Vector<TeamTalkBase> simclients = new Vector<>();
        while (!outputfailed) {
            TeamTalkBase sclient = newClientInstance();
            assertTrue(sclient.initSoundInputDevice(SoundDeviceConstants.TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL), "Init sclient sound input device");

            connect(sclient);
            login(sclient, NICKNAME, USERNAME, PASSWORD);
            assertTrue(waitCmdSuccess(sclient,
                    sclient.doJoinChannelByID(ttclient.getMyChannelID(), ""),
                    DEF_WAIT), "join channel");
            assertTrue(sclient.enableVoiceTransmission(true), "enable tx");

            boolean outputok = false;
            TTMessage msg = new TTMessage();
            do {
                assertTrue(ttclient.getMessage(msg, DEF_WAIT), "wait for audio start event");

                switch (msg.nClientEvent) {
                    case ClientEvent.CLIENTEVENT_USER_STATECHANGE:
                        if (msg.user.nUserID == sclient.getMyUserID() && (msg.user.uUserState & UserState.USERSTATE_VOICE) != 0) {
                            outputok = true;
                            outputs++;
                        }
                        break;
                    case ClientEvent.CLIENTEVENT_INTERNAL_ERROR:
                        assertEquals(ClientError.INTERR_SNDOUTPUT_FAILURE, msg.clienterrormsg.nErrorNo, "new user stopped audio output");
                        outputfailed = true;
                        break;
                }

            } while (!outputok && !outputfailed);

            simclients.add(sclient);
        }

        System.out.println("Managed to create " + outputs + " audio outputs");

        // now destroy clients and see that audio output resurrects

        for (int i = 0; i < simclients.size() - 1; ++i) {
            simclients.elementAt(i).closeTeamTalk();
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_USER_LOGGEDOUT, DEF_WAIT), "wait logout");
        }

        // new client can talk
        TeamTalkBase sclient = newClientInstance();
        assertTrue(sclient.initSoundInputDevice(SoundDeviceConstants.TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL), "Init sclient sound input device");

        connect(sclient);
        login(sclient, NICKNAME, USERNAME, PASSWORD);
        assertTrue(waitCmdSuccess(sclient,
                sclient.doJoinChannelByID(ttclient.getMyChannelID(), ""),
                DEF_WAIT), "join channel");
        assertTrue(sclient.enableVoiceTransmission(true), "enable tx");

        TTMessage msg = new TTMessage();
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "talking event");
        assertTrue((msg.user.uUserState & UserState.USERSTATE_VOICE) != 0, "new user talking");
        assertEquals(sclient.getMyUserID(), msg.user.nUserID, "correct new user");

        // last client who couldn't talk has to be "resurrected"
        assertTrue(ttclient.closeSoundOutputDevice(), "close sndoutput");
        assertTrue(ttclient.initSoundOutputDevice(sndoutputdevid), "Init ttclient sound output device again");

        boolean outputrestarted = false;
        do {
            assertTrue(ttclient.getMessage(msg, DEF_WAIT), "user update event");
            switch (msg.nClientEvent) {
                case ClientEvent.CLIENTEVENT_USER_STATECHANGE:
                    outputrestarted = (msg.user.uUserState & UserState.USERSTATE_VOICE) != 0;
                    break;
            }
        } while (!outputrestarted);

        assertEquals(simclients.lastElement().getMyUserID(), msg.user.nUserID, "correct old user restarted");
    }

    @Test
    public void testWebRTCAudioPreprocessor() {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_MULTI_LOGIN |
                UserRight.USERRIGHT_TRANSMIT_VOICE;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);
        assertTrue(ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE) > 0, "sub voice");
        ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 600);
        assertTrue(ttclient.enableVoiceTransmission(true), "enable tx");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);

        // test WebRTC
        AudioPreprocessor preprocess = new AudioPreprocessor();
        preprocess.nPreprocessor = AudioPreprocessorType.WEBRTC_AUDIOPREPROCESSOR;
        preprocess.webrtc.noisesuppression.bEnable = true;
        preprocess.webrtc.noisesuppression.nLevel = 2;

        assertTrue(ttclient.setSoundInputPreprocess(preprocess), "Enable WebRTC");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);

        preprocess = new AudioPreprocessor();
        preprocess.nPreprocessor = AudioPreprocessorType.WEBRTC_AUDIOPREPROCESSOR;
        preprocess.webrtc.gaincontroller2.bEnable = true;
        preprocess.webrtc.gaincontroller2.fixeddigital.fGainDB = 20;

        assertTrue(ttclient.setSoundInputPreprocess(preprocess), "Enable WebRTC");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);

        preprocess = new AudioPreprocessor();
        assertTrue(ttclient.setSoundInputPreprocess(preprocess), "Enable WebRTC");
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);
    }
}
