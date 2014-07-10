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
    public enum HotKey
    {
        HOTKEY_PUSHTOTALK = 1
    };

    public partial class HotKeyDlg : Form
    {
        TeamTalk ttclient;
        public List<int> keys = new List<int>();
        List<int> activekeys = new List<int>();

        public HotKeyDlg(TeamTalk tt)
        {
            ttclient = tt;
            InitializeComponent();

            ttclient.OnHotKeyTest += new TeamTalk.HotKeyTest(ttclient_OnHotKeyTest);
        }

        void ttclient_OnHotKeyTest(int nVkCode, bool bActive)
        {
            if (bActive)
            {
                string name = "";
                if (ttclient.HotKey_GetKeyString(nVkCode, out name))
                    textBox1.Text += name + " ";

                keys.Add(nVkCode);
                activekeys.Add(nVkCode);
            }
            else
            {
                activekeys.Remove(nVkCode);
            }
            if (activekeys.Count == 0)
                this.Close();
        }

        private void HotKeyDlg_Load(object sender, EventArgs e)
        {
            ttclient.HotKey_InstallTestHook();
        }

        private void HotKeyDlg_FormClosing(object sender, FormClosingEventArgs e)
        {
            ttclient.HotKey_RemoveTestHook();
        }


    }
}