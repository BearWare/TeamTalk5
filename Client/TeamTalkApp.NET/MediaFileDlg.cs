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
    public partial class MediaFileDlg : Form
    {
        private TeamTalkBase ttclient;
        MediaFileInfo info;

        public MediaFileDlg(TeamTalkBase ttclient)
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

            if (TeamTalkBase.GetMediaFileInfo(openFileDialog1.FileName, ref info))
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
