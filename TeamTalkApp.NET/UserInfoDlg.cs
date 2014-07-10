using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using BearWare;

namespace TeamTalkApp.NET
{
    public partial class UserInfoDlg : Form
    {
        TeamTalk ttclient;
        int userid;
        public UserInfoDlg(TeamTalk tt, int userid)
        {
            ttclient = tt;
            InitializeComponent();
            this.userid = userid;
            UpdateUser();
        }

        void UpdateUser()
        {
            User user;
            if (ttclient.GetUser(userid, out user))
            {
                useridTextBox.Text = user.nUserID.ToString();
                nicknameTextBox.Text = user.szNickname;
                usernameTextBox.Text = user.szUsername;
                statusmodeTextBox.Text = user.nStatusMode.ToString();
                statusmsgTextBox.Text = user.szStatusMsg;
                usertypeTextBox.Text = (user.uUserType & UserType.USERTYPE_ADMIN) == UserType.USERTYPE_ADMIN ? "Admin" : "Default";
                versionTextBox.Text = user.uVersion.ToString();
            }
            UserStatistics stats;
            if (ttclient.GetUserStatistics(userid, out stats))
            {
                audiolossTextBox.Text = String.Format("{0}/{1}", stats.nVoicePacketsLost, stats.nVoicePacketsRecv + stats.nVoicePacketsLost);
                videolossTextBox.Text = String.Format("{0}/{1}", stats.nVideoCaptureFramesLost, stats.nVideoCaptureFramesRecv + stats.nVideoCaptureFramesLost);
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            UpdateUser();
        }
    }
}