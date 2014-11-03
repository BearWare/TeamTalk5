package dk.bearware;

import junit.framework.TestCase;
import java.util.Vector;

import dk.bearware.*;

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
            UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO;
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

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000));

        assertTrue(ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true));

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));

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

        TTMessage msg = new TTMessage();

        assertTrue(ttclient.doListBans(0, 100)>0);
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_BANNEDUSER, DEF_WAIT, msg));
        BannedUser ban = msg.banneduser;
        assertTrue(ban.szIPAddress.length()>0);
    }
}
