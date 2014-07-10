namespace TeamTalkApp.NET
{
    partial class UserAccountsDlg
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
            this.components = new System.ComponentModel.Container();
            this.listView1 = new System.Windows.Forms.ListView();
            this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.deleteAccountToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.userrightsGroupBox = new System.Windows.Forms.GroupBox();
            this.audfilesCheckBox = new System.Windows.Forms.CheckBox();
            this.desktopinputCheckBox = new System.Windows.Forms.CheckBox();
            this.vidfileCheckBox = new System.Windows.Forms.CheckBox();
            this.srvupdCheckBox = new System.Windows.Forms.CheckBox();
            this.downloadCheckBox = new System.Windows.Forms.CheckBox();
            this.uploadCheckBox = new System.Windows.Forms.CheckBox();
            this.moveuserCheckBox = new System.Windows.Forms.CheckBox();
            this.banCheckBox = new System.Windows.Forms.CheckBox();
            this.kickCheckBox = new System.Windows.Forms.CheckBox();
            this.tempchanCheckBox = new System.Windows.Forms.CheckBox();
            this.desktopCheckBox = new System.Windows.Forms.CheckBox();
            this.multiloginCheckBox = new System.Windows.Forms.CheckBox();
            this.vidcapCheckBox = new System.Windows.Forms.CheckBox();
            this.voicetxCheckBox = new System.Windows.Forms.CheckBox();
            this.bcastCheckBox = new System.Windows.Forms.CheckBox();
            this.viewallCheckBox = new System.Windows.Forms.CheckBox();
            this.operatorenableCheckBox = new System.Windows.Forms.CheckBox();
            this.permchannelsCheckBox = new System.Windows.Forms.CheckBox();
            this.clrButton = new System.Windows.Forms.Button();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.delChanButton = new System.Windows.Forms.Button();
            this.addChanButton = new System.Windows.Forms.Button();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.selchanListBox = new System.Windows.Forms.ListBox();
            this.availchanListBox = new System.Windows.Forms.ListBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.initchanTextBox = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.noteTextBox = new System.Windows.Forms.TextBox();
            this.userdataTextBox = new System.Windows.Forms.TextBox();
            this.createButton = new System.Windows.Forms.Button();
            this.adminRadioButton = new System.Windows.Forms.RadioButton();
            this.defuserRadioButton = new System.Windows.Forms.RadioButton();
            this.passwordTextBox = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.usernameTextBox = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.label9 = new System.Windows.Forms.Label();
            this.bitrateNumericUpDown = new System.Windows.Forms.NumericUpDown();
            this.label8 = new System.Windows.Forms.Label();
            this.contextMenuStrip1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.userrightsGroupBox.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox4.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.bitrateNumericUpDown)).BeginInit();
            this.SuspendLayout();
            // 
            // listView1
            // 
            this.listView1.ContextMenuStrip = this.contextMenuStrip1;
            this.listView1.FullRowSelect = true;
            this.listView1.Location = new System.Drawing.Point(3, 2);
            this.listView1.Name = "listView1";
            this.listView1.Size = new System.Drawing.Size(244, 534);
            this.listView1.TabIndex = 0;
            this.listView1.UseCompatibleStateImageBehavior = false;
            this.listView1.View = System.Windows.Forms.View.Details;
            this.listView1.SelectedIndexChanged += new System.EventHandler(this.listView1_SelectedIndexChanged);
            // 
            // contextMenuStrip1
            // 
            this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.deleteAccountToolStripMenuItem});
            this.contextMenuStrip1.Name = "contextMenuStrip1";
            this.contextMenuStrip1.Size = new System.Drawing.Size(156, 26);
            // 
            // deleteAccountToolStripMenuItem
            // 
            this.deleteAccountToolStripMenuItem.Name = "deleteAccountToolStripMenuItem";
            this.deleteAccountToolStripMenuItem.Size = new System.Drawing.Size(155, 22);
            this.deleteAccountToolStripMenuItem.Text = "Delete Account";
            this.deleteAccountToolStripMenuItem.Click += new System.EventHandler(this.deleteAccountToolStripMenuItem_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.groupBox4);
            this.groupBox1.Controls.Add(this.userrightsGroupBox);
            this.groupBox1.Controls.Add(this.clrButton);
            this.groupBox1.Controls.Add(this.groupBox3);
            this.groupBox1.Controls.Add(this.groupBox2);
            this.groupBox1.Controls.Add(this.createButton);
            this.groupBox1.Controls.Add(this.adminRadioButton);
            this.groupBox1.Controls.Add(this.defuserRadioButton);
            this.groupBox1.Controls.Add(this.passwordTextBox);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.usernameTextBox);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Location = new System.Drawing.Point(253, 2);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(721, 534);
            this.groupBox1.TabIndex = 1;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "User Account Properties";
            // 
            // userrightsGroupBox
            // 
            this.userrightsGroupBox.Controls.Add(this.audfilesCheckBox);
            this.userrightsGroupBox.Controls.Add(this.desktopinputCheckBox);
            this.userrightsGroupBox.Controls.Add(this.vidfileCheckBox);
            this.userrightsGroupBox.Controls.Add(this.srvupdCheckBox);
            this.userrightsGroupBox.Controls.Add(this.downloadCheckBox);
            this.userrightsGroupBox.Controls.Add(this.uploadCheckBox);
            this.userrightsGroupBox.Controls.Add(this.moveuserCheckBox);
            this.userrightsGroupBox.Controls.Add(this.banCheckBox);
            this.userrightsGroupBox.Controls.Add(this.kickCheckBox);
            this.userrightsGroupBox.Controls.Add(this.tempchanCheckBox);
            this.userrightsGroupBox.Controls.Add(this.desktopCheckBox);
            this.userrightsGroupBox.Controls.Add(this.multiloginCheckBox);
            this.userrightsGroupBox.Controls.Add(this.vidcapCheckBox);
            this.userrightsGroupBox.Controls.Add(this.voicetxCheckBox);
            this.userrightsGroupBox.Controls.Add(this.bcastCheckBox);
            this.userrightsGroupBox.Controls.Add(this.viewallCheckBox);
            this.userrightsGroupBox.Controls.Add(this.operatorenableCheckBox);
            this.userrightsGroupBox.Controls.Add(this.permchannelsCheckBox);
            this.userrightsGroupBox.Location = new System.Drawing.Point(6, 101);
            this.userrightsGroupBox.Name = "userrightsGroupBox";
            this.userrightsGroupBox.Size = new System.Drawing.Size(347, 444);
            this.userrightsGroupBox.TabIndex = 10;
            this.userrightsGroupBox.TabStop = false;
            this.userrightsGroupBox.Text = "User Rights";
            // 
            // audfilesCheckBox
            // 
            this.audfilesCheckBox.AutoSize = true;
            this.audfilesCheckBox.Location = new System.Drawing.Point(6, 341);
            this.audfilesCheckBox.Name = "audfilesCheckBox";
            this.audfilesCheckBox.Size = new System.Drawing.Size(213, 17);
            this.audfilesCheckBox.TabIndex = 22;
            this.audfilesCheckBox.Text = "User can transmit audio files (wav, mp3)";
            this.audfilesCheckBox.UseVisualStyleBackColor = true;
            // 
            // desktopinputCheckBox
            // 
            this.desktopinputCheckBox.AutoSize = true;
            this.desktopinputCheckBox.Location = new System.Drawing.Point(6, 410);
            this.desktopinputCheckBox.Name = "desktopinputCheckBox";
            this.desktopinputCheckBox.Size = new System.Drawing.Size(255, 17);
            this.desktopinputCheckBox.TabIndex = 21;
            this.desktopinputCheckBox.Text = "User can get remote access to desktop sessions";
            this.desktopinputCheckBox.UseVisualStyleBackColor = true;
            // 
            // vidfileCheckBox
            // 
            this.vidfileCheckBox.AutoSize = true;
            this.vidfileCheckBox.Location = new System.Drawing.Point(6, 364);
            this.vidfileCheckBox.Name = "vidfileCheckBox";
            this.vidfileCheckBox.Size = new System.Drawing.Size(207, 17);
            this.vidfileCheckBox.TabIndex = 20;
            this.vidfileCheckBox.Text = "User can transmit video files (avi, mpg)";
            this.vidfileCheckBox.UseVisualStyleBackColor = true;
            // 
            // srvupdCheckBox
            // 
            this.srvupdCheckBox.AutoSize = true;
            this.srvupdCheckBox.Location = new System.Drawing.Point(6, 272);
            this.srvupdCheckBox.Name = "srvupdCheckBox";
            this.srvupdCheckBox.Size = new System.Drawing.Size(186, 17);
            this.srvupdCheckBox.TabIndex = 17;
            this.srvupdCheckBox.Text = "User can update server properties";
            this.srvupdCheckBox.UseVisualStyleBackColor = true;
            // 
            // downloadCheckBox
            // 
            this.downloadCheckBox.AutoSize = true;
            this.downloadCheckBox.Location = new System.Drawing.Point(6, 249);
            this.downloadCheckBox.Name = "downloadCheckBox";
            this.downloadCheckBox.Size = new System.Drawing.Size(139, 17);
            this.downloadCheckBox.TabIndex = 16;
            this.downloadCheckBox.Text = "User can download files";
            this.downloadCheckBox.UseVisualStyleBackColor = true;
            // 
            // uploadCheckBox
            // 
            this.uploadCheckBox.AutoSize = true;
            this.uploadCheckBox.Location = new System.Drawing.Point(6, 226);
            this.uploadCheckBox.Name = "uploadCheckBox";
            this.uploadCheckBox.Size = new System.Drawing.Size(125, 17);
            this.uploadCheckBox.TabIndex = 15;
            this.uploadCheckBox.Text = "User can upload files";
            this.uploadCheckBox.UseVisualStyleBackColor = true;
            // 
            // moveuserCheckBox
            // 
            this.moveuserCheckBox.AutoSize = true;
            this.moveuserCheckBox.Location = new System.Drawing.Point(6, 180);
            this.moveuserCheckBox.Name = "moveuserCheckBox";
            this.moveuserCheckBox.Size = new System.Drawing.Size(216, 17);
            this.moveuserCheckBox.TabIndex = 14;
            this.moveuserCheckBox.Text = "User can move users between channels";
            this.moveuserCheckBox.UseVisualStyleBackColor = true;
            // 
            // banCheckBox
            // 
            this.banCheckBox.AutoSize = true;
            this.banCheckBox.Location = new System.Drawing.Point(6, 157);
            this.banCheckBox.Name = "banCheckBox";
            this.banCheckBox.Size = new System.Drawing.Size(189, 17);
            this.banCheckBox.TabIndex = 13;
            this.banCheckBox.Text = "User can ban users from logging in";
            this.banCheckBox.UseVisualStyleBackColor = true;
            // 
            // kickCheckBox
            // 
            this.kickCheckBox.AutoSize = true;
            this.kickCheckBox.Location = new System.Drawing.Point(6, 134);
            this.kickCheckBox.Name = "kickCheckBox";
            this.kickCheckBox.Size = new System.Drawing.Size(185, 17);
            this.kickCheckBox.TabIndex = 12;
            this.kickCheckBox.Text = "User can kick users off the server";
            this.kickCheckBox.UseVisualStyleBackColor = true;
            // 
            // tempchanCheckBox
            // 
            this.tempchanCheckBox.AutoSize = true;
            this.tempchanCheckBox.Location = new System.Drawing.Point(6, 88);
            this.tempchanCheckBox.Name = "tempchanCheckBox";
            this.tempchanCheckBox.Size = new System.Drawing.Size(197, 17);
            this.tempchanCheckBox.TabIndex = 11;
            this.tempchanCheckBox.Text = "User can create temporary channels";
            this.tempchanCheckBox.UseVisualStyleBackColor = true;
            // 
            // desktopCheckBox
            // 
            this.desktopCheckBox.AutoSize = true;
            this.desktopCheckBox.Location = new System.Drawing.Point(6, 387);
            this.desktopCheckBox.Name = "desktopCheckBox";
            this.desktopCheckBox.Size = new System.Drawing.Size(187, 17);
            this.desktopCheckBox.TabIndex = 10;
            this.desktopCheckBox.Text = "User can transmit desktop session";
            this.desktopCheckBox.UseVisualStyleBackColor = true;
            // 
            // multiloginCheckBox
            // 
            this.multiloginCheckBox.AutoSize = true;
            this.multiloginCheckBox.Location = new System.Drawing.Point(6, 19);
            this.multiloginCheckBox.Name = "multiloginCheckBox";
            this.multiloginCheckBox.Size = new System.Drawing.Size(162, 17);
            this.multiloginCheckBox.TabIndex = 3;
            this.multiloginCheckBox.Text = "User can log in multiple times";
            this.multiloginCheckBox.UseVisualStyleBackColor = true;
            // 
            // vidcapCheckBox
            // 
            this.vidcapCheckBox.AutoSize = true;
            this.vidcapCheckBox.Location = new System.Drawing.Point(6, 318);
            this.vidcapCheckBox.Name = "vidcapCheckBox";
            this.vidcapCheckBox.Size = new System.Drawing.Size(210, 17);
            this.vidcapCheckBox.TabIndex = 9;
            this.vidcapCheckBox.Text = "User can transmit video data (webcam)";
            this.vidcapCheckBox.UseVisualStyleBackColor = true;
            // 
            // voicetxCheckBox
            // 
            this.voicetxCheckBox.AutoSize = true;
            this.voicetxCheckBox.Location = new System.Drawing.Point(6, 295);
            this.voicetxCheckBox.Name = "voicetxCheckBox";
            this.voicetxCheckBox.Size = new System.Drawing.Size(225, 17);
            this.voicetxCheckBox.TabIndex = 8;
            this.voicetxCheckBox.Text = "User can transmit voice data (microphone)";
            this.voicetxCheckBox.UseVisualStyleBackColor = true;
            // 
            // bcastCheckBox
            // 
            this.bcastCheckBox.AutoSize = true;
            this.bcastCheckBox.Location = new System.Drawing.Point(6, 111);
            this.bcastCheckBox.Name = "bcastCheckBox";
            this.bcastCheckBox.Size = new System.Drawing.Size(194, 17);
            this.bcastCheckBox.TabIndex = 5;
            this.bcastCheckBox.Text = "Users can broadcast text messages";
            this.bcastCheckBox.UseVisualStyleBackColor = true;
            // 
            // viewallCheckBox
            // 
            this.viewallCheckBox.AutoSize = true;
            this.viewallCheckBox.Location = new System.Drawing.Point(6, 42);
            this.viewallCheckBox.Name = "viewallCheckBox";
            this.viewallCheckBox.Size = new System.Drawing.Size(187, 17);
            this.viewallCheckBox.TabIndex = 4;
            this.viewallCheckBox.Text = "User can see users in all channels";
            this.viewallCheckBox.UseVisualStyleBackColor = true;
            // 
            // operatorenableCheckBox
            // 
            this.operatorenableCheckBox.AutoSize = true;
            this.operatorenableCheckBox.Location = new System.Drawing.Point(6, 203);
            this.operatorenableCheckBox.Name = "operatorenableCheckBox";
            this.operatorenableCheckBox.Size = new System.Drawing.Size(236, 17);
            this.operatorenableCheckBox.TabIndex = 1;
            this.operatorenableCheckBox.Text = "User can make other users channel operator";
            this.operatorenableCheckBox.UseVisualStyleBackColor = true;
            // 
            // permchannelsCheckBox
            // 
            this.permchannelsCheckBox.AutoSize = true;
            this.permchannelsCheckBox.Location = new System.Drawing.Point(6, 65);
            this.permchannelsCheckBox.Name = "permchannelsCheckBox";
            this.permchannelsCheckBox.Size = new System.Drawing.Size(183, 17);
            this.permchannelsCheckBox.TabIndex = 0;
            this.permchannelsCheckBox.Text = "User can create/modify channels";
            this.permchannelsCheckBox.UseVisualStyleBackColor = true;
            // 
            // clrButton
            // 
            this.clrButton.Location = new System.Drawing.Point(367, 500);
            this.clrButton.Name = "clrButton";
            this.clrButton.Size = new System.Drawing.Size(75, 23);
            this.clrButton.TabIndex = 9;
            this.clrButton.Text = "Clear";
            this.clrButton.UseVisualStyleBackColor = true;
            this.clrButton.Click += new System.EventHandler(this.clrButton_Click);
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.delChanButton);
            this.groupBox3.Controls.Add(this.addChanButton);
            this.groupBox3.Controls.Add(this.label7);
            this.groupBox3.Controls.Add(this.label6);
            this.groupBox3.Controls.Add(this.selchanListBox);
            this.groupBox3.Controls.Add(this.availchanListBox);
            this.groupBox3.Location = new System.Drawing.Point(359, 10);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(347, 219);
            this.groupBox3.TabIndex = 8;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Auto-Operator Channels";
            // 
            // delChanButton
            // 
            this.delChanButton.Location = new System.Drawing.Point(159, 106);
            this.delChanButton.Name = "delChanButton";
            this.delChanButton.Size = new System.Drawing.Size(28, 23);
            this.delChanButton.TabIndex = 5;
            this.delChanButton.Text = "<";
            this.delChanButton.UseVisualStyleBackColor = true;
            this.delChanButton.Click += new System.EventHandler(this.delChanButton_Click);
            // 
            // addChanButton
            // 
            this.addChanButton.Location = new System.Drawing.Point(159, 68);
            this.addChanButton.Name = "addChanButton";
            this.addChanButton.Size = new System.Drawing.Size(28, 23);
            this.addChanButton.TabIndex = 4;
            this.addChanButton.Text = ">";
            this.addChanButton.UseVisualStyleBackColor = true;
            this.addChanButton.Click += new System.EventHandler(this.addChanButton_Click);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(156, 16);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(96, 13);
            this.label7.TabIndex = 3;
            this.label7.Text = "Selected Channels";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(6, 16);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(97, 13);
            this.label6.TabIndex = 2;
            this.label6.Text = "Available Channels";
            // 
            // selchanListBox
            // 
            this.selchanListBox.FormattingEnabled = true;
            this.selchanListBox.Location = new System.Drawing.Point(193, 29);
            this.selchanListBox.Name = "selchanListBox";
            this.selchanListBox.Size = new System.Drawing.Size(147, 173);
            this.selchanListBox.Sorted = true;
            this.selchanListBox.TabIndex = 1;
            // 
            // availchanListBox
            // 
            this.availchanListBox.FormattingEnabled = true;
            this.availchanListBox.Location = new System.Drawing.Point(6, 29);
            this.availchanListBox.Name = "availchanListBox";
            this.availchanListBox.Size = new System.Drawing.Size(147, 173);
            this.availchanListBox.Sorted = true;
            this.availchanListBox.TabIndex = 0;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.initchanTextBox);
            this.groupBox2.Controls.Add(this.label4);
            this.groupBox2.Controls.Add(this.label5);
            this.groupBox2.Controls.Add(this.label3);
            this.groupBox2.Controls.Add(this.noteTextBox);
            this.groupBox2.Controls.Add(this.userdataTextBox);
            this.groupBox2.Location = new System.Drawing.Point(359, 294);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(347, 200);
            this.groupBox2.TabIndex = 6;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Optional";
            // 
            // initchanTextBox
            // 
            this.initchanTextBox.Location = new System.Drawing.Point(90, 27);
            this.initchanTextBox.Name = "initchanTextBox";
            this.initchanTextBox.Size = new System.Drawing.Size(180, 20);
            this.initchanTextBox.TabIndex = 5;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(9, 79);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(30, 13);
            this.label4.TabIndex = 2;
            this.label4.Text = "Note";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(9, 31);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(72, 13);
            this.label5.TabIndex = 4;
            this.label5.Text = "Initial channel";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(9, 57);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(55, 13);
            this.label3.TabIndex = 0;
            this.label3.Text = "User Data";
            // 
            // noteTextBox
            // 
            this.noteTextBox.AcceptsReturn = true;
            this.noteTextBox.Location = new System.Drawing.Point(90, 79);
            this.noteTextBox.Multiline = true;
            this.noteTextBox.Name = "noteTextBox";
            this.noteTextBox.Size = new System.Drawing.Size(241, 109);
            this.noteTextBox.TabIndex = 3;
            // 
            // userdataTextBox
            // 
            this.userdataTextBox.Location = new System.Drawing.Point(90, 53);
            this.userdataTextBox.Name = "userdataTextBox";
            this.userdataTextBox.Size = new System.Drawing.Size(180, 20);
            this.userdataTextBox.TabIndex = 1;
            this.userdataTextBox.Text = "0";
            this.userdataTextBox.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // createButton
            // 
            this.createButton.Location = new System.Drawing.Point(635, 500);
            this.createButton.Name = "createButton";
            this.createButton.Size = new System.Drawing.Size(75, 23);
            this.createButton.TabIndex = 7;
            this.createButton.Text = "Create";
            this.createButton.UseVisualStyleBackColor = true;
            this.createButton.Click += new System.EventHandler(this.createButton_Click);
            // 
            // adminRadioButton
            // 
            this.adminRadioButton.AutoSize = true;
            this.adminRadioButton.Location = new System.Drawing.Point(132, 78);
            this.adminRadioButton.Name = "adminRadioButton";
            this.adminRadioButton.Size = new System.Drawing.Size(85, 17);
            this.adminRadioButton.TabIndex = 5;
            this.adminRadioButton.Text = "Administrator";
            this.adminRadioButton.UseVisualStyleBackColor = true;
            this.adminRadioButton.CheckedChanged += new System.EventHandler(this.adminRadioButton_CheckedChanged);
            // 
            // defuserRadioButton
            // 
            this.defuserRadioButton.AutoSize = true;
            this.defuserRadioButton.Checked = true;
            this.defuserRadioButton.Location = new System.Drawing.Point(29, 78);
            this.defuserRadioButton.Name = "defuserRadioButton";
            this.defuserRadioButton.Size = new System.Drawing.Size(84, 17);
            this.defuserRadioButton.TabIndex = 4;
            this.defuserRadioButton.TabStop = true;
            this.defuserRadioButton.Text = "Default User";
            this.defuserRadioButton.UseVisualStyleBackColor = true;
            // 
            // passwordTextBox
            // 
            this.passwordTextBox.Location = new System.Drawing.Point(97, 50);
            this.passwordTextBox.Name = "passwordTextBox";
            this.passwordTextBox.Size = new System.Drawing.Size(132, 20);
            this.passwordTextBox.TabIndex = 3;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 53);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(53, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "Password";
            // 
            // usernameTextBox
            // 
            this.usernameTextBox.Location = new System.Drawing.Point(97, 22);
            this.usernameTextBox.Name = "usernameTextBox";
            this.usernameTextBox.Size = new System.Drawing.Size(132, 20);
            this.usernameTextBox.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 25);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(55, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Username";
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.label9);
            this.groupBox4.Controls.Add(this.bitrateNumericUpDown);
            this.groupBox4.Controls.Add(this.label8);
            this.groupBox4.Location = new System.Drawing.Point(360, 235);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(350, 52);
            this.groupBox4.TabIndex = 11;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Audio codec limitations";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(225, 26);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(90, 13);
            this.label9.TabIndex = 2;
            this.label9.Text = "kbps (0=disabled)";
            // 
            // bitrateNumericUpDown
            // 
            this.bitrateNumericUpDown.Location = new System.Drawing.Point(153, 23);
            this.bitrateNumericUpDown.Maximum = new decimal(new int[] {
            512,
            0,
            0,
            0});
            this.bitrateNumericUpDown.Name = "bitrateNumericUpDown";
            this.bitrateNumericUpDown.Size = new System.Drawing.Size(59, 20);
            this.bitrateNumericUpDown.TabIndex = 1;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(6, 26);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(141, 13);
            this.label8.TabIndex = 0;
            this.label8.Text = "Max bitrate for audio codecs";
            // 
            // UserAccountsDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(986, 545);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.listView1);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "UserAccountsDlg";
            this.Text = "User Accounts";
            this.contextMenuStrip1.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.userrightsGroupBox.ResumeLayout(false);
            this.userrightsGroupBox.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.bitrateNumericUpDown)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListView listView1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RadioButton adminRadioButton;
        private System.Windows.Forms.RadioButton defuserRadioButton;
        private System.Windows.Forms.TextBox passwordTextBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox usernameTextBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button createButton;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
        private System.Windows.Forms.ToolStripMenuItem deleteAccountToolStripMenuItem;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox noteTextBox;
        private System.Windows.Forms.TextBox userdataTextBox;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.ListBox availchanListBox;
        private System.Windows.Forms.ListBox selchanListBox;
        private System.Windows.Forms.Button delChanButton;
        private System.Windows.Forms.Button addChanButton;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox initchanTextBox;
        private System.Windows.Forms.Button clrButton;
        private System.Windows.Forms.GroupBox userrightsGroupBox;
        private System.Windows.Forms.CheckBox desktopCheckBox;
        private System.Windows.Forms.CheckBox multiloginCheckBox;
        private System.Windows.Forms.CheckBox vidcapCheckBox;
        private System.Windows.Forms.CheckBox voicetxCheckBox;
        private System.Windows.Forms.CheckBox bcastCheckBox;
        private System.Windows.Forms.CheckBox viewallCheckBox;
        private System.Windows.Forms.CheckBox operatorenableCheckBox;
        private System.Windows.Forms.CheckBox permchannelsCheckBox;
        private System.Windows.Forms.CheckBox tempchanCheckBox;
        private System.Windows.Forms.CheckBox desktopinputCheckBox;
        private System.Windows.Forms.CheckBox vidfileCheckBox;
        private System.Windows.Forms.CheckBox srvupdCheckBox;
        private System.Windows.Forms.CheckBox downloadCheckBox;
        private System.Windows.Forms.CheckBox uploadCheckBox;
        private System.Windows.Forms.CheckBox moveuserCheckBox;
        private System.Windows.Forms.CheckBox banCheckBox;
        private System.Windows.Forms.CheckBox kickCheckBox;
        private System.Windows.Forms.CheckBox audfilesCheckBox;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.NumericUpDown bitrateNumericUpDown;
        private System.Windows.Forms.Label label8;
    }
}