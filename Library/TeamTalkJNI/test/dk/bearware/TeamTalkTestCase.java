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

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.MethodOrderer;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.TestMethodOrder;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;
import java.text.ParsePosition;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.TimeZone;
import java.util.Vector;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

@TestMethodOrder(MethodOrderer.MethodName.class)
public abstract class TeamTalkTestCase extends TeamTalkTestCaseBase {

    @BeforeEach
    public void setUp() throws Exception {
        super.setUp();

        resetServerProperties();
    }

    @AfterEach
    public void tearDown() throws Exception {
        super.tearDown();
    }

    @Test
    public void testThis() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);
    }

    @Test
    public void testSoundDevs() {
        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);

        assertTrue(ttclient.setSoundOutputVolume(100),"Set output vol");
        assertTrue(ttclient.setSoundOutputMute(true), "Set output mute");
    }

    @Test
    public void testConnect() {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);

        assertTrue(waitCmdComplete(ttclient, ttclient.doPing(), DEF_WAIT), "ping");
    }

    @Test
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

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE,
                                DEF_WAIT, msg), "Wait for state change");


    }

    @Test
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

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join channel");

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

    @Test
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

    @Test
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

    @Test
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

        assertTrue(ttclient.sendDesktopWindow(wnd, BitmapFormat.BMP_RGB32) > 0, "send desktop window");

        TTMessage msg = new TTMessage();

        while(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER,
                           DEF_WAIT, msg) && msg.nBytesRemain > 0) {
        }

        assertTrue(msg.nBytesRemain == 0, "All bytes transferred");

        assertFalse(hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_TX_DESKTOP), "no tx desktop flag");

        assertTrue(hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_DESKTOP_ACTIVE), "Desktop active");

        assertTrue(ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_DESKTOP) > 0, "Subscribe to own");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, DEF_WAIT, msg));

        DesktopWindow wnd2 = ttclient.acquireUserDesktopWindow(msg.nSource);

        assertEquals(wnd2.nWidth, wnd.nWidth, "width");
        assertEquals(wnd2.nHeight, wnd.nHeight, "height");
        assertEquals(wnd2.frameBuffer.length, wnd.frameBuffer.length, "length");

        assertTrue(ttclient.closeDesktopWindow(), "Close desktop");
    }

    @Test
    public void testSendDesktopWindowTooBig() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                         UserRight.USERRIGHT_TRANSMIT_DESKTOP | UserRight.USERRIGHT_MULTI_LOGIN;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        TeamTalkBase rxclient = newClientInstance();
        connect(rxclient);
        login(rxclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(rxclient);

        DesktopWindow wnd = new DesktopWindow();
        wnd.nWidth = 5120;
        wnd.nHeight = 2880;
        wnd.bmpFormat = BitmapFormat.BMP_RGB32;
        wnd.nProtocol = DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1;
        wnd.frameBuffer = new byte[wnd.nWidth * wnd.nHeight * 4];

        assertFalse(ttclient.sendDesktopWindow(wnd, BitmapFormat.BMP_NONE) > 0, "send desktop window");
    }

    @Test
    public void testSendDesktopWindowLimits() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                         UserRight.USERRIGHT_TRANSMIT_DESKTOP | UserRight.USERRIGHT_MULTI_LOGIN;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        TeamTalkBase rxclient = newClientInstance();
        connect(rxclient);
        login(rxclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(rxclient);

        Vector<DesktopWindow> wndSizes = new Vector<DesktopWindow>();

        final int BLOCKS_MAX = 4095;
        {
            // RGB8
            DesktopWindow wnd = new DesktopWindow();
            wnd.nWidth = 120;
            wnd.nHeight = 34;
            final int PIXEL_SIZE = 1;
            final long PIXELS_MAX = wnd.nWidth * wnd.nHeight * BLOCKS_MAX;
            final long PIXEL_BYTES = PIXELS_MAX * PIXEL_SIZE;
            for (int w=120; w <= 65535; w += 120) {
                for (int h=34;h <= 65535; h += 34) {
                    if (w * h * PIXEL_SIZE > wnd.nWidth * wnd.nHeight * PIXEL_SIZE && w * h <= PIXELS_MAX) {
                        wnd.nWidth = w;
                        wnd.nHeight = h;
                    }
                }
            }
            wnd.bmpFormat = BitmapFormat.BMP_RGB8_PALETTE;
            wnd.nProtocol = DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1;
            wnd.frameBuffer = new byte[wnd.nWidth * wnd.nHeight * PIXEL_SIZE];
            for (int i=0;i<wnd.frameBuffer.length;++i)
                wnd.frameBuffer[i] = (byte)(i & 0xff);
            wndSizes.add(wnd);
        }
        {
            // RGB16_555
            DesktopWindow wnd = new DesktopWindow();
            wnd.nWidth = 102;
            wnd.nHeight = 20;
            final int PIXEL_SIZE = 2;
            final long PIXELS_MAX = wnd.nWidth * wnd.nHeight * BLOCKS_MAX;
            final long PIXEL_BYTES = PIXELS_MAX * PIXEL_SIZE;
            for (int w=102; w <= 65535; w += 102) {
                for (int h=20;h <= 65535; h += 20) {
                    if (w * h > wnd.nWidth * wnd.nHeight && w * h <= PIXELS_MAX) {
                        wnd.nWidth = w;
                        wnd.nHeight = h;
                    }
                }
            }
            wnd.bmpFormat = BitmapFormat.BMP_RGB16_555;
            wnd.nProtocol = DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1;
            wnd.frameBuffer = new byte[wnd.nWidth * wnd.nHeight * PIXEL_SIZE];
            for (int i=0;i<wnd.frameBuffer.length;++i)
                wnd.frameBuffer[i] = (byte)(i & 0xff);
           wndSizes.add(wnd);
        }
        {
            // RGB24
            DesktopWindow wnd = new DesktopWindow();
            wnd.nWidth = 85;
            wnd.nHeight = 16;
            final int PIXEL_SIZE = 3;
            final long PIXELS_MAX = wnd.nWidth * wnd.nHeight * BLOCKS_MAX;
            final long PIXEL_BYTES = PIXELS_MAX * PIXEL_SIZE;
            for (int w=85; w <= 65535; w += 85) {
                long padding = 0;
                if ((w * PIXEL_SIZE) % 4 != 0)
                    padding = 1;

                for (int h=16;h <= 65535; h += 16) {
                    if (w * h > wnd.nWidth * wnd.nHeight && (w + padding) * h <= PIXELS_MAX) {
                        wnd.nWidth = w;
                        wnd.nHeight = h;
                    }
                }
            }
            wnd.bmpFormat = BitmapFormat.BMP_RGB24;
            wnd.nProtocol = DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1;
            int widthBytes = wnd.nWidth * (int)PIXEL_SIZE;
            int padding = 0;
            if (widthBytes % 4 != 0) {
                padding = 4 - (widthBytes % 4);
                widthBytes += padding;
            }
            wnd.frameBuffer = new byte[widthBytes * wnd.nHeight];
            for (int h=0;h<wnd.nHeight;++h) {
                for (int i=0;i<widthBytes;++i) {
                    if (h==0) {
                        wnd.frameBuffer[i] = 127; // RGB24 padding
                    }
                    else {
                        wnd.frameBuffer[h * widthBytes + i] = (byte)(i & 0xff);
                    }
                }
            }
            wndSizes.add(wnd);
        }
        {
            // RGB32
            DesktopWindow wnd = new DesktopWindow();
            wnd.nWidth = 51;
            wnd.nHeight = 20;
            final int PIXEL_SIZE = 4;
            final long PIXELS_MAX = wnd.nWidth * wnd.nHeight * BLOCKS_MAX;
            final long PIXEL_BYTES = PIXELS_MAX * PIXEL_SIZE;
            for (int w=51; w <= 65535; w += 51) {
                for (int h=20;h <= 65535; h += 20) {
                    if (w * h > wnd.nWidth * wnd.nHeight && w * h <= PIXELS_MAX) {
                        wnd.nWidth = w;
                        wnd.nHeight = h;
                    }
                }
            }
            wnd.bmpFormat = BitmapFormat.BMP_RGB32;
            wnd.nProtocol = DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1;
            wnd.frameBuffer = new byte[wnd.nWidth * wnd.nHeight * PIXEL_SIZE];
            for (int i=0;i<wnd.frameBuffer.length;++i)
                wnd.frameBuffer[i] = (byte)(i & 0xff);
            wndSizes.add(wnd);
        }

        for (DesktopWindow wnd : wndSizes) {
            assertTrue(ttclient.sendDesktopWindow(wnd, BitmapFormat.BMP_NONE) > 0, "Send desktop window for RGB " + wnd.bmpFormat);

            TTMessage msg = new TTMessage();

            while(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER,
                               DEF_WAIT, msg) && msg.nBytesRemain > 0) {
            }

            assertTrue(msg.nBytesRemain == 0, "All bytes transferred for RGB " + wnd.bmpFormat);

            assertFalse(hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_TX_DESKTOP), "No tx desktop flag for RGB " + wnd.bmpFormat);

            assertTrue(hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_DESKTOP_ACTIVE), "Desktop active for RGB " + wnd.bmpFormat);

            boolean sameBitmap = false;
            while (waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, DEF_WAIT, msg)) {
                DesktopWindow wnd2 = rxclient.acquireUserDesktopWindow(msg.nSource);
                assertEquals(wnd.nWidth, wnd2.nWidth, "width for RGB " + wnd.bmpFormat);
                assertEquals(wnd.nHeight, wnd2.nHeight, "height for RGB " + wnd.bmpFormat);
                assertEquals(wnd.frameBuffer.length, wnd2.frameBuffer.length, "length for RGB " + wnd.bmpFormat);

                boolean same = true;
                for (int i=0;i<wnd.frameBuffer.length && same;++i) {
                    same &= wnd.frameBuffer[i] == wnd2.frameBuffer[i];
                }
                sameBitmap = same;
                if (same)
                    break;
            }
            assertTrue(sameBitmap, "Same bitmap for RGB " + wnd.bmpFormat);

            assertTrue(ttclient.closeDesktopWindow(), "Close desktop");
        }
    }

    @Test
    public void testVideoCaptureDevs() {

        if (VIDEODEVICEID.equals(VIDEODEVICEID_DISABLED)) {
            System.err.println("Video capture test skipped due to device id: " + VIDEODEVICEID_DISABLED);
            return;
        }

        TeamTalkBase ttclient = newClientInstance();
        Vector<VideoCaptureDevice> devs = new Vector<VideoCaptureDevice>();
        assertTrue(ttclient.getVideoCaptureDevices(devs), "retrieve video capture devices");
        assertTrue(devs.size() > 0, "video capture device available");
        for(int i=0;i<devs.size();i++) {
            assertTrue(!devs.get(i).szDeviceID.isEmpty(), "Device ID not empty");
            assertTrue(!devs.get(i).szDeviceName.isEmpty(), "Device name not empty");
            assertTrue(!devs.get(i).szCaptureAPI.isEmpty(), "Capture API not empty");

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

    @Test
    public void testVideoCaptureStream() {

        if (VIDEODEVICEID.equals(VIDEODEVICEID_DISABLED)) {
            System.err.println("Video capture test skipped due to device id: " + VIDEODEVICEID_DISABLED);
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
        assertTrue(ttclient.getVideoCaptureDevices(devs), "retrieve video capture devices");
        assertTrue(devs.size() > 0, "video capture device available");

        VideoCaptureDevice dev = devs.get(0);
        VideoFormat fmt = dev.videoFormats[0];

        assertTrue(ttclient.initVideoCaptureDevice(dev.szDeviceID, fmt), "Init video capture");

        assertTrue(waitCmdComplete(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(),
                                                                 Subscription.SUBSCRIBE_VIDEOCAPTURE),
                                    DEF_WAIT), "subscribe own video");

        VideoCodec vidcodec = new VideoCodec();
        vidcodec.nCodec = Codec.WEBM_VP8_CODEC;
        vidcodec.webm_vp8.nRcTargetBitrate = 256;

        assertTrue(ttclient.startVideoCaptureTransmission(vidcodec), "Start video capture");

        TTMessage msg = new TTMessage();

        int wait_frames = 100, frames_ok = 0;

        while(wait_frames-- > 0) {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_VIDEOCAPTURE, DEF_WAIT, msg), "get video frame");
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

        assertTrue(ttclient.stopVideoCaptureTransmission(), "stop video capture");

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

    @Test
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
        assertTrue(ttclient.getMediaFileInfo(MEDIAFILE_VIDEO, mfi), "Get media file info");

        VideoCodec vidcodec = new VideoCodec();
        vidcodec.nCodec = Codec.WEBM_VP8_CODEC;
        vidcodec.webm_vp8.nRcTargetBitrate = 256;

        assertTrue(ttclient.startStreamingMediaFileToChannel(MEDIAFILE_VIDEO, vidcodec), "Start");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg), "Wait stream event");

        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "Begin stream");
        assertEquals(mfi.szFileName, msg.mediafileinfo.szFileName, "Filename match");
        assertEquals(mfi.uDurationMSec, msg.mediafileinfo.uDurationMSec, "Found duration");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "Wait USER_STATECHANGE");

        VideoFrame vidfrm;
        int n_rx_frames = 0;
        while(hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_STREAM_VIDEO)) {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_MEDIAFILE_VIDEO, DEF_WAIT), "Wait  MEDIAFILE_VIDEO");

            vidfrm = ttclient.acquireUserMediaVideoFrame(ttclient.getMyUserID());
            if(vidfrm != null) {
                assertEquals(vidfrm.nWidth, mfi.videoFmt.nWidth, "Width ok");
                assertEquals(vidfrm.nHeight, mfi.videoFmt.nHeight, "Height ok");

                n_rx_frames++;
            }
        }
        assertTrue(n_rx_frames > 0, "Received frames");
        assertTrue(ttclient.stopStreamingMediaFileToChannel(), "Stopped");

        // play again 90% into the media file
        MediaFilePlayback mfp = new MediaFilePlayback();
        mfp.uOffsetMSec = (int)(mfi.uDurationMSec * 0.9);
        mfp.bPaused = false;
        mfp.audioPreprocessor.nPreprocessor = AudioPreprocessorType.SPEEXDSP_AUDIOPREPROCESSOR;
        mfp.audioPreprocessor.speexdsp = new SpeexDSP(SPEEXDSP_AVAILABLE);

        assertTrue(ttclient.startStreamingMediaFileToChannel(MEDIAFILE_VIDEO, mfp, vidcodec), "Start with offset");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg), "Wait stream event");

        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "Begin stream");
        assertEquals(mfi.szFileName, msg.mediafileinfo.szFileName, "Filename match");
        assertEquals(mfi.uDurationMSec, msg.mediafileinfo.uDurationMSec, "Found duration");
        assertTrue(mfi.uElapsedMSec >= mfp.uOffsetMSec, "Elapsed > mfp.uOffsetMSec");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "Wait USER_STATECHANGE");

        n_rx_frames = 0;
        while(hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_STREAM_VIDEO)) {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_MEDIAFILE_VIDEO, DEF_WAIT), "Wait  MEDIAFILE_VIDEO");

            vidfrm = ttclient.acquireUserMediaVideoFrame(ttclient.getMyUserID());
            if(vidfrm != null) {
                assertEquals(vidfrm.nWidth, mfi.videoFmt.nWidth, "Width ok");
                assertEquals(vidfrm.nHeight, mfi.videoFmt.nHeight, "Height ok");

                n_rx_frames++;
            }
        }
        assertTrue(n_rx_frames > 0, "Received frames");
        assertTrue(ttclient.stopStreamingMediaFileToChannel(), "Stopped");
    }

    @Test
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
        assertTrue(ttclient.getMediaFileInfo(MEDIAFILE_VIDEO, mfi), "Get media file info");

        VideoCodec vidcodec = new VideoCodec();
        vidcodec.nCodec = Codec.WEBM_VP8_CODEC;
        vidcodec.webm_vp8.nRcTargetBitrate = 256;

        // play again 90% into the media file
        MediaFilePlayback mfp = new MediaFilePlayback();
        mfp.uOffsetMSec = (int)(mfi.uDurationMSec * 0.9);
        mfp.bPaused = false;
        mfp.audioPreprocessor.nPreprocessor = AudioPreprocessorType.SPEEXDSP_AUDIOPREPROCESSOR;
        mfp.audioPreprocessor.speexdsp = new SpeexDSP(SPEEXDSP_AVAILABLE);

        assertTrue(ttclient.startStreamingMediaFileToChannel(MEDIAFILE_VIDEO, mfp, vidcodec), "Start with offset");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg), "Wait stream event");

        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "Begin stream");
        assertEquals(mfi.szFileName, msg.mediafileinfo.szFileName, "Filename match");
        assertEquals(mfi.uDurationMSec, msg.mediafileinfo.uDurationMSec, "Found duration");
        assertTrue(msg.mediafileinfo.uElapsedMSec >= mfp.uOffsetMSec, "Elapsed >= mfp.uOffsetMSec");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "Wait USER_STATECHANGE");

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, 0, msg), "Media file is still playing");

        mfp.bPaused = true;
        vidcodec.webm_vp8.nRcTargetBitrate = 128;
        //rewind
        mfp.uOffsetMSec = (int)(mfi.uDurationMSec * 0.9);

        assertTrue(ttclient.updateStreamingMediaFileToChannel(mfp, vidcodec), "Pause media stream");
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg), "Wait stream event");
        assertEquals(MediaFileStatus.MFS_PAUSED, msg.mediafileinfo.nStatus, "Paused stream");

        mfp.bPaused = false;
        assertTrue(ttclient.updateStreamingMediaFileToChannel(mfp, vidcodec), "Unpaused media stream");
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg), "Wait stream event");
        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "Started stream again");

        assertTrue(ttclient.stopStreamingMediaFileToChannel(), "Stopped");
    }

    @Test
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
        assertTrue(ttclient.getMediaFileInfo(HTTPS_MEDIAFILE, mfi), "Get media file info");

        VideoCodec vidcodec = new VideoCodec();

        assertTrue(ttclient.startStreamingMediaFileToChannel(HTTPS_MEDIAFILE, vidcodec), "Start");
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg), "Wait stream event");

        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "Begin stream");
        assertEquals(mfi.szFileName, msg.mediafileinfo.szFileName, "Filename match");
        assertEquals(mfi.uDurationMSec, msg.mediafileinfo.uDurationMSec, "Found duration");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "Wait USER_STATECHANGE");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient.stopStreamingMediaFileToChannel(), "Stopped");
    }



    @Test
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
            login(ttclient, NICKNAME + (i), USERNAME, PASSWORD);
            joinRoot(ttclient);

            ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq);
            clients.add(ttclient);

            freq += 150;
        }

        ttclient = clients.get(0);

        Channel chan = new Channel();
        ttclient.getChannel(ttclient.getMyChannelID(), chan);

        assertEquals(Codec.OPUS_CODEC, chan.audiocodec.nCodec, "OPUS codec running");
        String opuswavefile = String.format("%s_opus.wav", getTestMethodName());
        assertTrue(ttclient.startRecordingMuxedAudioFile(chan.audiocodec,
                                                                               STORAGEFOLDER + File.separator + opuswavefile, AudioFileFormat.AFF_WAVE_FORMAT), "Opus to muxed wave");

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

        String spxwavefile = String.format("%s_speex.wav", getTestMethodName());
        assertTrue(ttclient.startRecordingMuxedAudioFile(chan.audiocodec,
                                                         STORAGEFOLDER + File.separator + spxwavefile,
                                                         AudioFileFormat.AFF_WAVE_FORMAT),
                                                         "Speex to muxed wave");

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

    @Test
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
        String recfile = String.format("%s.wav", getTestMethodName());
        assertTrue(ttclient.startRecordingMuxedAudioFile(chan.audiocodec,
                                                                                    STORAGEFOLDER + File.separator + recfile, AudioFileFormat.AFF_WAVE_FORMAT), "Record muxed audio file");

        assertTrue(ttclient.enableVoiceTransmission(true), "enable voice tx");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);

        assertTrue(ttclient.stopRecordingMuxedAudioFile(), "Stop recording muxed audio file");

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 100), "Not record event should happen");

        ttclient.enableVoiceTransmission(false);
    }

    @Test
    public void testMediaStorage_OpusOutput() {

        if (!OPUSTOOLS_AVAILABLE) {
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
        assertTrue(ttclient1.setSoundInputPreprocess(new SpeexDSP()), "set preprocess");

        ttclient2 = newClientInstance();
        initSound(ttclient2);
        assertTrue(ttclient2.setSoundInputPreprocess(new SpeexDSP()), "set preprocess");

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
                                                                                   Subscription.SUBSCRIBE_VOICE), DEF_WAIT), "do subscribe");

        assertEquals(Codec.OPUS_CODEC, chan.audiocodec.nCodec, "OPUS codec running");
        String opusfile = String.format("%s.ogg", getTestMethodName());
        assertTrue(ttclient1.startRecordingMuxedAudioFile(chan.audiocodec,
                                                                              STORAGEFOLDER + File.separator + opusfile, AudioFileFormat.AFF_CHANNELCODEC_FORMAT),
                   "Mux to Opus file");

        assertTrue(ttclient1.enableVoiceTransmission(true), "enable voice tx 1");

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient1.enableVoiceTransmission(false);

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient2.enableVoiceTransmission(true), "enable voice tx 2");

        waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient2.enableVoiceTransmission(false);

        ttclient2.disconnect();

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient1.stopRecordingMuxedAudioFile(), "Stop recording");

        assertFalse(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 100));
    }

    @Test
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
        assertTrue(ttclient1.setSoundInputPreprocess(new SpeexDSP()), "set preprocess");

        ttclient2 = newClientInstance();
        initSound(ttclient2);
        assertTrue(ttclient2.setSoundInputPreprocess(new SpeexDSP()), "set preprocess");

        connect(ttclient1);
        login(ttclient1, NICKNAME, USERNAME, PASSWORD);
        Channel chan = buildDefaultChannel(ttclient1, "speex channel", Codec.SPEEX_CODEC);
        assertTrue(waitCmdSuccess(ttclient1, ttclient1.doJoinChannel(chan), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient1, ttclient1.doSubscribe(ttclient1.getMyUserID(),
                                                                  Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        assertTrue(ttclient1.getChannel(ttclient1.getMyChannelID(), chan), "get channel");

        assertEquals(Codec.SPEEX_CODEC, chan.audiocodec.nCodec, "Speex codec running");

        connect(ttclient2);
        login(ttclient2, NICKNAME, USERNAME, PASSWORD);
        assertTrue(waitCmdSuccess(ttclient2, ttclient2.doJoinChannelByID(chan.nChannelID, ""), DEF_WAIT));

        ttclient1.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq);
        ttclient2.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq += 300);

        // now store in Speex
        String spxfile = String.format("%s_speex.ogg", getTestMethodName());
        assertTrue(ttclient1.startRecordingMuxedAudioFile(chan.audiocodec,
                                                                              STORAGEFOLDER + File.separator + spxfile, AudioFileFormat.AFF_CHANNELCODEC_FORMAT),
                   "Mux to Speex file");

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
        assertTrue(waitCmdSuccess(ttclient2, ttclient2.doJoinChannel(chan), DEF_WAIT), "join speex vbr channel");

        assertTrue(waitCmdSuccess(ttclient2, ttclient2.doSubscribe(ttclient2.getMyUserID(),
                                                                   Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        // now store in Speex
        String spxvbrfile = String.format("%s_speexvbr.ogg", getTestMethodName());
        assertTrue(ttclient2.startRecordingMuxedAudioFile(chan.audiocodec,
                                                         STORAGEFOLDER + File.separator + spxvbrfile, AudioFileFormat.AFF_CHANNELCODEC_FORMAT),
                   "Mux to Speex VBR file");

        assertTrue(ttclient2.getChannel(ttclient2.getMyChannelID(), chan), "get channel spx vbr");

        assertEquals(Codec.SPEEX_VBR_CODEC, chan.audiocodec.nCodec, "Speex VBR codec running");
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

    @Test
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
        String recfile = String.format("%s.wav", getTestMethodName());
        assertTrue(ttclient.startRecordingMuxedAudioFile(chan.audiocodec,
                                                         STORAGEFOLDER + File.separator + recfile, AudioFileFormat.AFF_WAVE_FORMAT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(),
                                                                 Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        assertTrue(ttclient.enableVoiceTransmission(true));

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient.stopRecordingMuxedAudioFile());

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 100));

        ttclient.enableVoiceTransmission(false);
    }

    @Test
    public void testRecordMultipleChannels() {

        if (GITHUBSKIP) {
            // uses System.currentTimeMillis()
            return;
        }

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
            assertTrue(ttclient1.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 400), "tone1");
            assertTrue(ttclient2.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 800), "tone2");

            connect(ttclient1);
            connect(ttclient2);
            initSound(ttclient1);
            initSound(ttclient2);
            SpeexDSP spxdsp = new SpeexDSP();
            assertTrue(ttclient1.setSoundInputPreprocess(spxdsp), "disable spx dsp");
            assertTrue(ttclient2.setSoundInputPreprocess(spxdsp), "disable spx dsp");
            login(ttclient1, NICKNAME + "#" + ttclient1.getMyUserID(), USERNAME, PASSWORD);
            login(ttclient2, NICKNAME + "#" + ttclient2.getMyUserID(), USERNAME, PASSWORD);

            Channel chan = buildDefaultChannel(ttclient1, "Opus" + ttclient1.getMyUserID(), Codec.OPUS_CODEC);
            chan.audiocodec.opus.nTxIntervalMSec = txintervalsMSec[i];
            assertTrue(ttclient1.doJoinChannel(chan) > 0, "join1");
            assertTrue(ttclient2.doJoinChannelByID(ttclient1.getMyChannelID(), "") > 0, "join2");
        }

        TeamTalkBase ttadmin = newClientInstance();
        connect(ttadmin);
        initSound(ttadmin);
        login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        for (TeamTalkBase ttclient : clients) {
            assertTrue(waitCmdSuccess(ttadmin, ttadmin.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_INTERCEPT_VOICE), DEF_WAIT), "Intercept");
        }

        Channel chan = new Channel();
        Vector<String> filenames = new Vector<>();
        Vector<Integer> cbintervalMSec = new Vector<>();
        for (int i=0;i<clients.size();i+=2) {
            TeamTalkBase ttclient = clients.elementAt(i);
            assertTrue(ttclient.getChannel(ttclient.getMyChannelID(), chan), "get channel");
            String filename = STORAGEFOLDER + File.separator + "MuxedRecording-" +
                chan.szName + "-#" + ttclient.getMyUserID() + "-" + chan.audiocodec.opus.nTxIntervalMSec + "msec.wav";

            filenames.add(filename);
            cbintervalMSec.add(chan.audiocodec.opus.nTxIntervalMSec);
            assertTrue(ttadmin.startRecordingMuxedAudioFile(ttclient.getMyChannelID(), filename, AudioFileFormat.AFF_WAVE_FORMAT), "Record mux");
        }

        String rootfilename = STORAGEFOLDER + File.separator + "MuxedRecording-Root-Channel.wav";
        filenames.add(rootfilename);
        assertTrue(ttadmin.startRecordingMuxedAudioFile(ttadmin.getRootChannelID(), rootfilename, AudioFileFormat.AFF_WAVE_FORMAT), "Record root mux");
        assertTrue(ttadmin.getChannel(ttadmin.getRootChannelID(), chan), "get root channel");
        cbintervalMSec.add(chan.audiocodec.opus.nTxIntervalMSec);

        assertEquals(filenames.size(), cbintervalMSec.size(), "filenames and cbintervals");

        long starttime = System.currentTimeMillis();

        waitForEvent(ttadmin, ClientEvent.CLIENTEVENT_NONE, 1000);

        for (int i=0;i<clients.size();i+=2) {
            TeamTalkBase ttclient = clients.elementAt(i);
            assertTrue(ttclient.enableVoiceTransmission(true), "enable tone");
        }

        waitForEvent(ttadmin, ClientEvent.CLIENTEVENT_NONE, 3000);

        for (int i=0;i<clients.size();i+=2) {
            TeamTalkBase ttclient = clients.elementAt(i);
            assertTrue(ttclient.enableVoiceTransmission(false), "disable tone");
        }

        waitForEvent(ttadmin, ClientEvent.CLIENTEVENT_NONE, 1000);

        for (int i=1;i<clients.size();i+=2) {
            TeamTalkBase ttclient = clients.elementAt(i);
            assertTrue(ttclient.enableVoiceTransmission(true), "enable tone");
        }

        waitForEvent(ttadmin, ClientEvent.CLIENTEVENT_NONE, 3000);

        long duration = System.currentTimeMillis() - starttime;
        for (int i=0;i<clients.size();i+=2) {
            TeamTalkBase ttclient = clients.elementAt(i);
            assertTrue(ttadmin.stopRecordingMuxedAudioFile(ttclient.getMyChannelID()), "Stop Record mux");
        }

        assertTrue(ttadmin.stopRecordingMuxedAudioFile(ttadmin.getRootChannelID()), "Stop Record root mux");

        for (String filename : filenames) {
            MediaFileInfo mfi = new MediaFileInfo();
            assertTrue(ttadmin.getMediaFileInfo(filename, mfi), "Open media file " + filename);
            // callbacks from sound input device may not have
            // completed when file was closed, so put in a
            // tolerance. This is not a very precise way of testing
            // the duration of the files, but at least they should be
            // somewhere near the duration of the recording.
            int tolerance = cbintervalMSec.remove(0) * 3;
            assertTrue(mfi.uDurationMSec + tolerance >= duration, "Duration of " + filename + " is " + mfi.uDurationMSec + ", must be >= " + duration + ", tolerance " + tolerance);
        }
    }

    @Test
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
        assertEquals(Codec.OPUS_CODEC, chan.audiocodec.nCodec);
        int framesize = (int)((chan.audiocodec.opus.nTxIntervalMSec / 1000.) * chan.audiocodec.opus.nSampleRate);

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join");

        assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT), "subscribe");

        assertTrue(ttclient.enableVoiceTransmission(true), "vox");

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000), "no voice audioblock");

        assertTrue(ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true), "enable aud cb");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme voice audioblock");

        assertEquals(TTType.__STREAMTYPE, msg.ttType, "StreamType");
        assertEquals(StreamType.STREAMTYPE_VOICE, msg.nStreamType, "Voice StreamType");

        AudioBlock block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID());

        assertTrue(block.nSamples > 0, "Audio block has samples");

        //drain message before we start calculating
        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 100), "No queued events");

        assertTrue(ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_MEDIAFILE_AUDIO, true), "Enable aud cb");

        MediaFileInfo mfi = new MediaFileInfo();
        mfi.szFileName = STORAGEFOLDER + File.separator + "hest.wav";
        mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 2);
        mfi.uDurationMSec = 30 * 1000;

        assertTrue(TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600), "Write media file");

        assertTrue(ttclient.startStreamingMediaFileToChannel(mfi.szFileName, new VideoCodec()), "Start stream file");

        int n_voice_blocks = 0, n_mfa_blocks = 0, voice_sampleindex = -1, mfa_sampleindex = -1, voice_sid = -1, mfa_sid = -1;
        while (n_voice_blocks < 10 || n_mfa_blocks < 10)
        {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "Wait for audio block");
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
                    assertEquals(voice_sampleindex, block.uSampleIndex, "Voice sample index match");
                    assertEquals(voice_sid, block.nStreamID, "Voice stream ID match");
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
                    assertEquals(mfa_sampleindex, block.uSampleIndex, "Media file sample index match");
                    assertEquals(mfa_sid, block.nStreamID, "Media file stream ID match");
                }
                mfa_sampleindex += framesize;
                break;
            }
        }

        assertTrue(n_voice_blocks >= 10, "Voice blocks ok");
        assertTrue(n_mfa_blocks >= 10, "Media file ok");

        assertTrue(ttclient.stopStreamingMediaFileToChannel(), "stop streaming");

        assertTrue(waitCmdSuccess(ttclient, ttclient.doLeaveChannel(), DEF_WAIT), "leave channel");

        // drain audio blocks completely
        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000), "No queued events");
        while(ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID()) != null);
        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 0), "message queue has no audio block");

        //now test that mute stereo mode having effect
        chan.audiocodec = new AudioCodec();
        chan.audiocodec.nCodec = Codec.SPEEX_CODEC;
        chan.audiocodec.speex.bStereoPlayback = true;

        // test right channel is mute
        assertTrue(ttclient.setUserStereo(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true, false), "set right mute");

        assertTrue(ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 440), "start tone");

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join channel");

        n_voice_blocks = 0;
        while (n_voice_blocks++ < 10)
        {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "Wait for audio block");
            assertEquals(StreamType.STREAMTYPE_VOICE, msg.nStreamType,"stream is voice for right mute");

            block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertTrue(block != null, "got audio block for right mute");
            assertEquals(2, block.nChannels, "stereo");
            for(int i=0;i<block.lpRawAudio.length;i+=4) {
                assertEquals(0, block.lpRawAudio[i+2], "right channel is mute");
                assertEquals(0, block.lpRawAudio[i+3], "right channel is mute");
            }
        }

        assertTrue(ttclient.enableVoiceTransmission(false));

        // drain audio blocks completely
        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000), "No queued events");
        while(ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID()) != null);
        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 0), "message queue has no audio block");

        // test left channel is mute
        assertTrue(ttclient.setUserStereo(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, false, true), "set left mute");

        assertTrue(ttclient.enableVoiceTransmission(true));

        n_voice_blocks = 0;
        while (n_voice_blocks++ < 10)
        {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "got audio block");
            assertEquals(StreamType.STREAMTYPE_VOICE, msg.nStreamType,"stream is voice for left mute");

            block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertTrue(block != null, "got audio block for left mute");
            assertEquals(2, block.nChannels, "stereo");

            for(int i=0;i<block.lpRawAudio.length;i+=4) {
                assertEquals(0, block.lpRawAudio[i], "left channel is mute");
                assertEquals(0, block.lpRawAudio[i+1], "left channel is mute");
            }
        }
    }

    @Test
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
        assertEquals(Codec.OPUS_CODEC, chan.audiocodec.nCodec);

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT),"join");

        assertTrue(ttclient.enableVoiceTransmission(true), "vox");

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000), "no voice audioblock");

        assertTrue(ttclient.enableAudioBlockEvent(Constants.TT_LOCAL_USERID, StreamType.STREAMTYPE_VOICE, true), "enable aud cb");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme voice audioblock");

        assertEquals(TTType.__STREAMTYPE, msg.ttType, "StreamType");
        assertEquals(StreamType.STREAMTYPE_VOICE, msg.nStreamType, "Voice StreamType");

        AudioBlock block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);

        assertEquals(1, block.nStreamID, "First stream");
        assertTrue(block.nSamples > 0, "aud block has samples");

        int receiveSamples = block.nSampleRate * 3;
        while (receiveSamples > 0) {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme 3 secs of voice audioblock");
            assertEquals(Constants.TT_LOCAL_USERID, msg.nSource, "local userid");
            block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertEquals(1, block.nStreamID, "Still first stream");
            receiveSamples -= block.nSamples;
        }

        // ensure voice stream id changes
        assertTrue(ttclient.enableVoiceTransmission(false), "disable vox");
        assertTrue(ttclient.enableVoiceTransmission(true), "vox again");

        // drain remaining frames
        do {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "wait for next audioblock");
            block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
        } while (block.nStreamID == 1);

        receiveSamples = block.nSampleRate * 2;
        while (receiveSamples > 0) {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme 2 secs of voice audioblock");
            block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertEquals(2, block.nStreamID, "Second stream");
            receiveSamples -= block.nSamples;
        }
    }

    @Test
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
        assertTrue(rxclient.setSoundInputPreprocess(spxdsp), "disable spx dsp");
        assertTrue(rxclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 1000), "Gen tone");

        assertTrue(waitCmdSuccess(rxclient, rxclient.doJoinChannel(chan), DEF_WAIT), "join");

        assertTrue(rxclient.getChannel(rxclient.getMyChannelID(), chan), "get new chan");

        int freq = 0;
        for (TeamTalkBase ttclient : txclients) {
            connect(ttclient);
            initSound(ttclient);
            login(ttclient, NICKNAME, USERNAME, PASSWORD);
            assertTrue(ttclient.setSoundInputPreprocess(spxdsp), "disable spx dsp");
            assertTrue(ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq += 100), "Gen tone");
            assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join");
        }

        // limit number of active streams (we only care about the muxer)
        for (TeamTalkBase outerclient : txclients) {
            for (TeamTalkBase innerclient : txclients) {
                if (innerclient != outerclient)
                    assertTrue(waitCmdSuccess(innerclient, innerclient.doUnsubscribe(outerclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT), "unsubscribe");
            }
        }

        assertTrue(rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, true), "enable aud cb");
        assertTrue(rxclient.startRecordingMuxedAudioFile(chan.audiocodec, STORAGEFOLDER + File.separator + "muxfileoutput.wav", AudioFileFormat.AFF_WAVE_FORMAT), "enable aud mux file");

        int bytelen = chan.audiocodec.opus.nSampleRate * chan.audiocodec.opus.nChannels * 12/*seconds*/ * 2 /*short*/;
        try (FileOutputStream fs = newWaveFile(STORAGEFOLDER + File.separator + "muxoutput_opus.wav", chan.audiocodec.opus.nSampleRate, chan.audiocodec.opus.nChannels, bytelen);) {

            int receiveSamples = chan.audiocodec.opus.nSampleRate;
            do {
                assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg),"gimme 1 secs of voice audioblock");
                assertEquals(Constants.TT_MUXED_USERID, msg.nSource, "muxed userid");
                AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                assertTrue(block != null, "block valid");
                assertTrue(block.nSamples > 0, "aud block has samples");
                assertEquals(chan.audiocodec.opus.nSampleRate, block.nSampleRate, "channel samplerate");
                receiveSamples -= block.nSamples;
                fs.write(block.lpRawAudio);
            } while (receiveSamples > 0);
            // 1 sec

            assertTrue(rxclient.enableVoiceTransmission(true), "enable tx");
            receiveSamples = chan.audiocodec.opus.nSampleRate;
            do {
                assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "tone for 1 sec of voice audioblock");
                assertEquals(Constants.TT_MUXED_USERID, msg.nSource, "muxed userid");
                AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                assertTrue(block != null, "block valid");
                assertEquals(chan.audiocodec.opus.nSampleRate, block.nSampleRate, "channel samplerate");
                receiveSamples -= block.nSamples;
                fs.write(block.lpRawAudio);
            } while (receiveSamples > 0);
            // 2 sec

            assertTrue(rxclient.enableVoiceTransmission(false), "disable tx");

            for (TeamTalkBase ttclient : txclients) {
                assertTrue(ttclient.enableVoiceTransmission(true), "enable tx");
                receiveSamples = chan.audiocodec.opus.nSampleRate;
                do {
                    assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme 1 sec tone of voice audioblock, so far " + receiveSamples);
                    assertEquals(Constants.TT_MUXED_USERID, msg.nSource, "muxed userid");
                    AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                    assertTrue(block != null, "block valid");
                    assertEquals(chan.audiocodec.opus.nSampleRate, block.nSampleRate, "channel samplerate");
                    receiveSamples -= block.nSamples;
                    fs.write(block.lpRawAudio);
                } while (receiveSamples > 0);
                assertTrue(ttclient.enableVoiceTransmission(false), "disable tx");
            }
            // 11 sec

            for (TeamTalkBase ttclient : txclients) {
                assertTrue(ttclient.enableVoiceTransmission(true), "enable tx");
            }

            receiveSamples = chan.audiocodec.opus.nSampleRate;
            do {
                assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme 1 sec tone of voice audioblock, so far " + receiveSamples);
                assertEquals(Constants.TT_MUXED_USERID, msg.nSource, "muxed userid");
                AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                assertTrue(block != null, "block valid");
                assertEquals(chan.audiocodec.opus.nSampleRate, block.nSampleRate, "channel samplerate");
                receiveSamples -= block.nSamples;
                fs.write(block.lpRawAudio);
            } while (receiveSamples > 0);
            // 12 sec

            assertTrue(waitCmdSuccess(rxclient, rxclient.doLeaveChannel(), DEF_WAIT), "leave opus");

            assertTrue(rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, false), "disable aud cb and remove pending audio blocks");

            assertTrue(rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_MUXED_USERID) == null, "cleared audio blocks");

            assertFalse(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 500), "no audio mux after disable");

            chan = buildDefaultChannel(rxclient, "Speex VBR", Codec.SPEEX_VBR_CODEC);

            assertTrue(waitCmdSuccess(rxclient, rxclient.doJoinChannel(chan), DEF_WAIT), "join");

            assertTrue(rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, true), "enable aud cb again");

            assertTrue(rxclient.getChannel(rxclient.getMyChannelID(), chan), "get new chan");
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
            assertTrue(false, "invalid samplerate");
            samplerate = 0;
            break;
        }
        bytelen = samplerate * 5;
        try (FileOutputStream fs = newWaveFile(STORAGEFOLDER + File.separator + "muxoutput_speexvbr.wav", samplerate, 1, bytelen);) {
            int receiveSamples = samplerate;
            do {
                assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme 1 sec tone of voice speex vbr audioblock");
                assertEquals(Constants.TT_MUXED_USERID, msg.nSource, "muxed userid");
                AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                assertTrue(block != null, "block valid");
                assertEquals(samplerate, block.nSampleRate, "correct sample rate");
                assertEquals(1, block.nChannels, "correct channels");
                receiveSamples -= block.nSamples;
                fs.write(block.lpRawAudio);
            } while (receiveSamples > 0);

            for (TeamTalkBase ttclient : txclients) {
                assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join spx vbr");
            }

            receiveSamples = samplerate * 4;
            do {
                assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme 4 sec tones of voice speex vbr audioblock");
                assertEquals(Constants.TT_MUXED_USERID, msg.nSource, "muxed userid");
                AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                assertTrue(block != null, "block valid");
                assertEquals(samplerate, block.nSampleRate, "correct sample rate");
                assertEquals(1, block.nChannels, "correct channels");
                receiveSamples -= block.nSamples;
                fs.write(block.lpRawAudio);
            } while (receiveSamples > 0);

            assertTrue(waitCmdSuccess(rxclient, rxclient.doLeaveChannel(), DEF_WAIT), "leave spx");

            assertTrue(rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, false), "disable aud cb again");

            assertFalse(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 500), "no audio mux when out of channel");

            // drain any remaining audio blocks
            while(rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_MUXED_USERID) != null);

            // ensure 'muxfileoutput.wav' will continue writing again
            chan = buildDefaultChannel(rxclient, "Opus - Muxed secondary", Codec.OPUS_CODEC);
            chan.audiocodec.opus.nSampleRate = 12000;
            chan.audiocodec.opus.nChannels = 2;
            chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
            chan.audiocodec.opus.bDTX = false;

            assertTrue(waitCmdSuccess(rxclient, rxclient.doJoinChannel(chan), DEF_WAIT), "join");

            assertTrue(rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, true), "enable aud cb again");

            assertTrue(rxclient.getChannel(rxclient.getMyChannelID(), chan), "get new chan");
            receiveSamples = chan.audiocodec.opus.nSampleRate;
            do {
                assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme 1 secs of opus voice audioblock again");
                assertEquals(Constants.TT_MUXED_USERID, msg.nSource, "muxed userid");
                AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                assertTrue(block != null, "block valid");
                assertTrue(block.nSamples > 0, "aud block has samples");
                assertEquals(chan.audiocodec.opus.nSampleRate, block.nSampleRate, "12khz");
                receiveSamples -= block.nSamples;
            } while (receiveSamples > 0);
        }
    }


    @Test
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

        assertTrue(waitCmdSuccess(rxclient, rxclient.doJoinChannel(chan), DEF_WAIT), "join");
        assertTrue(rxclient.getChannel(rxclient.getMyChannelID(), chan), "get new chan");
        assertTrue(rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, true), "enable aud cb");

        // first receive initial audio blocks with sound input device active
        int receiveBlocks = 5;
        do {
            assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme 1 secs of voice audioblock");
            assertEquals(Constants.TT_MUXED_USERID, msg.nSource, "muxed userid");
            AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertTrue(block != null, "block valid initially");
            assertTrue(block.nSamples > 0, "aud block has samples");
            receiveBlocks--;
        } while (receiveBlocks > 0);

        // close sound input device and ensure we still receive samples
        assertTrue(rxclient.closeSoundInputDevice(), "close sound input");

        receiveBlocks = 10;
        do {
            assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme audioblock with sound input disabled");
            assertEquals(Constants.TT_MUXED_USERID, msg.nSource, "muxed userid");
            AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertTrue(block != null, "block valid with sound input disabled");
            assertTrue(block.nSamples > 0, "aud block has samples");
            receiveBlocks--;
        } while (receiveBlocks > 0);

        // restart audio blocks while sound input device is disabled
        assertTrue(rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, false), "disable aud cb and remove pending audio blocks");
        waitForEvent(rxclient, ClientEvent.CLIENTEVENT_NONE, 0);
        assertTrue(rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, true), "enable aud cb again");

        receiveBlocks = 5;
        do {
            assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme audio block with reenabled audioblocks and sound input disabled");
            assertEquals(Constants.TT_MUXED_USERID, msg.nSource, "muxed userid");
            AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            assertTrue(block != null, "block valid after reenable");
            assertTrue(block.nSamples > 0, "aud block has samples");
            receiveBlocks--;
        } while (receiveBlocks > 0);
    }

    @Test
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
        assertEquals(Codec.OPUS_CODEC, chan.audiocodec.nCodec);

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join");

        assertTrue(ttclient.enableVoiceTransmission(true), "vox");

        AudioFormat fmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 8000, 2);
        int chan_framesize = (int)((chan.audiocodec.opus.nTxIntervalMSec / 1000.) * fmt.nSampleRate);
        int mux_framesize = (int)((20 / 1000.) * fmt.nSampleRate);

        // test for TT_LOCAL_USERID
        assertTrue(ttclient.enableAudioBlockEvent(Constants.TT_LOCAL_USERID, StreamType.STREAMTYPE_VOICE, fmt, true), "enable aud cb");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme voice audioblock");

        assertEquals(Constants.TT_LOCAL_USERID, msg.nSource, "from local user");

        AudioBlock ab = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
        assertTrue(ab != null, "retrieve audioblock");

        assertEquals(fmt.nChannels, ab.nChannels, "stereo");
        assertEquals(fmt.nSampleRate, ab.nSampleRate, "sample rate");
        assertEquals(chan_framesize, ab.nSamples, "frame size matches");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme next voice audioblock");

        AudioBlock ab2 = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
        assertTrue(ab2 != null, "retrieve next audioblock");

        assertEquals(fmt.nSampleRate, ab2.nSampleRate, "same sample rate");
        assertEquals(ab.uSampleIndex + chan_framesize, ab2.uSampleIndex, "sample counter matches");

        assertTrue(ttclient.enableAudioBlockEvent(Constants.TT_LOCAL_USERID, StreamType.STREAMTYPE_VOICE, null, false), "disable local userid");
        while (ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_LOCAL_USERID) != null);

        // now do same test for TT_MUXED_USERID
        assertTrue(ttclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, fmt, true), "enable aud cb for muxed");
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme voice audioblock from muxed");
        assertEquals(Constants.TT_MUXED_USERID, msg.nSource, "from muxed user");

        ab = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
        assertTrue(ab != null, "retrieve audioblock from muxed");

        assertEquals(fmt.nChannels, ab.nChannels, "stereo in muxed");
        assertEquals(fmt.nSampleRate, ab.nSampleRate, "sample rate in muxed");
        assertEquals(mux_framesize, ab.nSamples, "frame size matches in muxed");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme next voice audioblock from muxed");
        ab2 = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
        assertTrue(ab2 != null, "retrieve next audioblock from muxed");

        assertEquals(fmt.nSampleRate, ab2.nSampleRate, "same sample rate for muxed");
        assertEquals(ab.uSampleIndex + mux_framesize, ab2.uSampleIndex, "sample counter matches for muxed");
    }

    @Test
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

        assertTrue(rxclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, true), "enable aud cb");
        assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme voice audioblock");
        assertTrue(txclient.enableVoiceTransmission(true), "enable tx");
        assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "User state changed to voice");
        assertTrue((msg.user.uUserState & UserState.USERSTATE_VOICE) == UserState.USERSTATE_VOICE, "User is talking");
        assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme voice audioblock again");
        assertTrue(txclient.enableVoiceTransmission(false), "disable tx");
        assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "User state changed to no voice");
        assertTrue((msg.user.uUserState & UserState.USERSTATE_VOICE) == UserState.USERSTATE_NONE, "User stopped talking");
    }

    @Test
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

        assertTrue(ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 500), "Gen tone");

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
            assertEquals(Codec.OPUS_CODEC, chan.audiocodec.nCodec);
            chan.audiocodec.opus.nChannels = 2;

            assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join with interval "+chan.audiocodec.opus.nTxIntervalMSec+" framesize " + frameMSec);

            Channel newchan = new Channel();
            assertTrue(ttclient.getChannel(ttclient.getMyChannelID(), newchan), "get new chan");
            assertEquals(chan.audiocodec.opus.nTxIntervalMSec, newchan.audiocodec.opus.nTxIntervalMSec, "same tx interval");
            assertEquals(chan.audiocodec.opus.nFrameSizeMSec, newchan.audiocodec.opus.nFrameSizeMSec, "same frame size");

            assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT), "subscribe");

            assertTrue(ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true), "enable aud cb");

            assertTrue(ttclient.enableVoiceTransmission(true), "vox");

            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme voice audioblock");

            AudioBlock block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID());

            assertEquals((int)(chan.audiocodec.opus.nSampleRate * (chan.audiocodec.opus.nTxIntervalMSec / 1000.)), block.nSamples, "aud block has " + chan.audiocodec.opus.nTxIntervalMSec + " msec samples");

            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 2000, msg);

            assertTrue(ttclient.enableVoiceTransmission(false), "vox disable");

            assertTrue(waitCmdSuccess(ttclient, ttclient.doLeaveChannel(), DEF_WAIT), "leave");

            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 0, msg);

            assertTrue(ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, false), "disable aud cb");
        }
    }

    @Test
    public void testListAccounts() {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, ADMIN_NICKNAME + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        TTMessage msg = new TTMessage();

        assertTrue(ttclient.doListUserAccounts(0, 100)>0);
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_USERACCOUNT, DEF_WAIT, msg));
        UserAccount account = msg.useraccount;
        assertTrue(account.uUserType != UserType.USERTYPE_NONE);
    }

    @Test
    public void testListBannedUsers() {

        TeamTalkBase ttadmin = newClientInstance();
        connect(ttadmin);
        login(ttadmin, ADMIN_NICKNAME + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        User user = new User();
        assertTrue(ttadmin.getUser(ttadmin.getMyUserID(), user), "get self");
        String IPADDR = "10.2.3.4";
        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doBanUser(ttadmin.getMyUserID(), 0), DEF_WAIT), "wait ban");
        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doBanIPAddress(IPADDR, 0), DEF_WAIT), "wait ip ban");

        TTMessage msg = new TTMessage();

        assertTrue(ttadmin.doListBans(0, 0, 100)>0, "list bans");
        assertTrue(waitForEvent(ttadmin, ClientEvent.CLIENTEVENT_CMD_BANNEDUSER, DEF_WAIT, msg), "wait ban list");
        BannedUser ban = msg.banneduser;
        assertTrue(ban.szIPAddress.length()>0);

        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doUnBanUser(user.szIPAddress, 0), DEF_WAIT), "unban user IP");
        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doUnBanUser(IPADDR, 0), DEF_WAIT), "unban specified IP");

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_MULTI_LOGIN;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        // ban user (ttadmin) by username from channel

        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        Channel chan = buildDefaultChannel(ttadmin, "BanTest");
        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join new channel");

        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT), "admin join (chan/username)");

        assertTrue(waitCmdSuccess(ttclient, ttclient.doBanUserEx(ttadmin.getMyUserID(), BanType.BANTYPE_CHANNEL | BanType.BANTYPE_USERNAME), DEF_WAIT), "ban admin by chan/username");

        assertTrue(waitCmdSuccess(ttclient, ttclient.doKickUser(ttadmin.getMyUserID(), ttclient.getMyChannelID()), DEF_WAIT), "kick admin");

        assertTrue(ttclient.doListBans(ttclient.getMyChannelID(), 0, 100)>0, "list bans");
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_BANNEDUSER, DEF_WAIT, msg), "wait ban list");
        ban = msg.banneduser;
        assertTrue(ban.szIPAddress.length()>0, "ip set");
        assertEquals(BanType.BANTYPE_CHANNEL | BanType.BANTYPE_USERNAME, ban.uBanTypes, "Ban type same");

        assertTrue(waitCmdError(ttadmin, ttadmin.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT), "admin cannot join");

        joinRoot(ttadmin);

        // clear ban and join again

        TeamTalkBase ttclient2 = newClientInstance();
        connect(ttclient2);
        login(ttclient2, NICKNAME, USERNAME, PASSWORD);

        assertTrue(waitCmdSuccess(ttclient2, ttclient2.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT), "ttclient2 join");

        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doLeaveChannel(), DEF_WAIT), "admin leave");
        assertTrue(waitCmdError(ttadmin, ttadmin.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT), "admin join denied");

        assertTrue(waitCmdSuccess(ttclient, ttclient.doUnBanUserEx(ban), DEF_WAIT), "unban");

        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT), "admin join (IP-ban)");

        assertTrue(waitCmdSuccess(ttclient, ttclient.doBan(ban), DEF_WAIT), "ban admin");

        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doLeaveChannel(), DEF_WAIT), "admin leave");
        assertTrue(waitCmdError(ttadmin, ttadmin.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT), "admin join denied (IP-ban)");

        assertTrue(waitCmdSuccess(ttclient, ttclient.doUnBanUserEx(ban), DEF_WAIT), "unban");

        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT), "admin join (chan/IP-address)");

        assertTrue(waitCmdSuccess(ttclient, ttclient.doBanUserEx(ttadmin.getMyUserID(), BanType.BANTYPE_CHANNEL | BanType.BANTYPE_IPADDR), DEF_WAIT), "ban admin by chan/IP-address");

        assertTrue(waitCmdSuccess(ttclient, ttclient.doKickUser(ttadmin.getMyUserID(), ttclient.getMyChannelID()), DEF_WAIT), "kick admin");

        assertTrue(waitCmdError(ttadmin, ttadmin.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT), "admin join denied");

        // ensure no duplicate bans
        chan = buildDefaultChannel(ttclient, "BanTest1");
        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join new channel");

        assertTrue(waitCmdSuccess(ttclient2, ttclient2.doJoinChannelByID(ttclient.getMyChannelID(), ""), DEF_WAIT), "ttclient2 join BanTest1");

        assertTrue(waitCmdSuccess(ttclient, ttclient.doBanUser(ttclient2.getMyUserID(), ttclient.getMyChannelID()), DEF_WAIT), "ban client2 once");
        assertTrue(waitCmdSuccess(ttclient, ttclient.doBanUser(ttclient2.getMyUserID(), ttclient.getMyChannelID()), DEF_WAIT), "ban client2 twice");

        assertTrue(ttclient.doListBans(ttclient.getMyChannelID(), 0, 100)>0, "list bans (expect 1)");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_BANNEDUSER, DEF_WAIT, msg), "wait ban list");
        UserAccount account = new UserAccount();
        assertTrue(ttclient.getMyUserAccount(account), "get my account");
        assertEquals(account.szUsername, msg.banneduser.szOwner, "owner set");
        msg = new TTMessage();
        assertTrue(ttclient.getMessage(msg, DEF_WAIT), "wait done msg");
        assertEquals(ClientEvent.CLIENTEVENT_CMD_SUCCESS, msg.nClientEvent, "done msg, only one ban expected");

    }

    @Test
    public void testBanNoChannel() {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_MULTI_LOGIN |
            UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient1 = newClientInstance();
        TeamTalkBase ttclient2 = newClientInstance();

        connect(ttclient1);
        login(ttclient1, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient1);

        connect(ttclient2);
        login(ttclient2, NICKNAME, USERNAME, PASSWORD);

        assertTrue(waitCmdError(ttclient1, ttclient1.doBanUserEx(ttclient2.getMyUserID(), BanType.BANTYPE_CHANNEL), DEF_WAIT), "ban user by channel");
    }

    @Test
    public void testBanUserOffline() {

        String NICKNAME = "jUnit - " + getTestMethodName();
        TeamTalkBase client1 = newClientInstance();
        connect(client1);
        login(client1, NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);

        BannedUser ban = new BannedUser();
        ban.uBanTypes = BanType.BANTYPE_IPADDR;
        ban.szNickname = "Foo";
        int cmdid = client1.doBan(ban);
        assertTrue(waitCmdSuccess(client1, cmdid, DEF_WAIT), "ban");

        TTMessage msg = new TTMessage();
        assertTrue(client1.doListBans(0, 0, 1) > 0, "list bans");
        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_BANNEDUSER, DEF_WAIT, msg), "wait ban list");
        assertEquals(ban.szNickname, msg.banneduser.szNickname, "nickname set");

        cmdid = client1.doUnBanUserEx(msg.banneduser);
        assertTrue(waitCmdSuccess(client1, cmdid, DEF_WAIT), "unban");
    }

    @Test
    public void testBanInvalidRegEx() {

        String NICKNAME = "jUnit - " + getTestMethodName();
        TeamTalkBase client1 = newClientInstance();
        connect(client1);
        login(client1, NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);

        BannedUser ban = new BannedUser();
        ban.uBanTypes = BanType.BANTYPE_IPADDR;
        ban.szNickname = "Foo";
        ban.szIPAddress = "*.";
        int cmdid = client1.doBan(ban);
        assertTrue(waitCmdSuccess(client1, cmdid, DEF_WAIT), "ban");

        TeamTalkBase client2 = newClientInstance();
        connect(client2);
        login(client2, NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);

        cmdid = client1.doUnBanUserEx(ban);
        assertTrue(waitCmdSuccess(client1, cmdid, DEF_WAIT), "unban");
    }

    @Test
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
        assertTrue(ttclient.enableVoiceTransmission(true), "enable tx");
        assertTrue(ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true), "enable audioblock");

        int[] opus_samplerates = {8000, 12000, 16000, 24000, 48000};

        for(int sr : opus_samplerates) {

            Channel chan;
            chan = buildDefaultChannel(ttclient, "Opus_" + sr);
            assertEquals(Codec.OPUS_CODEC, chan.audiocodec.nCodec, "OPUS enabled");
            chan.audiocodec.opus.nSampleRate = sr;
            assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join channel");


            for(int j=0;j<1000 / chan.audiocodec.opus.nTxIntervalMSec;j++) {
                assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT), "get audioblock " + j);
                AudioBlock audblk = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID());
                assertTrue(audblk.nStreamID>0, "Stream ID is set");
                assertEquals(sr, audblk.nSampleRate, "Sample rate as channel");
            }

            assertTrue(waitCmdSuccess(ttclient, ttclient.doLeaveChannel(), DEF_WAIT), "leave channel");

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

    @Test
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

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join chan success");

        assertTrue(ttclient.getChannel(ttclient.getMyChannelID(), chan), "get channel");

        while (ttclient.pumpMessage(ClientEvent.CLIENTEVENT_USER_STATECHANGE, ttclient.getMyUserID())) {
            int nextuserdata = ++chan.nUserData;
            assertTrue(ttclient.doUpdateChannel(chan) >= 0, "update channel");
            while (ttclient.getChannel(ttclient.getMyChannelID(), chan) && chan.nUserData != nextuserdata) {
                if (!ttclient.pumpMessage(ClientEvent.CLIENTEVENT_USER_STATECHANGE, ttclient.getMyUserID()))
                    break;
                Thread.sleep(0, 1);
            }
        }

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_INTERNAL_ERROR, DEF_WAIT, msg), "Internal error");
        assertTrue(msg.clienterrormsg.nErrorNo == ClientError.INTERR_TTMESSAGE_QUEUE_OVERFLOW, "Queue overflow");

        int cmdid = ttclient.doLeaveChannel();
        assertTrue(cmdid>0, "Update again after overflow");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_USER_LEFT, DEF_WAIT, msg), "Expect user-left message");
        assertEquals(ttclient.getMyUserID(), msg.user.nUserID, "message queue lives again");

    }

    @Test
    public void testAudioBlockSimStereo() throws IOException {
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

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

        assertTrue(ttclient.enableVoiceTransmission(true));

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000), "no voice audioblock");
        assertTrue(ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 440));
        assertTrue(ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true));

        try(FileOutputStream fs = newWaveFile(STORAGEFOLDER + File.separator + "MyWaveFile.wav", SAMPLERATE, CHANNELS, WRITE_BYTES);) {

            int c = 0;
            while(WRITE_BYTES > 0) {
                assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme voice audioblock #" + (c++));

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

    @Test
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
            assertTrue(ttclient.getDefaultSoundDevices(indev, outdev), "get default sound devices");

        if(OUTPUTDEVICEID>=0)
            outdev.value = OUTPUTDEVICEID;

        assertTrue(ttclient.initSoundInputDevice(indev.value), "init input dev (we skip output device for now)");

        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        int WRITE_BYTES = 256000, v, CHANNELS = 1, SAMPLERATE = 16000;

        Channel chan = buildDefaultChannel(ttclient, "Speex", Codec.SPEEX_CODEC);
        chan.audiocodec.speex.nBandmode = SpeexConstants.SPEEX_BANDMODE_WIDE; //16000

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT));

        assertTrue(ttclient.enableVoiceTransmission(true));

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000), "no voice audioblock");

        assertTrue(ttclient.enableAudioBlockEvent(Constants.TT_LOCAL_USERID, StreamType.STREAMTYPE_VOICE, true), "pass 0 user id as MYSELF");

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

            int n_ab = 0;
            while(WRITE_BYTES > 0) {
                assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme voice audioblock #" + (n_ab++));

                AudioBlock block = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, 0);
                assertTrue(block != null, "audio block is valid");
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

        assertTrue(ttclient.enableAudioBlockEvent(Constants.TT_LOCAL_USERID, StreamType.STREAMTYPE_VOICE, false), "disable callback for MYSELF");

        assertTrue(ttclient.enableVoiceTransmission(false), "disable voice now that we have the wav-file");

        assertTrue(ttclient.initSoundOutputDevice(outdev.value), "init output dev, so we can hear recorded wavfile");

        assertTrue(ttclient.startStreamingMediaFileToChannel(wavefilePath, new VideoCodec()), "Stream MyWaveFile.wav");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg), "get initial streaming event");

        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "Stream started");

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg)) {
            if (msg.mediafileinfo.nStatus == MediaFileStatus.MFS_FINISHED)
                break;
        }

        assertEquals(MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus, "Stream ended");
    }

    @Test
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
        assertTrue(ttclient.setSoundInputPreprocess(spxdsp), "SpeexDSP");

        TTMessage msg = new TTMessage();

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        Channel chan = new Channel();
        assertTrue(ttclient.getChannel(ttclient.getRootChannelID(), chan), "get root channel");
        assertEquals(Codec.OPUS_CODEC, chan.audiocodec.nCodec, "opus set");

        if (!supportsDuplexMode(ttclient, INPUTDEVICEID, OUTPUTDEVICEID, chan.audiocodec.opus.nSampleRate)) {
            System.err.println("Duplex tests skipped due to no shared sample rate");
            return;
        }

        int cmdid = ttclient.doJoinChannelByID(ttclient.getRootChannelID(), "");
        assertTrue(cmdid>0, "issued cmd");
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_PROCESSING, DEF_WAIT);

        do {
            assertTrue(ttclient.getMessage(msg, DEF_WAIT), "get event");
            switch (msg.nClientEvent) {
            case ClientEvent.CLIENTEVENT_INTERNAL_ERROR :
                assertTrue(msg.clienterrormsg.nErrorNo != ClientError.INTERR_SNDINPUT_FAILURE, "Sound input failure");
                assertTrue(msg.clienterrormsg.nErrorNo != ClientError.INTERR_SNDOUTPUT_FAILURE, "Sound output failure");
                assertTrue(msg.clienterrormsg.nErrorNo != ClientError.INTERR_AUDIOPREPROCESSOR_INIT_FAILED, "Preprocessor failure");
                break;
            }
        } while (msg.nClientEvent != ClientEvent.CLIENTEVENT_CMD_PROCESSING);
    }

    @Test
    public void testSoundInputAudioPreprocessor() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_VIEW_ALL_USERS |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_TRANSMIT_VOICE;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);
        assertTrue(ttclient.enableVoiceTransmission(true), "enable tx");

        AudioPreprocessor preprocess = new AudioPreprocessor();
        assertTrue(ttclient.getSoundInputPreprocess(preprocess), "get default preprocessor");
        assertEquals(AudioPreprocessorType.TEAMTALK_AUDIOPREPROCESSOR, preprocess.nPreprocessor, "TTAudioPreprocessor is default");

        assertEquals(preprocess.ttpreprocessor.nGainLevel, ttclient.getSoundInputGainLevel(), "gain compatible");
        assertEquals(SoundLevel.SOUND_GAIN_DEFAULT, preprocess.ttpreprocessor.nGainLevel, "gain default set");

        assertTrue(ttclient.setSoundInputGainLevel(SoundLevel.SOUND_GAIN_MAX), "Set gain level");
        assertTrue(ttclient.getSoundInputPreprocess(preprocess), "get updated AP preprocessor");
        assertEquals(SoundLevel.SOUND_GAIN_MAX, preprocess.ttpreprocessor.nGainLevel, "gain levelfrom AP is max");
        assertEquals(SoundLevel.SOUND_GAIN_MAX, ttclient.getSoundInputGainLevel(), "gain level is max");

        // test SpeexDSP
        preprocess = new AudioPreprocessor();
        preprocess.nPreprocessor = AudioPreprocessorType.SPEEXDSP_AUDIOPREPROCESSOR;
        preprocess.speexdsp.nGainLevel = 7777;
        assertTrue(ttclient.setSoundInputPreprocess(preprocess), "Enable SpeexDSP");
        SpeexDSP speexdsp = new SpeexDSP();
        assertTrue(ttclient.getSoundInputPreprocess(speexdsp), "Get SpeexDSP");
        assertEquals(7777, speexdsp.nGainLevel, "SpeexDSP and AudioPreprocessor are equals");
        assertTrue(ttclient.getSoundInputPreprocess(preprocess), "get updated AP with SpeexDSP preprocessor");
        assertEquals(7777, preprocess.speexdsp.nGainLevel, "SpeexDSP and AudioPreprocessor are equals");
        assertEquals(SoundLevel.SOUND_GAIN_MAX, ttclient.getSoundInputGainLevel(), "gain level not changed by SpeexDSP");

        Channel chan = buildDefaultChannel(ttclient, "Opus - Test", Codec.OPUS_CODEC);
        chan.audiocodec.opus.nChannels = 2;
        chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
        chan.audiocodec.opus.bDTX = false;
        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join");

        assertTrue(ttclient.getSoundInputPreprocess(preprocess), "get updated AP with SpeexDSP preprocessor after join");
        assertEquals(7777, preprocess.speexdsp.nGainLevel, "SpeexDSP and AudioPreprocessor are still equal");

        SpeexDSP spxdsp = new SpeexDSP(SPEEXDSP_AVAILABLE), spxdsp2 = new SpeexDSP();
        assertTrue(ttclient.setSoundInputPreprocess(spxdsp), "set Speex DSP");

        assertTrue(ttclient.getSoundInputPreprocess(spxdsp2), "get Speex DSP");
        assertEquals(spxdsp.bEnableAGC, spxdsp2.bEnableAGC, "agc1");
        assertEquals(spxdsp.nGainLevel, spxdsp2.nGainLevel, "agc2");
        assertEquals(spxdsp.nMaxIncDBSec, spxdsp2.nMaxIncDBSec, "agc3");
        assertEquals(spxdsp.nMaxDecDBSec, spxdsp2.nMaxDecDBSec, "agc4");
        assertEquals(spxdsp.nMaxGainDB, spxdsp2.nMaxGainDB, "agc5");
        assertEquals(spxdsp.bEnableDenoise, spxdsp2.bEnableDenoise, "agc6");
        assertEquals(spxdsp.nMaxNoiseSuppressDB, spxdsp2.nMaxNoiseSuppressDB, "agc7");
        assertEquals(spxdsp.nEchoSuppress, spxdsp2.nEchoSuppress, "agc8");
        assertEquals(spxdsp.nEchoSuppressActive, spxdsp2.nEchoSuppressActive, "agc9");

        // test WebRTC
        preprocess = new AudioPreprocessor();
        preprocess.nPreprocessor = AudioPreprocessorType.WEBRTC_AUDIOPREPROCESSOR;
        preprocess.webrtc.preamplifier.bEnable = true;
        preprocess.webrtc.preamplifier.fFixedGainFactor = 5;
        preprocess.webrtc.echocanceller.bEnable = false;
        preprocess.webrtc.gaincontroller2.bEnable = true;
        preprocess.webrtc.gaincontroller2.fixeddigital.fGainDB = 4;
        preprocess.webrtc.gaincontroller2.adaptivedigital.bEnable = true;
        preprocess.webrtc.gaincontroller2.adaptivedigital.fHeadRoomDB = 5;
        preprocess.webrtc.gaincontroller2.adaptivedigital.fMaxGainDB = 6;
        preprocess.webrtc.gaincontroller2.adaptivedigital.fInitialGainDB = 7;
        preprocess.webrtc.gaincontroller2.adaptivedigital.fMaxGainChangeDBPerSecond = 8;
        preprocess.webrtc.gaincontroller2.adaptivedigital.fMaxOutputNoiseLevelDBFS = 9;
        preprocess.webrtc.noisesuppression.bEnable = true;
        preprocess.webrtc.noisesuppression.nLevel = 2;

        if (WEBRTC_AVAILABLE) {
            assertTrue(ttclient.setSoundInputPreprocess(preprocess), "Enable WebRTC");
            AudioPreprocessor preprocess2 = new AudioPreprocessor();
            assertTrue(ttclient.getSoundInputPreprocess(preprocess2), "get WebRTC");

            assertEquals(preprocess.webrtc.echocanceller.bEnable, preprocess2.webrtc.echocanceller.bEnable, "webrtc0");

            assertEquals(preprocess.webrtc.gaincontroller2.bEnable, preprocess2.webrtc.gaincontroller2.bEnable, "webrtc1");
            assertEquals((int)preprocess.webrtc.gaincontroller2.fixeddigital.fGainDB, (int)preprocess2.webrtc.gaincontroller2.fixeddigital.fGainDB, "webrtc2");

            assertEquals(preprocess.webrtc.gaincontroller2.adaptivedigital.bEnable, preprocess2.webrtc.gaincontroller2.adaptivedigital.bEnable, "webrtc3");
            assertEquals((int)preprocess.webrtc.gaincontroller2.adaptivedigital.fHeadRoomDB, (int)preprocess2.webrtc.gaincontroller2.adaptivedigital.fHeadRoomDB, "webrtc4");
            assertEquals((int)preprocess.webrtc.gaincontroller2.adaptivedigital.fMaxGainDB, (int)preprocess2.webrtc.gaincontroller2.adaptivedigital.fMaxGainDB, "webrtc5");
            assertEquals((int)preprocess.webrtc.gaincontroller2.adaptivedigital.fInitialGainDB, (int)preprocess2.webrtc.gaincontroller2.adaptivedigital.fInitialGainDB, "webrtc6");
            assertEquals((int)preprocess.webrtc.gaincontroller2.adaptivedigital.fMaxGainChangeDBPerSecond, (int)preprocess2.webrtc.gaincontroller2.adaptivedigital.fMaxGainChangeDBPerSecond, "webrtc7");
            assertEquals((int)preprocess.webrtc.gaincontroller2.adaptivedigital.fMaxOutputNoiseLevelDBFS, (int)preprocess2.webrtc.gaincontroller2.adaptivedigital.fMaxOutputNoiseLevelDBFS, "webrtc8");

            assertEquals(preprocess.webrtc.noisesuppression.bEnable, preprocess2.webrtc.noisesuppression.bEnable, "webrtc9");
            assertEquals(preprocess.webrtc.noisesuppression.nLevel, preprocess2.webrtc.noisesuppression.nLevel, "webrtc10");
            assertEquals(preprocess.webrtc.preamplifier.bEnable, preprocess2.webrtc.preamplifier.bEnable, "webrtc11");
            assertEquals((int)preprocess.webrtc.preamplifier.fFixedGainFactor, (int)preprocess2.webrtc.preamplifier.fFixedGainFactor, "webrtc12");
        }
    }

    @Test
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
        assertTrue(ttclient.setSoundInputPreprocess(new SpeexDSP()), "input preprocess default");

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        Channel chan = new Channel();
        ttclient.getChannel(ttclient.getMyChannelID(), chan);

        assertEquals(Codec.OPUS_CODEC, chan.audiocodec.nCodec, "OPUS codec running");

        ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq);

        assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(),
                                                                                Subscription.SUBSCRIBE_VOICE), DEF_WAIT), "wait cmd ok");

        assertTrue(ttclient.setUserMediaStorageDir(ttclient.getMyUserID(),
                                                                            STORAGEFOLDER, "%username%_%counter%",
                                                                            AudioFileFormat.AFF_CHANNELCODEC_FORMAT), "specify audio storage");

        assertTrue(ttclient.enableVoiceTransmission(true), "enable voice tx");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg), "audio file created");

        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "recording started");
        assertEquals(STORAGEFOLDER + File.separator + USERNAME + "_" + "000000001.ogg", msg.mediafileinfo.szFileName, "correct filename");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient.enableVoiceTransmission(false);

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg), "audio file stopped");
        assertEquals(MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus, "recording started");

        assertTrue(ttclient.enableVoiceTransmission(true), "enable voice tx 2 ");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg), "audio file created 2");

        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "recording started 2");
        assertEquals(STORAGEFOLDER + File.separator + USERNAME + "_" + "000000002.ogg", msg.mediafileinfo.szFileName, "correct filename 2");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient.enableVoiceTransmission(false);

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg), "audio file stopped 2");
        assertEquals(MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus, "recording started 2");

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

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join speex channel");

        assertTrue(ttclient.enableVoiceTransmission(true), "enable voice tx 3");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg), "audio file created 3");

        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "recording started 3");
        assertEquals(STORAGEFOLDER + File.separator + USERNAME + "_" + "000000003.ogg", msg.mediafileinfo.szFileName, "correct filename 3");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient.enableVoiceTransmission(false);

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg), "audio file stopped 3");
        assertEquals(MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus, "recording started 3");


        assertTrue(ttclient.enableVoiceTransmission(true), "enable voice tx 4");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg), "audio file created 4");

        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "recording started 4");
        assertEquals(STORAGEFOLDER + File.separator + USERNAME + "_" + "000000004.ogg", msg.mediafileinfo.szFileName, "correct filename 4");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 10000);

        ttclient.enableVoiceTransmission(false);

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg), "audio file stopped 4");
        assertEquals(MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus, "recording started 4");

    }

    @Test
    public void testMediaStorageCompletion() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_MULTI_LOGIN |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_VIEW_ALL_USERS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase ttclient1, ttclient2;

        ttclient1 = newClientInstance();
        ttclient2 = newClientInstance();

        initSound(ttclient2);
        connect(ttclient2);
        login(ttclient2, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient2);

        initSound(ttclient1);
        connect(ttclient1);
        login(ttclient1, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient1);

        assertTrue(ttclient1.setUserMediaStorageDir(ttclient2.getMyUserID(),
                                                                            STORAGEFOLDER, "%username%_%counter%",
                                                                            AudioFileFormat.AFF_CHANNELCODEC_FORMAT), "specify audio storage");

        // test audio recording start/finished
        TTMessage msg = new TTMessage();
        assertTrue(ttclient2.enableVoiceTransmission(true), "enable voice tx");
        assertTrue(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg), "audio file created");
        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "recording started");
        assertTrue(ttclient2.enableVoiceTransmission(false), "disable voice tx");
        assertTrue(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg), "audio file done");
        assertEquals(MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus, "recording ended");

        // test audio recording start/finished when leaving channel
        assertTrue(ttclient2.enableVoiceTransmission(true), "enable voice tx 2");
        assertTrue(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg), "audio file created 2");
        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "recording started 2");
        Channel chan = buildDefaultChannel(ttclient2, "Opus");
        assertTrue(waitCmdSuccess(ttclient2, ttclient2.doJoinChannel(chan), DEF_WAIT), "join");
        assertTrue(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg), "audio file done 2");
        assertEquals(MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus, "recording ended 2");

        // test audio recording start/finished when resetting storage folder
        joinRoot(ttclient2);
        assertTrue(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg), "audio file created 3");
        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "recording started 3");
        assertTrue(ttclient1.setUserMediaStorageDir(ttclient2.getMyUserID(),
                                                                               STORAGEFOLDER, "",
                                                                               AudioFileFormat.AFF_NONE), "specify audio storage 3");
        assertTrue(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, DEF_WAIT, msg), "audio file done 3");
        assertEquals(MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus, "recording ended 3");
    }

    @Test
    public void testSoundLoopback() {
        TeamTalkBase ttclient;

        ttclient = newClientInstance();
        IntPtr in = new IntPtr(), out = new IntPtr();
        if(INPUTDEVICEID<0 || OUTPUTDEVICEID<0)
            assertTrue(TeamTalkBase.getDefaultSoundDevices(in, out), "Get default sound devices");
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
                assertEquals(SoundDeviceConstants.TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL, d.nDeviceID, "Virtual TeamTalk device");
            }
            if (d.nDeviceID == out.value)
                outdev = d;
        }

        long loop = ttclient.startSoundLoopbackTest(in.value, out.value, 48000, 1, false, null);
        assertTrue(loop>0, "Sound loopback started");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient.closeSoundLoopbackTest(loop), "Loop stopped");

        loop = ttclient.startSoundLoopbackTest(nodev.nDeviceID, out.value, 48000, 1, false, null);
        assertTrue(loop>0, "Sound loopback virtual input-dev started");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient.closeSoundLoopbackTest(loop), "Loop virtual input-dev stopped");

        loop = ttclient.startSoundLoopbackTest(in.value, nodev.nDeviceID, 48000, 2, false, null);
        assertTrue(loop>0, "Sound loopback virtual output-dev started");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient.closeSoundLoopbackTest(loop), "Loop virtual output-dev stopped");

        loop = ttclient.startSoundLoopbackTest(nodev.nDeviceID, nodev.nDeviceID, 48000, 2, true, new SpeexDSP(true));
        assertTrue(loop>0, "Sound loopback virtual duplex-dev started with SpeexDSP");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient.closeSoundLoopbackTest(loop), "Loop virtual duplex-dev stopped");

        AudioPreprocessor preprocessor = new AudioPreprocessor();
        preprocessor.nPreprocessor = AudioPreprocessorType.TEAMTALK_AUDIOPREPROCESSOR;
        preprocessor.ttpreprocessor.nGainLevel = 4000;
        preprocessor.ttpreprocessor.bMuteLeftSpeaker = true;

        SoundDeviceEffects effects = new SoundDeviceEffects();

        loop = ttclient.startSoundLoopbackTest(in.value, out.value, 48000, 2, false, preprocessor, effects);
        assertTrue(loop>0, "Sound loopback effects started");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient.closeSoundLoopbackTest(loop), "Loop effects stopped");

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
        assertTrue(loop>0, "Sound loopback AudioPreprocessor started");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient.closeSoundLoopbackTest(loop), "Loop Audio Preprocessor stopped");

        loop = ttclient.startSoundLoopbackTest(in.value, out.value, SAMPLERATE, 1, true, new SpeexDSP(true));
        assertTrue(loop>0, "Sound duplex loopback started");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

        assertTrue(ttclient.closeSoundLoopbackTest(loop), "Loop duplex stopped");

        if (out.value == SoundDeviceConstants.TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL) {
            System.err.println("Duplex test skipped due to virtual sound device as output");
        }
        else {
            loop = ttclient.startSoundLoopbackTest(nodev.nDeviceID, out.value, SAMPLERATE, 1, true, new SpeexDSP(true));
            assertTrue(loop<=0, "Sound loopback virtual duplex-dev cannot be mixed with real dev");
        }

        if (in.value == SoundDeviceConstants.TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL) {
            System.err.println("Duplex test skipped due to virtual sound device as input");
        }
        else {
            loop = ttclient.startSoundLoopbackTest(in.value, nodev.nDeviceID, SAMPLERATE, 1, true, new SpeexDSP(true));
            assertTrue(loop<=0, "Sound loopback virtual duplex-dev cannot be mixed with real dev");
        }
    }

    @Test
    public void testSharedSoundDevice() {

        TeamTalkBase ttclient1 = newClientInstance();
        TeamTalkBase ttclient2 = newClientInstance();
        TeamTalkBase ttclient3 = newClientInstance();

        int inputdeviceid = INPUTDEVICEID;
        int outputdeviceid = OUTPUTDEVICEID;

        IntPtr indev = new IntPtr(), outdev = new IntPtr();
        boolean gotdevs = ttclient1.getDefaultSoundDevices(indev, outdev);
        if (INPUTDEVICEID >= 0 && OUTPUTDEVICEID >= 0) {
            indev.value = INPUTDEVICEID;
            outdev.value = OUTPUTDEVICEID;
        }

        // cannot assert since test system might not have a sound input or output device.
        //assertTrue(ttclient1.getDefaultSoundDevices(indev, outdev), "get default devs");

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

        assertTrue(sharedindev != null, "shared in device exists");
        assertEquals(inputdeviceid, sharedindev.nDeviceID, "shared in device selected");

        assertTrue(sharedoutdev != null, "shared out device exists");
        assertEquals(outputdeviceid, sharedoutdev.nDeviceID, "shared out device selected");

        // toggle input and output device as shared audio devices
        inputdeviceid = sharedindev.nDeviceID | SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;
        outputdeviceid = sharedoutdev.nDeviceID | SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;

        // test two instances with same sample settings as original and one instance which requires resampling
        long sndloop1 = ttclient1.startSoundLoopbackTest(inputdeviceid, outputdeviceid, 48000, 2, false, null);
        assertTrue(sndloop1 != 0, "Start sound loop 1");
        long sndloop2 = ttclient2.startSoundLoopbackTest(inputdeviceid, outputdeviceid, 48000, 2, false, null);
        assertTrue(sndloop2 != 0, "Start sound loop 2");
        long sndloop3 = ttclient3.startSoundLoopbackTest(inputdeviceid, outputdeviceid, 48000, 1, false, null);
        assertTrue(sndloop3 != 0, "Start sound loop 3");

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue(ttclient1.closeSoundLoopbackTest(sndloop1), "Close sndloop1");
        assertTrue(ttclient2.closeSoundLoopbackTest(sndloop2), "Close sndloop2");
        assertTrue(ttclient3.closeSoundLoopbackTest(sndloop3), "Close sndloop3");

        // test two instances which require resampling
        long sndloop4 = ttclient1.startSoundLoopbackTest(inputdeviceid, outputdeviceid, 32000, 1, false, null);
        assertTrue(sndloop4 != 0, "Start sound loop 4");
        long sndloop5 = ttclient2.startSoundLoopbackTest(inputdeviceid, outputdeviceid, 44100, 2, false, null);
        assertTrue(sndloop5 != 0, "Start sound loop 5");

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue(ttclient1.closeSoundLoopbackTest(sndloop4), "Close sndloop4");
        assertTrue(ttclient2.closeSoundLoopbackTest(sndloop5), "Close sndloop5");

        Vector<Long> sndloops = new Vector<>();
        // now go through all sample rates
        for(int samplerate : sharedindev.inputSampleRates) {
            if (samplerate <= 0)
                continue;

            long sndloop = ttclient1.startSoundLoopbackTest(inputdeviceid, outputdeviceid, samplerate, 1, false, null);
            assertTrue(sndloop != 0, "Start sound loop at " + samplerate + " channels " + 1);
            sndloops.add(sndloop);
        }

        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_NONE, 5000);

        for(long sndloop : sndloops) {
            assertTrue(ttclient1.closeSoundLoopbackTest(sndloop), "Close sndloop");
        }
    }


    @Test
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

        assertTrue(ttvirt.initSoundInputDevice(SoundDeviceConstants.TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL), "Init virtual input dev");
        assertTrue(ttvirt.initSoundOutputDevice(SoundDeviceConstants.TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL), "Init virtual output dev");
        connect(ttvirt);
        login(ttvirt, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttvirt);

        assertTrue(ttvirt.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 300), "Gen tone");
        assertTrue(ttvirt.enableVoiceTransmission(true), "Enable virtual voice transmission");
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "Wait for talking event");
        assertEquals(UserState.USERSTATE_VOICE, msg.user.uUserState & UserState.USERSTATE_VOICE, "User state to voice");
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);

        assertTrue(ttvirt.enableVoiceTransmission(false), "Disable voice transmission");
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "Wait for talking event");
        assertEquals(0, msg.user.uUserState & UserState.USERSTATE_VOICE, "User state to voice");


        assertTrue(ttclient.enableVoiceTransmission(true), "Enable real voice transmission");
        assertTrue(waitForEvent(ttvirt, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "Wait for talking event");
        assertEquals(UserState.USERSTATE_VOICE, msg.user.uUserState & UserState.USERSTATE_VOICE, "User state to voice");
        waitForEvent(ttvirt, ClientEvent.CLIENTEVENT_NONE, 5000);
    }

    @Test
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
        assertEquals(Codec.OPUS_CODEC, chan.audiocodec.nCodec, "opus default");
        chan.uChannelType |= ChannelType.CHANNEL_SOLO_TRANSMIT;
        chan.nTransmitUsersQueueDelayMSec = 500;

        assertTrue(waitCmdSuccess(ttclient1, ttclient1.doJoinChannel(chan), DEF_WAIT), "join");

        assertTrue(ttclient1.getChannel(ttclient1.getMyChannelID(), chan), "Channel id set");

        for(int u : chan.transmitUsersQueue)
            assertEquals(0, u, "no users in queue");

        assertTrue(waitCmdSuccess(ttclient1, ttclient1.doSubscribe(ttclient1.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT), "subscribe");

        for(int i=0;i<2;i++) {

            TeamTalkBase ttclient = newClientInstance();
            clients.add(ttclient);

            connect(ttclient);
            initSound(ttclient);
            login(ttclient, NICKNAME, USERNAME, PASSWORD);

            assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannelByID(chan.nChannelID, chan.szPassword), DEF_WAIT), "join existing ");

            assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT), "subscribe");

            assertTrue(ttclient.enableVoiceTransmission(true), "Enable voice transmission");

            boolean chanUpEvent = false, userUpEvent = false;
            while (ttclient.getMessage(msg, DEF_WAIT) && (!chanUpEvent || !userUpEvent)) {
                switch (msg.nClientEvent) {
                case ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE :
                    assertTrue(ttclient.getChannel(ttclient.getMyChannelID(), chan), "Channel tx queue set");
                    assertEquals(ttclient.getMyUserID(), chan.transmitUsersQueue[0], "myself in queue #"+ ttclient.getMyUserID());
                    chanUpEvent = true;
                    break;
                case ClientEvent.CLIENTEVENT_USER_STATECHANGE :
                    assertEquals(UserState.USERSTATE_VOICE, msg.user.uUserState & UserState.USERSTATE_VOICE, "User state to voice");
                    assertEquals(ttclient.getMyUserID(), msg.user.nUserID, "myself talking");
                    userUpEvent = true;
                    break;
                }
            }

            //assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg), "wait chan update " + i);
            assertTrue(chanUpEvent, "wait chan update " + i);
            //assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "Wait for talking event");
            assertTrue(userUpEvent, "Wait for talking event");

            assertTrue(ttclient.enableVoiceTransmission(false), "Disable voice transmission");

            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "Wait for talking event stopped");
            assertEquals(UserState.USERSTATE_NONE, msg.user.uUserState & UserState.USERSTATE_VOICE, "User state to no voice");
            assertEquals(ttclient.getMyUserID(), msg.user.nUserID, "myself stopped talking");

            assertTrue(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT), "ttclient1, wait for tx queue start");
            assertTrue(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT), "ttclient1, wait for tx queue stop");
        }

        // wait for "reset" state
        assertTrue(waitCmdComplete(ttclient1, ttclient1.doPing(), DEF_WAIT), "ttclient1, drain client 1");

        assertTrue(ttclient1.enableVoiceTransmission(true), "ttclient1, Enable voice transmission");

        assertTrue(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg), "ttclient1, wait chan txq update");

        assertTrue(ttclient1.getChannel(ttclient1.getMyChannelID(), chan), "ttclient1, Channel tx queue set");

        assertEquals(ttclient1.getMyUserID(), chan.transmitUsersQueue[0], "ttclient1, myself is head in queue");

        // don't know if 'ClientEvent.CLIENTEVENT_USER_STATECHANGE' or
        // 'ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE' came first, so
        // don't assertTrue()
        waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_STATECHANGE, 1000, msg);
        User user = new User();
        assertTrue(ttclient1.getUser(ttclient1.getMyUserID(), user), "get ttclient1 state");
        assertEquals(UserState.USERSTATE_VOICE, user.uUserState & UserState.USERSTATE_VOICE, "ttclient1, User state to voice");
        assertEquals(ttclient1.getMyUserID(), msg.user.nUserID, "ttclient1, myself talking");

        // ensure ttclient2 doesn't take over transmit queue from ttclient1
        TeamTalkBase ttclient2 = clients.get(1);

        assertTrue(waitCmdComplete(ttclient2, ttclient2.doPing(), DEF_WAIT), "ttclient2, drain client 2");

        assertTrue(ttclient2.enableVoiceTransmission(true), "ttclient2, Enable voice transmission");

        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg), "ttclient2, wait chan txq update as no 2");

        assertTrue(ttclient2.getChannel(ttclient2.getMyChannelID(), chan), "ttclient2, Channel tx queue set");

        assertEquals(ttclient2.getMyUserID(), chan.transmitUsersQueue[1], "ttclient2, myself in queue");

        waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 1000);
        assertTrue(ttclient2.getUser(ttclient2.getMyUserID(), user) && (user.uUserState & UserState.USERSTATE_VOICE) == 0, "ttclient2 is not talking");


        // ensure ttclient2 takes over transmit queue when ttclient1 stops transmitting
        assertTrue(waitCmdComplete(ttclient1, ttclient1.doPing(), DEF_WAIT), "ttclient1, drain client 1");
        assertTrue(waitCmdComplete(ttclient2, ttclient2.doPing(), DEF_WAIT), "ttclient2, drain client 2");

        assertTrue(ttclient1.enableVoiceTransmission(false), "ttclient1, Disable voice transmission");

        assertTrue(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg), "ttclient1, wait chan txq update");

        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg), "ttclient2, wait chan txq update as no 1");
        assertTrue(ttclient2.getChannel(ttclient2.getMyChannelID(), chan), "ttclient2, retrieve channel");

        assertEquals(ttclient2.getMyUserID(), chan.transmitUsersQueue[0], "ttclient2 head in queue");

        //ensure transmit queue becomes empty when ttclient2 stops transmitting
        assertTrue(ttclient2.enableVoiceTransmission(false), "ttclient2, disable voice transmission");

        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg), "ttclient2, wait chan txq update clear");
        assertTrue(ttclient2.getChannel(ttclient2.getMyChannelID(), chan), "ttclient2, retrieve channel");

        assertEquals(0, chan.transmitUsersQueue[0], "ttclient2 empty queue");


        // ensure ttclient1 can take over transmit queue again
        assertTrue(waitCmdComplete(ttclient1, ttclient1.doPing(), DEF_WAIT), "drain ttclient1");

        assertTrue(ttclient1.enableVoiceTransmission(true), "ttclient1, Enable voice transmission");

        assertTrue(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg), "ttclient1, wait chan txq update");

        assertTrue(ttclient1.getChannel(ttclient1.getMyChannelID(), chan), "ttclient1, Channel tx queue set");

        assertEquals(ttclient1.getMyUserID(), chan.transmitUsersQueue[0], "ttclient1, myself is head again in queue ");
    }

    @Test
    public void testSoloTransmitChannelDelay() {
        if (GITHUBSKIP) {
            // uses System.currentTimeMillis()
            return;
        }

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
            UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_VIEW_ALL_USERS | UserRight.USERRIGHT_MULTI_LOGIN;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        Vector<TeamTalkBase> clients = new Vector<TeamTalkBase>();

        TTMessage msg = new TTMessage();

        TeamTalkBase ttclient1 = newClientInstance();
        clients.add(ttclient1);

        connect(ttclient1);
        initSound(ttclient1);
        login(ttclient1, NICKNAME, USERNAME, PASSWORD);

        Channel chan = buildDefaultChannel(ttclient1, "Opus");
        assertEquals(Codec.OPUS_CODEC, chan.audiocodec.nCodec, "opus default");
        chan.uChannelType |= ChannelType.CHANNEL_SOLO_TRANSMIT;
        chan.nTransmitUsersQueueDelayMSec = 500;
        assertTrue(waitCmdSuccess(ttclient1, ttclient1.doJoinChannel(chan), DEF_WAIT), "join");

        TeamTalkBase ttclient2 = newClientInstance();
        clients.add(ttclient2);

        connect(ttclient2);
        initSound(ttclient2);
        login(ttclient2, NICKNAME, USERNAME, PASSWORD);

        assertTrue(waitCmdSuccess(ttclient2, ttclient2.doJoinChannelByID(ttclient1.getMyChannelID(), ""), DEF_WAIT), "join");

        assertTrue(ttclient2.getChannel(ttclient2.getMyChannelID(), chan), "get channel");
        assertEquals(500, chan.nTransmitUsersQueueDelayMSec, "default switch timeout is 500");

        assertTrue(waitCmdComplete(ttclient1, ttclient1.doPing(), DEF_WAIT), "ttclient1, drain client 1");
        assertTrue(waitCmdComplete(ttclient2, ttclient2.doPing(), DEF_WAIT), "ttclient2, drain client 2");

        // validate 500 msec default switch timeout
        assertTrue(ttclient1.enableVoiceTransmission(true), "ttclient1, Enable voice transmission");
        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg), "ttclient2, wait chan txq update");
        assertEquals(ttclient1.getMyUserID(), msg.channel.transmitUsersQueue[0], "ttclient1 is head in queue ");

        assertTrue(ttclient2.enableVoiceTransmission(true), "ttclient2, Enable voice transmission");
        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg), "ttclient2, wait chan txq update");
        assertEquals(ttclient2.getMyUserID(), msg.channel.transmitUsersQueue[1], "ttclient2 is two in queue ");

        assertTrue(waitCmdComplete(ttclient1, ttclient1.doPing(), DEF_WAIT), "ttclient1, drain client 1");
        assertTrue(waitCmdComplete(ttclient2, ttclient2.doPing(), DEF_WAIT), "ttclient2, drain client 2");

        long switchStart = System.currentTimeMillis();
        assertTrue(ttclient1.enableVoiceTransmission(false), "ttclient1, disable voice transmission");
        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg), "ttclient2, wait chan txq update");
        assertEquals(ttclient2.getMyUserID(), msg.channel.transmitUsersQueue[0], "ttclient2 is head in queue ");
        assertTrue(System.currentTimeMillis() - switchStart >= 500, "default switch delay is ~500 msec");

        assertTrue(ttclient2.enableVoiceTransmission(false), "ttclient2, disable voice transmission");
        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg), "ttclient2, wait chan txq update");
        assertEquals(0, msg.channel.transmitUsersQueue[0], "queue is empty");
        assertTrue(waitCmdComplete(ttclient1, ttclient1.doPing(), DEF_WAIT), "ttclient1, drain client 1");
        assertTrue(waitCmdComplete(ttclient2, ttclient2.doPing(), DEF_WAIT), "ttclient2, drain client 2");

        // validate 500 msec default switch timeout no longer applies
        chan.nTransmitUsersQueueDelayMSec = 100;
        assertTrue(waitCmdSuccess(ttclient1, ttclient1.doUpdateChannel(chan), DEF_WAIT), "update channel");
        assertTrue(ttclient1.getChannel(ttclient1.getMyChannelID(), chan), "get channel");
        assertEquals(100, chan.nTransmitUsersQueueDelayMSec, "default switch timeout is 100");
        assertTrue(waitCmdComplete(ttclient2, ttclient2.doPing(), DEF_WAIT), "ttclient2, drain client 2");

        assertTrue(ttclient1.enableVoiceTransmission(true), "ttclient1, Enable voice transmission");
        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg), "ttclient2, wait chan txq update");
        assertEquals(ttclient1.getMyUserID(), msg.channel.transmitUsersQueue[0], "ttclient1 is head in queue");

        assertTrue(ttclient2.enableVoiceTransmission(true), "ttclient2, Enable voice transmission");
        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg), "ttclient2, wait chan txq update");
        assertEquals(ttclient2.getMyUserID(), msg.channel.transmitUsersQueue[1], "ttclient2 is two in queue ");

        assertTrue(waitCmdComplete(ttclient1, ttclient1.doPing(), DEF_WAIT), "ttclient1, drain client 1");
        assertTrue(waitCmdComplete(ttclient2, ttclient2.doPing(), DEF_WAIT), "ttclient2, drain client 2");

        switchStart = System.currentTimeMillis();
        assertTrue(ttclient1.enableVoiceTransmission(false), "ttclient1, disable voice transmission");
        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, msg), "ttclient2, wait chan txq update");
        assertEquals(ttclient2.getMyUserID(), msg.channel.transmitUsersQueue[0], "ttclient2 is head in queue ");
        assertTrue(System.currentTimeMillis() - switchStart >= chan.nTransmitUsersQueueDelayMSec, "default switch delay is ~100 msec");
        assertTrue(System.currentTimeMillis() - switchStart < 500, "default switch delay is less than 500 msec");
    }

    @Test
    public void testAbusePrevention() {
        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_TEXTMESSAGE_USER;

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

        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doNewUserAccount(account), DEF_WAIT), "create flood prevent account");

        TeamTalkBase ttclient = newClientInstance();

        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 2000);

        TextMessage txtmsg = new TextMessage();
        txtmsg.nMsgType = TextMsgType.MSGTYPE_USER;
        txtmsg.nToUserID = ttclient.getMyUserID();
        txtmsg.szMessage = "My text message that should go through";

        assertTrue(waitCmdSuccess(ttclient, ttclient.doTextMessage(txtmsg), DEF_WAIT), "do text message");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 200);

        txtmsg.szMessage = "My text message that should be blocked";

        assertTrue(waitCmdError(ttclient, ttclient.doTextMessage(txtmsg), DEF_WAIT), "do text message in less than cmd-timeout");

        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 2000);

        assertTrue(waitCmdSuccess(ttclient, ttclient.doTextMessage(txtmsg), DEF_WAIT), "do text message after cmd-timeout");
    }

    @Test
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

        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doNewUserAccount(account), DEF_WAIT), "create account");

        TeamTalkBase ttclient1 = newClientInstance();
        TeamTalkBase ttclient2 = newClientInstance();

        connect(ttclient1);
        login(ttclient1, NICKNAME, USERNAME, PASSWORD);
        connect(ttclient2);

        assertTrue(waitCmdError(ttclient2, ttclient2.doLogin(NICKNAME, USERNAME, PASSWORD), DEF_WAIT), "login failure");

        Thread.sleep(2000);
        login(ttclient2, NICKNAME, USERNAME, PASSWORD);

        srvprop.nLoginDelayMSec = orgValue;
        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doUpdateServer(srvprop), DEF_WAIT));
    }

    @Test
    public void testLoginAttempts() {

        TeamTalkBase ttadmin = newClientInstance();
        connect(ttadmin);
        login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        ServerProperties srvprop = new ServerProperties();
        assertTrue(ttadmin.getServerProperties(srvprop), "get srvprop");

        int oldValue = srvprop.nMaxLoginAttempts;

        srvprop.nMaxLoginAttempts = 2;

        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doUpdateServer(srvprop), DEF_WAIT), "update server");

        User user = new User();
        assertTrue(ttadmin.getUser(ttadmin.getMyUserID(), user), "get user");

        TeamTalkBase ttclient = newClientInstance();

        connect(ttclient);

        TTMessage msg = new TTMessage();
        int cmdid = ttclient.doLogin(ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, "wrongpassword1");
        assertTrue(waitCmdError(ttclient, cmdid, DEF_WAIT, msg), "wait login error");
        assertEquals(ClientError.CMDERR_INVALID_ACCOUNT, msg.clienterrormsg.nErrorNo, "invalid account");

        cmdid = ttclient.doLogin(ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, "wrongpassword2");
        assertTrue(waitCmdError(ttclient, cmdid, DEF_WAIT, msg), "wait login error");
        assertEquals(ClientError.CMDERR_INVALID_ACCOUNT, msg.clienterrormsg.nErrorNo, "invalid account");

        cmdid = ttclient.doLogin(ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, "wrongpassword3");
        assertTrue(waitCmdError(ttclient, cmdid, DEF_WAIT, msg), "wait login error");
        assertEquals(ClientError.CMDERR_SERVER_BANNED, msg.clienterrormsg.nErrorNo, "banned account");

        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doUnBanUser(user.szIPAddress, 0), DEF_WAIT), "unban success");

        srvprop.nMaxLoginAttempts = 0;
        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doUpdateServer(srvprop), DEF_WAIT), "update server");
    }

    @Test
    public void testZUserTimeout() throws IOException {

        TeamTalkBase ttadmin = newClientInstance();
        connect(ttadmin);
        login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        ServerProperties srvprop = new ServerProperties();
        assertTrue(ttadmin.getServerProperties(srvprop), "get srvprop");
        int orgValue = srvprop.nUserTimeout;
        srvprop.nUserTimeout = 60;
        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doUpdateServer(srvprop), DEF_WAIT), "update server");

        assertTrue(ttadmin.disconnect(), "Disconnect hard");

        connect(ttadmin);
        login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        assertTrue(ttadmin.doQuit()>0, "DoQuit");

        assertTrue(waitForEvent(ttadmin, ClientEvent.CLIENTEVENT_CON_LOST, DEF_WAIT), "Wait con lost");

        assertTrue(ttadmin.disconnect(), "Disconnect quit");

        connect(ttadmin);
        login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        srvprop.nUserTimeout = 1;

        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doUpdateServer(srvprop), DEF_WAIT), "update server");

        assertTrue(ttadmin.disconnect(), "Disconnect after tmo");

        try (Socket s = new Socket(IPADDR, TCPPORT);
             BufferedReader stream = new BufferedReader(new InputStreamReader(s.getInputStream()));) {

            if(!ENCRYPTED)
            {
                String welcome = stream.readLine();
                assertTrue(welcome.startsWith(SYSTEMID), "welcome msg");
            }

            boolean closed = false;
            try {
                closed = stream.readLine() == null;
            }
            catch(IOException e) {
                closed = true;
            }
            assertTrue(closed, "Closed socket");

            assertTrue(ttadmin.disconnect(), "Disconnect quit");

            connect(ttadmin);
            login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);
            srvprop.nUserTimeout = orgValue;
            assertTrue(waitCmdSuccess(ttadmin, ttadmin.doUpdateServer(srvprop), DEF_WAIT), "update server");
        }
    }

    @Test
    public void testKeyTranslate() {
        TeamTalkBase ttadmin = newClientInstance();
        DesktopInput[] inputs = new DesktopInput[2], outputs = new DesktopInput[2];
        for (int i=0;i<inputs.length;++i) {
            inputs[i] = new DesktopInput();
            inputs[i].uMousePosY = 100;
            inputs[i].uKeyState = DesktopKeyStates.DESKTOPKEYSTATE_NONE;
            outputs[i] = new DesktopInput();
        }
        assertTrue(WindowsHelper.desktopInputKeyTranslate(TTKeyTranslate.TTKEY_WINKEYCODE_TO_TTKEYCODE,
                                                                           inputs, outputs) >= 0, "Key translate");
        assertEquals(inputs[0].uMousePosY, outputs[0].uMousePosY, "Coordinate");
        assertEquals(inputs[0].uKeyState, outputs[0].uKeyState, "Keystate");
        assertEquals(inputs[1].uMousePosY, outputs[1].uMousePosY, "Coordinate");
        assertEquals(inputs[1].uKeyState, outputs[1].uKeyState, "Keystate");

        assertTrue(PlatformHelper.desktopInputExecute(outputs) >= 0, "move mouse");
    }

    @Test
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

        assertTrue(ttclient1.sendDesktopWindow(wnd, BitmapFormat.BMP_RGB32)>0, "send desktop #1 window");

        TTMessage msg = new TTMessage();
        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, DEF_WAIT, msg), "Wait for desktop #1 window");

        int desktop1ID = msg.nStreamID;
        assertTrue(desktop1ID > 0, "Desktop #1 shown");

        assertTrue(waitCmdSuccess(ttclient1,
                                                            ttclient1.doSubscribe(ttclient2.getMyUserID(),
                                                                                  Subscription.SUBSCRIBE_DESKTOPINPUT),
                                                            DEF_WAIT), "subscribe desktopinput");
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

            assertTrue(ttclient2.sendDesktopInput(ttclient1.getMyUserID(), inputs), "send desktop #1 input x="+x);

            assertTrue(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_DESKTOPINPUT, DEF_WAIT, msg), "get desktop #1 input[0]");
            assertEquals(x, msg.desktopinput.uMousePosX, "desktop #1 input[0] x");
            assertEquals(10, msg.desktopinput.uMousePosY, "desktop #1 input[0] y");

            assertTrue(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_DESKTOPINPUT, DEF_WAIT, msg), "get desktop #1 input[1]");
            assertEquals(x, msg.desktopinput.uMousePosX, "desktop #1 input[1] x");
            assertEquals(20, msg.desktopinput.uMousePosY, "desktop #1 input[1] y");
        }

        assertTrue(ttclient1.sendDesktopCursorPosition(5, 6), "send cursor pos");

        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_DESKTOPCURSOR, DEF_WAIT, msg), "get desktop cursor");
        assertEquals(5, msg.desktopinput.uMousePosX, "pos x");
        assertEquals(6, msg.desktopinput.uMousePosY, "pos y");

        // // start new desktop session
        // wnd = new DesktopWindow();
        // wnd.nWidth = 1024;
        // wnd.nHeight = 1024;
        // wnd.bmpFormat = BitmapFormat.BMP_RGB32;
        // wnd.nProtocol = DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1;
        // wnd.frameBuffer = new byte[wnd.nWidth * wnd.nHeight * 4];

        // assertTrue(ttclient1.sendDesktopWindow(wnd, BitmapFormat.BMP_RGB32)>0, "send desktop #2 window");

        // assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, DEF_WAIT, msg), "Wait for desktop #2 window");

        // assertTrue(desktop1ID != msg.nStreamID, "Desktop #2 shown");

        // DesktopInput[] input = new DesktopInput[1];
        // int y = wnd.nHeight;
        // for (int x=0;x<wnd.nWidth;x++) {
        //     inputs[0] = new DesktopInput();
        //     inputs[0].uMousePosX = x;
        //     inputs[0].uMousePosY = --y;
        //     inputs[0].uKeyState = DesktopKeyStates.DESKTOPKEYSTATE_NONE;

        //     assertTrue(ttclient2.sendDesktopInput(ttclient1.getMyUserID(), inputs), "send desktop #2 input x="+x);

        //     assertTrue(waitForEvent(ttclient1, ClientEvent.CLIENTEVENT_USER_DESKTOPINPUT, DEF_WAIT, msg), "get desktop #2 input 0");
        //     assertEquals(x, msg.desktopinput.uMousePosX, "desktop #2 input[0] x");
        //     assertEquals(y, msg.desktopinput.uMousePosY, "desktop #2 input[0] y");
        // }

    }

    @Test
    public void testLocalPlayback() {

        // load shared object
        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);

        // play single two second file
        MediaFileInfo mfi = new MediaFileInfo();
        mfi.szFileName = STORAGEFOLDER + File.separator + "hest.wav";
        mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 2);
        mfi.uDurationMSec = 2 * 1000;

        assertTrue(TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600), "Write media file");

        MediaFilePlayback mfp = new MediaFilePlayback();

        int sessionid = ttclient.initLocalPlayback(mfi.szFileName, mfp);
        assertTrue(sessionid > 0, "init playback");

        TTMessage msg = new TTMessage();
        assertTrue(DEF_WAIT > mfi.uDurationMSec);
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg), "Wait for playback");

        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "streaming started");

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
        assertTrue(playing, "Playing event");
        assertEquals(MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus, "Streaming ended");
        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, 0, msg), "Last playback event is finished");
    }

    @Test
    public void testLocalPlaybackPause() {

        // load shared object
        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);

        // play single two second file
        MediaFileInfo mfi = new MediaFileInfo();
        mfi.szFileName = STORAGEFOLDER + File.separator + "hest.wav";
        mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 2);
        mfi.uDurationMSec = 20 * 1000;

        assertTrue(TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600), "Write media file");

        MediaFilePlayback mfp = new MediaFilePlayback();

        TTMessage msg = new TTMessage();

        // play
        mfp.bPaused = true;
        int sessionid = ttclient.initLocalPlayback(mfi.szFileName, mfp);
        assertTrue(sessionid > 0, "init playback");

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 100), "Hold pause state");

        mfp.bPaused = false;
        assertTrue(ttclient.updateLocalPlayback(sessionid, mfp), "Unpause");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg), "Wait for start event");
        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "playback started");

        int starttime = msg.mediafileinfo.uElapsedMSec;
        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg)) {
            if (msg.mediafileinfo.uElapsedMSec - starttime >= 1000)
                break;
        }

        assertTrue(msg.mediafileinfo.uElapsedMSec - starttime >= 1000, "Play one sec");

        mfp.bPaused = true;
        assertTrue(ttclient.updateLocalPlayback(sessionid, mfp), "Pause again");

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg)) {
            if (msg.mediafileinfo.nStatus == MediaFileStatus.MFS_PAUSED)
                break;
        }
        assertEquals(MediaFileStatus.MFS_PAUSED, msg.mediafileinfo.nStatus, "streaming paused");
    }

    @Test
    public void testLocalPlaybackSeek() {

        // load shared object
        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);

        // play single two second file
        MediaFileInfo mfi = new MediaFileInfo();
        mfi.szFileName = STORAGEFOLDER + File.separator + "hest.wav";
        mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 2);
        mfi.uDurationMSec = 20 * 1000;

        assertTrue(TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600), "Write media file");

        MediaFilePlayback mfp = new MediaFilePlayback();

        TTMessage msg = new TTMessage();

        // play
        mfp.uOffsetMSec = 19 * 1000;
        int sessionid = ttclient.initLocalPlayback(mfi.szFileName, mfp);
        assertTrue(sessionid > 0, "init playback");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg), "Wait for start event");
        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "playback started");

        assertTrue(DEF_WAIT > mfi.uDurationMSec - mfp.uOffsetMSec);
        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg)) {
            if (msg.mediafileinfo.nStatus == MediaFileStatus.MFS_FINISHED)
                break;
        }
        assertEquals(MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus, "streaming finished");
    }

    @Test
    public void testLocalPlaybackSeekBackwards() {

        // load shared object
        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);

        // play single two second file
        MediaFileInfo mfi = new MediaFileInfo();
        mfi.szFileName = STORAGEFOLDER + File.separator + "hest.wav";
        mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 2);
        mfi.uDurationMSec = 20 * 1000;

        assertTrue(TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600), "Write media file");

        MediaFilePlayback mfp = new MediaFilePlayback();

        TTMessage msg = new TTMessage();

        // play
        mfp.uOffsetMSec = 19 * 1000;
        int sessionid = ttclient.initLocalPlayback(mfi.szFileName, mfp);
        assertTrue(sessionid > 0, "init playback");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg), "Wait for start event");
        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "playback started");
        int elapsed = msg.mediafileinfo.uElapsedMSec;

        mfp.uOffsetMSec = 18 * 1000;
        assertTrue(ttclient.updateLocalPlayback(sessionid, mfp), "Rewind");

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

        assertTrue(playing, "Wait for playing event");
        assertTrue(started, "Wait for start event");
        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "playback started");

        assertTrue(msg.mediafileinfo.uElapsedMSec < elapsed, "Playback from rewinded position");

        assertTrue(DEF_WAIT > mfi.uDurationMSec - msg.mediafileinfo.uElapsedMSec);

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg)) {

            if (msg.mediafileinfo.nStatus == MediaFileStatus.MFS_FINISHED)
                break;
        }

        assertEquals(MediaFileStatus.MFS_FINISHED, msg.mediafileinfo.nStatus, "streaming finished");
    }

    @Test
    public void testLocalPlaybackManySmall() {

        Map< TeamTalkBase, Vector<Integer> > clientsessions = new HashMap<TeamTalkBase, Vector<Integer>>();
        for (int i=0;i<3;++i) {
            TeamTalkBase ttclient = newClientInstance();
            initSound(ttclient);
            clientsessions.put(ttclient, new Vector<Integer>());
        }

        Vector<MediaFileInfo> files = new Vector<MediaFileInfo>();
        int[] durations = { 950, 150, 300, 50, 100, 130, 500};

        for (int duration : durations) {
            MediaFileInfo mfi = new MediaFileInfo();
            mfi.szFileName = STORAGEFOLDER + File.separator + String.format("hest_%d.wav", duration);
            mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 2);
            mfi.uDurationMSec = duration;
            assertTrue(TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600), "Write media file");
            files.add(mfi);
        }

        for (TeamTalkBase ttclient : clientsessions.keySet()) {
            MediaFilePlayback mfp = new MediaFilePlayback();
            mfp.uOffsetMSec = MediaFilePlaybackConstants.TT_MEDIAPLAYBACK_OFFSET_IGNORE;
            Vector<Integer> sessions = clientsessions.get(ttclient);
            for (MediaFileInfo mfi : files) {
                // play
                int sessionid = ttclient.initLocalPlayback(mfi.szFileName, mfp);
                assertTrue(sessionid > 0, "init playback");
                sessions.add(sessionid);
            }
        }

        while (clientsessions.size() > 0) {
            TeamTalkBase ttclient = clientsessions.keySet().iterator().next();
            Vector<Integer> sessions = clientsessions.get(ttclient);
            TTMessage msg = new TTMessage();
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg), "get media event");
            switch (msg.mediafileinfo.nStatus) {
                case MediaFileStatus.MFS_FINISHED :
                    sessions.removeElement(msg.nSource);
                    break;
            }
            if (sessions.size() == 0)
                clientsessions.remove(ttclient);
        }
    }

    @Test
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

        assertTrue(ttclient.enableAudioBlockEvent(Constants.TT_MUXED_USERID, StreamType.STREAMTYPE_VOICE, true), "enable muxed aud cb");
        assertTrue(ttclient.enableAudioBlockEvent(Constants.TT_LOCAL_USERID, StreamType.STREAMTYPE_VOICE, true), "enable local aud cb");

        FileOutputStream localWaveFile = null, muxedWaveFile = null;
        while (localWaveFile == null || muxedWaveFile == null) {
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme voice audioblock");
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

        assertTrue(ttclient.enableVoiceTransmission(true), "enable voice tx");

        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "get voice event");
        assertTrue(((msg.user.uUserState & UserState.USERSTATE_VOICE) != 0), "initial voice event");

        int STREAMID = 57;
        final int SAMPLERATE = 16000, CHANNELS = 1;

        byte[] tone = generateToneAsByte(500, SAMPLERATE, CHANNELS, 1000);

        assertEquals(SAMPLERATE, tone.length / 2 / CHANNELS, "one second of audio");

        AudioBlock ab = new AudioBlock();
        ab.nStreamID = STREAMID;
        ab.nSampleRate = SAMPLERATE;
        ab.nChannels = CHANNELS;
        ab.lpRawAudio = tone; //PCM16 mono
        ab.nSamples = tone.length / 2 / CHANNELS;
        ab.uSampleIndex = 0;

        assertFalse(ttclient.insertAudioBlock(ab), "Reject audio input during voicetx");

        assertTrue(ttclient.enableVoiceTransmission(false), "disable voice tx");

        assertTrue(ttclient.enableVoiceActivation(true), "enable voice act");

        assertFalse(ttclient.insertAudioBlock(ab), "Reject audio input during voiceact");

        assertTrue(ttclient.enableVoiceActivation(false), "disable voice act");

        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "voice stopped event");
        assertTrue(((msg.user.uUserState & UserState.USERSTATE_VOICE) == 0), "Voice stop event");

        assertTrue(ttclient.insertAudioBlock(ab), "Send audio block");

        int frames = 0;
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_AUDIOINPUT, DEF_WAIT, msg), "Audio input "+STREAMID+" started");
        frames++;

        assertEquals(STREAMID, msg.audioinputprogress.nStreamID, "Stream ID match");

        assertTrue(waitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "Playing audio input remotely");
        assertTrue(((msg.user.uUserState & UserState.USERSTATE_VOICE) != 0), "Voice event from audio input");

        assertFalse(ttclient.enableVoiceTransmission(true), "Reject voice tx");

        assertFalse(ttclient.enableVoiceActivation(true), "Reject voice act");

        assertTrue(ttclient.enableVoiceTransmission(false), "Accept voice tx disable");

        assertTrue(ttclient.enableVoiceActivation(false), "Accept voice act disable");

        boolean gotlocal = false, gotmuxed = false;
        do {
            assertTrue(ttclient.getMessage(msg, DEF_WAIT), "Event processing");
            switch (msg.nClientEvent) {
            case ClientEvent.CLIENTEVENT_AUDIOINPUT :
                if (msg.audioinputprogress.uElapsedMSec > 0)
                    frames++;
                break;
            case ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK :
                if (msg.nSource == Constants.TT_LOCAL_USERID) {
                    AudioBlock abl = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_LOCAL_USERID);
                    assertTrue(abl != null, "get local audio block");
                    localWaveFile.write(abl.lpRawAudio);
                    gotlocal = true;
                }
                if (msg.nSource == Constants.TT_MUXED_USERID) {
                    AudioBlock abm = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_MUXED_USERID);
                    assertTrue(abm != null, "get muxed audio block");
                    muxedWaveFile.write(abm.lpRawAudio);
                    gotmuxed = true;
                }
                break;
            }
        } while (msg.nClientEvent != ClientEvent.CLIENTEVENT_AUDIOINPUT ||
                 (msg.audioinputprogress.nStreamID == STREAMID &&
                  msg.audioinputprogress.uElapsedMSec != 0 &&
                  msg.audioinputprogress.uQueueMSec != 0));

        assertTrue(gotlocal, "Got local user audio");
        assertTrue(gotmuxed, "Got muxed user audio");

        // now send audio block whose nSamples doesn't match the
        // transmit interval, i.e. we need to flush to send
        // everything.
        STREAMID = 55;
        ab.nStreamID = STREAMID;

        ab.nSamples -= 43;
        assertTrue(ttclient.insertAudioBlock(ab), "Send audio block which needs to be flushed "+ STREAMID);

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_AUDIOINPUT, DEF_WAIT, msg), "Audio input "+STREAMID+" started");
        frames--;

        assertEquals(STREAMID, msg.audioinputprogress.nStreamID, "Stream ID match");

        gotlocal = gotmuxed = false;
        for (int i=0;i<frames-1;) {
            assertTrue(ttclient.getMessage(msg, DEF_WAIT), "Event processing");
            switch (msg.nClientEvent) {
            case ClientEvent.CLIENTEVENT_AUDIOINPUT :
                assertTrue(msg.audioinputprogress.nStreamID == STREAMID, "stream id match");
                assertTrue(msg.audioinputprogress.uElapsedMSec != 0, "elapsed increasing");
                assertTrue(msg.audioinputprogress.uQueueMSec != 0, "queue holding");
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
        assertTrue(gotlocal, "Got local user audio, before flushing");
        assertTrue(gotmuxed, "Got muxed user audio, before flushing");

        assertFalse(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_AUDIOINPUT, 100, msg), "Last frame will not appear until we flush");

        assertTrue(ttclient.insertAudioBlock(new AudioBlock()), "Flush queued audio "+ STREAMID);

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_AUDIOINPUT, DEF_WAIT, msg), "Audio input last frame "+STREAMID);
        assertTrue(msg.audioinputprogress.nStreamID == STREAMID, "stream id match");
        assertTrue(msg.audioinputprogress.uElapsedMSec != 0, "elapsed increasing");
        assertTrue(msg.audioinputprogress.uQueueMSec == 0, "queue zero");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_AUDIOINPUT, DEF_WAIT, msg), "Audio input ending "+STREAMID);
        assertTrue(msg.audioinputprogress.nStreamID == STREAMID, "stream id ended");
        assertTrue(msg.audioinputprogress.uElapsedMSec == 0, "elapsed done");
        assertTrue(msg.audioinputprogress.uQueueMSec == 0, "queue done");

        //drain audio blocks
        while (ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_LOCAL_USERID) != null);
        while (ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_MUXED_USERID) != null);
        waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 0);

        gotlocal = gotmuxed = false;
        while (!gotlocal || !gotmuxed) {

            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "get another audioblock");
            if (msg.nSource == Constants.TT_LOCAL_USERID) {
                AudioBlock abl = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_LOCAL_USERID);
                assertTrue(abl != null, "get local audio block");
                gotlocal = true;
            }
            if (msg.nSource == Constants.TT_MUXED_USERID) {
                AudioBlock abm = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_MUXED_USERID);
                assertTrue(abm != null, "get muxed audio block");
                gotmuxed = true;
            }
        }

        assertTrue(gotlocal, "Got local user audio, after flush");
        assertTrue(gotmuxed, "Got muxed user audio, after flush");

        // test audio input queue limits
        STREAMID = 77;
        ab.nStreamID = STREAMID;
        ab.lpRawAudio = generateToneAsByte(800, 16000, 1, 4000); // PCM16 mono
        ab.nSamples = ab.lpRawAudio.length / 2;

        // assertFalse("Maximum queue size for audio input is 3 sec", ttclient.insertAudioBlock(ab));
        localWaveFile.close();
        muxedWaveFile.close();
    }

    @Test
    public void testVoiceTransmitOpenCloseAudioInput() throws InterruptedException {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_MULTI_LOGIN | UserRight.USERRIGHT_TRANSMIT_VOICE;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase[] clients = new TeamTalkBase[4];
        for (int i=0;i<clients.length;++i) {
            clients[i] = newClientInstance();
            IntPtr indev = new IntPtr(), outdev = new IntPtr();
            boolean gotdevs = clients[i].getDefaultSoundDevices(indev, outdev);
            if (INPUTDEVICEID >= 0)
                indev.value = INPUTDEVICEID;
            if (OUTPUTDEVICEID >= 0)
                outdev.value = OUTPUTDEVICEID;
            assertTrue(clients[i].initSoundOutputDevice(outdev.value), "init output device");
            connect(clients[i]);
            login(clients[i], NICKNAME + "_" + i, USERNAME, PASSWORD);
            joinRoot(clients[i]);
        }

        for (int i=0;i < 5; i++) {
            for (TeamTalkBase ttclient : clients) {
                IntPtr indev = new IntPtr(), outdev = new IntPtr();
                boolean gotdevs = ttclient.getDefaultSoundDevices(indev, outdev);
                if (INPUTDEVICEID >= 0)
                    indev.value = INPUTDEVICEID;
                if (OUTPUTDEVICEID >= 0)
                    outdev.value = OUTPUTDEVICEID;
                assertTrue(ttclient.initSoundInputDevice(indev.value), "client init sndinput");
                assertTrue(ttclient.enableVoiceTransmission(true), "client enable voice tx");
            }

            waitForEvent(clients[0], ClientEvent.CLIENTEVENT_NONE, 1500);

            for (TeamTalkBase ttclient : clients) {
                assertTrue(ttclient.closeSoundInputDevice(), "client close sndinput");
                assertTrue(ttclient.enableVoiceTransmission(false), "client disable voice tx");
            }

            waitForEvent(clients[0], ClientEvent.CLIENTEVENT_NONE, 1000);
        }
    }

    @Test
    public void testSharedAudioInputOutput() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_MULTI_LOGIN | UserRight.USERRIGHT_TRANSMIT_VOICE |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase rxclient = newClientInstance();

        IntPtr indev = new IntPtr(), outdev = new IntPtr();
        if(INPUTDEVICEID < 0 && OUTPUTDEVICEID < 0)
           assertTrue(rxclient.getDefaultSoundDevices(indev, outdev), "get default devs");
        else
        {
            indev.value = INPUTDEVICEID;
            outdev.value = OUTPUTDEVICEID;
        }

        indev.value |= SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;
        outdev.value |= SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;

        assertTrue(rxclient.initSoundInputDevice(indev.value), "Init rx input");
        assertTrue(rxclient.initSoundOutputDevice(outdev.value), "Init rx output");
        connect(rxclient);
        login(rxclient, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);
        Channel chan = buildDefaultChannel(rxclient, "Speex", Codec.SPEEX_CODEC);
        chan.audiocodec.speex.nBandmode = SpeexConstants.SPEEX_BANDMODE_WIDE; //16000
        chan.audiocodec.speex.nTxIntervalMSec = 400;
        assertTrue(waitCmdSuccess(rxclient, rxclient.doJoinChannel(chan), DEF_WAIT), "rxclient join channel");

        TeamTalkBase txclient1 = newClientInstance();
        assertTrue(txclient1.initSoundInputDevice(indev.value), "Init tx1 input");
        assertTrue(txclient1.initSoundOutputDevice(outdev.value), "Init tx1 output");
        connect(txclient1);
        login(txclient1, NICKNAME, USERNAME, PASSWORD);
        joinRoot(txclient1);
        assertTrue(txclient1.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 300), "Gen tone tx1");

        TeamTalkBase txclient2 = newClientInstance();
        assertTrue(txclient2.initSoundInputDevice(indev.value), "Init tx2 input");
        assertTrue(txclient2.initSoundOutputDevice(outdev.value), "Init tx2 output");
        connect(txclient2);
        login(txclient2, NICKNAME, USERNAME, PASSWORD);
        assertTrue(waitCmdSuccess(txclient2, txclient2.doJoinChannelByID(rxclient.getMyChannelID(), ""), DEF_WAIT), "tx2 join existing");
        assertTrue(txclient2.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 600), "Gen tone tx2");

        TeamTalkBase txclient3 = newClientInstance();
        assertTrue(txclient3.initSoundInputDevice(indev.value), "Init tx3 input");
        assertTrue(txclient3.initSoundOutputDevice(outdev.value), "Init tx3 output");
        connect(txclient3);
        login(txclient3, NICKNAME, USERNAME, PASSWORD);
        chan = buildDefaultChannel(txclient3, "OPUS223", Codec.OPUS_CODEC);
        chan.audiocodec.opus.nSampleRate = 24000;
        chan.audiocodec.opus.nFrameSizeMSec = 60;
        chan.audiocodec.opus.nTxIntervalMSec = 60;
        assertTrue(waitCmdSuccess(txclient3, txclient3.doJoinChannel(chan), DEF_WAIT), "txclient3 join channel");
        assertTrue(txclient3.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 900), "Gen tone tx3");

        TeamTalkBase txclient4 = newClientInstance();
        assertTrue(txclient4.initSoundInputDevice(indev.value), "Init tx4 input");
        assertTrue(txclient4.initSoundOutputDevice(outdev.value), "Init tx4 output");
        connect(txclient4);
        login(txclient4, NICKNAME, USERNAME, PASSWORD);
        chan = buildDefaultChannel(txclient4, "OPUS224", Codec.OPUS_CODEC);
        chan.audiocodec.opus.nSampleRate = 12000;
        chan.audiocodec.opus.nFrameSizeMSec = 20;
        chan.audiocodec.opus.nTxIntervalMSec = 20;
        assertTrue(waitCmdSuccess(txclient4, txclient4.doJoinChannel(chan), DEF_WAIT), "txclient4 join channel");
        assertTrue(txclient4.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 1200), "Gen tone tx4");

        assertTrue(waitCmdSuccess(rxclient, rxclient.doSubscribe(txclient1.getMyUserID(), Subscription.SUBSCRIBE_INTERCEPT_VOICE), DEF_WAIT), "Intercept tx1");
        assertTrue(waitCmdSuccess(rxclient, rxclient.doSubscribe(txclient2.getMyUserID(), Subscription.SUBSCRIBE_INTERCEPT_VOICE), DEF_WAIT), "Intercept tx2");
        assertTrue(waitCmdSuccess(rxclient, rxclient.doSubscribe(txclient3.getMyUserID(), Subscription.SUBSCRIBE_INTERCEPT_VOICE), DEF_WAIT), "Intercept tx3");
        assertTrue(waitCmdSuccess(rxclient, rxclient.doSubscribe(txclient4.getMyUserID(), Subscription.SUBSCRIBE_INTERCEPT_VOICE), DEF_WAIT), "Intercept tx4");

        assertTrue(txclient1.enableVoiceTransmission(true), "tx1 transmit");
        assertTrue(txclient2.enableVoiceTransmission(true), "tx2 transmit");
        assertTrue(txclient3.enableVoiceTransmission(true), "tx3 transmit");
        assertTrue(txclient4.enableVoiceTransmission(true), "tx4 transmit");

        Vector<Integer> ids = new Vector<>();
        ids.add(txclient1.getMyUserID());
        ids.add(txclient2.getMyUserID());
        ids.add(txclient3.getMyUserID());
        ids.add(txclient4.getMyUserID());
        do {
            TTMessage msg = new TTMessage();
            assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "get voice event");
            if ((msg.user.uUserState & UserState.USERSTATE_VOICE) != 0)
                ids.removeElement(msg.user.nUserID);
        } while(ids.size() > 0);

        waitForEvent(rxclient, ClientEvent.CLIENTEVENT_NONE, 2000);

        assertTrue(txclient1.enableVoiceTransmission(false), "tx1 stop transmit");
        assertTrue(txclient2.enableVoiceTransmission(false), "tx2 stop transmit");
        assertTrue(txclient3.enableVoiceTransmission(false), "tx3 stop transmit");
        assertTrue(txclient4.enableVoiceTransmission(false), "tx4 stop transmit");

        ids.add(txclient1.getMyUserID());
        ids.add(txclient2.getMyUserID());
        ids.add(txclient3.getMyUserID());
        ids.add(txclient4.getMyUserID());
        do {
            TTMessage msg = new TTMessage();
            assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "get voice stop event");
            if ((msg.user.uUserState & UserState.USERSTATE_VOICE) == UserState.USERSTATE_NONE)
                ids.removeElement(msg.user.nUserID);
        } while(ids.size()>0);
    }

    @Test
    public void testInitSoundSharedInputOutput() {

        if (GITHUBSKIP) {
            // uses System.currentTimeMillis()
            return;
        }

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
        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "ttclient join channel");

        IntPtr indev = new IntPtr(), outdev = new IntPtr();
        if (INPUTDEVICEID < 0 && OUTPUTDEVICEID < 0)
           assertTrue(ttclient.getDefaultSoundDevices(indev, outdev), "get default devs");
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

            assertTrue(ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID()) == null, "no waiting audio blocks");

            assertTrue(ttclient.enableAudioBlockEvent(Constants.TT_LOCAL_USERID, StreamType.STREAMTYPE_VOICE, true), "enable local aud cb");
            assertTrue(TeamTalkBase.initSoundInputSharedDevice(samplerate, 2, samplerate), "setup shared input settings");
            assertTrue(TeamTalkBase.initSoundOutputSharedDevice(samplerate, 2, samplerate), "setup shared output settings");

            assertTrue(ttclient.initSoundInputDevice(indev.value), "Init "+samplerate+" input");
            assertTrue(ttclient.initSoundOutputDevice(outdev.value), "Init "+samplerate+" output");

            int samples = chan.audiocodec.opus.nSampleRate;
            AudioBlock ab;
            do {
                assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "get "+samplerate+" audioblock");
                assertEquals(Constants.TT_LOCAL_USERID, msg.nSource, "from local");
                ab = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_LOCAL_USERID);
                assertTrue(ab != null, "get local audio block");
                samples -= ab.nSamples;
            } while(samples > 0);

            long initialTS = System.currentTimeMillis();
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "get "+samplerate+" audioblock");
            long nextTS = System.currentTimeMillis();

            assertTrue(ttclient.closeSoundInputDevice(), "close input");
            assertTrue(ttclient.closeSoundOutputDevice(), "close output");
            assertTrue(ttclient.enableAudioBlockEvent(Constants.TT_LOCAL_USERID, StreamType.STREAMTYPE_VOICE, false), "disable local aud cb");
            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 0);
            while(ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, Constants.TT_LOCAL_USERID) != null);

            assertTrue(TeamTalkBase.initSoundInputSharedDevice(0, 0, 0), "reset shared input settings");
            assertTrue(TeamTalkBase.initSoundOutputSharedDevice(0, 0, 0), "reset shared output settings");

            assertTrue(nextTS - initialTS > 900, String.format("next in/out %d callback was %d msec later but should be ~1 sec", samplerate, nextTS - initialTS));
        }

        assertTrue(TeamTalkBase.initSoundInputSharedDevice(0, 0, 0), "reset shared input settings");

        assertTrue(ttclient.initSoundInputDevice(indev.value), "Init input");

        assertTrue(ttclient.enableVoiceTransmission(true), "enable tx");
        assertTrue(waitCmdSuccess(ttclient, ttclient.doSubscribe(ttclient.getMyUserID(), Subscription.SUBSCRIBE_VOICE), DEF_WAIT), "subscribe");

        for (int samplerate : samplerates) {

            if (!supportsOutputSampleRate(getSoundDevice(ttclient, outdev.value & SoundDeviceConstants.TT_SOUNDDEVICE_ID_MASK), samplerate)) {
                System.err.println("Output device doesn't support sample rate: " + samplerate);
                continue;
            }

            assertTrue(ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID()) == null, "no waiting audio blocks");

            assertTrue(ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, true), "enable local aud cb");
            assertTrue(TeamTalkBase.initSoundOutputSharedDevice(samplerate, 2, samplerate), "setup shared output settings");

            assertTrue(ttclient.initSoundOutputDevice(outdev.value), "Init "+samplerate+" output");

            int samples = chan.audiocodec.opus.nSampleRate;
            AudioBlock ab;
            do {
                assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "get "+samplerate+" audioblock");
                assertEquals(ttclient.getMyUserID(), msg.nSource, "from myself");
                ab = ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID());
                assertTrue(ab != null, "get audio block");
                samples -= ab.nSamples;
            } while(samples > 0);

            long initialTS = System.currentTimeMillis();
            assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "get "+samplerate+" audioblock");
            long nextTS = System.currentTimeMillis();

            assertTrue(ttclient.closeSoundOutputDevice(), "close output");
            assertTrue(ttclient.enableAudioBlockEvent(ttclient.getMyUserID(), StreamType.STREAMTYPE_VOICE, false), "disable aud cb");
            waitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 0);
            while(ttclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.getMyUserID()) != null);

            assertTrue(TeamTalkBase.initSoundOutputSharedDevice(0, 0, 0), "reset shared output settings");

            assertTrue(nextTS - initialTS > 900, String.format("next output %d callback was %d msec later but should be ~1 sec", samplerate, nextTS - initialTS));
        }
    }

    @Test
    public void testTransmissionTime() {

        if (GITHUBSKIP) {
            // uses System.currentTimeMillis()
            return;
        }

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_MULTI_LOGIN | UserRight.USERRIGHT_TRANSMIT_VOICE |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);
        Vector<String> files = new Vector<>();

        for (int i = 0; i < 3; i++) {
            TeamTalkBase txclient = newClientInstance();
            TeamTalkBase rxclient = newClientInstance();

            initSound(txclient);
            connect(txclient);
            login(txclient, NICKNAME, USERNAME, PASSWORD);

            initSound(rxclient);
            connect(rxclient);
            login(rxclient, NICKNAME, USERNAME, PASSWORD);

            Channel chan = buildDefaultChannel(txclient, "Opus", Codec.OPUS_CODEC);
            chan.audiocodec.nCodec = Codec.OPUS_CODEC;
            chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_VOIP;
            chan.audiocodec.opus.nTxIntervalMSec = 240;
            chan.audiocodec.opus.nFrameSizeMSec = 10;
            chan.audiocodec.opus.nBitRate = OpusConstants.OPUS_MIN_BITRATE;
            chan.audiocodec.opus.nChannels = 2;
            chan.audiocodec.opus.nComplexity = 10;
            chan.audiocodec.opus.nSampleRate = 48000;
            chan.audiocodec.opus.bDTX = true;
            chan.audiocodec.opus.bFEC = true;
            chan.audiocodec.opus.bVBR = false;
            chan.audiocodec.opus.bVBRConstraint = false;

            assertTrue(waitCmdSuccess(txclient, txclient.doJoinChannel(chan), DEF_WAIT), "txclient join channel");
            assertTrue(waitCmdSuccess(rxclient, rxclient.doJoinChannelByID(txclient.getMyChannelID(), ""), DEF_WAIT), "rxclient join channel");

            assertTrue(txclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 600), "txclient tone");
            String filename = getTestMethodName()+"_"+i;
            files.add(STORAGEFOLDER + File.separator + filename + ".wav");
            assertTrue(rxclient.setUserMediaStorageDir(txclient.getMyUserID(), STORAGEFOLDER, filename, AudioFileFormat.AFF_WAVE_FORMAT), "rxclient storage");

            assertTrue(txclient.enableVoiceTransmission(true), "enable tx");
            /*
             * A duration which ends on a third of a package size, will
             * produce different wav outputs (files are generated which
             * last 1220 ms (5x nTxIntervalMSec) and files are generated
             * which last 1440ms (6x nTxIntervalMSec)
             */
            int duration = (int)(chan.audiocodec.opus.nTxIntervalMSec * 5 + chan.audiocodec.opus.nTxIntervalMSec * 0.33);
            waitForEvent(txclient, ClientEvent.CLIENTEVENT_NONE, duration);
            assertTrue(txclient.enableVoiceTransmission(false), "Disable tx");

            TTMessage msg = new TTMessage();
            while(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg)) {
                if (msg.user.nUserID == txclient.getMyUserID() &&
                    (msg.user.uUserState & UserState.USERSTATE_VOICE) == 0)
                    break;
            }
            txclient.closeTeamTalk();
            rxclient.closeTeamTalk();
        }

        int duration = -1;
        for(String filename : files) {
            MediaFileInfo mfi = new MediaFileInfo();
            assertTrue(TeamTalkBase.getMediaFileInfo(filename, mfi), "Get media file info");
            if (duration < 0)
                duration = mfi.uDurationMSec;
            assertEquals(duration, mfi.uDurationMSec, "same file sizes");
        }
    }

    @Test
    public void testMultiLoginDeny() {
        String USERNAME = "tt_test", PASSWORD = "tt_test";
        int USERRIGHTS = UserRight.USERRIGHT_NONE;
        makeUserAccount(getTestMethodName(), USERNAME, PASSWORD, USERRIGHTS);

        // test when client1 is in channel
        TeamTalkBase client1 = newClientInstance();
        connect(client1);
        login(client1, "User1 " + getTestMethodName(), USERNAME, PASSWORD);
        joinRoot(client1);

        TeamTalkBase client2 = newClientInstance();
        connect(client2);
        login(client2, "User2 " + getTestMethodName(), USERNAME, PASSWORD);

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDOUT, DEF_WAIT), "client1 logged out");

        joinRoot(client2);

        assertTrue(client1.disconnect(), "disconnect1");
        assertTrue(client2.disconnect(), "disconnect2");

        // test when client1 is not in channel
        connect(client1);
        login(client1, "User1 " + getTestMethodName(), USERNAME, PASSWORD);

        connect(client2);
        login(client2, "User2 " + getTestMethodName(), USERNAME, PASSWORD);

        assertTrue(waitForEvent(client1, ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDOUT, DEF_WAIT), "client1 logged out");

        joinRoot(client2);
    }

    @Test
    public void testJoinChannel() {
        String USERNAME = "tt_test1", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_MODIFY_CHANNELS;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        // try joining new sub channel in existing channel (requires USERRIGHT_MODIFY_CHANNELS)
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        Channel chan = buildDefaultChannel(ttclient, "SubTest", Codec.OPUS_CODEC);
        chan.uChannelType |= ChannelType.CHANNEL_PERMANENT;
        assertTrue(waitCmdSuccess(ttclient, ttclient.doMakeChannel(chan), DEF_WAIT), "Make sub");
        int subchanid = ttclient.getChannelIDFromPath("/" + chan.szName);

        chan = buildDefaultChannel(ttclient, "SubSubTest", Codec.OPUS_CODEC);
        chan.nParentID = subchanid;

        assertTrue(waitCmdSuccess(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join new sub with USERRIGHT_MODIFY_CHANNELS");
        assertTrue(ttclient.disconnect(), "disconnect");

        // try joining new sub channel in existing channel (requires USERRIGHT_MODIFY_CHANNELS)

        USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        chan = buildDefaultChannel(ttclient, "SubSubTest", Codec.OPUS_CODEC);
        chan.nParentID = subchanid;

        assertTrue(waitCmdError(ttclient, ttclient.doJoinChannel(chan), DEF_WAIT), "join new sub with USERRIGHT_CREATE_TEMPORARY_CHANNEL");
    }

    @Test
    public void testLocalPlaybackAndroidCrash() {

        TeamTalkBase ttclient = newClientInstance();
        initSound(ttclient);

        MediaFileInfo mfi = new MediaFileInfo();
        mfi.szFileName = STORAGEFOLDER + File.separator + "hest.wav";
        mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 2);
        mfi.uDurationMSec = 1000;
        assertTrue(TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600), "Write media file");

        // Call TT_InitLocalPlayback for file 1, PAUSE=FALSE
        MediaFilePlayback mfp = new MediaFilePlayback();
        mfp.bPaused = false;
        mfp.uOffsetMSec = MediaFilePlaybackConstants.TT_MEDIAPLAYBACK_OFFSET_IGNORE;

        int sessionid = ttclient.initLocalPlayback(mfi.szFileName, mfp);
        assertTrue(sessionid > 0, "init playback");

        TTMessage msg = new TTMessage();
        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg) &&
               msg.mediafileinfo.nStatus != MediaFileStatus.MFS_FINISHED);

        // Call TT_InitLocalPlayback for file 1, PAUSE=TRUE (session = X)
        mfp.bPaused = true;
        sessionid = ttclient.initLocalPlayback(mfi.szFileName, mfp);

        // Call TT_InitLocalPlayback for file 2, PAUSE=FALSE
        mfp.bPaused = false;
        int sessionid2 = ttclient.initLocalPlayback(mfi.szFileName, mfp);
        assertTrue(sessionid2 > 0, "init playback");
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg), "media started");
        assertEquals(sessionid2, msg.nSource, "session 2 started");
        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "media started event");
        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg), "media playing");
        assertEquals(MediaFileStatus.MFS_PLAYING, msg.mediafileinfo.nStatus, "media playing event");
        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg) &&
               msg.mediafileinfo.nStatus != MediaFileStatus.MFS_FINISHED) {
            assertEquals(sessionid2, msg.nSource, "session 2 playing");
        }

        // Call TT_InitLocalPlayback for file 2, PAUSE=TRUE
        mfp.bPaused = true;
        sessionid2 = ttclient.initLocalPlayback(mfi.szFileName, mfp);

        // Call TT_UpdateLocalPlayback for session X => Crash
        mfp.bPaused = false;
        assertTrue(ttclient.updateLocalPlayback(sessionid, mfp), "Pause");
        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, DEF_WAIT, msg)) {
            if (msg.nSource == sessionid && msg.mediafileinfo.nStatus == MediaFileStatus.MFS_FINISHED)
                break;
        }
    }

    @Test
    public void testRecordingMuxedStreams() {

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_MULTI_LOGIN |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TTMessage msg = new TTMessage();

        int freq = 300;
        TeamTalkBase ttclient;

        ttclient = newClientInstance();
        initSound(ttclient);
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, freq);

        Channel chan = new Channel();
        ttclient.getChannel(ttclient.getMyChannelID(), chan);
        String recfile = String.format("%s.wav", getTestMethodName());
        assertTrue(ttclient.startRecordingMuxedStreams(StreamType.STREAMTYPE_VOICE,
                                                                                  chan.audiocodec,
                                                                                  recfile,
                                                                                  AudioFileFormat.AFF_WAVE_FORMAT), "Record muxed audio file");
        assertTrue(ttclient.enableVoiceTransmission(true), "enable voice tx");


        assertTrue(ttclient.stopRecordingMuxedAudioFile(), "Stop recording muxed audio file");

        ttclient.enableVoiceTransmission(false);
    }

    @Test
    public void testMediaStreamRestart() {
        TeamTalkBase ttclient = newClientInstance();

        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TTMessage msg = new TTMessage();

        initSound(ttclient);
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        joinRoot(ttclient);

        MediaFileInfo mfi = new MediaFileInfo();
        String filename = STORAGEFOLDER + File.separator + "hest.wav";
        mfi.szFileName = filename;
        mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 2);
        mfi.uDurationMSec = 1000;
        assertTrue(TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600), "Write media file");
        assertTrue(ttclient.getMediaFileInfo(filename, mfi), "Get media file info");

        // without bRestartable the stream cannot use
        // updateStreamingMediaFileToChannel() after MFS_FINISHED
        MediaFilePlayback mfp = new MediaFilePlayback();
        mfp.uOffsetMSec = (int)(mfi.uDurationMSec * 0.9);
        mfp.bPaused = false;

        assertTrue(ttclient.startStreamingMediaFileToChannel(filename, null), "Start media stream");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg), "Wait stream event");

        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "Begin stream");

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg) &&
               msg.mediafileinfo.nStatus != MediaFileStatus.MFS_FINISHED);
        assertFalse(ttclient.updateStreamingMediaFileToChannel(mfp, null), "Stream dead");

        assertTrue(ttclient.startStreamingMediaFileToChannel(filename, null), "Start immediately after");

        assertTrue(waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg), "Wait stream event");

        assertEquals(MediaFileStatus.MFS_STARTED, msg.mediafileinfo.nStatus, "Begin stream");

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, DEF_WAIT, msg) &&
               msg.mediafileinfo.nStatus != MediaFileStatus.MFS_FINISHED);
    }

    @Test
    public void testUserStateVoice() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase client = newClientInstance();
        initSound(client);
        connect(client);
        login(client, NICKNAME, USERNAME, PASSWORD);

        Channel chan = buildDefaultChannel(client, "New channel", Codec.OPUS_CODEC);
        chan.audiocodec.opus.nFrameSizeMSec = 5;
        chan.audiocodec.opus.nTxIntervalMSec = 10;
        assertTrue(waitCmdSuccess(client, client.doJoinChannel(chan), DEF_WAIT), "join channel");

        assertTrue(client.getChannel(client.getMyChannelID(), chan), "get new chan");

        assertTrue(client.setUserStoppedPlaybackDelay(client.getMyUserID(),
                                                                                     StreamType.STREAMTYPE_VOICE,
                                                                                     chan.audiocodec.opus.nTxIntervalMSec), "stopped talking delay voice");
        assertTrue(waitCmdSuccess(client, client.doSubscribe(client.getMyUserID(),
                                                                                Subscription.SUBSCRIBE_VOICE), DEF_WAIT), "subscribe voice");
        TTMessage msg = new TTMessage();

        assertTrue(client.enableVoiceTransmission(true), "vox");

        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "User state changed to voice");
        assertEquals(UserState.USERSTATE_VOICE, msg.user.uUserState, "User is talking");

        assertTrue(client.enableVoiceTransmission(false), "vox disable");

        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "User state changed to not voice");
        assertEquals(UserState.USERSTATE_NONE, msg.user.uUserState, "User is not talking");

        assertTrue(client.enableVoiceTransmission(true), "vox new stream");

        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "User state changed to voice on new stream");
        assertEquals(UserState.USERSTATE_VOICE, msg.user.uUserState, "User is talking on new stream");

        assertTrue(client.enableVoiceTransmission(false), "vox disable new stream");

        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "User state changed to not voice on new stream");
        assertEquals(UserState.USERSTATE_NONE, msg.user.uUserState, "User is not talking on new stream");
    }

    @Test
    public void testUserStateMediaFile() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_MEDIAFILE | UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase client = newClientInstance();
        initSound(client);
        connect(client);
        login(client, NICKNAME, USERNAME, PASSWORD);

        Channel chan = buildDefaultChannel(client, "New channel", Codec.OPUS_CODEC);
        chan.audiocodec.opus.nFrameSizeMSec = 5;
        chan.audiocodec.opus.nTxIntervalMSec = 10;
        assertTrue(waitCmdSuccess(client, client.doJoinChannel(chan), DEF_WAIT), "join channel");

        assertTrue(client.getChannel(client.getMyChannelID(), chan), "get new chan");

        assertTrue(client.setUserStoppedPlaybackDelay(client.getMyUserID(),
                                                                                  StreamType.STREAMTYPE_MEDIAFILE_AUDIO,
                                                                                  chan.audiocodec.opus.nTxIntervalMSec), "stopped talking delay mf");
        TTMessage msg = new TTMessage();

        MediaFileInfo mfi = new MediaFileInfo();
        mfi.szFileName = STORAGEFOLDER + File.separator + "tot.wav";
        mfi.audioFmt = new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, 48000, 1);
        mfi.uDurationMSec = 5 * 1000;

        assertTrue(TeamTalkBase.DBG_WriteAudioFileTone(mfi, 600), "Write media file");

        assertTrue(client.startStreamingMediaFileToChannel(mfi.szFileName, new VideoCodec()), "Start stream file");

        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "User state changed to media file");
        assertEquals(UserState.USERSTATE_MEDIAFILE_AUDIO, (msg.user.uUserState & UserState.USERSTATE_MEDIAFILE_AUDIO), "User is streaming");

        assertTrue(client.stopStreamingMediaFileToChannel(), "Stop streaming");

        assertTrue(waitForEvent(client, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, msg), "User state changed to not streaming");
        assertEquals(UserState.USERSTATE_NONE, (msg.user.uUserState & UserState.USERSTATE_MEDIAFILE_AUDIO), "User is not streaming");
    }

    @Test
    public void testUserAcountLastLogin() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_MEDIAFILE | UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase client = newClientInstance();
        connect(client);
        login(client, NICKNAME, USERNAME, PASSWORD);

        UserAccount first_login_account = new UserAccount();
        assertTrue(client.getMyUserAccount(first_login_account), "get account");
        //assertEquals("1970/01/01 00:00", first_login_account.szLastLoginTime);

        assertTrue(client.disconnect(), "disconnect");
        connect(client);
        login(client, NICKNAME, USERNAME, PASSWORD);

        UserAccount second_login_account = new UserAccount();
        assertTrue(client.getMyUserAccount(second_login_account), "get account again");
        //assertNotEquals("1970/01/01 00:00", second_login_account.szLastLoginTime);
        assertNotEquals(first_login_account.szLastLoginTime, second_login_account.szLastLoginTime);
    }

    @Test
    public void testTimeZone() {
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_MEDIAFILE | UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        SimpleDateFormat fmt = new SimpleDateFormat("yyyy/MM/dd HH:mm");
        Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("UTC"));
        calendar.set(1970, Calendar.JANUARY, 1, 0, 0, 0);

        TeamTalkBase client = newClientInstance();
        connect(client);
        login(client, NICKNAME, USERNAME, PASSWORD);

        UserAccount first_login_account = new UserAccount();
        assertTrue(client.getMyUserAccount(first_login_account), "get account");
        assertEquals(fmt.format(calendar.getTime()), first_login_account.szLastLoginTime, "must be 1970/01/01 in local time");

        assertTrue(client.disconnect(), "disconnect");
        connect(client);
        login(client, NICKNAME, USERNAME, PASSWORD);

        UserAccount second_login_account = new UserAccount();
        assertTrue(client.getMyUserAccount(second_login_account), "get account again");
        Date logintime = fmt.parse(second_login_account.szLastLoginTime, new ParsePosition(0));
        Date now = new Date();
        long diff = now.getTime() - logintime.getTime();
        diff /= 1000;
        assertEquals(diff, 0, 120, "time stamp match within 2 minutes");
    }

    @Test
    public void testAutoOperatorRemoval() {

        TeamTalkBase ttadmin = newClientInstance();
        connect(ttadmin);
        login(ttadmin, ADMIN_NICKNAME + " - " + getTestMethodName(), ADMIN_USERNAME, ADMIN_PASSWORD);

        // create permanent channel for auto-operator
        Channel chan = buildDefaultChannel(ttadmin, "Some channel");
        chan.uChannelType |= ChannelType.CHANNEL_PERMANENT;
        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doMakeChannel(chan), DEF_WAIT), "Make channel");
        int chanid = ttadmin.getChannelIDFromPath("/" + chan.szName);

        // create user acount where permanent channel is account's auto-operator channel
        final String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_NONE;
        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = USERNAME;
        useraccount.szPassword = PASSWORD;
        useraccount.uUserRights = USERRIGHTS;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        useraccount.autoOperatorChannels[0] = chanid;
        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doNewUserAccount(useraccount), DEF_WAIT), "New user account ok");

        // see that auto-operator is now set
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        useraccount = new UserAccount();
        assertTrue(ttclient.getMyUserAccount(useraccount), "get my account");
        assertEquals(chanid, useraccount.autoOperatorChannels[0], "auto operator channel set");
        assertTrue(waitCmdSuccess(ttclient, ttclient.doLogout(), DEF_WAIT));

        // remove permanent channel
        assertTrue(waitCmdSuccess(ttadmin, ttadmin.doRemoveChannel(chanid), DEF_WAIT));

        // permanent channel should now have been removed as auto-operator channel
        login(ttclient, NICKNAME, USERNAME, PASSWORD);
        useraccount = new UserAccount();
        assertTrue(ttclient.getMyUserAccount(useraccount), "get my account");
        assertEquals(0, useraccount.autoOperatorChannels[0], "auto operator channel removed");
    }

    /* cannot test output levels since a user is muted by sound system after decoding and callback.

    @Test
    public void testSharedAudioInputOutputLevels() {

        String USERNAME = "tt_test", PASSWORD = "tt_test", NICKNAME = "jUnit - " + getTestMethodName();
        int USERRIGHTS = UserRight.USERRIGHT_MULTI_LOGIN | UserRight.USERRIGHT_TRANSMIT_VOICE |
            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
        makeUserAccount(NICKNAME, USERNAME, PASSWORD, USERRIGHTS);

        TeamTalkBase rxclient = newClientInstance();

        IntPtr indev = new IntPtr(), outdev = new IntPtr();
        if(INPUTDEVICEID < 0 && OUTPUTDEVICEID < 0)
           assertTrue(rxclient.getDefaultSoundDevices(indev, outdev), "get default devs");
        else
        {
            indev.value = INPUTDEVICEID;
            outdev.value = OUTPUTDEVICEID;
        }

        indev.value |= SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;
        outdev.value |= SoundDeviceConstants.TT_SOUNDDEVICE_ID_SHARED_FLAG;

        assertTrue(rxclient.initSoundInputDevice(indev.value), "Init rx input");
        assertTrue(rxclient.initSoundOutputDevice(outdev.value), "Init rx output");
        connect(rxclient);
        login(rxclient, NICKNAME, USERNAME, PASSWORD);
        Channel chan = buildDefaultChannel(rxclient, "Opus_12khz", Codec.OPUS_CODEC);
        chan.audiocodec.opus.nSampleRate = 12000;
        chan.audiocodec.opus.nChannels = 2;
        chan.audiocodec.opus.nTxIntervalMSec = 20;
        chan.audiocodec.opus.nFrameSizeMSec = 5;
        assertTrue(waitCmdSuccess(rxclient, rxclient.doJoinChannel(chan), DEF_WAIT), "rxclient join channel");

        TeamTalkBase txclient1 = newClientInstance();
        assertTrue(txclient1.initSoundInputDevice(indev.value), "Init tx1 input");
        assertTrue(txclient1.initSoundOutputDevice(outdev.value), "Init tx1 output");
        connect(txclient1);
        login(txclient1, NICKNAME, USERNAME, PASSWORD);
        assertTrue(waitCmdSuccess(txclient1, txclient1.doJoinChannelByID(rxclient.getMyChannelID(), ""), DEF_WAIT), "tx2 join existing");
        assertTrue(txclient1.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 300), "Gen tone tx1");

        assertTrue(rxclient.enableAudioBlockEvent(txclient1.getMyUserID(), StreamType.STREAMTYPE_VOICE, true), "enable callback");
        assertTrue(rxclient.setUserMute(txclient1.getMyUserID(), StreamType.STREAMTYPE_VOICE, true), "mute user");

        assertTrue(txclient1.enableVoiceTransmission(true), "tx1 start transmit");

        TTMessage msg = new TTMessage();

        int count = 5;
        do {
            assertTrue(waitForEvent(rxclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, msg), "gimme voice audioblock");
            AudioBlock block = rxclient.acquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, txclient1.getMyUserID());
            assertTrue(block != null, "got audio block");
            assertEquals(2, block.nChannels, "stereo block");
            short[] audio = audioToShortArray(block.lpRawAudio);
            int max = 0;
            for(int i=0;i<audio.length;i++) {
                assertEquals(0, audio[i], "Muted user");
            }
        } while (count-- > 0);

        assertTrue(waitCmdSuccess(rxclient, rxclient.doLeaveChannel(), DEF_WAIT), "rxclient leave channel");
    }
    */
}
