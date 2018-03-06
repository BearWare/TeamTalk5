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
using System.Threading.Tasks;
using BearWare;
namespace TeamTalk5ProServer.NET
{
    public class ServerLogger
    {
        TeamTalk5Srv ttserver;
        public ServerLogger(TeamTalk5Srv ttserver)
        {
            this.ttserver = ttserver;
            ttserver.OnUserConnected += new TeamTalk5Srv.UserConnected(ttserver_OnUserConnected);
            ttserver.OnUserLoggedIn += new TeamTalk5Srv.UserLoggedIn(ttserver_OnUserLoggedIn);
            ttserver.OnUserLoggedOut += new TeamTalk5Srv.UserLoggedOut(ttserver_OnUserLoggedOut);
            ttserver.OnUserTimedout += new TeamTalk5Srv.UserTimedout(ttserver_OnUserTimedout);
            ttserver.OnUserDisconnected += new TeamTalk5Srv.UserDisconnected(ttserver_OnUserDisconnected);
            ttserver.OnUserKicked += new TeamTalk5Srv.UserKicked(ttserver_OnUserKicked);
            ttserver.OnUserUnbanned += new TeamTalk5Srv.UserUnbanned(ttserver_OnUserUnbanned);
            ttserver.OnUserBanned += new TeamTalk5Srv.UserBanned(ttserver_OnUserBanned);
            ttserver.OnUserUpdated += new TeamTalk5Srv.UserUpdated(ttserver_OnUserUpdated);
            ttserver.OnUserJoinedChannel += new TeamTalk5Srv.UserJoinedChannel(ttserver_OnUserJoinedChannel);
            ttserver.OnUserLeftChannel += new TeamTalk5Srv.UserLeftChannel(ttserver_OnUserLeftChannel);
            ttserver.OnUserMoved += new TeamTalk5Srv.UserMoved(ttserver_OnUserMoved);
            ttserver.OnUserTextMessage += new TeamTalk5Srv.UserTextMessage(ttserver_OnUserTextMessage);
            ttserver.OnChannelCreated += new TeamTalk5Srv.ChannelCreated(ttserver_OnChannelCreated);
            ttserver.OnChannelRemoved += new TeamTalk5Srv.ChannelRemoved(ttserver_OnChannelRemoved);
            ttserver.OnChannelUpdated += new TeamTalk5Srv.ChannelUpdated(ttserver_OnChannelUpdated);
            ttserver.OnFileDeleted += new TeamTalk5Srv.FileDeleted(ttserver_OnFileDeleted);
            ttserver.OnFileDownloaded += new TeamTalk5Srv.FileDownloaded(ttserver_OnFileDownloaded);
            ttserver.OnFileUploaded += new TeamTalk5Srv.FileUploaded(ttserver_OnFileUploaded);
            ttserver.OnServerUpdated += new TeamTalk5Srv.ServerUpdated(ttserver_OnServerUpdated);
            ttserver.OnSaveServerConfig += new TeamTalk5Srv.SaveServerConfig(ttserver_OnSaveServerConfig);
        }


        void ttserver_OnSaveServerConfig(ref User lpUser)
        {
            String str = String.Format("Server settings saved");
            Console.WriteLine(str);
        }

        void ttserver_OnServerUpdated(ref ServerProperties lpServerProperties, ref User lpUser)
        {
            String str = String.Format("Server properties updated, name is now {0}",
                                   lpServerProperties.szServerName);
            Console.WriteLine(str);
        }

        void ttserver_OnFileUploaded(ref RemoteFile lpRemoteFile, ref User lpUser)
        {
            String str = String.Format("File {0} uploaded to channel #{1}",
                                  lpRemoteFile.szFileName, lpRemoteFile.nChannelID);
            Console.WriteLine(str);
        }

        void ttserver_OnFileDownloaded(ref RemoteFile lpRemoteFile, ref User lpUser)
        {
            String str = String.Format("File {0} downloaded to channel #{1}",
                           lpRemoteFile.szFileName, lpRemoteFile.nChannelID);
            Console.WriteLine(str); ;
        }

        void ttserver_OnFileDeleted(ref RemoteFile lpRemoteFile, ref User lpUser)
        {
            String str = String.Format("File {0} deleted from channel #{1}",
                           lpRemoteFile.szFileName, lpRemoteFile.nChannelID);
            Console.WriteLine(str);
        }

        void ttserver_OnChannelUpdated(ref Channel lpChannel, ref User lpUser)
        {
            String str = String.Format("Channel #{0} updated",
                           lpChannel.nChannelID);
            Console.WriteLine(str);
        }

        void ttserver_OnChannelRemoved(ref Channel lpChannel, ref User lpUser)
        {
            String str = String.Format("Channel #{0} removed",
                           lpChannel.nChannelID);
            Console.WriteLine(str);
        }

        void ttserver_OnChannelCreated(ref Channel lpChannel, ref User lpUser)
        {
            String str = String.Format("Channel #{0} created",
                                lpChannel.nChannelID);
            Console.WriteLine(str);
        }

        void ttserver_OnUserTextMessage(ref User lpUser, ref TextMessage lpTextMessage)
        {
            String str = String.Format("User {0} sent text message",
                           lpUser.szUsername);
            Console.WriteLine(str);
        }

        void ttserver_OnUserMoved(ref User lpMover, ref User lpMovee)
        {
            String str = String.Format("User {0} moved {1} to channel #{2}",
                            lpMover.szUsername, lpMovee.szUsername,
                            lpMovee.nChannelID);
            Console.WriteLine(str);
        }

        void ttserver_OnUserLeftChannel(ref User lpUser, ref Channel lpChannel)
        {
            String str = String.Format("User {0} left channel #{1}",
                           lpUser.szUsername, lpChannel.nChannelID);
            Console.WriteLine(str);
        }

        void ttserver_OnUserJoinedChannel(ref User lpUser, ref Channel lpChannel)
        {
            String str = String.Format("User {0} join channel #{1}",
                                lpUser.szUsername, lpChannel.nChannelID);
            Console.WriteLine(str);
        }

        void ttserver_OnUserUpdated(ref User lpUser)
        {
            String str = String.Format("User {0} updated properties",
                           lpUser.szUsername);
            Console.WriteLine(str);
        }

        void ttserver_OnUserBanned(ref User lpBanner, ref User lpBanee, IntPtr lpChannel)
        {
            String str = String.Format("User {0} banned by {1}",
                            lpBanner.szUsername, lpBanee.szUsername);
            Console.WriteLine(str);
        }

        void ttserver_OnUserUnbanned(ref User lpUnbanner, string szIPAddress)
        {
            String str = String.Format("User {0} unbanned IP-address {1}",
                           lpUnbanner.szUsername, szIPAddress);
            Console.WriteLine(str);
        }

        void ttserver_OnUserKicked(ref User lpKicker, ref User lpKickee, ref Channel lpChannel)
        {
            String str = String.Format("User {0} kicked by {1}",
                            lpKicker.szUsername, lpKickee.szUsername);
            Console.WriteLine(str);
        }

        void ttserver_OnUserDisconnected(ref User lpUser)
        {
            String str = String.Format("User {0} disconnected",
                           lpUser.szUsername);
            Console.WriteLine(str);
        }

        void ttserver_OnUserTimedout(ref User lpUser)
        {
            String str = String.Format("User {0} timed out",
                             lpUser.szUsername);
            Console.WriteLine(str);
        }

        void ttserver_OnUserLoggedOut(ref User lpUser)
        {
            String str = String.Format("User {0} logged out",
                           lpUser.szUsername);
            Console.WriteLine(str);
        }

        void ttserver_OnUserLoggedIn(ref User lpUser)
        {
            String str = String.Format("User {0} logged in with nickname {1}",
                               lpUser.szUsername, lpUser.szNickname);
            Console.WriteLine(str);
        }

        void ttserver_OnUserConnected(ref User lpUser)
        {
            String str = String.Format("User with IP-address {0} connected",
                           lpUser.szIPAddress);
            Console.WriteLine(str);
        }
    }
}
