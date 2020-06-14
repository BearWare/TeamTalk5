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

import junit.framework.TestCase;
import java.util.Vector;
import java.util.List;
import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.File;
import java.net.Socket;
import java.nio.ByteBuffer;

public abstract class TeamTalkTestCase extends TeamTalkTestCaseBase {

    protected void setUp() throws Exception {
        super.setUp();

        if (this.IPADDR.length() > 0)
            resetServerProperties();
    }

    public void testThis() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        // TTMessage msg = new TTMessage();
        // assertTrue("Wait desktop window", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, DEF_WAIT, msg));

        // DesktopWindow wnd = ttclient.acquireUserDesktopWindow(msg.nSource);

        // System.out.println("Desktop window: " + wnd.nWidth + "x" + wnd.nHeight + " size " + wnd.frameBuffer.length);
    }

    public void testSoundDevs() {
        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);

        assertTrue("Set output vol", ttclient.setSoundOutputVolume(100));
        assertTrue("Set output mute", ttclient.setSoundOutputMute(true));
    }

    public void testConnect() {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);

        assertTrue("ping", waitCmdComplete(ttclient, ttclient.doPing(), DEF_WAIT));
    }

    public void testAuth() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
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

    public void testChannels() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
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

    public void testGenericMethods() {

        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);
        joinRoot(ttclient);
        IntPtr howmany = new IntPtr(0);

        Channel chan = buildDefaultChannel(ttclient, "Some channel");

        assertTrue(waitCmdSuccess(ttclient, ttclient.doChangeNickname("Foobar"), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doChangeNickname(ADMIN_NICKNAME + " - " + getTestMethodName()), DEF_WAIT));

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

    public void testUserAccount() {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

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


    public void testStats() {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        assertTrue(waitCmdSuccess(ttclient, ttclient.doQueryServerStats(), DEF_WAIT));

        IntPtr howmany = new IntPtr(0);
        assertTrue(ttclient.getServerUsers(null, howmany));

        User[] users = new User[howmany.value];
        assertTrue(ttclient.getServerUsers(users, howmany));

        ClientStatistics stats = new ClientStatistics();
        assertTrue(ttclient.getClientStatistics(stats));
    }

    public void testSendDesktopWindow() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                         UserRight.USERRIGHT_TRANSMIT_DESKTOP | UserRight.USERRIGHT_MULTI_LOGIN;
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

        assertTrue("send desktop window", ttclient.sendDesktopWindow(wnd, BitmapFormat.BMP_RGB32)>0);

        TTMessage msg = new TTMessage();

        while(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER,
                           DEF_WAIT, msg) && msg.nBytesRemain > 0) {
        }

        assertTrue("All bytes transferred", msg.nBytesRemain == 0);

        assertFalse("no tx desktop flag", hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_TX_DESKTOP));

        assertTrue("Desktop active", hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_DESKTOP_ACTIVE));

        assertTrue("Subscribe to own", ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_DESKTOP)>0);

        assertTrue("Wait for desktop window", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, DEF_WAIT, msg));

        DesktopWindow wnd2 = ttclient.acquireUserDesktopWindow(msg.nSource);

        assertEquals("width", wnd2.nWidth, wnd.nWidth);
        assertEquals("height", wnd2.nHeight, wnd.nHeight);
        assertEquals("length", wnd2.frameBuffer.length, wnd.frameBuffer.length);

        assertTrue("Close desktop", ttclient.closeDesktopWindow());
    }

    public void testVideoCaptureDevs() {

        if (VIDEODEVICEID.equals(VIDEODEVDISABLE)) {
            System.err.println("Video capture test skipped due to device id: " + VIDEODEVDISABLE);
            return;
        }

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

    public void testVideoCaptureStream() {

        if (VIDEODEVICEID.equals(VIDEODEVDISABLE)) {
            System.err.println("Video capture test skipped due to device id: " + VIDEODEVDISABLE);
            return;
        }

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
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

    public void testMediaStreaming() {

        if (MEDIAFILE_VIDEO.isEmpty()) {
            System.err.println(getTestMethodName() + " skipped due to missing " + MEDIAFILE_VIDEO);
            return;
        }

        TeamTalkBase ttclient = newClientInstance();

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TTMessage msg = new TTMessage();

        initSound(ttclient);
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        MediaFileInfo mfi = new MediaFileInfo();
        assertTrue("Get media file info", ttclient.getMediaFileInfo(MEDIAFILE_VIDEO, mfi));

        VideoCodec vidcodec = new VideoCodec();
        vidcodec.nCodec = Codec.WEBM_VP8_CODEC;
        vidcodec.webm_vp8.nRcTargetBitrate = 256;

        assertTrue("Start", ttclient.startStreamingMediaFileToChannel(MEDIAFILE_VIDEO, vidcodec));

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

        // play again 90% into the media file
        MediaFilePlayback mfp = new MediaFilePlayback();
        mfp.uOffsetMSec = (int)(mfi.uDurationMSec * 0.9);
        mfp.bPaused = false;
        mfp.audioPreprocessor.nPreprocessor = AudioPreprocessorType.SPEEXDSP_AUDIOPREPROCESSOR;
        mfp.audioPreprocessor.speexdsp = new SpeexDSP(true);

        assertTrue("Start with offset", ttclient.startStreamingMediaFileToChannel(MEDIAFILE_VIDEO, mfp, vidcodec));

        assertTrue("Wait stream event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("Begin stream", msg.mediafileinfo.nStatus, MediaFileStatus.MFS_STARTED);
        assertEquals("Filename match", msg.mediafileinfo.szFileName, mfi.szFileName);
        assertEquals("Found duration", msg.mediafileinfo.uDurationMSec, mfi.uDurationMSec);
        assertTrue("Elapsed > mfp.uOffsetMSec", mfi.uElapsedMSec >= mfp.uOffsetMSec);

        assertTrue("Wait USER_STATECHANGE", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));

        n_rx_frames = 0;
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

    public void testMediaStreamingOffset() {

        if (MEDIAFILE_VIDEO.isEmpty()) {
            System.err.println(getTestMethodName() + " skipped due to missing " + MEDIAFILE_VIDEO);
            return;
        }

        TeamTalkBase ttclient = newClientInstance();

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TTMessage msg = new TTMessage();

        initSound(ttclient);
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        MediaFileInfo mfi = new MediaFileInfo();
        assertTrue("Get media file info", ttclient.getMediaFileInfo(MEDIAFILE_VIDEO, mfi));

        VideoCodec vidcodec = new VideoCodec();
        vidcodec.nCodec = Codec.WEBM_VP8_CODEC;
        vidcodec.webm_vp8.nRcTargetBitrate = 256;

        // play again 90% into the media file
        MediaFilePlayback mfp = new MediaFilePlayback();
        mfp.uOffsetMSec = (int)(mfi.uDurationMSec * 0.9);
        mfp.bPaused = false;
        mfp.audioPreprocessor.nPreprocessor = AudioPreprocessorType.SPEEXDSP_AUDIOPREPROCESSOR;
        mfp.audioPreprocessor.speexdsp = new SpeexDSP(true);

        assertTrue("Start with offset", ttclient.startStreamingMediaFileToChannel(MEDIAFILE_VIDEO, mfp, vidcodec));

        assertTrue("Wait stream event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("Begin stream", msg.mediafileinfo.nStatus, MediaFileStatus.MFS_STARTED);
        assertEquals("Filename match", msg.mediafileinfo.szFileName, mfi.szFileName);
        assertEquals("Found duration", msg.mediafileinfo.uDurationMSec, mfi.uDurationMSec);
        assertTrue("Elapsed >= mfp.uOffsetMSec", msg.mediafileinfo.uElapsedMSec >= mfp.uOffsetMSec);

        assertTrue("Wait USER_STATECHANGE", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));

        assertFalse("Media file is still playing", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, 0, msg));

        mfp.bPaused = true;
        vidcodec.webm_vp8.nRcTargetBitrate = 128;
        //rewind
        mfp.uOffsetMSec = (int)(mfi.uDurationMSec * 0.9);

        assertTrue("Pause media stream", ttclient.updateStreamingMediaFileToChannel(mfp, vidcodec));
        assertTrue("Wait stream event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg));
        assertEquals("Paused stream", msg.mediafileinfo.nStatus, MediaFileStatus.MFS_PAUSED);

        mfp.bPaused = false;
        assertTrue("Unpaused media stream", ttclient.updateStreamingMediaFileToChannel(mfp, vidcodec));
        assertTrue("Wait stream event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg));
        assertEquals("Started stream again", msg.mediafileinfo.nStatus, MediaFileStatus.MFS_STARTED);

        assertTrue("Stopped", ttclient.stopStreamingMediaFileToChannel());
    }

    public void testMediaStreaming_https() {

        if (HTTPS_MEDIAFILE.isEmpty()) {
            System.err.println(getTestMethodName() + " skipped due to empty HTTPS URL");
            return;
        }

        TeamTalkBase ttclient = newClientInstance();

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TTMessage msg = new TTMessage();

        initSound(ttclient);
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        MediaFileInfo mfi = new MediaFileInfo();
        assertTrue("Get media file info", ttclient.getMediaFileInfo(HTTPS_MEDIAFILE, mfi));

        VideoCodec vidcodec = new VideoCodec();

        assertTrue("Start", ttclient.startStreamingMediaFileToChannel(HTTPS_MEDIAFILE, vidcodec));

        assertTrue("Wait stream event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("Begin stream", msg.mediafileinfo.nStatus, MediaFileStatus.MFS_STARTED);
        assertEquals("Filename match", msg.mediafileinfo.szFileName, mfi.szFileName);
        assertEquals("Found duration", msg.mediafileinfo.uDurationMSec, mfi.uDurationMSec);

        assertTrue("Wait USER_STATECHANGE", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue("Stopped", ttclient.stopStreamingMediaFileToChannel());
    }



    public void testMediaStorage_WaveOutput() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
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
        assertTrue("Opus to muxed wave", ttclient.startRecordingMuxedAudioFile(chan.audiocodec,
                                                                               STORAGEFOLDER + File.separator + MUXEDMEDIAFILE_WAVE, AudioFileFormat.AFF_WAVE_FORMAT));

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

        assertTrue("Speex to muxed wave", ttclient.startRecordingMuxedAudioFile(chan.audiocodec,
                                                                                STORAGEFOLDER + File.separator + MUXEDMEDIAFILE_WAVE, AudioFileFormat.AFF_WAVE_FORMAT));


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

    public void testMediaStorage_Self_WaveOutput() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
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
        assertTrue("Record muxed audio file", ttclient.startRecordingMuxedAudioFile(chan.audiocodec,
                                                                                    STORAGEFOLDER + File.separator + MUXEDMEDIAFILE_WAVE, AudioFileFormat.AFF_WAVE_FORMAT));

        assertTrue("enable voice tx", ttclient.enableVoiceTransmission(true));

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);

        assertTrue("Stop recording muxed audio file", ttclient.stopRecordingMuxedAudioFile());

        assertFalse("Not record event should happen", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 100));

        ttclient.enableVoiceTransmission(false);
    }

    public void testMediaStorage_OpusOutput() {

        if (!OPUSTOOLS) {
            System.err.println(getTestMethodName() + " skipped due to OPUS tools disabled.");
            return;
        }

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
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

        assertTrue("do subscribe", waitCmdSuccess(ttclient1, ttclient1.doSubscribe(ttclient1.getMyUserID(),
                                                                                   Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        assertEquals("OPUS codec running", Codec.OPUS_CODEC, chan.audiocodec.nCodec);
        assertTrue("Mux to Opus file", ttclient1.startRecordingMuxedAudioFile(chan.audiocodec,
                                                                              STORAGEFOLDER + File.separator + MUXEDMEDIAFILE_OPUS, AudioFileFormat.AFF_CHANNELCODEC_FORMAT));

        assertTrue("enable voice tx 1", ttclient1.enableVoiceTransmission(true));

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient1.enableVoiceTransmission(false);

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue("enable voice tx 2", ttclient2.enableVoiceTransmission(true));

        waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient2.enableVoiceTransmission(false);

        ttclient2.disconnect();

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue("Stop recording", ttclient1.stopRecordingMuxedAudioFile());

        assertFalse(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 100));
    }

    public void testMediaStorage_SpeexOutput() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
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
        assertTrue("Mux to Speex file", ttclient1.startRecordingMuxedAudioFile(chan.audiocodec,
                                                                               STORAGEFOLDER + File.separator + MUXEDMEDIAFILE_SPEEX, AudioFileFormat.AFF_CHANNELCODEC_FORMAT));

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
        assertTrue("Mux to Speex VBR file", ttclient2.startRecordingMuxedAudioFile(chan.audiocodec,
                                                                                   STORAGEFOLDER + File.separator + MUXEDMEDIAFILE_SPEEX_VBR, AudioFileFormat.AFF_CHANNELCODEC_FORMAT));

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

    public void testSelfEchoMediaStorage() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
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
        assertTrue(ttclient.startRecordingMuxedAudioFile(chan.audiocodec,
                                                         STORAGEFOLDER + File.separator + MUXEDMEDIAFILE_WAVE, AudioFileFormat.AFF_WAVE_FORMAT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(),
                                                                 Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        assertTrue(ttclient.enableVoiceTransmission(true));

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient.stopRecordingMuxedAudioFile());

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 100));

        ttclient.enableVoiceTransmission(false);
    }

    public void testRecordMultipleChannels() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_MULTI_LOGIN |
            UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        Vector<TeamTalkBase> clients = new Vector<>();
        int[] txintervalsMSec = {40, 100, 250, 500};
        for (int i=0;i<4;++i) {

            TeamTalkBase ttclient1 = newClientInstance(), ttclient2 = newClientInstance();
            clients.add(ttclient1);
            clients.add(ttclient2);
            assertTrue("tone1", ttclient1.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 400));
            assertTrue("tone2", ttclient2.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 800));

            connect(ttclient1);
            connect(ttclient2);
            initSound(ttclient1);
            initSound(ttclient2);
            SpeexDSP spxdsp = new SpeexDSP();
            assertTrue("disable spx dsp", ttclient1.setSoundInputPreprocess(spxdsp));
            assertTrue("disable spx dsp", ttclient2.setSoundInputPreprocess(spxdsp));
            login(ttclient1, NICKNAME + "#" + ttclient1.getMyUserID(), USERNAME, PASSWORD);
            login(ttclient2, NICKNAME + "#" + ttclient2.getMyUserID(), USERNAME, PASSWORD);

            Channel chan = buildDefaultChannel(ttclient1, "Opus" + ttclient1.getMyUserID(), Codec.OPUS_CODEC);
            chan.audiocodec.opus.nTxIntervalMSec = txintervalsMSec[i];
            assertTrue("join1", waitCmdSuccess(ttclient1, ttclient1.doJoinChannel(chan), DEF_WAIT));

            assertTrue("join2", waitCmdSuccess(ttclient2, ttclient2.doJoinChannelByID(ttclient1.getMyChannelID(), ""), DEF_WAIT));
        }

        TeamTalkBase ttadmin = newClientInstance();
        connect(ttadmin);
        initSound(ttadmin);
        login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        for (TeamTalkBase ttclient : clients) {
            assertTrue("Intercept", waitCmdSuccess(ttadmin, ttadmin.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_INTERCEPT_VOICE), DEF_WAIT));
        }

        Channel chan = new Channel();
        Vector<String> filenames = new Vector<>();
        Vector<Integer> cbintervalMSec = new Vector<>();
        for (int i=0;i<clients.size();i+=2) {
            TeamTalkBase ttclient = clients.elementAt(i);
            assertTrue("get channel", ttclient.getChannel(ttclient.getMyChannelID(), chan));
            String filename = STORAGEFOLDER + File.separator + "MuxedRecording-" +
                chan.szName + "-#" + ttclient.getMyUserID() + "-" + chan.audiocodec.opus.nTxIntervalMSec + "msec.wav";

            filenames.add(filename);
            cbintervalMSec.add(chan.audiocodec.opus.nTxIntervalMSec);
            assertTrue("Record mux", ttadmin.startRecordingMuxedAudioFile(ttclient.getMyChannelID(), filename, AudioFileFormat.AFF_WAVE_FORMAT));
        }

        String rootfilename = STORAGEFOLDER + File.separator + "MuxedRecording-Root-Channel.wav";
        filenames.add(rootfilename);
        assertTrue("Record mux", ttadmin.startRecordingMuxedAudioFile(ttadmin.getRootChannelID(), rootfilename, AudioFileFormat.AFF_WAVE_FORMAT));
        assertTrue("get root channel", ttadmin.getChannel(ttadmin.getRootChannelID(), chan));
        cbintervalMSec.add(chan.audiocodec.opus.nTxIntervalMSec);

        assertEquals("filenames and cbintervals", filenames.size(), cbintervalMSec.size());

        long starttime = System.currentTimeMillis();

        waitForEvent(ttadmin, ClientEvent.CLIENTEVENT_NONE, 1000);

        for (int i=0;i<clients.size();i+=2) {
            TeamTalkBase ttclient = clients.elementAt(i);
            assertTrue("enable tone", ttclient.enableVoiceTransmission(true));
        }

        waitForEvent(ttadmin, ClientEvent.CLIENTEVENT_NONE, 3000);

        for (int i=0;i<clients.size();i+=2) {
            TeamTalkBase ttclient = clients.elementAt(i);
            assertTrue("disable tone", ttclient.enableVoiceTransmission(false));
        }

        waitForEvent(ttadmin, ClientEvent.CLIENTEVENT_NONE, 1000);

        for (int i=1;i<clients.size();i+=2) {
            TeamTalkBase ttclient = clients.elementAt(i);
            assertTrue("enable tone", ttclient.enableVoiceTransmission(true));
        }

        waitForEvent(ttadmin, ClientEvent.CLIENTEVENT_NONE, 3000);

        long duration = System.currentTimeMillis() - starttime;
        for (int i=0;i<clients.size();i+=2) {
            TeamTalkBase ttclient = clients.elementAt(i);
            assertTrue("Stop Record mux", ttadmin.stopRecordingMuxedAudioFile(ttclient.getMyChannelID()));
        }

        assertTrue("Stop Record root mux", ttadmin.stopRecordingMuxedAudioFile(ttadmin.getRootChannelID()));

        for (String filename : filenames) {
            MediaFileInfo mfi = new MediaFileInfo();
            assertTrue("Open media file " + filename, ttadmin.getMediaFileInfo(filename, mfi));
            // callbacks from sound input device may not have
            // completed when file was closed, so put in a
            // tolerance. This is not a very precise way of testing
            // the duration of the files, but at least they should be
            // somewhere near the duration of the recording.
            int tolerance = cbintervalMSec.remove(0) * 3;
            assertTrue(String.format("Media file %s duration is %d, must be >= %d, tolerance %d",
                                     filename, mfi.uDurationMSec, duration, tolerance),
                       mfi.uDurationMSec + tolerance >= duration);
        }
    }

    public void testAudioBlock() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
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
        int framesize = (int)((chan.audiocodec.opus.nTxIntervalMSec / 1000.) * chan.audiocodec.opus.nSampleRate);

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

        MediaFileInfo mfi = new MediaFileInfo();
        mfi.szFileName = STORAGEFOLDER + File.separator + "hest.wav";
        mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 2);
        mfi.uDurationMSec = 30 * 1000;

        assertTrue("Write media file", TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600));

        assertTrue("Start stream file", ttclient.startStreamingMediaFileToChannel(mfi.szFileName, new VideoCodec()));

        int n_voice_blocks = 0, n_mfa_blocks = 0, voice_sampleindex = -1, mfa_sampleindex = -1, voice_sid = -1, mfa_sid = -1;
        while (n_voice_blocks < 10 || n_mfa_blocks < 10)
        {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            switch(msg.nStreamType)
            {
            case StreamType.STREAMTYPE_VOICE :
                n_voice_blocks++;
                block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                assertTrue(block.nSamples > 0);
                if (voice_sampleindex == -1) {
                    voice_sampleindex = block.uSampleIndex;
                    voice_sid = block.nStreamID;
                }
                else {
                    assertEquals("Voice sample index match", voice_sampleindex, block.uSampleIndex);
                    assertEquals("Voice stream ID match", voice_sid, block.nStreamID);
                }
                voice_sampleindex += framesize;
                break;
            case StreamType.STREAMTYPE_MEDIAFILE_AUDIO :
                n_mfa_blocks++;
                block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_MEDIAFILE_AUDIO, msg.nSource);
                assertTrue(block.nSamples > 0);
                if (mfa_sampleindex == -1) {
                    mfa_sampleindex = block.uSampleIndex;
                    mfa_sid = block.nStreamID;
                }
                else {
                    assertEquals("Media file sample index match", mfa_sampleindex, block.uSampleIndex);
                    assertEquals("Media file stream ID match", mfa_sid, block.nStreamID);
                }
                mfa_sampleindex += framesize;
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
        assertFalse("message queue has no audio block", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 0));

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
            assertEquals("stream is voice for right mute", StreamType.STREAMTYPE_VOICE, msg.nStreamType);

            block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertTrue("got audio block for right mute", block != null);
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
        assertFalse("message queue has no audio block", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 0));

        // test left channel is mute
        assertTrue("set left mute", ttclient.setUserStereo(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, false, true));

        assertTrue(ttclient.enableVoiceTransmission(true));

        n_voice_blocks = 0;
        while (n_voice_blocks++ < 10)
        {
            assertTrue("got audio block", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            assertEquals("stream is voice for left mute", StreamType.STREAMTYPE_VOICE, msg.nStreamType);

            block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertTrue("got audio block for left mute", block != null);
            assertEquals("stereo", 2, block.nChannels);

            for(int i=0;i<block.lpRawAudio.length;i+=4) {
                assertEquals("left channel is mute", 0, block.lpRawAudio[i]);
                assertEquals("left channel is mute", 0, block.lpRawAudio[i+1]);
            }
        }
    }

    public void testLocalAudioBlock() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
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

        assertTrue("vox", ttclient.enableVoiceTransmission(true));

        assertFalse("no voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000));

        assertTrue("enable aud cb", ttclient.enableAudioBlockEvent(Constants.TT_LOCAL_USERID, StreamType.STREAMTYPE_VOICE, true));

        assertTrue("gimme voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));

        assertEquals("StreamType", TTType.__STREAMTYPE, msg.ttType);
        assertEquals("Voice StreamType", StreamType.STREAMTYPE_VOICE, msg.nStreamType);

        AudioBlock block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);

        assertEquals("First stream", 1, block.nStreamID);
        assertTrue("aud block has samples", block.nSamples > 0);

        int receiveSamples = block.nSampleRate * 3;
        while (receiveSamples > 0) {
            assertTrue("gimme 3 secs of voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            assertEquals("local userid", Constants.TT_LOCAL_USERID, msg.nSource);
            block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertEquals("Still first stream", 1, block.nStreamID);
            receiveSamples -= block.nSamples;
        }

        // ensure voice stream id changes
        assertTrue("disable vox", ttclient.enableVoiceTransmission(false));
        assertTrue("vox again", ttclient.enableVoiceTransmission(true));

        // drain remaining frames
        do {
            assertTrue("wait for next audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
        } while (block.nStreamID == 1);

        receiveSamples = block.nSampleRate * 2;
        while (receiveSamples > 0) {
            assertTrue("gimme 2 secs of voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertEquals("Second stream", 2, block.nStreamID);
            receiveSamples -= block.nSamples;
        }
    }

    public void testMuxedAudioBlock() throws IOException {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
            UserRight.USERRIGHT_TRANSMIT_VOICE |
            UserRight.USERRIGHT_MULTI_LOGIN;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase rxclient = newClientInstance();
        TeamTalkBase[] txclients = new TeamTalkBase[9];
        for (int i=0;i<txclients.length;++i)
            txclients[i] = newClientInstance();

        TTMessage msg = new TTMessage();

        connect(rxclient);
        initSound(rxclient);
        login(rxclient, NICKNAME, USERNAME, PASSWORD);

        Channel chan = buildDefaultChannel(rxclient, "Opus - Mux initial", Codec.OPUS_CODEC);
        chan.audiocodec.opus.nChannels = 2;
        chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
        chan.audiocodec.opus.bDTX = false;
        SpeexDSP spxdsp = new SpeexDSP();
        assertTrue("disable spx dsp", rxclient.setSoundInputPreprocess(spxdsp));
        assertTrue("Gen tone", rxclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 1000));

        assertTrue("join", waitCmdSuccess(rxclient, rxclient.doJoinChannel(chan), DEF_WAIT));

        assertTrue("get new chan", rxclient.getChannel(rxclient.getMyChannelID(), chan));

        int freq = 0;
        for (TeamTalkBase ttclient : txclients) {
            connect(ttclient);
            initSound(ttclient);
            login(ttclient, NICKNAME, USERNAME, PASSWORD);
            assertTrue("disable spx dsp", ttclient.setSoundInputPreprocess(spxdsp));
            assertTrue("Gen tone", ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq += 100));
            assertTrue("join", waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));
        }

        // limit number of active streams (we only care about the muxer)
        for (TeamTalkBase outerclient : txclients) {
            for (TeamTalkBase innerclient : txclients) {
                if (innerclient != outerclient)
                    assertTrue("unsubscribe", waitCmdSuccess(innerclient, innerclient.doUnsubscribe(outerclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT));
            }
        }

        assertTrue("enable aud cb", rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, true));
        assertTrue("enable aud mux file", rxclient.startRecordingMuxedAudioFile(chan.audiocodec, STORAGEFOLDER + File.separator + "muxfileoutput.wav", AudioFileFormat.AFF_WAVE_FORMAT));

        int bytelen = chan.audiocodec.opus.nSampleRate * chan.audiocodec.opus.nChannels * 12/*seconds*/ * 2 /*short*/;
        try (FileOutputStream fs = newWaveFile(STORAGEFOLDER + File.separator + "muxoutput_opus.wav", chan.audiocodec.opus.nSampleRate, chan.audiocodec.opus.nChannels, bytelen);) {

            int receiveSamples = chan.audiocodec.opus.nSampleRate;
            do {
                assertTrue("gimme 1 secs of voice audioblock", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
                assertEquals("muxed userid", Constants.TT_MUXED_USERID, msg.nSource);
                AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                assertTrue("block valid", block != null);
                assertTrue("aud block has samples", block.nSamples > 0);
                receiveSamples -= block.nSamples;
                fs.write(block.lpRawAudio);
            } while (receiveSamples > 0);
            // 1 sec

            assertTrue("enable tx", rxclient.enableVoiceTransmission(true));
            receiveSamples = chan.audiocodec.opus.nSampleRate;
            do {
                assertTrue("tone for 1 sec of voice audioblock", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
                assertEquals("muxed userid", Constants.TT_MUXED_USERID, msg.nSource);
                AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                assertTrue("block valid", block != null);
                receiveSamples -= block.nSamples;
                fs.write(block.lpRawAudio);
            } while (receiveSamples > 0);
            // 2 sec

            assertTrue("disable tx", rxclient.enableVoiceTransmission(false));

            for (TeamTalkBase ttclient : txclients) {
                assertTrue("enable tx", ttclient.enableVoiceTransmission(true));
                receiveSamples = chan.audiocodec.opus.nSampleRate;
                do {
                    assertTrue("gimme 1 sec tone of voice audioblock", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
                    assertEquals("muxed userid", Constants.TT_MUXED_USERID, msg.nSource);
                    AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                    assertTrue("block valid", block != null);
                    receiveSamples -= block.nSamples;
                    fs.write(block.lpRawAudio);
                } while (receiveSamples > 0);
                assertTrue("disable tx", ttclient.enableVoiceTransmission(false));
            }
            // 11 sec

            for (TeamTalkBase ttclient : txclients) {
                assertTrue("enable tx", ttclient.enableVoiceTransmission(true));
            }

            receiveSamples = chan.audiocodec.opus.nSampleRate;
            do {
                assertTrue("gimme 1 sec tone of voice audioblock", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
                assertEquals("muxed userid", Constants.TT_MUXED_USERID, msg.nSource);
                AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                assertTrue("block valid", block != null);
                receiveSamples -= block.nSamples;
                fs.write(block.lpRawAudio);
            } while (receiveSamples > 0);
            // 12 sec

            assertTrue("leave opus", waitCmdSuccess(rxclient, rxclient.doLeaveChannel(), DEF_WAIT));

            assertTrue("disable aud cb and remove pending audio blocks", rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, false));

            assertTrue("cleared audio blocks", rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_MUXED_USERID) == null);

            assertTrue("enable aud cb again", rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, true));

            assertFalse("no audio mux when out of channel", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 500));

            chan = buildDefaultChannel(rxclient, "Speex VBR", Codec.SPEEX_VBR_CODEC);

            assertTrue("join", waitCmdSuccess(rxclient, rxclient.doJoinChannel(chan), DEF_WAIT));

            assertTrue("get new chan", rxclient.getChannel(rxclient.getMyChannelID(), chan));
        }

        int samplerate;
        switch (chan.audiocodec.speex_vbr.nBandmode) {
        case SpeexConstants.SPEEX_BANDMODE_NARROW :
            samplerate = 8000; break;
        case SpeexConstants.SPEEX_BANDMODE_WIDE :
            samplerate = 16000; break;
        case SpeexConstants.SPEEX_BANDMODE_UWIDE :
            samplerate = 32000; break;
        default :
            assertTrue("invalid samplerate", false);
            samplerate = 0;
            break;
        }
        bytelen = samplerate * 5;
        try (FileOutputStream fs = newWaveFile(STORAGEFOLDER + File.separator + "muxoutput_speexvbr.wav", samplerate, 1, bytelen);) {
            int receiveSamples = samplerate;
            do {
                assertTrue("gimme 1 sec tone of voice speex vbr audioblock", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
                assertEquals("muxed userid", Constants.TT_MUXED_USERID, msg.nSource);
                AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                assertTrue("block valid", block != null);
                assertEquals("correct sample rate", samplerate, block.nSampleRate);
                assertEquals("correct channels", 1, block.nChannels);
                receiveSamples -= block.nSamples;
                fs.write(block.lpRawAudio);
            } while (receiveSamples > 0);

            for (TeamTalkBase ttclient : txclients) {
                assertTrue("join spx vbr", waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));
            }

            receiveSamples = samplerate * 4;
            do {
                assertTrue("gimme 4 sec tones of voice speex vbr audioblock", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
                assertEquals("muxed userid", Constants.TT_MUXED_USERID, msg.nSource);
                AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                assertTrue("block valid", block != null);
                assertEquals("correct sample rate", samplerate, block.nSampleRate);
                assertEquals("correct channels", 1, block.nChannels);
                receiveSamples -= block.nSamples;
                fs.write(block.lpRawAudio);
            } while (receiveSamples > 0);

            assertTrue("leave spx", waitCmdSuccess(rxclient, rxclient.doLeaveChannel(), DEF_WAIT));

            assertFalse("no audio mux when out of channel", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 500));

            // drain any remaining audio blocks
            while(rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_MUXED_USERID) != null);

            // ensure 'muxfileoutput.wav' will continue writing again
            chan = buildDefaultChannel(rxclient, "Opus - Muxed secondary", Codec.OPUS_CODEC);
            chan.audiocodec.opus.nChannels = 2;
            chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
            chan.audiocodec.opus.bDTX = false;

            assertTrue("join", waitCmdSuccess(rxclient, rxclient.doJoinChannel(chan), DEF_WAIT));

            assertTrue("get new chan", rxclient.getChannel(rxclient.getMyChannelID(), chan));
            receiveSamples = chan.audiocodec.opus.nSampleRate;
            do {
                assertTrue("gimme 1 secs of opus voice audioblock again", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
                assertEquals("muxed userid", Constants.TT_MUXED_USERID, msg.nSource);
                AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                assertTrue("block valid", block != null);
                assertTrue("aud block has samples", block.nSamples > 0);
                receiveSamples -= block.nSamples;
            } while (receiveSamples > 0);
        }
    }


    public void testMuxedAudioBlockSoundInputDisabled() throws IOException {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
            UserRight.USERRIGHT_TRANSMIT_VOICE |
            UserRight.USERRIGHT_MULTI_LOGIN;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase rxclient = newClientInstance();

        TTMessage msg = new TTMessage();

        connect(rxclient);
        initSound(rxclient);
        login(rxclient, NICKNAME, USERNAME, PASSWORD);

        Channel chan = buildDefaultChannel(rxclient, "Opus - Mux initial", Codec.OPUS_CODEC);
        chan.audiocodec.opus.nChannels = 2;
        chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
        chan.audiocodec.opus.bDTX = false;

        assertTrue("join", waitCmdSuccess(rxclient, rxclient.doJoinChannel(chan), DEF_WAIT));
        assertTrue("get new chan", rxclient.getChannel(rxclient.getMyChannelID(), chan));
        assertTrue("enable aud cb", rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, true));

        // first receive initial audio blocks with sound input device active
        int receiveBlocks = 5;
        do {
            assertTrue("gimme 1 secs of voice audioblock", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            assertEquals("muxed userid", Constants.TT_MUXED_USERID, msg.nSource);
            AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertTrue("block valid initially", block != null);
            assertTrue("aud block has samples", block.nSamples > 0);
            receiveBlocks--;
        } while (receiveBlocks > 0);

        // close sound input device and ensure we still receive samples
        assertTrue("close sound input", rxclient.closeSoundInputDevice());

        receiveBlocks = 10;
        do {
            assertTrue("gimme audioblock with sound input disabled", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            assertEquals("muxed userid", Constants.TT_MUXED_USERID, msg.nSource);
            AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertTrue("block valid with sound input disabled", block != null);
            assertTrue("aud block has samples", block.nSamples > 0);
            receiveBlocks--;
        } while (receiveBlocks > 0);

        // restart audio blocks while sound input device is disabled
        assertTrue("disable aud cb and remove pending audio blocks", rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, false));
        waitForEvent(rxclient, ClientEvent.CLIENTEVENT_NONE, 0);
        assertTrue("enable aud cb again", rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, true));

        receiveBlocks = 5;
        do {
            assertTrue("gimme audio block with reenabled audioblocks and sound input disabled", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            assertEquals("muxed userid", Constants.TT_MUXED_USERID, msg.nSource);
            AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertTrue("block valid after reenable", block != null);
            assertTrue("aud block has samples", block.nSamples > 0);
            receiveBlocks--;
        } while (receiveBlocks > 0);
    }

    public void testResampledAudioBlock() {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
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
        chan.audiocodec.opus.nFrameSizeMSec = 10;
        chan.audiocodec.opus.nTxIntervalMSec = 100;
        assertEquals(chan.audiocodec.nCodec, Codec.OPUS_CODEC);

        assertTrue("join", waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        assertTrue("vox", ttclient.enableVoiceTransmission(true));

        AudioFormat fmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 8000, 2);
        int framesize = (int)((chan.audiocodec.opus.nTxIntervalMSec / 1000.) * fmt.nSampleRate);

        // test for TT_LOCAL_USERID
        assertTrue("enable aud cb", ttclient.enableAudioBlockEvent(Constants.TT_LOCAL_USERID, StreamType.STREAMTYPE_VOICE, fmt, true));

        assertTrue("gimme voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));

        assertEquals("from local user", Constants.TT_LOCAL_USERID, msg.nSource);

        AudioBlock ab = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
        assertTrue("retrieve audioblock", ab != null);

        assertEquals("stereo", fmt.nChannels, ab.nChannels);
        assertEquals("sample rate", fmt.nSampleRate, ab.nSampleRate);
        assertEquals("frame size matches", framesize, ab.nSamples);

        assertTrue("gimme next voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));

        AudioBlock ab2 = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
        assertTrue("retrieve next audioblock", ab2 != null);

        assertEquals("same sample rate", fmt.nSampleRate, ab2.nSampleRate);
        assertEquals("sample counter matches", ab.uSampleIndex + framesize, ab2.uSampleIndex);

        assertTrue("disable local userid", ttclient.enableAudioBlockEvent(Constants.TT_LOCAL_USERID, StreamType.STREAMTYPE_VOICE, null, false));
        while (ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_LOCAL_USERID) != null);

        // now do same test for TT_MUXED_USERID
        assertTrue("enable aud cb for muxed", ttclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, fmt, true));
        assertTrue("gimme voice audioblock from muxed", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
        assertEquals("from muxed user", Constants.TT_MUXED_USERID, msg.nSource);

        ab = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
        assertTrue("retrieve audioblock from muxed", ab != null);

        assertEquals("stereo in muxed", fmt.nChannels, ab.nChannels);
        assertEquals("sample rate in muxed", fmt.nSampleRate, ab.nSampleRate);
        assertEquals("frame size matches in muxed", framesize, ab.nSamples);

        assertTrue("gimme next voice audioblock from muxed", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
        ab2 = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
        assertTrue("retrieve next audioblock from muxed", ab2 != null);

        assertEquals("same sample rate for muxed", fmt.nSampleRate, ab2.nSampleRate);
        assertEquals("sample counter matches for muxed", ab.uSampleIndex + framesize, ab2.uSampleIndex);
    }

    public void testUserEventMuxedStream() {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
            UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_MULTI_LOGIN;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase rxclient = newClientInstance();
        TeamTalkBase txclient = newClientInstance();

        TTMessage msg = new TTMessage();

        connect(rxclient);
        initSound(rxclient);
        login(rxclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(rxclient);

        connect(txclient);
        initSound(txclient);
        login(txclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(txclient);

        assertTrue("enable aud cb", rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, true));
        assertTrue("gimme voice audioblock", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
        assertTrue("enable tx", txclient.enableVoiceTransmission(true));
        assertTrue("User state changed to voice", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));
        assertTrue("User is talking", (msg.user.uUserState & UserState.USERSTATE_VOICE) == UserState.USERSTATE_VOICE);
        assertTrue("gimme voice audioblock again", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
        assertTrue("disable tx", txclient.enableVoiceTransmission(false));
        assertTrue("User state changed to no voice", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));
        assertTrue("User stopped talking", (msg.user.uUserState & UserState.USERSTATE_VOICE) == UserState.USERSTATE_NONE);
    }

    public void testOpusFrameSizeMSec() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
            UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO |
            UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();

        TTMessage msg = new TTMessage();

        connect(ttclient);
        initSound(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        assertTrue("Gen tone", ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 500));

        //                          0      1      2       3      4      5      6      7      8      9     10
        double[] txintervals = {  120,   120,   120,    120,   120,   120,    80,   200,   240,   500,  1000};
        double[] framesizes  = {  2.5,     5,    10,     20,    40,    60,    80,   100,   120,    10,    20};
        boolean[] vbr        = { true,  true,  true,  false,  true,  true,  true, false,  true,  true, false};
        int[] bitrates       = {    6,    32,    64,    128,   128,   256,   384,   160,   128,    60,    32};

        for (int i=0;i<5;i++) {

            double txintervalMSec = txintervals[i];
            double frameMSec = framesizes[i];

            Channel chan = buildDefaultChannel(ttclient, String.format("OPUS tx-interval %d, framesize %d, vbr %d, birate %d",
                                                                       (int)txintervalMSec,
                                                                       (int)frameMSec,
                                                                       (vbr[i]? 1:0),
                                                                       bitrates[i]));
            chan.audiocodec.opus.nTxIntervalMSec = (int)txintervalMSec;
            chan.audiocodec.opus.nFrameSizeMSec = (int)frameMSec;
            chan.audiocodec.opus.bVBR = vbr[i];
            //chan.audiocodec.opus.bVBRConstraint = (vbr[i])?true:false;
            chan.audiocodec.opus.nBitRate = bitrates[i] * 1000;
            assertEquals(chan.audiocodec.nCodec, Codec.OPUS_CODEC);
            chan.audiocodec.opus.nChannels = 2;

            assertTrue("join with interval "+chan.audiocodec.opus.nTxIntervalMSec+" framesize " + frameMSec,
                       waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

            Channel newchan = new Channel();
            assertTrue("get new chan", ttclient.getChannel(ttclient.getMyChannelID(), newchan));
            assertEquals("same tx interval", chan.audiocodec.opus.nTxIntervalMSec, newchan.audiocodec.opus.nTxIntervalMSec);
            assertEquals("same frame size", chan.audiocodec.opus.nFrameSizeMSec, newchan.audiocodec.opus.nFrameSizeMSec);

            assertTrue("subscribe", waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

            assertTrue("enable aud cb", ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true));

            assertTrue("vox", ttclient.enableVoiceTransmission(true));

            assertTrue("gimme voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));

            AudioBlock block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID());

            assertEquals("aud block has " + chan.audiocodec.opus.nTxIntervalMSec + " msec samples",
                         (int)(chan.audiocodec.opus.nSampleRate * (chan.audiocodec.opus.nTxIntervalMSec / 1000.)), block.nSamples);

            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 2000, msg);

            assertTrue("vox disable", ttclient.enableVoiceTransmission(false));

            assertTrue("leave", waitCmdSuccess(ttclient, ttclient.doLeaveChannel(), DEF_WAIT));

            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 0, msg);

            assertTrue("disable aud cb", ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, false));
        }
    }

    public void testListAccounts() {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, "test08_ListAccounts", ADMIN_USERNAME, ADMIN_PASSWORD);

        TTMessage msg = new TTMessage();

        assertTrue(ttclient.doListUserAccounts(0, 100)>0);
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_USERACCOUNT, DEF_WAIT, msg));
        UserAccount account = msg.useraccount;
        assertTrue(account.uUserType != UserType.USERTYPE_NONE);
    }

    public void testListBannedUsers() {

        TeamTalkBase ttadmin = newClientInstance();
        connect(ttadmin);
        login(ttadmin, "test09_ListBannedUsers", ADMIN_USERNAME, ADMIN_PASSWORD);

        User user = new User();
        assertTrue("get self", ttadmin.getUser(ttadmin.getMyUserID(), user));
        String IPADDR = "10.2.3.4";
        assertTrue("wait ban", waitCmdSuccess(ttadmin, ttadmin.doBanUser(ttadmin.getMyUserID(), 0), DEF_WAIT));
        assertTrue("wait ip ban", waitCmdSuccess(ttadmin, ttadmin.doBanIPAddress(IPADDR, 0), DEF_WAIT));

        TTMessage msg = new TTMessage();

        assertTrue("list bans", ttadmin.doListBans(0, 0, 100)>0);
        assertTrue("wait ban list", waitForEvent(ttadmin, ClientEvent.CLIENTEVENT_CMD_BANNEDUSER, DEF_WAIT, msg));
        BannedUser ban = msg.banneduser;
        assertTrue(ban.szIPAddress.length()>0);

        assertTrue("unban user IP", waitCmdSuccess(ttadmin, ttadmin.doUnBanUser(user.szIPAddress, 0), DEF_WAIT));
        assertTrue("unban specified IP", waitCmdSuccess(ttadmin, ttadmin.doUnBanUser(IPADDR, 0), DEF_WAIT));

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_MULTI_LOGIN;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        // ban user (ttadmin) by username from channel

        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        Channel chan = buildDefaultChannel(ttadmin, "BanTest");
        assertTrue("join new channel", waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        assertTrue("admin join (chan/username)", waitCmdSuccess(ttadmin, ttadmin.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT));

        assertTrue("ban admin by chan/username", waitCmdSuccess(ttclient, ttclient.doBanUserEx(ttadmin.getMyUserID(), BanType.BANTYPE_CHANNEL | BanType.BANTYPE_USERNAME), DEF_WAIT));

        assertTrue("kick admin", waitCmdSuccess(ttclient, ttclient.doKickUser(ttadmin.getMyUserID(), ttclient.getMyChannelID()), DEF_WAIT));

        assertTrue("list bans", ttclient.doListBans(ttclient.getMyChannelID(), 0, 100)>0);
        assertTrue("wait ban list", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_BANNEDUSER, DEF_WAIT, msg));
        ban = msg.banneduser;
        assertTrue("ip set", ban.szIPAddress.length()>0);
        assertEquals("Ban type same", BanType.BANTYPE_CHANNEL | BanType.BANTYPE_USERNAME, ban.uBanTypes);

        assertTrue("admin cannot join", waitCmdError(ttadmin, ttadmin.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT));

        joinRoot(ttadmin);

        // clear ban and join again

        TeamTalkBase ttclient2 = newClientInstance();
        connect(ttclient2);
        login(ttclient2, NICKNAME, USERNAME, PASSWORD);

        assertTrue("ttclient2 join", waitCmdSuccess(ttclient2, ttclient2.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT));

        assertTrue("admin leave", waitCmdSuccess(ttadmin, ttadmin.doLeaveChannel(), DEF_WAIT));
        assertTrue("admin join denied", waitCmdError(ttadmin, ttadmin.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT));

        assertTrue("unban", waitCmdSuccess(ttclient, ttclient.doUnBanUserEx(ban), DEF_WAIT));

        assertTrue("admin join (IP-ban)", waitCmdSuccess(ttadmin, ttadmin.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT));

        assertTrue("ban admin", waitCmdSuccess(ttclient, ttclient.doBan(ban), DEF_WAIT));

        assertTrue("admin leave", waitCmdSuccess(ttadmin, ttadmin.doLeaveChannel(), DEF_WAIT));
        assertTrue("admin join denied (IP-ban)", waitCmdError(ttadmin, ttadmin.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT));

        assertTrue("unban", waitCmdSuccess(ttclient, ttclient.doUnBanUserEx(ban), DEF_WAIT));

        assertTrue("admin join (chan/IP-address)", waitCmdSuccess(ttadmin, ttadmin.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT));

        assertTrue("ban admin by chan/IP-address", waitCmdSuccess(ttclient, ttclient.doBanUserEx(ttadmin.getMyUserID(), BanType.BANTYPE_CHANNEL | BanType.BANTYPE_IPADDR), DEF_WAIT));

        assertTrue("kick admin", waitCmdSuccess(ttclient, ttclient.doKickUser(ttadmin.getMyUserID(), ttclient.getMyChannelID()), DEF_WAIT));

        assertTrue("admin join denied", waitCmdError(ttadmin, ttadmin.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT));

        // ensure no duplicate bans
        chan = buildDefaultChannel(ttclient, "BanTest1");
        assertTrue("join new channel", waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        assertTrue("ttclient2 join BanTest1", waitCmdSuccess(ttclient2, ttclient2.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT));

        assertTrue("ban client2 once", waitCmdSuccess(ttclient, ttclient.doBanUser(ttclient2.getMyUserID(), ttclient.getMyChannelID()), DEF_WAIT));
        assertTrue("ban client2 twice", waitCmdSuccess(ttclient, ttclient.doBanUser(ttclient2.getMyUserID(), ttclient.getMyChannelID()), DEF_WAIT));

        assertTrue("list bans (expect 1)", ttclient.doListBans(ttclient.getMyChannelID(), 0, 100)>0);

        assertTrue("wait ban list", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_BANNEDUSER, DEF_WAIT, msg));
        msg = new TTMessage();
        assertTrue("wait done msg", ttclient.getMessage(msg, DEF_WAIT));
        assertEquals("done msg, only one ban expected", msg.nClientEvent, ClientEvent.CLIENTEVENT_CMD_SUCCESS);

    }

    public void testChannelSwitch() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_TRANSMIT_VOICE;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();

        connect(ttclient);
        initSound(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT));
        assertTrue("enable tx", ttclient.enableVoiceTransmission(true));
        assertTrue("enable audioblock", ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true));

        int[] opus_samplerates = {8000, 12000, 16000, 24000, 48000};

        for(int sr : opus_samplerates) {

            Channel chan;
            chan = buildDefaultChannel(ttclient, "Opus_" + sr);
            assertEquals("OPUS enabled", chan.audiocodec.nCodec, Codec.OPUS_CODEC);
            chan.audiocodec.opus.nSampleRate = sr;
            assertTrue("join channel", waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));


            for(int j=0;j<1000 / chan.audiocodec.opus.nTxIntervalMSec;j++) {
                assertTrue("get audioblock " + j, waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT));
                AudioBlock audblk = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID());
                assertTrue("Stream ID is set", audblk.nStreamID>0);
                assertEquals("Sample rate as channel", sr, audblk.nSampleRate);
            }

            assertTrue("leave channel", waitCmdSuccess(ttclient, ttclient.doLeaveChannel(), DEF_WAIT));

            // drain messages after channel switch
            while (ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID()) != null);
            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 0);
        }
    }

    public void bug_test18_ViewAllUsers() throws InterruptedException {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - "
            + getTestMethodName();
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

    public void testMessageQueue() throws InterruptedException {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - "
            + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();

        TTMessage msg = new TTMessage();

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        Channel chan = buildDefaultChannel(ttclient, "hest");

        assertTrue("join chan success", waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        for(int i=0;i<10000;i++)
            ttclient.pumpMessage(ClientEvent.CLIENTEVENT_USER_STATECHANGE, ttclient.getMyUserID());

        assertTrue("Internal error", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_INTERNAL_ERROR, DEF_WAIT, msg));
        assertTrue("Queue overflow", msg.clienterrormsg.nErrorNo == ClientError.INTERR_TTMESSAGE_QUEUE_OVERFLOW);

        int cmdid = ttclient.doLeaveChannel();
        assertTrue("Update again after overflow", cmdid>0);

        assertTrue("Expect user-left message", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_USER_LEFT, DEF_WAIT, msg));
        assertEquals("message queue lives again", ttclient.getMyUserID(), msg.user.nUserID);

    }

    public void testWaveFile() throws IOException {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
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

        int WRITE_BYTES = 512000, CHANNELS = 2, SAMPLERATE = 16000;

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

        try(FileOutputStream fs = newWaveFile(STORAGEFOLDER + File.separator + "MyWaveFile.wav", SAMPLERATE, CHANNELS, WRITE_BYTES);) {

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
        }
    }

    public void testRecordPlayback() throws IOException {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
            UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO |
            UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();

        TTMessage msg = new TTMessage();

        connect(ttclient);
        IntPtr indev = new IntPtr(), outdev = new IntPtr();
        if(INPUTDEVICEID>=0)
            indev.value = INPUTDEVICEID;
        else
            assertTrue("get default sound devices", ttclient.getDefaultSoundDevices(indev, outdev));

        if(OUTPUTDEVICEID>=0)
            outdev.value = OUTPUTDEVICEID;

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

        String wavefilePath = STORAGEFOLDER + File.separator + "MyWaveFile.wav";

        try(FileOutputStream fs = new FileOutputStream(wavefilePath);) {

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
        }

        assertTrue("disable callback for MYSELF", ttclient.enableAudioBlockEvent(0, StreamType.STREAMTYPE_VOICE, false));

        assertTrue("disable voice now that we have the wav-file", ttclient.enableVoiceTransmission(false));

        assertTrue("init output dev, so we can hear recorded wavfile", ttclient.initSoundOutputDevice(outdev.value));

        assertTrue("Stream MyWaveFile.wav",  ttclient.startStreamingMediaFileToChannel(wavefilePath, new VideoCodec()));

        assertTrue("get initial streaming event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("Stream started", msg.mediafileinfo.nStatus, MediaFileStatus.MFS_STARTED);

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg)) {
            if (msg.mediafileinfo.nStatus == MediaFileStatus.MFS_FINISHED)
                break;
        }

        assertEquals("Stream ended", msg.mediafileinfo.nStatus, MediaFileStatus.MFS_FINISHED);
    }

    public void testSoundDuplex() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        if (INPUTDEVICEID != -1 && (INPUTDEVICEID & SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG) == SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG ||
            OUTPUTDEVICEID != -1 && (OUTPUTDEVICEID & SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG) == SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG) {
            System.err.println("Duplex tests skipped due to shared sound device as input/output");
            return;
        }

        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient, true);

        // setup echo cancellation
        SpeexDSP spxdsp = new SpeexDSP();
        spxdsp.bEnableAGC = true;
        spxdsp.bEnableDenoise = true;
        spxdsp.bEnableEchoCancellation = true;
        assertTrue("SpeexDSP", ttclient.setSoundInputPreprocess(spxdsp));

        TTMessage msg = new TTMessage();

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        Channel chan = new Channel();
        assertTrue("get root channel", ttclient.getChannel(ttclient.getRootChannelID(), chan));
        assertEquals("opus set", Codec.OPUS_CODEC, chan.audiocodec.nCodec);

        if (!supportsDuplexMode(ttclient, INPUTDEVICEID, OUTPUTDEVICEID, chan.audiocodec.opus.nSampleRate)) {
            System.err.println("Duplex tests skipped due to no shared sample rate");
            return;
        }

        int cmdid = ttclient.doJoinChannelByID(ttclient.getRootChannelID(), "");
        assertTrue("issued cmd", cmdid>0);
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_PROCESSING, DEF_WAIT);

        do {
            assertTrue("get event", ttclient.getMessage(msg, DEF_WAIT));
            switch (msg.nClientEvent) {
            case ClientEvent.CLIENTEVENT_INTERNAL_ERROR :
                assertTrue("Sound input failure", msg.clienterrormsg.nErrorNo != ClientError.INTERR_SNDINPUT_FAILURE);
                assertTrue("Sound output failure", msg.clienterrormsg.nErrorNo != ClientError.INTERR_SNDOUTPUT_FAILURE);
                assertTrue("Preprocessor failure", msg.clienterrormsg.nErrorNo != ClientError.INTERR_AUDIOPREPROCESSOR_INIT_FAILED);
                break;
            }
        } while (msg.nClientEvent != ClientEvent.CLIENTEVENT_CMD_PROCESSING);
    }

    public void testSoundInputAudioPreprocessor() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_TRANSMIT_VOICE;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);
        assertTrue("enable tx", ttclient.enableVoiceTransmission(true));

        AudioPreprocessor preprocess = new AudioPreprocessor();
        assertTrue("get default preprocessor", ttclient.getSoundInputPreprocess(preprocess));
        assertEquals("TTAudioPreprocessor is default", AudioPreprocessorType.TEAMTALK_AUDIOPREPROCESSOR, preprocess.nPreprocessor);

        assertEquals("gain compatible", preprocess.ttpreprocessor.nGainLevel, ttclient.getSoundInputGainLevel());
        assertEquals("gain default set", SoundLevel.SOUND_GAIN_DEFAULT, preprocess.ttpreprocessor.nGainLevel);

        assertTrue("Set gain level", ttclient.setSoundInputGainLevel(SoundLevel.SOUND_GAIN_MAX));
        assertTrue("get updated AP preprocessor", ttclient.getSoundInputPreprocess(preprocess));
        assertEquals("gain levelfrom AP is max", SoundLevel.SOUND_GAIN_MAX, preprocess.ttpreprocessor.nGainLevel);
        assertEquals("gain level is max", SoundLevel.SOUND_GAIN_MAX, ttclient.getSoundInputGainLevel());

        // test SpeexDSP
        preprocess = new AudioPreprocessor();
        preprocess.nPreprocessor = AudioPreprocessorType.SPEEXDSP_AUDIOPREPROCESSOR;
        preprocess.speexdsp.nGainLevel = 7777;
        assertTrue("Enable SpeexDSP", ttclient.setSoundInputPreprocess(preprocess));
        SpeexDSP speexdsp = new SpeexDSP();
        assertTrue("Get SpeexDSP", ttclient.getSoundInputPreprocess(speexdsp));
        assertEquals("SpeexDSP and AudioPreprocessor are equals", 7777, speexdsp.nGainLevel);
        assertTrue("get updated AP with SpeexDSP preprocessor", ttclient.getSoundInputPreprocess(preprocess));
        assertEquals("SpeexDSP and AudioPreprocessor are equals", 7777, preprocess.speexdsp.nGainLevel);
        assertEquals("gain level not changed by SpeexDSP", SoundLevel.SOUND_GAIN_MAX, ttclient.getSoundInputGainLevel());

        Channel chan = buildDefaultChannel(ttclient, "Opus - Test", Codec.OPUS_CODEC);
        chan.audiocodec.opus.nChannels = 2;
        chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
        chan.audiocodec.opus.bDTX = false;
        assertTrue("join", waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        assertTrue("get updated AP with SpeexDSP preprocessor after join", ttclient.getSoundInputPreprocess(preprocess));
        assertEquals("SpeexDSP and AudioPreprocessor are still equal", 7777, preprocess.speexdsp.nGainLevel);

        SpeexDSP spxdsp = new SpeexDSP(true), spxdsp2 = new SpeexDSP();
        assertTrue("set Speex DSP", ttclient.setSoundInputPreprocess(spxdsp));

        assertTrue("get Speex DSP", ttclient.getSoundInputPreprocess(spxdsp2));
        assertEquals("agc1", spxdsp.bEnableAGC, spxdsp2.bEnableAGC);
        assertEquals("agc2", spxdsp.nGainLevel, spxdsp2.nGainLevel);
        assertEquals("agc3", spxdsp.nMaxIncDBSec, spxdsp2.nMaxIncDBSec);
        assertEquals("agc4", spxdsp.nMaxDecDBSec, spxdsp2.nMaxDecDBSec);
        assertEquals("agc5", spxdsp.nMaxGainDB, spxdsp2.nMaxGainDB);
        assertEquals("agc6", spxdsp.bEnableDenoise, spxdsp2.bEnableDenoise);
        assertEquals("agc7", spxdsp.nMaxNoiseSuppressDB, spxdsp2.nMaxNoiseSuppressDB);
        assertEquals("agc8", spxdsp.nEchoSuppress, spxdsp2.nEchoSuppress);
        assertEquals("agc9", spxdsp.nEchoSuppressActive, spxdsp2.nEchoSuppressActive);
    }

    public void testStoreUserVoiceInFileFormats() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_MULTI_LOGIN |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TTMessage msg = new TTMessage();

        int freq = 300;
        TeamTalkBase ttclient;

        ttclient = newClientInstance();
        initSound(ttclient);
        assertTrue("input preprocess default", ttclient.setSoundInputPreprocess(new SpeexDSP()));

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        Channel chan = new Channel();
        ttclient.getChannel(ttclient.getMyChannelID(), chan);

        assertEquals("OPUS codec running", Codec.OPUS_CODEC, chan.audiocodec.nCodec);

        ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq);

        assertTrue("wait cmd ok", waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(),
                                                                                Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        assertTrue("specify audio storage", ttclient.setUserMediaStorageDir(ttclient.getMyUserID(),
                                                                            STORAGEFOLDER, "%username%_%counter%",
                                                                            AudioFileFormat.AFF_CHANNELCODEC_FORMAT));

        assertTrue("enable voice tx", ttclient.enableVoiceTransmission(true));

        assertTrue("audio file created", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("recording started", MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus);
        assertEquals("correct filename", STORAGEFOLDER + File.separator + USERNAME + "_" + "000000001.ogg", msg.mediafileinfo.szFileName);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient.enableVoiceTransmission(false);

        assertTrue("audio file stopped", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg));
        assertEquals("recording started", MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus);

        assertTrue("enable voice tx 2 ", ttclient.enableVoiceTransmission(true));

        assertTrue("audio file created 2", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("recording started 2", MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus);
        assertEquals("correct filename 2", STORAGEFOLDER + File.separator + USERNAME + "_" + "000000002.ogg", msg.mediafileinfo.szFileName);

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
        assertEquals("correct filename 3", STORAGEFOLDER + File.separator + USERNAME + "_" + "000000003.ogg", msg.mediafileinfo.szFileName);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient.enableVoiceTransmission(false);

        assertTrue("audio file stopped 3", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg));
        assertEquals("recording started 3", MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus);


        assertTrue("enable voice tx 4", ttclient.enableVoiceTransmission(true));

        assertTrue("audio file created 4", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("recording started 4", MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus);
        assertEquals("correct filename 4", STORAGEFOLDER + File.separator + USERNAME + "_" + "000000004.ogg", msg.mediafileinfo.szFileName);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient.enableVoiceTransmission(false);

        assertTrue("audio file stopped 4", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg));
        assertEquals("recording started 4", MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus);

    }

    public void testSoundLoopback() {
        TeamTalkBase ttclient;

        ttclient = newClientInstance();
        IntPtr in = new IntPtr(), out = new IntPtr();
        if(INPUTDEVICEID<0 || OUTPUTDEVICEID<0)
            assertTrue("Get default sound devices", TeamTalkBase.getDefaultSoundDevices(in, out));
        else {
            in.value = INPUTDEVICEID;
            out.value = OUTPUTDEVICEID;
        }

        SoundDevice nodev = null, outdev = null;
        Vector<SoundDevice> devs = new Vector<SoundDevice>();
        ttclient.getSoundDevices(devs);
        for(SoundDevice d : devs) {
            if(d.nSoundSystem == SoundSystem.SOUNDSYSTEM_NONE) {
                nodev = d;
                assertEquals("Virtual TeamTalk device", SoundDeviceConstants.TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL, d.nDeviceID);
            }
            if (d.nDeviceID == out.value)
                outdev = d;
        }

        long loop = ttclient.startSoundLoopbackTest(in.value, out.value, 48000, 1, false, null);
        assertTrue("Sound loopback started", loop>0);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue("Loop stopped", ttclient.closeSoundLoopbackTest(loop));

        loop = ttclient.startSoundLoopbackTest(nodev.nDeviceID, out.value, 48000, 1, false, null);
        assertTrue("Sound loopback virtual input-dev started", loop>0);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue("Loop virtual input-dev stopped", ttclient.closeSoundLoopbackTest(loop));

        loop = ttclient.startSoundLoopbackTest(in.value, nodev.nDeviceID, 48000, 2, false, null);
        assertTrue("Sound loopback virtual output-dev started", loop>0);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue("Loop virtual output-dev stopped", ttclient.closeSoundLoopbackTest(loop));

        loop = ttclient.startSoundLoopbackTest(nodev.nDeviceID, nodev.nDeviceID, 48000, 2, true, new SpeexDSP(true));
        assertTrue("Sound loopback virtual duplex-dev started with SpeexDSP", loop>0);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue("Loop virtual duplex-dev stopped", ttclient.closeSoundLoopbackTest(loop));

        AudioPreprocessor preprocessor = new AudioPreprocessor();
        preprocessor.nPreprocessor = AudioPreprocessorType.TEAMTALK_AUDIOPREPROCESSOR;
        preprocessor.ttpreprocessor.nGainLevel = 4000;
        preprocessor.ttpreprocessor.bMuteLeftSpeaker = true;

        SoundDeviceEffects effects = new SoundDeviceEffects();

        loop = ttclient.startSoundLoopbackTest(in.value, out.value, 48000, 2, false, preprocessor, effects);
        assertTrue("Sound loopback effects started", loop>0);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue("Loop effects stopped", ttclient.closeSoundLoopbackTest(loop));

        if ((in.value & SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG) == SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG ||
            (out.value & SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG) == SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG) {
            System.err.println("Duplex tests skipped due to shared sound device as input/output");
            return;
        }

        final int SAMPLERATE = outdev.nDefaultSampleRate;
        if (!supportsDuplexMode(ttclient, in.value, out.value, SAMPLERATE)) {
            System.err.println("Duplex tests skipped due to no shared sample rate");
            return;
        }

        loop = ttclient.startSoundLoopbackTest(in.value, out.value, SAMPLERATE, 2, true, preprocessor, null);
        assertTrue("Sound loopback AudioPreprocessor started", loop>0);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue("Loop Audio Preprocessor stopped", ttclient.closeSoundLoopbackTest(loop));

        loop = ttclient.startSoundLoopbackTest(in.value, out.value, SAMPLERATE, 1, true, new SpeexDSP(true));
        assertTrue("Sound duplex loopback started", loop>0);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue("Loop duplex stopped", ttclient.closeSoundLoopbackTest(loop));

        if (out.value == SoundDeviceConstants.TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL) {
            System.err.println("Duplex test skipped due to virtual sound device as output");
        }
        else {
            loop = ttclient.startSoundLoopbackTest(nodev.nDeviceID, out.value, SAMPLERATE, 1, true, new SpeexDSP(true));
            assertTrue("Sound loopback virtual duplex-dev cannot be mixed with real dev", loop<=0);
        }

        if (in.value == SoundDeviceConstants.TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL) {
            System.err.println("Duplex test skipped due to virtual sound device as input");
        }
        else {
            loop = ttclient.startSoundLoopbackTest(in.value, nodev.nDeviceID, SAMPLERATE, 1, true, new SpeexDSP(true));
            assertTrue("Sound loopback virtual duplex-dev cannot be mixed with real dev", loop<=0);
        }
    }

    public void testSharedSoundDevice() {

        TeamTalkBase ttclient1 = newClientInstance();
        TeamTalkBase ttclient2 = newClientInstance();
        TeamTalkBase ttclient3 = newClientInstance();

        int inputdeviceid = INPUTDEVICEID;
        int outputdeviceid = OUTPUTDEVICEID;

        IntPtr indev = new IntPtr(), outdev = new IntPtr();
        boolean gotdevs = ttclient1.getDefaultSoundDevices(indev, outdev);
        // cannot assert since test system might not have a sound input or output device.
        //assertTrue("get default devs", ttclient1.getDefaultSoundDevices(indev, outdev));

        if (inputdeviceid == -1) {
            if (gotdevs)
                inputdeviceid = indev.value;
            else {
                System.err.println("Skipped shared audio test due to missing audio input device");
                return;
            }
        }
        if (outputdeviceid == -1) {
            if (gotdevs)
                outputdeviceid = outdev.value;
            else {
                System.err.println("Skipped shared audio test due to missing audio output device");
                return;
            }
        }

        // if INPUTDEVICEID or OUTPUTDEVICEID is already a shared device then clear it
        inputdeviceid = (inputdeviceid & SoundDeviceConstants.TT_SOUNDDEVICE_ID_MASK);
        outputdeviceid = (outputdeviceid & SoundDeviceConstants.TT_SOUNDDEVICE_ID_MASK);

        SoundDevice sharedindev = null, sharedoutdev = null;
        Vector<SoundDevice> devs = new Vector<>();
        TeamTalkBase.getSoundDevices(devs);
        for(SoundDevice d : devs) {
            if (d.nDeviceID == inputdeviceid)
                sharedindev = d;
            if (d.nDeviceID == outputdeviceid)
                sharedoutdev = d;
        }

        assertTrue("shared in device exists", sharedindev != null);
        assertEquals("shared in device selected", inputdeviceid, sharedindev.nDeviceID);

        assertTrue("shared out device exists", sharedoutdev != null);
        assertEquals("shared out device selected", outputdeviceid, sharedoutdev.nDeviceID);

        // toggle input and output device as shared audio devices
        inputdeviceid = sharedindev.nDeviceID | SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;
        outputdeviceid = sharedoutdev.nDeviceID | SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;

        // test two instances with same sample settings as original and one instance which requires resampling
        long sndloop1 = ttclient1.startSoundLoopbackTest(inputdeviceid, outputdeviceid, 48000, 2, false, null);
        assertTrue("Start sound loop 1", sndloop1 != 0);
        long sndloop2 = ttclient2.startSoundLoopbackTest(inputdeviceid, outputdeviceid, 48000, 2, false, null);
        assertTrue("Start sound loop 2", sndloop2 != 0);
        long sndloop3 = ttclient3.startSoundLoopbackTest(inputdeviceid, outputdeviceid, 48000, 1, false, null);
        assertTrue("Start sound loop 3", sndloop3 != 0);

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue("Close sndloop1", ttclient1.closeSoundLoopbackTest(sndloop1));
        assertTrue("Close sndloop2", ttclient2.closeSoundLoopbackTest(sndloop2));
        assertTrue("Close sndloop3", ttclient3.closeSoundLoopbackTest(sndloop3));

        // test two instances which require resampling
        long sndloop4 = ttclient1.startSoundLoopbackTest(inputdeviceid, outputdeviceid, 32000, 1, false, null);
        assertTrue("Start sound loop 4", sndloop4 != 0);
        long sndloop5 = ttclient2.startSoundLoopbackTest(inputdeviceid, outputdeviceid, 44100, 2, false, null);
        assertTrue("Start sound loop 5", sndloop5 != 0);

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue("Close sndloop4", ttclient1.closeSoundLoopbackTest(sndloop4));
        assertTrue("Close sndloop5", ttclient2.closeSoundLoopbackTest(sndloop5));

        Vector<Long> sndloops = new Vector<>();
        // now go through all sample rates
        for(int samplerate : sharedindev.inputSampleRates) {
            if (samplerate <= 0)
                continue;

            long sndloop = ttclient1.startSoundLoopbackTest(inputdeviceid, outputdeviceid, samplerate, 1, false, null);
            assertTrue("Start sound loop at " + samplerate + " channels " + 1, sndloop != 0);
            sndloops.add(sndloop);
        }

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);

        for(long sndloop : sndloops) {
            assertTrue("Close sndloop", ttclient1.closeSoundLoopbackTest(sndloop));
        }
    }


    public void testVirtualSoundDevice() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
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

    public void testSoloTransmitChannel() {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
            UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO |
            UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_MULTI_LOGIN;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        Vector<TeamTalkBase> clients = new Vector<TeamTalkBase>();

        TTMessage msg = new TTMessage();

        TeamTalkBase ttclient1 = newClientInstance();
        clients.add(ttclient1);

        connect(ttclient1);
        initSound(ttclient1);
        login(ttclient1, NICKNAME, USERNAME, PASSWORD);

        Channel chan = buildDefaultChannel(ttclient1, "Opus");
        assertEquals("opus default", chan.audiocodec.nCodec, Codec.OPUS_CODEC);
        chan.uChannelType |= ChannelType.CHANNEL_SOLO_TRANSMIT;

        assertTrue("join", waitCmdSuccess(ttclient1, ttclient1.doJoinChannel(chan), DEF_WAIT));

        assertTrue("Channel id set", ttclient1.getChannel(ttclient1.getMyChannelID(), chan));

        for(int u : chan.transmitUsersQueue)
            assertEquals("no users in queue", 0, u);

        assertTrue("subscribe", waitCmdSuccess(ttclient1, ttclient1.doSubscribe(ttclient1.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        for(int i=0;i<2;i++) {

            TeamTalkBase ttclient = newClientInstance();
            clients.add(ttclient);

            connect(ttclient);
            initSound(ttclient);
            login(ttclient, NICKNAME, USERNAME, PASSWORD);

            assertTrue("join existing " , waitCmdSuccess(ttclient, ttclient.doJoinChannelByID(chan.nChannelID, chan.szPassword), DEF_WAIT));

            assertTrue("subscribe", waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

            assertTrue("Enable voice transmission", ttclient.enableVoiceTransmission(true));

            boolean chanUpEvent = false, userUpEvent = false;
            while (ttclient.getMessage(msg, DEF_WAIT) && (!chanUpEvent || !userUpEvent)) {
                switch (msg.nClientEvent) {
                case ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE :
                    assertTrue("Channel tx queue set", ttclient.getChannel(ttclient.getMyChannelID(), chan));
                    assertEquals("myself in queue", ttclient.getMyUserID(), chan.transmitUsersQueue[0]);
                    chanUpEvent = true;
                    break;
                case ClientEvent.CLIENTEVENT_USER_STATECHANGE :
                    assertEquals("User state to voice", UserState.USERSTATE_VOICE, msg.user.uUserState & UserState.USERSTATE_VOICE);
                    assertEquals("myself talking", ttclient.getMyUserID(), msg.user.nUserID);
                    userUpEvent = true;
                    break;
                }
            }

            //assertTrue("wait chan update " + i, waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg));
            assertTrue("wait chan update " + i, chanUpEvent);
            //assertTrue("Wait for talking event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));
            assertTrue("Wait for talking event", userUpEvent);

            assertTrue("Disable voice transmission", ttclient.enableVoiceTransmission(false));

            assertTrue("Wait for talking event stopped", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));
            assertEquals("User state to no voice", UserState.USERSTATE_NONE, msg.user.uUserState & UserState.USERSTATE_VOICE);
            assertEquals("myself stopped talking", ttclient.getMyUserID(), msg.user.nUserID);

            assertTrue("ttclient1, wait for tx queue start", waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT));
            assertTrue("ttclient1, wait for tx queue stop", waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT));
        }

        // wait for "reset" state
        assertTrue("ttclient1, drain client 1", waitCmdComplete(ttclient1, ttclient1.doPing(), DEF_WAIT));

        assertTrue("ttclient1, Enable voice transmission", ttclient1.enableVoiceTransmission(true));

        assertTrue("ttclient1, wait chan txq update", waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg));

        assertTrue("ttclient1, Channel tx queue set", ttclient1.getChannel(ttclient1.getMyChannelID(), chan));

        assertEquals("ttclient1, myself is head in queue", ttclient1.getMyUserID(), chan.transmitUsersQueue[0]);

        // don't know if 'ClientEvent.CLIENTEVENT_USER_STATECHANGE' or
        // 'ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE' came first, so
        // don't assertTrue()
        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_STATECHANGE, 1000, msg);
        User user = new User();
        assertTrue("get ttclient1 state", ttclient1.getUser(ttclient1.getMyUserID(), user));
        assertEquals("ttclient1, User state to voice", UserState.USERSTATE_VOICE, user.uUserState & UserState.USERSTATE_VOICE);
        assertEquals("ttclient1, myself talking", ttclient1.getMyUserID(), msg.user.nUserID);

        // ensure ttclient2 doesn't take over transmit queue from ttclient1
        TeamTalkBase ttclient2 = clients.get(1);

        assertTrue("ttclient2, drain client 2", waitCmdComplete(ttclient2, ttclient2.doPing(), DEF_WAIT));

        assertTrue("ttclient2, Enable voice transmission", ttclient2.enableVoiceTransmission(true));

        assertTrue("ttclient2, wait chan txq update as no 2", waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg));

        assertTrue("ttclient2, Channel tx queue set", ttclient2.getChannel(ttclient2.getMyChannelID(), chan));

        assertEquals("ttclient2, myself in queue", ttclient2.getMyUserID(), chan.transmitUsersQueue[1]);

        waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 1000);
        assertTrue("ttclient2 is not talking", ttclient2.getUser(ttclient2.getMyUserID(), user) && (user.uUserState & UserState.USERSTATE_VOICE) == 0);


        // ensure ttclient2 takes over transmit queue when ttclient1 stops transmitting
        assertTrue("ttclient1, drain client 1", waitCmdComplete(ttclient1, ttclient1.doPing(), DEF_WAIT));
        assertTrue("ttclient2, drain client 2", waitCmdComplete(ttclient2, ttclient2.doPing(), DEF_WAIT));

        assertTrue("ttclient1, Disable voice transmission", ttclient1.enableVoiceTransmission(false));

        assertTrue("ttclient1, wait chan txq update", waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg));

        assertTrue("ttclient2, wait chan txq update as no 1", waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg));
        assertTrue("ttclient2, retrieve channel", ttclient2.getChannel(ttclient2.getMyChannelID(), chan));

        assertEquals("ttclient2 head in queue", ttclient2.getMyUserID(), chan.transmitUsersQueue[0]);

        //ensure transmit queue becomes empty when ttclient2 stops transmitting
        assertTrue("ttclient2, disable voice transmission", ttclient2.enableVoiceTransmission(false));

        assertTrue("ttclient2, wait chan txq update clear", waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg));
        assertTrue("ttclient2, retrieve channel", ttclient2.getChannel(ttclient2.getMyChannelID(), chan));

        assertEquals("ttclient2 empty queue", 0, chan.transmitUsersQueue[0]);


        // ensure ttclient1 can take over transmit queue again
        assertTrue("drain ttclient1", waitCmdComplete(ttclient1, ttclient1.doPing(), DEF_WAIT));

        assertTrue("ttclient1, Enable voice transmission", ttclient1.enableVoiceTransmission(true));

        assertTrue("ttclient1, wait chan txq update", waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg));

        assertTrue("ttclient1, Channel tx queue set", ttclient1.getChannel(ttclient1.getMyChannelID(), chan));

        assertEquals("ttclient1, myself is head again in queue ", ttclient1.getMyUserID(), chan.transmitUsersQueue[0]);
    }

    public void testAbusePrevention() {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;

        TTMessage msg = new TTMessage();

        TeamTalkBase ttadmin = newClientInstance();
        connect(ttadmin);
        login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        UserAccount account = new UserAccount();
        account.szUsername = USERNAME;
        account.szPassword = PASSWORD;
        account.uUserType = UserType.USERTYPE_DEFAULT;
        account.uUserRights = USERRIGHTS;
        account.abusePrevent.nCommandsLimit = 1;
        account.abusePrevent.nCommandsIntervalMSec = 1000;

        assertTrue("create flood prevent account", waitCmdSuccess(ttadmin, ttadmin.doNewUserAccount(account), DEF_WAIT));

        TeamTalkBase ttclient = newClientInstance();

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 2000);

        TextMessage txtmsg = new TextMessage();
        txtmsg.nMsgType = TextMsgType.MSGTYPE_USER;
        txtmsg.nToUserID = ttclient.getMyUserID();
        txtmsg.szMessage = "My text message that should go through";

        assertTrue("do text message", waitCmdSuccess(ttclient, ttclient.doTextMessage(txtmsg), DEF_WAIT));

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 200);

        txtmsg.szMessage = "My text message that should be blocked";

        assertTrue("do text message in less than cmd-timeout", waitCmdError(ttclient, ttclient.doTextMessage(txtmsg), DEF_WAIT));

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 2000);

        assertTrue("do text message after cmd-timeout", waitCmdSuccess(ttclient, ttclient.doTextMessage(txtmsg), DEF_WAIT));
    }

    public void testLoginDelay() throws InterruptedException {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_MULTI_LOGIN;

        TTMessage msg = new TTMessage();

        TeamTalkBase ttadmin = newClientInstance();
        connect(ttadmin);
        login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        ServerProperties srvprop = new ServerProperties();
        assertTrue(ttadmin.getServerProperties(srvprop));
        int orgValue = srvprop.nLoginDelayMSec;
        srvprop.nLoginDelayMSec = 1000;
        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doUpdateServer(srvprop), DEF_WAIT));

        UserAccount account = new UserAccount();
        account.szUsername = USERNAME;
        account.szPassword = PASSWORD;
        account.uUserType = UserType.USERTYPE_DEFAULT;
        account.uUserRights = USERRIGHTS;

        assertTrue("create account", waitCmdSuccess(ttadmin, ttadmin.doNewUserAccount(account), DEF_WAIT));

        TeamTalkBase ttclient1 = newClientInstance();
        TeamTalkBase ttclient2 = newClientInstance();

        connect(ttclient1);
        login(ttclient1, NICKNAME, USERNAME, PASSWORD);
        connect(ttclient2);

        assertTrue("login failure", waitCmdError(ttclient2, ttclient2.doLogin(NICKNAME, USERNAME, PASSWORD), DEF_WAIT));

        Thread.sleep(2000);
        login(ttclient2, NICKNAME, USERNAME, PASSWORD);

        srvprop.nLoginDelayMSec = orgValue;
        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doUpdateServer(srvprop), DEF_WAIT));
    }

    public void testLoginAttempts() {

        TeamTalkBase ttadmin = newClientInstance();
        connect(ttadmin);
        login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        ServerProperties srvprop = new ServerProperties();
        assertTrue("get srvprop", ttadmin.getServerProperties(srvprop));

        int oldValue = srvprop.nMaxLoginAttempts;

        srvprop.nMaxLoginAttempts = 2;

        assertTrue("update server", waitCmdSuccess(ttadmin, ttadmin.doUpdateServer(srvprop), DEF_WAIT));

        User user = new User();
        assertTrue("get user", ttadmin.getUser(ttadmin.getMyUserID(), user));

        TeamTalkBase ttclient = newClientInstance();

        connect(ttclient);

        TTMessage msg = new TTMessage();
        int cmdid = ttclient.doLogin(ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, "wrongpassword1");
        assertTrue("wait login error", waitCmdError(ttclient, cmdid, DEF_WAIT, msg));
        assertEquals("invalid account", ClientError.CMDERR_INVALID_ACCOUNT, msg.clienterrormsg.nErrorNo);

        cmdid = ttclient.doLogin(ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, "wrongpassword2");
        assertTrue("wait login error", waitCmdError(ttclient, cmdid, DEF_WAIT, msg));
        assertEquals("invalid account", ClientError.CMDERR_INVALID_ACCOUNT, msg.clienterrormsg.nErrorNo);

        cmdid = ttclient.doLogin(ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, "wrongpassword3");
        assertTrue("wait login error", waitCmdError(ttclient, cmdid, DEF_WAIT, msg));
        assertEquals("banned account", ClientError.CMDERR_SERVER_BANNED, msg.clienterrormsg.nErrorNo);

        assertTrue("unban success", waitCmdSuccess(ttadmin, ttadmin.doUnBanUser(user.szIPAddress, 0), DEF_WAIT));

        srvprop.nMaxLoginAttempts = 0;
        assertTrue("update server", waitCmdSuccess(ttadmin, ttadmin.doUpdateServer(srvprop), DEF_WAIT));
    }

    public void testDisconnect() throws IOException {

        TeamTalkBase ttadmin = newClientInstance();
        connect(ttadmin);
        login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        ServerProperties srvprop = new ServerProperties();
        assertTrue("get srvprop", ttadmin.getServerProperties(srvprop));
        int orgValue = srvprop.nUserTimeout;
        srvprop.nUserTimeout = 60;
        assertTrue("update server", waitCmdSuccess(ttadmin, ttadmin.doUpdateServer(srvprop), DEF_WAIT));

        assertTrue("Disconnect hard", ttadmin.disconnect());

        connect(ttadmin);
        login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        assertTrue("DoQuit", ttadmin.doQuit()>0);

        assertTrue("Wait con lost", waitForEvent(ttadmin, ClientEvent.CLIENTEVENT_CON_LOST, DEF_WAIT));

        assertTrue("Disconnect quit", ttadmin.disconnect());

        connect(ttadmin);
        login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        srvprop.nUserTimeout = 1;

        assertTrue("update server", waitCmdSuccess(ttadmin, ttadmin.doUpdateServer(srvprop), DEF_WAIT));

        assertTrue("Disconnect after tmo", ttadmin.disconnect());

        try (Socket s = new Socket(IPADDR, TCPPORT);
             BufferedReader stream = new BufferedReader(new InputStreamReader(s.getInputStream()));) {

            if(!ENCRYPTED)
            {
                String welcome = stream.readLine();
                assertTrue("welcome msg", welcome.startsWith(SYSTEMID));
            }

            boolean closed = false;
            try {
                closed = stream.readLine() == null;
            }
            catch(IOException e) {
                closed = true;
            }
            assertTrue("Closed socket", closed);

            assertTrue("Disconnect quit", ttadmin.disconnect());

            connect(ttadmin);
            login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);
            srvprop.nUserTimeout = orgValue;
            assertTrue("update server", waitCmdSuccess(ttadmin, ttadmin.doUpdateServer(srvprop), DEF_WAIT));
        }
    }

    public void testKeyTranslate() {
        TeamTalkBase ttadmin = newClientInstance();
        DesktopInput[] inputs = new DesktopInput[2], outputs = new DesktopInput[2];
        for (int i=0;i<inputs.length;++i) {
            inputs[i] = new DesktopInput();
            inputs[i].uMousePosY = 100;
            inputs[i].uKeyState = DesktopKeyStates.DESKTOPKEYSTATE_NONE;
            outputs[i] = new DesktopInput();
        }
        assertTrue("Key translate", WindowsHelper.desktopInputKeyTranslate(TTKeyTranslate.TTKEY_WINKEYCODE_TO_TTKEYCODE,
                                                                           inputs, outputs) >= 0);
        assertEquals("Coordinate", inputs[0].uMousePosY, outputs[0].uMousePosY);
        assertEquals("Keystate", inputs[0].uKeyState, outputs[0].uKeyState);
        assertEquals("Coordinate", inputs[1].uMousePosY, outputs[1].uMousePosY);
        assertEquals("Keystate", inputs[1].uKeyState, outputs[1].uKeyState);

        assertTrue("move mouse", PlatformHelper.desktopInputExecute(outputs) >= 0);
    }

    public void testDesktopInput() {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_TRANSMIT_DESKTOP |
            UserRight.USERRIGHT_TRANSMIT_DESKTOPINPUT | UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_MULTI_LOGIN;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient1 = newClientInstance(), ttclient2 = newClientInstance();

        connect(ttclient1);
        login(ttclient1, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient1);

        connect(ttclient2);
        login(ttclient2, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient2);

        // init a desktop session for desktop input
        DesktopWindow wnd = new DesktopWindow();
        wnd.nWidth = 128;
        wnd.nHeight = 128;
        wnd.bmpFormat = BitmapFormat.BMP_RGB32;
        wnd.nProtocol = DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1;
        wnd.frameBuffer = new byte[wnd.nWidth * wnd.nHeight * 4];

        assertTrue("send desktop #1 window", ttclient1.sendDesktopWindow(wnd, BitmapFormat.BMP_RGB32)>0);

        TTMessage msg = new TTMessage();
        assertTrue("Wait for desktop #1 window", waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, DEF_WAIT, msg));

        int desktop1ID = msg.nStreamID;
        assertTrue("Desktop #1 shown", desktop1ID > 0);

        assertTrue("subscribe desktopinput", waitCmdSuccess(ttclient1,
                                                            ttclient1.doSubscribe(ttclient2.getMyUserID(),
                                                                                  Subscription.SUBSCRIBE_DESKTOPINPUT),
                                                            DEF_WAIT));
        DesktopInput[] inputs = new DesktopInput[2];
        for (int x=0;x<wnd.nWidth;x++) {
            inputs[0] = new DesktopInput();
            inputs[0].uMousePosX = x;
            inputs[0].uMousePosY = 10;
            inputs[0].uKeyState = DesktopKeyStates.DESKTOPKEYSTATE_NONE;

            inputs[1] = new DesktopInput();
            inputs[1].uMousePosX = x;
            inputs[1].uMousePosY = 20;
            inputs[1].uKeyState = DesktopKeyStates.DESKTOPKEYSTATE_NONE;

            assertTrue("send desktop #1 input x="+x, ttclient2.sendDesktopInput(ttclient1.getMyUserID(), inputs));

            assertTrue("get desktop #1 input[0]", waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_DESKTOPINPUT, DEF_WAIT, msg));
            assertEquals("desktop #1 input[0] x", x, msg.desktopinput.uMousePosX);
            assertEquals("desktop #1 input[0] y", 10, msg.desktopinput.uMousePosY);

            assertTrue("get desktop #1 input[1]", waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_DESKTOPINPUT, DEF_WAIT, msg));
            assertEquals("desktop #1 input[1] x", x, msg.desktopinput.uMousePosX);
            assertEquals("desktop #1 input[1] y", 20, msg.desktopinput.uMousePosY);
        }

        assertTrue("send cursor pos", ttclient1.sendDesktopCursorPosition(5, 6));

        assertTrue("get desktop cursor", waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_DESKTOPCURSOR, DEF_WAIT, msg));
        assertEquals("pos x", 5, msg.desktopinput.uMousePosX);
        assertEquals("pos y", 6, msg.desktopinput.uMousePosY);

        // // start new desktop session
        // wnd = new DesktopWindow();
        // wnd.nWidth = 1024;
        // wnd.nHeight = 1024;
        // wnd.bmpFormat = BitmapFormat.BMP_RGB32;
        // wnd.nProtocol = DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1;
        // wnd.frameBuffer = new byte[wnd.nWidth * wnd.nHeight * 4];

        // assertTrue("send desktop #2 window", ttclient1.sendDesktopWindow(wnd, BitmapFormat.BMP_RGB32)>0);

        // assertTrue("Wait for desktop #2 window", waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, DEF_WAIT, msg));

        // assertTrue("Desktop #2 shown", desktop1ID != msg.nStreamID);

        // DesktopInput[] input = new DesktopInput[1];
        // int y = wnd.nHeight;
        // for (int x=0;x<wnd.nWidth;x++) {
        //     inputs[0] = new DesktopInput();
        //     inputs[0].uMousePosX = x;
        //     inputs[0].uMousePosY = --y;
        //     inputs[0].uKeyState = DesktopKeyStates.DESKTOPKEYSTATE_NONE;

        //     assertTrue("send desktop #2 input x="+x, ttclient2.sendDesktopInput(ttclient1.getMyUserID(), inputs));

        //     assertTrue("get desktop #2 input 0", waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_DESKTOPINPUT, DEF_WAIT, msg));
        //     assertEquals("desktop #2 input[0] x", x, msg.desktopinput.uMousePosX);
        //     assertEquals("desktop #2 input[0] y", y, msg.desktopinput.uMousePosY);
        // }

    }

    public void testWebLogin() {
        String USERNAME = "facebook", PASSWORD = "code=123", NICKNAME = "jUnit - " + getTestMethodName();

        TeamTalkBase ttclient1 = newClientInstance();
        TeamTalkBase ttclient2 = newClientInstance();

        connect(ttclient1);
        connect(ttclient2);

        int cmdid = ttclient1.doLoginEx(NICKNAME, USERNAME, PASSWORD, "");
        assertTrue("do login 1", cmdid > 0);
        cmdid = ttclient2.doLoginEx(NICKNAME, USERNAME, PASSWORD, "");
        assertTrue("do login 2", cmdid > 0);

        TTMessage msg = new TTMessage();
        assertTrue("wait login failure 1", waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_CMD_ERROR, DEF_WAIT, msg));
        assertTrue("wait login failure 2", waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_ERROR, DEF_WAIT, msg));

        ttclient1.disconnect();
        ttclient2.disconnect();
    }

    public void testLocalPlayback() {

        // load shared object
        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);

        // play single two second file
        MediaFileInfo mfi = new MediaFileInfo();
        mfi.szFileName = STORAGEFOLDER + File.separator + "hest.wav";
        mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 2);
        mfi.uDurationMSec = 2 * 1000;

        assertTrue("Write media file", TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600));

        MediaFilePlayback mfp = new MediaFilePlayback();

        int sessionid = ttclient.initLocalPlayback(mfi.szFileName, mfp);
        assertTrue("init playback", sessionid > 0);

        TTMessage msg = new TTMessage();
        assertTrue(DEF_WAIT > mfi.uDurationMSec);
        assertTrue("Wait for playback", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg));

        assertEquals("streaming started", MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus);

        boolean finished = false, playing = false;
        while (!finished && waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg)) {
            switch(msg.mediafileinfo.nStatus) {
            case MediaFileStatus.MFS_PLAYING :
                playing = true;
                break;
            case MediaFileStatus.MFS_FINISHED :
                finished = true;
                break;
            }
        }
        assertTrue("Playing event", playing);
        assertEquals("Streaming ended", MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus);
        assertFalse("Last playback event is finished", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, 0, msg));
    }

    public void testLocalPlaybackPause() {

        // load shared object
        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);

        // play single two second file
        MediaFileInfo mfi = new MediaFileInfo();
        mfi.szFileName = STORAGEFOLDER + File.separator + "hest.wav";
        mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 2);
        mfi.uDurationMSec = 20 * 1000;

        assertTrue("Write media file", TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600));

        MediaFilePlayback mfp = new MediaFilePlayback();

        TTMessage msg = new TTMessage();

        // play
        mfp.bPaused = true;
        int sessionid = ttclient.initLocalPlayback(mfi.szFileName, mfp);
        assertTrue("init playback", sessionid > 0);

        assertFalse("Hold pause state", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 100));

        mfp.bPaused = false;
        assertTrue("Unpause", ttclient.updateLocalPlayback(sessionid, mfp));

        assertTrue("Wait for start event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg));
        assertEquals("playback started", MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus);

        int starttime = msg.mediafileinfo.uElapsedMSec;
        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg)) {
            if (msg.mediafileinfo.uElapsedMSec - starttime >= 1000)
                break;
        }

        assertTrue("Play one sec", msg.mediafileinfo.uElapsedMSec - starttime >= 1000);

        mfp.bPaused = true;
        assertTrue("Pause again", ttclient.updateLocalPlayback(sessionid, mfp));

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg)) {
            if (msg.mediafileinfo.nStatus == MediaFileStatus.MFS_PAUSED)
                break;
        }
        assertEquals("streaming paused", MediaFileStatus.MFS_PAUSED, msg.mediafileinfo.nStatus);
    }

    public void testLocalPlaybackSeek() {

        // load shared object
        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);

        // play single two second file
        MediaFileInfo mfi = new MediaFileInfo();
        mfi.szFileName = STORAGEFOLDER + File.separator + "hest.wav";
        mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 2);
        mfi.uDurationMSec = 20 * 1000;

        assertTrue("Write media file", TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600));

        MediaFilePlayback mfp = new MediaFilePlayback();

        TTMessage msg = new TTMessage();

        // play
        mfp.uOffsetMSec = 19 * 1000;
        int sessionid = ttclient.initLocalPlayback(mfi.szFileName, mfp);
        assertTrue("init playback", sessionid > 0);

        assertTrue("Wait for start event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg));
        assertEquals("playback started", MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus);

        assertTrue(DEF_WAIT > mfi.uDurationMSec - mfp.uOffsetMSec);
        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg)) {
            if (msg.mediafileinfo.nStatus == MediaFileStatus.MFS_FINISHED)
                break;
        }
        assertEquals("streaming finished", MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus);
    }

    public void testLocalPlaybackSeekBackwards() {

        // load shared object
        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);

        // play single two second file
        MediaFileInfo mfi = new MediaFileInfo();
        mfi.szFileName = STORAGEFOLDER + File.separator + "hest.wav";
        mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 2);
        mfi.uDurationMSec = 20 * 1000;

        assertTrue("Write media file", TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600));

        MediaFilePlayback mfp = new MediaFilePlayback();

        TTMessage msg = new TTMessage();

        // play
        mfp.uOffsetMSec = 19 * 1000;
        int sessionid = ttclient.initLocalPlayback(mfi.szFileName, mfp);
        assertTrue("init playback", sessionid > 0);

        assertTrue("Wait for start event", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg));
        assertEquals("playback started", MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus);
        int elapsed = msg.mediafileinfo.uElapsedMSec;

        mfp.uOffsetMSec = 18 * 1000;
        assertTrue("Rewind", ttclient.updateLocalPlayback(sessionid, mfp));

        boolean playing = false, started = false;
        while (!started && waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg)) {

            switch (msg.mediafileinfo.nStatus) {
            case MediaFileStatus.MFS_STARTED :
                started = true;
                break;
            case MediaFileStatus.MFS_PLAYING :
                playing = true;
                break;
            }
        }

        assertTrue("Wait for playing event", playing);
        assertTrue("Wait for start event", started);
        assertEquals("playback started", MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus);

        assertTrue("Playback from rewinded position", msg.mediafileinfo.uElapsedMSec < elapsed);

        assertTrue(DEF_WAIT > mfi.uDurationMSec - msg.mediafileinfo.uElapsedMSec);

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg)) {

            if (msg.mediafileinfo.nStatus == MediaFileStatus.MFS_FINISHED)
                break;
        }

        assertEquals("streaming finished", MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus);
    }

    public void testAudioInput() throws IOException {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_MULTI_LOGIN;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TTMessage msg = new TTMessage();

        TeamTalkBase ttclient = newClientInstance();

        initSound(ttclient);
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        TeamTalkBase ttclient2 = newClientInstance();

        initSound(ttclient2);
        connect(ttclient2);
        login(ttclient2, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient2);

        assertTrue("enable muxed aud cb", ttclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, true));
        assertTrue("enable local aud cb", ttclient.enableAudioBlockEvent(Constants.TT_LOCAL_USERID, StreamType.STREAMTYPE_VOICE, true));

        FileOutputStream localWaveFile = null, muxedWaveFile = null;
        while (localWaveFile == null || muxedWaveFile == null) {
            assertTrue("gimme voice audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            if (msg.nSource == Constants.TT_LOCAL_USERID) {
                AudioBlock ab = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_LOCAL_USERID);
                if (localWaveFile == null) {
                    localWaveFile = newWaveFile(STORAGEFOLDER + File.separator + "local_audioinput.wav", ab.nSampleRate, ab.nChannels, 1024*1024 /* no idea */);
                }
                localWaveFile.write(ab.lpRawAudio);
            }

            if (msg.nSource == Constants.TT_MUXED_USERID) {
                AudioBlock ab = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_MUXED_USERID);
                if (muxedWaveFile == null) {
                    muxedWaveFile = newWaveFile(STORAGEFOLDER + File.separator + "muxed_audioinput.wav", ab.nSampleRate, ab.nChannels, 1024*1024 /* no idea */);
                }
                muxedWaveFile.write(ab.lpRawAudio);
            }
        }

        assertTrue("enable voice tx", ttclient.enableVoiceTransmission(true));

        assertTrue("get voice event", waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));
        assertTrue("initial voice event", ((msg.user.uUserState & UserState.USERSTATE_VOICE) != 0));

        int STREAMID = 57;
        final int SAMPLERATE = 16000, CHANNELS = 1;

        byte[] tone = generateToneAsByte(500, SAMPLERATE, CHANNELS, 1000);

        assertEquals("one second of audio", SAMPLERATE, tone.length / 2 / CHANNELS);

        AudioBlock ab = new AudioBlock();
        ab.nStreamID = STREAMID;
        ab.nSampleRate = SAMPLERATE;
        ab.nChannels = CHANNELS;
        ab.lpRawAudio = tone; //PCM16 mono
        ab.nSamples = tone.length / 2 / CHANNELS;
        ab.uSampleIndex = 0;

        assertFalse("Reject audio input during voicetx", ttclient.insertAudioBlock(ab));

        assertTrue("disable voice tx", ttclient.enableVoiceTransmission(false));

        assertTrue("enable voice act", ttclient.enableVoiceActivation(true));

        assertFalse("Reject audio input during voiceact", ttclient.insertAudioBlock(ab));

        assertTrue("disable voice act", ttclient.enableVoiceActivation(false));

        assertTrue("voice stopped event", waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));
        assertTrue("Voice stop event", ((msg.user.uUserState & UserState.USERSTATE_VOICE) == 0));

        assertTrue("Send audio block", ttclient.insertAudioBlock(ab));

        int frames = 0;
        assertTrue("Audio input "+STREAMID+" started", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_AUDIOINPUT, DEF_WAIT, msg));
        frames++;

        assertEquals("Stream ID match", STREAMID, msg.audioinputprogress.nStreamID);

        assertTrue("Playing audio input remotely", waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));
        assertTrue("Voice event from audio input", ((msg.user.uUserState & UserState.USERSTATE_VOICE) != 0));

        assertFalse("Reject voice tx", ttclient.enableVoiceTransmission(true));

        assertFalse("Reject voice act", ttclient.enableVoiceActivation(true));

        assertTrue("Accept voice tx disable", ttclient.enableVoiceTransmission(false));

        assertTrue("Accept voice act disable", ttclient.enableVoiceActivation(false));

        boolean gotlocal = false, gotmuxed = false;
        do {
            assertTrue("Event processing", ttclient.getMessage(msg, DEF_WAIT));
            switch (msg.nClientEvent) {
            case ClientEvent.CLIENTEVENT_AUDIOINPUT :
                if (msg.audioinputprogress.uElapsedMSec > 0)
                    frames++;
                break;
            case ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK :
                if (msg.nSource == Constants.TT_LOCAL_USERID) {
                    AudioBlock abl = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_LOCAL_USERID);
                    assertTrue("get local audio block", abl != null);
                    localWaveFile.write(abl.lpRawAudio);
                    gotlocal = true;
                }
                if (msg.nSource == Constants.TT_MUXED_USERID) {
                    AudioBlock abm = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_MUXED_USERID);
                    assertTrue("get muxed audio block", abm != null);
                    muxedWaveFile.write(abm.lpRawAudio);
                    gotmuxed = true;
                }
                break;
            }
        } while (msg.nClientEvent != ClientEvent.CLIENTEVENT_AUDIOINPUT ||
                 (msg.audioinputprogress.nStreamID == STREAMID &&
                  msg.audioinputprogress.uElapsedMSec != 0 &&
                  msg.audioinputprogress.uQueueMSec != 0));

        assertTrue("Got local user audio", gotlocal);
        assertTrue("Got muxed user audio", gotmuxed);

        // now send audio block whose nSamples doesn't match the
        // transmit interval, i.e. we need to flush to send
        // everything.
        STREAMID = 55;
        ab.nStreamID = STREAMID;

        ab.nSamples -= 43;
        assertTrue("Send audio block which needs to be flushed "+ STREAMID, ttclient.insertAudioBlock(ab));

        assertTrue("Audio input "+STREAMID+" started", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_AUDIOINPUT, DEF_WAIT, msg));
        frames--;

        assertEquals("Stream ID match", STREAMID, msg.audioinputprogress.nStreamID);

        gotlocal = gotmuxed = false;
        for (int i=0;i<frames-1;) {
            assertTrue("Event processing", ttclient.getMessage(msg, DEF_WAIT));
            switch (msg.nClientEvent) {
            case ClientEvent.CLIENTEVENT_AUDIOINPUT :
                assertTrue("stream id match", msg.audioinputprogress.nStreamID == STREAMID);
                assertTrue("elapsed increasing", msg.audioinputprogress.uElapsedMSec != 0);
                assertTrue("queue holding", msg.audioinputprogress.uQueueMSec != 0);
                i++;
                break;
            case ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK :
                if (msg.nSource == Constants.TT_LOCAL_USERID) {
                    gotlocal = true;
                }
                if (msg.nSource == Constants.TT_MUXED_USERID) {
                    gotmuxed = true;
                }
                break;
            }
        }
        assertTrue("Got local user audio, before flushing", gotlocal);
        assertTrue("Got muxed user audio, before flushing", gotmuxed);

        assertFalse("Last frame will not appear until we flush", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_AUDIOINPUT, 100, msg));

        assertTrue("Flush queued audio "+ STREAMID, ttclient.insertAudioBlock(new AudioBlock()));

        assertTrue("Audio input last frame "+STREAMID, waitForEvent(ttclient, ClientEvent.CLIENTEVENT_AUDIOINPUT, DEF_WAIT, msg));
        assertTrue("stream id match", msg.audioinputprogress.nStreamID == STREAMID);
        assertTrue("elapsed increasing", msg.audioinputprogress.uElapsedMSec != 0);
        assertTrue("queue zero", msg.audioinputprogress.uQueueMSec == 0);

        assertTrue("Audio input ending "+STREAMID, waitForEvent(ttclient, ClientEvent.CLIENTEVENT_AUDIOINPUT, DEF_WAIT, msg));
        assertTrue("stream id ended", msg.audioinputprogress.nStreamID == STREAMID);
        assertTrue("elapsed done", msg.audioinputprogress.uElapsedMSec == 0);
        assertTrue("queue done", msg.audioinputprogress.uQueueMSec == 0);

        //drain audio blocks
        while (ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_LOCAL_USERID) != null);
        while (ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_MUXED_USERID) != null);
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 0);

        gotlocal = gotmuxed = false;
        while (!gotlocal || !gotmuxed) {

            assertTrue("get another audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            if (msg.nSource == Constants.TT_LOCAL_USERID) {
                AudioBlock abl = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_LOCAL_USERID);
                assertTrue("get local audio block", abl != null);
                gotlocal = true;
            }
            if (msg.nSource == Constants.TT_MUXED_USERID) {
                AudioBlock abm = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_MUXED_USERID);
                assertTrue("get muxed audio block", abm != null);
                gotmuxed = true;
            }
        }

        assertTrue("Got local user audio, after flush", gotlocal);
        assertTrue("Got muxed user audio, after flush", gotmuxed);

        // test audio input queue limits
        STREAMID = 77;
        ab.nStreamID = STREAMID;
        ab.lpRawAudio = generateToneAsByte(800, 16000, 1, 4000); // PCM16 mono
        ab.nSamples = ab.lpRawAudio.length / 2;

        // assertFalse("Maximum queue size for audio input is 3 sec", ttclient.insertAudioBlock(ab));
        localWaveFile.close();
        muxedWaveFile.close();
    }

    public void testVoiceTransmitOpenCloseAudioInput() throws InterruptedException {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_MULTI_LOGIN | UserRight.USERRIGHT_TRANSMIT_VOICE;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase[] clients = new TeamTalkBase[4];
        for (int i=0;i<clients.length;++i) {
            clients[i] = newClientInstance();
            IntPtr indev = new IntPtr(), outdev = new IntPtr();
            assertTrue("get default sound devices", clients[i].getDefaultSoundDevices(indev, outdev));
            assertTrue("init output device", clients[i].initSoundOutputDevice(outdev.value));
            connect(clients[i]);
            login(clients[i], NICKNAME + "_" + i, USERNAME, PASSWORD);
            joinRoot(clients[i]);
        }

        for (int i=0;i < 5; i++) {
            for (TeamTalkBase ttclient : clients) {
                IntPtr indev = new IntPtr(), outdev = new IntPtr();
                assertTrue("get default sound devices", ttclient.getDefaultSoundDevices(indev, outdev));
                assertTrue("client init sndinput", ttclient.initSoundInputDevice(indev.value));
                assertTrue("client enable voice tx", ttclient.enableVoiceTransmission(true));
            }

            waitForEvent(clients[0], ClientEvent.CLIENTEVENT_NONE, 1500);

            for (TeamTalkBase ttclient : clients) {
                assertTrue("client close sndinput", ttclient.closeSoundInputDevice());
                assertTrue("client disable voice tx", ttclient.enableVoiceTransmission(false));
            }

            waitForEvent(clients[0], ClientEvent.CLIENTEVENT_NONE, 1000);
        }
    }

    public void testSharedAudioInputOutput() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_MULTI_LOGIN | UserRight.USERRIGHT_TRANSMIT_VOICE |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase rxclient = newClientInstance();

        IntPtr indev = new IntPtr(), outdev = new IntPtr();
        if(INPUTDEVICEID < 0 && OUTPUTDEVICEID < 0)
           assertTrue("get default devs", rxclient.getDefaultSoundDevices(indev, outdev));
        else
        {
            indev.value = INPUTDEVICEID;
            outdev.value = OUTPUTDEVICEID;
        }

        indev.value |= SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;
        outdev.value |= SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;

        assertTrue("Init rx input", rxclient.initSoundInputDevice(indev.value));
        assertTrue("Init rx output", rxclient.initSoundOutputDevice(outdev.value));
        connect(rxclient);
        login(rxclient, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);
        Channel chan = buildDefaultChannel(rxclient, "Speex", Codec.SPEEX_CODEC);
        chan.audiocodec.speex.nBandmode = SpeexConstants.SPEEX_BANDMODE_WIDE; //16000
        chan.audiocodec.speex.nTxIntervalMSec = 400;
        assertTrue("rxclient join channel", waitCmdSuccess(rxclient, rxclient.doJoinChannel(chan), DEF_WAIT));

        TeamTalkBase txclient1 = newClientInstance();
        assertTrue("Init tx1 input", txclient1.initSoundInputDevice(indev.value));
        assertTrue("Init tx1 output", txclient1.initSoundOutputDevice(outdev.value));
        connect(txclient1);
        login(txclient1, NICKNAME, USERNAME, PASSWORD);
        joinRoot(txclient1);
        assertTrue("Gen tone tx1", txclient1.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 300));

        TeamTalkBase txclient2 = newClientInstance();
        assertTrue("Init tx2 input", txclient2.initSoundInputDevice(indev.value));
        assertTrue("Init tx2 output", txclient2.initSoundOutputDevice(outdev.value));
        connect(txclient2);
        login(txclient2, NICKNAME, USERNAME, PASSWORD);
        assertTrue("tx2 join existing" , waitCmdSuccess(txclient2, txclient2.doJoinChannelByID(rxclient.getMyChannelID(), ""), DEF_WAIT));
        assertTrue("Gen tone tx2", txclient2.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 600));

        TeamTalkBase txclient3 = newClientInstance();
        assertTrue("Init tx3 input", txclient3.initSoundInputDevice(indev.value));
        assertTrue("Init tx3 output", txclient3.initSoundOutputDevice(outdev.value));
        connect(txclient3);
        login(txclient3, NICKNAME, USERNAME, PASSWORD);
        chan = buildDefaultChannel(txclient3, "OPUS223", Codec.OPUS_CODEC);
        chan.audiocodec.opus.nSampleRate = 24000;
        chan.audiocodec.opus.nFrameSizeMSec = 60;
        chan.audiocodec.opus.nTxIntervalMSec = 60;
        assertTrue("txclient3 join channel", waitCmdSuccess(txclient3, txclient3.doJoinChannel(chan), DEF_WAIT));
        assertTrue("Gen tone tx3", txclient3.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 900));

        TeamTalkBase txclient4 = newClientInstance();
        assertTrue("Init tx4 input", txclient4.initSoundInputDevice(indev.value));
        assertTrue("Init tx4 output", txclient4.initSoundOutputDevice(outdev.value));
        connect(txclient4);
        login(txclient4, NICKNAME, USERNAME, PASSWORD);
        chan = buildDefaultChannel(txclient4, "OPUS224", Codec.OPUS_CODEC);
        chan.audiocodec.opus.nSampleRate = 12000;
        chan.audiocodec.opus.nFrameSizeMSec = 20;
        chan.audiocodec.opus.nTxIntervalMSec = 20;
        assertTrue("txclient4 join channel", waitCmdSuccess(txclient4, txclient4.doJoinChannel(chan), DEF_WAIT));
        assertTrue("Gen tone tx4", txclient4.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 1200));

        assertTrue("Intercept tx1", waitCmdSuccess(rxclient, rxclient.doSubscribe(txclient1.getMyUserID(), Subscription.SUBSCRIBE_INTERCEPT_VOICE), DEF_WAIT));
        assertTrue("Intercept tx2", waitCmdSuccess(rxclient, rxclient.doSubscribe(txclient2.getMyUserID(), Subscription.SUBSCRIBE_INTERCEPT_VOICE), DEF_WAIT));
        assertTrue("Intercept tx3", waitCmdSuccess(rxclient, rxclient.doSubscribe(txclient3.getMyUserID(), Subscription.SUBSCRIBE_INTERCEPT_VOICE), DEF_WAIT));
        assertTrue("Intercept tx4", waitCmdSuccess(rxclient, rxclient.doSubscribe(txclient4.getMyUserID(), Subscription.SUBSCRIBE_INTERCEPT_VOICE), DEF_WAIT));

        assertTrue("tx1 transmit", txclient1.enableVoiceTransmission(true));
        assertTrue("tx2 transmit", txclient2.enableVoiceTransmission(true));
        assertTrue("tx3 transmit", txclient3.enableVoiceTransmission(true));
        assertTrue("tx4 transmit", txclient4.enableVoiceTransmission(true));

        Vector<Integer> ids = new Vector<>();
        ids.add(txclient1.getMyUserID());
        ids.add(txclient2.getMyUserID());
        ids.add(txclient3.getMyUserID());
        ids.add(txclient4.getMyUserID());
        do {
            TTMessage msg = new TTMessage();
            assertTrue("get voice event", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));
            if ((msg.user.uUserState & UserState.USERSTATE_VOICE) != 0)
                ids.remove(new Integer(msg.user.nUserID));
        } while(ids.size() > 0);

        waitForEvent(rxclient, ClientEvent.CLIENTEVENT_NONE, 2000);

        assertTrue("tx1 stop transmit", txclient1.enableVoiceTransmission(false));
        assertTrue("tx2 stop transmit", txclient2.enableVoiceTransmission(false));
        assertTrue("tx3 stop transmit", txclient3.enableVoiceTransmission(false));
        assertTrue("tx4 stop transmit", txclient4.enableVoiceTransmission(false));

        ids.add(txclient1.getMyUserID());
        ids.add(txclient2.getMyUserID());
        ids.add(txclient3.getMyUserID());
        ids.add(txclient4.getMyUserID());
        do {
            TTMessage msg = new TTMessage();
            assertTrue("get voice stop event", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg));
            if ((msg.user.uUserState & UserState.USERSTATE_VOICE) == UserState.USERSTATE_NONE)
                ids.remove(new Integer(msg.user.nUserID));
        } while(ids.size()>0);
    }

    public void testInitSoundSharedInputOutput() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_MULTI_LOGIN | UserRight.USERRIGHT_TRANSMIT_VOICE |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        Channel chan = buildDefaultChannel(ttclient, "Opus", Codec.OPUS_CODEC);
        chan.audiocodec.opus.nFrameSizeMSec = 20;
        chan.audiocodec.opus.nTxIntervalMSec = 20;
        assertTrue("ttclient join channel", waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        IntPtr indev = new IntPtr(), outdev = new IntPtr();
        if(INPUTDEVICEID < 0 && OUTPUTDEVICEID < 0)
           assertTrue("get default devs", ttclient.getDefaultSoundDevices(indev, outdev));
        else
        {
            indev.value = INPUTDEVICEID;
            outdev.value = OUTPUTDEVICEID;
        }

        indev.value |= SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;
        outdev.value |= SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;

        TTMessage msg = new TTMessage();

        int[] samplerates = {8000, 12000, 16000, 24000, 48000};
        for (int samplerate : samplerates) {

            if (!supportsInputSampleRate(getSoundDevice(ttclient, indev.value & SoundDeviceConstants.TT_SOUNDDEVICE_ID_MASK), samplerate)) {
                System.err.println("Input device doesn't support sample rate: " + samplerate);
                continue;
            }

            if (!supportsOutputSampleRate(getSoundDevice(ttclient, outdev.value & SoundDeviceConstants.TT_SOUNDDEVICE_ID_MASK), samplerate)) {
                System.err.println("Output device doesn't support sample rate: " + samplerate);
                continue;
            }

            assertTrue("no waiting audio blocks", ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID()) == null);

            assertTrue("enable local aud cb", ttclient.enableAudioBlockEvent(Constants.TT_LOCAL_USERID, StreamType.STREAMTYPE_VOICE, true));
            assertTrue("setup shared input settings", TeamTalkBase.initSoundInputSharedDevice(samplerate, 2, samplerate));
            assertTrue("setup shared output settings", TeamTalkBase.initSoundOutputSharedDevice(samplerate, 2, samplerate));

            assertTrue("Init "+samplerate+" input", ttclient.initSoundInputDevice(indev.value));
            assertTrue("Init "+samplerate+" output", ttclient.initSoundOutputDevice(outdev.value));

            int samples = chan.audiocodec.opus.nSampleRate;
            AudioBlock ab;
            do {
                assertTrue("get "+samplerate+" audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
                assertEquals("from local", Constants.TT_LOCAL_USERID, msg.nSource);
                ab = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_LOCAL_USERID);
                assertTrue("get local audio block", ab != null);
                samples -= ab.nSamples;
            } while(samples > 0);

            long initialTS = System.currentTimeMillis();
            assertTrue("get "+samplerate+" audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            long nextTS = System.currentTimeMillis();

            assertTrue("close input", ttclient.closeSoundInputDevice());
            assertTrue("close output", ttclient.closeSoundOutputDevice());
            assertTrue("disable local aud cb", ttclient.enableAudioBlockEvent(Constants.TT_LOCAL_USERID, StreamType.STREAMTYPE_VOICE, false));
            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 0);
            while(ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_LOCAL_USERID) != null);

            assertTrue("reset shared input settings", TeamTalkBase.initSoundInputSharedDevice(0, 0, 0));
            assertTrue("reset shared output settings", TeamTalkBase.initSoundOutputSharedDevice(0, 0, 0));

            assertTrue(String.format("next in/out %d callback was %d msec later but should be ~1 sec", samplerate, nextTS - initialTS), nextTS - initialTS > 900);
        }

        assertTrue("reset shared input settings", TeamTalkBase.initSoundInputSharedDevice(0, 0, 0));

        assertTrue("Init input", ttclient.initSoundInputDevice(indev.value));

        assertTrue("enable tx", ttclient.enableVoiceTransmission(true));
        assertTrue("subscribe", waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        for (int samplerate : samplerates) {

            if (!supportsOutputSampleRate(getSoundDevice(ttclient, outdev.value & SoundDeviceConstants.TT_SOUNDDEVICE_ID_MASK), samplerate)) {
                System.err.println("Output device doesn't support sample rate: " + samplerate);
                continue;
            }

            assertTrue("no waiting audio blocks", ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID()) == null);

            assertTrue("enable local aud cb", ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true));
            assertTrue("setup shared output settings", TeamTalkBase.initSoundOutputSharedDevice(samplerate, 2, samplerate));

            assertTrue("Init "+samplerate+" output", ttclient.initSoundOutputDevice(outdev.value));

            int samples = chan.audiocodec.opus.nSampleRate;
            AudioBlock ab;
            do {
                assertTrue("get "+samplerate+" audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
                assertEquals("from myself", ttclient.getMyUserID(), msg.nSource);
                ab = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID());
                assertTrue("get audio block", ab != null);
                samples -= ab.nSamples;
            } while(samples > 0);

            long initialTS = System.currentTimeMillis();
            assertTrue("get "+samplerate+" audioblock", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            long nextTS = System.currentTimeMillis();

            assertTrue("close output", ttclient.closeSoundOutputDevice());
            assertTrue("disable aud cb", ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, false));
            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 0);
            while(ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID()) != null);

            assertTrue("reset shared output settings", TeamTalkBase.initSoundOutputSharedDevice(0, 0, 0));

            assertTrue(String.format("next output %d callback was %d msec later but should be ~1 sec", samplerate, nextTS - initialTS), nextTS - initialTS > 900);
        }
    }

    /* cannot test output levels since a user is muted by sound system after decoding and callback.

    public void testSharedAudioInputOutputLevels() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_MULTI_LOGIN | UserRight.USERRIGHT_TRANSMIT_VOICE |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase rxclient = newClientInstance();

        IntPtr indev = new IntPtr(), outdev = new IntPtr();
        if(INPUTDEVICEID < 0 && OUTPUTDEVICEID < 0)
           assertTrue("get default devs", rxclient.getDefaultSoundDevices(indev, outdev));
        else
        {
            indev.value = INPUTDEVICEID;
            outdev.value = OUTPUTDEVICEID;
        }

        indev.value |= SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;
        outdev.value |= SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;

        assertTrue("Init rx input", rxclient.initSoundInputDevice(indev.value));
        assertTrue("Init rx output", rxclient.initSoundOutputDevice(outdev.value));
        connect(rxclient);
        login(rxclient, NICKNAME, USERNAME, PASSWORD);
        Channel chan = buildDefaultChannel(rxclient, "Opus_12khz", Codec.OPUS_CODEC);
        chan.audiocodec.opus.nSampleRate = 12000;
        chan.audiocodec.opus.nChannels = 2;
        chan.audiocodec.opus.nTxIntervalMSec = 20;
        chan.audiocodec.opus.nFrameSizeMSec = 5;
        assertTrue("rxclient join channel", waitCmdSuccess(rxclient, rxclient.doJoinChannel(chan), DEF_WAIT));

        TeamTalkBase txclient1 = newClientInstance();
        assertTrue("Init tx1 input", txclient1.initSoundInputDevice(indev.value));
        assertTrue("Init tx1 output", txclient1.initSoundOutputDevice(outdev.value));
        connect(txclient1);
        login(txclient1, NICKNAME, USERNAME, PASSWORD);
        assertTrue("tx2 join existing" , waitCmdSuccess(txclient1, txclient1.doJoinChannelByID(rxclient.getMyChannelID(), ""), DEF_WAIT));
        assertTrue("Gen tone tx1", txclient1.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 300));

        assertTrue("enable callback", rxclient.enableAudioBlockEvent(txclient1.getMyUserID(), StreamType.STREAMTYPE_VOICE, true));
        assertTrue("mute user", rxclient.setUserMute(txclient1.getMyUserID(), StreamType.STREAMTYPE_VOICE, true));

        assertTrue("tx1 start transmit", txclient1.enableVoiceTransmission(true));

        TTMessage msg = new TTMessage();

        int count = 5;
        do {
            assertTrue("gimme voice audioblock", waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg));
            AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, txclient1.getMyUserID());
            assertTrue("got audio block", block != null);
            assertEquals("stereo block", 2, block.nChannels);
            short[] audio = audioToShortArray(block.lpRawAudio);
            int max = 0;
            for(int i=0;i<audio.length;i++) {
                assertEquals("Muted user", 0, audio[i]);
            }
        } while (count-- > 0);

        assertTrue("rxclient leave channel", waitCmdSuccess(rxclient, rxclient.doLeaveChannel(), DEF_WAIT));
    }
    */
}
