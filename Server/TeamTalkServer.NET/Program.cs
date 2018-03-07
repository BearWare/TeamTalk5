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
using BearWare;

namespace TeamTalk5ProServer.NET
{
    public class TeamTalkServer
    {
        String FILESTORAGE_FOLDER = "/tmp";
        long MAX_DISKUSAGE = 1000000000, DEFAULT_CHANNEL_QUOTA = 10000000;
        String ADMIN_USERNAME = "admin", ADMIN_PASSWORD = "admin";
        String IPADDR = "127.0.0.1";
        int TCPPORT = 10333, UDPPORT = 10333;
        bool ENCRYPTED = false;
        List<UserAccount> useraccounts = new List<UserAccount>();
        public TeamTalkServer()
        {
            // Instantiate server
            TeamTalk5Srv server = new TeamTalk5Srv();
            TeamTalk5Srv.SetEncryptionContext("", "");

            if (server.SetChannelFilesRoot(FILESTORAGE_FOLDER, MAX_DISKUSAGE, DEFAULT_CHANNEL_QUOTA) != ClientError.SUCCESS)
            {
                Console.WriteLine("Failed to setup file sharing");
            }
            // create administrator user account
            UserAccount useraccount = new UserAccount();
            useraccount.szUsername = ADMIN_USERNAME;
            useraccount.szPassword = ADMIN_PASSWORD;
            useraccount.uUserType = UserType.USERTYPE_ADMIN;
            useraccount.szNote = "An example administrator user account with all user-rights";
            useraccount.uUserRights = UserRight.USERRIGHT_ALL;
            useraccounts.Add(useraccount);
            // Make root channel
            Channel chan = new Channel();
            chan.nChannelID = 1;
            chan.nParentID = 0;
            chan.nMaxUsers = 10;
            chan.szName = "/";
            chan.uChannelType = ChannelType.CHANNEL_PERMANENT;
            chan.audiocodec = new AudioCodec();
            chan.audiocfg = new AudioConfig(true);
            // Subscribe Events
            ServerCallback serverCallback = new ServerCallback(server, useraccounts);
            ServerLogger logger = new ServerLogger(server);
          
            ServerProperties srv = new ServerProperties();
            srv.nUserTimeout = 200;
            srv.nMaxUsers = 1000;
            //Update Serverproperties
            server.UpdateServer(srv);

            if(server.MakeChannel(chan) != ClientError.SUCCESS)
            {
                Console.WriteLine("Failed to create root channel");
            }

            // Start server
            if (!server.StartServer(IPADDR, TCPPORT, UDPPORT, ENCRYPTED))
            {
                Console.WriteLine("Failed to start server");
            }
            Console.Title = "TeamTalk Server v." + TeamTalk5Srv.GetVersion();
            Console.WriteLine("Started TeamTalk {0} Server v.{1}\nTCP port: {2}, UDP port: {3}",
                                (ENCRYPTED? "Encrypted" : "Non-Encrypted"),  TeamTalk5Srv.GetVersion(),
                                TCPPORT, UDPPORT);
            //run server forever
            while (server.RunEventLoop(0)) ;
        }
    }
    class Program
    {
        static void Main(string[] args)
        {
            TeamTalkServer srv = new TeamTalkServer();
        }
    }
}
