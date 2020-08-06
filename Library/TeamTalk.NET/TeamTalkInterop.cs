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

using System;
using System.Text;
#if PocketPC
using Microsoft.WindowsCE.Forms;
#endif
using System.Windows.Forms;
using System.Runtime.InteropServices;


namespace c_tt
{

#if PocketPC
    class MyEventHandler : MessageWindow
#else
    class MyEventHandler : System.Windows.Forms.UserControl
#endif
    {
        public const int WM_TEAMTALK_CLIENTEVENT = 0x8000 + 1; /* WM_APP */

        public MyEventHandler(BearWare.TeamTalkBase tt)
        {
            m_tt = tt;
        }

        protected BearWare.TeamTalkBase m_tt;
        protected override void WndProc(ref Message m)
        {
            if (m.Msg == WM_TEAMTALK_CLIENTEVENT)
            {
                BearWare.TTMessage msg = new BearWare.TTMessage();
                if(m_tt.GetMessage(ref msg, 0))
                    m_tt.ProcessMsg(msg);
            }
            else
                base.WndProc(ref m);
        }
    }

    class TTDLL
    {
#if ENABLE_TEAMTALKPRO

        public const string dllname = "TeamTalk5Pro.dll";
        public const string mgtdllname = "TeamTalk5Pro.NET.dll";

#else

        public const string dllname = "TeamTalk5.dll";
        public const string mgtdllname = "TeamTalk5.NET.dll";

#endif
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr TT_GetVersion();
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr TT_InitTeamTalk(IntPtr hWnd, uint uMsg);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr TT_InitTeamTalkPoll();
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_CloseTeamTalk(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetMessage(IntPtr lpTTInstance,
                                               ref BearWare.TTMessage pMsg,
                                               ref int pnWaitMs);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_PumpMessage(IntPtr lpTTInstance,
                                                 BearWare.ClientEvent nClientEvent,
                                                 int nIdentifier);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern BearWare.ClientFlag TT_GetFlags(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetLicenseInformation([MarshalAs(UnmanagedType.LPWStr)] string szRegName,
                                                           [MarshalAs(UnmanagedType.LPWStr)] string szRegKey);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetDefaultSoundDevices(ref int lpnInputDeviceID,
                                                            ref int lpnOutputDeviceID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetDefaultSoundDevicesEx(BearWare.SoundSystem nSoundSystem,
                                                              ref int lpnInputDeviceID,
                                                              ref int lpnOutputDeviceID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetSoundDevices([In, Out] BearWare.SoundDevice[] lpSoundDevices,
                                                     ref int lpnHowMany);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode, EntryPoint = "TT_GetSoundDevices")] /* Windows CE NULL pointer workaround */
        public static extern bool TT_GetSoundDevices_NULL(IntPtr lpSoundDevices,
                                                          ref int lpnHowMany);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_RestartSoundSystem();
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr TT_StartSoundLoopbackTest(int nInputDeviceID, int nOutputDeviceID,
                                                              int nSampleRate, int nChannels,
                                                              bool bDuplexMode,
                                                              ref BearWare.SpeexDSP lpSpeexDSP);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr TT_StartSoundLoopbackTestEx(int nInputDeviceID, int nOutputDeviceID,
                                                                int nSampleRate, int nChannels,
                                                                bool bDuplexMode,
                                                                ref BearWare.AudioPreprocessor lpAudioPreprocessor,
                                                                ref BearWare.SoundDeviceEffects lpSoundDeviceEffects);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_CloseSoundLoopbackTest(IntPtr lpTTSoundLoop);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_InitSoundInputDevice(IntPtr lpTTInstance,
                                                     int nInputDeviceID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_InitSoundInputSharedDevice(int nSampleRate, int nChannels, int nFrameSize);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_InitSoundOutputDevice(IntPtr lpTTInstance,
                                                      int nOutputDeviceID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_InitSoundOutputSharedDevice(int nSampleRate, int nChannels, int nFrameSize);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_InitSoundDuplexDevices(IntPtr lpTTInstance,
                                                      int nInputDeviceID,
                                                      int nOutputDeviceID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_CloseSoundInputDevice(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_CloseSoundOutputDevice(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_CloseSoundDuplexDevices(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetSoundDeviceEffects(IntPtr lpTTInstance, ref BearWare.SoundDeviceEffects lpSoundDeviceEffect);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetSoundDeviceEffects(IntPtr lpTTInstance, ref BearWare.SoundDeviceEffects lpSoundDeviceEffect);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_GetSoundInputLevel(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetSoundInputGainLevel(IntPtr lpTTInstance,
                                                       int nLevel);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_GetSoundInputGainLevel(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetSoundInputPreprocess(IntPtr lpTTInstance, ref BearWare.SpeexDSP lpSpeexDSP);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetSoundInputPreprocess(IntPtr lpTTInstance, ref BearWare.SpeexDSP lpSpeexDSP);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetSoundInputPreprocessEx(IntPtr lpTTInstance, ref BearWare.AudioPreprocessor lpAudioPreprocessor);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetSoundInputPreprocessEx(IntPtr lpTTInstance, ref BearWare.AudioPreprocessor lpAudioPreprocessor);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetSoundOutputVolume(IntPtr lpTTInstance,
                                                     int nVolume);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_GetSoundOutputVolume(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetSoundOutputMute(IntPtr lpTTInstance,
                                                   bool bMuteAll);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Enable3DSoundPositioning(IntPtr lpTTInstance,
                                                         bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_AutoPositionUsers(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_EnableAudioBlockEvent(IntPtr lpTTInstance, int nUserID, 
                                                           BearWare.StreamType nStreamType, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_EnableAudioBlockEventEx(IntPtr lpTTInstance, int nUserID,
                                                           BearWare.StreamType nStreamType, ref BearWare.AudioFormat lpAudioFormat, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_InsertAudioBlock(IntPtr lpTTInstance, ref BearWare.AudioBlock lpAudioBlock);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_EnableVoiceTransmission(IntPtr lpTTInstance, bool bEnable);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_EnableVoiceActivation(IntPtr lpTTInstance,
                                                      bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetVoiceActivationLevel(IntPtr lpTTInstance,
                                                        int nLevel);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_GetVoiceActivationLevel(IntPtr lpTTInstance);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetVoiceActivationStopDelay(IntPtr lpTTInstance,
                                                                 int nDelayMSec);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_GetVoiceActivationStopDelay(IntPtr lpTTInstance);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_StartRecordingMuxedAudioFile(IntPtr lpTTInstance,
                                                       ref BearWare.AudioCodec lpAudioCodec,
                    [MarshalAs(UnmanagedType.LPWStr)] string szAudioFileName,
                                                       BearWare.AudioFileFormat aff);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_StartRecordingMuxedAudioFileEx(IntPtr lpTTInstance,
                                                       int nChannelID,
                    [MarshalAs(UnmanagedType.LPWStr)] string szAudioFileName,
                                                       BearWare.AudioFileFormat aff);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_StopRecordingMuxedAudioFile(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_StopRecordingMuxedAudioFileEx(IntPtr lpTTInstance, int nChannelID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_StartVideoCaptureTransmission(IntPtr lpTTInstance, ref BearWare.VideoCodec lpVideoCodec);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_StopVideoCaptureTransmission(IntPtr lpTTInstance);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode, EntryPoint = "TT_GetVideoCaptureDevices")] /* Windows CE NULL pointer workaround */
        public static extern bool TT_GetVideoCaptureDevices_NULL(IntPtr lpVideoDevices,
                                                                 ref int lpnHowMany);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetVideoCaptureDevices([In, Out] BearWare.VideoCaptureDevice[] lpVideoDevices,
                                                            ref int lpnHowMany);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_InitVideoCaptureDevice(IntPtr lpTTInstance,
                                                       [MarshalAs(UnmanagedType.LPWStr)] string szDeviceID,
                                                      ref BearWare.VideoFormat lpVideoFormat);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_CloseVideoCaptureDevice(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_PaintVideoFrame(int nUserID, IntPtr hDC, int nPosX,
                                                     int nPosY, int nWidth, int nHeight, ref BearWare.VideoFrame lpVideoFrame);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_PaintVideoFrameEx(int nUserID, IntPtr hDC, int XDest,
                                                       int YDest, int nDestWidth, int nDestHeight,
                                                       int XSrc, int YSrc, int nSrcWidth, int nSrcHeight,
                                                       ref BearWare.VideoFrame lpVideoFrame);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern System.IntPtr TT_AcquireUserVideoCaptureFrame(IntPtr lpTTInstance, int nUserID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_ReleaseUserVideoCaptureFrame(IntPtr lpTTInstance, System.IntPtr lpVideoFrame);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_StartStreamingMediaFileToChannel(IntPtr lpTTInstance,
                                                                     [MarshalAs(UnmanagedType.LPWStr)] string szMediaFilePath,
                                                                     ref BearWare.VideoCodec lpVideoCodec);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_StartStreamingMediaFileToChannelEx(IntPtr lpTTInstance,
                                                                     [MarshalAs(UnmanagedType.LPWStr)] string szMediaFilePath,
                                                                     ref BearWare.MediaFilePlayback lpMediaFilePlayback,
                                                                     ref BearWare.VideoCodec lpVideoCodec);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_UpdateStreamingMediaFileToChannel(IntPtr lpTTInstance,
                                                                       ref BearWare.MediaFilePlayback lpMediaFilePlayback,
                                                                       ref BearWare.VideoCodec lpVideoCodec);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_StopStreamingMediaFileToChannel(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_InitLocalPlayback(IntPtr lpTTInstance,
                                                      [MarshalAs(UnmanagedType.LPWStr)] string szMediaFilePath,
                                                      ref BearWare.MediaFilePlayback lpMediaFilePlayback);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_UpdateLocalPlayback(IntPtr lpTTInstance, int nPlaybackSessionID,
                                                      ref BearWare.MediaFilePlayback lpMediaFilePlayback);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_StopLocalPlayback(IntPtr lpTTInstance, int nPlaybackSessionID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetMediaFileInfo([MarshalAs(UnmanagedType.LPWStr)] string szMediaFilePath,
                                                      ref BearWare.MediaFileInfo pMediaFileInfo);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern System.IntPtr TT_AcquireUserMediaVideoFrame(IntPtr lpTTInstance, int nUserID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_ReleaseUserMediaVideoFrame(IntPtr lpTTInstance, System.IntPtr lpVideoFrame);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_SendDesktopWindow(IntPtr lpTTInstance,
                                               ref BearWare.DesktopWindow lpDesktopWindow,
                                               BearWare.BitmapFormat nConvertBmpFormat);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_CloseDesktopWindow(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr TT_Palette_GetColorTable(BearWare.BitmapFormat nBmpPalette,
                                                             int nIndex);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr TT_Windows_GetDesktopActiveHWND();
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr TT_Windows_GetDesktopHWND();
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Windows_GetDesktopWindowHWND(int nIndex,
                                                                  ref IntPtr lpHWnd);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Windows_GetWindow(IntPtr hWnd,
                                                       ref BearWare.ShareWindow lpShareWindow);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_SendDesktopWindowFromHWND(IntPtr lpTTInstance,
                                                       IntPtr hWnd,
                                                       BearWare.BitmapFormat nBitmapFormat,
                                                       BearWare.DesktopProtocol nDesktopProtocol);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_PaintDesktopWindow(IntPtr lpTTInstance,
                                                        int nUserID,
                                                        IntPtr hDC,
                                                        int XDest,
                                                        int YDest,
                                                        int nDestWidth,
                                                        int nDestHeight);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_PaintDesktopWindowEx(IntPtr lpTTInstance,
                                                          int nUserID,
                                                         IntPtr hDC,
                                                         int XDest,
                                                         int YDest,
                                                         int nDestWidth,
                                                         int nDestHeight,
                                                         int XSrc,
                                                         int YSrc,
                                                         int nSrcWidth,
                                                         int nSrcHeight);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SendDesktopCursorPosition(IntPtr lpTTInstance,
                                                                ushort nPosX,
                                                                ushort nPosY);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SendDesktopInput(IntPtr lpTTInstance,
                                                      int nUserID,
                                                      [In, Out] BearWare.DesktopInput[] lpDesktopInputs,
                                                      int nDesktopInputCount);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr TT_AcquireUserDesktopWindow(IntPtr lpTTInstance, 
                                                                int nUserID);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr TT_AcquireUserDesktopWindowEx(IntPtr lpTTInstance,
                                                                  int nUserID,
                                                                  BearWare.BitmapFormat nBitmapFormat);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_ReleaseUserDesktopWindow(IntPtr lpTTInstance,
                                                              IntPtr lpDesktopWindow);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Connect(IntPtr lpTTInstance,
                                             [MarshalAs(UnmanagedType.LPWStr)] string szHostAddress,
                                             int nTcpPort,
                                             int nUdpPort,
                                             int nLocalTcpPort,
                                             int nLocalUdpPort,
                                             bool bEncrypted);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_ConnectSysID(IntPtr lpTTInstance,
                                                  [MarshalAs(UnmanagedType.LPWStr)] string szHostAddress,
                                                  int nTcpPort,
                                                  int nUdpPort,
                                                  int nLocalTcpPort,
                                                  int nLocalUdpPort,
                                                  bool bEncrypted,
                                                  [MarshalAs(UnmanagedType.LPWStr)] string szSystemID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_ConnectEx(IntPtr lpTTInstance,
                                               [MarshalAs(UnmanagedType.LPWStr)] string szHostAddress,
                                               int nTcpPort,
                                               int nUdpPort,
                                               [MarshalAs(UnmanagedType.LPWStr)] string szBindIPAddr,
                                               int nLocalTcpPort,
                                               int nLocalUdpPort,
                                               bool bEncrypted);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Disconnect(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_QueryMaxPayload(IntPtr lpTTInstance, int nUserID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetClientStatistics(IntPtr lpTTInstance,
                                                         ref BearWare.ClientStatistics lpStats);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetClientKeepAlive(IntPtr lpTTInstance,
                                             ref BearWare.ClientKeepAlive lpClientKeepAlive);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetClientKeepAlive(IntPtr lpTTInstance,
                                             ref BearWare.ClientKeepAlive lpClientKeepAlive);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoPing(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoLogin(IntPtr lpTTInstance,
                                         [MarshalAs(UnmanagedType.LPWStr)] string szNickname,
                                         [MarshalAs(UnmanagedType.LPWStr)] string szUsername,
                                         [MarshalAs(UnmanagedType.LPWStr)] string szPassword);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoLoginEx(IntPtr lpTTInstance,
                                         [MarshalAs(UnmanagedType.LPWStr)] string szNickname,
                                         [MarshalAs(UnmanagedType.LPWStr)] string szUsername,
                                         [MarshalAs(UnmanagedType.LPWStr)] string szPassword,
                                         [MarshalAs(UnmanagedType.LPWStr)] string szClientName);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoLogout(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoJoinChannel(IntPtr lpTTInstance,
                                               ref BearWare.Channel lpChannel);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoJoinChannelByID(IntPtr lpTTInstance,
                                                   int nChannelID,
                                                   [MarshalAs(UnmanagedType.LPWStr)] string szPassword);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoLeaveChannel(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoChangeNickname(IntPtr lpTTInstance,
                                              [MarshalAs(UnmanagedType.LPWStr)] string szNewNick);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoChangeStatus(IntPtr lpTTInstance,
                                                int nStatusMode,
                                                [MarshalAs(UnmanagedType.LPWStr)] string szStatusMessage);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoTextMessage(IntPtr lpTTInstance,
                                               ref BearWare.TextMessage lpTextMessage);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoChannelOp(IntPtr lpTTInstance,
                                             int nUserID,
                                             int nChannelID,
                                             bool bMakeOperator);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoChannelOpEx(IntPtr lpTTInstance,
                                               int nUserID,
                                               int nChannelID,
                                               [MarshalAs(UnmanagedType.LPWStr)] string szOpPassword,
                                               bool bMakeOperator);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoKickUser(IntPtr lpTTInstance,
                                            int nUserID,
                                            int nChannelID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoSendFile(IntPtr lpTTInstance,
                                            int nChannelID,
                                            [MarshalAs(UnmanagedType.LPWStr)] string szLocalFilePath);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoRecvFile(IntPtr lpTTInstance,
                                            int nChannelID,
                                            int nFileID,
                                            [MarshalAs(UnmanagedType.LPWStr)] string szLocalFilePath);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoDeleteFile(IntPtr lpTTInstance,
                                              int nChannelID,
                                              int nFileID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoSubscribe(IntPtr lpTTInstance,
                                             int nUserID,
                                             BearWare.Subscription uSubscriptions);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoUnsubscribe(IntPtr lpTTInstance,
                                               int nUserID,
                                               BearWare.Subscription uSubscriptions);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoMakeChannel(IntPtr lpTTInstance,
                                               ref BearWare.Channel lpChannel);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoUpdateChannel(IntPtr lpTTInstance,
                                                 ref BearWare.Channel lpChannel);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoRemoveChannel(IntPtr lpTTInstance,
                                                 int nChannelID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoMoveUser(IntPtr lpTTInstance,
                                            int nUserID,
                                            int nChannelID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoUpdateServer(IntPtr lpTTInstance,
                                                ref BearWare.ServerProperties lpServerInfo);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoListUserAccounts(IntPtr lpTTInstance,
                                                    int nIndex,
                                                    int nCount);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoNewUserAccount(IntPtr lpTTInstance,
                                                  ref BearWare.UserAccount lpUserAccount);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoDeleteUserAccount(IntPtr lpTTInstance,
                                                     [MarshalAs(UnmanagedType.LPWStr)] string szUsername);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoBanUser(IntPtr lpTTInstance,
                                              int nUserID, int nChannelID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoBanUserEx(IntPtr lpTTInstance,
                                                int nUserID, BearWare.BanType uBanTypes);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoBan(IntPtr lpTTInstance, ref BearWare.BannedUser lpBannedUser);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoBanIPAddress(IntPtr lpTTInstance,
                                                   [MarshalAs(UnmanagedType.LPWStr)] string szIPAddress,
                                                   int nChannelID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoUnBanUser(IntPtr lpTTInstance,
                                                [MarshalAs(UnmanagedType.LPWStr)] string szIPAddress,
                                                int nChannelID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoUnBanUserEx(IntPtr lpTTInstance,
                                                  ref BearWare.BannedUser lpBannedUser);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoListBans(IntPtr lpTTInstance, int nChannelID, int nIndex, int nCount);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoSaveConfig(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoQueryServerStats(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DoQuit(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetServerProperties(IntPtr lpTTInstance,
                                                    ref BearWare.ServerProperties lpProperties);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode, EntryPoint = "TT_GetServerUsers")] /* Windows CE NULL pointer workaround */
        public static extern bool TT_GetServerUsers_NULL(IntPtr lpTTInstance,
                                                IntPtr lpUsers,
                                                ref int lpnHowMany);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetServerUsers(IntPtr lpTTInstance,
                                                [In, Out] BearWare.User[] lpUsers,
                                                ref int lpnHowMany);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_GetRootChannelID(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_GetMyChannelID(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetChannel(IntPtr lpTTInstance,
                                           int nChannelID,
                                           ref BearWare.Channel lpChannel);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetChannelPath(IntPtr lpTTInstance,
                                               int nChannelID,
                                               IntPtr szChannelPath);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_GetChannelIDFromPath(IntPtr lpTTInstance,
                                                      [MarshalAs(UnmanagedType.LPWStr)] string szChannelPath);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode, EntryPoint = "TT_GetChannelUsers")] /* Windows CE NULL pointer workaround */
        public static extern bool TT_GetChannelUsers_NULL(IntPtr lpTTInstance,
                                                int nChannelID,
                                                IntPtr lpUsers,
                                                ref int lpnHowMany);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetChannelUsers(IntPtr lpTTInstance,
                                                int nChannelID,
                                                [In, Out] BearWare.User[] lpUsers,
                                                ref int lpnHowMany);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode, EntryPoint = "TT_GetChannelFiles")] /* Windows CE NULL pointer workaround */
        public static extern bool TT_GetChannelFiles_NULL(IntPtr lpTTInstance,
                                                int nChannelID,
                                                IntPtr lpFileInfos,
                                                ref int lpnHowMany);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetChannelFiles(IntPtr lpTTInstance,
                                                int nChannelID,
                                                [In, Out] BearWare.RemoteFile[] lpRemoteFiles,
                                                ref int lpnHowMany);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetChannelFile(IntPtr lpTTInstance,
                                                    int nChannelID,
                                                    int nFileID,
                                                    ref BearWare.RemoteFile lpRemoteFile);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_IsChannelOperator(IntPtr lpTTInstance,
                                                  int nUserID,
                                                  int nChannelID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode, EntryPoint = "TT_GetServerChannels")] /* Windows CE NULL pointer workaround */
        public static extern bool TT_GetServerChannels_NULL(IntPtr lpTTInstance,
                                                IntPtr lpUserIDs,
                                                ref int lpnHowMany);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetServerChannels(IntPtr lpTTInstance,
                                                [In, Out] BearWare.Channel[] lpChannels,
                                                ref int lpnHowMany);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_GetMyUserID(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetMyUserAccount(IntPtr lpTTInstance,
                                                 ref BearWare.UserAccount lpUserAccount);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern BearWare.UserType TT_GetMyUserType(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern BearWare.UserRight TT_GetMyUserRights(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_GetMyUserData(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetUser(IntPtr lpTTInstance,
                                        int nUserID, ref BearWare.User lpUser);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetUserStatistics(IntPtr lpTTInstance,
                                                  int nUserID,
                                                  ref BearWare.UserStatistics lpStats);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetUserByUsername(IntPtr lpTTInstance,
                                                  [MarshalAs(UnmanagedType.LPWStr)] string szUsername,
                                                 ref BearWare.User lpUser);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetTextMessage(IntPtr lpTTInstance,
                                               int nMsgID,
                                               bool bRemoveMsg,
                                               ref BearWare.TextMessage lpTextMessage);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetUserVolume(IntPtr lpTTInstance,
                                              int nUserID, BearWare.StreamType nStreamType, int nVolume);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetUserMute(IntPtr lpTTInstance,
                                            int nUserID, BearWare.StreamType nStreamType, bool bMute);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetUserStoppedPlaybackDelay(IntPtr lpTTInstance,
                                                                 int nUserID,
                                                                 BearWare.StreamType nStreamType,
                                                                 int nDelayMSec);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetUserPosition(IntPtr lpTTInstance,
                                                int nUserID,
                                                BearWare.StreamType nStreamType,
                                                float x,
                                                float y,
                                                float z);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetUserStereo(IntPtr lpTTInstance,
                                              int nUserID,
                                              BearWare.StreamType nStreamType, 
                                              bool bLeftSpeaker,
                                              bool bRightSpeaker);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetUserMediaStorageDir(IntPtr lpTTInstance,
                                                   int nUserID,
                                                   [MarshalAs(UnmanagedType.LPWStr)] string szFolderPath,
                                                   [MarshalAs(UnmanagedType.LPWStr)] string szFileNameVars,
                                                   BearWare.AudioFileFormat uAFF);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_SetUserAudioStreamBufferSize(IntPtr lpTTInstance,
                                                            int nUserID,
                                                            BearWare.StreamType uStreamType,
                                                            int nMSec);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr TT_AcquireUserAudioBlock(IntPtr lpTTInstance,
                                                             BearWare.StreamType nStreamType,
                                                             int nUserID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_ReleaseUserAudioBlock(IntPtr lpTTInstance, IntPtr lpAudioBlock);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_GetFileTransferInfo(IntPtr lpTTInstance,
                                                    int nTransferID,
                                                    ref BearWare.FileTransfer lpFileTransfer);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_CancelFileTransfer(IntPtr lpTTInstance,
                                                   int nTransferID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void TT_GetErrorMessage(int nError,
                                                IntPtr szErrorMsg);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DesktopInput_KeyTranslate(BearWare.TTKeyTranslate nTranslate,
                                                              [In] BearWare.DesktopInput[] lpDesktopInputs,
                                                             [In, Out] BearWare.DesktopInput[] lpTranslatedDesktopInputs,
                                                              int nDesktopInputCount);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DesktopInput_Execute([In, Out] BearWare.DesktopInput[] lpDesktopInputs,
                                                         int nDesktopInputCount);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_HotKey_Register(IntPtr lpTTInstance,
                                                int nHotKeyID,
                                                [In, Out] int[] lpnVKCodes,
                                                int nVKCodeCount);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_HotKey_Unregister(IntPtr lpTTInstance,
                                                  int nHotKeyID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_HotKey_IsActive(IntPtr lpTTInstance,
                                                 int nHotKeyID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_HotKey_InstallTestHook(IntPtr lpTTInstance,
                                                            /* HWND */ IntPtr hWnd, uint uMsg);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_HotKey_RemoveTestHook(IntPtr lpTTInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_HotKey_GetKeyString(IntPtr lpTTInstance,
                                                    int nVKCode,
                                                    IntPtr szKeyName);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_DBG_SIZEOF(BearWare.TTType nType);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_DBG_SetSoundInputTone(IntPtr lpTTInstance,
                                                           BearWare.StreamType uStreamTypes,
                                                           int nFrequency);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_DBG_WriteAudioFileTone(ref BearWare.MediaFileInfo lpMediaFileInfo,
                                                            int nFrequency);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern System.IntPtr TT_DBG_GETDATAPTR(ref BearWare.TTMessage pMsg);

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_Mixer_GetMixerCount();
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Mixer_GetMixerName(int nMixerIndex,
                                               IntPtr szMixerName);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Mixer_GetWaveInName(int nWaveDeviceID,
                                                IntPtr szMixerName);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Mixer_GetWaveOutName(int nWaveDeviceID,
                                                 IntPtr szMixerName);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Mixer_SetWaveOutMute(int nWaveDeviceID,
                                                 BearWare.MixerControl nControl,
                                                 bool bMute);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_Mixer_GetWaveOutMute(int nWaveDeviceID,
                                                  BearWare.MixerControl nControl);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Mixer_SetWaveOutVolume(int nWaveDeviceID,
                                                   BearWare.MixerControl nControl,
                                                   int nVolume);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_Mixer_GetWaveOutVolume(int nWaveDeviceID,
                                                    BearWare.MixerControl nControl);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Mixer_SetWaveInSelected(int nWaveDeviceID,
                                                    BearWare.MixerControl nControl);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_Mixer_GetWaveInSelected(int nWaveDeviceID,
                                                     BearWare.MixerControl nControl);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Mixer_SetWaveInVolume(int nWaveDeviceID,
                                                  BearWare.MixerControl nControl,
                                                  int nVolume);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_Mixer_GetWaveInVolume(int nWaveDeviceID,
                                                   BearWare.MixerControl nControl);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Mixer_SetWaveInBoost(int nWaveDeviceID,
                                                 bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_Mixer_GetWaveInBoost(int nWaveDeviceID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Mixer_SetWaveInMute(int nWaveDeviceID,
                                                bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_Mixer_GetWaveInMute(int nWaveDeviceID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TT_Mixer_GetWaveInControlCount(int nWaveDeviceID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Mixer_GetWaveInControlName(int nWaveDeviceID,
                                                       int nControlIndex,
                                                       IntPtr szDeviceName);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Mixer_SetWaveInControlSelected(int nWaveDeviceID,
                                                           int nControlIndex);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Mixer_GetWaveInControlSelected(int nWaveDeviceID,
                                                            int nControlIndex);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Firewall_IsEnabled();
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Firewall_Enable(bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Firewall_AppExceptionExists([MarshalAs(UnmanagedType.LPWStr)] string szExecutable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Firewall_AddAppException([MarshalAs(UnmanagedType.LPWStr)] string szName, 
                                                     [MarshalAs(UnmanagedType.LPWStr)] string  szExecutable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TT_Firewall_RemoveAppException([MarshalAs(UnmanagedType.LPWStr)] string szExecutable);
    }
}
