using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using System.Drawing;
using System.Diagnostics;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using BearWare;
using System.Drawing.Imaging;
using System.Runtime.CompilerServices;

namespace TeamTalkTest.NET
{
    [TestClass]
    public class TeamTalkTest
    {
        const bool DEBUG_OUTPUT = false;

        const int DEF_WAIT = 15000;
        const string ADMIN_USERNAME = "", ADMIN_PASSWORD = "", ADMIN_NICKNAME = "Admin";
        const string IPADDR = "";
        //const string IPADDR = "localhost";
        const int TCPPORT = 10333, UDPPORT = 10333;
        const bool ENCRYPTED = false;

        const string MUXRECORDFILENAME = "";
        const string MEDIAFOLDER = "";
        const string MEDIAFILE_AUDIO = "";
        const string MEDIAFILE_VIDEO = "";
        const string UPLOADFILE = "";
        const string DOWNLOADFILE = "";

        List<TeamTalk> ttclients = new List<TeamTalk>();

        public TeamTalkTest()
        {
            TeamTalk.SetLicenseInformation("", "");
        }

        [TestInitialize]
        public void SetUp()
        {
        }

        [TestCleanup]
        public void TearDown()
        {
            foreach (TeamTalk ttclient in ttclients)
            {
                ttclient.Disconnect();
                ttclient.CloseSoundDuplexDevices();
                ttclient.CloseSoundInputDevice();
                ttclient.CloseSoundOutputDevice();
                ttclient.CloseVideoCaptureDevice();
                TTMessage msg = new TTMessage();
                while (ttclient.GetMessage(ref msg, 0)) ;
            }
            ttclients.Clear();
        }

        [TestMethod]
        public void TestStatic()
        {
            Assert.IsTrue(TeamTalk.GetVersion().Length > 0, "Test version info");
        }

        [TestMethod]
        public void TestSoundInit()
        {
            System.GC.Collect();
            TeamTalk ttclient = NewClientInstance();

            int devin = 0, devout = 0;
            Assert.IsTrue(TeamTalk.GetDefaultSoundDevices(ref devin, ref devout), "Get default devices");

            Assert.IsTrue(ttclient.InitSoundInputDevice(devin), "Init sound input");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_SNDINPUT_READY), "Input ready");

            Assert.IsTrue(ttclient.InitSoundOutputDevice(devout), "Init sound output");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_SNDOUTPUT_READY), "Output ready");

            Assert.IsTrue(ttclient.CloseSoundOutputDevice(), "close snd out");
            Assert.IsTrue(ttclient.CloseSoundInputDevice(), "close snd in");

            Assert.IsTrue(TeamTalk.GetDefaultSoundDevicesEx(SoundSystem.SOUNDSYSTEM_DSOUND, ref devin, ref devout),
                          "get dpx for dsound");

            Assert.IsTrue(ttclient.InitSoundDuplexDevices(devin, devout), "init dpx");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX), "dpx ready");

            Assert.IsTrue(ttclient.CloseSoundDuplexDevices(), "close dpx");

            Assert.IsTrue(ttclient.Flags == ClientFlag.CLIENT_CLOSED);

            Assert.IsTrue(TeamTalk.RestartSoundSystem(), "restart snd sys");

            Assert.IsTrue(TeamTalk.GetDefaultSoundDevices(ref devin, ref devout), "Get default devices");

            Assert.IsTrue(ttclient.InitSoundInputDevice(devin), "Init sound input");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_SNDINPUT_READY), "Input ready");

            Assert.IsTrue(ttclient.InitSoundOutputDevice(devout), "Init sound output");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_SNDOUTPUT_READY), "Output ready");

            Assert.IsTrue(ttclient.CloseSoundOutputDevice(), "close snd out");
            Assert.IsTrue(ttclient.CloseSoundInputDevice(), "close snd in");
        }

        [TestMethod]
        public void TestSoundDevices()
        {
            SoundDevice[] devs;
            Assert.IsTrue(TeamTalk.GetSoundDevices(out devs));
            foreach (SoundDevice s in devs)
            {
                Assert.IsTrue(s.nDefaultSampleRate > 0);
                Assert.IsTrue(s.szDeviceName.Length > 0);
                Assert.IsTrue(s.nSoundSystem != SoundSystem.SOUNDSYSTEM_NONE);
            }

            int devin = 0, devout = 0;
            Assert.IsTrue(TeamTalk.GetDefaultSoundDevices(ref devin, ref devout), "Get default devices");
            SoundDevice sin = devs.First<SoundDevice>(m => m.nDeviceID == devin);
            SoundDevice sout = devs.First<SoundDevice>(m => m.nDeviceID == devout);

            int minChan = Math.Min(sin.nMaxInputChannels, sout.nMaxOutputChannels);

            IntPtr sndloop = TeamTalk.StartSoundLoopbackTest(devin, devout, sin.nDefaultSampleRate, minChan, false, new SpeexDSP());
            Assert.IsTrue(sndloop != IntPtr.Zero,
                            "Start loopback test");

            Assert.IsTrue(TeamTalk.CloseSoundLoopbackTest(sndloop), "Stop loopback test");

            Assert.IsTrue(TeamTalk.GetDefaultSoundDevicesEx(SoundSystem.SOUNDSYSTEM_DSOUND,
                                                             ref devin, ref devout), "Get default DSound devices");

            sin = devs.First<SoundDevice>(m => m.nDeviceID == devin);
            sout = devs.First<SoundDevice>(m => m.nDeviceID == devout);

            SpeexDSP spxdsp = new SpeexDSP();
            spxdsp.bEnableAGC = SpeexDSPConstants.DEFAULT_AGC_ENABLE;
            spxdsp.nGainLevel = SpeexDSPConstants.DEFAULT_AGC_GAINLEVEL;
            spxdsp.nMaxIncDBSec = SpeexDSPConstants.DEFAULT_AGC_INC_MAXDB;
            spxdsp.nMaxDecDBSec = SpeexDSPConstants.DEFAULT_AGC_DEC_MAXDB;
            spxdsp.nMaxGainDB = SpeexDSPConstants.DEFAULT_AGC_GAINMAXDB;

            sndloop = TeamTalk.StartSoundLoopbackTest(sin.nDeviceID, sout.nDeviceID, sin.nDefaultSampleRate, 1, true, spxdsp);
            Assert.IsTrue(sndloop != IntPtr.Zero,
                          "Testing duplex with AGC");

            Assert.IsTrue(TeamTalk.CloseSoundLoopbackTest(sndloop), "Stop duplex test with AGC");

            spxdsp.bEnableAGC = false;

            spxdsp.bEnableDenoise = SpeexDSPConstants.DEFAULT_DENOISE_ENABLE;
            spxdsp.nMaxNoiseSuppressDB = SpeexDSPConstants.DEFAULT_DENOISE_SUPPRESS;

            sndloop = TeamTalk.StartSoundLoopbackTest(sin.nDeviceID, sout.nDeviceID, sin.nDefaultSampleRate, 1, true, spxdsp);
            Assert.IsTrue(IntPtr.Zero != sndloop,
                          "Testing duplex with denoise");

            Assert.IsTrue(TeamTalk.CloseSoundLoopbackTest(sndloop), "Stop duplex test with denoise");

            spxdsp.bEnableDenoise = false;

            spxdsp.bEnableEchoCancellation = true;
            spxdsp.nEchoSuppress = SpeexDSPConstants.DEFAULT_ECHO_SUPPRESS;
            spxdsp.nEchoSuppressActive = SpeexDSPConstants.DEFAULT_ECHO_SUPPRESS_ACTIVE;

            sndloop = TeamTalk.StartSoundLoopbackTest(sin.nDeviceID, sout.nDeviceID, sin.nDefaultSampleRate, 1, true, spxdsp);
            Assert.IsTrue(IntPtr.Zero != sndloop,
                          "Testing duplex with AEC");

            Assert.IsTrue(TeamTalk.CloseSoundLoopbackTest(sndloop), "Stop duplex test with AEC");

            spxdsp.bEnableAGC = spxdsp.bEnableDenoise = spxdsp.bEnableEchoCancellation = true;

            sndloop = TeamTalk.StartSoundLoopbackTest(sin.nDeviceID, sout.nDeviceID, sin.nDefaultSampleRate, 1, true, spxdsp);
            Assert.IsTrue(IntPtr.Zero != sndloop,
                          "Testing duplex with AGC, AEC, denoise");

            Assert.IsTrue(TeamTalk.CloseSoundLoopbackTest(sndloop), "Stop duplex test with AGC, AEC, denoise");
        }

        [TestMethod]
        public void TestVoice()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            InitSound(ttclient);
            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            JoinRoot(ttclient);

            Assert.IsTrue(ttclient.EnableVoiceActivation(true), "Enable voice act");
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_VOICE_ACTIVATION, 1000), "voice act event");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_SNDINPUT_VOICEACTIVATED), "voice act flag");
            Assert.IsTrue(ttclient.SetVoiceActivationLevel(0), "set voice act level");
            Assert.AreEqual(0, ttclient.GetVoiceActivationLevel(), "get voice act level");
            Assert.IsTrue(ttclient.SetVoiceActivationStopDelay(2000), "set voice act stop delay");
            Assert.AreEqual(2000, ttclient.GetVoiceActivationStopDelay(), "get voice act stop delay");

            Assert.IsTrue(ttclient.GetSoundInputLevel() >= 0, "GetSoundInputLevel ok");

            ttclient.SetSoundInputGainLevel(SoundLevel.SOUND_GAIN_MAX / 2);
            Assert.AreEqual(SoundLevel.SOUND_GAIN_MAX / 2, ttclient.GetSoundInputGainLevel(), "Gain equal");

            ttclient.SetSoundOutputVolume(SoundLevel.SOUND_VOLUME_MAX / 2);
            Assert.AreEqual(SoundLevel.SOUND_VOLUME_MAX / 2, ttclient.GetSoundOutputVolume(), "out vol same");

            ttclient.SetSoundOutputMute(true);
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_SNDOUTPUT_MUTE), "mute out");
            ttclient.SetSoundOutputMute(false);


            Assert.IsTrue(ttclient.Enable3DSoundPositioning(true), "3d check");
            Assert.IsTrue(ttclient.AutoPositionUsers(), "auto pos 3d ok");

            Assert.IsTrue(ttclient.EnableVoiceActivation(false), "false voice act");

            Assert.IsFalse(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 0)); //drain msg q

            Assert.IsFalse(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 100), "no audio block");

            Assert.IsTrue(ttclient.EnableAudioBlockEvent(0, StreamType.STREAMTYPE_VOICE, true), "audio block");

            TTMessage msg = new TTMessage();
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000, ref msg), "audio block event");
            Assert.AreEqual(0, msg.nSource);
            AudioBlock audblk = ttclient.AcquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
            Assert.IsTrue(audblk.nSamples > 0, "samples audio block");
            Assert.IsTrue(audblk.nChannels > 0, "channels audio block");
            Assert.IsTrue(audblk.nSampleRate > 0, "sample rate audio block");
            Assert.AreNotEqual(null, audblk.lpRawAudio, "data audio block");

            Assert.AreEqual(0, msg.nSource, "audio block from local instance");

            Assert.IsTrue(ttclient.ReleaseUserAudioBlock(audblk), "release audio block");

            Assert.IsTrue(ttclient.EnableVoiceTransmission(true), "voice tx enable");

            User user = new User();
            Assert.IsTrue(ttclient.GetUser(ttclient.GetMyUserID(), ref user), "get myself");

            Assert.IsFalse(user.uUserState.HasFlag(UserState.USERSTATE_VOICE), "No voice received");

            int cmdid = ttclient.DoSubscribe(ttclient.GetMyUserID(), Subscription.SUBSCRIBE_VOICE);
            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, DEF_WAIT), "subscribe voice");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, ref msg), "user state change");

            user = (User)msg.DataToObject();
            Assert.IsTrue(user.uUserState.HasFlag(UserState.USERSTATE_VOICE), "user state has voice");

            Assert.IsTrue(ttclient.EnableVoiceTransmission(false), "voice tx disable");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, ref msg), "user state change");
            user = (User)msg.DataToObject();
            Assert.IsFalse(user.uUserState.HasFlag(UserState.USERSTATE_VOICE), "user state has no voice");

            Assert.IsTrue(ttclient.EnableVoiceActivation(true), "Enable voice act");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, ref msg), "user state change");

            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_SNDINPUT_VOICEACTIVATED), "voice act flag");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_SNDINPUT_VOICEACTIVE), "voice active flag");

            user = (User)msg.DataToObject();
            Assert.IsTrue(user.uUserState.HasFlag(UserState.USERSTATE_VOICE), "user state has voice");

            Assert.IsTrue(ttclient.SetUserAudioStreamBufferSize(user.nUserID, StreamType.STREAMTYPE_VOICE, 3000));
            Assert.IsTrue(ttclient.GetUser(user.nUserID, ref user));
            Assert.AreEqual(3000, user.nBufferMSecVoice);
        }

        [TestMethod]
        public void TestRecordAudio()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VOICE;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            InitSound(ttclient);
            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            JoinRoot(ttclient);
            
            TTMessage msg = new TTMessage();

            Channel chan = new Channel();
            Assert.IsTrue(ttclient.GetChannel(ttclient.GetMyChannelID(), ref chan), "get channel");
            Assert.IsTrue(ttclient.StartRecordingMuxedAudioFile(chan.audiocodec, MUXRECORDFILENAME, AudioFileFormat.AFF_WAVE_FORMAT),
                          "Record to file");

            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_MUX_AUDIOFILE), "Mux record enabled");

            Assert.IsTrue(ttclient.EnableVoiceTransmission(true), "voice tx disable");

            int cmdid = ttclient.DoSubscribe(ttclient.GetMyUserID(), Subscription.SUBSCRIBE_VOICE);
            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, DEF_WAIT), "subscribe voice");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, 2000, ref msg), "user state change");

            User user = (User)msg.DataToObject();
            Assert.IsTrue(user.uUserState.HasFlag(UserState.USERSTATE_VOICE), "user state has voice");

            Assert.IsTrue(ttclient.SetUserMediaStorageDir(ttclient.GetMyUserID(), MEDIAFOLDER, "", AudioFileFormat.AFF_WAVE_FORMAT),
                          "Record user audio files");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 1000, ref msg), "Record media file event");

            MediaFileInfo fileinfo = (MediaFileInfo)msg.DataToObject();
            Assert.AreEqual(ttclient.GetMyUserID(), user.nUserID, "Self record voice");
            Assert.IsTrue(fileinfo.nStatus == MediaFileStatus.MFS_STARTED, "Started recording");

            Assert.IsTrue(ttclient.SetUserStoppedPlaybackDelay(ttclient.GetMyUserID(), StreamType.STREAMTYPE_VOICE, 2000),
                          "Stop voice stopped delay");
            Assert.IsTrue(ttclient.SetUserVolume(ttclient.GetMyUserID(), StreamType.STREAMTYPE_VOICE, SoundLevel.SOUND_VOLUME_MAX / 2));
            Assert.IsTrue(ttclient.SetUserVolume(ttclient.GetMyUserID(), StreamType.STREAMTYPE_MEDIAFILE_AUDIO, SoundLevel.SOUND_VOLUME_MAX / 2));
            Assert.IsTrue(ttclient.SetUserMute(ttclient.GetMyUserID(), StreamType.STREAMTYPE_VOICE, true));
            Assert.IsTrue(ttclient.SetUserMute(ttclient.GetMyUserID(), StreamType.STREAMTYPE_MEDIAFILE_AUDIO, true));
            Assert.IsTrue(ttclient.SetUserStereo(ttclient.GetMyUserID(), StreamType.STREAMTYPE_MEDIAFILE_AUDIO, true, false));
            Assert.IsTrue(ttclient.SetUserStereo(ttclient.GetMyUserID(), StreamType.STREAMTYPE_VOICE, false, true));

            Assert.IsTrue(ttclient.EnableVoiceTransmission(false), "voice disable");

            Assert.IsFalse(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, 1000, ref msg), "no user state change");

            Assert.IsTrue(ttclient.GetUser(ttclient.GetMyUserID(), ref user), "Get self");
            Assert.AreEqual(2000, user.nStoppedDelayVoice, "Voice stopped delay eq 2000");
            Assert.AreEqual(SoundLevel.SOUND_VOLUME_MAX / 2, user.nVolumeVoice);
            Assert.AreEqual(SoundLevel.SOUND_VOLUME_MAX / 2, user.nVolumeMediaFile);
            Assert.IsTrue(user.uUserState.HasFlag(UserState.USERSTATE_MUTE_VOICE));
            Assert.IsTrue(user.uUserState.HasFlag(UserState.USERSTATE_MUTE_MEDIAFILE));
            Assert.AreEqual(false, user.stereoPlaybackVoice[0]);
            Assert.AreEqual(true, user.stereoPlaybackVoice[1]);
            Assert.AreEqual(true, user.stereoPlaybackMediaFile[0]);
            Assert.AreEqual(false, user.stereoPlaybackMediaFile[1]);

            Assert.IsTrue(user.uUserState.HasFlag(UserState.USERSTATE_VOICE), "Voice active");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 3000, ref msg), "Record media file event stop");
            fileinfo = (MediaFileInfo)msg.DataToObject();

            Assert.IsTrue(fileinfo.nStatus == MediaFileStatus.MFS_FINISHED, "Ended recording");

            Assert.IsTrue(ttclient.GetUser(ttclient.GetMyUserID(), ref user), "Get self");
            Assert.IsFalse(user.uUserState.HasFlag(UserState.USERSTATE_VOICE), "Voice inactive");

            Assert.IsTrue(ttclient.StopRecordingMuxedAudioFile(), "Stop mux record");
        }

        [TestMethod]
        public void TestVideoCapture()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_VIDEOCAPTURE;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            VideoCaptureDevice[] devs;
            Assert.IsTrue(TeamTalk.GetVideoCaptureDevices(out devs), "get video devs");

            Assert.IsTrue(devs.Length > 0, "Video devs available");

            VideoCaptureDevice dev = devs[0];

            Assert.IsTrue(ttclient.InitVideoCaptureDevice(dev.szDeviceID, dev.videoFormats[0]),
                          "Init video capture device");

            TTMessage msg = new TTMessage();
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_VIDEOCAPTURE, 10000, ref msg),
                          "Get Video capture frame");
            Assert.AreEqual(0, msg.nSource, "Video from self");

            Bitmap bmp = null;
            int counter = 0;

            VideoFrame frm = ttclient.AcquireUserVideoCaptureFrame(0, out bmp);
            Assert.AreNotEqual(0, frm.nHeight, "Valid video frame");
            Assert.AreEqual(dev.videoFormats[0].nWidth, frm.nWidth);
            Assert.AreEqual(dev.videoFormats[0].nHeight, frm.nHeight);

            bmp.Save(MEDIAFOLDER + "\\" + counter++ + ".bmp");
            
            Assert.IsTrue(ttclient.ReleaseUserVideoCaptureFrame(frm), "release video frame");

            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            JoinRoot(ttclient);

            Assert.IsTrue(ttclient.CloseVideoCaptureDevice(), "Close vid dev");

            VideoFormat vidfmt = new VideoFormat();
            vidfmt.nFPS_Numerator = 10;
            vidfmt.nFPS_Denominator = 1;
            vidfmt.nWidth = 640;
            vidfmt.nHeight = 480;
            vidfmt.picFourCC = FourCC.FOURCC_RGB32;

            Assert.IsTrue(ttclient.InitVideoCaptureDevice(dev.szDeviceID, vidfmt),
                          "Init video capture device");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_VIDEOCAPTURE_READY), "vid cap ready");

            VideoCodec vidcodec = new VideoCodec();
            vidcodec.nCodec = Codec.WEBM_VP8_CODEC;
            vidcodec.webm_vp8.nRcTargetBitrate = 0;

            Assert.IsTrue(ttclient.StartVideoCaptureTransmission(vidcodec), "Start vid tx");

            int cmdid = ttclient.DoSubscribe(ttclient.GetMyUserID(), Subscription.SUBSCRIBE_VIDEOCAPTURE);
            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, DEF_WAIT), "sub vidcap");

            List<VideoFrame> vidframes = new List<VideoFrame>();

            while (WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_VIDEOCAPTURE, 10000, ref msg))
            {
                if (msg.nSource == ttclient.GetMyUserID())
                {
                    frm = ttclient.AcquireUserVideoCaptureFrame(msg.nSource, out bmp);
                    Assert.IsTrue(frm.nStreamID > 0, "got remote video frame");
                    bmp.Save(MEDIAFOLDER + "\\" + counter++ + ".bmp");

                    vidframes.Add(frm);
                }
                if (vidframes.Count == 10)
                    break;
            }

            foreach (VideoFrame v in vidframes)
                Assert.IsTrue(ttclient.ReleaseUserVideoCaptureFrame(v), "release vid frame");

            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_TX_VIDEOCAPTURE), "Tx'ing vidcap");

            Assert.IsTrue(ttclient.StopVideoCaptureTransmission(), "stop vidcap");

            Assert.IsTrue(ttclient.CloseVideoCaptureDevice(), "close vidcap");
        }

        [TestMethod]
        public void TestMediaStreamAudio()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            InitSound(ttclient);
            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            JoinRoot(ttclient);

            MediaFileInfo mf = new MediaFileInfo();

            Assert.IsTrue(TeamTalk.GetMediaFileInfo(MEDIAFILE_AUDIO, ref mf), "get media file info");
            Assert.IsTrue(mf.uDurationMSec > 0, "media file time");
            Assert.IsTrue(mf.audioFmt.nSampleRate > 0, "sample rate");
            Assert.IsTrue(mf.audioFmt.nChannels > 0, "channels");
            
            VideoCodec vidcodec = new VideoCodec();
            vidcodec.nCodec = Codec.NO_CODEC;
            Assert.IsTrue(ttclient.StartStreamingMediaFileToChannel(MEDIAFILE_AUDIO, vidcodec), "start stream media file");

            TTMessage msg = new TTMessage();
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, ref msg), "wait for state change, player launch");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT, ref msg), "wait for state change, playback");
            User user = (User)msg.DataToObject();
            Assert.IsTrue(user.uUserState.HasFlag(UserState.USERSTATE_MEDIAFILE_AUDIO), "media playback");

            WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);

            Assert.IsTrue(ttclient.SetUserAudioStreamBufferSize(ttclient.UserID, StreamType.STREAMTYPE_MEDIAFILE_AUDIO, 5000));

            Assert.IsTrue(ttclient.GetUser(ttclient.UserID, ref user));
            Assert.AreEqual(5000, user.nBufferMSecMediaFile);

            Assert.IsTrue(ttclient.StopStreamingMediaFileToChannel(), "stop stream");

            UserStatistics stats = new UserStatistics();
            Assert.IsTrue(ttclient.GetUserStatistics(ttclient.GetMyUserID(), ref stats));
            Assert.IsTrue(stats.nMediaFileAudioPacketsRecv > 0);

            Assert.IsFalse(ttclient.StartStreamingMediaFileToChannel("sdfsdf", vidcodec), "start stream invalid media file");
        }

        [TestMethod]
        public void TestMediaStreamVideo()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            InitSound(ttclient);
            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            JoinRoot(ttclient);

            MediaFileInfo mf = new MediaFileInfo();

            Assert.IsTrue(TeamTalk.GetMediaFileInfo(MEDIAFILE_VIDEO, ref mf), "get media file info");
            Assert.IsTrue(mf.uDurationMSec > 0, "media file time");
            Assert.IsTrue(mf.audioFmt.nSampleRate > 0, "sample rate");
            Assert.IsTrue(mf.audioFmt.nChannels > 0, "channels");
            Assert.IsTrue(mf.videoFmt.nWidth > 0, "has video");

            VideoCodec vidcodec = new VideoCodec();
            vidcodec.nCodec = Codec.WEBM_VP8_CODEC;
            vidcodec.webm_vp8.nRcTargetBitrate = 0;

            Assert.IsTrue(ttclient.StartStreamingMediaFileToChannel(MEDIAFILE_VIDEO, vidcodec), "start stream media file");

            TTMessage msg = new TTMessage();
            User user = new User();
            bool video = false, audio = false;
            int videoframes = 0;
            while (ttclient.GetMessage(ref msg, 2000))
            {
                switch (msg.nClientEvent)
                {
                    case ClientEvent.CLIENTEVENT_USER_STATECHANGE:
                        user = (User)msg.DataToObject();
                        audio |= user.uUserState.HasFlag(UserState.USERSTATE_MEDIAFILE_AUDIO);
                        video |= user.uUserState.HasFlag(UserState.USERSTATE_MEDIAFILE_VIDEO);
                        break;
                    case ClientEvent.CLIENTEVENT_USER_MEDIAFILE_VIDEO:
                        {
                            Bitmap bmp;
                            VideoFrame f = ttclient.AcquireUserMediaVideoFrame(msg.nSource, out bmp);
                            videoframes++;
                            Assert.IsTrue(f.nWidth > 0, "acquired video frame");
                            Assert.IsTrue(ttclient.ReleaseUserMediaVideoFrame(f), "release video frame");
                            break;
                        }
                }
            }
            
            Assert.IsTrue(audio, "media audio playback");
            Assert.IsTrue(video, "media video playback");
            Assert.IsTrue(videoframes>0, "got video frames");

            UserStatistics stats = new UserStatistics();
            Assert.IsTrue(ttclient.GetUserStatistics(ttclient.GetMyUserID(), ref stats));
            Assert.IsTrue(stats.nMediaFileAudioPacketsRecv > 0);
            Assert.IsTrue(stats.nMediaFileVideoFramesRecv > 0);
            Assert.IsTrue(stats.nMediaFileVideoPacketsRecv > 0);

            Assert.IsTrue(ttclient.StopStreamingMediaFileToChannel(), "stop stream");
        }

        [TestMethod]
        public void TestMediaStreamFileInvalid()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            InitSound(ttclient);
            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            JoinRoot(ttclient);
            TTMessage msg = new TTMessage();

            WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 0);

            Assert.IsFalse(ttclient.StartStreamingMediaFileToChannel("c:\\temp\foo", new VideoCodec()), "start stream media file");

            Assert.IsFalse(ttclient.GetMessage(ref msg, 0), "no msg expected");

        }

        [TestMethod]
        public void TestDesktopShare()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_DESKTOP | UserRight.USERRIGHT_MULTI_LOGIN;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            InitSound(ttclient);
            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            JoinRoot(ttclient);

            int BMP_HEIGHT = 168, BMP_WIDTH = 120;
            PixelFormat pixelformat = PixelFormat.Format32bppRgb;
            Bitmap bmp = new Bitmap(BMP_WIDTH, BMP_HEIGHT, pixelformat);
            Rectangle rect = new Rectangle(0, 0, bmp.Width, bmp.Height);
            BitmapData bmpData = bmp.LockBits(rect, System.Drawing.Imaging.ImageLockMode.ReadWrite,
                                              bmp.PixelFormat);
            IntPtr ptr = bmpData.Scan0;

            DesktopWindow wnd = new DesktopWindow();
            wnd.nBytesPerLine = bmpData.Stride;
            wnd.nHeight = bmp.Height;
            wnd.nWidth = bmp.Width;
            wnd.nProtocol = DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1;
            wnd.bmpFormat = BitmapFormat.BMP_RGB32;
            wnd.frameBuffer = ptr;
            wnd.nFrameBufferSize = bmpData.Stride * bmpData.Height;

            int tx = ttclient.SendDesktopWindow(wnd, BitmapFormat.BMP_RGB32);
            Assert.IsTrue(tx > 0, "tx bitmap");
            bmp.UnlockBits(bmpData);

            TTMessage msg = new TTMessage();
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, 2000, ref msg), "tx bmp started");
            int remain = (int)msg.DataToObject();
            while (remain > 0)
            {
                Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, 2000, ref msg), "tx bmp started");
                remain = (int)msg.DataToObject();
            }

            Assert.IsFalse(ttclient.Flags.HasFlag(ClientFlag.CLIENT_TX_DESKTOP), "tx desktop done");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_DESKTOP_ACTIVE), "desktop active");

            TeamTalk ttclient2 = NewClientInstance();
            InitSound(ttclient2);
            Connect(ttclient2);
            Login(ttclient2, NICKNAME, USERNAME, PASSWORD);
            JoinRoot(ttclient2);

            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, 5000, ref msg), "receive desktop window");

            Assert.AreEqual(msg.nSource, ttclient.GetMyUserID(), "desktop window from same source");

            DesktopWindow wnd2 = ttclient2.AcquireUserDesktopWindow(msg.nSource);
            Assert.IsTrue(wnd2.nSessionID>0, "get wnd info");
            Assert.AreEqual(wnd.bmpFormat, wnd2.bmpFormat, "bmp fmt");
            Assert.AreEqual(wnd.nBytesPerLine, wnd2.nBytesPerLine, "bmp line");
            Assert.AreEqual(wnd.nHeight, wnd2.nHeight, "height");
            Assert.AreEqual(wnd.nWidth, wnd2.nWidth, "width");
            Bitmap bmp2 = new Bitmap(BMP_WIDTH, BMP_HEIGHT, wnd2.nBytesPerLine, pixelformat, wnd2.frameBuffer);
            for (int x = 0; x < BMP_WIDTH; x++)
            {
                for (int y = 0; y < BMP_HEIGHT; y++)
                    Assert.AreEqual(bmp.GetPixel(x, y), bmp2.GetPixel(x, y), "pixels match");
            }

            Assert.IsTrue(ttclient2.ReleaseUserDesktopWindow(wnd2), "release desk wnd");
            Assert.IsTrue(ttclient.CloseDesktopWindow(), "close desk");

            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, 3000, ref msg), "get close msg");
            Assert.AreEqual(msg.nSource, ttclient.GetMyUserID(), "desktop window from same source");
            Assert.AreEqual(0, (int)msg.DataToObject(), "desktop session is 0");


            tx = ttclient.SendDesktopWindow(wnd, BitmapFormat.BMP_RGB32);
            Assert.IsTrue(tx > 0, "tx bitmap");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, 2000, ref msg), "tx bmp started");
            remain = (int)msg.DataToObject();
            while (remain > 0)
            {
                Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, 2000, ref msg), "tx bmp started");
                remain = (int)msg.DataToObject();
            }

            Assert.IsFalse(ttclient.Flags.HasFlag(ClientFlag.CLIENT_TX_DESKTOP), "tx desktop done");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_DESKTOP_ACTIVE), "desktop active");

            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, 5000, ref msg), "receive desktop window");

            wnd2 = ttclient2.AcquireUserDesktopWindowEx(msg.nSource, BitmapFormat.BMP_RGB16_555);
            Assert.AreNotEqual(wnd.nFrameBufferSize, wnd2.nFrameBufferSize);
            Assert.IsTrue(wnd2.nSessionID > 0, "get wnd info");
            Assert.AreEqual(wnd.nHeight, wnd2.nHeight, "height");
            Assert.AreEqual(wnd.nWidth, wnd2.nWidth, "width");

            Assert.IsTrue(ttclient2.ReleaseUserDesktopWindow(wnd2), "release desk wnd");

        }

        [TestMethod]
        public void TestDesktopHWND()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_DESKTOP;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            InitSound(ttclient);
            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            JoinRoot(ttclient);

            int n_windows = 0;
            IntPtr hWnd = IntPtr.Zero;
            while (WindowsHelper.GetDesktopWindowHWND(n_windows++, ref hWnd))
            {
                ShareWindow shWnd = new ShareWindow();
                Assert.IsTrue(WindowsHelper.GetWindow(hWnd, ref shWnd));
            }
            
            Assert.IsTrue(n_windows > 0, "enum windows");
            
            hWnd = WindowsHelper.GetDesktopHWND();
            Assert.AreNotEqual(IntPtr.Zero, hWnd, "desktop hWnd");

            TTMessage msg = new TTMessage();
            int remain, tx;

            //RGB8 test
            tx = ttclient.SendDesktopWindowFromHWND(hWnd, BitmapFormat.BMP_RGB8_PALETTE, DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1);
            Assert.IsTrue(tx > 0, "send desktop as 8bpp");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, 20000, ref msg), "tx bmp started");
            remain = (int)msg.DataToObject();
            while (remain > 0)
            {
                Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, 10000, ref msg), "tx bmp started");
                remain = (int)msg.DataToObject();
            }

            Assert.IsFalse(ttclient.Flags.HasFlag(ClientFlag.CLIENT_TX_DESKTOP), "tx desktop done");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_DESKTOP_ACTIVE), "desktop active");

            //RGB16 test
            tx = ttclient.SendDesktopWindowFromHWND(hWnd, BitmapFormat.BMP_RGB16_555, DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1);
            Assert.IsTrue(tx > 0, "send desktop as 16bpp");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, 20000, ref msg), "tx bmp started");
            remain = (int)msg.DataToObject();
            while (remain > 0)
            {
                Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, 10000, ref msg), "tx bmp started");
                remain = (int)msg.DataToObject();
            }

            Assert.IsFalse(ttclient.Flags.HasFlag(ClientFlag.CLIENT_TX_DESKTOP), "tx desktop done");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_DESKTOP_ACTIVE), "desktop active");

            //RGB24 test
            tx = ttclient.SendDesktopWindowFromHWND(hWnd, BitmapFormat.BMP_RGB24, DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1);
            Assert.IsTrue(tx > 0, "send desktop as 24bpp");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, 20000, ref msg), "tx bmp started");
            remain = (int)msg.DataToObject();
            while (remain > 0)
            {
                Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, 10000, ref msg), "tx bmp started");
                remain = (int)msg.DataToObject();
            }

            Assert.IsFalse(ttclient.Flags.HasFlag(ClientFlag.CLIENT_TX_DESKTOP), "tx desktop done");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_DESKTOP_ACTIVE), "desktop active");

            //RGB32 test
            tx = ttclient.SendDesktopWindowFromHWND(hWnd, BitmapFormat.BMP_RGB32, DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1);
            Assert.IsTrue(tx > 0, "send desktop as 32bpp");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, 20000, ref msg), "tx bmp started");
            remain = (int)msg.DataToObject();
            while (remain > 0)
            {
                Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, 10000, ref msg), "tx bmp started");
                remain = (int)msg.DataToObject();
            }

            Assert.IsFalse(ttclient.Flags.HasFlag(ClientFlag.CLIENT_TX_DESKTOP), "tx desktop done");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_DESKTOP_ACTIVE), "desktop active");
        }

        [TestMethod]
        public void TestDesktopInput()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_TRANSMIT_DESKTOP | UserRight.USERRIGHT_TRANSMIT_DESKTOPINPUT | UserRight.USERRIGHT_MULTI_LOGIN;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            InitSound(ttclient);
            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            JoinRoot(ttclient);

            TeamTalk ttclient2 = NewClientInstance();
            InitSound(ttclient2);
            Connect(ttclient2);
            Login(ttclient2, NICKNAME, USERNAME, PASSWORD);
            JoinRoot(ttclient2);

            TTMessage msg = new TTMessage();
            IntPtr hWnd = WindowsHelper.GetDesktopActiveHWND();
            int tx = ttclient.SendDesktopWindowFromHWND(hWnd, BitmapFormat.BMP_RGB24, DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1);
            Assert.IsTrue(tx > 0);

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, 20000, ref msg));

            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, 20000, ref msg), "receive desktop window");

            Assert.AreEqual(msg.nSource, ttclient.GetMyUserID(), "desktop window from same source");

            ushort POSX = 45, POSY = 56;
            Assert.IsTrue(ttclient.SendDesktopCursorPosition(POSX, POSY), "send pos");

            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_DESKTOPCURSOR, 2000, ref msg), "receive desktop cursor");

            Assert.AreEqual(ttclient.GetMyUserID(), msg.nSource, "pos from origin");
            DesktopInput dskinput = (DesktopInput)msg.DataToObject();
            Assert.AreEqual(POSX, dskinput.uMousePosX);
            Assert.AreEqual(POSY, dskinput.uMousePosY);

            int cmdid = ttclient.DoSubscribe(ttclient2.GetMyUserID(), Subscription.SUBSCRIBE_DESKTOPINPUT);
            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, 2000), "wait sub");

            DesktopInput[] inputs = new DesktopInput[1];
            inputs[0] = new DesktopInput();
            inputs[0].uMousePosX = POSX;
            inputs[0].uMousePosY = POSY;
            inputs[0].uKeyCode = DesktopInputConstants.DESKTOPINPUT_KEYCODE_IGNORE;
            inputs[0].uKeyState = DesktopKeyState.DESKTOPKEYSTATE_NONE;

            DesktopInput[] trans_inputs;
            Assert.IsTrue(WindowsHelper.DesktopInputKeyTranslate(TTKeyTranslate.TTKEY_WINKEYCODE_TO_TTKEYCODE, inputs, out trans_inputs) > 0, "translate from");
            Assert.IsTrue(ttclient2.SendDesktopInput(ttclient.GetMyUserID(), trans_inputs), "send input");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_DESKTOPINPUT, 2000, ref msg), "get desktop inputs");
            inputs[0] = (DesktopInput)msg.DataToObject();
            Assert.IsTrue(WindowsHelper.DesktopInputKeyTranslate(TTKeyTranslate.TTKEY_TTKEYCODE_TO_WINKEYCODE, inputs, out trans_inputs) > 0, "translate to");
            Assert.AreEqual(POSX, trans_inputs[0].uMousePosX, "same x");
            Assert.AreEqual(POSY, trans_inputs[0].uMousePosY, "same y");
            Assert.IsTrue(WindowsHelper.DesktopInputExecute(trans_inputs) > 0, "exec input");
        }

        [TestMethod]
        public void TestConnectivity()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_NONE;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            Assert.IsTrue(ttclient.Connect(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED), "connect call");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CON_SUCCESS, 1000), "wait connect");

            Assert.IsTrue(ttclient.Disconnect(), "disconnect");

            Assert.IsTrue(ttclient.ConnectEx(IPADDR, TCPPORT, UDPPORT, "0.0.0.0", 0, 0, ENCRYPTED), "connect call");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CON_SUCCESS, 1000), "wait connect");

            Login(ttclient, NICKNAME, USERNAME, PASSWORD);

            Assert.IsTrue(ttclient.QueryMaxPayload(0), "query payload");

            TTMessage msg = new TTMessage();
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED, 1000, ref msg), "payload");

            int payload = (int)msg.DataToObject();
            Assert.IsTrue(payload > 0, "payload up");

            ClientStatistics stats = new ClientStatistics();
            Assert.IsTrue(ttclient.GetClientStatistics(ref stats), "got stats");

            Assert.AreNotEqual(0, stats.nUdpBytesSent, "stats sent");
            Assert.AreNotEqual(0, stats.nUdpBytesRecv, "stats recv");

            //Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoQueryServerStats(), DEF_WAIT));

            User[] users;
            Assert.IsTrue(ttclient.GetServerUsers(out users));

            Channel[] channels;
            Assert.IsTrue(ttclient.GetServerChannels(out channels));
            
            Assert.IsTrue(ttclient.Disconnect(), "disconnect");
        }

        [TestMethod]
        public void TestServerStats()
        {
            TeamTalk ttclient = NewClientInstance();
            Connect(ttclient);
            Login(ttclient, GetCurrentMethod(), ADMIN_USERNAME, ADMIN_PASSWORD);
            TTMessage msg = new TTMessage();
            Assert.IsTrue(ttclient.DoQueryServerStats()>0);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SERVERSTATISTICS, DEF_WAIT, ref msg));

            ServerStatistics stats = (ServerStatistics)msg.DataToObject();
            Assert.IsTrue(stats.nTotalBytesRX > 0);
        }

        [TestMethod]
        public void TestLogin()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_VIEW_ALL_USERS;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            int cmdid;
            cmdid = ttclient.DoLogout();
            Assert.IsTrue(cmdid > 0, "logout issued");

            cmdid = ttclient.DoLogin(NICKNAME, USERNAME, PASSWORD);
            Assert.IsTrue(cmdid > 0, "login issued 2");

            Channel chan = new Channel();
            chan.nParentID = ttclient.GetRootChannelID();
            chan.szName = "my channel";
            chan.szTopic = "a topic";
            chan.nMaxUsers = 128;
            chan.audiocodec.nCodec = Codec.OPUS_CODEC;
            chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
            chan.audiocodec.opus.nChannels = 1;
            chan.audiocodec.opus.nBitRate = 64000;
            chan.audiocodec.opus.nComplexity = 5;
            chan.audiocodec.opus.nTxIntervalMSec = 20;
            chan.audiocodec.opus.nSampleRate = 48000;
            chan.audiocodec.opus.bDTX = true;
            chan.audiocodec.opus.bFEC = true;
            chan.audiocodec.opus.bVBR = true;
            chan.audiocodec.opus.bVBRConstraint = false;

            cmdid = ttclient.DoJoinChannel(chan);
            Assert.IsTrue(cmdid > 0, "join issued");
            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, DEF_WAIT), "join complete");

            cmdid = ttclient.DoJoinChannelByID(ttclient.GetRootChannelID(), "");

            Assert.IsTrue(cmdid > 0, "join root issued");
            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, DEF_WAIT), "join root complete");

            cmdid = ttclient.DoLeaveChannel();
            Assert.IsTrue(cmdid > 0, "leave root issued");
            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, DEF_WAIT), "leave root complete");

            String newnick = "New Nickname";
            cmdid = ttclient.DoChangeNickname(newnick);
            TTMessage msg = new TTMessage();
            while (WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_USER_UPDATE, DEF_WAIT, ref msg))
            {
                if (((User)msg.DataToObject()).nUserID == ttclient.GetMyUserID())
                    break;
            }
            Assert.AreEqual(ttclient.GetMyUserID(), ((User)msg.DataToObject()).nUserID, "me updated nick");
            Assert.AreEqual(newnick, ((User)msg.DataToObject()).szNickname, "me updated nick");

            int mode = 34343;
            string newstatus = "New Status";
            cmdid = ttclient.DoChangeStatus(mode, newstatus);
            while (WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_USER_UPDATE, DEF_WAIT, ref msg))
            {
                if (((User)msg.DataToObject()).nUserID == ttclient.GetMyUserID())
                    break;
            }
            Assert.AreEqual(ttclient.GetMyUserID(), ((User)msg.DataToObject()).nUserID, "me updated status");
            Assert.AreEqual(newstatus, ((User)msg.DataToObject()).szStatusMsg, "me updated status");
            Assert.AreEqual(mode, ((User)msg.DataToObject()).nStatusMode, "me updated status");
        }

        [TestMethod]
        public void TestTextMessage()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_VIEW_ALL_USERS;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            int cmdid;

            TTMessage msg = new TTMessage();

            Channel chan = new Channel();
            chan.nParentID = ttclient.GetRootChannelID();
            chan.szName = "my channel";
            chan.szTopic = "a topic";
            chan.nMaxUsers = 128;
            chan.audiocodec.nCodec = Codec.OPUS_CODEC;
            chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
            chan.audiocodec.opus.nChannels = 1;
            chan.audiocodec.opus.nBitRate = 64000;
            chan.audiocodec.opus.nComplexity = 5;
            chan.audiocodec.opus.nTxIntervalMSec = 20;
            chan.audiocodec.opus.nSampleRate = 48000;
            chan.audiocodec.opus.bDTX = true;
            chan.audiocodec.opus.bFEC = true;
            chan.audiocodec.opus.bVBR = true;
            chan.audiocodec.opus.bVBRConstraint = false;

            TeamTalk ttclient2 = NewClientInstance();
            Connect(ttclient2);
            Login(ttclient2, ADMIN_NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);

            //text message
            string newmessage = "this is a new message";
            TextMessage txtmsg = new TextMessage();
            txtmsg.nMsgType = TextMsgType.MSGTYPE_USER;
            txtmsg.szMessage = newmessage;
            txtmsg.nToUserID = 55555;

            cmdid = ttclient.DoTextMessage(txtmsg);
            Assert.IsTrue(cmdid > 0, "text msg err issued");
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_ERROR, DEF_WAIT), "invalid userid");

            txtmsg.nToUserID = ttclient2.GetMyUserID();
            
            cmdid = ttclient.DoTextMessage(txtmsg);
            Assert.IsTrue(cmdid > 0, "text msg issued");
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT), "send tx msg to userid");

            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_USER_TEXTMSG, DEF_WAIT, ref msg), "recv text msg");
            TextMessage recv_txtmsg = (TextMessage)msg.DataToObject();

            Assert.AreEqual(txtmsg.nMsgType, recv_txtmsg.nMsgType);
            Assert.AreEqual(txtmsg.nChannelID, recv_txtmsg.nChannelID);
            Assert.AreEqual(txtmsg.szMessage, recv_txtmsg.szMessage);

            //custom message
            txtmsg.nMsgType = TextMsgType.MSGTYPE_CUSTOM;

            cmdid = ttclient.DoTextMessage(txtmsg);
            Assert.IsTrue(cmdid > 0, "custom text msg issued");
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT), "send custom tx msg to userid");

            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_USER_TEXTMSG, DEF_WAIT, ref msg), "recv custom text msg");
            recv_txtmsg = (TextMessage)msg.DataToObject();

            Assert.AreEqual(txtmsg.nMsgType, recv_txtmsg.nMsgType);
            Assert.AreEqual(txtmsg.nChannelID, recv_txtmsg.nChannelID);
            Assert.AreEqual(txtmsg.szMessage, recv_txtmsg.szMessage);

            //broadcast
            txtmsg.nMsgType = TextMsgType.MSGTYPE_BROADCAST;
            
            cmdid = ttclient.DoTextMessage(txtmsg);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_ERROR, DEF_WAIT), "no bcast rights");

            cmdid = ttclient2.DoTextMessage(txtmsg);
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT), "has bcast rights");

            cmdid = ttclient.DoJoinChannel(chan);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, ref msg), "has rights");
            Assert.AreEqual(cmdid, msg.nSource);

            //channel msg
            txtmsg.nMsgType = TextMsgType.MSGTYPE_CHANNEL;
            txtmsg.nChannelID = ttclient.GetRootChannelID();
            cmdid = ttclient.DoTextMessage(txtmsg);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_ERROR, DEF_WAIT), "no chan msg rights");

            txtmsg.nChannelID = ttclient.GetMyChannelID();
            cmdid = ttclient2.DoTextMessage(txtmsg);
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT), "has chan msg rights");

            cmdid = ttclient.DoTextMessage(txtmsg);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT), "own chan msg rights");

            cmdid = ttclient.DoLeaveChannel();
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT), "leave chan");

            //invalid text message
            txtmsg = new TextMessage();
            txtmsg.nMsgType = (TextMsgType)55;
            txtmsg.szMessage = "blahhhhhh";
            txtmsg.nToUserID = 55555;

            cmdid = ttclient.DoTextMessage(txtmsg);
            Assert.IsTrue(cmdid > 0, "text msg issued");
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_ERROR, DEF_WAIT, ref msg), "protocol error");
            ClientErrorMsg errmsg = (ClientErrorMsg)msg.DataToObject();
            Assert.AreEqual((int)ClientError.CMDERR_INCOMPATIBLE_PROTOCOLS, errmsg.nErrorNo);
        }

        [TestMethod]
        public void TestChannelOp()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_VIEW_ALL_USERS;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            int cmdid;

            TTMessage msg = new TTMessage();

            Channel chan = new Channel();
            chan.nParentID = ttclient.GetRootChannelID();
            chan.szName = "my channel";
            chan.szTopic = "a topic";
            chan.szOpPassword = "123";
            chan.nMaxUsers = 128;
            chan.audiocodec.nCodec = Codec.OPUS_CODEC;
            chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
            chan.audiocodec.opus.nChannels = 1;
            chan.audiocodec.opus.nBitRate = 64000;
            chan.audiocodec.opus.nComplexity = 5;
            chan.audiocodec.opus.nTxIntervalMSec = 20;
            chan.audiocodec.opus.nSampleRate = 48000;
            chan.audiocodec.opus.bDTX = true;
            chan.audiocodec.opus.bFEC = true;
            chan.audiocodec.opus.bVBR = true;
            chan.audiocodec.opus.bVBRConstraint = false;

            cmdid = ttclient.DoJoinChannel(chan);
            Assert.IsTrue(cmdid > 0, "join issued");
            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, DEF_WAIT), "join complete");

            cmdid = ttclient.DoJoinChannelByID(ttclient.GetRootChannelID(), "");

            Assert.IsTrue(cmdid > 0, "join root issued");
            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, DEF_WAIT), "join root complete");

            TeamTalk ttclient2 = NewClientInstance();
            Connect(ttclient2);
            Login(ttclient2, ADMIN_NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);

            cmdid = ttclient.DoJoinChannel(chan);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT), "join chan");

            Assert.IsTrue(ttclient.IsChannelOperator(ttclient.GetMyUserID(), ttclient.GetMyChannelID()), "op in own chan");

            //TODO: should be possible
            //cmdid = ttclient.DoChannelOp(ttclient.GetMyUserID(), ttclient.GetMyChannelID(), false);
            //Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, ref msg), "de op");

            cmdid = ttclient2.DoChannelOp(ttclient.GetMyUserID(), ttclient.GetMyChannelID(), false);
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, ref msg), "admin de op");
            Assert.IsTrue(msg.nSource == cmdid);

            cmdid = ttclient.DoChannelOp(ttclient.GetMyUserID(), ttclient.GetMyChannelID(), true);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_ERROR, DEF_WAIT, ref msg), "op fail");
            Assert.IsTrue(msg.nSource == cmdid);

            Assert.IsFalse(ttclient.IsChannelOperator(ttclient.GetMyUserID(), ttclient.GetMyChannelID()), "not op in own chan");

            cmdid = ttclient.DoChannelOpEx(ttclient.GetMyUserID(), ttclient.GetMyChannelID(), chan.szOpPassword, true);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, ref msg), "op passwd success");
            Assert.IsTrue(msg.nSource == cmdid);

            Assert.IsTrue(ttclient.IsChannelOperator(ttclient.GetMyUserID(), ttclient.GetMyChannelID()), "op in own chan");

            cmdid = ttclient.DoKickUser(ttclient.GetMyUserID(), 0);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_ERROR, DEF_WAIT), "cannot kick off server");

            cmdid = ttclient.DoKickUser(ttclient.GetMyUserID(), ttclient.GetMyChannelID());
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT), "kick from channel");

            cmdid = ttclient.DoJoinChannel(chan);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT), "join channel");

            cmdid = ttclient2.DoKickUser(ttclient.GetMyUserID(), ttclient.GetMyChannelID());
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT), "kick from channel");

            cmdid = ttclient.DoJoinChannel(chan);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT), "join channel");

            cmdid = ttclient2.DoKickUser(ttclient.GetMyUserID(), 0);
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT), "kick from server");

            cmdid = ttclient.DoJoinChannel(chan);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_ERROR, DEF_WAIT), "no join channel");
        }


        [TestMethod]
        public void TestChannelPassword()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            string username1 = "guest1", password1 = "guest1";
            string username2 = "guest2", password2 = "guest2";

            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            int cmdid;

            TTMessage msg = new TTMessage();

            Channel chan = new Channel();
            chan.nParentID = ttclient.GetRootChannelID();
            chan.szName = "my channel";
            chan.szTopic = "a topic";
            chan.szOpPassword = "123";
            chan.szPassword = "123";
            chan.nMaxUsers = 128;
            chan.audiocodec.nCodec = Codec.OPUS_CODEC;
            chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
            chan.audiocodec.opus.nChannels = 1;
            chan.audiocodec.opus.nBitRate = 64000;
            chan.audiocodec.opus.nComplexity = 5;
            chan.audiocodec.opus.nTxIntervalMSec = 20;
            chan.audiocodec.opus.nSampleRate = 48000;
            chan.audiocodec.opus.bDTX = true;
            chan.audiocodec.opus.bFEC = true;
            chan.audiocodec.opus.bVBR = true;
            chan.audiocodec.opus.bVBRConstraint = false;

            cmdid = ttclient.DoJoinChannel(chan);
            Assert.IsTrue(cmdid > 0, "join issued");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, ref msg));

            Channel chan2 = (Channel)msg.DataToObject();

            Assert.AreEqual(chan.szPassword, chan2.szPassword);

            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, DEF_WAIT), "join complete");

            //see own channel password (as operator)
            MakeUserAccount(GetCurrentMethod(), username1, password1, UserRight.USERRIGHT_VIEW_ALL_USERS);
            TeamTalk ttclient2 = NewClientInstance();

            Connect(ttclient2);
            Login(ttclient2, "test user 1", username1, username1);
            JoinRoot(ttclient2);

            Channel chan3 = new Channel();
            Assert.IsTrue(ttclient2.GetChannel(chan2.nChannelID, ref chan3));
            Assert.AreNotEqual(chan.szPassword, chan3.szPassword, "not see passwd");

            Assert.IsTrue(WaitCmdComplete(ttclient2, ttclient2.DoJoinChannelByID(chan2.nChannelID, chan2.szPassword), DEF_WAIT));

            Assert.IsTrue(ttclient2.GetChannel(chan2.nChannelID, ref chan3));
            Assert.AreNotEqual(chan.szPassword, chan3.szPassword, "not see passwd");

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoUpdateChannel(chan2), DEF_WAIT), "unchanged channel");

            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT));

            Assert.IsTrue(ttclient2.GetChannel(chan2.nChannelID, ref chan3));
            Assert.AreNotEqual(chan.szPassword, chan3.szPassword, "not see passwd");

            //see channel password with USERRIGHT_MODIFY_CHANNELS
            MakeUserAccount(GetCurrentMethod(), username2, password2, UserRight.USERRIGHT_MODIFY_CHANNELS);
            TeamTalk ttclient3 = NewClientInstance();

            Connect(ttclient3);
            Login(ttclient3, "test user 2", username2, username2);
            JoinRoot(ttclient3);

            //see chan passwd as operator
            Channel chan4 = new Channel();
            Assert.IsTrue(ttclient3.GetChannel(chan2.nChannelID, ref chan4));
            Assert.AreEqual(chan.szPassword, chan4.szPassword, "see passwd");

            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoChannelOpEx(ttclient2.GetMyUserID(), chan2.nChannelID, chan.szOpPassword, true), DEF_WAIT), "Op self");

            Assert.IsTrue(ttclient2.GetChannel(chan2.nChannelID, ref chan3));
            Assert.AreEqual(chan.szPassword, chan3.szPassword, "see passwd");
        }

        [TestMethod]
        public void TestFileUpDown()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | UserRight.USERRIGHT_UPLOAD_FILES | UserRight.USERRIGHT_DOWNLOAD_FILES;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            int cmdid;
            
            TTMessage msg = new TTMessage();

            Channel chan = new Channel();
            chan.nParentID = ttclient.GetRootChannelID();
            chan.szName = "my channel";
            chan.szTopic = "a topic";
            chan.szOpPassword = "123";
            chan.nMaxUsers = 128;
            chan.audiocodec.nCodec = Codec.OPUS_CODEC;
            chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
            chan.audiocodec.opus.nChannels = 1;
            chan.audiocodec.opus.nBitRate = 64000;
            chan.audiocodec.opus.nComplexity = 5;
            chan.audiocodec.opus.nTxIntervalMSec = 20;
            chan.audiocodec.opus.nSampleRate = 48000;
            chan.audiocodec.opus.bDTX = true;
            chan.audiocodec.opus.bFEC = true;
            chan.audiocodec.opus.bVBR = true;
            chan.audiocodec.opus.bVBRConstraint = false;

            cmdid = ttclient.DoJoinChannel(chan);
            Assert.IsTrue(cmdid > 0, "join issued");
            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, DEF_WAIT), "join complete");

            cmdid = ttclient.DoSendFile(ttclient.GetMyChannelID(), UPLOADFILE);
            Assert.IsTrue(cmdid > 0, "file upload");
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, ref msg), "reg upload success");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_FILETRANSFER, 2000, ref msg));

            FileTransfer tx = (FileTransfer)msg.DataToObject();
            Assert.IsFalse(tx.bInbound);
            Assert.AreEqual(ttclient.GetMyChannelID(), tx.nChannelID);
            Assert.AreEqual(UPLOADFILE, tx.szLocalFilePath);
            Assert.AreEqual(FileTransferStatus.FILETRANSFER_ACTIVE, tx.nStatus);
            FileTransfer tx2 = new FileTransfer();
            Assert.IsTrue(ttclient.GetFileTransferInfo(tx.nTransferID, ref tx2));
            Assert.AreEqual(tx.bInbound, tx2.bInbound);
            Assert.AreEqual(tx.nChannelID, tx2.nChannelID);
            Assert.AreEqual(tx.nFileSize, tx2.nFileSize);
            Assert.AreEqual(tx.nStatus, tx2.nStatus);
            Assert.AreEqual(tx.nTransferID, tx2.nTransferID);
            //Assert.AreEqual(tx.nTransferred, tx2.nTransferred);
            Assert.AreEqual(tx.szLocalFilePath, tx2.szLocalFilePath);
            Assert.AreEqual(tx.szRemoteFileName, tx2.szRemoteFileName);

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_FILE_NEW, DEF_WAIT, ref msg));
            RemoteFile file = (RemoteFile)msg.DataToObject();

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_FILETRANSFER, 10000, ref msg));
            tx = (FileTransfer)msg.DataToObject();

            Assert.IsFalse(tx.bInbound);
            Assert.AreEqual(ttclient.GetMyChannelID(), tx.nChannelID);
            Assert.AreEqual(UPLOADFILE, tx.szLocalFilePath);
            Assert.AreEqual(FileTransferStatus.FILETRANSFER_FINISHED, tx.nStatus);

            RemoteFile[] files;
            Assert.IsTrue(ttclient.GetChannelFiles(ttclient.GetMyChannelID(), out files));

            RemoteFile file2 = new RemoteFile();
            Assert.IsTrue(ttclient.GetChannelFile(ttclient.GetMyChannelID(), file.nFileID, ref file2));

            Assert.AreEqual(file.nFileID, file2.nFileID);
            Assert.AreEqual(file.nFileSize, file2.nFileSize);
            Assert.AreEqual(file.szFileName, file2.szFileName);
            Assert.AreEqual(file.szUsername, file2.szUsername);

            cmdid = ttclient.DoRecvFile(ttclient.GetMyChannelID(), file.nFileID, DOWNLOADFILE);
            Assert.IsTrue(cmdid > 0, "recv issued");
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, ref msg), "reg down success");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_FILETRANSFER, 2000, ref msg));

            FileTransfer rx = (FileTransfer)msg.DataToObject();
            Assert.IsTrue(rx.bInbound);
            Assert.AreEqual(ttclient.GetMyChannelID(), rx.nChannelID);
            Assert.AreEqual(DOWNLOADFILE, rx.szLocalFilePath);
            Assert.AreEqual(FileTransferStatus.FILETRANSFER_ACTIVE, rx.nStatus);

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_FILETRANSFER, 10000, ref msg));
            rx = (FileTransfer)msg.DataToObject();

            Assert.IsTrue(rx.bInbound);
            Assert.AreEqual(ttclient.GetMyChannelID(), rx.nChannelID);
            Assert.AreEqual(DOWNLOADFILE, rx.szLocalFilePath);
            Assert.AreEqual(FileTransferStatus.FILETRANSFER_FINISHED, rx.nStatus);

            cmdid = ttclient.DoRecvFile(ttclient.GetMyChannelID(), file.nFileID, DOWNLOADFILE);
            Assert.IsTrue(cmdid > 0, "recv issued");
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, ref msg), "reg down success");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_FILETRANSFER, 2000, ref msg));

            rx = (FileTransfer)msg.DataToObject();
            Assert.AreEqual(FileTransferStatus.FILETRANSFER_ACTIVE, rx.nStatus);
            Assert.IsTrue(ttclient.CancelFileTranfer(rx.nTransferID));

            cmdid = ttclient.DoDeleteFile(ttclient.GetMyChannelID(), file.nFileID);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, ref msg), "delete file success");
        }

        [TestMethod]
        public void TestChannelCommands()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            int cmdid;
            TTMessage msg = new TTMessage();

            TeamTalk ttclient2 = NewClientInstance();
            Connect(ttclient2);
            Login(ttclient2, ADMIN_NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);

            Channel chan = new Channel();
            chan.nParentID = ttclient2.GetRootChannelID();
            chan.uChannelType = ChannelType.CHANNEL_PERMANENT | ChannelType.CHANNEL_SOLO_TRANSMIT;
            chan.nUserData = 555666;
            chan.szName = "my channel";
            chan.szTopic = "a topic";
            chan.szPassword = "a";
            chan.szOpPassword = "123";
            chan.nMaxUsers = 128;
            chan.nDiskQuota = 5548726;
            chan.audiocodec.nCodec = Codec.OPUS_CODEC;
            chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
            chan.audiocodec.opus.nChannels = 1;
            chan.audiocodec.opus.nBitRate = 64000;
            chan.audiocodec.opus.nComplexity = 5;
            chan.audiocodec.opus.nTxIntervalMSec = 20;
            chan.audiocodec.opus.nSampleRate = 48000;
            chan.audiocodec.opus.bDTX = true;
            chan.audiocodec.opus.bFEC = true;
            chan.audiocodec.opus.bVBR = true;
            chan.audiocodec.opus.bVBRConstraint = false;
            chan.audiocfg = new AudioConfig(true);

            //ensure channel doesn't already exists
            WaitCmdComplete(ttclient2, ttclient2.DoRemoveChannel(ttclient2.GetChannelIDFromPath(chan.szName)), DEF_WAIT);

            cmdid = ttclient2.DoMakeChannel(chan);
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW, DEF_WAIT, ref msg), "add chan");
            Channel chan2 = (Channel)msg.DataToObject();
            Channel permChan = chan2;

            Assert.AreEqual(chan.nParentID, chan2.nParentID);
            Assert.AreEqual(chan.uChannelType, chan2.uChannelType);
            Assert.AreEqual(chan.nUserData, chan2.nUserData);
            Assert.AreEqual(chan.szName, chan2.szName);
            Assert.AreEqual(chan.szTopic, chan2.szTopic);
            Assert.AreEqual(chan.szPassword, chan2.szPassword);
            Assert.AreEqual(chan.szOpPassword, chan2.szOpPassword);
            Assert.AreEqual(chan.nMaxUsers, chan2.nMaxUsers);
            Assert.AreEqual(chan.nDiskQuota, chan2.nDiskQuota);
            Assert.AreEqual(chan.audiocodec.nCodec, chan2.audiocodec.nCodec);
            Assert.AreEqual(chan.audiocodec.opus.nApplication, chan2.audiocodec.opus.nApplication);
            Assert.AreEqual(chan.audiocodec.opus.nChannels, chan2.audiocodec.opus.nChannels);
            Assert.AreEqual(chan.audiocodec.opus.nBitRate, chan2.audiocodec.opus.nBitRate);
            Assert.AreEqual(chan.audiocodec.opus.nComplexity, chan2.audiocodec.opus.nComplexity);
            Assert.AreEqual(chan.audiocodec.opus.nTxIntervalMSec, chan2.audiocodec.opus.nTxIntervalMSec);
            Assert.AreEqual(chan.audiocodec.opus.nSampleRate, chan2.audiocodec.opus.nSampleRate);
            Assert.AreEqual(chan.audiocodec.opus.bDTX, chan2.audiocodec.opus.bDTX);
            Assert.AreEqual(chan.audiocodec.opus.bFEC, chan2.audiocodec.opus.bFEC);
            Assert.AreEqual(chan.audiocodec.opus.bVBR, chan2.audiocodec.opus.bVBR);
            Assert.AreEqual(chan.audiocodec.opus.bVBRConstraint, chan2.audiocodec.opus.bVBRConstraint);
            Assert.AreEqual(chan.audiocfg.bEnableAGC, chan2.audiocfg.bEnableAGC);
            Assert.AreEqual(chan.audiocfg.nGainLevel, chan2.audiocfg.nGainLevel);

            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            Assert.IsTrue(ttclient.GetMyUserRights().HasFlag(UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL));
            Assert.IsFalse(ttclient.GetMyUserRights().HasFlag(UserRight.USERRIGHT_MODIFY_CHANNELS));

            cmdid = ttclient.DoJoinChannelByID(chan2.nChannelID, chan.szPassword);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, ref msg), "join complete");

            chan.uChannelType = ChannelType.CHANNEL_CLASSROOM;
            chan.nMaxUsers = 1024;
            chan.nParentID = chan2.nChannelID;
            cmdid = ttclient.DoJoinChannel(chan);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW, DEF_WAIT, ref msg), "join complete");
            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, DEF_WAIT));
            chan2 = (Channel)msg.DataToObject();

            Assert.AreEqual(chan.nParentID, chan2.nParentID);
            Assert.AreEqual(chan.uChannelType, chan2.uChannelType);
            Assert.AreEqual(chan.szName, chan2.szName);
            Assert.AreEqual(chan.szTopic, chan2.szTopic);
            //Assert.AreEqual(chan.szPassword, chan2.szPassword);
            //Assert.AreEqual(chan.szOpPassword, chan2.szOpPassword);
            Assert.AreNotEqual(chan.nMaxUsers, chan2.nMaxUsers);
            Assert.AreNotEqual(chan.nDiskQuota, chan2.nDiskQuota);
            Assert.AreEqual(chan.audiocodec.nCodec, chan2.audiocodec.nCodec);
            Assert.AreEqual(chan.audiocodec.opus.nApplication, chan2.audiocodec.opus.nApplication);
            Assert.AreEqual(chan.audiocodec.opus.nChannels, chan2.audiocodec.opus.nChannels);
            Assert.AreEqual(chan.audiocodec.opus.nBitRate, chan2.audiocodec.opus.nBitRate);
            Assert.AreEqual(chan.audiocodec.opus.nComplexity, chan2.audiocodec.opus.nComplexity);
            Assert.AreEqual(chan.audiocodec.opus.nTxIntervalMSec, chan2.audiocodec.opus.nTxIntervalMSec);
            Assert.AreEqual(chan.audiocodec.opus.nSampleRate, chan2.audiocodec.opus.nSampleRate);
            Assert.AreEqual(chan.audiocodec.opus.bDTX, chan2.audiocodec.opus.bDTX);
            Assert.AreEqual(chan.audiocodec.opus.bFEC, chan2.audiocodec.opus.bFEC);
            Assert.AreEqual(chan.audiocodec.opus.bVBR, chan2.audiocodec.opus.bVBR);
            Assert.AreEqual(chan.audiocodec.opus.bVBRConstraint, chan2.audiocodec.opus.bVBRConstraint);
            Assert.AreEqual(chan.audiocfg.bEnableAGC, chan2.audiocfg.bEnableAGC);
            Assert.AreEqual(chan.audiocfg.nGainLevel, chan2.audiocfg.nGainLevel);
            //Assert.AreEqual(chan.audiocfg.bEnableEchoCancellation, chan2.audiocfg.bEnableEchoCancellation);
            //Assert.AreEqual(chan.audiocfg.nEchoSuppress, chan2.audiocfg.nEchoSuppress);
            //Assert.AreEqual(chan.audiocfg.nEchoSuppressActive, chan2.audiocfg.nEchoSuppressActive);

            chan.nChannelID = chan2.nChannelID;
            chan.szName = "new channel name";
            cmdid = ttclient.DoUpdateChannel(chan);

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, ref msg), "update complete");
            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, DEF_WAIT));

            chan2 = (Channel)msg.DataToObject();

            Assert.AreEqual(chan.nParentID, chan2.nParentID);
            Assert.AreEqual(chan.nChannelID, chan2.nChannelID);
            Assert.AreEqual(chan.uChannelType, chan2.uChannelType);
            Assert.AreEqual(chan.szName, chan2.szName);
            Assert.AreEqual(chan.szTopic, chan2.szTopic);
            //Assert.AreEqual(chan.szPassword, chan2.szPassword);
            //Assert.AreEqual(chan.szOpPassword, chan2.szOpPassword);
            Assert.AreNotEqual(chan.nMaxUsers, chan2.nMaxUsers);
            Assert.AreNotEqual(chan.nDiskQuota, chan2.nDiskQuota);
            Assert.AreEqual(chan.audiocodec.nCodec, chan2.audiocodec.nCodec);
            Assert.AreEqual(chan.audiocodec.opus.nApplication, chan2.audiocodec.opus.nApplication);
            Assert.AreEqual(chan.audiocodec.opus.nChannels, chan2.audiocodec.opus.nChannels);
            Assert.AreEqual(chan.audiocodec.opus.nBitRate, chan2.audiocodec.opus.nBitRate);
            Assert.AreEqual(chan.audiocodec.opus.nComplexity, chan2.audiocodec.opus.nComplexity);
            Assert.AreEqual(chan.audiocodec.opus.nTxIntervalMSec, chan2.audiocodec.opus.nTxIntervalMSec);
            Assert.AreEqual(chan.audiocodec.opus.nSampleRate, chan2.audiocodec.opus.nSampleRate);
            Assert.AreEqual(chan.audiocodec.opus.bDTX, chan2.audiocodec.opus.bDTX);
            Assert.AreEqual(chan.audiocodec.opus.bFEC, chan2.audiocodec.opus.bFEC);
            Assert.AreEqual(chan.audiocodec.opus.bVBR, chan2.audiocodec.opus.bVBR);
            Assert.AreEqual(chan.audiocodec.opus.bVBRConstraint, chan2.audiocodec.opus.bVBRConstraint);
            Assert.AreEqual(chan.audiocfg.bEnableAGC, chan2.audiocfg.bEnableAGC);
            Assert.AreEqual(chan.audiocfg.nGainLevel, chan2.audiocfg.nGainLevel);

            string chpath = "";
            Assert.IsTrue(ttclient.GetChannelPath(chan2.nChannelID, ref chpath));
            Assert.AreEqual(chan2.nChannelID, ttclient.GetChannelIDFromPath(chpath));

            User[] users;
            Assert.IsTrue(ttclient.GetChannelUsers(chan2.nChannelID, out users));

            //ensure UserRight.USERRIGHT_MODIFY_CHANNELS can update
            chan.szName = "foobar";
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoUpdateChannel(chan), DEF_WAIT));

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, ref msg), "update complete");
            Assert.IsTrue(ttclient.GetChannel(chan.nChannelID, ref chan2));
            Assert.AreEqual(chan2.szName, chan.szName);

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoJoinChannelByID(permChan.nChannelID, permChan.szPassword), DEF_WAIT));
            chan = permChan;

            //non-op cannot update
            Assert.IsTrue(WaitCmdError(ttclient, ttclient.DoUpdateChannel(chan), DEF_WAIT));

            //make op
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoChannelOp(ttclient.GetMyUserID(), chan.nChannelID, true), DEF_WAIT));

            //update as op
            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoUpdateChannel(chan), DEF_WAIT));

            //cannot update perm
            chan.uChannelType = ChannelType.CHANNEL_DEFAULT;
            Assert.IsTrue(WaitCmdError(ttclient, ttclient.DoUpdateChannel(chan), DEF_WAIT));

            //cannot update name
            chan = permChan;
            chan.szName = "fod2";
            Assert.IsTrue(WaitCmdError(ttclient, ttclient.DoUpdateChannel(chan), DEF_WAIT));

            //update as op
            chan = permChan;
            chan.szTopic = "a new topic";
            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoUpdateChannel(chan), DEF_WAIT));

            //no remove as op
            Assert.IsTrue(WaitCmdError(ttclient, ttclient.DoRemoveChannel(chan.nChannelID), DEF_WAIT));

            //remove with rights
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoRemoveChannel(chan.nChannelID), DEF_WAIT));
        }

        [TestMethod]
        public void TestMoveUserCommands()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            int cmdid;
            TTMessage msg = new TTMessage();

            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);

            TeamTalk ttclient2 = NewClientInstance();
            Connect(ttclient2);
            Login(ttclient2, ADMIN_NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);

            Channel chan = new Channel();
            chan.nParentID = ttclient2.GetRootChannelID();
            chan.uChannelType = ChannelType.CHANNEL_DEFAULT;
            chan.szName = "my channel";
            chan.szTopic = "a topic";
            chan.szPassword = "a";
            chan.szOpPassword = "123";
            chan.nMaxUsers = 128;
            chan.nDiskQuota = 5548726;
            chan.audiocodec.nCodec = Codec.OPUS_CODEC;
            chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
            chan.audiocodec.opus.nChannels = 1;
            chan.audiocodec.opus.nBitRate = 64000;
            chan.audiocodec.opus.nComplexity = 5;
            chan.audiocodec.opus.nTxIntervalMSec = 20;
            chan.audiocodec.opus.nSampleRate = 48000;
            chan.audiocodec.opus.bDTX = true;
            chan.audiocodec.opus.bFEC = true;
            chan.audiocodec.opus.bVBR = true;
            chan.audiocodec.opus.bVBRConstraint = false;
            chan.audiocfg = new AudioConfig(true);

            cmdid = ttclient.DoJoinChannel(chan);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, ref msg), "join complete");

            cmdid = ttclient.DoMoveUser(ttclient.GetMyUserID(), ttclient.GetRootChannelID());
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_ERROR, DEF_WAIT, ref msg), "move failure");

            cmdid = ttclient2.DoMoveUser(ttclient.GetMyUserID(), ttclient.GetRootChannelID());
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT, ref msg), "move failure");

            Assert.AreEqual(ttclient.GetRootChannelID(), ttclient.GetMyChannelID());
        }

        [TestMethod]
        public void TestServerUpdateCommands()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_NONE;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            int cmdid;
            TTMessage msg = new TTMessage();

            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);

            ServerProperties prop = new ServerProperties(), prop2 = new ServerProperties(), org_prop = new ServerProperties();
            Assert.IsTrue(ttclient.GetServerProperties(ref prop));
            cmdid = ttclient.DoUpdateServer(prop);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_ERROR, DEF_WAIT, ref msg), "server update fail");

            TeamTalk ttclient2 = NewClientInstance();
            Connect(ttclient2);
            cmdid = ttclient2.DoLogin(ADMIN_NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);
            Assert.IsTrue(cmdid > 0, "login issued");
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_SERVER_UPDATE, DEF_WAIT, ref msg), "login complete");
            org_prop = prop2 = (ServerProperties)msg.DataToObject();

            Assert.AreEqual(prop.nMaxDesktopTxPerSecond, prop2.nMaxDesktopTxPerSecond);
            Assert.AreEqual(prop.nMaxLoginsPerIPAddress, prop2.nMaxLoginsPerIPAddress);
            Assert.AreEqual(prop.nMaxMediaFileTxPerSecond, prop2.nMaxMediaFileTxPerSecond);
            Assert.AreEqual(prop.nMaxTotalTxPerSecond, prop2.nMaxTotalTxPerSecond);
            Assert.AreEqual(prop.nMaxUsers, prop2.nMaxUsers);
            Assert.AreEqual(prop.nMaxVideoCaptureTxPerSecond, prop2.nMaxVideoCaptureTxPerSecond);
            Assert.AreEqual(prop.nMaxVoiceTxPerSecond, prop2.nMaxVoiceTxPerSecond);
            Assert.AreEqual(prop.nUserTimeout, prop2.nUserTimeout);
            Assert.AreEqual(prop.szMOTD, prop2.szMOTD);
            Assert.AreEqual(prop.szServerName, prop2.szServerName);
            Assert.AreEqual(prop.szServerProtocolVersion, prop2.szServerProtocolVersion);
            Assert.AreEqual(prop.szServerVersion, prop2.szServerVersion);

            //Assert.AreEqual(prop.bAutoSave, prop2.bAutoSave);
            //Assert.AreEqual(prop.szMOTDRaw, prop2.szMOTDRaw);
            //Assert.AreEqual(prop.nMaxLoginAttempts, prop2.nMaxLoginAttempts);
            //Assert.AreEqual(prop.nTcpPort, prop2.nTcpPort);
            //Assert.AreEqual(prop.nUdpPort, prop2.nUdpPort);

            prop = prop2;

            prop.bAutoSave = false;
            prop.nMaxDesktopTxPerSecond = 45000;
            prop.nMaxLoginAttempts = 2;
            prop.nMaxLoginsPerIPAddress = 300;
            prop.nMaxMediaFileTxPerSecond = 56000;
            prop.nMaxTotalTxPerSecond = 4566;
            prop.nMaxUsers = 45;
            prop.nMaxVideoCaptureTxPerSecond = 56700;
            prop.nMaxVoiceTxPerSecond = 44567;
            prop.nTcpPort = 450;
            prop.nUdpPort = 564;
            prop.nUserTimeout = 2323;
            prop.szMOTDRaw = "this is the motd";
            prop.szServerName = "this is the server name";

            prop.szMOTD = "foo";
            prop.szServerProtocolVersion = "445";
            prop.szServerVersion = "444";

            cmdid = ttclient2.DoUpdateServer(prop);
            Assert.IsTrue(cmdid > 0, "issued srv upd");
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_SERVER_UPDATE, DEF_WAIT, ref msg), "srv upd");
            prop2 = (ServerProperties)msg.DataToObject();

            Assert.AreEqual(prop.nMaxDesktopTxPerSecond, prop2.nMaxDesktopTxPerSecond);
            Assert.AreEqual(prop.nMaxLoginsPerIPAddress, prop2.nMaxLoginsPerIPAddress);
            Assert.AreEqual(prop.nMaxMediaFileTxPerSecond, prop2.nMaxMediaFileTxPerSecond);
            Assert.AreEqual(prop.nMaxTotalTxPerSecond, prop2.nMaxTotalTxPerSecond);
            Assert.AreEqual(prop.nMaxUsers, prop2.nMaxUsers);
            Assert.AreEqual(prop.nMaxVideoCaptureTxPerSecond, prop2.nMaxVideoCaptureTxPerSecond);
            Assert.AreEqual(prop.nMaxVoiceTxPerSecond, prop2.nMaxVoiceTxPerSecond);
            Assert.AreEqual(prop.nUserTimeout, prop2.nUserTimeout);
            Assert.AreEqual(prop.szServerName, prop2.szServerName);

            Assert.AreNotEqual(prop.szMOTD, prop2.szMOTD);
            Assert.AreNotEqual(prop.szServerProtocolVersion, prop2.szServerProtocolVersion);
            Assert.AreNotEqual(prop.szServerVersion, prop2.szServerVersion);

            Assert.AreEqual(prop.bAutoSave, prop2.bAutoSave);
            Assert.AreEqual(prop.szMOTDRaw, prop2.szMOTDRaw);
            Assert.AreEqual(prop.nMaxLoginAttempts, prop2.nMaxLoginAttempts);
            Assert.AreEqual(prop.nTcpPort, prop2.nTcpPort);
            Assert.AreEqual(prop.nUdpPort, prop2.nUdpPort);

            cmdid = ttclient2.DoUpdateServer(org_prop);
            Assert.IsTrue(cmdid > 0, "issued srv upd");
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_SERVER_UPDATE, DEF_WAIT, ref msg), "srv upd");

            //prop.szServerProtocolVersion = "";
            //prop.szMOTD = "this is the motd";
            //prop.szServerVersion = "";
        }

        [TestMethod]
        public void TestIpLogins()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_UPDATE_SERVERPROPERTIES | UserRight.USERRIGHT_MULTI_LOGIN;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);

            ServerProperties srvprop = new ServerProperties();
            Assert.IsTrue(ttclient.GetServerProperties(ref srvprop));

            ServerProperties srvprop2 = srvprop;
            srvprop2.nMaxLoginsPerIPAddress = 2;

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoUpdateServer(srvprop2), DEF_WAIT), "Update server");

            ttclient.Disconnect();

            TeamTalk ttclient2 = new TeamTalk(true);
            Connect(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);
            Connect(ttclient2);
            Login(ttclient2, NICKNAME, USERNAME, PASSWORD);

            TeamTalk ttclient3 = new TeamTalk(true);
            Connect(ttclient3);
            Assert.IsTrue(!WaitCmdSuccess(ttclient3, ttclient3.DoLogin(NICKNAME, USERNAME, PASSWORD), 500), "Login failure");
            
            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoUpdateServer(srvprop), DEF_WAIT), "Update server");
        }

        [TestMethod]
        public void TestUserAccountRights()
        {
            TeamTalk ttclient = NewClientInstance();
            Connect(ttclient);
            Login(ttclient, ADMIN_NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);

            TTMessage msg = new TTMessage();
            string BANIPADDR = "192.25.2.1";
            UserAccount account = new UserAccount();
            account.szInitChannel = "";
            account.autoOperatorChannels = new int[TeamTalk.TT_CHANNELS_OPERATOR_MAX];
            account.szUsername = "svend";
            account.szPassword = "passwd";
            account.szNote = "the note";
            account.nUserData = 456;
            account.uUserRights = UserRight.USERRIGHT_NONE;
            account.uUserType = UserType.USERTYPE_DEFAULT;
            account.nAudioCodecBpsLimit = 22000;

            int cmdid = ttclient.DoNewUserAccount(account);
            WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT);

            TeamTalk ttclient2 = NewClientInstance();
            Connect(ttclient2);

            Login(ttclient2, "blah", account.szUsername, account.szPassword);

            UserAccount account2 = new UserAccount();
            Assert.IsTrue(ttclient2.GetMyUserAccount(ref account2), "get account");

            Assert.AreEqual(account.szUsername, account2.szUsername);
            //Assert.AreEqual(account.szPassword, account2.szPassword);
            Assert.AreEqual(account.nUserData, account2.nUserData);
            Assert.AreEqual(account.szInitChannel, account2.szInitChannel);
            Assert.AreEqual(account.szNote, account2.szNote);
            Assert.AreEqual(account.uUserRights, account2.uUserRights);
            Assert.AreEqual(account.uUserType, account2.uUserType);
            Assert.AreEqual(account.nAudioCodecBpsLimit, account2.nAudioCodecBpsLimit);

            CollectionAssert.AreEqual(account.autoOperatorChannels, account2.autoOperatorChannels);

            Channel chan = new Channel();
            chan.nParentID = ttclient2.GetRootChannelID();
            chan.szName = "foobar";

            //USERRIGHT_NONE
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoJoinChannelByID(ttclient2.GetRootChannelID(), ""), DEF_WAIT));
            Assert.IsTrue(WaitCmdError(ttclient2, ttclient2.DoSendFile(ttclient2.GetMyChannelID(), UPLOADFILE), DEF_WAIT));
            Assert.IsTrue(WaitCmdError(ttclient2, ttclient2.DoBanUser(ttclient2.GetMyUserID(), 0), DEF_WAIT));
            Assert.IsTrue(WaitCmdError(ttclient2, ttclient2.DoBanIPAddress(BANIPADDR, 0), DEF_WAIT));
            Assert.IsTrue(WaitCmdError(ttclient2, ttclient2.DoUnBanUser(BANIPADDR, 0), DEF_WAIT));
            Assert.IsTrue(WaitCmdError(ttclient2, ttclient2.DoListBans(0, 0, 200), DEF_WAIT));
            Assert.IsTrue(WaitCmdError(ttclient2, ttclient2.DoNewUserAccount(account), DEF_WAIT));

            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoLeaveChannel(), DEF_WAIT));
            Assert.IsTrue(WaitCmdError(ttclient2, ttclient2.DoMakeChannel(chan), DEF_WAIT));
            Assert.IsTrue(WaitCmdError(ttclient2, ttclient2.DoJoinChannel(chan), DEF_WAIT));
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoLogout(), DEF_WAIT));

            //USERRIGHT_CREATE_TEMPORARY_CHANNEL
            account.uUserRights |= UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoNewUserAccount(account), DEF_WAIT));

            Login(ttclient2, "blah", account.szUsername, account.szPassword);
            Assert.IsTrue(ttclient2.GetMyUserAccount(ref account2), "get account");
            Assert.AreEqual(account.uUserRights, account2.uUserRights);
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoJoinChannel(chan), DEF_WAIT));

            chan.szName = "foobar2";
            chan.nChannelID = 0;
            chan.audiocodec.nCodec = Codec.SPEEX_CODEC;
            chan.audiocodec.speex.nBandmode = 1;
            chan.audiocodec.speex.nTxIntervalMSec = 40;
            chan.audiocodec.speex.nQuality = 10;
            chan.audiocodec.speex.bStereoPlayback = false;
            Assert.IsTrue(WaitCmdError(ttclient2, ttclient2.DoJoinChannel(chan), DEF_WAIT));
            chan.audiocodec.speex.nQuality = 2;
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoJoinChannel(chan), DEF_WAIT));

            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoLogout(), DEF_WAIT));

            //USERRIGHT_BAN_USERS
            account.uUserRights |= UserRight.USERRIGHT_BAN_USERS;
            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoNewUserAccount(account), DEF_WAIT));

            Login(ttclient2, "blah", account.szUsername, account.szPassword);
            Assert.IsTrue(ttclient2.GetMyUserAccount(ref account2), "get account");
            Assert.AreEqual(account.uUserRights, account2.uUserRights);
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoBanUser(ttclient2.GetMyUserID(), 0), DEF_WAIT));
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoBanIPAddress(BANIPADDR, 0), DEF_WAIT));
            ttclient2.OnCmdBannedUser += new TeamTalk.ListBannedUser(ttclient2_OnCmdBannedUser);
            bannedusers.Clear();
            Assert.IsTrue(WaitCmdComplete(ttclient2, ttclient2.DoListBans(0, 0, 200), DEF_WAIT));

            BannedUser b = bannedusers.First<BannedUser>(m => m.szUsername == account2.szUsername);
            Assert.IsTrue(bannedusers.Count(m => m.szIPAddress == BANIPADDR) == 1);

            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoUnBanUser(b.szIPAddress, 0), DEF_WAIT));
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoUnBanUser(BANIPADDR, 0), DEF_WAIT));
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoLogout(), DEF_WAIT));

            //USERRIGHT_UPLOAD_FILES
            account.uUserRights |= UserRight.USERRIGHT_UPLOAD_FILES;
            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoNewUserAccount(account), DEF_WAIT));

            Login(ttclient2, "blah", account.szUsername, account.szPassword);
            Assert.IsTrue(ttclient2.GetMyUserAccount(ref account2), "get account");
            Assert.AreEqual(account.uUserRights, account2.uUserRights);
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoJoinChannel(chan), DEF_WAIT));
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoSendFile(ttclient2.GetMyChannelID(), UPLOADFILE), 20000));
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_FILETRANSFER, 20000, ref msg));
            Assert.AreEqual(FileTransferStatus.FILETRANSFER_ACTIVE, ((FileTransfer)msg.DataToObject()).nStatus);
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_FILETRANSFER, 20000, ref msg));
            Assert.AreEqual(FileTransferStatus.FILETRANSFER_FINISHED, ((FileTransfer)msg.DataToObject()).nStatus);
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoLogout(), DEF_WAIT));

            //USERRIGHT_DOWNLOAD_FILES
            account.uUserRights |= UserRight.USERRIGHT_DOWNLOAD_FILES;
            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoNewUserAccount(account), DEF_WAIT));

            Login(ttclient2, "blah", account.szUsername, account.szPassword);
            Assert.IsTrue(ttclient2.GetMyUserAccount(ref account2), "get account");
            Assert.AreEqual(account.uUserRights, account2.uUserRights);
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoJoinChannel(chan), DEF_WAIT));
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoSendFile(ttclient2.GetMyChannelID(), UPLOADFILE), 20000));
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_FILETRANSFER, 20000, ref msg));
            Assert.AreEqual(FileTransferStatus.FILETRANSFER_ACTIVE, ((FileTransfer)msg.DataToObject()).nStatus);
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_FILETRANSFER, 20000, ref msg));
            Assert.AreEqual(FileTransferStatus.FILETRANSFER_FINISHED, ((FileTransfer)msg.DataToObject()).nStatus);
            RemoteFile[] files;
            Assert.IsTrue(ttclient2.GetChannelFiles(ttclient2.GetMyChannelID(), out files));
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoRecvFile(files[0].nChannelID, files[0].nFileID, DOWNLOADFILE), 20000));
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_FILETRANSFER, 20000, ref msg));
            Assert.AreEqual(FileTransferStatus.FILETRANSFER_ACTIVE, ((FileTransfer)msg.DataToObject()).nStatus);
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_FILETRANSFER, 20000, ref msg));
            Assert.AreEqual(FileTransferStatus.FILETRANSFER_FINISHED, ((FileTransfer)msg.DataToObject()).nStatus);

            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoLogout(), DEF_WAIT));

            //TODO: remaining rights
            //UserRight.USERRIGHT_KICK_USERS
            //UserRight.USERRIGHT_MOVE_USERS
            //UserRight.USERRIGHT_
            //UserRight.USERRIGHT_MODIFY_CHANNELS

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoSaveConfig(), DEF_WAIT));

            cmdid = ttclient.DoDeleteUserAccount(account.szUsername);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, DEF_WAIT));

            cmdid = ttclient.DoDeleteUserAccount("godj3g");
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_ERROR, DEF_WAIT));
        }

        List<BannedUser> bannedusers = new List<BannedUser>();

        void ttclient2_OnCmdBannedUser(BannedUser banneduser)
        {
            bannedusers.Add(banneduser);
        }

        List<UserAccount> useraccounts = new List<UserAccount>();

        void ttclient2_OnCmdUserAccount(UserAccount useraccount)
        {
            useraccounts.Add(useraccount);
        }

        [TestMethod]
        public void TestAdminUserAccount()
        {
            TeamTalk ttclient = NewClientInstance();
            Connect(ttclient);
            Login(ttclient, ADMIN_NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);

            UserAccount account = new UserAccount();
            account.szInitChannel = "";
            account.autoOperatorChannels = new int[TeamTalk.TT_CHANNELS_OPERATOR_MAX];
            account.szUsername = "svend";
            account.szPassword = "passwd";
            account.szNote = "the note";
            account.nUserData = 456;
            account.uUserRights = UserRight.USERRIGHT_KICK_USERS;
            account.uUserType = UserType.USERTYPE_ADMIN;

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoNewUserAccount(account), DEF_WAIT));

            ttclient.OnCmdUserAccount += new TeamTalk.ListUserAccount(ttclient_OnCmdUserAccount);
            useraccounts.Clear();
            Assert.IsTrue(WaitCmdComplete(ttclient, ttclient.DoListUserAccounts(0, 1000000), DEF_WAIT));

            Assert.IsTrue(useraccounts.Count>0);

            TeamTalk ttclient2 = NewClientInstance();
            Connect(ttclient2);
            Login(ttclient2, GetCurrentMethod(), account.szUsername, account.szPassword);

            UserAccount a = new UserAccount();
            Assert.IsTrue(ttclient2.GetMyUserAccount(ref a));

            foreach (UserRight u in (UserRight[])Enum.GetValues(typeof(UserRight)))
                Assert.IsTrue(a.uUserRights.HasFlag(u));
        }

        [TestMethod]
        public void TestManyUserAccounts()
        {
            TeamTalk ttclient = NewClientInstance();
            Connect(ttclient);
            Login(ttclient, ADMIN_NICKNAME, ADMIN_USERNAME, ADMIN_PASSWORD);

            UserAccount account = new UserAccount();
            account.szInitChannel = "";
            account.autoOperatorChannels = new int[TeamTalk.TT_CHANNELS_OPERATOR_MAX];
            account.szUsername = "svend";
            account.szPassword = "passwd";
            account.szNote = "the note";
            account.nUserData = 456;
            account.uUserRights = UserRight.USERRIGHT_NONE;
            account.uUserType = UserType.USERTYPE_DEFAULT;

            const int N_ACCOUNT = 20;
            string username = account.szUsername;
            for (int i = 0; i < N_ACCOUNT; i++)
            {
                account.szUsername = username + i;
                Assert.IsTrue(ttclient.DoNewUserAccount(account) > 0);
            }
            ttclient.OnCmdUserAccount += new TeamTalk.ListUserAccount(ttclient_OnCmdUserAccount);
            useraccounts.Clear();
            Assert.IsTrue(WaitCmdComplete(ttclient, ttclient.DoListUserAccounts(0, N_ACCOUNT), 30000));
            Assert.IsTrue(useraccounts.Count >= N_ACCOUNT);
            for (int i = 0; i < N_ACCOUNT; i++)
            {
                Assert.IsTrue(ttclient.DoDeleteUserAccount(username + i) > 0);
            }
            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoLogout(), 200000));
        }

        void ttclient_OnCmdUserAccount(UserAccount useraccount)
        {
            useraccounts.Add(useraccount);
        }

        [TestMethod]
        public void TestViewAllUsers()
        {
            const string USERNAME1 = "guest1", PASSWORD1 = "guest1", NICKNAME1 = "TeamTalk.NET1";
            const string USERNAME2 = "guest2", PASSWORD2 = "guest2", NICKNAME2 = "TeamTalk.NET2";
            const UserRight USERRIGHTS1 = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
            const UserRight USERRIGHTS2 = UserRight.USERRIGHT_MULTI_LOGIN;
            MakeUserAccount(GetCurrentMethod(), USERNAME1, PASSWORD1, USERRIGHTS1);
            MakeUserAccount(GetCurrentMethod(), USERNAME2, PASSWORD2, USERRIGHTS2);
            TeamTalk ttclient = NewClientInstance();

            TTMessage msg = new TTMessage();

            Connect(ttclient);
            Login(ttclient, NICKNAME1, USERNAME1, PASSWORD1);

            TeamTalk ttclient2 = NewClientInstance();
            Connect(ttclient2);
            Login(ttclient2, NICKNAME2, USERNAME2, PASSWORD2);

            WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 0);
            WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 0);

            User user = new User();
            Assert.IsFalse(ttclient.GetUser(ttclient2.UserID, ref user));
            Assert.IsFalse(ttclient2.GetUser(ttclient.UserID, ref user));

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoLogout(), DEF_WAIT));
            Assert.IsFalse(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_USER_LOGGEDOUT, 500));

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoLogin(NICKNAME1, USERNAME1, PASSWORD1), DEF_WAIT));

            Assert.IsFalse(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_USER_LOGGEDIN, 500));

            JoinRoot(ttclient);

            Assert.IsFalse(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_USER_JOINED, 500));

            Channel chan = BuildDefaultChannel(ttclient, "view all users test");
            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoJoinChannel(chan), DEF_WAIT));

            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW, DEF_WAIT));

            Assert.IsFalse(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_USER_JOINED, 500));

            WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 500);

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoLogout(), DEF_WAIT));

            Assert.IsFalse(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_CMD_USER_LOGGEDOUT, 500));
            
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_NONE, 0));

            List<TeamTalk> clients = new List<TeamTalk>();
            for (int i = 0; i < 10; i++)
            {
                TeamTalk client = NewClientInstance();
                Connect(client);
                Login(client, NICKNAME2, USERNAME2, PASSWORD2);
                JoinRoot(client);
            }
            foreach (TeamTalk t in clients)
                t.DoQuit();

            Assert.IsFalse(ttclient2.GetMessage(ref msg, 0));
        }

        [TestMethod]
        public void TestChannelClassroom()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL | 
                                         UserRight.USERRIGHT_TRANSMIT_VOICE |
                                         UserRight.USERRIGHT_TRANSMIT_VIDEOCAPTURE |
                                         UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO |
                                         UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO |
                                         UserRight.USERRIGHT_TRANSMIT_DESKTOP;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            const string USERNAME2 = "guest2", PASSWORD2 = "guest2", NICKNAME2 = "TeamTalk.NET2";
            const UserRight USERRIGHTS2 = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                                          UserRight.USERRIGHT_MULTI_LOGIN |
                                         UserRight.USERRIGHT_TRANSMIT_VOICE |
                                         UserRight.USERRIGHT_TRANSMIT_VIDEOCAPTURE |
                                         UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO |
                                         UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO |
                                         UserRight.USERRIGHT_TRANSMIT_DESKTOP;
            MakeUserAccount(GetCurrentMethod(), USERNAME2, PASSWORD2, USERRIGHTS2);

            int cmdid;
            TTMessage msg = new TTMessage();

            Connect(ttclient);
            InitSound(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);

            Channel chan = BuildDefaultChannel(ttclient, "classroom test");
            chan.uChannelType = ChannelType.CHANNEL_CLASSROOM;
            //chan.GetTransmitStreamTypes(ttclient.UserID);
            Assert.IsTrue((cmdid = ttclient.DoJoinChannel(chan))>0);
            
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW, DEF_WAIT, ref msg));
            chan = (Channel)msg.DataToObject();

            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, DEF_WAIT));

            TeamTalk ttclient2 = NewClientInstance();
            InitSound(ttclient2);
            Connect(ttclient2);
            Login(ttclient2, NICKNAME2, USERNAME2, PASSWORD2);
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoJoinChannelByID(chan.nChannelID, chan.szPassword), DEF_WAIT));

            User user;
            Assert.IsTrue(ttclient.EnableVoiceTransmission(true));
            Assert.IsFalse(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_STATECHANGE, 500), "no voice");

            chan.AddTransmitUser(ttclient.UserID, StreamType.STREAMTYPE_VOICE);

            Assert.IsTrue((cmdid = ttclient.DoUpdateChannel(chan)) > 0);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, ref msg));
            chan = (Channel)msg.DataToObject();

            Assert.AreEqual(ttclient.ChannelID, ttclient2.ChannelID);
            Assert.IsTrue(chan.GetTransmitStreamTypes(ttclient.UserID) == StreamType.STREAMTYPE_VOICE, "get transmitUsers");

            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_STATECHANGE, 30000, ref msg), "voice");

            user = (User)msg.DataToObject();
            Assert.AreEqual(ttclient.UserID, user.nUserID);
            Assert.IsTrue(user.uUserState.HasFlag(UserState.USERSTATE_VOICE));

            chan.RemoveTransmitUser(ttclient.UserID, StreamType.STREAMTYPE_VOICE);
            Assert.IsTrue((cmdid = ttclient.DoUpdateChannel(chan)) > 0);
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, DEF_WAIT, ref msg));
            chan = (Channel)msg.DataToObject();

            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_STATECHANGE, 10000, ref msg), "no voice");
            user = (User)msg.DataToObject();
            Assert.AreEqual(ttclient.UserID, user.nUserID);
            Assert.IsFalse(user.uUserState.HasFlag(UserState.USERSTATE_VOICE));
        }

        [TestMethod]
        public void TestChannelNoVoiceActivation()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                                         UserRight.USERRIGHT_TRANSMIT_VOICE |
                                         UserRight.USERRIGHT_TRANSMIT_VIDEOCAPTURE |
                                         UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO |
                                         UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO |
                                         UserRight.USERRIGHT_TRANSMIT_DESKTOP;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            const string USERNAME2 = "guest2", PASSWORD2 = "guest2", NICKNAME2 = "TeamTalk.NET2";
            const UserRight USERRIGHTS2 = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                                          UserRight.USERRIGHT_MULTI_LOGIN |
                                         UserRight.USERRIGHT_TRANSMIT_VOICE |
                                         UserRight.USERRIGHT_TRANSMIT_VIDEOCAPTURE |
                                         UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO |
                                         UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO |
                                         UserRight.USERRIGHT_TRANSMIT_DESKTOP;
            MakeUserAccount(GetCurrentMethod(), USERNAME2, PASSWORD2, USERRIGHTS2);

            int cmdid;
            TTMessage msg = new TTMessage();

            Connect(ttclient);
            InitSound(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);

            Channel chan = BuildDefaultChannel(ttclient, "no voice act test");
            chan.uChannelType = ChannelType.CHANNEL_NO_VOICEACTIVATION;
            //chan.GetTransmitStreamTypes(ttclient.UserID);
            Assert.IsTrue((cmdid = ttclient.DoJoinChannel(chan)) > 0);

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW, DEF_WAIT, ref msg));
            chan = (Channel)msg.DataToObject();

            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, DEF_WAIT));

            TeamTalk ttclient2 = NewClientInstance();
            InitSound(ttclient2);
            Connect(ttclient2);
            Login(ttclient2, NICKNAME2, USERNAME2, PASSWORD2);
            Assert.IsTrue(WaitCmdSuccess(ttclient2, ttclient2.DoJoinChannelByID(chan.nChannelID, chan.szPassword), DEF_WAIT));

            User user;
            Assert.IsTrue(ttclient.EnableVoiceActivation(true));
            Assert.IsFalse(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_STATECHANGE, 500, ref msg), "no voice");

            Assert.IsTrue(ttclient.EnableVoiceActivation(false));

            Assert.IsTrue(ttclient.EnableVoiceTransmission(true));
            Assert.IsTrue(WaitForEvent(ttclient2, ClientEvent.CLIENTEVENT_USER_STATECHANGE, 500, ref msg), "voice");

            user = (User)msg.DataToObject();
            Assert.AreEqual(ttclient.UserID, user.nUserID);
            Assert.IsTrue(user.uUserState.HasFlag(UserState.USERSTATE_VOICE));
        }

        [TestMethod]
        public void TestSoundInputPreprocess()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                                         UserRight.USERRIGHT_TRANSMIT_VOICE;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            int cmdid;

            Connect(ttclient);
            InitSound(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);

            SpeexDSP spxdsp = new SpeexDSP(true), spxdsp2 = new SpeexDSP();
            Assert.IsTrue(ttclient.SetSoundInputPreprocess(spxdsp), "set aud cfg");

            Channel chan = BuildDefaultChannel(ttclient, "foo");
            Assert.AreEqual(chan.audiocodec.nCodec, Codec.OPUS_CODEC);

            Assert.IsTrue(ttclient.GetSoundInputPreprocess(ref spxdsp2));
            Assert.AreEqual(spxdsp2.bEnableAGC, spxdsp.bEnableAGC);
            Assert.AreEqual(spxdsp2.nGainLevel, spxdsp.nGainLevel);
            Assert.AreEqual(spxdsp2.nMaxIncDBSec, spxdsp.nMaxIncDBSec);
            Assert.AreEqual(spxdsp2.nMaxDecDBSec, spxdsp.nMaxDecDBSec);
            Assert.AreEqual(spxdsp2.nMaxGainDB, spxdsp.nMaxGainDB);
            Assert.AreEqual(spxdsp2.bEnableDenoise, spxdsp.bEnableDenoise);
            Assert.AreEqual(spxdsp2.nMaxNoiseSuppressDB, spxdsp.nMaxNoiseSuppressDB);
            Assert.AreEqual(spxdsp2.bEnableEchoCancellation, spxdsp.bEnableEchoCancellation);
            Assert.AreEqual(spxdsp2.nEchoSuppress, spxdsp.nEchoSuppress);
            Assert.AreEqual(spxdsp2.nEchoSuppressActive, spxdsp.nEchoSuppressActive);

            cmdid = ttclient.DoJoinChannel(chan);

        }

        [TestMethod]
        public void TestAudioCodecs()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                                         UserRight.USERRIGHT_TRANSMIT_VOICE;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            Connect(ttclient);
            InitSound(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);

            Channel chan = BuildDefaultChannel(ttclient, "Opus");
            Assert.AreEqual(chan.audiocodec.nCodec, Codec.OPUS_CODEC);

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoJoinChannel(chan), DEF_WAIT));

            Channel chan2 = new Channel();
            Assert.IsTrue(ttclient.GetChannel(ttclient.GetMyChannelID(), ref chan2));
            Assert.AreEqual(chan.audiocodec.nCodec, chan2.audiocodec.nCodec);
            Assert.AreEqual(chan.audiocodec.opus.bDTX, chan2.audiocodec.opus.bDTX);
            Assert.AreEqual(chan.audiocodec.opus.bFEC, chan2.audiocodec.opus.bFEC);
            Assert.AreEqual(chan.audiocodec.opus.bVBR, chan2.audiocodec.opus.bVBR);
            Assert.AreEqual(chan.audiocodec.opus.bVBRConstraint, chan2.audiocodec.opus.bVBRConstraint);
            Assert.AreEqual(chan.audiocodec.opus.nApplication, chan2.audiocodec.opus.nApplication);
            Assert.AreEqual(chan.audiocodec.opus.nBitRate, chan2.audiocodec.opus.nBitRate);
            Assert.AreEqual(chan.audiocodec.opus.nChannels, chan2.audiocodec.opus.nChannels);
            Assert.AreEqual(chan.audiocodec.opus.nComplexity, chan2.audiocodec.opus.nComplexity);
            Assert.AreEqual(chan.audiocodec.opus.nTxIntervalMSec, chan2.audiocodec.opus.nTxIntervalMSec);
            Assert.AreEqual(chan.audiocodec.opus.nSampleRate, chan2.audiocodec.opus.nSampleRate);

            chan.szName = "Speex";
            chan.audiocodec.nCodec = Codec.SPEEX_CODEC;
            chan.audiocodec.speex.bStereoPlayback = true;
            chan.audiocodec.speex.nBandmode = 2;
            chan.audiocodec.speex.nTxIntervalMSec = 80;
            chan.audiocodec.speex.nQuality = 7;

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoJoinChannel(chan), DEF_WAIT));
            Assert.IsTrue(ttclient.GetChannel(ttclient.GetMyChannelID(), ref chan2));

            Assert.AreEqual(chan.audiocodec.nCodec, chan2.audiocodec.nCodec);
            Assert.AreEqual(chan.audiocodec.speex.bStereoPlayback, chan2.audiocodec.speex.bStereoPlayback);
            Assert.AreEqual(chan.audiocodec.speex.nBandmode, chan2.audiocodec.speex.nBandmode);
            Assert.AreEqual(chan.audiocodec.speex.nTxIntervalMSec, chan2.audiocodec.speex.nTxIntervalMSec);
            Assert.AreEqual(chan.audiocodec.speex.nQuality, chan2.audiocodec.speex.nQuality);

            chan.szName = "Speex VBR";
            chan.audiocodec.nCodec = Codec.SPEEX_VBR_CODEC;
            chan.audiocodec.speex_vbr.bDTX = true;
            chan.audiocodec.speex_vbr.bStereoPlayback = true;
            chan.audiocodec.speex_vbr.nBandmode = 2;
            chan.audiocodec.speex_vbr.nBitRate = 22300;
            chan.audiocodec.speex_vbr.nMaxBitRate = 30000;
            chan.audiocodec.speex_vbr.nTxIntervalMSec = 80;
            chan.audiocodec.speex_vbr.nQuality = 4;

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoJoinChannel(chan), DEF_WAIT));
            Assert.IsTrue(ttclient.GetChannel(ttclient.GetMyChannelID(), ref chan2));

            Assert.AreEqual(chan.audiocodec.nCodec, chan2.audiocodec.nCodec);
            Assert.AreEqual(chan.audiocodec.speex_vbr.bDTX, chan2.audiocodec.speex_vbr.bDTX);
            Assert.AreEqual(chan.audiocodec.speex_vbr.bStereoPlayback, chan2.audiocodec.speex_vbr.bStereoPlayback);
            Assert.AreEqual(chan.audiocodec.speex_vbr.nBandmode, chan2.audiocodec.speex_vbr.nBandmode);
            Assert.AreEqual(chan.audiocodec.speex_vbr.nBitRate, chan2.audiocodec.speex_vbr.nBitRate);
            Assert.AreEqual(chan.audiocodec.speex_vbr.nMaxBitRate, chan2.audiocodec.speex_vbr.nMaxBitRate);
            Assert.AreEqual(chan.audiocodec.speex_vbr.nTxIntervalMSec, chan2.audiocodec.speex_vbr.nTxIntervalMSec);
            Assert.AreEqual(chan.audiocodec.speex_vbr.nQuality, chan2.audiocodec.speex_vbr.nQuality);

        }

        [TestMethod]
        public void TestChannelNoRecording()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                                         UserRight.USERRIGHT_TRANSMIT_VOICE;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            TTMessage msg = new TTMessage();

            Connect(ttclient);
            InitSound(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);

            Channel chan = BuildDefaultChannel(ttclient, "Opus");
            Assert.AreEqual(chan.audiocodec.nCodec, Codec.OPUS_CODEC);

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoJoinChannel(chan), DEF_WAIT));

            User myself = new User();
            Assert.IsTrue(ttclient.GetUser(ttclient.UserID, ref myself));
            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoSubscribe(ttclient.UserID, Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

            Assert.IsTrue(ttclient.SetUserMediaStorageDir(ttclient.GetMyUserID(), MEDIAFOLDER, "", AudioFileFormat.AFF_WAVE_FORMAT),
                          "Record user audio files");

            Assert.IsTrue(ttclient.EnableVoiceTransmission(true));

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 1000, ref msg), "Record media file event");

            MediaFileInfo fileinfo = (MediaFileInfo)msg.DataToObject();
            Assert.AreEqual(ttclient.GetMyUserID(), msg.nSource, "Self record voice");
            Assert.IsTrue(fileinfo.nStatus == MediaFileStatus.MFS_STARTED, "Started recording");

            Assert.IsTrue(ttclient.EnableVoiceTransmission(false));

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 5000, ref msg), "Record media file event 2");

            fileinfo = (MediaFileInfo)msg.DataToObject();
            Assert.AreEqual(ttclient.GetMyUserID(), msg.nSource, "Self record voice");
            Assert.IsTrue(fileinfo.nStatus == MediaFileStatus.MFS_FINISHED, "Started recording");

            Assert.IsTrue(ttclient.GetChannel(ttclient.ChannelID, ref chan), "get own chan");
            chan.uChannelType |= ChannelType.CHANNEL_NO_RECORDING;
            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoUpdateChannel(chan), DEF_WAIT), "update chan");

            Assert.IsTrue(ttclient.EnableVoiceTransmission(true));

            Assert.IsFalse(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 500, ref msg), "Record media file event 2");

            Assert.IsTrue(ttclient.EnableVoiceTransmission(false));

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, DEF_WAIT));

            Assert.IsTrue(ttclient.StartRecordingMuxedAudioFile(chan.audiocodec, MUXRECORDFILENAME, AudioFileFormat.AFF_WAVE_FORMAT), "mux in wav-format");

            Assert.IsFalse(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, 5000, ref msg), "Record muxed media file event");
        }

        [TestMethod]
        public void TestAudioBlock()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                                         UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            TTMessage msg = new TTMessage();

            Connect(ttclient);
            InitSound(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);

            Channel chan = BuildDefaultChannel(ttclient, "Opus");
            Assert.AreEqual(chan.audiocodec.nCodec, Codec.OPUS_CODEC);

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoJoinChannel(chan), DEF_WAIT));

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoSubscribe(ttclient.UserID, Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

            Assert.IsTrue(ttclient.EnableVoiceTransmission(true));

            Assert.IsFalse(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000));

            Assert.IsTrue(ttclient.EnableAudioBlockEvent(ttclient.UserID, StreamType.STREAMTYPE_VOICE, true));

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, ref msg));

            Assert.IsTrue((StreamType)msg.DataToObject() == StreamType.STREAMTYPE_VOICE);
            
            AudioBlock block = ttclient.AcquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.UserID);

            Assert.IsTrue(block.nSamples > 0);

            Assert.IsTrue(ttclient.ReleaseUserAudioBlock(block));

            //drain message before we start calculating
            Assert.IsFalse(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 100));

            Assert.IsTrue(ttclient.EnableAudioBlockEvent(ttclient.UserID, StreamType.STREAMTYPE_MEDIAFILE_AUDIO, true));

            Assert.IsTrue(ttclient.StartStreamingMediaFileToChannel(MEDIAFILE_AUDIO, new VideoCodec()));

            int n_voice_blocks = 0, n_mfa_blocks = 0;
            while (n_voice_blocks < 100 || n_mfa_blocks < 100)
            {
                Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, DEF_WAIT, ref msg));
                switch((StreamType)msg.DataToObject())
                {
                    case StreamType.STREAMTYPE_VOICE :
                        n_voice_blocks++;
                        block = ttclient.AcquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, msg.nSource);
                        Assert.IsTrue(block.nSamples > 0);
                        break;
                    case StreamType.STREAMTYPE_MEDIAFILE_AUDIO :
                        n_mfa_blocks++;
                        block = ttclient.AcquireUserAudioBlock(StreamType.STREAMTYPE_MEDIAFILE_AUDIO, msg.nSource);
                        Assert.IsTrue(block.nSamples > 0);
                        ttclient.ReleaseUserAudioBlock(block);
                        break;
                }
            }

            Assert.IsTrue(n_voice_blocks >= 10, "voice ok");
            Assert.IsTrue(n_mfa_blocks >= 10, "media file ok");
        }

        [TestMethod]
        public void TestSpeexSimStereo()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                                         UserRight.USERRIGHT_TRANSMIT_VOICE;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            Connect(ttclient);
            InitSound(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);

            Assert.IsTrue(ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 440));

            Channel chan = BuildDefaultChannel(ttclient, "Speex");
            chan.audiocodec = BuildSpeexCodec();
            chan.audiocodec.speex.bStereoPlayback = true;

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoJoinChannel(chan), DEF_WAIT));

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoSubscribe(ttclient.UserID, Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

            WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 1000);

            Assert.IsTrue(ttclient.EnableVoiceTransmission(true));

            WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 2000);

            Assert.IsTrue(ttclient.EnableVoiceTransmission(false));

            WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 5000);

            Assert.IsTrue(ttclient.EnableVoiceTransmission(true));

            WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 2000);

            Assert.IsTrue(ttclient.EnableVoiceTransmission(false));

            WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, 2000);

        }

        [TestMethod]
        public void TestVoiceTxRx()
        {
            const string USERNAME = "tt_test", PASSWORD = "tt_test"; string NICKNAME = "TeamTalk.NET - " + GetCurrentMethod();
            const UserRight USERRIGHTS = UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                                         UserRight.USERRIGHT_TRANSMIT_VOICE | UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO;
            MakeUserAccount(GetCurrentMethod(), USERNAME, PASSWORD, USERRIGHTS);
            TeamTalk ttclient = NewClientInstance();

            Connect(ttclient);
            InitSound(ttclient);
            Login(ttclient, NICKNAME, USERNAME, PASSWORD);

            Assert.IsTrue(ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 440));

            Channel chan = BuildDefaultChannel(ttclient, "Opus");
            Assert.AreEqual(chan.audiocodec.nCodec, Codec.OPUS_CODEC);

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoJoinChannel(chan), DEF_WAIT));

            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoSubscribe(ttclient.UserID, Subscription.SUBSCRIBE_VOICE), DEF_WAIT));

            Assert.IsTrue(ttclient.EnableAudioBlockEvent(ttclient.UserID, StreamType.STREAMTYPE_VOICE, true));

            VoiceTxRx(ttclient, 30000, 5000, 5000);
            VoiceTxRx(ttclient, 60000, 5000, 30000);
            VoiceTxRx(ttclient, 120000, 5000, 31000);


            //VoiceTxRx(ttclient, 20000, 1000, 200);
            //VoiceTxRx(ttclient, 20000, 1000, 500);
            //VoiceTxRx(ttclient, 30000, 1000, 2000);
            //VoiceTxRx(ttclient, 120000, 20000, 5000);
            //VoiceTxRx(ttclient, 360000, 1000, 65000);
            //VoiceTxRx(ttclient, 500000, 25000, 125000);
            //VoiceTxRx(ttclient, 3600000, 30000, 125000);
        }

        void VoiceTxRx(TeamTalk ttclient, int TEST_DURATION, int VOICE_TX_DURATION, int SILENCE_DURATION)
        {
            Debug.WriteLine("Total Duration {0} {1} {2}", TEST_DURATION, VOICE_TX_DURATION, SILENCE_DURATION);
            Channel chan = new Channel();
            Assert.IsTrue(ttclient.GetChannel(ttclient.ChannelID, ref chan));
            TTMessage msg = new TTMessage();

            int tx_interval = chan.audiocodec.opus.nTxIntervalMSec;
            int test_duration = 0, voice_tx_duration;
            while (test_duration < TEST_DURATION)
            {
                Debug.WriteLine("Duration {0}", test_duration);

                voice_tx_duration = 0;
                AudioBlock block;

                Assert.IsTrue(ttclient.EnableVoiceTransmission(true));

                Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, 1000, ref msg));
                Assert.IsTrue(((User)msg.DataToObject()).uUserState.HasFlag(UserState.USERSTATE_VOICE));

                do
                {
                    Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 1000));
                    block = ttclient.AcquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.UserID);
                    Assert.IsTrue(block.nStreamID > 0);
                    Assert.IsTrue(ttclient.ReleaseUserAudioBlock(block));
                }
                while ((voice_tx_duration += tx_interval) < VOICE_TX_DURATION);

                Assert.IsTrue(ttclient.EnableVoiceTransmission(false));

                Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_STATECHANGE, 5000, ref msg));
                Assert.IsFalse(((User)msg.DataToObject()).uUserState.HasFlag(UserState.USERSTATE_VOICE));

                WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_NONE, SILENCE_DURATION);

                //drain queue
                do
                {
                    block = ttclient.AcquireUserAudioBlock(StreamType.STREAMTYPE_VOICE, ttclient.UserID);
                    if(block.nStreamID>0)
                        ttclient.ReleaseUserAudioBlock(block);
                }
                while(block.nStreamID>0);

                while(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, 0))


                test_duration += voice_tx_duration;
                test_duration += SILENCE_DURATION;
            }
        }

        private void InitSound(TeamTalk ttclient)
        {
            int devin = 0, devout = 0;
            Assert.IsTrue(TeamTalk.GetDefaultSoundDevicesEx(SoundSystem.SOUNDSYSTEM_WASAPI,
                                                 ref devin, ref devout), "Get default DSound devices");

            SpeexDSP spxdsp = new SpeexDSP();
            spxdsp.bEnableAGC = true;
            spxdsp.nGainLevel = SpeexDSPConstants.DEFAULT_AGC_GAINLEVEL;
            spxdsp.nMaxIncDBSec = SpeexDSPConstants.DEFAULT_AGC_INC_MAXDB;
            spxdsp.nMaxDecDBSec = SpeexDSPConstants.DEFAULT_AGC_DEC_MAXDB;
            spxdsp.nMaxGainDB = SpeexDSPConstants.DEFAULT_AGC_GAINMAXDB;
            spxdsp.bEnableDenoise = true;
            spxdsp.nMaxNoiseSuppressDB = SpeexDSPConstants.DEFAULT_DENOISE_SUPPRESS;
            spxdsp.bEnableEchoCancellation = true;
            spxdsp.nEchoSuppress = SpeexDSPConstants.DEFAULT_ECHO_SUPPRESS;
            spxdsp.nEchoSuppressActive = SpeexDSPConstants.DEFAULT_ECHO_SUPPRESS_ACTIVE;

            Assert.IsTrue(ttclient.InitSoundInputDevice(devin), "Init sound input");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_SNDINPUT_READY), "Input ready");

            Assert.IsTrue(ttclient.InitSoundOutputDevice(devout), "Init sound output");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_SNDOUTPUT_READY), "Output ready");
        }

        private static void Connect(TeamTalk ttclient)
        {
            Assert.IsTrue(ttclient.Connect(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED), "connect call");

            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CON_SUCCESS, 1000), "wait connect");
        }

        private static void Login(TeamTalk ttclient, string nick, string username, string passwd)
        {
            int cmdid = ttclient.DoLogin(nick, username, passwd);
            Assert.IsTrue(cmdid > 0, "do login");

            TTMessage msg = new TTMessage();
            Assert.IsTrue(WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDIN, DEF_WAIT, ref msg), "wait login");

            UserAccount account = (UserAccount)msg.DataToObject();
            Assert.AreEqual(username, account.szUsername, "username set");
            //Assert.AreEqual(passwd, account.szPassword, "password set");
            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, 1000), "Wait login complete");
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED));
        }

        private void MakeUserAccount(string nickname, string username, string password, UserRight userrights)
        {
            TeamTalk ttclient = NewClientInstance();
            Connect(ttclient);
            Login(ttclient, nickname, ADMIN_USERNAME, ADMIN_PASSWORD);
            UserAccount useraccount = new UserAccount();
            useraccount.szUsername = username;
            useraccount.szPassword = password;
            useraccount.uUserRights = userrights;
            useraccount.uUserType = UserType.USERTYPE_DEFAULT;
            Assert.IsTrue(WaitCmdSuccess(ttclient, ttclient.DoNewUserAccount(useraccount), DEF_WAIT));
            Assert.IsTrue(ttclient.Disconnect());
        }

        private static void JoinRoot(TeamTalk ttclient)
        {
            Assert.IsTrue(ttclient.Flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED), "Auth ok");

            Assert.IsTrue(ttclient.GetRootChannelID() > 0, "root exists");

            int cmdid = ttclient.DoJoinChannelByID(ttclient.GetRootChannelID(), "");
            
            Assert.IsTrue(cmdid > 0, "do join root");

            Assert.IsTrue(WaitCmdComplete(ttclient, cmdid, 1000), "Wait join complete");
        }

        private static bool WaitForEvent(TeamTalk ttclient, ClientEvent e, int waittimeout, ref TTMessage msg)
        {
            long start = DateTime.Now.Ticks / 10000;
            TTMessage tmp = new TTMessage();
            while (ttclient.GetMessage(ref tmp, waittimeout) && tmp.nClientEvent != e)
            {
                ttclient.ProcessMsg(tmp);
                if(DEBUG_OUTPUT)
                    Debug.WriteLine(DateTime.Now + " #" + ttclient.GetMyUserID() + ": " + tmp.nClientEvent);
                if(DateTime.Now.Ticks / 10000 - start >= waittimeout)
                    break;
            }

            if (tmp.nClientEvent == e)
            {
                ttclient.ProcessMsg(tmp);
                if (DEBUG_OUTPUT)
                    Debug.WriteLine(DateTime.Now + " #" + ttclient.GetMyUserID() + ": " + tmp.nClientEvent);
                msg = tmp;
            }
            return tmp.nClientEvent == e;
        }

        private static bool WaitForEvent(TeamTalk ttclient, ClientEvent e, int waittimeout)
        {
            TTMessage msg = new TTMessage();
            return WaitForEvent(ttclient, e, waittimeout, ref msg);
        }

        private static bool WaitCmdComplete(TeamTalk ttclient, int cmdid, int waittimeout)
        {
            TTMessage msg = new TTMessage();

            while (WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_PROCESSING, waittimeout, ref msg))
            {
                if (msg.nSource == cmdid && (bool)msg.DataToObject() == false)
                    return true;
            }
            return false;
        }

        private static bool WaitCmdSuccess(TeamTalk ttclient, int cmdid, int waittimeout)
        {
            TTMessage msg = new TTMessage();

            while (WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, waittimeout, ref msg))
            {
                if (msg.nSource == cmdid)
                    return true;
            }

            return false;
        }

        private static bool WaitCmdError(TeamTalk ttclient, int cmdid, int waittimeout)
        {
            TTMessage msg = new TTMessage();

            while (WaitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_ERROR, waittimeout, ref msg))
            {
                if (msg.nSource == cmdid)
                    return true;
            }

            return false;
        }

        private static Channel BuildDefaultChannel(TeamTalk ttclient, string name)
        {
            Channel chan = new Channel(true);
            chan.nParentID = ttclient.GetRootChannelID();
            chan.szName = name;
            chan.szTopic = "a topic";
            chan.nMaxUsers = 128;
            chan.audiocodec.nCodec = Codec.OPUS_CODEC;
            chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
            chan.audiocodec.opus.nChannels = 1;
            chan.audiocodec.opus.nBitRate = 64000;
            chan.audiocodec.opus.nComplexity = 5;
            chan.audiocodec.opus.nTxIntervalMSec = 20;
            chan.audiocodec.opus.nSampleRate = 48000;
            chan.audiocodec.opus.bDTX = true;
            chan.audiocodec.opus.bFEC = true;
            chan.audiocodec.opus.bVBR = true;
            chan.audiocodec.opus.bVBRConstraint = false;
            return chan;
        }

        private static AudioCodec BuildSpeexCodec()
        {
            AudioCodec codec = new AudioCodec();
            codec.nCodec = Codec.SPEEX_CODEC;
            codec.speex.nBandmode = 1;
            codec.speex.nQuality = 4;
            codec.speex.nTxIntervalMSec = 40;
            codec.speex.bStereoPlayback = false;

            return codec;
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        public static string GetCurrentMethod()
        {
            StackTrace st = new StackTrace();
            StackFrame sf = st.GetFrame(1);

            return sf.GetMethod().Name;
        }

        TeamTalk NewClientInstance()
        {
            TeamTalk ttclient = new TeamTalk(true);
            ttclients.Add(ttclient);
            return ttclient;
        }
    }

}
