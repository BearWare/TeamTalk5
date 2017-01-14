/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using c_tt;
using System.Collections;
using System.Reflection;

namespace BearWare
{
    public abstract class TeamTalkSrvBase : IDisposable
    {
        // brief A server instance
        // see TTS_InitTeamTalk()
        static IntPtr m_ttsInst;
        public void Dispose()
        {
            deleteMe();
        }

        private void deleteMe()
        {
            if (m_ttsInst != IntPtr.Zero)
            {
                TTProDLL.TTS_CloseTeamTalk(m_ttsInst);
                m_ttsInst = IntPtr.Zero;
            }
        }
        ~TeamTalkSrvBase()
        {
            deleteMe();
        }
        protected TeamTalkSrvBase()
        {
            m_ttsInst = TTProDLL.TTS_InitTeamTalk();
        }
        protected TeamTalkSrvBase(Channel lpChannel)
            : this()
        {
            MakeChannel(lpChannel);
        }
        protected TeamTalkSrvBase(Channel lpChannel,ServerProperties lpServerProperties)
            : this()
        {
            UpdateServer(lpServerProperties);
            MakeChannel(lpChannel);
        }
        public void Close()
        {
            deleteMe();
        }
        public bool StartServer(string szBindIPAddr, int nTcpPort, int nUdpPort, bool bEncrypted)
        {
            return TTProDLL.TTS_StartServer(m_ttsInst, szBindIPAddr, nTcpPort, nUdpPort, bEncrypted);
        }
        public bool StopServer()
        {
            return TTProDLL.TTS_StopServer(m_ttsInst);
        }
        public ClientError MoveUser(int nUserID, BearWare.Channel lpChannel)
        {
            return TTProDLL.TTS_MoveUser(m_ttsInst, nUserID, ref lpChannel);
        }
        public ClientError SetChannelFilesRoot(string szFilesRoot, Int64 nMaxDiskUsage, Int64 nDefaultChannelQuota)
        {
            return TTProDLL.TTS_SetChannelFilesRoot(m_ttsInst, szFilesRoot, nMaxDiskUsage, nDefaultChannelQuota);
        }
        public ClientError AddFileToChannel(string szLocalFilePath, BearWare.RemoteFile lpRemoteFile)
        {
            return TTProDLL.TTS_AddFileToChannel(m_ttsInst, ref szLocalFilePath, ref lpRemoteFile);
        }
        public ClientError RemoveFileFromChannel(RemoteFile lpRemoteFile)
        {
            return TTProDLL.TTS_RemoveFileFromChannel(m_ttsInst, ref  lpRemoteFile);
        }
        public static bool SetEncryptionContext(string szCertificateFile, string szPrivateKeyFile)
        {
            return TTProDLL.TTS_SetEncryptionContext(m_ttsInst, ref szCertificateFile, ref  szPrivateKeyFile);
        }
        public static string GetVersion() { return Marshal.PtrToStringAuto(TTProDLL.TT_GetVersion()); }
        public ClientError RemoveChannel(int nChannelID)
        {
            return TTProDLL.TTS_RemoveChannel(m_ttsInst, nChannelID);
        }
        public ClientError UpdateChannel(Channel lpChannel)
        {
            return TTProDLL.TTS_UpdateChannel(m_ttsInst, ref lpChannel);
        }
        public virtual ClientError MakeChannel(BearWare.Channel lpChannel)
        {
            return TTProDLL.TTS_MakeChannel(m_ttsInst, ref lpChannel);
        }
        public ClientError UpdateServer([In] BearWare.ServerProperties lpServerInfo)
        {
            return TTProDLL.TTS_UpdateServer(m_ttsInst, ref lpServerInfo);
        }
        public bool RunEventLoop(int pnWaitMs)
        {
            return TTProDLL.TTS_RunEventLoop(m_ttsInst, pnWaitMs);
        }
        class CallBack : IDisposable
        {
            private GCHandle hCallBack;
            private IntPtr pCallFuncPointer;
            Delegate lpCallback;
            public CallBack(Delegate lpCallback)
            {
                this.lpCallback = lpCallback;
                hCallBack = GCHandle.Alloc(lpCallback);
                pCallFuncPointer = Marshal.GetFunctionPointerForDelegate(lpCallback);
                GC.KeepAlive(lpCallback);
                GC.Collect();
            }
            ~CallBack()
            {
                Dispose(true);
            }

            public IntPtr GetFuncPointer()
            {
                return pCallFuncPointer;
            }
            public void Dispose()
            {
                GC.SuppressFinalize(this);
                Dispose(true);
            }
            protected void Dispose(bool disposing)
            {
                if (disposing)
                {
                    // Do Something
                }
                // free ressource
                if (hCallBack.IsAllocated)
                    hCallBack.Free();
                pCallFuncPointer = IntPtr.Zero;
            }

            public bool Compare(Delegate x)
            {
                return Object.Equals(lpCallback, x);
            }
        }
        private delegate bool DLL(IntPtr lpTTSInstance, IntPtr lpCallback, int lpUserData, bool bEnable);
        private Dictionary<CallBack, DLL> Delegate2DLL = new Dictionary<CallBack, DLL>();
        private bool RegisterServerCallback(Delegate lpCallback, int lpUserData, bool bEnable)
        {
            CallBack callBack = null;
            bool b = false;
            string TTProDLL_Method = "TTS_Register" + lpCallback.GetType().ToString().Split('+')[1];
            MethodInfo method = typeof(TTProDLL).GetMethod(TTProDLL_Method);
            DLL dg = (DLL)Delegate.CreateDelegate(typeof(DLL), method);

            foreach (KeyValuePair<CallBack, DLL> cb in Delegate2DLL)
            {
                if (cb.Key.Compare(lpCallback))
                {
                    if (bEnable == true) return true;
                    callBack = cb.Key;
                    b = Delegate2DLL[callBack](m_ttsInst, callBack.GetFuncPointer(), 0, bEnable);
                    callBack.Dispose();
                    Delegate2DLL.Remove(callBack);
                    callBack = null;
                    return b;
                }
            }
            if (callBack == null && bEnable)
            {
                callBack = new CallBack(lpCallback);
                Delegate2DLL.Add(callBack, dg);
                b = Delegate2DLL[callBack](m_ttsInst, callBack.GetFuncPointer(), 0, bEnable);
            }
            return b;
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserLoggedInCallback(IntPtr lpTTSInstance, [In] IntPtr lpUserData, [In] ref User lpUser);
        private event UserLoggedInCallback onUserLoggedInCallback;
        internal event UserLoggedInCallback OnUserLoggedInCallBack
        {
            add
            {
                lock (objectLock)
                {
                    onUserLoggedInCallback += value;
                    RegisterServerCallback(value, 0, true);
                }
            }
            remove
            {
                lock (objectLock)
                {
                    onUserLoggedInCallback -= value;
                    RegisterServerCallback(value, 0, false);
                }
            }

        }

        /// <summary>
        /// brief Callback when a user is requesting to log on to the
        /// server.
        /// 
        /// This callback occurs in the context of TT_DoLogin().
        /// 
        /// Register using TTS_RegisterUserLoginCallback().
        /// </summary>
        /// <param name="lpTTSInstance">lpTTSInstance The server instance where the event is occurring.</param>
        /// <param name="lpUserData">lpUserData The user data supplied to register-callback function.</param>
        /// <param name="lpClientErrorMsg">lpClientErrorMsg Error message which should be sent back to user. Set @c nErrorNo to #CMDERR_SUCCESS if user is authorized.</param>
        /// <param name="lpUser">The user properties gathered so far.</param>
        /// <param name="lpUserAccount">lpUserAccount The user account information which shouldbe set for this user.</param>
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserLoginCallback(IntPtr lpTTSInstance, IntPtr lpUserData, [In, Out] ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, [In, Out]ref UserAccount lpUserAccount);
        object objectLock = new Object();
        private event UserLoginCallback onUserLoginCallback;
        internal event UserLoginCallback OnUserLoginCallback
        {
            add
            {
                lock (objectLock)
                {
                    onUserLoginCallback += value;
                    RegisterServerCallback(value, 0, true);
                }
            }
            remove
            {
                lock (objectLock)
                {
                    onUserLoginCallback -= value;
                    RegisterServerCallback(value, 0, false);
                }
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserCreateUserAccountCallback(IntPtr lpTTSInstance, IntPtr lpUserData, [In, Out] ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, [In, Out]ref UserAccount lpUserAccount);
        private event UserCreateUserAccountCallback onUserCreateUserAccountCallback;
        internal event UserCreateUserAccountCallback OnUserCreateUserAccountCallback
        {
            add
            {
                onUserCreateUserAccountCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onUserCreateUserAccountCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserDeleteUserAccountCallback(IntPtr lpTTSInstance, IntPtr lpUserData, [In, Out] ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, [In]  [MarshalAs(UnmanagedType.LPWStr)] string szUsername);
        private event UserDeleteUserAccountCallback onUserDeleteUserAccountCallback;
        internal event UserDeleteUserAccountCallback OnUserDeleteUserAccountCallback
        {
            add
            {
                onUserDeleteUserAccountCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onUserDeleteUserAccountCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserAddServerBanCallback(IntPtr lpTTSInstance, IntPtr lpUserData, [In, Out] ref ClientErrorMsg lpClientErrorMsg, [In,Out] ref User lpBanner, [In,Out] ref User lpBanee);
        private event UserAddServerBanCallback onUserAddServerBanCallback;
        internal event UserAddServerBanCallback OnUserAddServerBanCallback
        {
            add
            {
                onUserAddServerBanCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onUserAddServerBanCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserAddServerBanIPAddressCallback(IntPtr lpTTSInstance,
                                                   IntPtr lpUserData,
                                                   [In, Out] ref ClientErrorMsg lpClientErrorMsg,
                                                   [In] ref User lpBanner,
                                                   [In,Out]  [MarshalAs(UnmanagedType.LPWStr)] string szIPAddress);
        private event UserAddServerBanIPAddressCallback onUserAddServerBanIPAddressCallback;
        internal event UserAddServerBanIPAddressCallback OnUserAddServerBanIPAddressCallback
        {
            add
            {
                onUserAddServerBanIPAddressCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onUserAddServerBanIPAddressCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserDeleteServerBanCallback(IntPtr lpTTSInstance,
                                             IntPtr lpUserData,
                                             [In, Out] ref ClientErrorMsg lpClientErrorMsg,
                                             [In] ref User lpUser,
                                             [In]  [MarshalAs(UnmanagedType.LPWStr)] string szIPAddress);
        private event UserDeleteServerBanCallback onUserDeleteServerBanCallback;
        internal event UserDeleteServerBanCallback OnUserDeleteServerBanCallback
        {
            add
            {
                onUserDeleteServerBanCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onUserDeleteServerBanCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserConnectedCallback(IntPtr lpTTSInstance,
                                       IntPtr lpUserData, [In] ref User lpUser);
        private event UserConnectedCallback onUserConnectedCallback;
        internal event UserConnectedCallback OnUserConnectedCallback
        {
            add
            {
                onUserConnectedCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onUserConnectedCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserLoggedOutCallback(IntPtr lpTTSInstance,
                                       IntPtr lpUserData, [In] ref User lpUser);
        private event UserLoggedOutCallback onUserLoggedOutCallback;
        internal event UserLoggedOutCallback OnUserLoggedOutCallback
        {
            add
            {
                onUserLoggedOutCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onUserLoggedOutCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserDisconnectedCallback(IntPtr lpTTSInstance,
                                       IntPtr lpUserData, [In] ref User lpUser);
        private event UserDisconnectedCallback onUserDisconnectedCallback;
        internal event UserDisconnectedCallback OnUserDisconnectedCallback
        {
            add
            {
                onUserDisconnectedCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onUserDisconnectedCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserTimedoutCallback(IntPtr lpTTSInstance,
                                       IntPtr lpUserData, [In] ref User lpUser);
        private event UserTimedoutCallback onUserTimedoutCallback;
        internal event UserTimedoutCallback OnUserTimedoutCallback
        {
            add
            {
                onUserTimedoutCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onUserTimedoutCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserKickedCallback(IntPtr lpTTSInstance,
                                IntPtr lpUserData, [In] ref User lpKicker,
                                [In] ref User lpKickee, [In] IntPtr lpChannel);
        private event UserKickedCallback onUserKickedCallback;
        internal event UserKickedCallback OnUserKickedCallback
        {
            add
            {
                onUserKickedCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onUserKickedCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserBannedCallback(IntPtr lpTTSInstance,IntPtr lpUserData, [In,Out] ref User lpBanner,[In,Out] ref User lpBanee,  IntPtr  lpChannel);
        private event UserBannedCallback onUserBannedCallback;
        internal event UserBannedCallback OnUserBannedCallback
        {
            add
            {
                onUserBannedCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onUserBannedCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
         [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserUnbannedCallback(IntPtr lpTTSInstance,
                                      IntPtr lpUserData, [In] ref User lpUnbanner,
                                      [In] [MarshalAs(UnmanagedType.LPWStr)] string szIPAddress);
         private event UserUnbannedCallback onUserUnbannedCallback;
         internal event UserUnbannedCallback OnUserUnbannedCallback
         {
             add
             {
                 onUserUnbannedCallback += value;
                 RegisterServerCallback(value, 0, true);
             }
             remove
             {
                 onUserUnbannedCallback -= value;
                 RegisterServerCallback(value, 0, false);
             }
         }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserUpdatedCallback(IntPtr lpTTSInstance,
                                     IntPtr lpUserData, [In] ref User lpUser);
        private event UserUpdatedCallback onUserUpdatedCallback;
        internal event UserUpdatedCallback OnUserUpdatedCallback
        {
            add
            {
                onUserUpdatedCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onUserUpdatedCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserJoinedChannelCallback(IntPtr lpTTSInstance,
                                           IntPtr lpUserData, [In, Out] ref User lpUser,
                                           [In,Out] ref Channel lpChannel);
        private event UserJoinedChannelCallback onUserJoinedChannelCallback;
        internal event UserJoinedChannelCallback OnUserJoinedChannelCallback
        {
            add
            {
                onUserJoinedChannelCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onUserJoinedChannelCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserLeftChannelCallback(IntPtr lpTTSInstance,
                                           IntPtr lpUserData, [In,Out] ref User lpUser,
                                           [In,Out] ref Channel lpChannel);
        private event UserLeftChannelCallback onUserLeftChannelCallback;
        internal event UserLeftChannelCallback OnUserLeftChannelCallback
        {
            add
            {
                onUserLeftChannelCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onUserLeftChannelCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
         [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserMovedCallback(IntPtr lpTTSInstance,
                                   IntPtr lpUserData, [In,Out] ref User lpMover,
                                   [In,Out] ref User lpMovee);
         private event UserMovedCallback onUserMovedCallback;
         internal event UserMovedCallback OnUserMovedCallback
         {
             add
             {
                 onUserMovedCallback += value;
                 RegisterServerCallback(value, 0, true);
             }
             remove
             {
                 onUserMovedCallback -= value;
                 RegisterServerCallback(value, 0, false);
             }
         }
         [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserTextMessageCallback(IntPtr lpTTSInstance,
                                         IntPtr lpUserData, [In] ref User lpUser,
                                        [In] ref TextMessage lpTextMessage);
         private event UserTextMessageCallback onUserTextMessageCallback;
         internal event UserTextMessageCallback OnUserTextMessageCallback
         {
             add
             {
                 onUserTextMessageCallback  += value;
                 RegisterServerCallback(value, 0, true);
             }
             remove
             {
                 onUserTextMessageCallback -= value;
                 RegisterServerCallback(value, 0, false);
             }
         }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        public delegate void ChannelCreatedCallback(IntPtr lpTTSInstance,
                                        IntPtr lpUserData,[In] ref Channel lpChannel,
                                        IntPtr lpUser);
        private event ChannelCreatedCallback onChannelCreatedCallback;
        public event ChannelCreatedCallback OnChannelCreatedCallback
        {
            add
            {
                onChannelCreatedCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onChannelCreatedCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void ChannelUpdatedCallback(IntPtr lpTTSInstance,
                                        IntPtr lpUserData, [In] ref Channel lpChannel,
                                        IntPtr lpUser);
        private event ChannelUpdatedCallback onChannelUpdatedCallback;
        internal event ChannelUpdatedCallback OnChannelUpdatedCallback
        {
            add
            {
                onChannelUpdatedCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onChannelUpdatedCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void ChannelRemovedCallback(IntPtr lpTTSInstance,
                                        IntPtr lpUserData, [In] ref Channel lpChannel,
                                        IntPtr  lpUser);
        private event ChannelRemovedCallback onChannelRemovedCallback;
        internal event ChannelRemovedCallback OnChannelRemovedCallback
        {
            add
            {
                onChannelRemovedCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onChannelRemovedCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void FileUploadedCallback(IntPtr lpTTSInstance,
                                      IntPtr lpUserData, 
                                      [In] ref RemoteFile lpRemoteFile,
                                      [In] ref User lpUser);
        private event FileUploadedCallback onFileUploadedCallback;
        internal event FileUploadedCallback OnFileUploadedCallback
        {
            add
            {
                onFileUploadedCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onFileUploadedCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void FileDownloadedCallback(IntPtr lpTTSInstance,
                                      IntPtr lpUserData,
                                      [In] ref RemoteFile lpRemoteFile,
                                      [In] ref User lpUser);
        private event FileDownloadedCallback onFileDownloadedCallback;
        internal event FileDownloadedCallback OnFileDownloadedCallback
        {
            add
            {
                onFileDownloadedCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onFileDownloadedCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void FileDeletedCallback(IntPtr lpTTSInstance,
                                      IntPtr lpUserData,
                                      [In] ref RemoteFile lpRemoteFile,
                                      [In] ref User lpUser);
        private event FileDeletedCallback onFileDeletedCallback;
        internal event FileDeletedCallback OnFileDeletedCallback
        {
            add
            {
                onFileDeletedCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onFileDeletedCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void ServerUpdatedCallback(IntPtr lpTTSInstance,
                                       IntPtr lpUserData, 
                                       [In] ref ServerProperties lpServerProperties,
                                       [In] ref User lpUser);
        private event ServerUpdatedCallback onServerUpdatedCallback;
        internal event ServerUpdatedCallback OnServerUpdatedCallback
        {
            add
            {
                onServerUpdatedCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onServerUpdatedCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void SaveServerConfigCallback(IntPtr lpTTSInstance,
                                          IntPtr lpUserData, 
                                          [In] ref User lpUser);
        private event SaveServerConfigCallback onSaveServerConfigCallback;
        internal event SaveServerConfigCallback OnSaveServerConfigCallback
        {
            add
            {
                onSaveServerConfigCallback += value;
                RegisterServerCallback(value, 0, true);
            }
            remove
            {
                onSaveServerConfigCallback -= value;
                RegisterServerCallback(value, 0, false);
            }
        }

    }
   
}
