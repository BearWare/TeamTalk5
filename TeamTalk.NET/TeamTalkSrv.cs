/*
 * Copyright (c) 2005-2017, BearWare.dk
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

namespace BearWare
{
    /** @ingroup serverapi
     *
     * @brief Instantiate this class to start a TeamTalk server.
     *
     * These are the steps to start a TeamTalk server:
     * - First call TeamTalk5Srv.SetEncryptionContext() to setup encryption.
     * - Second call TeamTalk5Srv.UpdateServer() to setup server properties.
     * - Third call TeamTalk5Srv.MakeChannel() to create the root channel.
     *
     * @see TeamTalk5Srv.StartServer() */
    public class TeamTalk5Srv : TeamTalkSrvBase, IDisposable
    {

        /** @addtogroup servercallbacks
         * @{ */

        /**
         * @brief Callback when a user is requesting to log on to the
         * server.
         *
         * This callback occurs in the context of TeamTalk.DoLogin().
         *
         * @param lpClientErrorMsg Error message which should be sent back to
         * user. Set @c nErrorNo to ::CMDERR_SUCCESS if user is authorized.
         * @param lpUser The user properties gathered so far.
         * @param lpUserAccount The user account information which should
         * be set for this user. */
        public delegate void UserLogin(ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, ref UserAccount lpUserAccount);
        /**
         * @brief Register using #UserLogin delegate.
         */
        public event UserLogin OnUserLogin;

        /**
         * @brief Callback when a user is requesting to create a new user
         * account.
         *
         * This callback occurs in the context of TeamTalk.DoNewUserAccount().
         *
         * @param lpClientErrorMsg Error message which should be sent back to
         * user. Set @c nErrorNo to ::CMDERR_SUCCESS if user is authorized.
         * @param lpUser The user's properties.
         * @param lpUserAccount The properties of the user account to be created. */
        public delegate void UserCreateUserAccount(ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, ref UserAccount lpUserAccount);
        /**
         * @brief Register using #UserCreateUserAccount delegate.
         */
        public event UserCreateUserAccount OnUserCreateUserAccount;

        /**
         * @brief Callback when a user is requesting to delete a user
         * account.
         *
         * This callback occurs in the context of TeamTalk.DoDeleteUserAccount().
         *
         * @param lpClientErrorMsg Error message which should be sent back to
         * user. Set @c nErrorNo to ::CMDERR_SUCCESS if user is authorized.
         * @param lpUser The properties of the user requesting.
         * @param szUsername The username of the account to delete. */
        public delegate void UserDeleteUserAccount(ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, string szUsername);
        /**
         * @brief Register using #UserCreateUserAccount delegate.
         */
        public event UserDeleteUserAccount OnUserDeleteUserAccount;

        /**
         * @brief Callback when a user is requesting to ban a user.
         *
         * This callback occurs in the context of TeamTalk.DoBanUser().
         *
         * @param lpClientErrorMsg Error message which should be sent back to
         * user. Set @c nErrorNo to ::CMDERR_SUCCESS if user is authorized.
         * @param lpBanner The properties of the user requesting the ban.
         * @param lpBanee The properties of the user who should be banned. */
        public delegate void UserAddServerBan(ref ClientErrorMsg lpClientErrorMsg,  ref User lpBanner, ref User lpBanee);
        /**
         * @brief Register using #UserAddServerBan delegate.
         */
        public event UserAddServerBan OnUserAddServerBan;

        /**
         * @brief Callback when a user is requesting to ban an IP-address.
         *
         * This callback occurs in the context of TeamTalk.DoBanIPAddress().
         *
         * @param lpClientErrorMsg Error message which should be sent back to
         * user. Set @c nErrorNo to ::CMDERR_SUCCESS if user is authorized.
         * @param lpBanner The properties of the user requesting the ban. This value
         * can be NULL if #BearWare.ServerProperties @c nMaxLoginAttempts is enabled.
         * @param szIPAddress The IP-address to be banned. */
        public delegate void UserAddServerBanIPAddress(ref ClientErrorMsg lpClientErrorMsg,ref User lpBanner, string szIPAddress);
        /**
         * @brief Register using #UserAddServerBan delegate.
         */
        public event UserAddServerBanIPAddress OnUserAddServerBanIPAddress;

        /**
         * @brief Callback when a user is requesting to remove a ban.
         *
         * This callback occurs in the context of TeamTalk.DoUnBanUser().
         *
         * @param lpClientErrorMsg Error message which should be sent back to
         * user. Set @c nErrorNo to ::CMDERR_SUCCESS if user is authorized.
         * @param lpUser The properties of the user doing the request.
         * @param szIPAddress The IP-address to be unbanned. */
        public delegate void UserDeleteServerBan(ref ClientErrorMsg lpClientErrorMsg,ref User lpUser, string szIPAddress);
        /**
         * @brief Register using #UserAddServerBanIPAddress delegate.
         */
        public event UserDeleteServerBan OnUserDeleteServerBan;

        /** @} */

        /** @addtogroup serverlogevents
         * @{ */

        /**
         * @brief Callback when a new user is connecting to the server.
         *
         * @param lpUser The user properties gathered so far. */
        public delegate void UserConnected(ref User lpUser);

        /**
         * @brief Register using #UserConnected delegate.
         */
        public event UserConnected OnUserConnected;

        /**
         * @brief Callback when a user has logged in.
         *
         * This callback occurs in the context of TeamTalk.DoLogin() and if
         * #UserLogin returned ::CMDERR_SUCCESS.
         *
         * @param lpUser The user properties of the user who logged in. */
        public delegate void UserLoggedIn(ref User lpUser);
        /** 
         * @brief Register using #UserLoggedIn delegate.
         */
        public event UserLoggedIn OnUserLoggedIn;

        /**
         * @brief Callback when a user has logged out.
         *
         * @param lpUser The properties of the user.   */
        public delegate void UserLoggedOut(ref User lpUser);
        /**
         * @brief Register using #UserLoggedOut delegate.
         */
        public event UserLoggedOut OnUserLoggedOut;

        /**
         * @brief Callback when user has disconnected.
         *
         * @param lpUser The properties of the user.   */
        public delegate void UserDisconnected(ref User lpUser);
        /**
         * @brief Register using #UserDisconnected delegate.
         */
        public event UserDisconnected OnUserDisconnected;

        /**
         * @brief Callback when a user's connection has timed out.
         *
         * @param lpUser The properties of the user. */
        public delegate void UserTimedout(ref User lpUser);
        /**
         * @brief Register using #UserTimedout delegate.
         */
        public event UserTimedout OnUserTimedout;

        /**
         * @brief Callback when a user has been kicked.
         *
         * @param lpKicker The user who had initiated the kick. This can be 0.
         * @param lpKickee The user who has been kicked.
         * @param lpChannel The channel where the user is kicked from. The can be 0. */
        public delegate void UserKicked(ref User lpKicker, ref User lpKickee, ref Channel lpChannel);
        /**
         * @brief Register using #UserKicked delegate.
         */
        public event UserKicked OnUserKicked;

        /**
         * @brief Callback when a user has been banned.
         *
         * @param lpBanner The user who had initiated the ban. This can be 0.
         * @param lpBanee The user who has been banned. This may only
         * contain an IP-address.
         * @param lpChannel The channel where the user is banned from. The can be 0. */
        public delegate void UserBanned(ref User lpBanner, ref User lpBanee, IntPtr lpChannel);
        /**
         * @brief Register using #UserBanned delegate.
         */
        public event UserBanned OnUserBanned;

        /**
         * @brief Callback when a ban is removed.
         *
         * This callback occurs in the contect of TeamTalk.DoUnBanUser().
         *
         * @param lpUnbanner The user removing the ban.
         * @param szIPAddress The IP-address which is unbanned.   */
        public delegate void UserUnbanned(ref User lpUnbanner, string szIPAddress);
        /**
         * @brief Register using #UserUnbanned delegate.
         */
        public event UserUnbanned OnUserUnbanned;

        /**
         * @brief Callback when a user's properties are being updated.
         *
         * @param lpUser The properties of the user.   */
        public delegate void UserUpdated(ref User lpUser);
        /**
         * @brief Register using #UserUpdated delegate.
         */
        public event UserUpdated OnUserUpdated;

        /**
         * @brief Callback when a user has joined a channel.
         *
         * @param lpUser The properties of the user.
         * @param lpChannel The properties of the channel being joined.   */
        public delegate void UserJoinedChannel(ref User lpUser, ref Channel lpChannel);
        /**
         * @brief Register using #UserJoinedChannel delegate.
         */
        public event UserJoinedChannel OnUserJoinedChannel;

        /**
         * @brief Callback when a user has left a channel.
         *
         * @param lpUser The properties of the user.
         * @param lpChannel The properties of the channel being left. */
        public delegate void UserLeftChannel(ref User lpUser, ref Channel lpChannel);
        /**
         * @brief Register using #UserLeftChannel delegate.
         */
        public event UserLeftChannel OnUserLeftChannel;

        /**
         * @brief Callback when a user has been moved.
         *
         * This callback occurs in the context of TeamTalk.DoMoveUser().
         *
         * @param lpMover The user who initiated the move.
         * @param lpMovee The user who has been moved. */
        public delegate void UserMoved(ref User lpMover, ref User lpMovee);
        /**
         * @brief Register using #UserMoved delegate.
         */
        public event UserMoved OnUserMoved;

        /**
         * @brief Callback when a user is sending a text message.
         *
         * This callback occurs in the context of TeamTalk.DoTextMessage().
         *
         * @param lpUser The properties of the user.
         * @param lpTextMessage The text message being sent.   */
        public delegate void UserTextMessage(ref User lpUser,ref TextMessage lpTextMessage);
        /**
         * @brief Register using #UserTextMessage delegate.
         */
        public event UserTextMessage OnUserTextMessage;

        /**
         * @brief Callback when a new channel has been created.
         *
         * This callback occurs in the context of TeamTalk.DoMakeChannel() or
         * TeamTalk.DoJoinChannel().
         *
         * @param lpChannel The channel which has been created.
         * @param lpUser The user who created the channel. This can be 0.   */
        public delegate void ChannelCreated(ref Channel lpChannel,ref User lpUser);
        /**
         * @brief Register using #ChannelCreated delegate.
         */
        public event ChannelCreated OnChannelCreated;

        /**
         * @brief Callback when a channel has been updated.
         *
         * This callback occurs in the context of TeamTalk.DoUpdateChannel().
         *
         * @param lpChannel The new properties of the channel.
         * @param lpUser The user who initiated the update. This can be 0.   */
        public delegate void ChannelUpdated(ref Channel lpChannel,ref User lpUser);
        /**
         * @brief Register using #ChannelUpdated delegate.
         */
        public event ChannelUpdated OnChannelUpdated;

        /**
         * @brief Callback when channel has been removed.
         *
         * @param lpChannel The properties of the channel which has been removed.
         * @param lpUser The properties of the who initiated the
         * removal. This can be 0.  */
        public delegate void ChannelRemoved(ref Channel lpChannel,ref User lpUser);
        /**
         * @brief Register using #ChannelRemoved delegate.
         */
        public event ChannelRemoved OnChannelRemoved;

        /**
         * @brief Callback when a new file has been uploaded to a channel.
         *
         * @param lpRemoteFile The properties of the file.
         * @param lpUser The properties of the user who uploaded the file. */
        public delegate void FileUploaded(ref RemoteFile lpRemoteFile,ref User lpUser);
        /**
         * @brief Register using #FileUploaded delegate.
         */
        public event FileUploaded OnFileUploaded;

        /**
         * @brief Callback when a user has downloaded a file.
         *
         * @param lpRemoteFile The properties of the file.
         * @param lpUser The properties of the user who downloaded the file. */
        public delegate void FileDownloaded(ref RemoteFile lpRemoteFile,ref User lpUser);
        /**
         * @brief Register using #FileDownloaded delegate.
         */
        public event FileDownloaded OnFileDownloaded;

        /**
         * @brief Callback when a user has deleted a file.
         *
         * @param lpRemoteFile The properties of the file.
         * @param lpUser The properties of the user who deleted the file.   */
        public delegate void FileDeleted(ref RemoteFile lpRemoteFile,ref User lpUser);
        /**
         * @brief Register using #FileDeleted delegate.
         */
        public event FileDeleted OnFileDeleted;

        /**
         * @brief Callback when a user has updated the server properties.
         *
         * This callback occurs in the context of TeamTalk.DoUpdateServer().
         *
         * @param lpServerProperties The properties of the server.
         * @param lpUser The user who initiated the server update.   */
        public delegate void ServerUpdated(ref ServerProperties lpServerProperties,ref User lpUser);
        /**
         * @brief Register using #ServerUpdated delegate.
         */
        public event ServerUpdated OnServerUpdated;

        /**
         * @brief Callback when a user has reguested to save the server
         * configuration.
         *
         * @param lpUser The properties of the user who requested to save
         * the server configuration. This can be 0.  */
        public delegate void SaveServerConfig(ref User lpUser);
        /**
         * @brief Register using #SaveServerConfig delegate.
         */
        public event SaveServerConfig OnSaveServerConfig;

        /** @} */

        /** @addtogroup serverapi
         * @{ */

        /** @brief Instantiate TeamTalk server. Call UpdateServer()
         * and MakeChannel() to setup server properties and root
         * channel.
         *
         * Users cannot log in unless root channel and #BearWare.ServerProperties have been set.
         * @see UpdateServer()
         * @see MakeChannel()
         * @see StartServer() */
        public TeamTalk5Srv()
        {
            Init();
        }

        /** @brief Instantiate TeamTalk server with a root channel.
         *
         * Users cannot log in unless root channel and #BearWare.ServerProperties have been set.
         * @see UpdateServer()
         * @see MakeChannel()
         * @see StartServer() */
        public TeamTalk5Srv(Channel lpChannel)
            : base(lpChannel)
        {
            Init();
        }

        /** @brief Instantiate TeamTalk server with root channel and server properties.
         *
         * @see UpdateServer()
         * @see MakeChannel()
         * @see StartServer() */
        public TeamTalk5Srv(Channel lpChannel, ServerProperties lpServerProperties)
            : base(lpChannel, lpServerProperties)
        {
            Init();
        }

        void Init()
        {
            base.OnChannelCreatedCallback += new ChannelCreatedCallback(TeamTalkSrv_OnChannelCreatedCallback);
            base.OnChannelRemovedCallback += new ChannelRemovedCallback(TeamTalkSrv_OnChannelRemovedCallback);
            base.OnChannelUpdatedCallback += new ChannelUpdatedCallback(TeamTalkSrv_OnChannelUpdatedCallback);
            base.OnUserLoginCallback += new UserLoginCallback(TeamTalkSrv_OnUserLoginCallback);
            base.OnUserLoggedOutCallback += new UserLoggedOutCallback(TeamTalkSrv_OnUserLoggedOutCallback);
            base.OnUserLoggedInCallBack += new UserLoggedInCallback(TeamTalkSrv_OnUserLoggedInCallBack);
            base.OnUserTextMessageCallback += new UserTextMessageCallback(TeamTalkSrv_OnUserTextMessageCallback);
            base.OnUserAddServerBanCallback += new UserAddServerBanCallback(TeamTalkSrv_OnUserAddServerBanCallback);
            base.OnUserAddServerBanIPAddressCallback += new UserAddServerBanIPAddressCallback(TeamTalkSrv_OnUserAddServerBanIPAddressCallback);
            base.OnUserDeleteServerBanCallback += new UserDeleteServerBanCallback(TeamTalkSrv_OnUserDeleteServerBanCallback);
            base.OnUserBannedCallback += new UserBannedCallback(TeamTalkSrv_OnUserBannedCallback);
            base.OnUserConnectedCallback += new UserConnectedCallback(TeamTalkSrv_OnUserConnectedCallback);
            base.OnUserDisconnectedCallback += new UserDisconnectedCallback(TeamTalkSrv_OnUserDisconnectedCallback);
            base.OnUserTimedoutCallback += new UserTimedoutCallback(TeamTalkSrv_OnUserTimedoutCallback);
            base.OnUserLeftChannelCallback += new UserLeftChannelCallback(TeamTalkSrv_OnUserLeftChannelCallback);
            base.OnUserJoinedChannelCallback += new UserJoinedChannelCallback(TeamTalkSrv_OnUserJoinedChannelCallback);
            base.OnUserKickedCallback += new UserKickedCallback(TeamTalkSrv_OnUserKickedCallback);
            base.OnUserMovedCallback += new UserMovedCallback(TeamTalkSrv_OnUserMovedCallback);
            base.OnUserUpdatedCallback += new UserUpdatedCallback(TeamTalkSrv_OnUserUpdatedCallback);
            base.OnSaveServerConfigCallback += new SaveServerConfigCallback(TeamTalkSrv_OnSaveServerConfigCallback);
            base.OnServerUpdatedCallback += new ServerUpdatedCallback(TeamTalkSrv_OnServerUpdatedCallback);
            base.OnUserCreateUserAccountCallback += new UserCreateUserAccountCallback(TeamTalkSrv_OnUserCreateUserAccountCallback);
            base.OnUserDeleteUserAccountCallback += new UserDeleteUserAccountCallback(TeamTalkSrv_OnUserDeleteUserAccountCallback);
            base.OnFileDeletedCallback += new FileDeletedCallback(TeamTalkSrv_OnFileDeletedCallback);
            base.OnFileDownloadedCallback += new FileDownloadedCallback(TeamTalkSrv_OnFileDownloadedCallback);
            base.OnFileUploadedCallback += new FileUploadedCallback(TeamTalkSrv_OnFileUploadedCallback);
        }

        /** @} */

        void TeamTalkSrv_OnFileUploadedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref RemoteFile lpRemoteFile, ref User lpUser)
        {
           if(OnFileUploaded != null)
           {
               OnFileUploaded(ref lpRemoteFile, ref lpUser);
           }
        }

        void TeamTalkSrv_OnFileDownloadedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref RemoteFile lpRemoteFile, ref User lpUser)
        {
            
           if(OnFileDownloaded != null)
           {
               OnFileDownloaded(ref lpRemoteFile, ref lpUser);
           }
        }
        void TeamTalkSrv_OnFileDeletedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref RemoteFile lpRemoteFile, ref User lpUser)
        {
            if(OnFileDeleted != null)
            {
                OnFileDeleted(ref lpRemoteFile, ref lpUser);
            }
        }

        void TeamTalkSrv_OnUserDeleteUserAccountCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, string szUsername)
        {
            
            if(OnUserDeleteUserAccount != null)
            {
                OnUserDeleteUserAccount(ref lpClientErrorMsg, ref lpUser, szUsername);
            }
        }

        void TeamTalkSrv_OnUserCreateUserAccountCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, ref UserAccount lpUserAccount)
        {
            if(OnUserCreateUserAccount != null)
            {
                OnUserCreateUserAccount(ref lpClientErrorMsg, ref lpUser, ref lpUserAccount);
            }
        }
        
        void TeamTalkSrv_OnServerUpdatedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref ServerProperties lpServerProperties, ref User lpUser)
        {
            if(OnServerUpdated != null)
            {
                OnServerUpdated(ref lpServerProperties, ref lpUser);
            }
        }
        
        void TeamTalkSrv_OnSaveServerConfigCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser)
        {
            if(OnSaveServerConfig != null)
            {
                OnSaveServerConfig(ref lpUser);
            }
        }
        
        void TeamTalkSrv_OnUserUpdatedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser)
        {
            if(OnUserUpdated != null)
            {
                OnUserUpdated(ref lpUser);
            }
        }
        
        void TeamTalkSrv_OnUserMovedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpMover, ref User lpMovee)
        {
            if(OnUserMoved != null)
            {
                OnUserMoved(ref lpMover, ref lpMovee);
            }
        }
        
        void TeamTalkSrv_OnUserKickedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpKicker, ref User lpKickee, IntPtr lpChannel)
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
        
        void TeamTalkSrv_OnUserJoinedChannelCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser, ref Channel lpChannel)
        {
            if(OnUserJoinedChannel !=null)
            {
                OnUserJoinedChannel(ref lpUser, ref lpChannel);
            }
        }
        
        void TeamTalkSrv_OnUserLeftChannelCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser, ref Channel lpChannel)
        {
            if(OnUserLeftChannel != null)
            {
                OnUserLeftChannel(ref lpUser, ref lpChannel);
            }
        }
        
        void TeamTalkSrv_OnUserTimedoutCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser)
        {
            if(OnUserTimedout != null)
            {
                OnUserTimedout(ref lpUser);
            }
        }
        
        void TeamTalkSrv_OnUserDisconnectedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser)
        {
            if(OnUserDisconnected != null)
            {
                OnUserDisconnected(ref lpUser);
            }
        }
        
        void TeamTalkSrv_OnUserConnectedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser)
        {
            if(OnUserConnected != null)
            {
                OnUserConnected(ref lpUser);
            }
        }
        
        void TeamTalkSrv_OnUserBannedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpBanner, ref User lpBanee, IntPtr lpChannel)
        {
            if(OnUserBanned != null)
            {
                OnUserBanned(ref lpBanner, ref lpBanee, lpChannel);
            }
        }
        
        void TeamTalkSrv_OnUserDeleteServerBanCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, string szIPAddress)
        {
            if(OnUserDeleteServerBan != null)
            {
                OnUserDeleteServerBan(ref lpClientErrorMsg, ref lpUser, szIPAddress);
            }
        }
        
        void TeamTalkSrv_OnUserAddServerBanIPAddressCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref ClientErrorMsg lpClientErrorMsg, ref User lpBanner, string szIPAddress)
        {
            if(OnUserAddServerBanIPAddress != null)
            {
                OnUserAddServerBanIPAddress(ref lpClientErrorMsg, ref lpBanner, szIPAddress);
            }
        }
        
        void TeamTalkSrv_OnUserAddServerBanCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref ClientErrorMsg lpClientErrorMsg, ref User lpBanner, ref User lpBanee)
        {
            if(OnUserAddServerBan != null)
            {
                OnUserAddServerBan(ref lpClientErrorMsg, ref lpBanner, ref lpBanee);
            }
        }
        
        void TeamTalkSrv_OnUserTextMessageCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser, ref TextMessage lpTextMessage)
        {
            if(OnUserTextMessage !=null)
            {
                OnUserTextMessage(ref lpUser, ref lpTextMessage);
            }
        }
        
        void TeamTalkSrv_OnUserLoggedOutCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser)
        {
            if (OnUserLoggedOut != null)
            {
                OnUserLoggedOut(ref lpUser);
            }
        }
        
        void TeamTalkSrv_OnUserLoggedInCallBack(IntPtr lpTTSInstance, IntPtr lpUserData, ref User lpUser)
        {
            if (OnUserLoggedIn != null)
            {
                OnUserLoggedIn(ref lpUser);
            }
        }
        
        void TeamTalkSrv_OnUserLoginCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, ref UserAccount lpUserAccount)
        {
            if (OnUserLogin != null)
            {
                OnUserLogin(ref lpClientErrorMsg, ref lpUser, ref lpUserAccount);
            }
        }
        
        void TeamTalkSrv_OnChannelUpdatedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref Channel lpChannel, IntPtr lpUser)
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
        
        void TeamTalkSrv_OnChannelRemovedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref Channel lpChannel, IntPtr lpUser)
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
        
        void TeamTalkSrv_OnChannelCreatedCallback(IntPtr lpTTSInstance, IntPtr lpUserData, ref Channel lpChannel, IntPtr lpUser)
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
    }
}
