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
        TeamTalkBase ttclient;
        int userid;
        public MessageDlg(TeamTalkBase tt, int userid)
        {
            ttclient = tt;
            this.userid = userid;

            InitializeComponent();
            this.CenterToScreen();
        }

        public void NewMessage(TextMessage msg)
        {
            User user = new User();
            if (!ttclient.GetUser(msg.nFromUserID, ref user))
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
