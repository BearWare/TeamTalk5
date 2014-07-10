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
    public partial class MediaFileDlg : Form
    {
        private TeamTalk ttclient;
        MediaFileInfo info;

        public MediaFileDlg(TeamTalk ttclient)
        {
            this.ttclient = ttclient;
            InitializeComponent();
        }

        private void openButton_Click(object sender, EventArgs e)
        {
            OpenFileDialog openFileDialog1 = new OpenFileDialog();

            openFileDialog1.RestoreDirectory = true;
            openFileDialog1.Filter = "All files (*.*)|*.*";
            if (openFileDialog1.ShowDialog() != DialogResult.OK)
                return;

            textBox1.Text = openFileDialog1.FileName;

            if (TeamTalk.GetMediaFileInfo(openFileDialog1.FileName, out info))
            {
                if (info.audioFmt.nAudioFmt != AudioFileFormat.AFF_NONE)
                    audioLabel.Text = String.Format("{0} Hz, {1}", info.audioFmt.nSampleRate,
                    (info.audioFmt.nChannels == 1 ? "Mono" : "Stereo"));
                else
                    audioLabel.Text = "Unknown";

                if (info.videoFmt.picFourCC != FourCC.FOURCC_NONE)
                {
                    videoLabel.Text = String.Format("{0}x{1} FPS: {2}",
                        info.videoFmt.nWidth, info.videoFmt.nHeight, info.videoFmt.nFPS_Numerator);
                }
                else
                    videoLabel.Text = "Unknown";
            }
            else
            {
                info = new MediaFileInfo();
            }
        }

        private void okButton_Click(object sender, EventArgs e)
        {
            VideoCodec vid = new VideoCodec();
            vid.nCodec = Codec.NO_CODEC;

            if (info.videoFmt.picFourCC != FourCC.FOURCC_NONE)
            {
                vid.nCodec = Codec.WEBM_VP8_CODEC;
                vid.webm_vp8.nRcTargetBitrate = 0;
            }
            if (!ttclient.StartStreamingMediaFileToChannel(textBox1.Text,
                                                           vid))
                MessageBox.Show("Failed to stream media file. Ensure media file can be played in WMP!");
        }
    }
}