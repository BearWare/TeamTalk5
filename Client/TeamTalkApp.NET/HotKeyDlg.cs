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
    public enum HotKey
    {
        HOTKEY_PUSHTOTALK = 1
    };

    public partial class HotKeyDlg : Form
    {
        TeamTalkBase ttclient;
        public List<int> keys = new List<int>();
        List<int> activekeys = new List<int>();

        public HotKeyDlg(TeamTalkBase tt)
        {
            ttclient = tt;
            InitializeComponent();

            ttclient.OnHotKeyTest += new TeamTalkBase.HotKeyTest(ttclient_OnHotKeyTest);
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
