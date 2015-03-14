namespace TeamTalkApp.NET
{
    partial class DesktopShareDlg
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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.sharedesktopRadioButton = new System.Windows.Forms.RadioButton();
            this.shareactiveRadioButton = new System.Windows.Forms.RadioButton();
            this.sharespecificRadioButton = new System.Windows.Forms.RadioButton();
            this.windowsComboBox = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.rgbmodeComboBox = new System.Windows.Forms.ComboBox();
            this.updintervalCheckBox = new System.Windows.Forms.CheckBox();
            this.intervalNumericUpDown = new System.Windows.Forms.NumericUpDown();
            this.sharecursorCheckBox = new System.Windows.Forms.CheckBox();
            this.label2 = new System.Windows.Forms.Label();
            this.okButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.intervalNumericUpDown)).BeginInit();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.windowsComboBox);
            this.groupBox1.Controls.Add(this.sharespecificRadioButton);
            this.groupBox1.Controls.Add(this.shareactiveRadioButton);
            this.groupBox1.Controls.Add(this.sharedesktopRadioButton);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(366, 98);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Window to Share";
            // 
            // sharedesktopRadioButton
            // 
            this.sharedesktopRadioButton.AutoSize = true;
            this.sharedesktopRadioButton.Location = new System.Drawing.Point(6, 19);
            this.sharedesktopRadioButton.Name = "sharedesktopRadioButton";
            this.sharedesktopRadioButton.Size = new System.Drawing.Size(123, 17);
            this.sharedesktopRadioButton.TabIndex = 0;
            this.sharedesktopRadioButton.TabStop = true;
            this.sharedesktopRadioButton.Text = "Share entire desktop";
            this.sharedesktopRadioButton.UseVisualStyleBackColor = true;
            this.sharedesktopRadioButton.CheckedChanged += new System.EventHandler(this.sharedesktopRadioButton_CheckedChanged);
            // 
            // shareactiveRadioButton
            // 
            this.shareactiveRadioButton.AutoSize = true;
            this.shareactiveRadioButton.Location = new System.Drawing.Point(6, 42);
            this.shareactiveRadioButton.Name = "shareactiveRadioButton";
            this.shareactiveRadioButton.Size = new System.Drawing.Size(124, 17);
            this.shareactiveRadioButton.TabIndex = 1;
            this.shareactiveRadioButton.TabStop = true;
            this.shareactiveRadioButton.Text = "Share active window";
            this.shareactiveRadioButton.UseVisualStyleBackColor = true;
            this.shareactiveRadioButton.CheckedChanged += new System.EventHandler(this.shareactiveRadioButton_CheckedChanged);
            // 
            // sharespecificRadioButton
            // 
            this.sharespecificRadioButton.AutoSize = true;
            this.sharespecificRadioButton.Location = new System.Drawing.Point(6, 65);
            this.sharespecificRadioButton.Name = "sharespecificRadioButton";
            this.sharespecificRadioButton.Size = new System.Drawing.Size(131, 17);
            this.sharespecificRadioButton.TabIndex = 2;
            this.sharespecificRadioButton.TabStop = true;
            this.sharespecificRadioButton.Text = "Share specific window";
            this.sharespecificRadioButton.UseVisualStyleBackColor = true;
            this.sharespecificRadioButton.CheckedChanged += new System.EventHandler(this.sharespecificRadioButton_CheckedChanged);
            // 
            // windowsComboBox
            // 
            this.windowsComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.windowsComboBox.Enabled = false;
            this.windowsComboBox.FormattingEnabled = true;
            this.windowsComboBox.Location = new System.Drawing.Point(140, 65);
            this.windowsComboBox.Name = "windowsComboBox";
            this.windowsComboBox.Size = new System.Drawing.Size(220, 21);
            this.windowsComboBox.TabIndex = 3;
            this.windowsComboBox.SelectedIndexChanged += new System.EventHandler(this.windowsComboBox_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(15, 122);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(60, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Color mode";
            // 
            // rgbmodeComboBox
            // 
            this.rgbmodeComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.rgbmodeComboBox.FormattingEnabled = true;
            this.rgbmodeComboBox.Location = new System.Drawing.Point(81, 119);
            this.rgbmodeComboBox.Name = "rgbmodeComboBox";
            this.rgbmodeComboBox.Size = new System.Drawing.Size(146, 21);
            this.rgbmodeComboBox.TabIndex = 2;
            // 
            // updintervalCheckBox
            // 
            this.updintervalCheckBox.AutoSize = true;
            this.updintervalCheckBox.Location = new System.Drawing.Point(18, 146);
            this.updintervalCheckBox.Name = "updintervalCheckBox";
            this.updintervalCheckBox.Size = new System.Drawing.Size(98, 17);
            this.updintervalCheckBox.TabIndex = 3;
            this.updintervalCheckBox.Text = "Update interval";
            this.updintervalCheckBox.UseVisualStyleBackColor = true;
            this.updintervalCheckBox.CheckedChanged += new System.EventHandler(this.updintervalCheckBox_CheckedChanged);
            // 
            // intervalNumericUpDown
            // 
            this.intervalNumericUpDown.Enabled = false;
            this.intervalNumericUpDown.Increment = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.intervalNumericUpDown.Location = new System.Drawing.Point(136, 145);
            this.intervalNumericUpDown.Maximum = new decimal(new int[] {
            100000,
            0,
            0,
            0});
            this.intervalNumericUpDown.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.intervalNumericUpDown.Name = "intervalNumericUpDown";
            this.intervalNumericUpDown.Size = new System.Drawing.Size(91, 20);
            this.intervalNumericUpDown.TabIndex = 4;
            this.intervalNumericUpDown.Value = new decimal(new int[] {
            100,
            0,
            0,
            0});
            // 
            // sharecursorCheckBox
            // 
            this.sharecursorCheckBox.AutoSize = true;
            this.sharecursorCheckBox.Location = new System.Drawing.Point(18, 169);
            this.sharecursorCheckBox.Name = "sharecursorCheckBox";
            this.sharecursorCheckBox.Size = new System.Drawing.Size(127, 17);
            this.sharecursorCheckBox.TabIndex = 5;
            this.sharecursorCheckBox.Text = "Share desktop cursor";
            this.sharecursorCheckBox.UseVisualStyleBackColor = true;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(233, 150);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(32, 13);
            this.label2.TabIndex = 6;
            this.label2.Text = "msec";
            // 
            // okButton
            // 
            this.okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.okButton.Location = new System.Drawing.Point(303, 182);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(75, 23);
            this.okButton.TabIndex = 7;
            this.okButton.Text = "OK";
            this.okButton.UseVisualStyleBackColor = true;
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // cancelButton
            // 
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(222, 182);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 8;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.UseVisualStyleBackColor = true;
            // 
            // DesktopShareDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(390, 211);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.okButton);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.sharecursorCheckBox);
            this.Controls.Add(this.intervalNumericUpDown);
            this.Controls.Add(this.updintervalCheckBox);
            this.Controls.Add(this.rgbmodeComboBox);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.groupBox1);
            this.Name = "DesktopShareDlg";
            this.Text = "Desktop Sharing";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.intervalNumericUpDown)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.ComboBox windowsComboBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox rgbmodeComboBox;
        private System.Windows.Forms.CheckBox updintervalCheckBox;
        private System.Windows.Forms.NumericUpDown intervalNumericUpDown;
        private System.Windows.Forms.CheckBox sharecursorCheckBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
        public System.Windows.Forms.RadioButton sharedesktopRadioButton;
        public System.Windows.Forms.RadioButton sharespecificRadioButton;
        public System.Windows.Forms.RadioButton shareactiveRadioButton;
    }
}