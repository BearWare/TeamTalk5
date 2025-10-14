﻿/*
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
    public class ServerCallback
    {
        TeamTalk5Srv ttserver;
        List<UserAccount> useraccounts;

        List<String> banned_ipaddr = new List<String>();

        UserAccount getUserAccount(String username)
        {
            foreach (UserAccount u in useraccounts)
            {
                if (u.szUsername.Equals(username))
                    return u;
            }
            return new UserAccount();
        }
        public ServerCallback(TeamTalk5Srv ttserver, List<UserAccount> useraccounts)
        {
            this.ttserver = ttserver;
            this.useraccounts = useraccounts;
            ttserver.OnUserLogin += new TeamTalk5Srv.UserLogin(ttserver_OnUserLogin);
            ttserver.OnUserCreateUserAccount += new TeamTalk5Srv.UserCreateUserAccount(ttserver_OnUserCreateUserAccount);
            ttserver.OnUserDeleteUserAccount += new TeamTalk5Srv.UserDeleteUserAccount(ttserver_OnUserDeleteUserAccount);
            ttserver.OnUserAddServerBan += new TeamTalk5Srv.UserAddServerBan(ttserver_OnUserAddServerBan);
            ttserver.OnUserAddServerBanIPAddress += new TeamTalk5Srv.UserAddServerBanIPAddress(ttserver_OnUserAddServerBanIPAddress);
            ttserver.OnUserDeleteServerBan += new TeamTalk5Srv.UserDeleteServerBan(ttserver_OnUserDeleteServerBan);
            ttserver.OnUserChangeNickname += new TeamTalk5Srv.UserChangeNickname(ttserver_OnUserChangeNickname);
            ttserver.OnUserChangeStatus += new TeamTalk5Srv.UserChangeStatus(ttserver_OnUserChangeStatus);
        
        }

        void ttserver_OnUserChangeStatus(ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, ref int nNewStatusMode, string szNewStatusMsg)
        {
                String str = String.Format("User {0} is requesting to change status message to {1}",
                                           lpUser.szNickname, szNewStatusMsg);
                Console.WriteLine(str);

                if(szNewStatusMsg.IndexOf("crap")>=0) {
                    lpClientErrorMsg.nErrorNo = 4568;
                    lpClientErrorMsg.szErrorMsg = "Status not allowed";
                }
                else {
                    lpClientErrorMsg.nErrorNo = (int)ClientError.CMDERR_SUCCESS;
}
        }

        void ttserver_OnUserChangeNickname(ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, string szNewNickname)
        {
            
                String str = String.Format("User {0} is requesting to change nickname to {1}",
                                           lpUser.szNickname, szNewNickname);
                Console.WriteLine(str);

                if(szNewNickname.IndexOf("crap")>=0) {
                    lpClientErrorMsg.nErrorNo = 4567;
                    lpClientErrorMsg.szErrorMsg = "Nickname not allowed";
                }
                else {
                    lpClientErrorMsg.nErrorNo = (int)ClientError.CMDERR_SUCCESS;
}
        }



        void ttserver_OnUserDeleteServerBan(ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, string szIPAddress)
        {
            UserAccount ua = getUserAccount(lpUser.szUsername);

            if ((ua.uUserRights & UserRight.USERRIGHT_BAN_USERS) != 0)
            {
                lpClientErrorMsg.nErrorNo = (int)ClientError.SUCCESS;
                banned_ipaddr.Remove(szIPAddress);
            }
            else
            {
                lpClientErrorMsg.nErrorNo = (int)ClientError.CMDERR_NOT_AUTHORIZED;
                lpClientErrorMsg.szErrorMsg = "Not allowed!";
            }
        }

        void ttserver_OnUserAddServerBanIPAddress(ref ClientErrorMsg lpClientErrorMsg, ref User lpBanner, string szIPAddress)
        {
            UserAccount ua = getUserAccount(lpBanner.szUsername);

            if ((ua.uUserRights & UserRight.USERRIGHT_BAN_USERS) != 0)
            {
                lpClientErrorMsg.nErrorNo = (int)ClientError.SUCCESS;
                banned_ipaddr.Add(szIPAddress);
            }
            else
            {
                lpClientErrorMsg.nErrorNo = (int)ClientError.CMDERR_NOT_AUTHORIZED;
                lpClientErrorMsg.szErrorMsg = "Not allowed!";
            }
        }

        void ttserver_OnUserAddServerBan(ref ClientErrorMsg lpClientErrorMsg, ref User lpBanner, ref User lpBanee)
        {
            UserAccount ua = getUserAccount(lpBanner.szUsername);

            if ((ua.uUserRights & UserRight.USERRIGHT_BAN_USERS) != 0)
            {
                lpClientErrorMsg.nErrorNo = (int)ClientError.SUCCESS;
                banned_ipaddr.Add(lpBanee.szIPAddress);
            }
            else
            {
                lpClientErrorMsg.nErrorNo = (int)ClientError.CMDERR_NOT_AUTHORIZED;
                lpClientErrorMsg.szErrorMsg = "Not allowed!";
            }
        }



        void ttserver_OnUserDeleteUserAccount(ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, string szUsername)
        {
            String str = String.Format("User {0} is deleting useraccount {1}",
                           lpUser.szUsername, szUsername);
            Console.WriteLine(str);

            switch (lpUser.uUserType)
            {
                case UserType.USERTYPE_ADMIN:
                    UserAccount ua = new UserAccount();
                    foreach (UserAccount u in useraccounts)
                    {
                        if (u.szUsername.Equals(szUsername))
                            ua = u;
                    }
                    useraccounts.Remove(ua);
                    lpClientErrorMsg.nErrorNo = (int)ClientError.SUCCESS;
                    break;
                case UserType.USERTYPE_DEFAULT:
                    lpClientErrorMsg.nErrorNo = (int)ClientError.CMDERR_NOT_AUTHORIZED;
                    lpClientErrorMsg.szErrorMsg = "Not allowed!";
                    break;
                default:
                    break;
            }
        }

        void ttserver_OnUserCreateUserAccount(ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, ref UserAccount lpUserAccount)
        {
            String str = String.Format("User {0} is creating useraccount {1}",
    lpUser.szUsername, lpUserAccount.szUsername);
            Console.WriteLine(str);

            switch (lpUser.uUserType)
            {
                case UserType.USERTYPE_ADMIN:

                    // remove existing user account
                    UserAccount ua = new UserAccount();
                    foreach (UserAccount u in useraccounts)
                    {
                        if (u.szUsername.Equals(lpUserAccount.szUsername))
                            ua = u;
                    }
                    useraccounts.Remove(ua);

                    // add user account to list
                    useraccounts.Add(lpUserAccount);

                    lpClientErrorMsg.nErrorNo = (int)ClientError.SUCCESS;
                    break;
                case UserType.USERTYPE_DEFAULT:
                    lpClientErrorMsg.nErrorNo = (int)ClientError.CMDERR_NOT_AUTHORIZED;
                    lpClientErrorMsg.szErrorMsg = "Not allowed!";
                    break;
                default:
                    break;
            }
        }

        void ttserver_OnUserLogin(ref ClientErrorMsg lpClientErrorMsg, ref User lpUser, ref UserAccount lpUserAccount)
        {
            String str = String.Format("Login attempt from IP {0}, username={1}, password={2}",
                           lpUser.szIPAddress, lpUserAccount.szUsername,
                           lpUserAccount.szPassword);
            Console.WriteLine(str);

            foreach (UserAccount u in useraccounts)
            {
                // validate user account
                if (u.szUsername.Equals(lpUserAccount.szUsername) &&
                   u.szPassword.Equals(lpUserAccount.szPassword))
                {
                    // manually copy every field
                    lpUserAccount.szUsername = u.szUsername;
                    lpUserAccount.szPassword = u.szPassword;
                    lpUserAccount.uUserRights = u.uUserRights;
                    lpUserAccount.uUserType = UserType.USERTYPE_ADMIN;
                    lpClientErrorMsg.nErrorNo = (int)ClientError.SUCCESS;
                    return;
                }
            }

            // login rejected
            lpClientErrorMsg.nErrorNo = (int)ClientError.CMDERR_INVALID_ACCOUNT;
            lpClientErrorMsg.szErrorMsg = "Invalid username or password";
        }
    }
}
