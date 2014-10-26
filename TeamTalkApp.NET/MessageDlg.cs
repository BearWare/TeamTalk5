/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
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
