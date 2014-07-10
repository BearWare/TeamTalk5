namespace TeamTalkApp.NET
{
    partial class ConnectDlg
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.cryptCheckBox = new System.Windows.Forms.CheckBox();
            this.ipaddrComboBox = new System.Windows.Forms.ComboBox();
            this.udpportNumericUpDown = new System.Windows.Forms.NumericUpDown();
            this.tcpportNumericUpDown = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.serversListBox = new System.Windows.Forms.ListBox();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.udpportNumericUpDown)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.tcpportNumericUpDown)).BeginInit();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // button1
            // 
            this.button1.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.button1.Location = new System.Drawing.Point(211, 279);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 6;
            this.button1.Text = "OK";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.button2.Location = new System.Drawing.Point(18, 279);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 7;
            this.button2.Text = "Cancel";
            this.button2.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.cryptCheckBox);
            this.groupBox1.Controls.Add(this.ipaddrComboBox);
            this.groupBox1.Controls.Add(this.udpportNumericUpDown);
            this.groupBox1.Controls.Add(this.tcpportNumericUpDown);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Location = new System.Drawing.Point(12, 140);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(274, 133);
            this.groupBox1.TabIndex = 11;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Connect to specific server";
            // 
            // cryptCheckBox
            // 
            this.cryptCheckBox.AutoSize = true;
            this.cryptCheckBox.Location = new System.Drawing.Point(92, 99);
            this.cryptCheckBox.Name = "cryptCheckBox";
            this.cryptCheckBox.Size = new System.Drawing.Size(106, 17);
            this.cryptCheckBox.TabIndex = 12;
            this.cryptCheckBox.Text = "Encrypted server";
            this.cryptCheckBox.UseVisualStyleBackColor = true;
            // 
            // ipaddrComboBox
            // 
            this.ipaddrComboBox.FormattingEnabled = true;
            this.ipaddrComboBox.Items.AddRange(new object[] {
            "localhost",
            "192.168.1.110"});
            this.ipaddrComboBox.Location = new System.Drawing.Point(92, 19);
            this.ipaddrComboBox.Name = "ipaddrComboBox";
            this.ipaddrComboBox.Size = new System.Drawing.Size(145, 21);
            this.ipaddrComboBox.TabIndex = 7;
            // 
            // udpportNumericUpDown
            // 
            this.udpportNumericUpDown.Location = new System.Drawing.Point(92, 73);
            this.udpportNumericUpDown.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.udpportNumericUpDown.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.udpportNumericUpDown.Name = "udpportNumericUpDown";
            this.udpportNumericUpDown.Size = new System.Drawing.Size(63, 20);
            this.udpportNumericUpDown.TabIndex = 11;
            this.udpportNumericUpDown.Value = new decimal(new int[] {
            10333,
            0,
            0,
            0});
            // 
            // tcpportNumericUpDown
            // 
            this.tcpportNumericUpDown.Location = new System.Drawing.Point(92, 47);
            this.tcpportNumericUpDown.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.tcpportNumericUpDown.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.tcpportNumericUpDown.Name = "tcpportNumericUpDown";
            this.tcpportNumericUpDown.Size = new System.Drawing.Size(63, 20);
            this.tcpportNumericUpDown.TabIndex = 9;
            this.tcpportNumericUpDown.Value = new decimal(new int[] {
            10333,
            0,
            0,
            0});
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(19, 76);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(51, 13);
            this.label3.TabIndex = 10;
            this.label3.Text = "UDP-port";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(19, 49);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(49, 13);
            this.label2.TabIndex = 8;
            this.label2.Text = "TCP-port";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(19, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(57, 13);
            this.label1.TabIndex = 6;
            this.label1.Text = "IP-address";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.serversListBox);
            this.groupBox2.Location = new System.Drawing.Point(12, 11);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(274, 120);
            this.groupBox2.TabIndex = 12;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Public TeamTalk servers";
            // 
            // serversListBox
            // 
            this.serversListBox.FormattingEnabled = true;
            this.serversListBox.Location = new System.Drawing.Point(6, 19);
            this.serversListBox.Name = "serversListBox";
            this.serversListBox.Size = new System.Drawing.Size(262, 95);
            this.serversListBox.TabIndex = 9;
            this.serversListBox.SelectedIndexChanged += new System.EventHandler(this.serversListBox_SelectedIndexChanged);
            // 
            // ConnectDlg
            // 
            this.AcceptButton = this.button1;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.button2;
            this.ClientSize = new System.Drawing.Size(298, 314);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Name = "ConnectDlg";
            this.Text = "Connect";
            this.Load += new System.EventHandler(this.ConnectDlg_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.udpportNumericUpDown)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.tcpportNumericUpDown)).EndInit();
            this.groupBox2.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.ComboBox ipaddrComboBox;
        private System.Windows.Forms.NumericUpDown udpportNumericUpDown;
        private System.Windows.Forms.NumericUpDown tcpportNumericUpDown;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.ListBox serversListBox;
        private System.Windows.Forms.CheckBox cryptCheckBox;
    }
}