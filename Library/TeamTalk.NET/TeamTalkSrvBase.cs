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
using c_tt;
using System.Collections;
using System.Reflection;

namespace BearWare
{
    /** @ingroup serverapi
     * @brief Base class for #BearWare.TeamTalk5Srv. */
    public abstract class TeamTalkSrvBase : IDisposable
    {
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

        /** @addtogroup serverapi
         * @{ */

        /**
         * @brief Set certificate and private key for encrypted server.
         *
         * @verbatim
         * NOTE: AT THE MOMENT CALL SetEncryptionContext() BEFORE
         * CREATING THE SERVER INSTANCE. IN OTHER WORDS ONLY ONE
         * ENCRYPTION CONTEXT IS SUPPORTED AT THE MOMENT.
         * @endverbatim
         *
         * The encrypted server's certificate and private key must be set
         * prior to starting the server using StartServer().
         *
         * Look in @ref serversetup on how to generate the certificate and
         * private key file using OpenSSL.
         *
         * @param szCertificateFile Path to server's certificate file. 
         * @param szPrivateKeyFile Path to server's private key file. */
        public static bool SetEncryptionContext(string szCertificateFile, string szPrivateKeyFile)
        {
            return TTProDLL.TTS_SetEncryptionContext(m_ttsInst, ref szCertificateFile, ref  szPrivateKeyFile);
        }

        /**
         * @brief Create new TeamTalk server instance.
         * 
         * Once server instance is created call UpdateServer() to set
         * the server's properties followed by MakeChannel() to create
         * the root channel.
         *
         * @verbatim
         * NOTE: AT THE MOMENT CALL SetEncryptionContext() BEFORE
         * CREATING THE SERVER INSTANCE, TeamTalkSrvBase(). 
         * IN OTHER WORDS ONLY ONE ENCRYPTION CONTEXT IS
         * SUPPORTED AT THE MOMENT.
         * @endverbatim
         *
         * @see StartServer() */
        protected TeamTalkSrvBase()
        {
            m_ttsInst = TTProDLL.TTS_InitTeamTalk();
        }
        protected TeamTalkSrvBase(Channel lpChannel)
            : this()
        {
            MakeChannel(lpChannel);
        }
        protected TeamTalkSrvBase(Channel lpChannel, ServerProperties lpServerProperties)
            : this()
        {
            UpdateServer(lpServerProperties);
            MakeChannel(lpChannel);
        }
        /**
         * @brief Close TeamTalk server instance.
         **/
        public void Close()
        {
            deleteMe();
        }
        /**
         * @brief Run the server's event loop.
         * 
         * @param pnWaitMs The amount of time to wait for the event. If NULL or -1
         * the function will block forever or until the next event occurs.
         * @return Returns TRUE if an event has occured otherwise FALSE. */
        public bool RunEventLoop(int pnWaitMs)
        {
            return TTProDLL.TTS_RunEventLoop(m_ttsInst, pnWaitMs);
        }
        /**
         * @brief The root folder of where users should upload files to.
         *
         * The root file folder cannot be changed after the server has
         * been started.
         *
         * Ensure to set #UserRight.USERRIGHT_UPLOAD_FILES and #UserRight.USERRIGHT_DOWNLOAD_FILES
         * in user's #BearWare.UserAccount.
         *
         * @param szFilesRoot Directory where to store uploaded files.
         * @param nMaxDiskUsage The maximum number of bytes which can be used for
         * file storage.
         * @param nDefaultChannelQuota The number of bytes available to temporary
         * channels (not #ChannelType.CHANNEL_PERMANENT). This will be the value in #BearWare.Channel
         * @c nDiskQuota.
         *
         * @return Error code from #ClientError. */
        public ClientError SetChannelFilesRoot(string szFilesRoot, Int64 nMaxDiskUsage, Int64 nDefaultChannelQuota)
        {
            return TTProDLL.TTS_SetChannelFilesRoot(m_ttsInst, szFilesRoot, nMaxDiskUsage, nDefaultChannelQuota);
        }
        /**
         * @brief Set server properties.
         *
         * Set server's properties, like e.g. maximum number of users,
         * server name, etc.
         *
         * Server properties must be set prior to starting a server.
         *
         * @param lpServerProperties The server's properties which will be
         * see by all users who log on to the server.
         * @return Returns a #ClientError.
         *
         * @see StartServer() */
        public ClientError UpdateServer([In] BearWare.ServerProperties lpServerProperties)
        {
            return TTProDLL.TTS_UpdateServer(m_ttsInst, ref lpServerProperties);
        }
        /**
         * @brief Make new channel.
         *
         * Create a new channel on the server. Before starting a server
         * using StartServer() the server MUST have a root
         * channel. I.e. a #BearWare.Channel where @c nParentID is 0.
         *
         * @param lpChannel The new channel to create.
         * @return Returns a #ClientError.
         *
         * @see UpdateChannel()
         * @see RemoveChannel() */
        public virtual ClientError MakeChannel(BearWare.Channel lpChannel)
        {
            return TTProDLL.TTS_MakeChannel(m_ttsInst, ref lpChannel);
        }
        /**
         * @brief Update an existing channel.
         *
         * @param lpChannel The updated channel properties. @c nChannelID
         * and @c nParentID must remain the same.
         * @return Returns a #ClientError.
         *
         * @see MakeChannel()
         * @see RemoveChannel() */
        public ClientError UpdateChannel(Channel lpChannel)
        {
            return TTProDLL.TTS_UpdateChannel(m_ttsInst, ref lpChannel);
        }
        /**
         * @brief Remove a channel.
         *
         * @param nChannelID The ID of the channel to remove.
         * @return Returns a #ClientError.
         *
         * @see MakeChannel()
         * @see UpdateChannel() */
        public ClientError RemoveChannel(int nChannelID)
        {
            return TTProDLL.TTS_RemoveChannel(m_ttsInst, nChannelID);
        }
        /**
         * @brief Add a file to an existing channel.
         *
         * Ensure to have set up file storage first using SetChannelFilesRoot().
         * Also ensure #BearWare.Channel's @c nDiskQuota is specified.
         *
         * @param szLocalFilePath Path to file.
         * @param lpRemoteFile Properties of file to add.
         * @return Command error code from #ClientError.
         *
         * @see SetChannelFilesRoot().
         * @see MakeChannel() */
        public ClientError AddFileToChannel(string szLocalFilePath, BearWare.RemoteFile lpRemoteFile)
        {
            return TTProDLL.TTS_AddFileToChannel(m_ttsInst, ref szLocalFilePath, ref lpRemoteFile);
        }
        /**
         * @brief Remove a file from a channel.
         *
         * Ensure to have set up file storage first using SetChannelFilesRoot().
         * Also ensure #BearWare.Channel's @c nDiskQuota is specified.
         *
         * @param lpRemoteFile Properties of file to remove. Channel ID and 
         * file name is enough.
         * @return Command error code from #ClientError.
         *
         * @see SetChannelFilesRoot().
         * @see MakeChannel() */
        public ClientError RemoveFileFromChannel(RemoteFile lpRemoteFile)
        {
            return TTProDLL.TTS_RemoveFileFromChannel(m_ttsInst, ref  lpRemoteFile);
        }
        /**
         * @brief Move a user from one channel to another.
         * 
         * @param nUserID The ID of the user to move.
         * @param lpChannel The channel the user should move to. If the
         * channel already exists then simply set @c nChannelID. To make
         * a user leave a channel set @c nChannelID to 0.
         * @return Returns a #ClientError. */
        public ClientError MoveUser(int nUserID, BearWare.Channel lpChannel)
        {
            return TTProDLL.TTS_MoveUser(m_ttsInst, nUserID, ref lpChannel);
        }
        /**
         * @brief Send text message from server to clients.
         *
         * 
         * @param lpTextMessage Text message to send. The message type determines
         * how the message will be sent.
         * @return Returns a #ClientError.
         * @see TeamTalkBase.DoTextMessage() */
        public ClientError SendTextMessage(BearWare.TextMessage lpTextMessage)
        {
            return TTProDLL.TTS_SendTextMessage(m_ttsInst, ref lpTextMessage);
        }
        /** 
         * @brief Start server on specified IP-address and ports.
         *
         * Before starting a server the root channel must be created using
         * MakeChannel().
         *
         * @param szBindIPAddr The IP-address to bind to.
         * @param nTcpPort The TCP port to bind to.
         * @param nUdpPort The UDP port to bind to.
         * @param bEncrypted If encryption is enabled then encryption context
         * must be set prior to this call using SetEncryptionContext().
         *
         * @see SetEncryptionContext()
         * @see MakeChannel() */
        public bool StartServer(string szBindIPAddr, int nTcpPort, int nUdpPort, bool bEncrypted)
        {
            return TTProDLL.TTS_StartServer(m_ttsInst, szBindIPAddr, nTcpPort, nUdpPort, bEncrypted);
        }
        /**
         * @brief Same as StartServer() but with the option of
         * specifying a system-ID.
         *
         * Requires TeamTalk version 5.1.3.4506.
         *
         * @param szBindIPAddr The IP-address to bind to.
         * @param nTcpPort The TCP port to bind to.
         * @param nUdpPort The UDP port to bind to.
         * @param bEncrypted If encryption is enabled then encryption context
         * must be set prior to this call using SetEncryptionContext().
         * @param szSystemID The identification of the conferencing system.
         * The default value is "teamtalk". See TeamTalkBase.ConnectSysID(). */
        public bool StartServerSysID(string szBindIPAddr, int nTcpPort, int nUdpPort, bool bEncrypted,
                                     string szSystemID)
        {
            return TTProDLL.TTS_StartServerSysID(m_ttsInst, szBindIPAddr, nTcpPort, nUdpPort, bEncrypted, szSystemID);
        }
        /**
         * @brief Stop server and drop all users.
         *
         * @see StartServer() */
        public bool StopServer()
        {
            return TTProDLL.TTS_StopServer(m_ttsInst);
        }

        public static string GetVersion() { return Marshal.PtrToStringAuto(TTProDLL.TT_GetVersion()); }
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

        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserChangeNicknameCallback(IntPtr lpTTSInstance, IntPtr lpUserData, [In, Out] ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, [In] [MarshalAs(UnmanagedType.LPWStr)] string szNewNickname);
        private event UserChangeNicknameCallback onUserChangeNicknameCallback;
        internal event UserChangeNicknameCallback OnUserChangeNicknameCallback
        {
            add 
            {
                lock(objectLock)
                {
                    onUserChangeNicknameCallback += value;
                    RegisterServerCallback(value, 0, true);
                }
            }
            remove
            {
                lock(objectLock)
                {
                    onUserChangeNicknameCallback -= value;
                    RegisterServerCallback(value, 0, false);
                }
            }
        }
        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
        internal delegate void UserChangeStatusCallback(IntPtr lpTTSInstance, IntPtr lpUserData, [In,Out] ref ClientErrorMsg lpClientErrorMsg,  [In] ref User lpUser, [In] ref int nNewStatusMode, [In] [MarshalAs(UnmanagedType.LPWStr)] string szNewStatusMsg);
        private event UserChangeStatusCallback onUserChangeStatusCallback;
        internal event UserChangeStatusCallback OnUserChangeStatusCallback
        {
            add
            {
                lock(objectLock)
                {
                    onUserChangeStatusCallback += value;
                    RegisterServerCallback(value, 0, true);
                }
            }
            remove
            {
                lock (objectLock)
                {
                    onUserChangeStatusCallback -= value;
                    RegisterServerCallback(value, 0, false);
                }
            }
        }

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
                                IntPtr lpUserData, IntPtr lpKicker,
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
        internal delegate void ChannelCreatedCallback(IntPtr lpTTSInstance,
                                        IntPtr lpUserData,[In] ref Channel lpChannel,
                                        IntPtr lpUser);
        private event ChannelCreatedCallback onChannelCreatedCallback;
        internal event ChannelCreatedCallback OnChannelCreatedCallback
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
                                          IntPtr lpUser);
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
