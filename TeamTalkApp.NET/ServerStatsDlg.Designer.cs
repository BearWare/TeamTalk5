namespace TeamTalkApp.NET
{
    partial class ServerStatsDlg
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
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.totalTextBox = new System.Windows.Forms.TextBox();
            this.voiceTextBox = new System.Windows.Forms.TextBox();
            this.videoTextBox = new System.Windows.Forms.TextBox();
            this.updButton = new System.Windows.Forms.Button();
            this.mediafileTextBox = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(68, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Total RX/TX";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 42);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(71, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Voice RX/TX";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 75);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(111, 13);
            this.label3.TabIndex = 2;
            this.label3.Text = "Video Capture RX/TX";
            // 
            // totalTextBox
            // 
            this.totalTextBox.Location = new System.Drawing.Point(154, 6);
            this.totalTextBox.Name = "totalTextBox";
            this.totalTextBox.Size = new System.Drawing.Size(100, 20);
            this.totalTextBox.TabIndex = 3;
            // 
            // voiceTextBox
            // 
            this.voiceTextBox.Location = new System.Drawing.Point(154, 39);
            this.voiceTextBox.Name = "voiceTextBox";
            this.voiceTextBox.Size = new System.Drawing.Size(100, 20);
            this.voiceTextBox.TabIndex = 4;
            // 
            // videoTextBox
            // 
            this.videoTextBox.Location = new System.Drawing.Point(154, 72);
            this.videoTextBox.Name = "videoTextBox";
            this.videoTextBox.Size = new System.Drawing.Size(100, 20);
            this.videoTextBox.TabIndex = 5;
            // 
            // updButton
            // 
            this.updButton.Location = new System.Drawing.Point(92, 178);
            this.updButton.Name = "updButton";
            this.updButton.Size = new System.Drawing.Size(75, 23);
            this.updButton.TabIndex = 6;
            this.updButton.Text = "Update";
            this.updButton.UseVisualStyleBackColor = true;
            this.updButton.Click += new System.EventHandler(this.updButton_Click);
            // 
            // mediafileTextBox
            // 
            this.mediafileTextBox.Location = new System.Drawing.Point(154, 105);
            this.mediafileTextBox.Name = "mediafileTextBox";
            this.mediafileTextBox.Size = new System.Drawing.Size(100, 20);
            this.mediafileTextBox.TabIndex = 8;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(12, 108);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(92, 13);
            this.label4.TabIndex = 7;
            this.label4.Text = "Media File RX/TX";
            // 
            // ServerStatsDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(266, 213);
            this.Controls.Add(this.mediafileTextBox);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.updButton);
            this.Controls.Add(this.videoTextBox);
            this.Controls.Add(this.voiceTextBox);
            this.Controls.Add(this.totalTextBox);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Name = "ServerStatsDlg";
            this.Text = "Server Statistics";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox totalTextBox;
        private System.Windows.Forms.TextBox voiceTextBox;
        private System.Windows.Forms.TextBox videoTextBox;
        private System.Windows.Forms.Button updButton;
        private System.Windows.Forms.TextBox mediafileTextBox;
        private System.Windows.Forms.Label label4;
    }
}