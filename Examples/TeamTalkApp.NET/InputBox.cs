/*
 * Copyright (c) 2005-2016, BearWare.dk
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
