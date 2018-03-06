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
    public partial class DesktopShareDlg : Form
    {
        List<IntPtr> windowHandles = new List<IntPtr>();

        public IntPtr hShareWnd = IntPtr.Zero;
        public int update_interval = 1000;
        public bool share_cursor = false;
        public BitmapFormat bmpformat = BitmapFormat.BMP_NONE;

        public DesktopShareDlg()
        {
            InitializeComponent();
            this.CenterToScreen();

            rgbmodeComboBox.Items.Add("8-bit colors");
            rgbmodeComboBox.Items.Add("16-bit colors");
            rgbmodeComboBox.Items.Add("24-bit colors");
            rgbmodeComboBox.Items.Add("32-bit colors");
            rgbmodeComboBox.SelectedIndex = 3;

            intervalNumericUpDown.Value = (decimal)update_interval;

            int i = 0;
            IntPtr ptr = IntPtr.Zero;
            while (WindowsHelper.GetDesktopWindowHWND(i, ref ptr))
            {
                ShareWindow wnd = new ShareWindow();
                if (WindowsHelper.GetWindow(ptr, ref wnd))
                {
                    windowsComboBox.Items.Add(wnd.szWindowTitle);
                    windowHandles.Add(wnd.hWnd);
                }
                i++;
            }
            if (i > 0)
                windowsComboBox.SelectedIndex = 0;
        }

        private void updintervalCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            intervalNumericUpDown.Enabled = updintervalCheckBox.Checked;
        }

        private void okButton_Click(object sender, EventArgs e)
        {
            update_interval = (int)intervalNumericUpDown.Value;
            share_cursor = shareactiveRadioButton.Checked;
            switch(rgbmodeComboBox.SelectedIndex)
            {
                case 0 :
                    bmpformat = BitmapFormat.BMP_RGB8_PALETTE;
                    break;
                case 1 :
                    bmpformat = BitmapFormat.BMP_RGB16_555;
                    break;
                case 2 :
                    bmpformat = BitmapFormat.BMP_RGB24;
                    break;
                case 3 :
                    bmpformat = BitmapFormat.BMP_RGB32;
                    break;
            }
            if (!updintervalCheckBox.Checked)
                update_interval = 0;
        }

        private void shareactiveRadioButton_CheckedChanged(object sender, EventArgs e)
        {
            hShareWnd = IntPtr.Zero;
        }

        private void sharedesktopRadioButton_CheckedChanged(object sender, EventArgs e)
        {
            hShareWnd = WindowsHelper.GetDesktopHWND();
        }

        private void sharespecificRadioButton_CheckedChanged(object sender, EventArgs e)
        {
            windowsComboBox.Enabled  = sharespecificRadioButton.Checked;
            hShareWnd = windowHandles[windowsComboBox.SelectedIndex];
        }

        private void windowsComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            hShareWnd = windowHandles[windowsComboBox.SelectedIndex];
        }
    }
}
