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
                    lpUserAccount.szUsername = u.szPassword;
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
