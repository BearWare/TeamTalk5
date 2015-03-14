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
    public partial class ServerPropertiesDlg : Form
    {
        TeamTalk ttclient;
        ServerProperties prop;

        public ServerPropertiesDlg(TeamTalk tt)
        {
            ttclient = tt;
            InitializeComponent();

            ttclient.GetServerProperties(ref prop);
            srvnameTextBox.Text = prop.szServerName;
            maxusersNumericUpDown.Value = prop.nMaxUsers;
            motdTextBox.Text = prop.szMOTD;
            motdrawTextBox.Text = prop.szMOTDRaw;
            tcpportNumericUpDown.Value = prop.nTcpPort;
            udpportNumericUpDown.Value = prop.nUdpPort;
            usertimeoutNumericUpDown.Value = prop.nUserTimeout;
            autosaveCheckBox.Checked = prop.bAutoSave;

            voiceNumericUpDown1.Value = prop.nMaxVoiceTxPerSecond / 1024;
            vidcapNumericUpDown5.Value = prop.nMaxVideoCaptureTxPerSecond /1024;
            mediafileNumericUpDown2.Value = prop.nMaxMediaFileTxPerSecond / 1024;
            desktopNumericUpDown3.Value = prop.nMaxDesktopTxPerSecond / 1024;
            totalNumericUpDown4.Value = prop.nMaxTotalTxPerSecond / 1024;

            versionTextBox.Text = prop.szServerVersion;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            prop.szServerName = srvnameTextBox.Text;
            prop.nMaxUsers = (int)maxusersNumericUpDown.Value;
            prop.szMOTDRaw = motdrawTextBox.Text;
            prop.nTcpPort = (int)tcpportNumericUpDown.Value;
            prop.nUdpPort = (int)udpportNumericUpDown.Value;
            prop.nUserTimeout = (int)usertimeoutNumericUpDown.Value;
            prop.bAutoSave = autosaveCheckBox.Checked;
            prop.nMaxVoiceTxPerSecond = (int)voiceNumericUpDown1.Value * 1024;
            prop.nMaxVideoCaptureTxPerSecond = (int)vidcapNumericUpDown5.Value * 1024;
            prop.nMaxMediaFileTxPerSecond = (int)mediafileNumericUpDown2.Value * 1024;
            prop.nMaxDesktopTxPerSecond = (int)desktopNumericUpDown3.Value * 1024;
            prop.nMaxTotalTxPerSecond = (int)totalNumericUpDown4.Value * 1024;

            ttclient.DoUpdateServer(prop);
        }
    }
}
