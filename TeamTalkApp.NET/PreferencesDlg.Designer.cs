namespace TeamTalkApp.NET
{
    partial class PreferencesDlg
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
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.wasapiRadioButton = new System.Windows.Forms.RadioButton();
            this.echocancelCheckBox = new System.Windows.Forms.CheckBox();
            this.duplexCheckBox = new System.Windows.Forms.CheckBox();
            this.sndTestCheckBox = new System.Windows.Forms.CheckBox();
            this.winmmRadioButton = new System.Windows.Forms.RadioButton();
            this.dsoundRadioButton = new System.Windows.Forms.RadioButton();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.sndinputComboBox = new System.Windows.Forms.ComboBox();
            this.sndoutputComboBox = new System.Windows.Forms.ComboBox();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.label10 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.vidbitrateNumericUpDown = new System.Windows.Forms.NumericUpDown();
            this.vidcodecComboBox = new System.Windows.Forms.ComboBox();
            this.label5 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.formatComboBox = new System.Windows.Forms.ComboBox();
            this.viddevComboBox = new System.Windows.Forms.ComboBox();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.tabPage4 = new System.Windows.Forms.TabPage();
            this.groupBox6 = new System.Windows.Forms.GroupBox();
            this.fwCheckBox = new System.Windows.Forms.CheckBox();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.vidbitrateNumericUpDown)).BeginInit();
            this.groupBox2.SuspendLayout();
            this.tabPage4.SuspendLayout();
            this.groupBox6.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Controls.Add(this.tabPage4);
            this.tabControl1.Location = new System.Drawing.Point(8, 7);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(333, 358);
            this.tabControl1.TabIndex = 0;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.groupBox1);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(325, 332);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Sound";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.wasapiRadioButton);
            this.groupBox1.Controls.Add(this.echocancelCheckBox);
            this.groupBox1.Controls.Add(this.duplexCheckBox);
            this.groupBox1.Controls.Add(this.sndTestCheckBox);
            this.groupBox1.Controls.Add(this.winmmRadioButton);
            this.groupBox1.Controls.Add(this.dsoundRadioButton);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.sndinputComboBox);
            this.groupBox1.Controls.Add(this.sndoutputComboBox);
            this.groupBox1.Location = new System.Drawing.Point(14, 13);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(300, 251);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Sound devices";
            // 
            // wasapiRadioButton
            // 
            this.wasapiRadioButton.AutoSize = true;
            this.wasapiRadioButton.Checked = true;
            this.wasapiRadioButton.Location = new System.Drawing.Point(17, 31);
            this.wasapiRadioButton.Name = "wasapiRadioButton";
            this.wasapiRadioButton.Size = new System.Drawing.Size(67, 17);
            this.wasapiRadioButton.TabIndex = 0;
            this.wasapiRadioButton.TabStop = true;
            this.wasapiRadioButton.Text = "WASAPI";
            this.wasapiRadioButton.UseVisualStyleBackColor = true;
            this.wasapiRadioButton.Click += new System.EventHandler(this.UpdateSoundSystem);
            // 
            // echocancelCheckBox
            // 
            this.echocancelCheckBox.AutoSize = true;
            this.echocancelCheckBox.Enabled = false;
            this.echocancelCheckBox.Location = new System.Drawing.Point(17, 147);
            this.echocancelCheckBox.Name = "echocancelCheckBox";
            this.echocancelCheckBox.Size = new System.Drawing.Size(286, 17);
            this.echocancelCheckBox.TabIndex = 8;
            this.echocancelCheckBox.Text = "Enable echo cancellation (remove echo from speakers)";
            this.echocancelCheckBox.UseVisualStyleBackColor = true;
            // 
            // duplexCheckBox
            // 
            this.duplexCheckBox.AutoSize = true;
            this.duplexCheckBox.Location = new System.Drawing.Point(17, 124);
            this.duplexCheckBox.Name = "duplexCheckBox";
            this.duplexCheckBox.Size = new System.Drawing.Size(271, 17);
            this.duplexCheckBox.TabIndex = 7;
            this.duplexCheckBox.Text = "Enable duplex mode (required for echo cancellation)";
            this.duplexCheckBox.UseVisualStyleBackColor = true;
            this.duplexCheckBox.CheckedChanged += new System.EventHandler(this.duplexCheckBox_CheckedChanged);
            // 
            // sndTestCheckBox
            // 
            this.sndTestCheckBox.Appearance = System.Windows.Forms.Appearance.Button;
            this.sndTestCheckBox.AutoSize = true;
            this.sndTestCheckBox.Location = new System.Drawing.Point(101, 213);
            this.sndTestCheckBox.Name = "sndTestCheckBox";
            this.sndTestCheckBox.Size = new System.Drawing.Size(89, 23);
            this.sndTestCheckBox.TabIndex = 9;
            this.sndTestCheckBox.Text = "Loopback Test";
            this.sndTestCheckBox.UseVisualStyleBackColor = true;
            this.sndTestCheckBox.CheckedChanged += new System.EventHandler(this.sndTestCheckBox_CheckedChanged);
            // 
            // winmmRadioButton
            // 
            this.winmmRadioButton.AutoSize = true;
            this.winmmRadioButton.Location = new System.Drawing.Point(180, 31);
            this.winmmRadioButton.Name = "winmmRadioButton";
            this.winmmRadioButton.Size = new System.Drawing.Size(90, 17);
            this.winmmRadioButton.TabIndex = 2;
            this.winmmRadioButton.Text = "Windows MM";
            this.winmmRadioButton.UseVisualStyleBackColor = true;
            this.winmmRadioButton.Click += new System.EventHandler(this.UpdateSoundSystem);
            // 
            // dsoundRadioButton
            // 
            this.dsoundRadioButton.AutoSize = true;
            this.dsoundRadioButton.Location = new System.Drawing.Point(90, 31);
            this.dsoundRadioButton.Name = "dsoundRadioButton";
            this.dsoundRadioButton.Size = new System.Drawing.Size(84, 17);
            this.dsoundRadioButton.TabIndex = 1;
            this.dsoundRadioButton.Text = "DirectSound";
            this.dsoundRadioButton.UseVisualStyleBackColor = true;
            this.dsoundRadioButton.Click += new System.EventHandler(this.UpdateSoundSystem);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(14, 62);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(71, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "Input devices";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(14, 94);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(79, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "Output devices";
            // 
            // sndinputComboBox
            // 
            this.sndinputComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.sndinputComboBox.FormattingEnabled = true;
            this.sndinputComboBox.Location = new System.Drawing.Point(101, 59);
            this.sndinputComboBox.Name = "sndinputComboBox";
            this.sndinputComboBox.Size = new System.Drawing.Size(179, 21);
            this.sndinputComboBox.TabIndex = 4;
            this.sndinputComboBox.SelectedIndexChanged += new System.EventHandler(this.UpdateSelectedSoundDevices);
            // 
            // sndoutputComboBox
            // 
            this.sndoutputComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.sndoutputComboBox.FormattingEnabled = true;
            this.sndoutputComboBox.Location = new System.Drawing.Point(101, 86);
            this.sndoutputComboBox.Name = "sndoutputComboBox";
            this.sndoutputComboBox.Size = new System.Drawing.Size(179, 21);
            this.sndoutputComboBox.TabIndex = 6;
            this.sndoutputComboBox.SelectedIndexChanged += new System.EventHandler(this.UpdateSelectedSoundDevices);
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.groupBox3);
            this.tabPage2.Controls.Add(this.groupBox2);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(325, 332);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Video";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.label10);
            this.groupBox3.Controls.Add(this.label7);
            this.groupBox3.Controls.Add(this.vidbitrateNumericUpDown);
            this.groupBox3.Controls.Add(this.vidcodecComboBox);
            this.groupBox3.Controls.Add(this.label5);
            this.groupBox3.Location = new System.Drawing.Point(6, 99);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(313, 89);
            this.groupBox3.TabIndex = 1;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Video Codec";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(245, 59);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(47, 13);
            this.label10.TabIndex = 6;
            this.label10.Text = "0 = VBR";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(11, 59);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(78, 13);
            this.label7.TabIndex = 5;
            this.label7.Text = "Bit Rate (KBits)";
            // 
            // vidbitrateNumericUpDown
            // 
            this.vidbitrateNumericUpDown.Location = new System.Drawing.Point(123, 55);
            this.vidbitrateNumericUpDown.Maximum = new decimal(new int[] {
            8192,
            0,
            0,
            0});
            this.vidbitrateNumericUpDown.Name = "vidbitrateNumericUpDown";
            this.vidbitrateNumericUpDown.Size = new System.Drawing.Size(120, 20);
            this.vidbitrateNumericUpDown.TabIndex = 4;
            // 
            // vidcodecComboBox
            // 
            this.vidcodecComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.vidcodecComboBox.FormattingEnabled = true;
            this.vidcodecComboBox.Items.AddRange(new object[] {
            "WebM VP8"});
            this.vidcodecComboBox.Location = new System.Drawing.Point(123, 25);
            this.vidcodecComboBox.Name = "vidcodecComboBox";
            this.vidcodecComboBox.Size = new System.Drawing.Size(154, 21);
            this.vidcodecComboBox.TabIndex = 1;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(11, 29);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(38, 13);
            this.label5.TabIndex = 0;
            this.label5.Text = "Codec";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.label4);
            this.groupBox2.Controls.Add(this.label3);
            this.groupBox2.Controls.Add(this.formatComboBox);
            this.groupBox2.Controls.Add(this.viddevComboBox);
            this.groupBox2.Location = new System.Drawing.Point(6, 6);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(313, 87);
            this.groupBox2.TabIndex = 0;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Video Devices";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(6, 53);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(79, 13);
            this.label4.TabIndex = 3;
            this.label4.Text = "Capture Format";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(6, 22);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(111, 13);
            this.label3.TabIndex = 2;
            this.label3.Text = "Video Capture Device";
            // 
            // formatComboBox
            // 
            this.formatComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.formatComboBox.FormattingEnabled = true;
            this.formatComboBox.Location = new System.Drawing.Point(123, 50);
            this.formatComboBox.Name = "formatComboBox";
            this.formatComboBox.Size = new System.Drawing.Size(184, 21);
            this.formatComboBox.TabIndex = 1;
            // 
            // viddevComboBox
            // 
            this.viddevComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.viddevComboBox.FormattingEnabled = true;
            this.viddevComboBox.Location = new System.Drawing.Point(123, 19);
            this.viddevComboBox.Name = "viddevComboBox";
            this.viddevComboBox.Size = new System.Drawing.Size(184, 21);
            this.viddevComboBox.TabIndex = 0;
            this.viddevComboBox.SelectedIndexChanged += new System.EventHandler(this.viddevComboBox_SelectedIndexChanged);
            // 
            // tabPage3
            // 
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage3.Size = new System.Drawing.Size(325, 332);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Connection";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // tabPage4
            // 
            this.tabPage4.Controls.Add(this.groupBox6);
            this.tabPage4.Location = new System.Drawing.Point(4, 22);
            this.tabPage4.Name = "tabPage4";
            this.tabPage4.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage4.Size = new System.Drawing.Size(325, 332);
            this.tabPage4.TabIndex = 3;
            this.tabPage4.Text = "Advanced";
            this.tabPage4.UseVisualStyleBackColor = true;
            // 
            // groupBox6
            // 
            this.groupBox6.Controls.Add(this.fwCheckBox);
            this.groupBox6.Location = new System.Drawing.Point(6, 6);
            this.groupBox6.Name = "groupBox6";
            this.groupBox6.Size = new System.Drawing.Size(313, 56);
            this.groupBox6.TabIndex = 0;
            this.groupBox6.TabStop = false;
            this.groupBox6.Text = "Windows Firewall";
            // 
            // fwCheckBox
            // 
            this.fwCheckBox.AutoSize = true;
            this.fwCheckBox.Location = new System.Drawing.Point(6, 24);
            this.fwCheckBox.Name = "fwCheckBox";
            this.fwCheckBox.Size = new System.Drawing.Size(175, 17);
            this.fwCheckBox.TabIndex = 0;
            this.fwCheckBox.Text = "Add application to exception list";
            this.fwCheckBox.UseVisualStyleBackColor = true;
            // 
            // button1
            // 
            this.button1.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.button1.Location = new System.Drawing.Point(266, 371);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 1;
            this.button1.Text = "OK";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.button2.Location = new System.Drawing.Point(8, 371);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 2;
            this.button2.Text = "Cancel";
            this.button2.UseVisualStyleBackColor = true;
            // 
            // PreferencesDlg
            // 
            this.AcceptButton = this.button1;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.button2;
            this.ClientSize = new System.Drawing.Size(351, 398);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.tabControl1);
            this.Name = "PreferencesDlg";
            this.Text = "Preferences";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.PreferencesDlg_FormClosing);
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.tabPage2.ResumeLayout(false);
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.vidbitrateNumericUpDown)).EndInit();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.tabPage4.ResumeLayout(false);
            this.groupBox6.ResumeLayout(false);
            this.groupBox6.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RadioButton winmmRadioButton;
        private System.Windows.Forms.RadioButton dsoundRadioButton;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox sndinputComboBox;
        private System.Windows.Forms.ComboBox sndoutputComboBox;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.CheckBox sndTestCheckBox;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ComboBox formatComboBox;
        private System.Windows.Forms.ComboBox viddevComboBox;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.ComboBox vidcodecComboBox;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.NumericUpDown vidbitrateNumericUpDown;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.CheckBox echocancelCheckBox;
        private System.Windows.Forms.CheckBox duplexCheckBox;
        private System.Windows.Forms.TabPage tabPage4;
        private System.Windows.Forms.GroupBox groupBox6;
        private System.Windows.Forms.CheckBox fwCheckBox;
        private System.Windows.Forms.RadioButton wasapiRadioButton;
    }
}