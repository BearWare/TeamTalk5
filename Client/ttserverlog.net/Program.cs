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
using System.IO;
using System.Diagnostics;
using BearWare;

namespace ttserverlog.net
{
    class Program
    {
        static BearWare.TeamTalkBase ttclient;

        static string audio_dir = "c:\\temp";

        static int cur_cmd_id = 0;

        [STAThread] //this macro must be set in console applications for TeamTalk to work properly
        static void Main(string[] args)
        {
#if ENABLE_ENCRYPTION
            ttclient = new TeamTalk5Pro(true);
            int tcpport = TeamTalk5Pro.DEFAULT_TCPPORT, udpport = TeamTalk5Pro.DEFAULT_UDPPORT;
#else
            ttclient = new TeamTalk5(true);
            int tcpport = TeamTalk5.DEFAULT_TCPPORT, udpport = TeamTalk5.DEFAULT_UDPPORT;
#endif
            string ipaddr = "192.168.1.110";
            string username = "admin", password = "admin";
            bool encrypted = false;

            Console.WriteLine("TeamTalk 5 server logger.");
            Console.WriteLine();
            Console.WriteLine("TeamTalk 5 server login information.");
            Console.Write("Specify IP-address of server to connect to: ");
            ipaddr = get_str(ipaddr);
            Console.Write("Specify TCP port: ");
            tcpport = get_int(tcpport);
            Console.Write("Specify UDP port: ");
            udpport = get_int(udpport);
            Console.Write("Is server encrypted? ");
            encrypted = get_str("N").ToLower() == "y";
            Console.WriteLine("Administrator account information.");
            Console.Write("Specify username: ");
            username = get_str(username);
            Console.Write("Specify password: ");
            password = get_str(password);
            Console.Write("Specify directory where to store audio: ");
            audio_dir = get_str(audio_dir);

            if(!File.Exists("lame_enc.dll"))
                Console.WriteLine("lame_enc.dll not found, so audio will be stored to .wav instead of .mp3");

            //hook events for checking connection
            ttclient.OnConnectionFailed += new TeamTalkBase.Connection(ttclient_OnConnectFailed);
            ttclient.OnConnectionSuccess += new TeamTalkBase.Connection(ttclient_OnConnectSuccess);
            ttclient.OnConnectionLost += new TeamTalkBase.Connection(ttclient_OnConnectionLost);

            //now that we got all the information we needed we can connect and logon
            if (!ttclient.Connect(ipaddr, tcpport, udpport, 0, 0, encrypted))
            {
                Console.WriteLine("Failed to connect to server");
                return;
            }

            TTMessage msg = new TTMessage();
            //wait for 10 seconds for connect event
            if (!ttclient.GetMessage(ref msg, 10000))
            {
                Console.WriteLine("Did not connect in time. Aborting...");
                return;
            }

            Debug.Assert((ttclient.Flags & ClientFlag.CLIENT_CONNECTED) == ClientFlag.CLIENT_CONNECTED);

            //hook the remaining events we want to process
            ttclient.OnCmdMyselfLoggedIn += new TeamTalkBase.MyselfLoggedIn(ttclient_OnCmdMyselfLoggedIn);
            ttclient.OnCmdServerUpdate += new TeamTalkBase.ServerUpdate(ttclient_OnCmdServerUpdate);
            ttclient.OnCmdChannelNew += new TeamTalkBase.ChannelUpdate(ttclient_OnCmdChannelNew);
            ttclient.OnCmdChannelUpdate += new TeamTalkBase.ChannelUpdate(ttclient_OnCmdChannelUpdate);
            ttclient.OnCmdChannelRemove += new TeamTalkBase.ChannelUpdate(ttclient_OnCmdChannelRemove);
            ttclient.OnCmdUserLoggedIn += new TeamTalkBase.UserUpdate(ttclient_OnCmdUserLoggedIn);
            ttclient.OnCmdUserLoggedOut += new TeamTalkBase.UserUpdate(ttclient_OnCmdUserLoggedOut);
            ttclient.OnCmdUserUpdate += new TeamTalkBase.UserUpdate(ttclient_OnCmdUserUpdate);
            ttclient.OnCmdUserJoinedChannel += new TeamTalkBase.UserUpdate(ttclient_OnCmdUserJoinedChannel);
            ttclient.OnCmdUserLeftChannel += new TeamTalkBase.UserUpdate(ttclient_OnCmdUserLeftChannel);
            ttclient.OnCmdFileNew += new TeamTalkBase.FileUpdate(ttclient_OnCmdFileNew);
            ttclient.OnCmdFileRemove += new TeamTalkBase.FileUpdate(ttclient_OnCmdFileRemove);
            ttclient.OnCmdUserTextMessage += new TeamTalkBase.UserTextMessage(ttclient_OnCmdUserTextMessage);
            ttclient.OnCmdProcessing += new TeamTalkBase.CommandProcessing(ttclient_OnCmdProcessing);
            ttclient.OnCmdError += new TeamTalkBase.CommandError(ttclient_OnCmdError);
            ttclient.OnCmdSuccess += new TeamTalkBase.CommandSuccess(ttclient_OnCmdSuccess);
            ttclient.OnUserRecordMediaFile += new TeamTalkBase.UserRecordMediaFile(ttclient_OnUserAudioFile);

            //now that we're connected log on
            cur_cmd_id = ttclient.DoLogin("ttserverlog", username, password);
            Console.WriteLine("Login command got cmd ID #" + cur_cmd_id);
            if (cur_cmd_id < 0)
            {
                Console.WriteLine("Failed to issue command. Aborting...");
                return;
            }

            //wait for login command to complete
            while (ttclient.GetMessage(ref msg, 10000))
            {
                ttclient.ProcessMsg(msg);
                if (msg.nClientEvent == ClientEvent.CLIENTEVENT_CMD_PROCESSING &&
                    msg.nSource == cur_cmd_id && (bool)msg.DataToObject() /* bActive */ == false)
                    break;
            }

            //if this assertion failed it's because the login information was incorrect
            Debug.Assert((ttclient.Flags & ClientFlag.CLIENT_AUTHORIZED) == ClientFlag.CLIENT_AUTHORIZED);
            //we have to be admin user to do logging on the server.
            Debug.Assert((ttclient.UserType & UserType.USERTYPE_ADMIN) == UserType.USERTYPE_ADMIN);

            //now process events forever.
            while (ttclient.GetMessage(ref msg, -1))
            {
                ttclient.ProcessMsg(msg);
            }
        }

        static void ttclient_OnUserAudioFile(int nUserID, MediaFileInfo mediafileinfo)
        {
            User user = new User();
            if (ttclient.GetUser(nUserID, ref user))
            {
                switch (mediafileinfo.nStatus)
                {
                    case MediaFileStatus.MFS_ERROR :
                        Console.WriteLine("Failed to store audio file for #{0} {1}.",
                            user.nUserID, user.szNickname);
                        break;
                    case MediaFileStatus.MFS_STARTED :
                        Console.WriteLine("Storing audio from #{0} for {1} to file.",
                            user.nUserID, user.szNickname);
                        break;
                    case MediaFileStatus.MFS_FINISHED :
                        Console.WriteLine("Finished storing audio from #{0} {1} to file.",
                            user.nUserID, user.szNickname);
                        break;
                    case MediaFileStatus.MFS_ABORTED :
                        Console.WriteLine("Aborted storing audio from #{0} {1} to file.",
                            user.nUserID, user.szNickname);
                        break;
                }
            }
        }

        static void ttclient_OnCmdError(int nCmdID, ClientErrorMsg clienterrormsg)
        {
            Console.WriteLine("Command with ID #{0} failed with the following error: {1}",
                nCmdID, clienterrormsg.szErrorMsg);
        }

        static void ttclient_OnCmdSuccess(int nCmdID)
        {
            Console.WriteLine("Command with ID #{0} succeeded.", nCmdID);
        }

        static void ttclient_OnCmdProcessing(int nCmdID, bool bActive)
        {
            if (bActive)
                Console.WriteLine("Started processing cmd ID #{0}", nCmdID);
            else
            {
                Console.WriteLine("Finished processing cmd ID #{0}", nCmdID);
                cur_cmd_id = 0;
            }
        }

        static void ttclient_OnCmdUserTextMessage(TextMessage textmessage)
        {
            User user = new User(), touser = new User();
            if (!ttclient.GetUser(textmessage.nFromUserID, ref user))
                return;
            ttclient.GetUser(textmessage.nToUserID, ref touser);
            string chanpath = "";
            ttclient.GetChannelPath(textmessage.nChannelID, ref chanpath);
            switch (textmessage.nMsgType)
            {
                case TextMsgType.MSGTYPE_USER :
                    Console.WriteLine("Text message from user #{0} {1} to user #{2} {3} content: {4}",
                        user.nUserID, user.szNickname, touser.nUserID, touser.szNickname, textmessage.szMessage);
                    break;
                case TextMsgType.MSGTYPE_CHANNEL :
                    Console.WriteLine("Text message from user #{0} {1} to channel {2} context: {3}",
                        user.nUserID, user.szNickname, chanpath, textmessage.szMessage);
                    break;
                case TextMsgType.MSGTYPE_BROADCAST :
                    Console.WriteLine("Text messagr from user #{0} {1} to entire server content: {2}",
                        user.nUserID, user.szNickname, textmessage.szMessage);
                    break;
            }
        }

        static void ttclient_OnCmdFileRemove(RemoteFile fileinfo)
        {
            string chanpath = "";
            if (ttclient.GetChannelPath(fileinfo.nChannelID, ref chanpath))
                Console.WriteLine("File #{0} {1} removed from channel {1}", 
                    fileinfo.nFileID, fileinfo.szFileName, chanpath);
        }

        static void ttclient_OnCmdFileNew(RemoteFile fileinfo)
        {
            string chanpath = "";
            if (ttclient.GetChannelPath(fileinfo.nChannelID, ref chanpath))
                Console.WriteLine("File #{0} {1} added to channel {2}",
                    fileinfo.nFileID, fileinfo.szFileName, chanpath);

        }

        static void ttclient_OnCmdUserLeftChannel(User user)
        {
            string chanpath = "";
            if (ttclient.GetChannelPath(user.nChannelID, ref chanpath))
                Console.WriteLine("User #{0} {1} left {2}",
                    user.nUserID, user.szNickname, chanpath);
        }

        static void ttclient_OnCmdUserJoinedChannel(User user)
        {
            string chanpath = "";
            if (ttclient.GetChannelPath(user.nChannelID, ref chanpath))
            {
                Console.WriteLine("User #{0} {1} joined {2}",
                    user.nUserID, user.szNickname, chanpath);
                int cmd_id = ttclient.DoSubscribe(user.nUserID, Subscription.SUBSCRIBE_INTERCEPT_USER_MSG |
                    Subscription.SUBSCRIBE_INTERCEPT_CHANNEL_MSG | Subscription.SUBSCRIBE_INTERCEPT_VOICE);
                if (cmd_id < 0)
                    Console.WriteLine("Failed to issue subscribe command");
                else
                    Console.WriteLine("Issued command to subscribe to #{0}", user.nUserID);
            }
        }

        static void ttclient_OnCmdUserUpdate(User user)
        {
            string chanpath = "";
            if (ttclient.GetChannelPath(user.nChannelID, ref chanpath))
                Console.WriteLine("User #{0} {1} updated {2}",
                    user.nUserID, user.szNickname, chanpath);
        }

        static void ttclient_OnCmdUserLoggedOut(User user)
        {
            //cannot retrieve user since he no longer exists
            Console.WriteLine("User #{0} logged out", user.nUserID);
        }

        static void ttclient_OnCmdUserLoggedIn(User user)
        {
            Console.WriteLine("User #{0} {1} logged in",
                user.nUserID, user.szNickname);
            ttclient.SetUserMediaStorageDir(user.nUserID, audio_dir, "", AudioFileFormat.AFF_WAVE_FORMAT);
        }

        static void ttclient_OnCmdChannelRemove(Channel channel)
        {
            //cannot retrieve
            Console.WriteLine("Removed channel #{0}", channel.nChannelID);
        }

        static void ttclient_OnCmdChannelUpdate(Channel channel)
        {
            string chanpath = "";
            if (ttclient.GetChannelPath(channel.nChannelID, ref chanpath))
                Console.WriteLine("Updated channel {0}", chanpath);
        }

        static void ttclient_OnCmdChannelNew(Channel channel)
        {
            string chanpath = "";
            if (ttclient.GetChannelPath(channel.nChannelID, ref chanpath))
            {
                Console.WriteLine("Added channel {0}. Audio storage: {1}", chanpath, audio_dir);
            }
        }

        static void ttclient_OnCmdServerUpdate(ServerProperties serverproperties)
        {
            Console.WriteLine("Get new server properties.");
            Console.WriteLine("Server Name: " + serverproperties.szServerName);
            Console.WriteLine("MOTD: " + serverproperties.szMOTD);
            Console.WriteLine("Server Version: " + serverproperties.szServerVersion);
        }

        static void ttclient_OnCmdMyselfLoggedIn(int nMyUserID, UserAccount useraccount)
        {
            Console.WriteLine("Logged in successfully...");
            Console.WriteLine("Got user ID #" + nMyUserID);
            Debug.Assert(nMyUserID == ttclient.UserID);
        }

        static void ttclient_OnConnectFailed()
        {
            Console.WriteLine("Failed to connect to server");
        }

        static void ttclient_OnConnectSuccess()
        {
            Console.WriteLine("Connected successfully to server");
        }

        static void ttclient_OnConnectionLost()
        {
            Console.WriteLine("Connection was lost to the server");
        }











        static string get_str(string def_str)
        {
            Console.Write("(\"" + def_str + "\")");
            string str = Console.ReadLine();
            if (str.Length == 0)
                return def_str;
            return str;
        }

        static int get_int(int def_int)
        {
            string str = get_str(def_int.ToString());
            return int.Parse(str);
        }
    }
}
