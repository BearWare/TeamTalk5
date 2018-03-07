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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using BearWare;

namespace c_tt
{
    class TTProDLL : c_tt.TTDLL
    {

        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_StartServer(IntPtr lpTTSInstance, [MarshalAs(UnmanagedType.LPWStr)] string szBindIPAddr,
                                                  [In] int nTcpPort, [In] int nUdpPort, bool bEncrypted);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_StartServerSysID(IntPtr lpTTSInstance, [MarshalAs(UnmanagedType.LPWStr)] string szBindIPAddr,
                                                       [In] int nTcpPort, [In] int nUdpPort, bool bEncrypted,
                                                       [MarshalAs(UnmanagedType.LPWStr)] string szSystemID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr TTS_InitTeamTalk();
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern BearWare.ClientError TTS_UpdateServer(IntPtr lpTTSInstance, [In] ref BearWare.ServerProperties lpServerInfo);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern BearWare.ClientError TTS_MakeChannel(IntPtr lpTTSInstance, [In, Out] ref BearWare.Channel lpChannel);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RunEventLoop(IntPtr lpTTSInstance, int pnWaitMs);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserLoginCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_CloseTeamTalk(IntPtr lpTTSInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern BearWare.ClientError TTS_MoveUser(IntPtr lpTTSInstance, int nUserID, [In] ref BearWare.Channel lpChannel);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern BearWare.ClientError TTS_SendTextMessage(IntPtr lpTTSInstance, [In] ref BearWare.TextMessage lpTextMessage);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern BearWare.ClientError TTS_RemoveChannel(IntPtr lpTTSInstance, [In]  int nChannelID);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern BearWare.ClientError TTS_RemoveFileFromChannel(IntPtr lpTTSInstance, [In] ref BearWare.RemoteFile lpRemoteFile);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_SetEncryptionContext(IntPtr lpTTSInstance, [In] ref string szCertificateFile, [In] ref string szPrivateKeyFile);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_StopServer(IntPtr lpTTSInstance);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern BearWare.ClientError TTS_AddFileToChannel(IntPtr lpTTSInstance, [In] ref string szLocalFilePath, [In] ref BearWare.RemoteFile lpRemoteFile);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern BearWare.ClientError TTS_SetChannelFilesRoot(IntPtr lpTTSInstance, [In, Out] string szFilesRoot, [In] Int64 nMaxDiskUsage, [In] Int64 nDefaultChannelQuota);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern BearWare.ClientError TTS_UpdateChannel(IntPtr lpTTSInstance, [In] ref BearWare.Channel lpChannel);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserLoggedInCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserLoggedOutCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserDisconnectedCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserTimedoutCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserKickedCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserBannedCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserUnbannedCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserUpdatedCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserJoinedChannelCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserLeftChannelCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserMovedCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserTextMessageCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterChannelCreatedCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterChannelUpdatedCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterChannelRemovedCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterFileUploadedCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterFileDownloadedCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterFileDeletedCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterServerUpdatedCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterSaveServerConfigCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserCreateUserAccountCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserDeleteUserAccountCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserAddServerBanCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserAddServerBanIPAddressCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserDeleteServerBanCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserConnectedCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserChangeNicknameCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        [DllImport(dllname, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool TTS_RegisterUserChangeStatusCallback(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
       
    }
}
