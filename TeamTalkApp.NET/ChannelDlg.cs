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
    public enum ChannelDlgType
    {
        JOIN_CHANNEL,
        CREATE_CHANNEL,
        UPDATE_CHANNEL,
        VIEW_CHANNEL
    }

    enum CodecTabs
    {
        SPEEX_CBR_TAB = 0,
        SPEEX_VBR_TAB,
        OPUS_TAB
    }

    public partial class ChannelDlg : Form
    {
        TeamTalk ttclient;
        ChannelDlgType dlgtype;
        Channel chan;

        public ChannelDlg(TeamTalk tt, ChannelDlgType dlgtype, int channelid, int parentid)
        {
            ttclient = tt;
            this.dlgtype = dlgtype;

            InitializeComponent();

            if (channelid > 0 && ttclient.GetChannel(channelid, ref chan))
            {
                string path = "";
                ttclient.GetChannelPath(channelid, ref path);
                chanpathTextBox.Text = path;
                channameTextBox.Text = chan.szName;
                topicTextBox.Text = chan.szTopic;
                passwdTextBox.Text = chan.szPassword;
                maxusersNumericUpDown.Value = chan.nMaxUsers;
                diskquotaNumericUpDown.Value = chan.nDiskQuota / 1024;
                permchannelCheckBox.Checked = chan.uChannelType.HasFlag(ChannelType.CHANNEL_PERMANENT);
                singletxCheckBox.Checked = chan.uChannelType.HasFlag(ChannelType.CHANNEL_SOLO_TRANSMIT);
                classroomCheckBox.Checked = chan.uChannelType.HasFlag(ChannelType.CHANNEL_CLASSROOM);
                oprecvonlyCheckBox.Checked = chan.uChannelType.HasFlag(ChannelType.CHANNEL_OPERATOR_RECVONLY);
                novoiceactCheckBox.Checked = chan.uChannelType.HasFlag(ChannelType.CHANNEL_NO_VOICEACTIVATION);
                norecordCheckBox.Checked = chan.uChannelType.HasFlag(ChannelType.CHANNEL_NO_RECORDING);

                switch (chan.audiocodec.nCodec)
                {
                    case Codec.SPEEX_CODEC:
                        tabControl1.SelectedIndex = (int)CodecTabs.SPEEX_CBR_TAB;
                        speexnbRadioButton.Checked = chan.audiocodec.speex.nBandmode == 0;
                        speexwbRadioButton.Checked = chan.audiocodec.speex.nBandmode == 1;
                        speexuwbRadioButton.Checked = chan.audiocodec.speex.nBandmode == 2;
                        speexqualityTrackBar.Value = chan.audiocodec.speex.nQuality;
                        speexTxInterval.Value = chan.audiocodec.speex.nTxIntervalMSec;
                        speexstereoCheckBox.Checked = chan.audiocodec.speex.bStereoPlayback;
                        break;
                    case Codec.SPEEX_VBR_CODEC :
                        tabControl1.SelectedIndex = (int)CodecTabs.SPEEX_VBR_TAB;
                        speexvbrnbRadioButton.Checked = chan.audiocodec.speex_vbr.nBandmode == 0;
                        speexvbrwbRadioButton.Checked = chan.audiocodec.speex_vbr.nBandmode == 1;
                        speexvbruwbRadioButton.Checked = chan.audiocodec.speex_vbr.nBandmode == 2;
                        speexvbrqualityTrackBar.Value = chan.audiocodec.speex_vbr.nQuality;
                        speexvbrbitrateNumericUpDown.Value = chan.audiocodec.speex_vbr.nBitRate;
                        speexvbrmaxbrNumericUpDown.Value = chan.audiocodec.speex_vbr.nMaxBitRate;
                        speexvbrTxInterval.Value = chan.audiocodec.speex_vbr.nTxIntervalMSec;
                        speexvbrdtxCheckBox.Checked = chan.audiocodec.speex_vbr.bDTX;
                        speexvbrstereoCheckBox.Checked = chan.audiocodec.speex_vbr.bStereoPlayback;
                        break;
                    case Codec.OPUS_CODEC:
                        tabControl1.SelectedIndex = (int)CodecTabs.OPUS_TAB;
                        int i = opussamplerateComboBox.FindString(chan.audiocodec.opus.nSampleRate.ToString());
                        if (i < 0)
                            i = 0;
                        opussamplerateComboBox.SelectedIndex = i;
                        opuschannelsComboBox.SelectedIndex = chan.audiocodec.opus.nChannels - 1;
                        switch (chan.audiocodec.opus.nApplication)
                        {
                            case OpusConstants.OPUS_APPLICATION_AUDIO :
                                opusaudioRadioButton.Checked = true;
                                break;
                            case OpusConstants.OPUS_APPLICATION_VOIP :
                                opusvoipRadioButton.Checked = true;
                                break;
                            default:
                                opusvoipRadioButton.Checked = true;
                                break;
                        }
                        opuscomplexityNumericUpDown.Value = chan.audiocodec.opus.nComplexity;
                        opusfecCheckBox.Checked = chan.audiocodec.opus.bFEC;
                        opusdtxCheckBox.Checked = chan.audiocodec.opus.bDTX;
                        opusvbrCheckBox.Checked = chan.audiocodec.opus.bVBR;
                        opusvbrconstraintCheckBox.Checked = chan.audiocodec.opus.bVBRConstraint;
                        opusbitrateNumericUpDown.Value = chan.audiocodec.opus.nBitRate / 1000;
                        opusTxIntervalNumericUpDown.Value = chan.audiocodec.opus.nTxIntervalMSec;
                        break;
                }

                agcCheckBox.Checked = chan.audiocfg.bEnableAGC;
                gainlevelTrackBar.Value = chan.audiocfg.nGainLevel;
            }
            else
            {
                string path = "";
                ttclient.GetChannelPath(parentid, ref path);
                chanpathTextBox.Text = path;
                chan.nParentID = parentid;
            }

            UserType usertype = ttclient.GetMyUserType();
            switch (dlgtype)
            {
                case ChannelDlgType.JOIN_CHANNEL :
                    //when joining a channel 'disk quota' and 'max users' are set to server's default values
                    diskquotaNumericUpDown.ReadOnly = true;
                    maxusersNumericUpDown.ReadOnly = true;
                    goto case ChannelDlgType.CREATE_CHANNEL;
                case ChannelDlgType.CREATE_CHANNEL :
                    opussamplerateComboBox.SelectedIndex = 0;
                    opuschannelsComboBox.SelectedIndex = 0;
                    opuschannelsComboBox.SelectedIndex = 0;
                    //only admins can change static channel property
                    permchannelCheckBox.Enabled = ttclient.UserRights.HasFlag(UserRight.USERRIGHT_MODIFY_CHANNELS);
                    diskquotaNumericUpDown.ReadOnly = !ttclient.UserRights.HasFlag(UserRight.USERRIGHT_MODIFY_CHANNELS);
                    agcCheckBox.Checked = true;
                    gainlevelTrackBar.Value = 8000;
                    break;
                case ChannelDlgType.UPDATE_CHANNEL :
                    break;
                case ChannelDlgType.VIEW_CHANNEL :
                    channameTextBox.ReadOnly = true;
                    topicTextBox.ReadOnly = true;
                    passwdTextBox.ReadOnly = true;
                    oppasswdTextBox.ReadOnly = true;
                    maxusersNumericUpDown.ReadOnly = true;
                    diskquotaNumericUpDown.ReadOnly = true;
                    permchannelCheckBox.Enabled = false;
                    singletxCheckBox.Enabled = false;
                    classroomCheckBox.Enabled = false;
                    oprecvonlyCheckBox.Enabled = false;
                    novoiceactCheckBox.Enabled = false;
                    norecordCheckBox.Enabled = false;
                    agcCheckBox.Enabled = false;
                    gainlevelTrackBar.Enabled = false;
                    tabControl1.Enabled = false;
                    break;
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            chan.szName = channameTextBox.Text;
            chan.szTopic = topicTextBox.Text;
            chan.szPassword = passwdTextBox.Text;
            chan.szOpPassword = oppasswdTextBox.Text;
            chan.nMaxUsers = (int)maxusersNumericUpDown.Value;
            chan.nDiskQuota = (long)diskquotaNumericUpDown.Value * 1024;

            switch(tabControl1.SelectedIndex)
            {
                case (int)CodecTabs.SPEEX_CBR_TAB:

                    chan.audiocodec.nCodec = Codec.SPEEX_CODEC;
                    if (speexnbRadioButton.Checked)
                        chan.audiocodec.speex.nBandmode = 0;
                    else if (speexwbRadioButton.Checked)
                        chan.audiocodec.speex.nBandmode = 1;
                    else if (speexuwbRadioButton.Checked)
                        chan.audiocodec.speex.nBandmode = 2;
                    chan.audiocodec.speex.nQuality = speexqualityTrackBar.Value;
                    chan.audiocodec.speex.nTxIntervalMSec = (int)speexTxInterval.Value;
                    chan.audiocodec.speex.bStereoPlayback = speexstereoCheckBox.Checked;
                    break;

                case (int)CodecTabs.SPEEX_VBR_TAB :

                    chan.audiocodec.nCodec = Codec.SPEEX_VBR_CODEC;
                    if (speexvbrnbRadioButton.Checked)
                        chan.audiocodec.speex_vbr.nBandmode = 0;
                    else if (speexvbrwbRadioButton.Checked)
                        chan.audiocodec.speex_vbr.nBandmode = 1;
                    else if (speexvbruwbRadioButton.Checked)
                        chan.audiocodec.speex_vbr.nBandmode = 2;
                    chan.audiocodec.speex_vbr.nQuality = speexvbrqualityTrackBar.Value;
                    chan.audiocodec.speex_vbr.nBitRate = (int)speexvbrbitrateNumericUpDown.Value;
                    chan.audiocodec.speex_vbr.nMaxBitRate = (int)speexvbrmaxbrNumericUpDown.Value;
                    chan.audiocodec.speex_vbr.bDTX = speexvbrdtxCheckBox.Checked;
                    chan.audiocodec.speex_vbr.nTxIntervalMSec = (int)speexvbrTxInterval.Value;
                    chan.audiocodec.speex_vbr.bStereoPlayback = speexvbrstereoCheckBox.Checked;
                    break;
                case (int)CodecTabs.OPUS_TAB :

                    chan.audiocodec.nCodec = Codec.OPUS_CODEC;
                    chan.audiocodec.opus.nSampleRate = int.Parse(opussamplerateComboBox.Text);
                    chan.audiocodec.opus.nBitRate = (int)opusbitrateNumericUpDown.Value * 1000;
                    chan.audiocodec.opus.nChannels = opuschannelsComboBox.SelectedIndex + 1;
                    if(opusvoipRadioButton.Checked)
                        chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_VOIP;
                    else
                        chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
                    chan.audiocodec.opus.nComplexity = (int)opuscomplexityNumericUpDown.Value;
                    chan.audiocodec.opus.bDTX = opusdtxCheckBox.Checked;
                    chan.audiocodec.opus.bFEC = opusfecCheckBox.Checked;
                    chan.audiocodec.opus.bVBR = opusvbrCheckBox.Checked;
                    chan.audiocodec.opus.bVBRConstraint = opusvbrconstraintCheckBox.Checked;
                    chan.audiocodec.opus.nTxIntervalMSec = (int)opusTxIntervalNumericUpDown.Value;
                    break;
            }
            chan.uChannelType |= permchannelCheckBox.Checked ? ChannelType.CHANNEL_PERMANENT : ChannelType.CHANNEL_DEFAULT;
            chan.uChannelType |= singletxCheckBox.Checked ? ChannelType.CHANNEL_SOLO_TRANSMIT : ChannelType.CHANNEL_DEFAULT;
            chan.uChannelType |= classroomCheckBox.Checked ? ChannelType.CHANNEL_CLASSROOM: ChannelType.CHANNEL_DEFAULT;
            chan.uChannelType |= oprecvonlyCheckBox.Checked ? ChannelType.CHANNEL_OPERATOR_RECVONLY : ChannelType.CHANNEL_DEFAULT;
            chan.uChannelType |= novoiceactCheckBox.Checked ? ChannelType.CHANNEL_NO_VOICEACTIVATION : ChannelType.CHANNEL_DEFAULT;
            chan.uChannelType |= norecordCheckBox.Checked ? ChannelType.CHANNEL_NO_RECORDING : ChannelType.CHANNEL_DEFAULT;


            chan.audiocfg.bEnableAGC = agcCheckBox.Checked;
            chan.audiocfg.nGainLevel = gainlevelTrackBar.Value;

            switch (dlgtype)
            {
                case ChannelDlgType.JOIN_CHANNEL :
                    if(ttclient.DoJoinChannel(chan)<0)
                        MessageBox.Show("Unable to join channel", "Join Channel");
                    //else wait for OnCmdChannelNew event
                    break;
                case ChannelDlgType.CREATE_CHANNEL :
                    if(ttclient.DoMakeChannel(chan)<0)
                        MessageBox.Show("Unable to create channel", "Create Channel");
                    //else wait for OnCmdChannelNew event
                    break;
                case ChannelDlgType.UPDATE_CHANNEL :
                    if(permchannelCheckBox.Checked)
                        chan.uChannelType |= ChannelType.CHANNEL_PERMANENT;
                    else
                        chan.uChannelType &= ~ChannelType.CHANNEL_PERMANENT;
                    if (singletxCheckBox.Checked)
                        chan.uChannelType |= ChannelType.CHANNEL_SOLO_TRANSMIT;
                    else
                        chan.uChannelType &= ~ChannelType.CHANNEL_SOLO_TRANSMIT;
                    if (classroomCheckBox.Checked)
                        chan.uChannelType |= ChannelType.CHANNEL_CLASSROOM;
                    else
                        chan.uChannelType &= ~ChannelType.CHANNEL_CLASSROOM;
                    if(oprecvonlyCheckBox.Checked)
                        chan.uChannelType |= ChannelType.CHANNEL_OPERATOR_RECVONLY;
                    else
                        chan.uChannelType &= ~ChannelType.CHANNEL_OPERATOR_RECVONLY;
                    if (novoiceactCheckBox.Checked)
                        chan.uChannelType |= ChannelType.CHANNEL_NO_VOICEACTIVATION;
                    else
                        chan.uChannelType &= ~ChannelType.CHANNEL_NO_VOICEACTIVATION;
                    if (norecordCheckBox.Checked)
                        chan.uChannelType |= ChannelType.CHANNEL_NO_RECORDING;
                    else
                        chan.uChannelType &= ~ChannelType.CHANNEL_NO_RECORDING;

                    if (ttclient.DoUpdateChannel(chan) < 0)
                        MessageBox.Show("Unable to update channel", "Update Channel");
                    break;
            }
        }

        private void channameTextBox_TextChanged(object sender, EventArgs e)
        {
            string path = "";
            ttclient.GetChannelPath(chan.nParentID, ref path);
            path += channameTextBox.Text;
            chanpathTextBox.Text = path;
        }

        private void gainlevelTrackBar_ValueChanged(object sender, EventArgs e)
        {
            label11.Text = gainlevelTrackBar.Value.ToString();
        }

        private void speexvbrCheckBox_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void label16_Click(object sender, EventArgs e)
        {

        }

        private void speexvbrTxInterval_ValueChanged(object sender, EventArgs e)
        {

        }

        private void speexvbrdtxCheckBox_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void label18_Click(object sender, EventArgs e)
        {

        }

        private void speexvbrmaxbrNumericUpDown_ValueChanged(object sender, EventArgs e)
        {

        }
    }
}

