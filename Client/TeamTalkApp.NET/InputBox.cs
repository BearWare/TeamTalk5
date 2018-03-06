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

namespace TeamTalkApp.NET
{
    public partial class InputBox : Form
    {
        private InputBox()
        {
            InitializeComponent();
        }

        private void InputBox_Load(object sender, EventArgs e)
        {
            this.CenterToScreen();
        }

        public static string Get(string caption, string label)
        {
            InputBox dlg = new InputBox();
            dlg.Text = caption;
            dlg.label1.Text = label;
            if (dlg.ShowDialog() == DialogResult.OK)
                return dlg.textBox1.Text;
            return "";
        }

        public static string Get(string caption, string label, string input)
        {
            InputBox dlg = new InputBox();
            dlg.Text = caption;
            dlg.label1.Text = label;
            dlg.textBox1.Text = input;
            if (dlg.ShowDialog() == DialogResult.OK)
                return dlg.textBox1.Text;
            return "";
        }
    }
}
