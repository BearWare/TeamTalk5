package dk.bearware;

import junit.framework.TestCase;
import java.util.Vector;
import java.util.List;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.File;

import dk.bearware.AudioBlock;
import dk.bearware.AudioFileFormat;
import dk.bearware.Channel;
import dk.bearware.ClientEvent;
import dk.bearware.Codec;
import dk.bearware.SpeexConstants;
import dk.bearware.SpeexDSP;
import dk.bearware.StreamType;
import dk.bearware.TTMessage;
import dk.bearware.TeamTalkBase;

public class TeamTalkTestCase extends TeamTalkTestCaseBase {

    protected void setUp() throws Exception {
        super.setUp();
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test_01_This() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        // TTMessage msg = new TTMessage();
        // assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, DEF_WAIT, msg));

        // DesktopWindow wnd = ttclient.acquireUserDesktopWindow(msg.nSource);

        // System.out.println("Desktop window: " + wnd.nWidth + "x" + wnd.nHeight + " size " + wnd.frameBuffer.length);
    }

    public void test_02_SoundDevs() {
        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);
        
        assertTrue(ttclient.setSoundOutputVolume(100));
        assertTrue(ttclient.setSoundOutputMute(true));
    }
    
    public void test_03_Connect() {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        
        assertTrue("ping", waitCmdComplete(ttclient, ttclient.doPing(), DEF_WAIT));
    }
    
    public void test_04_Auth() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        
        UserAccount myaccount = new UserAccount();
        assertTrue(ttclient.getMyUserAccount(myaccount));
        assertEquals(USERNAME, myaccount.szUsername);
        //assertEquals(PASSWORD,  myaccount.szPassword);
        assertEquals(USERRIGHTS, myaccount.uUserRights);
        
        assertTrue(waitCmdSuccess(ttclient, ttclient.doLogout(), DEF_WAIT));

        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        assertTrue(ttclient.pumpMessage(ClientEvent.CLIENTEVENT_USER_STATECHANGE, ttclient.getMyUserID()));

        TTMessage msg = new TTMessage();

        assertTrue("Wait for state change", 
                   waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE,
                                DEF_WAIT, msg));

        
    }
    
    public void test_05_Channels() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        
        IntPtr howmany = new IntPtr(0);

        assertTrue(ttclient.getServerChannels(null, howmany));
        Channel[] channels = new Channel[howmany.value];
        assertTrue(ttclient.getServerChannels(channels, howmany));
        
        int chanid = ttclient.getRootChannelID();

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannelByID(chanid, ""), DEF_WAIT));
        
        chanid = ttclient.getMyChannelID();
        assertTrue(ttclient.getChannelUsers(chanid, null, howmany));
        User[] users = new User[howmany.value];
        assertTrue(ttclient.getChannelUsers(chanid, users, howmany));

        Channel chan = new Channel();
        chan.nParentID = ttclient.getRootChannelID();
        chan.szName = "My Channel";
        chan.szPassword = "2222";
        chan.szOpPassword = "123";
        chan.nMaxUsers = 200;
        chan.uChannelType = ChannelType.CHANNEL_DEFAULT;
        chan.audiocodec.nCodec = Codec.SPEEX_CODEC;
        chan.audiocodec.speex = new SpeexCodec();
        chan.audiocodec.speex.nBandmode = 1;
        chan.audiocodec.speex.nQuality = 5;
        chan.audiocodec.speex.nTxIntervalMSec = 40;
        chan.audiocodec.speex.bStereoPlayback = false;
        chan.audiocfg.bEnableAGC = true;
        
        assertTrue("join channel", waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        Channel joinchan = new Channel();
        assertTrue(ttclient.getChannel(ttclient.getChannelIDFromPath("/" + chan.szName), joinchan));

        joinchan.szName = "My Channel 2";
        
        assertTrue(waitCmdSuccess(ttclient, ttclient.doUpdateChannel(joinchan), DEF_WAIT));

        Channel joinchan2 = new Channel();
        assertTrue(ttclient.getChannel(joinchan.nChannelID, joinchan2));
        
        assertTrue(waitCmdSuccess(ttclient, ttclient.doLeaveChannel(), DEF_WAIT));

        //join channel again
        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));
    }
    
    public void test_06_GenericMethods() {
        
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, ADMIN_NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);
        joinRoot(ttclient);
        IntPtr howmany = new IntPtr(0);
        
        Channel chan = buildDefaultChannel(ttclient, "Some channel");
        
        assertTrue(waitCmdSuccess(ttclient, ttclient.doChangeNickname("Foobar"), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doChangeNickname(ADMIN_NICKNAME), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doChangeStatus(0, "My status message"), DEF_WAIT));
        
        TextMessage txtmsg = new TextMessage();
        txtmsg.nMsgType = TextMsgType.MSGTYPE_USER;
        txtmsg.nToUserID = ttclient.getMyUserID();
        txtmsg.szMessage = "My text message";

        assertTrue(waitCmdSuccess(ttclient, ttclient.doTextMessage(txtmsg), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doChannelOp(ttclient.getMyUserID(), 
                                     ttclient.getMyChannelID(), false), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doChannelOpEx(ttclient.getMyUserID(), 
                                       ttclient.getMyChannelID(), "123", true), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doKickUser(ttclient.getMyUserID(), 
                                    ttclient.getMyChannelID()), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doRemoveChannel(ttclient.getMyChannelID()), DEF_WAIT));

        chan.szName = "My Test Channel 2";

        assertTrue(waitCmdSuccess(ttclient, ttclient.doMakeChannel(chan), DEF_WAIT));

        int chanid = ttclient.getChannelIDFromPath("/" + chan.szName);

        assertTrue(waitCmdSuccess(ttclient, ttclient.doRemoveChannel(chanid), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

    }
    
    //no files on android dev
//    public void testFileTx() {
//        TeamTalkBase ttclient = newClientInstance();
//        connect(ttclient);
//        login(ttclient, ADMIN_NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);
//        
//        Channel chan = buildDefaultChannel(ttclient, "foo");
//        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));
//        
//        assertTrue(waitCmdSuccess(ttclient, ttclient.doSendFile(ttclient.getMyChannelID(),
//                   UPLOADFILE), DEF_WAIT));
//
//    }
    
    public void test_07_UserAccount() {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, ADMIN_NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);
        
        IntPtr howmany = new IntPtr(0);
        
        ServerProperties srvprop = new ServerProperties();
        assertTrue(ttclient.getServerProperties(srvprop));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doUpdateServer(srvprop), DEF_WAIT));
        
        assertTrue(waitCmdSuccess(ttclient, ttclient.doListUserAccounts(0, 100), DEF_WAIT));
        
//        howmany = new IntPtr(0);
//        assertTrue(ttclient.getUserAccounts(null, howmany));
//        
//        UserAccount[] accounts = new UserAccount[howmany.value];
//        assertTrue(ttclient.getUserAccounts(accounts, howmany));
//        UserAccount account = new UserAccount();
//        account.szUsername = "foo";
//        account.szPassword = "bar";
//        account.uUserType = UserType.USERTYPE_DEFAULT;
//        account.uUserRights = UserRight.USERRIGHT_NONE;
//        account.autoOperatorChannels[0] = ttclient.getRootChannelID();
//        account.autoOperatorChannels[1] = ttclient.getMyChannelID();
//
//        assertTrue(waitCmdSuccess(ttclient, ttclient.doDeleteUserAccount(account.szUsername), DEF_WAIT));
//
//        assertTrue(waitCmdSuccess(ttclient, ttclient.doNewUserAccount(account), DEF_WAIT));
    }
    
   
    public void test_08_Stats() {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, ADMIN_NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);
        
        assertTrue(waitCmdSuccess(ttclient, ttclient.doQueryServerStats(), DEF_WAIT));
        
        IntPtr howmany = new IntPtr(0);
        assertTrue(ttclient.getServerUsers(null, howmany));
        
        User[] users = new User[howmany.value];
        assertTrue(ttclient.getServerUsers(users, howmany));

        ClientStatistics stats = new ClientStatistics();
        assertTrue(ttclient.getClientStatistics(stats));
    }

    public void test_09_SendDesktopWindow() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                         UserRight.USERRIGHT_TRANSMIT_DESKTOP;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        DesktopWindow wnd = new DesktopWindow();
        wnd.nWidth = 128;
        wnd.nHeight = 128;
        wnd.bmpFormat = BitmapFormat.BMP_RGB32;
        wnd.nProtocol = DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1;
        wnd.frameBuffer = new byte[wnd.nWidth * wnd.nHeight * 4];

        assertTrue(ttclient.sendDesktopWindow(wnd, BitmapFormat.BMP_RGB32)>0);

        TTMessage msg = new TTMessage();

        while(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, 
                           DEF_WAIT, msg) && msg.nBytesRemain > 0) {
        }

        assertTrue(msg.nBytesRemain == 0);

        assertFalse(hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_TX_DESKTOP));

        assertTrue(hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_DESKTOP_ACTIVE));

        assertTrue(ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_DESKTOP)>0);

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, DEF_WAIT, msg));

        DesktopWindow wnd2 = ttclient.acquireUserDesktopWindow(msg.nSource);

        assertEquals(wnd2.nWidth, wnd.nWidth);
        assertEquals(wnd2.nHeight, wnd.nHeight);
        assertEquals(wnd2.frameBuffer.length, wnd.frameBuffer.length);

        assertTrue(ttclient.closeDesktopWindow());
    }

    public void test_10_VideoCaptureDevs() {
        TeamTalkBase ttclient = newClientInstance();
        Vector<VideoCaptureDevice> devs = new Vector<VideoCaptureDevice>();
        assertTrue(ttclient.getVideoCaptureDevices(devs));
        for(int i=0;i<devs.size();i++) {
            assertTrue(!devs.get(i).szDeviceID.isEmpty());
            assertTrue(!devs.get(i).szDeviceName.isEmpty());
            assertTrue(!devs.get(i).szCaptureAPI.isEmpty());

            System.out.println("Video dev #" + i + ":");
            System.out.println("\tName: " + devs.get(i).szDeviceName);
            System.out.println("\tID: " + devs.get(i).szDeviceID);
            System.out.println("\tAPI: " + devs.get(i).szCaptureAPI);
            System.out.println("\tVideo formats:");
            for(int j=0;j<devs.get(i).videoFormats.length;j++) {
                VideoFormat fmt = devs.get(i).videoFormats[j];
                assertTrue(fmt.nWidth >0);
                assertTrue(fmt.nHeight >0);
                assertTrue(fmt.nFPS_Numerator >0);
                assertTrue(fmt.nFPS_Denominator >0);
                assertTrue(fmt.picFourCC >0);
                System.out.println("\tFmt #"+j+": " + fmt.nWidth + "x" + 
                                   fmt.nHeight + " " + 
                                   fmt.nFPS_Numerator + "/" + 
                                   fmt.nFPS_Denominator + 
                                   " FourCC: " + fmt.picFourCC);
            }
        }

        VideoFormat fmt = devs.get(0).videoFormats[0];
        assertTrue(ttclient.initVideoCaptureDevice(devs.get(0).szDeviceID, 
                                                   fmt));

        TTMessage msg = new TTMessage();
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_VIDEOCAPTURE, DEF_WAIT));

        VideoFrame frm = ttclient.acquireUserVideoCaptureFrame(0);

        assertEquals(frm.nWidth, fmt.nWidth);
        assertEquals(frm.nHeight, fmt.nHeight);

        assertTrue(ttclient.closeVideoCaptureDevice());
    }

    public void test_11_VideoCaptureStream() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VIDEOCAPTURE;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        Vector<VideoCaptureDevice> devs = new Vector<VideoCaptureDevice>();
        assertTrue(ttclient.getVideoCaptureDevices(devs));

        VideoCaptureDevice dev = devs.get(0);
        VideoFormat fmt = new VideoFormat();
        fmt.nWidth = 320;
        fmt.nHeight = 240;
        fmt.nFPS_Numerator = 10;
        fmt.nFPS_Denominator = 1;
        fmt.picFourCC = FourCC.FOURCC_RGB32;

        assertTrue(ttclient.initVideoCaptureDevice(dev.szDeviceID, fmt));

        assertTrue(waitCmdComplete(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(),
                                                                  Subscription.SUBSCRIBE_VIDEOCAPTURE), 
                                   DEF_WAIT));

        VideoCodec vidcodec = new VideoCodec();
        vidcodec.nCodec = Codec.WEBM_VP8_CODEC;
        vidcodec.webm_vp8.nRcTargetBitrate = 256;
        
        assertTrue(ttclient.startVideoCaptureTransmission(vidcodec));

        TTMessage msg = new TTMessage();
        
        int wait_frames = 100, frames_ok = 0;

        while(wait_frames-- > 0) {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_VIDEOCAPTURE, DEF_WAIT, msg));
            if(msg.nSource == 0) {
                wait_frames++;
                continue;
            }

            VideoFrame frm = ttclient.acquireUserVideoCaptureFrame(ttclient.getMyUserID());
            if(frm != null) {
                assertEquals(frm.nWidth, fmt.nWidth);
                assertEquals(frm.nHeight, fmt.nHeight);
                frames_ok++;
            }
        }

        assertTrue(frames_ok>0);

        assertTrue(ttclient.stopVideoCaptureTransmission());

        assertTrue(waitCmdComplete(ttclient, ttclient.doUnsubscribe(ttclient.getMyUserID(),
                                                                  Subscription.SUBSCRIBE_VIDEOCAPTURE), 
                                   DEF_WAIT));

        assertTrue(ttclient.startVideoCaptureTransmission(vidcodec));

        //test video capture stream without initial key-frame
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue(waitCmdComplete(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(),
                                                                  Subscription.SUBSCRIBE_VIDEOCAPTURE), 
                                   DEF_WAIT));

        wait_frames = 100; frames_ok = 0;

        while(wait_frames-- > 0) {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_VIDEOCAPTURE, DEF_WAIT, msg));
            if(msg.nSource == 0) {
                wait_frames++;
                continue;
            }

            VideoFrame frm = ttclient.acquireUserVideoCaptureFrame(ttclient.getMyUserID());
            if(frm != null) {
                assertEquals(frm.nWidth, fmt.nWidth);
                assertEquals(frm.nHeight, fmt.nHeight);
                frames_ok++;
            }
        }

        assertTrue(frames_ok>0);

        assertTrue(ttclient.closeVideoCaptureDevice());
    }

    public void test_12_MediaStreaming() {
        TeamTalkBase ttclient = newClientInstance();

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        
        TTMessage msg = new TTMessage();

        initSound(ttclient);
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        MediaFileInfo mfi = new MediaFileInfo();
        assertTrue("Get media file info", ttclient.getMediaFileInfo(MEDIAFILE, mfi));
        
        VideoCodec vidcodec = new VideoCodec();
        vidcodec.nCodec = Codec.WEBM_VP8_CODEC;
        vidcodec.webm_vp8.nRcTargetBitrate = 256;

        assertTrue("Start", ttclient.startStreamingMediaFileToChannel(MEDIAFILE, vidcodec));

        assertTrue("Wait stream event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("Begin stream", msg.mediafileinfo.nStatus, MediaFileStatus.MFS_STARTED);
        assertEquals("Filename match", msg.mediafileinfo.szFileName, mfi.szFileName);
        assertEquals("Found duration", msg.mediafileinfo.uDurationMSec, mfi.uDurationMSec);

        assertTrue("Wait USER_STATECHANGE", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));

        VideoFrame vidfrm;
        int n_rx_frames = 0;
        while(hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_STREAM_VIDEO)) {
            assertTrue("Wait  MEDIAFILE_VIDEO", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_MEDIAFILE_VIDEO, DEF_WAIT));

            vidfrm = ttclient.acquireUserMediaVideoFrame(ttclient.getMyUserID());
            if(vidfrm != null) {
                assertEquals("Width ok", vidfrm.nWidth, mfi.videoFmt.nWidth);
                assertEquals("Height ok", vidfrm.nHeight, mfi.videoFmt.nHeight);

                n_rx_frames++;
            }
        }
        assertTrue("Received frames", n_rx_frames>0);
        assertTrue("Stopped", ttclient.stopStreamingMediaFileToChannel());
    }

    public void test_13_MediaStorage_WaveOutput() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_MULTI_LOGIN |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        
        TTMessage msg = new TTMessage();

        int freq = 300;
        TeamTalkBase ttclient;
        Vector<TeamTalkBase> clients = new Vector<TeamTalkBase>();
        for(int i=0;i<4;i++) {
            ttclient = newClientInstance();
            initSound(ttclient);
            assertTrue(ttclient.setSoundInputPreprocess(new SpeexDSP()));

            connect(ttclient);
            login(ttclient, "ttclient" + (i), USERNAME, PASSWORD);
            joinRoot(ttclient);

            ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq);
            clients.add(ttclient);

            freq += 150;
        }

        ttclient = clients.get(0);

        Channel chan = new Channel();
        ttclient.getChannel(ttclient.getMyChannelID(), chan);

        assertEquals("OPUS codec running", Codec.OPUS_CODEC, chan.audiocodec.nCodec);
        assertTrue("Opus to muxed wave", ttclient.startRecordingMuxedAudioFile(chan.audiocodec, MUXEDMEDIAFILE_WAVE, AudioFileFormat.AFF_WAVE_FORMAT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(),
                                                                 Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        assertTrue(ttclient.enableVoiceTransmission(true));

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient.stopRecordingMuxedAudioFile());

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 100));

        ttclient.enableVoiceTransmission(false);

        chan = buildDefaultChannel(ttclient, "Some channel", Codec.SPEEX_CODEC);
        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 50);

        for(int i=1;i<clients.size();i++) {
            assertTrue(waitCmdSuccess(clients.get(i), clients.get(i).doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT));
        }

        assertTrue("Speex to muxed wave", ttclient.startRecordingMuxedAudioFile(chan.audiocodec, MUXEDMEDIAFILE_WAVE, AudioFileFormat.AFF_WAVE_FORMAT));


        ttclient.enableVoiceTransmission(true);

        TeamTalkBase ttclient1 = clients.get(1);
        TeamTalkBase ttclient2 = clients.get(2);
        TeamTalkBase ttclient3 = clients.get(3);

        for(int i=0;i<5;i++) {

            ttclient1.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 300);
            assertTrue(ttclient1.enableVoiceTransmission(true));
            waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);
            ttclient1.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 0);

            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

            ttclient2.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 450);
            assertTrue(ttclient2.enableVoiceTransmission(true));
            waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 5000);
            ttclient2.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 0);

            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

            ttclient3.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 600);
            assertTrue(ttclient3.enableVoiceTransmission(true));
            waitForEvent(ttclient3, ClientEvent.CLIENTEVENT_NONE, 10000);
            ttclient3.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 0);

            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

            assertTrue(ttclient1.enableVoiceTransmission(false));
            assertTrue(ttclient2.enableVoiceTransmission(false));
            assertTrue(ttclient3.enableVoiceTransmission(false));
            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 45000);
        }

        for(int i=0;i<5;i++) {

            ttclient1.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 300);
            assertTrue(ttclient1.enableVoiceTransmission(true));
            waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 2000);
            ttclient1.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 0);

            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

            ttclient2.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 450);
            assertTrue(ttclient2.enableVoiceTransmission(true));
            waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 2000);
            ttclient2.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 0);

            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

            ttclient3.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 600);
            assertTrue(ttclient3.enableVoiceTransmission(true));
            waitForEvent(ttclient3, ClientEvent.CLIENTEVENT_NONE, 2000);
            ttclient3.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 0);

            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

            //assertTrue(ttclient1.enableVoiceTransmission(false));
            assertTrue(ttclient2.enableVoiceTransmission(false));
            assertTrue(ttclient3.enableVoiceTransmission(false));
            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);
        }

        assertTrue(ttclient.stopRecordingMuxedAudioFile());

    }

    public void test_13_MediaStorage_Self_WaveOutput() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_MULTI_LOGIN |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        
        TTMessage msg = new TTMessage();

        int freq = 300;
        TeamTalkBase ttclient;

        ttclient = newClientInstance();
        initSound(ttclient);
        assertTrue(ttclient.setSoundInputPreprocess(new SpeexDSP()));

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq);

        Channel chan = new Channel();
        ttclient.getChannel(ttclient.getMyChannelID(), chan);
        assertTrue(ttclient.startRecordingMuxedAudioFile(chan.audiocodec, MUXEDMEDIAFILE_WAVE, AudioFileFormat.AFF_WAVE_FORMAT));

        assertTrue(ttclient.enableVoiceTransmission(true));

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);

        assertTrue(ttclient.stopRecordingMuxedAudioFile());

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 100));

        ttclient.enableVoiceTransmission(false);
    }

    public void test_13_MediaStorage_OpusOutput() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_MULTI_LOGIN |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        
        TTMessage msg = new TTMessage();

        int freq = 300;
        TeamTalkBase ttclient1, ttclient2;

        ttclient1 = newClientInstance();
        initSound(ttclient1);
        assertTrue("set preprocess", ttclient1.setSoundInputPreprocess(new SpeexDSP()));

        ttclient2 = newClientInstance();
        initSound(ttclient2);
        assertTrue("set preprocess", ttclient2.setSoundInputPreprocess(new SpeexDSP()));

        connect(ttclient1);
        login(ttclient1, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient1);

        connect(ttclient2);
        login(ttclient2, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient2);

        ttclient1.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq);
        ttclient2.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq += 300);

        Channel chan = new Channel();
        ttclient1.getChannel(ttclient1.getMyChannelID(), chan);

        assertTrue(waitCmdSuccess(ttclient1, ttclient1.doSubscribe(ttclient1.getMyUserID(),
                                                                   Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        assertEquals("OPUS codec running", Codec.OPUS_CODEC, chan.audiocodec.nCodec);
        assertTrue("Mux to Opus file", ttclient1.startRecordingMuxedAudioFile(chan.audiocodec, MUXEDMEDIAFILE_OPUS, AudioFileFormat.AFF_CHANNELCODEC_FORMAT));

        assertTrue(ttclient1.enableVoiceTransmission(true));

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient1.enableVoiceTransmission(false);

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient2.enableVoiceTransmission(true));

        waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient2.enableVoiceTransmission(false);

        ttclient2.disconnect();

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);
        
        assertTrue(ttclient1.stopRecordingMuxedAudioFile());

        assertFalse(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 100));
    }

    public void test_13_MediaStorage_SpeexOutput() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_MULTI_LOGIN |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        
        TTMessage msg = new TTMessage();

        int freq = 300;
        TeamTalkBase ttclient1, ttclient2;

        ttclient1 = newClientInstance();
        initSound(ttclient1);
        assertTrue("set preprocess", ttclient1.setSoundInputPreprocess(new SpeexDSP()));

        ttclient2 = newClientInstance();
        initSound(ttclient2);
        assertTrue("set preprocess", ttclient2.setSoundInputPreprocess(new SpeexDSP()));

        connect(ttclient1);
        login(ttclient1, NICKNAME, USERNAME, PASSWORD);
        Channel chan = buildDefaultChannel(ttclient1, "speex channel", Codec.SPEEX_CODEC);
        assertTrue(waitCmdSuccess(ttclient1, ttclient1.doJoinChannel(chan), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient1, ttclient1.doSubscribe(ttclient1.getMyUserID(),
                                                                  Subscription.SUBSCRIBE_VOICE), DEF_WAIT));
        
        assertTrue("speex channel", ttclient1.getChannel(ttclient1.getMyChannelID(), chan));

        assertEquals("Speex codec running", Codec.SPEEX_CODEC, chan.audiocodec.nCodec);

        connect(ttclient2);
        login(ttclient2, NICKNAME, USERNAME, PASSWORD);
        assertTrue(waitCmdSuccess(ttclient2, ttclient2.doJoinChannelByID(chan.nChannelID, ""), DEF_WAIT));

        ttclient1.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq);
        ttclient2.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq += 300);

        // now store in Speex
        assertTrue("Mux to Speex file", ttclient1.startRecordingMuxedAudioFile(chan.audiocodec, MUXEDMEDIAFILE_SPEEX, AudioFileFormat.AFF_CHANNELCODEC_FORMAT));

        assertTrue(ttclient1.enableVoiceTransmission(true));

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient1.enableVoiceTransmission(false);

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient2.enableVoiceTransmission(true));

        waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient2.enableVoiceTransmission(false);

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);
        
        assertTrue(ttclient1.stopRecordingMuxedAudioFile());

        ttclient1.enableVoiceTransmission(false);
        ttclient2.enableVoiceTransmission(false);
        
        // Now store in Speex VBR
        chan = buildDefaultChannel(ttclient2, "speex vbr channel", Codec.SPEEX_VBR_CODEC);
        assertTrue("wait cmd", waitCmdSuccess(ttclient2, ttclient2.doJoinChannel(chan), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient2, ttclient2.doSubscribe(ttclient2.getMyUserID(),
                                                                   Subscription.SUBSCRIBE_VOICE), DEF_WAIT));
        
        // now store in Speex
        assertTrue("Mux to Speex VBR file", ttclient2.startRecordingMuxedAudioFile(chan.audiocodec, MUXEDMEDIAFILE_SPEEX_VBR, AudioFileFormat.AFF_CHANNELCODEC_FORMAT));

        assertTrue("get channel spx vbr", ttclient2.getChannel(ttclient2.getMyChannelID(), chan));

        assertEquals("Speex VBR codec running", Codec.SPEEX_VBR_CODEC, chan.audiocodec.nCodec);
        assertTrue(waitCmdSuccess(ttclient1, ttclient1.doJoinChannelByID(chan.nChannelID, ""), DEF_WAIT));

        assertTrue(ttclient1.enableVoiceTransmission(true));

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient1.enableVoiceTransmission(false);

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient2.enableVoiceTransmission(true));

        waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient2.enableVoiceTransmission(false);

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);
    }

    public void test_13_SelfEchoMediaStorage() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_MULTI_LOGIN |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        
        TTMessage msg = new TTMessage();

        int freq = 300;
        TeamTalkBase ttclient;

        ttclient = newClientInstance();
        initSound(ttclient);
        assertTrue(ttclient.setSoundInputPreprocess(new SpeexDSP()));

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq);

        Channel chan = new Channel();
        ttclient.getChannel(ttclient.getMyChannelID(), chan);
        assertTrue(ttclient.startRecordingMuxedAudioFile(chan.audiocodec, MUXEDMEDIAFILE_WAVE, AudioFileFormat.AFF_WAVE_FORMAT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(),
                                                                 Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        assertTrue(ttclient.enableVoiceTransmission(true));

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient.stopRecordingMuxedAudioFile());

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 100));

        ttclient.enableVoiceTransmission(false);
    }


    public void test_14_AudioBlock() {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
            UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO |
            UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();

        TTMessage msg = new TTMessage();

        connect(ttclient);
        initSound(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        Channel chan = buildDefaultChannel(ttclient, "Opus");
        assertEquals(chan.audiocodec.nCodec, Codec.OPUS_CODEC);

        assertTrue("join", waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        assertTrue("subscribe", waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        assertTrue("vox", ttclient.enableVoiceTransmission(true));

        assertFalse("no voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000));

        assertTrue("enable aud cb", ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true));

        assertTrue("gimme voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));

        assertEquals("StreamType", TTType.__STREAMTYPE, msg.ttType);
        assertEquals("Voice StreamType", StreamType.STREAMTYPE_VOICE, msg.nStreamType);
            
        AudioBlock block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID());

        assertTrue("aud block has samples", block.nSamples > 0);

        //drain message before we start calculating
        assertFalse("No queued events", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 100));

        assertTrue("Enable aud cb", ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_MEDIAFILE_AUDIO, true));
        assertTrue("Start stream file", ttclient.startStreamingMediaFileToChannel(MEDIAFILE_AUDIO, new VideoCodec()));

        int n_voice_blocks = 0, n_mfa_blocks = 0;
        while (n_voice_blocks < 10 || n_mfa_blocks < 10)
        {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            switch(msg.nStreamType)
            {
            case StreamType.STREAMTYPE_VOICE :
                n_voice_blocks++;
                block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                assertTrue(block.nSamples > 0);
                break;
            case StreamType.STREAMTYPE_MEDIAFILE_AUDIO :
                n_mfa_blocks++;
                block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_MEDIAFILE_AUDIO, msg.nSource);
                assertTrue(block.nSamples > 0);
                break;
            }
        }

        assertTrue("voice ok", n_voice_blocks >= 10);
        assertTrue("media file ok", n_mfa_blocks >= 10);

        assertTrue("stop streaming", ttclient.stopStreamingMediaFileToChannel());

        assertTrue("leave channel", waitCmdSuccess(ttclient, ttclient.doLeaveChannel(), DEF_WAIT));
        
        // drain audio blocks completely
        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000));
        while(ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID()) != null);

        //now test that mute stereo mode having effect
        chan.audiocodec = new AudioCodec();
        chan.audiocodec.nCodec = Codec.SPEEX_CODEC;
        chan.audiocodec.speex.bStereoPlayback = true;

        // test right channel is mute
        assertTrue("set right mute", ttclient.setUserStereo(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true, false));

        assertTrue("start tone", ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 440));

        assertTrue("join channel", waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        n_voice_blocks = 0;
        while (n_voice_blocks++ < 10)
        {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            assertEquals(StreamType.STREAMTYPE_VOICE, msg.nStreamType);

            block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertEquals("stereo", 2, block.nChannels);
            
            for(int i=0;i<block.lpRawAudio.length;i+=4) {
                assertEquals("right channel is mute", 0, block.lpRawAudio[i+2]);
                assertEquals("right channel is mute", 0, block.lpRawAudio[i+3]);
            }
        }

        assertTrue(ttclient.enableVoiceTransmission(false));

        // drain audio blocks completely
        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000));
        while(ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID()) != null);

        // test left channel is mute
        assertTrue("set left mute", ttclient.setUserStereo(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, false, true));

        assertTrue(ttclient.enableVoiceTransmission(true));

        n_voice_blocks = 0;
        while (n_voice_blocks++ < 10)
        {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            assertEquals(StreamType.STREAMTYPE_VOICE, msg.nStreamType);

            block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertEquals("stereo", 2, block.nChannels);
            
            for(int i=0;i<block.lpRawAudio.length;i+=4) {
                assertEquals("left channel is mute", 0, block.lpRawAudio[i]);
                assertEquals("left channel is mute", 0, block.lpRawAudio[i+1]);
            }
        }


    }

    public void test_15_ListAccounts() {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, "test_08_ListAccounts", ADMIN_USERNAME, ADMIN_PASSWORD);

        TTMessage msg = new TTMessage();

        assertTrue(ttclient.doListUserAccounts(0, 100)>0);
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_USERACCOUNT, DEF_WAIT, msg));
        UserAccount account = msg.useraccount;
        assertTrue(account.uUserType != UserType.USERTYPE_NONE);
    }

    public void test_16_ListBannedUsers() {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, "test_09_ListBannedUsers", ADMIN_USERNAME, ADMIN_PASSWORD);
        
        User user = new User();
        assertTrue(ttclient.getUser(ttclient.getMyUserID(), user));
        String IPADDR = "10.2.3.4";
        assertTrue(waitCmdSuccess(ttclient, ttclient.doBanUser(ttclient.getMyUserID(), 0), DEF_WAIT));
        assertTrue(waitCmdSuccess(ttclient, ttclient.doBanIPAddress(IPADDR, 0), DEF_WAIT));

        TTMessage msg = new TTMessage();

        assertTrue(ttclient.doListBans(0, 0, 100)>0);
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_BANNEDUSER, DEF_WAIT, msg));
        BannedUser ban = msg.banneduser;
        assertTrue(ban.szIPAddress.length()>0);
        
        assertTrue(waitCmdSuccess(ttclient, ttclient.doUnBanUser(user.szIPAddress, 0), DEF_WAIT));
        assertTrue(waitCmdSuccess(ttclient, ttclient.doUnBanUser(IPADDR, 0), DEF_WAIT));
    }
    
    public void test_17_ChannelSwitch() throws InterruptedException{
        
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();

        TTMessage msg = new TTMessage();

        connect(ttclient);
        initSound(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        assertTrue(ttclient.enableVoiceTransmission(true));
        assertTrue(ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true));
        assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        for(int i=0;i<5;i++) {

            AudioBlock audblk = new AudioBlock();
            for(int j=0;j<200;j++) {
                assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT));
                audblk = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID());
                assertTrue(audblk.nStreamID>0);
            }
            
            Channel chan;
            if(i % 2 == 0) {
                chan = buildDefaultChannel(ttclient, "Opus_" + i);
                assertEquals(chan.audiocodec.nCodec, Codec.OPUS_CODEC);
            }
            else {
                chan = new Channel();
                assertTrue(ttclient.getChannel(ttclient.getRootChannelID(), chan));
            }
            
            assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));
        }
    }
    
    public void bug_test_18_ViewAllUsers() throws InterruptedException {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - "
            + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL
            | UserRight.USERRIGHT_TRANSMIT_VOICE
            | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();

        TTMessage msg = new TTMessage();

        connect(ttclient);
        initSound(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        assertTrue(ttclient.enableVoiceTransmission(true));
        assertTrue(ttclient.enableAudioBlockEvent(ttclient.getMyUserID(),
            StreamType.STREAMTYPE_VOICE, true));
        assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(
            ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT));
    }
    
    public void test_19_MessageQueue() throws InterruptedException {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - "
            + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL
            | UserRight.USERRIGHT_TRANSMIT_VOICE
            | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();

        TTMessage msg = new TTMessage();

        String longstr = "";
        for(int i=0;i<Constants.TT_STRLEN-2;i++)
            longstr = longstr.concat("T");
        String channame = longstr.substring(0, 500); 
        
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        
        Channel chan = buildDefaultChannel(ttclient, channame);
        chan.nUserData = 0x7fffffff;
        chan.uChannelType = ~ChannelType.CHANNEL_PERMANENT;
        chan.szOpPassword = longstr;
        chan.szPassword = longstr;
        chan.szTopic = longstr;
        
        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));
        
        chan.nChannelID = ttclient.getMyChannelID();
        
        for(int j = 0;j < 20;j++) {
            for(int i = 0;i < 50;i++) {
                if(ttclient.doUpdateChannel(chan) < 0)
                    break;
            }
            Thread.sleep(500);
        }
        
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_INTERNAL_ERROR, DEF_WAIT, msg));
        assertTrue(msg.clienterrormsg.nErrorNo == ClientError.INTERR_TTMESSAGE_QUEUE_OVERFLOW);
        
        int cmdid = ttclient.doLeaveChannel();
        assertTrue("Update again after overflow", cmdid>0);
        
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_USER_LEFT, DEF_WAIT, msg));
        assertEquals("message queue lives again", ttclient.getMyUserID(), msg.user.nUserID);
        
    }
    
    public void test_20_WaveFile() throws IOException {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
            UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO |
            UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();

        TTMessage msg = new TTMessage();

        connect(ttclient);
        initSound(ttclient);
        //disable AGC
        assertTrue(ttclient.setSoundInputPreprocess(new SpeexDSP()));
        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        int WRITE_BYTES = 512000, v, CHANNELS = 2, SAMPLERATE = 16000;

        Channel chan = buildDefaultChannel(ttclient, "Speex", Codec.SPEEX_CODEC);
        chan.audiocodec.speex.nBandmode = SpeexConstants.SPEEX_BANDMODE_WIDE; //16000
        chan.audiocodec.speex.bStereoPlayback = true;
//        Channel chan = buildDefaultChannel(ttclient, "OPUS", Codec.OPUS_CODEC);
//        chan.audiocodec.opus.nChannels = CHANNELS;
//        chan.audiocodec.opus.nSampleRate = SAMPLERATE;

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        assertTrue(ttclient.enableVoiceTransmission(true));

//        assertTrue(ttclient.setUserMediaStorageDir(ttclient.getMyUserID(), "", "", AudioFileFormat.AFF_WAVE_FORMAT));
        
        assertFalse("no voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000));
        assertTrue(ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 440));
        assertTrue(ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true));

        FileOutputStream fs = new FileOutputStream("MyWaveFile.wav");

        fs.write(new String("RIFF").getBytes());
        v = WRITE_BYTES + 36 - 8;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF), (byte)((v>>16) & 0xFF), (byte)((v>>24) & 0xFF)}); //WRITE_BYTES - 36 - 8
        fs.write(new String("WAVEfmt ").getBytes());
        fs.write(new byte[] {0x10, 0x0, 0x0, 0x0}); //hdr size
        fs.write(new byte[] {0x1, 0x0}); //type
        fs.write(new byte[] {(byte)CHANNELS, 0x0}); //channels
        v = SAMPLERATE;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF), (byte)((v>>16) & 0xFF), (byte)((v>>24) & 0xFF)}); //sample rate
        v = (SAMPLERATE * 16 * CHANNELS) / 8;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF), (byte)((v>>16) & 0xFF), (byte)((v>>24) & 0xFF)}); //bytes/sec
        v = (16 * CHANNELS) / 8;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF)}); //block align
        fs.write(new byte[] {0x10, 0x0}); //bit depth
        fs.write(new String("data").getBytes());
        v = WRITE_BYTES - 44;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF), (byte)((v>>16) & 0xFF), (byte)((v>>24) & 0xFF)}); //WRITE_BYTES - 44

        while(WRITE_BYTES > 0) {
            assertTrue("gimme voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));

            AudioBlock block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID());
            assertTrue(block.nSamples > 0);
            assertEquals(CHANNELS, block.nChannels);
            assertEquals(SAMPLERATE, block.nSampleRate);
            
            if(block.lpRawAudio.length <= WRITE_BYTES)
                fs.write(block.lpRawAudio);
            else
                fs.write(block.lpRawAudio, 0, WRITE_BYTES);
            
            WRITE_BYTES -= block.lpRawAudio.length;
        }
        fs.close();
    }

    public void test_21_RecordPlayback() throws IOException {
        
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
            UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO |
            UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();

        TTMessage msg = new TTMessage();

        connect(ttclient);
        IntPtr indev = new IntPtr(), outdev = new IntPtr();
        assertTrue("get default sound devices", ttclient.getDefaultSoundDevices(indev, outdev));
        
        assertTrue("init input dev (we skip output device for now)", ttclient.initSoundInputDevice(indev.value));

        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        int WRITE_BYTES = 256000, v, CHANNELS = 1, SAMPLERATE = 16000;

        Channel chan = buildDefaultChannel(ttclient, "Speex", Codec.SPEEX_CODEC);
        chan.audiocodec.speex.nBandmode = SpeexConstants.SPEEX_BANDMODE_WIDE; //16000

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        assertTrue(ttclient.enableVoiceTransmission(true));

//        assertTrue(ttclient.setUserMediaStorageDir(ttclient.getMyUserID(), "", "", AudioFileFormat.AFF_WAVE_FORMAT));
        
        assertFalse("no voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000));

        assertTrue("pass 0 user id as MYSELF", ttclient.enableAudioBlockEvent(0, StreamType.STREAMTYPE_VOICE, true));

        FileOutputStream fs = new FileOutputStream("MyWaveFile.wav");

        fs.write(new String("RIFF").getBytes());
        v = WRITE_BYTES + 36 - 8;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF), (byte)((v>>16) & 0xFF), (byte)((v>>24) & 0xFF)}); //WRITE_BYTES - 36 - 8
        fs.write(new String("WAVEfmt ").getBytes());
        fs.write(new byte[] {0x10, 0x0, 0x0, 0x0}); //hdr size
        fs.write(new byte[] {0x1, 0x0}); //type
        fs.write(new byte[] {(byte)CHANNELS, 0x0}); //channels
        v = SAMPLERATE;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF), (byte)((v>>16) & 0xFF), (byte)((v>>24) & 0xFF)}); //sample rate
        v = (SAMPLERATE * 16 * CHANNELS) / 8;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF), (byte)((v>>16) & 0xFF), (byte)((v>>24) & 0xFF)}); //bytes/sec
        v = (16 * CHANNELS) / 8;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF)}); //block align
        fs.write(new byte[] {0x10, 0x0}); //bit depth
        fs.write(new String("data").getBytes());
        v = WRITE_BYTES - 44;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF), (byte)((v>>16) & 0xFF), (byte)((v>>24) & 0xFF)}); //WRITE_BYTES - 44

        while(WRITE_BYTES > 0) {
            assertTrue("gimme voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));

            AudioBlock block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, 0);
            assertTrue("audio block is valid", block != null);
            assertTrue(block.nSamples > 0);
            assertEquals(CHANNELS, block.nChannels);
            assertEquals(SAMPLERATE, block.nSampleRate);
            
            if(block.lpRawAudio.length <= WRITE_BYTES)
                fs.write(block.lpRawAudio);
            else
                fs.write(block.lpRawAudio, 0, WRITE_BYTES);
            
            WRITE_BYTES -= block.lpRawAudio.length;
        }
        fs.close();

        assertTrue("disable callback for MYSELF", ttclient.enableAudioBlockEvent(0, StreamType.STREAMTYPE_VOICE, false));

        assertTrue("disable voice now that we have the wav-file", ttclient.enableVoiceTransmission(false));

        assertTrue("init output dev, so we can hear recorded wavfile", ttclient.initSoundOutputDevice(outdev.value));

        assertTrue(ttclient.startStreamingMediaFileToChannel("MyWaveFile.wav", new VideoCodec()));

        assertTrue("get initial streaming event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("Stream started", msg.mediafileinfo.nStatus, MediaFileStatus.MFS_STARTED);
        
        assertTrue("wait for finish streaming event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("Stream ended", msg.mediafileinfo.nStatus, MediaFileStatus.MFS_FINISHED);
    }

    // test-case requires a user who is transmitting video capture to root channel
    public void test_22_VidcapTest() {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();

        TTMessage msg = new TTMessage();

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);
     
        int frames = 0;
        while(frames < 10) {
            assertTrue("Get video frame", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_VIDEOCAPTURE, DEF_WAIT, msg));
            long start = System.currentTimeMillis();
            VideoFrame frm = ttclient.acquireUserVideoCaptureFrame(msg.nSource);
            if(frm != null) {
                System.out.println("bearware: Frame "+ frm.nWidth +"x"+ frm.nHeight + " get time " + (System.currentTimeMillis() - start));
                frames++;
            }
            else {
                System.out.println("bearware: No frame built" + (System.currentTimeMillis() - start));
            }
        }
    }

    public void test_23_SoundDuplex() {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient, true);

        // setup echo cancellation
        SpeexDSP spxdsp = new SpeexDSP();
        spxdsp.bEnableAGC = true;
        spxdsp.bEnableDenoise = true;
        spxdsp.bEnableEchoCancellation = true;
        assertTrue(ttclient.setSoundInputPreprocess(spxdsp));

        TTMessage msg = new TTMessage();

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        assertFalse("Wait event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000, msg));
    }

    public void test_24_StoreUserVoiceInFileFormats() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_MULTI_LOGIN |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        
        TTMessage msg = new TTMessage();

        int freq = 300;
        TeamTalkBase ttclient;

        ttclient = newClientInstance();
        initSound(ttclient);
        assertTrue(ttclient.setSoundInputPreprocess(new SpeexDSP()));

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        Channel chan = new Channel();
        ttclient.getChannel(ttclient.getMyChannelID(), chan);

        assertEquals("OPUS codec running", Codec.OPUS_CODEC, chan.audiocodec.nCodec);

        ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq);

        assertTrue("wait cmd ok", waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(),
                                                                                Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        String cwd = System.getProperty("user.dir");
        assertTrue("specify audio storage", ttclient.setUserMediaStorageDir(ttclient.getMyUserID(),
                                                                            cwd, "%username%_%counter%",
                                                                            AudioFileFormat.AFF_CHANNELCODEC_FORMAT));

        assertTrue("enable voice tx", ttclient.enableVoiceTransmission(true));

        assertTrue("audio file created", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("recording started", MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus);
        assertEquals("correct filename", cwd + File.separator + USERNAME + "_" + "000000001.ogg", msg.mediafileinfo.szFileName);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient.enableVoiceTransmission(false);

        assertTrue("audio file stopped", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg));
        assertEquals("recording started", MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus);

        assertTrue("enable voice tx 2 ", ttclient.enableVoiceTransmission(true));

        assertTrue("audio file created 2", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("recording started 2", MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus);
        assertEquals("correct filename 2", cwd + File.separator + USERNAME + "_" + "000000002.ogg", msg.mediafileinfo.szFileName);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient.enableVoiceTransmission(false);

        assertTrue("audio file stopped 2", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg));
        assertEquals("recording started 2", MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus);

        // now test Speex recording
        chan = new Channel();
        chan.nParentID = ttclient.getRootChannelID();
        chan.szName = "My Channel";
        chan.szPassword = "2222";
        chan.szOpPassword = "123";
        chan.nMaxUsers = 200;
        chan.uChannelType = ChannelType.CHANNEL_DEFAULT;
        chan.audiocodec.nCodec = Codec.SPEEX_CODEC;
        chan.audiocodec.speex = new SpeexCodec();
        chan.audiocodec.speex.nBandmode = 1;
        chan.audiocodec.speex.nQuality = 5;
        chan.audiocodec.speex.nTxIntervalMSec = 40;
        chan.audiocodec.speex.bStereoPlayback = false;

        assertTrue("join speex channel", waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        assertTrue("enable voice tx 3", ttclient.enableVoiceTransmission(true));

        assertTrue("audio file created 3", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("recording started 3", MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus);
        assertEquals("correct filename 3", cwd + File.separator + USERNAME + "_" + "000000003.ogg", msg.mediafileinfo.szFileName);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient.enableVoiceTransmission(false);

        assertTrue("audio file stopped 3", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg));
        assertEquals("recording started 3", MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus);


        assertTrue("enable voice tx 4", ttclient.enableVoiceTransmission(true));

        assertTrue("audio file created 4", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("recording started 4", MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus);
        assertEquals("correct filename 4", cwd + File.separator + USERNAME + "_" + "000000004.ogg", msg.mediafileinfo.szFileName);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient.enableVoiceTransmission(false);

        assertTrue("audio file stopped 4", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg));
        assertEquals("recording started 4", MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus);

    }

    public void test_25_SoundLoopback() {
        TeamTalkBase ttclient;

        ttclient = newClientInstance();
        IntPtr in = new IntPtr(), out = new IntPtr();
        assertTrue("Get default sound devices", TeamTalkBase.getDefaultSoundDevices(in, out));

        SoundDevice nodev = null;
        Vector<SoundDevice> devs = new Vector<SoundDevice>();
        ttclient.getSoundDevices(devs);
        for(SoundDevice d : devs) {
            if(d.nSoundSystem == SoundSystem.SOUNDSYSTEM_NONE) {
                nodev = d;
                assertEquals("Virtual TeamTalk device", SoundDeviceConstants.TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL, d.nDeviceID);
            }
        }

        long loop = ttclient.startSoundLoopbackTest(in.value, out.value, 48000, 1, true, new SpeexDSP(true));
        assertTrue("Sound duplex loopback started", loop>0);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue("Loop duplex stopped", ttclient.closeSoundLoopbackTest(loop));

        loop = ttclient.startSoundLoopbackTest(in.value, out.value, 48000, 1, false, null);
        assertTrue("Sound loopback started", loop>0);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue("Loop stopped", ttclient.closeSoundLoopbackTest(loop));

        loop = ttclient.startSoundLoopbackTest(nodev.nDeviceID, out.value, 48000, 1, false, null);
        assertTrue("Sound loopback virtual input-dev started", loop>0);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue("Loop virtual input-dev stopped", ttclient.closeSoundLoopbackTest(loop));

        loop = ttclient.startSoundLoopbackTest(in.value, nodev.nDeviceID, 48000, 2, false, null);
        assertTrue("Sound loopback virtual output-dev started", loop>0);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue("Loop virtual output-dev stopped", ttclient.closeSoundLoopbackTest(loop));

        loop = ttclient.startSoundLoopbackTest(nodev.nDeviceID, nodev.nDeviceID, 48000, 2, true, new SpeexDSP(true));
        assertTrue("Sound loopback virtual duplex-dev started", loop>0);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue("Loop virtual duplex-dev stopped", ttclient.closeSoundLoopbackTest(loop));

        loop = ttclient.startSoundLoopbackTest(nodev.nDeviceID, out.value, 48000, 1, true, new SpeexDSP(true));
        assertTrue("Sound loopback virtual duplex-dev cannot be mixed with real dev", loop<=0);

        loop = ttclient.startSoundLoopbackTest(in.value, nodev.nDeviceID, 48000, 1, true, new SpeexDSP(true));
        assertTrue("Sound loopback virtual duplex-dev cannot be mixed with real dev", loop<=0);
    }

    public void test_26_VirtualSoundDevice() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_MULTI_LOGIN |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        
        TTMessage msg = new TTMessage();

        TeamTalkBase ttvirt = newClientInstance();
        TeamTalkBase ttclient = newClientInstance();
        
        initSound(ttclient);
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        assertTrue("Init virtual input dev", ttvirt.initSoundInputDevice(SoundDeviceConstants.TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL));
        assertTrue("Init virtual output dev", ttvirt.initSoundOutputDevice(SoundDeviceConstants.TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL));
        connect(ttvirt);
        login(ttvirt, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttvirt);

        assertTrue("Gen tone", ttvirt.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 300));
        assertTrue("Enable virtual voice transmission", ttvirt.enableVoiceTransmission(true));
        assertTrue("Wait for talking event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));
        assertEquals("User state to voice", UserState.USERSTATE_VOICE, msg.user.uUserState & UserState.USERSTATE_VOICE);
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue("Disable voice transmission", ttvirt.enableVoiceTransmission(false));
        assertTrue("Wait for talking event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));
        assertEquals("User state to voice", 0, msg.user.uUserState & UserState.USERSTATE_VOICE);


        assertTrue("Enable real voice transmission", ttclient.enableVoiceTransmission(true));
        assertTrue("Wait for talking event", waitForEvent(ttvirt, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));
        assertEquals("User state to voice", UserState.USERSTATE_VOICE, msg.user.uUserState & UserState.USERSTATE_VOICE);
        waitForEvent(ttvirt, ClientEvent.CLIENTEVENT_NONE, 5000);

    }
}
