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