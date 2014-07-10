using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using BearWare;

namespace TeamTalkApp.NET
{
    struct AudioFormat
    {
        public AudioFileFormat aff;
        public string name;
        public AudioFormat(AudioFileFormat aff, string name)
        {
            this.aff = aff; this.name = name;
        }
        public override string ToString()
        {
            return name;
        }
    }

    public partial class AudioStorageDlg : Form
    {
        Settings settings;

        public AudioStorageDlg(Settings settings)
        {
            this.settings = settings;

            InitializeComponent();
            this.CenterToScreen();

            fileformatComboBox.Items.Add(new AudioFormat(AudioFileFormat.AFF_WAVE_FORMAT, "Wave format"));
            fileformatComboBox.Items.Add(new AudioFormat(AudioFileFormat.AFF_MP3_64KBIT_FORMAT, "MP3 format (64kbit)"));
            fileformatComboBox.Items.Add(new AudioFormat(AudioFileFormat.AFF_MP3_128KBIT_FORMAT, "MP3 format (128kbit)"));
            fileformatComboBox.Items.Add(new AudioFormat(AudioFileFormat.AFF_MP3_256KBIT_FORMAT, "MP3 format (256kbit)"));
            fileformatComboBox.SelectedIndex = 0;
        }

        private void button3_Click(object sender, EventArgs e)
        {
            if (folderBrowserDialog1.ShowDialog() == DialogResult.OK)
                folderpathTextBox.Text = folderBrowserDialog1.SelectedPath;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (!Directory.Exists(folderpathTextBox.Text))
            {
                MessageBox.Show("Folder for audio storage doesn't exist", "Folder for audio files");
                return;
            }
            AudioFileFormat aff = ((AudioFormat)fileformatComboBox.SelectedItem).aff;
            settings.aff = aff;
            settings.audiofolder = folderpathTextBox.Text;
            settings.muxed_audio_file = muxedRadioButton.Checked;
        }

        private void fileformatComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            AudioFileFormat aff = ((AudioFormat)fileformatComboBox.SelectedItem).aff;
            if (!File.Exists("lame_enc.dll"))
            {
                switch (aff)
                {
                    case AudioFileFormat.AFF_MP3_64KBIT_FORMAT:
                    case AudioFileFormat.AFF_MP3_128KBIT_FORMAT:
                    case AudioFileFormat.AFF_MP3_256KBIT_FORMAT:
                        MessageBox.Show("DLL file lame_enc.dll is required for MP3 encoding");
                        fileformatComboBox.SelectedIndex = 0;
                        break;
                }
            }
        }
    }
}