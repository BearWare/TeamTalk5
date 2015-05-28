package dk.bearware;

import junit.framework.TestCase;
import java.util.Vector;
import java.io.FileOutputStream;
import java.io.IOException;

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
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE;
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
        
        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

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
        assertTrue(TeamTalk5.getVideoCaptureDevices(devs));
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
        assertTrue(TeamTalk5.getVideoCaptureDevices(devs));

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
        assertTrue(TeamTalk5.getMediaFileInfo(MEDIAFILE, mfi));
        
        VideoCodec vidcodec = new VideoCodec();
        vidcodec.nCodec = Codec.WEBM_VP8_CODEC;
        vidcodec.webm_vp8.nRcTargetBitrate = 256;

        assertTrue(ttclient.startStreamingMediaFileToChannel(MEDIAFILE, vidcodec));

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg));

        assertEquals(msg.mediafileinfo.nStatus, MediaFileStatus.MFS_STARTED);
        assertEquals(msg.mediafileinfo.szFileName, mfi.szFileName);
        assertEquals(msg.mediafileinfo.uDurationMSec, mfi.uDurationMSec);

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));

        VideoFrame vidfrm;
        int n_rx_frames = 0;
        while(hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_STREAM_VIDEO)) {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_MEDIAFILE_VIDEO, DEF_WAIT));

            vidfrm = ttclient.acquireUserMediaVideoFrame(ttclient.getMyUserID());
            if(vidfrm != null) {
                assertEquals(vidfrm.nWidth, mfi.videoFmt.nWidth);
                assertEquals(vidfrm.nHeight, mfi.videoFmt.nHeight);

                n_rx_frames++;
            }
        }
        assertTrue(n_rx_frames>0);
        assertTrue(ttclient.stopStreamingMediaFileToChannel());
    }

    public void test_13_MediaStorage() {
        TeamTalkBase ttclient = newClientInstance();

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getCurrentMethod();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        
        TTMessage msg = new TTMessage();

        initSound(ttclient);
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        Channel chan = new Channel();
        ttclient.getChannel(ttclient.getMyChannelID(), chan);
        assertTrue(ttclient.startRecordingMuxedAudioFile(chan.audiocodec, MUXEDMEDIAFILE, AudioFileFormat.AFF_WAVE_FORMAT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(),
                                                                 Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        ttclient.enableVoiceTransmission(true);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient.stopRecordingMuxedAudioFile());

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 0));

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

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        assertTrue(ttclient.enableVoiceTransmission(true));

        assertFalse("no voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000));

        assertTrue(ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true));

        assertTrue("gimme voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));

        assertEquals(TTType.__STREAMTYPE, msg.ttType);
        assertEquals(StreamType.STREAMTYPE_VOICE, msg.nStreamType);
            
        AudioBlock block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID());

        assertTrue(block.nSamples > 0);

        //drain message before we start calculating
        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 100));

        assertTrue(ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_MEDIAFILE_AUDIO, true));
        assertTrue(ttclient.startStreamingMediaFileToChannel(MEDIAFILE_AUDIO, new VideoCodec()));

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
    
    public void test_21_VidcapTest() {
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
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_VIDEOCAPTURE, DEF_WAIT, msg));
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
        
}
