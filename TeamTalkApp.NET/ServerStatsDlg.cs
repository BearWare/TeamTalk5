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