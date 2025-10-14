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
using System.Diagnostics;
using BearWare;

namespace TeamTalkApp.NET
{
    struct ItemData
    {
        public string text;
        public int id;
        public ItemData(string text, int id)
        {
            this.text = text;
            this.id = id;
        }

        public override string ToString() 
        {
            return text;
        }
    }

    public partial class PreferencesDlg : Form
    {
        TeamTalkBase ttclient;
        Settings settings;
        VideoCaptureDevice[] videodevs;
        IntPtr soundloop;

        public PreferencesDlg(TeamTalkBase tt, Settings settings)
        {
            ttclient = tt;
            this.settings = settings;
            InitializeComponent();
            this.CenterToScreen();

            dsoundRadioButton.Checked = settings.soundsystem == SoundSystem.SOUNDSYSTEM_DSOUND;
            winmmRadioButton.Checked = settings.soundsystem == SoundSystem.SOUNDSYSTEM_WINMM;

            UpdateSoundSystem(null, null);

            vidcodecComboBox.SelectedIndex = 0;
            this.vidbitrateNumericUpDown.Value = settings.codec.webm_vp8.nRcTargetBitrate;
            UpdateVideoCaptureDevices();

            fwCheckBox.Checked = WindowsFirewall.AppExceptionExists(Application.ExecutablePath);
        }

        void UpdateSoundSystem(object sender, EventArgs e)
        {
            sndinputComboBox.Items.Clear();
            sndoutputComboBox.Items.Clear();

            SoundDevice[] devs;
            TeamTalkBase.GetSoundDevices(out devs);

            SoundSystem soundsystem = SoundSystem.SOUNDSYSTEM_WASAPI;
            if (dsoundRadioButton.Checked)
            {
                soundsystem = SoundSystem.SOUNDSYSTEM_DSOUND;
                Debug.WriteLine("DirectSound devices");
            }
            else if (winmmRadioButton.Checked)
            {
                soundsystem = SoundSystem.SOUNDSYSTEM_WINMM;
                Debug.WriteLine("WinMM devices");
            }
            else
                Debug.WriteLine("WASAPI devices");
            
            Debug.WriteLine("INPUT DEVICES");
            foreach (SoundDevice dev in devs)
            {
                if (dev.nSoundSystem != soundsystem)
                    continue;
                Debug.WriteLine("Name " + dev.szDeviceName);
                Debug.WriteLine("\tID #" + dev.nDeviceID);
                Debug.WriteLine("\tUnique ID #" + dev.szDeviceID);
                Debug.WriteLine("\tWaveDeviceID #" + dev.nWaveDeviceID);
                string tmp = "";
                if (WindowsMixer.GetWaveInName(dev.nWaveDeviceID, ref tmp))
                    Debug.WriteLine("\tMixer name: " + tmp);
                for (int i = 0; i < WindowsMixer.GetWaveInControlCount(dev.nWaveDeviceID); i++)
                    if (WindowsMixer.GetWaveInControlName(dev.nWaveDeviceID, i, ref tmp))
                    {
                        Debug.WriteLine("\t\tControl name: " + tmp);
                        Debug.WriteLine("\t\tSelected: " + WindowsMixer.GetWaveInControlSelected(dev.nWaveDeviceID, i));
                    }

                if (dev.nMaxInputChannels > 0)
                {
                    int index = sndinputComboBox.Items.Add(new ItemData(dev.szDeviceName, dev.nDeviceID));
                    if (dev.nDeviceID == settings.sndinputid)
                        sndinputComboBox.SelectedIndex = index;
                }
                if (dev.nMaxOutputChannels > 0)
                {
                    int index = sndoutputComboBox.Items.Add(new ItemData(dev.szDeviceName, dev.nDeviceID));
                    if (dev.nDeviceID == settings.sndoutputid)
                        sndoutputComboBox.SelectedIndex = index;
                }
            }
            if (sndinputComboBox.SelectedIndex < 0 && sndinputComboBox.Items.Count>0)
                sndinputComboBox.SelectedIndex = 0;
            if (sndoutputComboBox.SelectedIndex < 0 && sndoutputComboBox.Items.Count > 0)
                sndoutputComboBox.SelectedIndex = 0;
        }

        private void UpdateSelectedSoundDevices(object sender, EventArgs e)
        {
            if (sndinputComboBox.SelectedItem == null ||
               sndoutputComboBox.SelectedItem == null)
                return;

            int inputid = ((ItemData)sndinputComboBox.SelectedItem).id;
            int outputid = ((ItemData)sndoutputComboBox.SelectedItem).id;
            
            SoundDevice[] devs;
            TeamTalkBase.GetSoundDevices(out devs);

            int in_samplerate = 0, out_samplerate = 0;
            foreach (SoundDevice dev in devs)
            {
                if(dev.nDeviceID == inputid)
                    in_samplerate = dev.nDefaultSampleRate;
                if (dev.nDeviceID == outputid)
                    out_samplerate = dev.nDefaultSampleRate;
            }

            //for duplex mode both input and output sound device must support the same sample rate
            duplexCheckBox.Enabled = in_samplerate == out_samplerate;
            if (in_samplerate != out_samplerate)
            {
                duplexCheckBox.Checked = false;
                echocancelCheckBox.Checked = false;
            }
            echocancelCheckBox.Enabled = duplexCheckBox.Checked;
        }

        private void sndTestCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            ItemData input = (ItemData) sndinputComboBox.SelectedItem;
            ItemData output = (ItemData)sndoutputComboBox.SelectedItem;
            if (sndTestCheckBox.Checked)
            {
                //Extract input device and get its default samplerate.
                //WASAPI devices only support one sample rate so it's important to use the correct one.
                SoundDevice[] devs;
                TeamTalkBase.GetSoundDevices(out devs);

                int in_samplerate = 0;
                foreach (SoundDevice dev in devs)
                {
                    if (dev.nDeviceID == input.id)
                        in_samplerate = dev.nDefaultSampleRate;
                }

                SpeexDSP spxdsp = new SpeexDSP(true);
                spxdsp.bEnableAGC = true;
                spxdsp.bEnableDenoise = true;
                spxdsp.bEnableEchoCancellation = echocancelCheckBox.Checked;
                soundloop = TeamTalkBase.StartSoundLoopbackTest(input.id, output.id, in_samplerate, 1, duplexCheckBox.Checked, spxdsp);
                if (soundloop == IntPtr.Zero)
                {
                    MessageBox.Show("Failed to test selected device");
                    sndTestCheckBox.Checked = false;
                }
            }
            else
                TeamTalkBase.CloseSoundLoopbackTest(soundloop);
        }

        private void PreferencesDlg_FormClosing(object sender, FormClosingEventArgs e)
        {
            TeamTalkBase.CloseSoundLoopbackTest(soundloop);
        }

        private void UpdateVideoCaptureDevices()
        {
            if (!TeamTalkBase.GetVideoCaptureDevices(out videodevs) || videodevs.Length == 0)
                return;

            foreach (VideoCaptureDevice dev in videodevs)
            {
                int index = viddevComboBox.Items.Add(dev.szDeviceName);
                if (dev.szDeviceID == settings.videoid)
                    viddevComboBox.SelectedIndex = index;
            }
            if(viddevComboBox.SelectedIndex<0)
                viddevComboBox.SelectedIndex = 0;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            ClientFlag flags = ttclient.GetFlags();

            //Audio-tab
            if ((ttclient.Flags & ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX) ==
                ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX)
                ttclient.CloseSoundDuplexDevices();
            else
            {
                ttclient.CloseSoundInputDevice();
                ttclient.CloseSoundOutputDevice();
            }

            ItemData inputItem = (ItemData)sndinputComboBox.SelectedItem;
            ItemData outputItem = (ItemData)sndoutputComboBox.SelectedItem;
            settings.sndinputid = inputItem.id;
            settings.sndoutputid = outputItem.id;

            if (duplexCheckBox.Checked)
            {
                if (!ttclient.InitSoundDuplexDevices(settings.sndinputid, settings.sndoutputid))
                    MessageBox.Show("Failed to init sound devices");

                SpeexDSP spxdsp = new SpeexDSP(false);
                ttclient.GetSoundInputPreprocess(ref spxdsp);
                spxdsp.nEchoSuppress = SpeexDSPConstants.DEFAULT_ECHO_SUPPRESS;
                spxdsp.nEchoSuppressActive = SpeexDSPConstants.DEFAULT_ECHO_SUPPRESS_ACTIVE;
                spxdsp.bEnableEchoCancellation = echocancelCheckBox.Checked;
                ttclient.SetSoundInputPreprocess(spxdsp);
            }
            else
            {
                if (!ttclient.InitSoundInputDevice(settings.sndinputid))
                    MessageBox.Show("Failed to init sound input device");

                if (!ttclient.InitSoundOutputDevice(settings.sndoutputid))
                    MessageBox.Show("Failed to init sound output device");
            }

            if (wasapiRadioButton.Checked)
                settings.soundsystem = SoundSystem.SOUNDSYSTEM_WASAPI;
            else if (dsoundRadioButton.Checked)
                settings.soundsystem = SoundSystem.SOUNDSYSTEM_DSOUND;
            else if(winmmRadioButton.Checked)
                settings.soundsystem = SoundSystem.SOUNDSYSTEM_WINMM;

            //Video-tab
            if (viddevComboBox.Items.Count > 0)
            {
                VideoCodec codec = new VideoCodec();
                codec.nCodec = Codec.WEBM_VP8_CODEC;
                codec.webm_vp8.nRcTargetBitrate = (int)vidbitrateNumericUpDown.Value;
                codec.webm_vp8.nEncodeDeadline = WebMVP8CodecConstants.WEBM_VPX_DL_REALTIME;

                VideoFormat capformat = videodevs[viddevComboBox.SelectedIndex].videoFormats[formatComboBox.SelectedIndex];

                if (ttclient.Flags.HasFlag(ClientFlag.CLIENT_VIDEOCAPTURE_READY) &&
                   !(settings.videoid == videodevs[viddevComboBox.SelectedIndex].szDeviceID &&
                    Util.Equals(codec, settings.codec) &&
                    Util.Equals(capformat, settings.capformat)))
                    ttclient.CloseVideoCaptureDevice();

                settings.codec.nCodec = Codec.WEBM_VP8_CODEC;
                settings.codec.webm_vp8.nRcTargetBitrate = (int)vidbitrateNumericUpDown.Value;

                settings.videoid = videodevs[viddevComboBox.SelectedIndex].szDeviceID;
                settings.capformat = capformat;
                if (!ttclient.Flags.HasFlag(ClientFlag.CLIENT_VIDEOCAPTURE_READY))
                {
                    if(!ttclient.InitVideoCaptureDevice(settings.videoid, settings.capformat))
                        MessageBox.Show("Failed to initialize video capture device");
                }
            }

            //Advanced-tab
            if (fwCheckBox.Checked != WindowsFirewall.AppExceptionExists(Application.ExecutablePath))
            {
                if (fwCheckBox.Checked)
                    WindowsFirewall.AddAppException(Application.ProductName, Application.ExecutablePath);
                else
                    WindowsFirewall.RemoveAppException(Application.ExecutablePath);
            }
        }

        private void viddevComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            formatComboBox.Items.Clear();
            for (int i = 0; i < videodevs[viddevComboBox.SelectedIndex].nVideoFormatsCount;i++ )
            {
                VideoFormat cap = videodevs[viddevComboBox.SelectedIndex].videoFormats[i];
                int fps = cap.nFPS_Numerator / cap.nFPS_Denominator;
                string imgfmt = "";
                switch (cap.picFourCC)
                {
                    case FourCC.FOURCC_I420:
                        imgfmt = "I420"; break;
                    case FourCC.FOURCC_YUY2:
                        imgfmt = "YUY2"; break;
                    case FourCC.FOURCC_RGB32:
                        imgfmt = "RGB32"; break;
                }
                int index = formatComboBox.Items.Add(cap.nWidth + "x" + cap.nHeight + " FPS: " + fps + " " + imgfmt);
                if (Util.Equals(settings.capformat, cap))
                    formatComboBox.SelectedIndex = index;
            }
            if (formatComboBox.SelectedIndex < 0 && formatComboBox.Items.Count>0)
                formatComboBox.SelectedIndex = 0;
        }

        private void duplexCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            echocancelCheckBox.Enabled = duplexCheckBox.Checked;
        }
    }
}
