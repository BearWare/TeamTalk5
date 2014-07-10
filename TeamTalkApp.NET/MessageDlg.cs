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
    public partial class MessageDlg : Form
    {
        TeamTalk ttclient;
        int userid;
        public MessageDlg(TeamTalk tt, int userid)
        {
            ttclient = tt;
            this.userid = userid;

            InitializeComponent();
            this.CenterToScreen();
        }

        public void NewMessage(TextMessage msg)
        {
            User user;
            if (!ttclient.GetUser(msg.nFromUserID, out user))
                return;
            this.Text = "Message - " + user.szNickname;
            historyTextBox.AppendText("<" + user.szNickname + "> " + msg.szMessage + Environment.NewLine);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if(newmsgTextBox.Text.Length == 0)
                return;

            TextMessage msg;
            msg.nChannelID = 0; //only applies to channel messages
            msg.nFromUserID = ttclient.GetMyUserID();
            msg.szFromUsername = ""; //not required
            msg.nMsgType = TextMsgType.MSGTYPE_USER;
            msg.nToUserID = userid;
            msg.szMessage = newmsgTextBox.Text;
            newmsgTextBox.Text = "";
            if (ttclient.DoTextMessage(msg) > 0)
                NewMessage(msg);
        }
    }
}