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
    public partial class ServerStatsDlg : Form
    {
        TeamTalk ttclient;
        int cmd_id;

        public ServerStatsDlg(TeamTalk tt)
        {
            InitializeComponent();
            ttclient = tt;

            ttclient.OnCmdServerStatistics += new TeamTalk.ServerStats(ttclient_OnCmdServerStatistics);

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
