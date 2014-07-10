using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Diagnostics;
using System.Windows.Forms;
using BearWare;

namespace TeamTalkApp.NET
{
    class UsersView
    {
        TeamTalk ttclient;
        ListView listview;

        public UsersView(TeamTalk tt, ListView list)
        {
            ttclient = tt;
            listview = list;
            listview.Columns.Add("ID");
            listview.Columns.Add("Nickname");
            listview.Columns.Add("Username");
            listview.Columns.Add("Channel");
            listview.Columns.Add("IP-Address");
            listview.Columns.Add("StatusMsg");
            listview.Columns.Add("User Type");
            listview.Columns.Add("UserData");

            ttclient.OnCmdUserLoggedIn += new TeamTalk.UserUpdate(ttclient_OnCmdUserLoggedIn);
            ttclient.OnCmdUserLoggedOut += new TeamTalk.UserUpdate(ttclient_OnCmdUserLoggedOut);
            ttclient.OnCmdUserUpdate += new TeamTalk.UserUpdate(ttclient_OnCmdUserUpdate);
        }

        int GetSelectedUser()
        {
            if (listview.SelectedItems.Count > 0)
                return (int)listview.SelectedItems[0].Tag;
            return 0;
        }

        ListViewItem GetUser(int userid)
        {
            foreach (ListViewItem item in listview.Items)
                if ((int)item.Tag == userid)
                    return item;
            return null;
        }

        public List<int> GetUsers()
        {
            List<int> users = new List<int>();
            foreach (ListViewItem item in listview.Items)
                users.Add((int)item.Tag);
            return users;
        }

        void ttclient_OnCmdUserLoggedIn(User user)
        {
            ListViewItem item = new ListViewItem();
            item.Text = user.nUserID.ToString();
            item.SubItems.Add(user.szNickname);
            item.SubItems.Add(user.szUsername);
            string chanpath = "";
            if (user.nChannelID > 0)
                ttclient.GetChannelPath(user.nChannelID, out chanpath);
            item.SubItems.Add(chanpath);
            item.SubItems.Add(user.szIPAddress);
            item.SubItems.Add(user.szStatusMsg);
            item.SubItems.Add((user.uUserType & UserType.USERTYPE_ADMIN) == UserType.USERTYPE_ADMIN ? "Admin" : "Default");
            item.SubItems.Add(user.nUserData.ToString());
            item.Tag = user.nUserID;
            listview.Items.Add(item);
        }

        void ttclient_OnCmdUserLoggedOut(User user)
        {
            listview.Items.Remove(GetUser(user.nUserID));
        }

        void ttclient_OnCmdUserUpdate(User user)
        {
            ListViewItem item = GetUser(user.nUserID);
            if (item == null)
                return;

            item.SubItems[1].Text = user.szNickname;
            string chanpath = "";
            if (user.nChannelID > 0)
                ttclient.GetChannelPath(user.nChannelID, out chanpath);
            item.SubItems[3].Text = chanpath;
            item.SubItems[4].Text = user.szIPAddress;
            item.SubItems[5].Text = user.szStatusMsg;
        }

    }
}
