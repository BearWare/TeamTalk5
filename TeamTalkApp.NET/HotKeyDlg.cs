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
                if (ttclient.HotKey_GetKeyString(nVkCode, ref name))
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
