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
    public partial class ServerStatsDlg : Form
    {
        TeamTalkBase ttclient;
        int cmd_id;

        public ServerStatsDlg(TeamTalkBase tt)
        {
            InitializeComponent();
            ttclient = tt;

            ttclient.OnCmdServerStatistics += new TeamTalkBase.ServerStats(ttclient_OnCmdServerStatistics);

            cmd_id = ttclient.DoQueryServerStats();
            this.CenterToScreen();
        }

        void ttclient_OnCmdServerStatistics(ServerStatistics serverstatistics)
        {
            totalTextBox.Text = String.Format("{0}/{1} KBytes", serverstatistics.nTotalBytesRX / 1024, serverstatistics.nTotalBytesTX / 1024);
            voiceTextBox.Text = String.Format("{0}/{1} KBytes", serverstatistics.nVoiceBytesRX / 1024, serverstatistics.nVoiceBytesTX / 1024);
            videoTextBox.Text = String.Format("{0}/{1} KBytes", serverstatistics.nVideoCaptureBytesRX / 1024, serverstatistics.nVideoCaptureBytesTX / 1024);
            mediafileTextBox.Text = String.Format("{0}/{1} KBytes", serverstatistics.nMediaFileBytesRX / 1024, serverstatistics.nMediaFileBytesTX / 1024);
        }

        private void updButton_Click(object sender, EventArgs e)
        {
            cmd_id = ttclient.DoQueryServerStats();
        }
    }
}
