/*
 * Copyright (c) 2005-2016, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
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

namespace BearWare
{
    public class TeamTalk5Srv : TeamTalkSrv, IDisposable
    {

        public delegate void UserLoggedIn(ref User lpUser);
        public event UserLoggedIn OnUserLoggedIn;

        /// <summary>
        /// brief   when a user is requesting to log On to the server.
        /// 
        /// 
        /// This   occurs in the cOntext of TT_DoLogin().
        /// 
        /// Register using TTS_RegisterUserLogin ().
        /// </summary>
        /// <param name="lpClientErrorMsg">lpClientErrorMsg Error message which should be sent back to user. Set @c nErrorNo to #CMDERR_SUCCESS if user is authorized.</param>
        /// <param name="lpUser">The user properties gathered so far.</param>
        /// <param name="lpUserAccount">lpUserAccount The user account informatiOn which shouldbe set for this user.</param>
        public delegate void UserLogin( ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, ref UserAccount lpUserAccount);
        public event UserLogin OnUserLogin;

        public delegate void UserCreateUserAccount(ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, ref UserAccount lpUserAccount);
        public event UserCreateUserAccount OnUserCreateUserAccount;

        public delegate void UserDeleteUserAccount(ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, string szUsername);
        public event UserDeleteUserAccount OnUserDeleteUserAccount;

        public delegate void UserAddServerBan(ref ClientErrorMsg lpClientErrorMsg,  ref User lpBanner, ref User lpBanee);
        public event UserAddServerBan OnUserAddServerBan;

        public delegate void UserAddServerBanIPAddress(ref ClientErrorMsg lpClientErrorMsg,ref User lpBanner, string szIPAddress);
        public event UserAddServerBanIPAddress OnUserAddServerBanIPAddress;

        public delegate void UserDeleteServerBan(ref ClientErrorMsg lpClientErrorMsg,ref User lpUser, string szIPAddress);
        public event UserDeleteServerBan OnUserDeleteServerBan;

        public delegate void UserConnected(ref User lpUser);
        public event UserConnected OnUserConnected;

        public delegate void UserLoggedOut(ref User lpUser);
        public event UserLoggedOut OnUserLoggedOut;

        public delegate void UserDisconnected(ref User lpUser);
        public event UserDisconnected OnUserDisconnected;

        public delegate void UserTimedout(ref User lpUser);
        public event UserTimedout OnUserTimedout;

        public delegate void UserKicked(ref User lpKicker,ref User lpKickee, ref Channel lpChannel);
        public event UserKicked OnUserKicked;

        public delegate void UserBanned(ref User lpBanner,  ref User lpBanee, IntPtr lpChannel);
        public event UserBanned OnUserBanned;

        public delegate void UserUnbanned(ref User lpUnbanner, string szIPAddress);
        public event UserUnbanned OnUserUnbanned;

        public delegate void UserUpdated(ref User lpUser);
        public event UserUpdated OnUserUpdated;

        public delegate void UserJoinedChannel(ref User lpUser, ref Channel lpChannel);
        public event UserJoinedChannel OnUserJoinedChannel;

        public delegate void UserLeftChannel(ref User lpUser, ref Channel lpChannel);
        public event UserLeftChannel OnUserLeftChannel;

        public delegate void UserMoved(ref User lpMover, ref User lpMovee);
        public event UserMoved OnUserMoved;

        public delegate void UserTextMessage(ref User lpUser,ref TextMessage lpTextMessage);
        public event UserTextMessage OnUserTextMessage;

        public delegate void ChannelCreated(ref Channel lpChannel,ref User lpUser);
        public event ChannelCreated OnChannelCreated;

        public delegate void ChannelUpdated(ref Channel lpChannel,ref User lpUser);
        public event ChannelUpdated OnChannelUpdated;

        public delegate void ChannelRemoved(ref Channel lpChannel,ref User lpUser);
        public event ChannelRemoved OnChannelRemoved;

        public delegate void FileUploaded(ref RemoteFile lpRemoteFile,ref User lpUser);
        public event FileUploaded OnFileUploaded;

        public delegate void FileDownloaded(ref RemoteFile lpRemoteFile,ref User lpUser);
        public event FileDownloaded OnFileDownloaded;

        public delegate void FileDeleted(ref RemoteFile lpRemoteFile,ref User lpUser);
        public event FileDeleted OnFileDeleted;

        public delegate void ServerUpdated(ref ServerProperties lpServerProperties,ref User lpUser);
        public event ServerUpdated OnServerUpdated;

        public delegate void SaveServerConfig(ref User lpUser);
        public event SaveServerConfig OnSaveServerConfig;

        public TeamTalk5Srv()
        {
            Init();
        }

        public TeamTalk5Srv(Channel lpChannel)
            : base(lpChannel)
        {
            Init(lpChannel);
        }

        public TeamTalk5Srv(Channel lpChannel, ServerProperties lpServerProperties)
            : base(lpChannel, lpServerProperties)
        {
            Init(lpChannel);
        }

        public void Init(Nullable<Channel> lpChannel = null)
        {
            base.OnChannelCreatedCallback += new ChannelCreatedCallback(TeamTalk5Srv_OnChannelCreatedCallback);
            base.OnChannelRemovedCallback += new ChannelRemovedCallback(TeamTalk5Srv_OnChannelRemovedCallback);
            base.OnChannelUpdatedCallback += new ChannelUpdatedCallback(TeamTalk5Srv_OnChannelUpdatedCallback);
            base.OnUserLoginCallback += new UserLoginCallback(TeamTalk5Srv_OnUserLoginCallback);
            base.OnUserLoggedOutCallback += new UserLoggedOutCallback(TeamTalk5Srv_OnUserLoggedOutCallback);
            base.OnUserLoggedInCallBack += new UserLoggedInCallback(TeamTalk5Srv_OnUserLoggedInCallBack);
            base.OnUserTextMessageCallback += new UserTextMessageCallback(TeamTalk5Srv_OnUserTextMessageCallback);
            base.OnUserAddServerBanCallback += new UserAddServerBanCallback(TeamTalk5Srv_OnUserAddServerBanCallback);
            base.OnUserAddServerBanIPAddressCallback += new UserAddServerBanIPAddressCallback(TeamTalk5Srv_OnUserAddServerBanIPAddressCallback);
            base.OnUserDeleteServerBanCallback += new UserDeleteServerBanCallback(TeamTalk5Srv_OnUserDeleteServerBanCallback);
            base.OnUserBannedCallback += new UserBannedCallback(TeamTalk5Srv_OnUserBannedCallback);
            base.OnUserConnectedCallback += new UserConnectedCallback(TeamTalk5Srv_OnUserConnectedCallback);
            base.OnUserDisconnectedCallback += new UserDisconnectedCallback(TeamTalk5Srv_OnUserDisconnectedCallback);
            base.OnUserTimedoutCallback += new UserTimedoutCallback(TeamTalk5Srv_OnUserTimedoutCallback);
            base.OnUserLeftChannelCallback += new UserLeftChannelCallback(TeamTalk5Srv_OnUserLeftChannelCallback);
            base.OnUserJoinedChannelCallback += new UserJoinedChannelCallback(TeamTalk5Srv_OnUserJoinedChannelCallback);
            base.OnUserKickedCallback += new UserKickedCallback(TeamTalk5Srv_OnUserKickedCallback);
            base.OnUserMovedCallback += new UserMovedCallback(TeamTalk5Srv_OnUserMovedCallback);
            base.OnUserUpdatedCallback += new UserUpdatedCallback(TeamTalk5Srv_OnUserUpdatedCallback);
            base.OnSaveServerConfigCallback += new SaveServerConfigCallback(TeamTalk5Srv_OnSaveServerConfigCallback);
            base.OnServerUpdatedCallback += new ServerUpdatedCallback(TeamTalk5Srv_OnServerUpdatedCallback);
            base.OnUserCreateUserAccountCallback += new UserCreateUserAccountCallback(TeamTalk5Srv_OnUserCreateUserAccountCallback);
            base.OnUserDeleteUserAccountCallback += new UserDeleteUserAccountCallback(TeamTalk5Srv_OnUserDeleteUserAccountCallback);
            base.OnFileDeletedCallback += new FileDeletedCallback(TeamTalk5Srv_OnFileDeletedCallback);
            base.OnFileDownloadedCallback += new FileDownloadedCallback(TeamTalk5Srv_OnFileDownloadedCallback);
            base.OnFileUploadedCallback += new FileUploadedCallback(TeamTalk5Srv_OnFileUploadedCallback);

            if (lpChannel != null)
            {
                MakeChannel((Channel)lpChannel);
            }
        }

        void TeamTalk5Srv_OnFileUploadedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref RemoteFile lpRemoteFile, ref User lpUser)
        {
           if(OnFileUploaded != null)
           {
               OnFileUploaded(ref lpRemoteFile, ref lpUser);
           }
        }

        void TeamTalk5Srv_OnFileDownloadedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref RemoteFile lpRemoteFile, ref User lpUser)
        {
            
           if(OnFileDownloaded != null)
           {
               OnFileDownloaded(ref lpRemoteFile, ref lpUser);
           }
        }
        void TeamTalk5Srv_OnFileDeletedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref RemoteFile lpRemoteFile, ref User lpUser)
        {
            if(OnFileDeleted != null)
            {
                OnFileDeleted(ref lpRemoteFile, ref lpUser);
            }
        }

        void TeamTalk5Srv_OnUserDeleteUserAccountCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, string szUsername)
        {
            
            if(OnUserDeleteUserAccount != null)
            {
                OnUserDeleteUserAccount(ref lpClientErrorMsg, ref lpUser, szUsername);
            }
        }

        void TeamTalk5Srv_OnUserCreateUserAccountCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, ref UserAccount lpUserAccount)
        {
            if(OnUserCreateUserAccount != null)
            {
                OnUserCreateUserAccount(ref lpClientErrorMsg, ref lpUser, ref lpUserAccount);
            }
        }
        
        void TeamTalk5Srv_OnServerUpdatedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref ServerProperties lpServerProperties, ref User lpUser)
        {
            if(OnServerUpdated != null)
            {
                OnServerUpdated(ref lpServerProperties, ref lpUser);
            }
        }
        
        void TeamTalk5Srv_OnSaveServerConfigCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser)
        {
            if(OnSaveServerConfig != null)
            {
                OnSaveServerConfig(ref lpUser);
            }
        }
        
        void TeamTalk5Srv_OnUserUpdatedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser)
        {
            if(OnUserUpdated != null)
            {
                OnUserUpdated(ref lpUser);
            }
        }
        
        void TeamTalk5Srv_OnUserMovedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpMover, ref User lpMovee)
        {
            if(OnUserMoved != null)
            {
                OnUserMoved(ref lpMover, ref lpMovee);
            }
        }
        
        void TeamTalk5Srv_OnUserKickedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpKicker, ref User lpKickee, IntPtr lpChannel)
        {
            Channel chan = new Channel();
            if(lpChannel != IntPtr.Zero)
            {
                chan = (Channel)Marshal.PtrToStructure(lpChannel, typeof(Channel));
            }
            if(OnUserKicked != null)
            {
                OnUserKicked(ref lpKicker, ref lpKickee, ref chan);
            }
        }
        
        void TeamTalk5Srv_OnUserJoinedChannelCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser, ref Channel lpChannel)
        {
            if(OnUserJoinedChannel !=null)
            {
                OnUserJoinedChannel(ref lpUser, ref lpChannel);
            }
        }
        
        void TeamTalk5Srv_OnUserLeftChannelCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser, ref Channel lpChannel)
        {
            if(OnUserLeftChannel != null)
            {
                OnUserLeftChannel(ref lpUser, ref lpChannel);
            }
        }
        
        void TeamTalk5Srv_OnUserTimedoutCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser)
        {
            if(OnUserTimedout != null)
            {
                OnUserTimedout(ref lpUser);
            }
        }
        
        void TeamTalk5Srv_OnUserDisconnectedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser)
        {
            if(OnUserDisconnected != null)
            {
                OnUserDisconnected(ref lpUser);
            }
        }
        
        void TeamTalk5Srv_OnUserConnectedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser)
        {
            if(OnUserConnected != null)
            {
                OnUserConnected(ref lpUser);
            }
        }
        
        void TeamTalk5Srv_OnUserBannedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpBanner, ref User lpBanee, IntPtr lpChannel)
        {
            if(OnUserBanned != null)
            {
                OnUserBanned(ref lpBanner, ref lpBanee, lpChannel);
            }
        }
        
        void TeamTalk5Srv_OnUserDeleteServerBanCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, string szIPAddress)
        {
            if(OnUserDeleteServerBan != null)
            {
                OnUserDeleteServerBan(ref lpClientErrorMsg, ref lpUser, szIPAddress);
            }
        }
        
        void TeamTalk5Srv_OnUserAddServerBanIPAddressCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref ClientErrorMsg lpClientErrorMsg, ref User lpBanner, string szIPAddress)
        {
            if(OnUserAddServerBanIPAddress != null)
            {
                OnUserAddServerBanIPAddress(ref lpClientErrorMsg, ref lpBanner, szIPAddress);
            }
        }
        
        void TeamTalk5Srv_OnUserAddServerBanCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref ClientErrorMsg lpClientErrorMsg, ref User lpBanner, ref User lpBanee)
        {
            if(OnUserAddServerBan != null)
            {
                OnUserAddServerBan(ref lpClientErrorMsg, ref lpBanner, ref lpBanee);
            }
        }
        
        void TeamTalk5Srv_OnUserTextMessageCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser, ref TextMessage lpTextMessage)
        {
            if(OnUserTextMessage !=null)
            {
                OnUserTextMessage(ref lpUser, ref lpTextMessage);
            }
        }
        
        void TeamTalk5Srv_OnUserLoggedOutCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser)
        {
            if (OnUserLoggedOut != null)
            {
                OnUserLoggedOut(ref lpUser);
            }
        }
        
        void TeamTalk5Srv_OnUserLoggedInCallBack(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser)
        {
            if (OnUserLoggedIn != null)
            {
                OnUserLoggedIn(ref lpUser);
            }
        }
        
        void TeamTalk5Srv_OnUserLoginCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, ref UserAccount lpUserAccount)
        {
            if (OnUserLogin != null)
            {
                OnUserLogin(ref lpClientErrorMsg, ref lpUser, ref lpUserAccount);
            }
        }
        
        void TeamTalk5Srv_OnChannelUpdatedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref Channel lpChannel, IntPtr lpUser)
        {
            User user = new User();
            if (lpUser != IntPtr.Zero)
            {
                user = (User)Marshal.PtrToStructure(lpUser, typeof(User));
            }
            if (OnChannelUpdated != null)
            {
                OnChannelUpdated(ref lpChannel, ref user);
            }
        }
        
        void TeamTalk5Srv_OnChannelRemovedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref Channel lpChannel, IntPtr lpUser)
        {
            User user = new User();
            if (lpUser != IntPtr.Zero)
            {
                user = (User)Marshal.PtrToStructure(lpUser, typeof(User));
            }
            if (OnChannelRemoved != null)
            {
                OnChannelRemoved(ref lpChannel, ref user);
            }
        }
        
        void TeamTalk5Srv_OnChannelCreatedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref Channel lpChannel, IntPtr lpUser)
        {
            User user = new User();
            if (lpUser != IntPtr.Zero)
            {
                user = (User)Marshal.PtrToStructure(lpUser, typeof(User));
            }
            if (OnChannelCreated != null)
            {
                OnChannelCreated(ref lpChannel, ref user);
            }
        }

        public override bool StartServer(string szBindIPAddr, int nTcpPort, int nUdpPort, bool bEncrypted)
        {
            return base.StartServer(szBindIPAddr, nTcpPort, nUdpPort, bEncrypted);
        }
    }
}
