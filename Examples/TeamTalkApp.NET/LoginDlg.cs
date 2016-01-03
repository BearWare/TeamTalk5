/*
 * Copyright (c) 2005-2016, BearWare.dk
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
    public partial class LoginDlg : Form
    {
        TeamTalk ttclient;
        Settings settings;
        int login_cmdid = 0;

        public LoginDlg(TeamTalk tt, Settings settings)
        {
            ttclient = tt;
            this.settings = settings;
            InitializeComponent();

            nickTextBox.Text = settings.nickname;
            usernameTextBox.Text = settings.server.username;
            passwdTextBox.Text = settings.server.password;

            ttclient.OnCmdProcessing += new TeamTalk.CommandProcessing(ttclient_OnCmdProcessing);
            ttclient.OnCmdError += new TeamTalk.CommandError(ttclient_OnCmdError);
        }

        void ttclient_OnCmdError(int nCmdID, ClientErrorMsg clienterrormsg)
        {
            if (nCmdID == login_cmdid)
            {
                login_cmdid = 0; //reset cmdid
                button1.Enabled = true;
                nickTextBox.Enabled = true;
                usernameTextBox.Enabled = true;
                passwdTextBox.Enabled = true;

                MessageBox.Show(clienterrormsg.szErrorMsg);
            }
        }

        void ttclient_OnCmdProcessing(int nCmdID, bool bActive)
        {
            if (!bActive && nCmdID == login_cmdid)
                this.Close();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            settings.nickname = nickTextBox.Text;
            settings.server.username = usernameTextBox.Text;
            settings.server.password = passwdTextBox.Text;

            login_cmdid = ttclient.DoLogin(nickTextBox.Text, usernameTextBox.Text,
                                           passwdTextBox.Text);
            button1.Enabled = false;
            nickTextBox.Enabled = false;
            usernameTextBox.Enabled = false;
            passwdTextBox.Enabled = false;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            ttclient.DoQuit();
        }

        private void LoginDlg_Load(object sender, EventArgs e)
        {
            this.CenterToScreen();
        }
    }
}
