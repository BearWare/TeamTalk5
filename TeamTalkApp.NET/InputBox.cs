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