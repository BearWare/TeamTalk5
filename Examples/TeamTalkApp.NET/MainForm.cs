/*
 * Copyright (c) 2005-2017, BearWare.dk
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
using System.Drawing.Imaging;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.Globalization;
using System.Threading;
using BearWare;

namespace TeamTalkApp.NET
{

    public partial class MainForm : Form
    {
        TeamTalk ttclient;
        ChannelsView channels;
        UsersView users;
        FilesView files;

        Dictionary<int, MessageDlg> msgdialogs;
        Dictionary<int, VideoDlg> viddialogs;
        Dictionary<int, DesktopDlg> desktopdialogs;

        ClientStatistics statistics;

        Settings settings;

        IntPtr hShareWnd = IntPtr.Zero;
        BitmapFormat bmpShareFormat = BitmapFormat.BMP_NONE;

        //We want to pass the same bitmap reference every time so we don't
        //waste memory reallocating a new bitmap every time we call 
        //ttclient.GetUserVideoFrame
        Bitmap local_bitmap = null;

        public MainForm()
        {
            InitializeComponent();

            /* Set license information before creating the client instance */
            //TeamTalk.SetLicenseInformation("", "");

            msgdialogs = new Dictionary<int, MessageDlg>();
            viddialogs = new Dictionary<int, VideoDlg>();
            desktopdialogs = new Dictionary<int, DesktopDlg>();

            inputProgressBar.Minimum = SoundLevel.SOUND_VU_MIN;
            inputProgressBar.Maximum = SoundLevel.SOUND_VU_MAX;
            voiceactTrackBar.Minimum = SoundLevel.SOUND_VU_MIN;
            voiceactTrackBar.Maximum = SoundLevel.SOUND_VU_MAX;
            inputgainTrackBar.Minimum = SoundLevel.SOUND_GAIN_MIN;
            inputgainTrackBar.Maximum = SoundLevel.SOUND_GAIN_MAX;

            volumeTrackBar.Minimum = SoundLevel.SOUND_VOLUME_MIN;
            volumeTrackBar.Maximum = 16000;

            /* we pass 'false' to poll_events since we don't want to 
             * manually process events using ttclient.GetMessage */
            ttclient = new TeamTalk(false);
            channels = new ChannelsView(ttclient, treeView1);
            users = new UsersView(ttclient, listView1);
            files = new FilesView(ttclient, filesListView);
            settings = new Settings();

            voiceactTrackBar.Value = ttclient.GetVoiceActivationLevel();
            inputgainTrackBar.Value = ttclient.GetSoundInputGainLevel();
            volumeTrackBar.Value = ttclient.GetSoundOutputVolume();

            //get default devices
            TeamTalk.GetDefaultSoundDevices(ref settings.sndinputid, ref settings.sndoutputid);

            ttclient.OnConnectionSuccess += new TeamTalk.Connection(ttclient_OnConnectionSuccess);
            ttclient.OnConnectionFailed += new TeamTalk.Connection(ttclient_OnConnectionFailed);
            ttclient.OnConnectionLost += new TeamTalk.Connection(ttclient_OnConnectionLost);

            ttclient.OnCmdProcessing += new TeamTalk.CommandProcessing(ttclient_OnCmdProcessing);
            ttclient.OnCmdError += new TeamTalk.CommandError(ttclient_OnCmdError);
            ttclient.OnCmdMyselfLoggedIn += new TeamTalk.MyselfLoggedIn(ttclient_OnCmdMyselfLoggedIn);
            ttclient.OnCmdMyselfLoggedOut += new TeamTalk.MyselfLoggedOut(ttclient_OnCmdMyselfLoggedOut);
            ttclient.OnCmdUserLoggedIn += new TeamTalk.UserUpdate(ttclient_OnCmdUserLoggedIn);
            ttclient.OnCmdUserJoinedChannel += new TeamTalk.UserUpdate(ttclient_OnCmdUserJoinedChannel);
            ttclient.OnCmdUserLeftChannel += new TeamTalk.UserUpdate(ttclient_OnCmdUserLeftChannel);
            ttclient.OnCmdUserTextMessage += new TeamTalk.UserTextMessage(ttclient_OnCmdUserTextMessage);
            ttclient.OnCmdChannelNew += new TeamTalk.ChannelUpdate(ttclient_OnCmdChannelNew);
            ttclient.OnCmdChannelUpdate += new TeamTalk.ChannelUpdate(ttclient_OnCmdChannelUpdate);
            ttclient.OnCmdChannelRemove += new TeamTalk.ChannelUpdate(ttclient_OnCmdChannelRemove);

            ttclient.OnInternalError += new TeamTalk.ErrorOccured(ttclient_OnInternalError);
            ttclient.OnHotKeyToggle += new TeamTalk.HotKeyToggle(ttclient_OnHotKeyToggle);
            ttclient.OnUserVideoCapture += new TeamTalk.UserVideoFrame(ttclient_OnUserVideoCapture);
            ttclient.OnStreamMediaFile += new TeamTalk.StreamMediaFile(ttclient_OnStreamMediaFile);
            ttclient.OnUserRecordMediaFile += new TeamTalk.UserRecordMediaFile(ttclient_OnUserRecordMediaFile);
            ttclient.OnUserAudioBlock += new TeamTalk.NewAudioBlock(ttclient_OnUserAudioBlock);
            ttclient.OnUserDesktopInput += new TeamTalk.UserDesktopInput(ttclient_OnUserDesktopInput);
            ttclient.OnFileTransfer += new TeamTalk.FileTransferUpdate(ttclient_OnFileTransfer);
            ttclient.OnUserDesktopWindow += new TeamTalk.NewDesktopWindow(ttclient_OnUserDesktopWindow);

            vumeterTimer.Enabled = true;
            timer1.Enabled = true;
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            this.CenterToScreen();

            UpdateControls();
        }

        void UpdateControls()
        {
            ClientFlag flags = ttclient.Flags;
            UserType myusertype = ttclient.UserType;
            ServerProperties srvprop = new ServerProperties();

            if ((flags & ClientFlag.CLIENT_CONNECTED) != ClientFlag.CLIENT_CONNECTED)
                ResetControls();

            ttclient.GetServerProperties(ref srvprop);

            int userid = channels.GetSelectedUser();
            int channelid = channels.GetSelectedChannel();

            connectToolStripMenuItem.Text =
                ((flags & ClientFlag.CLIENT_CONNECTED) == ClientFlag.CLIENT_CONNECTED ||
                (flags & ClientFlag.CLIENT_CONNECTING) == ClientFlag.CLIENT_CONNECTING) ? "Disconnect" : "Connect";

            changeNicknameToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
            changeStatusModeToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
            enableDesktopSharingToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
            enableDesktopSharingToolStripMenuItem.Checked = flags.HasFlag(ClientFlag.CLIENT_DESKTOP_ACTIVE);

            muteAllToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_SNDOUTPUT_READY);
            muteAllToolStripMenuItem.Checked = flags.HasFlag(ClientFlag.CLIENT_SNDOUTPUT_MUTE);
            storeAudioToDiskToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
            storeAudioToDiskToolStripMenuItem.Checked = settings.audiofolder.Length > 0;

            User user = new User();
            if (userid > 0 && ttclient.GetUser(userid, ref user))
            {
                Debug.Assert((flags & ClientFlag.CLIENT_AUTHORIZED) == ClientFlag.CLIENT_AUTHORIZED);
                viewUserInformationToolStripMenuItem.Enabled = true;
                sendMessageToolStripMenuItem.Enabled = true;
                opDeOpToolStripMenuItem.Enabled = true;
                mutevoiceToolStripMenuItem.Enabled = true;
                mutevoiceToolStripMenuItem.Checked = user.uUserState.HasFlag(UserState.USERSTATE_MUTE_VOICE);
                kickToolStripMenuItem.Enabled = true;
                kickAndBanToolStripMenuItem.Enabled = true;
                subscriptionsToolStripMenuItem.Enabled = true;
                allowDesktopAccessToolStripMenuItem.Enabled = true;
                allowDesktopAccessToolStripMenuItem.Checked = user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_DESKTOPINPUT);

                userMessagesToolStripMenuItem.Checked = user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_USER_MSG);
                channelMessagesToolStripMenuItem.Checked = user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_CHANNEL_MSG);
                broadcastMessagesToolStripMenuItem.Checked = user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_BROADCAST_MSG);
                voiceToolStripMenuItem.Checked = user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_VOICE);
                videocaptureToolStripMenuItem.Checked = user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_VIDEOCAPTURE);
                desktopToolStripMenuItem.Checked = user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_DESKTOP);
                desktopAccessToolStripMenuItem.Checked = user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_DESKTOPINPUT);
                interceptUserMessagesToolStripMenuItem.Checked = user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_INTERCEPT_USER_MSG);
                interceptUserMessagesToolStripMenuItem.Enabled = myusertype.HasFlag(UserType.USERTYPE_ADMIN);
                interceptChannelMessagesToolStripMenuItem.Checked = user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_INTERCEPT_CHANNEL_MSG);
                interceptChannelMessagesToolStripMenuItem.Enabled = myusertype.HasFlag(UserType.USERTYPE_ADMIN);
                interceptvoiceToolStripMenuItem.Checked = user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_INTERCEPT_VOICE);
                interceptvoiceToolStripMenuItem.Enabled = myusertype.HasFlag(UserType.USERTYPE_ADMIN);
                interceptvideocaptureToolStripMenuItem.Checked = user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_INTERCEPT_VIDEOCAPTURE);
                interceptvideocaptureToolStripMenuItem.Enabled = myusertype.HasFlag(UserType.USERTYPE_ADMIN);
                interceptDesktopToolStripMenuItem.Checked = user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_INTERCEPT_DESKTOP);
                interceptDesktopToolStripMenuItem.Enabled = myusertype.HasFlag(UserType.USERTYPE_ADMIN);

                advancedToolStripMenuItem.Enabled = true;

                Channel userchan = new Channel();
                if (ttclient.GetChannel(user.nChannelID, ref userchan))
                {
                    allowVoiceTransmissionToolStripMenuItem.Checked = userchan.GetTransmitStreamTypes(user.nUserID).HasFlag(StreamType.STREAMTYPE_VOICE);
                    allowVideoTransmissionToolStripMenuItem.Checked = userchan.GetTransmitStreamTypes(user.nUserID).HasFlag(StreamType.STREAMTYPE_VIDEOCAPTURE);
                    allowVoiceTransmissionToolStripMenuItem.Enabled = userchan.uChannelType.HasFlag(ChannelType.CHANNEL_CLASSROOM);
                    allowVideoTransmissionToolStripMenuItem.Enabled = userchan.uChannelType.HasFlag(ChannelType.CHANNEL_CLASSROOM);
                }
            }
            else
            {
                viewUserInformationToolStripMenuItem.Enabled = false;
                sendMessageToolStripMenuItem.Enabled = false;
                opDeOpToolStripMenuItem.Enabled = false;
                mutevoiceToolStripMenuItem.Enabled = false;
                kickToolStripMenuItem.Enabled = false;
                kickAndBanToolStripMenuItem.Enabled = false;
                allowDesktopAccessToolStripMenuItem.Enabled = false;
                subscriptionsToolStripMenuItem.Enabled = false;
                advancedToolStripMenuItem.Enabled = false;
                allowVoiceTransmissionToolStripMenuItem.Enabled = false;
                allowVideoTransmissionToolStripMenuItem.Enabled = false;
            }

            Channel chan = new Channel();
            if (channelid > 0 && ttclient.GetChannel(channelid, ref chan))
            {
                Debug.Assert((flags & ClientFlag.CLIENT_AUTHORIZED) == ClientFlag.CLIENT_AUTHORIZED);
                joinChannelToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
                joinNewChannelToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
                viewChannelInformationToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
                createChannelToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
                updateChannelToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
                deleteChannelToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
                streamMediaFileToChannelToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
            }
            else
            {
                joinChannelToolStripMenuItem.Enabled = false;
                joinNewChannelToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
                viewChannelInformationToolStripMenuItem.Enabled = false;
                createChannelToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
                updateChannelToolStripMenuItem.Enabled = false;
                deleteChannelToolStripMenuItem.Enabled = false;
                streamMediaFileToChannelToolStripMenuItem.Enabled = false;
            }

            listUserAccountsToolStripMenuItem.Enabled = myusertype.HasFlag(UserType.USERTYPE_ADMIN);
            serverPropertiesToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
            saveConfigurationToolStripMenuItem.Enabled = myusertype.HasFlag(UserType.USERTYPE_ADMIN);
            broadcastTestMessageToolStripMenuItem.Enabled = flags.HasFlag(ClientFlag.CLIENT_AUTHORIZED);
            serverStatisticsToolStripMenuItem.Enabled = myusertype.HasFlag(UserType.USERTYPE_ADMIN);
        }

        void ResetControls()
        {
            listView1.Items.Clear();
            treeView1.Nodes.Clear();
        }

        void AddStatusMessage(string msg)
        {
            chatTextBox.AppendText("* " + msg + Environment.NewLine);
        }

        void ttclient_OnConnectionSuccess()
        {
            AddStatusMessage("Connected to server");

            //the login dialog does its own comand error handling, so unregister event
            ttclient.OnCmdError -= ttclient_OnCmdError;

            LoginDlg dlg = new LoginDlg(ttclient, settings);
            dlg.ShowDialog();

            ttclient.OnCmdError += new TeamTalk.CommandError(ttclient_OnCmdError);

            UpdateControls();
        }

        void ttclient_OnConnectionFailed()
        {
            AddStatusMessage("Failed to connect");

            //close connnection
            ttclient.Disconnect();

            MessageBox.Show("Failed to connect");

            UpdateControls();
        }

        void ttclient_OnConnectionLost()
        {
            AddStatusMessage("Connection lost");

            //close connnection
            ttclient.Disconnect();

            MessageBox.Show("Connection dropped");

            UpdateControls();
        }

        void ttclient_OnCmdProcessing(int nCmdID, bool bActive)
        {
            if (!bActive)
            {
                UpdateControls();
            }
        }

        void ttclient_OnCmdError(int nCmdID, ClientErrorMsg clienterrormsg)
        {
            MessageBox.Show(clienterrormsg.szErrorMsg, "Command Error");
        }

        void ttclient_OnCmdMyselfLoggedOut()
        {
            AddStatusMessage("Logged out of server");
        }

        void ttclient_OnCmdMyselfLoggedIn(int nMyUserID, UserAccount useraccount)
        {
            AddStatusMessage("Logged on to server successfully");
            string s = String.Format("User account information\r\n" +
                "Username: {0}\r\n" +
                "UserType: {1}\r\n" +
                "UserData: {2}\r\n" +
                "UserRights: {3}\r\n" +
                "Note: {4}\r\n" +
                "Initial Channel: {5}\r\n",
                useraccount.szUsername,
                useraccount.uUserType.ToString(),
                useraccount.nUserData,
                useraccount.uUserRights.ToString(),
                useraccount.szNote,
                useraccount.szInitChannel);
            AddStatusMessage(s);
        }

        void ttclient_OnCmdUserLoggedIn(User user)
        {
            //store audio to disk if an audio-folder has been specified
            if (!settings.muxed_audio_file && settings.audiofolder.Length > 0)
                ttclient.SetUserMediaStorageDir(user.nUserID, settings.audiofolder, "", settings.aff);
        }

        void ttclient_OnCmdUserJoinedChannel(User user)
        {
            //set default gain level for user (software gain volume)
            if (user.nChannelID == ttclient.GetMyChannelID())
                AddStatusMessage(user.szNickname + " joined channel");

            UpdateControls();
        }

        void ttclient_OnCmdUserLeftChannel(User user)
        {
            if (user.nChannelID == ttclient.GetMyChannelID())
                AddStatusMessage(user.szNickname + " left channel");
            UpdateControls();

            //if user has sent desktopinput ensure keys are released
            closeUserDesktopInput(user.nUserID);
        }

        void ttclient_OnCmdUserTextMessage(TextMessage textmessage)
        {
            switch (textmessage.nMsgType)
            {
                case TextMsgType.MSGTYPE_USER :
                    MessageDlg dlg;
                    if (msgdialogs.TryGetValue(textmessage.nFromUserID, out dlg))
                    {
                        dlg.NewMessage(textmessage);
                        if (!dlg.Visible)
                            dlg.Show();
                    }
                    else
                    {
                        dlg = new MessageDlg(ttclient, textmessage.nFromUserID);
                        dlg.FormClosed += new FormClosedEventHandler(dlg_MessageDlgClosed);
                        dlg.NewMessage(textmessage);
                        dlg.Show();
                        msgdialogs.Add(textmessage.nFromUserID, dlg);
                    }
                    break;
                case TextMsgType.MSGTYPE_CHANNEL:
                    {
                        User user = new User();
                        if (ttclient.GetUser(textmessage.nFromUserID, ref user))
                            chatTextBox.AppendText("<" + user.szNickname + "> " + textmessage.szMessage + Environment.NewLine);
                        break;
                    }
                case TextMsgType.MSGTYPE_BROADCAST:
                    {
                        User user = new User();
                        if (ttclient.GetUser(textmessage.nFromUserID, ref user))
                        {
                            MessageBox.Show("Broadcast message from " + user.szNickname + Environment.NewLine +
                                            textmessage.szMessage);
                        }
                    }
                    break;
                case TextMsgType.MSGTYPE_CUSTOM:
                    {
                    }
                    break;
            }
        }

        void ttclient_OnCmdChannelNew(Channel channel)
        {
            UpdateControls();
        }

        void ttclient_OnCmdChannelUpdate(Channel channel)
        {
            UpdateControls();
        }

        void ttclient_OnCmdChannelRemove(Channel channel)
        {
            UpdateControls();
        }


        void ttclient_OnInternalError(ClientErrorMsg clienterrormsg)
        {
            MessageBox.Show(clienterrormsg.szErrorMsg, "Internal Error");
        }

        void ttclient_OnHotKeyToggle(int nHotKeyID, bool bActive)
        {
            switch ((HotKey)nHotKeyID)
            {
                case HotKey.HOTKEY_PUSHTOTALK :
                    ttclient.EnableVoiceTransmission(bActive);
                    break;
            }
            Debug.WriteLine("HotKey " + nHotKeyID + " active " + bActive);
        }

        Dictionary<int, VideoFrame> videoframes = new Dictionary<int, VideoFrame>();

        void ttclient_OnUserVideoCapture(int nUserID, int nStreamID)
        {
            if (nUserID == 0) //from local capture device
            {
                //Release bitmap resources since we're about to release
                //the memory shared between the .NET application and the
                //TeamTalk DLL.
                if (local_bitmap != null)
                    local_bitmap.Dispose();

                VideoFrame vidfrm;

                //Release shared memory
                if (videoframes.TryGetValue(nUserID, out vidfrm))
                {
                    ttclient.ReleaseUserVideoCaptureFrame(vidfrm);
                    videoframes.Remove(nUserID);
                }

                Bitmap bmp;
                vidfrm = ttclient.AcquireUserVideoCaptureFrame(nUserID, out bmp);
                if (vidfrm.nFrameBufferSize>0)
                {
                    local_bitmap = bmp;
                    vidlocalPictureBox.Image = local_bitmap;
                    videoframes.Add(nUserID, vidfrm);
                }
                else
                {
                    //Failure situation. Set image to NULL so we don't 
                    //get an access violation exception by referencing
                    //released memory.
                    vidlocalPictureBox.Image = null;
                }
            }
            else
            {
                VideoDlg dlg;
                if (!viddialogs.TryGetValue(nUserID, out dlg))
                {
                    //Local video is 'nUserID' = 0;
                    dlg = new VideoDlg(ttclient, nUserID);
                    viddialogs.Add(nUserID, dlg);
                    dlg.FormClosed += new FormClosedEventHandler(videodlg_FormClosed);
                    dlg.Show();
                }
            }
        }

        void ttclient_OnUserRecordMediaFile(int nUserID, MediaFileInfo mediafileinfo)
        {
            User user = new User();
            ttclient.GetUser(nUserID, ref user);
            switch (mediafileinfo.nStatus)
            {
                case MediaFileStatus.MFS_STARTED :
                    AddStatusMessage("Start audio file for " + user.szNickname);
                    break;
                case MediaFileStatus.MFS_FINISHED:
                    AddStatusMessage("Finished audio file for " + user.szNickname);
                    break;
                case MediaFileStatus.MFS_ERROR:
                    AddStatusMessage("Error writing audio file for " + user.szNickname);
                    break;
                case MediaFileStatus.MFS_ABORTED:
                    AddStatusMessage("Aborted audio file for " + user.szNickname);
                    break;
            }
        }

        void ttclient_OnUserAudioBlock(int nUserID, StreamType nStreamType)
        {
            AudioBlock block = ttclient.AcquireUserAudioBlock(nStreamType, nUserID);
            if(block.nSamples>0)
            {
                ttclient.ReleaseUserAudioBlock(block);
            }
        }

        void ttclient_OnUserDesktopWindow(int nUserID, int nSessionID)
        {
            DesktopDlg dlg;
            if (!desktopdialogs.TryGetValue(nUserID, out dlg))
            {
                //Local video is 'nUserID' = 0;
                dlg = new DesktopDlg(ttclient, nUserID);
                desktopdialogs.Add(nUserID, dlg);
                dlg.FormClosed += new FormClosedEventHandler(desktopdlg_FormClosed);
                dlg.Show();
            }

            //release any keys held down by the user
            if (nSessionID == 0)
                closeUserDesktopInput(nUserID);
        }

        private void connectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ClientFlag flags = ttclient.GetFlags();
            if (flags.HasFlag(ClientFlag.CLIENT_CONNECTED) ||
                flags.HasFlag(ClientFlag.CLIENT_CONNECTING))
            {
                ttclient.Disconnect();
                UpdateControls();
                return;
            }

            ConnectDlg dlg = new ConnectDlg(ttclient, settings);
            if (dlg.ShowDialog() == DialogResult.OK)
            {
                if ((flags & ClientFlag.CLIENT_SNDINPUT_READY) != ClientFlag.CLIENT_SNDINPUT_READY &&
                    !ttclient.InitSoundInputDevice(settings.sndinputid))
                    MessageBox.Show("Failed to initialize sound input device");
                if ((flags & ClientFlag.CLIENT_SNDOUTPUT_READY) != ClientFlag.CLIENT_SNDOUTPUT_READY &&
                    !ttclient.InitSoundOutputDevice(settings.sndoutputid))
                    MessageBox.Show("Failed to initialize sound output device");
            }
            UpdateControls();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void preferencesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            PreferencesDlg dlg = new PreferencesDlg(ttclient, settings);
            dlg.ShowDialog();
        }

        private void joinChannelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int channelid = channels.GetSelectedChannel();
            if (channelid <= 0)
                return;
            //check if password protected
            Channel chan = new Channel();
            if (!ttclient.GetChannel(channelid, ref chan))
                return;
            string passwd = "";
            if (chan.bPassword)
                passwd = InputBox.Get("Join channel", "Password");
            ttclient.DoJoinChannelByID(channelid, passwd);
        }


        private void joinNewChannelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int channelid = channels.GetSelectedChannel();
            if (channelid == 0)
                channelid = ttclient.GetRootChannelID(); //make the root parent if nothing is selected

            ChannelDlg dlg = new ChannelDlg(ttclient, ChannelDlgType.JOIN_CHANNEL, 0, channelid);
            dlg.ShowDialog();
        }

        private void vumeterTimer_Tick(object sender, EventArgs e)
        {
            inputProgressBar.Value = ttclient.GetSoundInputLevel();
        }

        private void voiceactTrackBar_ValueChanged(object sender, EventArgs e)
        {
            ttclient.SetVoiceActivationLevel(voiceactTrackBar.Value);
        }

        private void inputgainTrackBar_ValueChanged(object sender, EventArgs e)
        {
            ttclient.SetSoundInputGainLevel(inputgainTrackBar.Value);
        }

        private void volumeTrackBar_ValueChanged(object sender, EventArgs e)
        {
            ttclient.SetSoundOutputVolume(volumeTrackBar.Value);
        }

        private void treeView1_AfterSelect(object sender, TreeViewEventArgs e)
        {
            UpdateControls();
        }

        private void chanmsgButton_Click(object sender, EventArgs e)
        {
            if (chanmsgTextBox.Text.Length > 0)
            {
                TextMessage msg;
                msg.nMsgType = TextMsgType.MSGTYPE_CHANNEL;
                msg.nChannelID = ttclient.GetMyChannelID();
                msg.nFromUserID = ttclient.GetMyUserID();
                msg.szFromUsername = ""; //not required
                msg.nToUserID = 0;
                msg.szMessage = chanmsgTextBox.Text;
                chanmsgTextBox.Text = "";
                ttclient.DoTextMessage(msg);
            }
        }

        private void enablePushToTalkToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void viewChannelInformationToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int channelid = channels.GetSelectedChannel();
            ChannelDlg dlg = new ChannelDlg(ttclient, ChannelDlgType.VIEW_CHANNEL, channelid, 0);
            dlg.ShowDialog();
        }

        private void createChannelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int channelid = channels.GetSelectedChannel();
            if (channelid == 0)
                channelid = ttclient.GetRootChannelID(); //make the root parent if nothing is selected

            ChannelDlg dlg = new ChannelDlg(ttclient, ChannelDlgType.CREATE_CHANNEL, 0, channelid);
            dlg.ShowDialog();
        }

        private void updateChannelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int channelid = channels.GetSelectedChannel();
            ChannelDlg dlg = new ChannelDlg(ttclient, ChannelDlgType.UPDATE_CHANNEL, channelid, 0);
            dlg.ShowDialog();
        }

        private void deleteChannelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int channelid = channels.GetSelectedChannel();
            string name = "";
            ttclient.GetChannelPath(channelid, ref name);
            if (MessageBox.Show("Delete channel: " + name, "Delete Channel", MessageBoxButtons.YesNo) == DialogResult.Yes)
                ttclient.DoRemoveChannel(channelid);
        }

        private void sendMessageToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int userid = channels.GetSelectedUser();
            MessageDlg dlg;
            if (msgdialogs.TryGetValue(userid, out dlg))
                dlg.Show();
            else
            {
                dlg = new MessageDlg(ttclient, userid);
                dlg.FormClosed += new FormClosedEventHandler(dlg_MessageDlgClosed);
                dlg.Show();
                msgdialogs.Add(userid, dlg);
            }
        }

        void dlg_MessageDlgClosed(object sender, FormClosedEventArgs e)
        {
            foreach (KeyValuePair<int, MessageDlg> dlg in msgdialogs)
            {
                if (dlg.Value == sender)
                {
                    msgdialogs.Remove(dlg.Key);
                    break;
                }
            }
        }

        void videodlg_FormClosed(object sender, FormClosedEventArgs e)
        {
            foreach (KeyValuePair<int, VideoDlg> dlg in viddialogs)
            {
                if (dlg.Value == sender)
                {
                    viddialogs.Remove(dlg.Key);
                    break;
                }
            }
        }

        void desktopdlg_FormClosed(object sender, FormClosedEventArgs e)
        {
            foreach (KeyValuePair<int, DesktopDlg> dlg in desktopdialogs)
            {
                if (dlg.Value == sender)
                {
                    desktopdialogs.Remove(dlg.Key);
                    break;
                }
            }
        }

        private void changeNicknameToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string nickname = InputBox.Get("Change Nickname", "Nickname");
            ttclient.DoChangeNickname(nickname);
        }

        private void changeStatusModeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string msg = InputBox.Get("Change Status Mode", "Status Message");
            ttclient.DoChangeStatus(0, msg);
        }

        private void opDeOpToolStripMenuItem_Click(object sender, EventArgs e)
        {
            User user = new User();
            if (!ttclient.GetUser(channels.GetSelectedUser(), ref user))
                return;

            ttclient.DoChannelOp(user.nUserID, user.nChannelID, 
                !ttclient.IsChannelOperator(user.nUserID, user.nChannelID));
        }

        private void kickToolStripMenuItem_Click(object sender, EventArgs e)
        {
            User user = new User();
            if (!ttclient.GetUser(channels.GetSelectedUser(), ref user))
                return;
            //pass 0 as 'nChannelID' to kick from server instead of channel
            ttclient.DoKickUser(user.nUserID, user.nChannelID);
        }

        private void kickAndBanToolStripMenuItem_Click(object sender, EventArgs e)
        {
            User user = new User();
            if (!ttclient.GetUser(channels.GetSelectedUser(), ref user))
                return;
            //Req. UserRight.USERRIGHT_BAN_USERS
            //Req. UserRight.USERRIGHT_KICK_USERS
            ttclient.DoBanUser(user.nUserID, 0);
            ttclient.DoKickUser(user.nUserID, user.nChannelID);
        }

        private void listUserAccountsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //only admins can do this.
            UserAccountsDlg dlg = new UserAccountsDlg(ttclient);
            dlg.ShowDialog();
        }

        private void serverPropertiesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ServerPropertiesDlg dlg = new ServerPropertiesDlg(ttclient);
            dlg.ShowDialog();
        }

        private void saveConfigurationToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ttclient.DoSaveConfig();
        }

        private void muteAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ttclient.SetSoundOutputMute(muteAllToolStripMenuItem.Checked);
        }

        private void subscribeCommon(int userid, Subscription sub)
        {
            User user = new User();
            if (!ttclient.GetUser(userid, ref user))
                return;

            if ((user.uLocalSubscriptions & sub) == sub)
                ttclient.DoUnsubscribe(userid, sub);
            else
                ttclient.DoSubscribe(userid, sub);
        }

        private void userMessagesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            subscribeCommon(channels.GetSelectedUser(), Subscription.SUBSCRIBE_USER_MSG);
        }

        private void channelMessagesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            subscribeCommon(channels.GetSelectedUser(), Subscription.SUBSCRIBE_CHANNEL_MSG);
        }

        private void broadcastMessagesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            subscribeCommon(channels.GetSelectedUser(), Subscription.SUBSCRIBE_BROADCAST_MSG);
        }

        private void voiceToolStripMenuItem_Click(object sender, EventArgs e)
        {
            subscribeCommon(channels.GetSelectedUser(), Subscription.SUBSCRIBE_VOICE);
        }

        private void videocaptureToolStripMenuItem_Click(object sender, EventArgs e)
        {
            subscribeCommon(channels.GetSelectedUser(), Subscription.SUBSCRIBE_VIDEOCAPTURE);
        }

        private void desktopToolStripMenuItem_Click(object sender, EventArgs e)
        {
            subscribeCommon(channels.GetSelectedUser(), Subscription.SUBSCRIBE_DESKTOP);
        }

        private void desktopAccessToolStripMenuItem_Click(object sender, EventArgs e)
        {
            subscribeCommon(channels.GetSelectedUser(), Subscription.SUBSCRIBE_DESKTOPINPUT);
        }

        private void allowDesktopAccessToolStripMenuItem_Click(object sender, EventArgs e)
        {
            desktopAccessToolStripMenuItem_Click(sender, e);
        }

        private void interceptUserMessagesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            subscribeCommon(channels.GetSelectedUser(), Subscription.SUBSCRIBE_INTERCEPT_USER_MSG);
        }

        private void interceptChannelMessagesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            subscribeCommon(channels.GetSelectedUser(), Subscription.SUBSCRIBE_INTERCEPT_CHANNEL_MSG);
        }

        private void interceptvoiceToolStripMenuItem_Click(object sender, EventArgs e)
        {
            subscribeCommon(channels.GetSelectedUser(), Subscription.SUBSCRIBE_INTERCEPT_VOICE);
        }

        private void interceptvideocaptureToolStripMenuItem_Click(object sender, EventArgs e)
        {
            subscribeCommon(channels.GetSelectedUser(), Subscription.SUBSCRIBE_INTERCEPT_VIDEOCAPTURE);
        }

        private void interceptDesktopToolStripMenuItem_Click(object sender, EventArgs e)
        {
            subscribeCommon(channels.GetSelectedUser(), Subscription.SUBSCRIBE_INTERCEPT_DESKTOP);
        }

        private void chanmsgTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
                chanmsgButton_Click(sender, e);
        }

        private void viewUserInformationToolStripMenuItem_Click(object sender, EventArgs e)
        {
            UserInfoDlg dlg = new UserInfoDlg(ttclient, channels.GetSelectedUser());
            dlg.ShowDialog();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            ClientStatistics tmp = new ClientStatistics();
            if (!ttclient.GetClientStatistics(ref tmp))
                return;

            double totalrx = (tmp.nUdpBytesRecv - statistics.nUdpBytesRecv) / 1024;
            double totaltx = (tmp.nUdpBytesSent - statistics.nUdpBytesSent) / 1024;
            double voicerx = (tmp.nVoiceBytesRecv - statistics.nVoiceBytesRecv) / 1024;
            double voicetx = (tmp.nVoiceBytesSent - statistics.nVoiceBytesSent) / 1024;
            double videorx = (tmp.nVideoCaptureBytesRecv - statistics.nVideoCaptureBytesRecv) / 1024;
            double videotx = (tmp.nVideoCaptureBytesSent - statistics.nVideoCaptureBytesSent) / 1024;
            double desktoprx = (tmp.nDesktopBytesRecv - statistics.nDesktopBytesRecv) / 1024;
            double desktoptx = (tmp.nDesktopBytesSent - statistics.nDesktopBytesSent) / 1024;
            double mediafilerx = (tmp.nMediaFileAudioBytesRecv + tmp.nMediaFileVideoBytesRecv
                                  - statistics.nMediaFileAudioBytesRecv + statistics.nMediaFileVideoBytesRecv) / 1024;
            double mediafiletx = (tmp.nMediaFileAudioBytesSent + tmp.nMediaFileVideoBytesSent
                                  - statistics.nMediaFileAudioBytesSent + statistics.nMediaFileVideoBytesSent) / 1024;
            statistics = tmp;

            //toolStripStatusLabel1.Text = String.Format("RX: {0:F} TX: {1:F} VoiceRX: {2:F} VoiceTX {3:F} VideoRX {4:F} VideoTX {5:F}",
            //                totalrx, totaltx, voicerx, voicetx, videorx, videotx);

            toolStripStatusLabel1.Text = String.Format("RX/TX Total: {0:F}/{1:F} Voice: {2:F}/{3:F} Video: {4:F}/{5:F} Desktop: {4:F}/{5:F} Media Files: {6:F}/{7:F} in KBytes",
                            totalrx, totaltx, voicerx, voicetx, videorx, videotx, desktoprx, desktoptx, mediafilerx, mediafiletx);
        }

        private void storeAudioToDiskToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (storeAudioToDiskToolStripMenuItem.Checked)
            {
                AudioStorageDlg dlg = new AudioStorageDlg(settings);
                if (dlg.ShowDialog() != DialogResult.OK)
                    return;

                if (settings.muxed_audio_file)
                {
                    if (ttclient.ChannelID > 0)
                        newMuxedAudioRecording();
                }
                else
                {
                    List<int> userids = users.GetUsers();
                    foreach (int id in userids)
                        ttclient.SetUserMediaStorageDir(id, settings.audiofolder, "", settings.aff);
                    //To store audio in other channels, log in as admin, call DoSubscribe(.,SUBSCRIBE_INTERCEPT_AUDIO) 
                    //and set SetUserMediaStorageDir() on the user
                }
            }
            else
            {
                settings.audiofolder = "";
                settings.muxed_audio_file = false;
                settings.aff = AudioFileFormat.AFF_NONE;
                //clear if single file
                ttclient.StopRecordingMuxedAudioFile();
                //clear if separate files
                List<int> userids = users.GetUsers();
                foreach (int id in userids)
                    ttclient.SetUserMediaStorageDir(id, "", "", settings.aff);
            }

            UpdateControls();
        }

        private void newMuxedAudioRecording()
        {
            ttclient.StopRecordingMuxedAudioFile();

            Channel chan = new Channel();
            if (!ttclient.GetChannel(ttclient.ChannelID, ref chan))
            {
                MessageBox.Show("Must be in a channel to start muxed audio recording");
                return;
            }

            //generate a filename for the new mux file.
            CultureInfo currentCulture = Thread.CurrentThread.CurrentCulture;
            Thread.CurrentThread.CurrentCulture = CultureInfo.CreateSpecificCulture("ja-JP");
            string timestamp = DateTime.Now.ToString();
            timestamp = timestamp.Replace(":", "");
            timestamp = timestamp.Replace("/", "");
            Thread.CurrentThread.CurrentCulture = currentCulture;
            string extension = "";
            switch (settings.aff)
            {
                case AudioFileFormat.AFF_MP3_64KBIT_FORMAT :
                case AudioFileFormat.AFF_MP3_128KBIT_FORMAT :
                case AudioFileFormat.AFF_MP3_256KBIT_FORMAT :
                    extension = ".mp3"; break;
                case AudioFileFormat.AFF_WAVE_FORMAT :
                    extension = ".wav";
                    break;
            }
            string filename = settings.audiofolder + "\\" + timestamp + " Conference" + extension;
            if (!ttclient.StartRecordingMuxedAudioFile(chan.audiocodec, filename, settings.aff))
                MessageBox.Show("Failed to create muxed audio file: " + filename);
            else
                AddStatusMessage("Recording to " + filename);
        }

        List<ToolStripItem> contextmenuitems = new List<ToolStripItem>();
        ToolStripMenuItem menu;

        private void contextMenuStrip1_Opening(object sender, CancelEventArgs e)
        {
            int userid = channels.GetSelectedUser();
            if (userid > 0)
            {
                menu = usersToolStripMenuItem;
                foreach(ToolStripItem item in usersToolStripMenuItem.DropDownItems)
                    contextmenuitems.Add(item);
            }
            else
            {
                menu = channelsToolStripMenuItem;
                foreach(ToolStripItem item in channelsToolStripMenuItem.DropDownItems)
                    contextmenuitems.Add(item);
            }
            foreach (ToolStripItem item in contextmenuitems)
                channelsContextMenuStrip.Items.Add(item);
            e.Cancel = false;
        }

        private void contextMenuStrip1_Closed(object sender, ToolStripDropDownClosedEventArgs e)
        {
            foreach (ToolStripItem item in contextmenuitems)
                menu.DropDownItems.Add(item);
            contextmenuitems.Clear();
        }

        private void broadcastTestMessageToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TextMessage msg;
            msg.nMsgType = TextMsgType.MSGTYPE_BROADCAST;
            msg.nChannelID = 0;
            msg.nFromUserID = ttclient.GetMyUserID();
            msg.szFromUsername = ""; //not required
            msg.nToUserID = 0;
            msg.szMessage = InputBox.Get("Broadcast message", "Message");
            ttclient.DoTextMessage(msg);
        }

        private void filesContextMenuStrip_Opened(object sender, EventArgs e)
        {
            downloadFileToolStripMenuItem.Enabled = files.GetSelectedFile() > 0;
            uploadFileToolStripMenuItem.Enabled = files.GetSelectedChannel() > 0;
            deleteFileToolStripMenuItem.Enabled = files.GetSelectedFile() > 0;
        }

        private void uploadFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog openFileDialog1 = new OpenFileDialog();

            openFileDialog1.RestoreDirectory = true;
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                ttclient.DoSendFile(files.GetSelectedChannel(), openFileDialog1.FileName);
            }
        }

        private void downloadFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int fileid = files.GetSelectedFile();
            int channelid = files.GetSelectedChannel();

            RemoteFile file = new RemoteFile();
            if (!ttclient.GetChannelFile(channelid, fileid, ref file))
                return;

            SaveFileDialog saveFileDialog1 = new SaveFileDialog();
            saveFileDialog1.FileName = file.szFileName;
            saveFileDialog1.RestoreDirectory = true;

            if (saveFileDialog1.ShowDialog() == DialogResult.OK)
                ttclient.DoRecvFile(channelid, fileid, saveFileDialog1.FileName);
        }

        private void deleteFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ttclient.DoDeleteFile(files.GetSelectedChannel(), files.GetSelectedFile());
        }

        void ttclient_OnFileTransfer(FileTransfer filetransfer)
        {
            if (filetransfer.nStatus == FileTransferStatus.FILETRANSFER_ACTIVE)
                new FileTransferDlg(ttclient, filetransfer.nTransferID).Show();
        }

        private void vidtxCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (ttclient.Flags.HasFlag(ClientFlag.CLIENT_VIDEOCAPTURE_READY))
            {
                if(vidtxCheckBox.Checked)
                {
                    VideoCodec vidcodec = new VideoCodec();
                    vidcodec.nCodec = Codec.WEBM_VP8_CODEC;
                    vidcodec.webm_vp8.nRcTargetBitrate = 0;
                    ttclient.StartVideoCaptureTransmission(vidcodec);
                }
                else
                    ttclient.StopVideoCaptureTransmission();
            }
            else
            {
                MessageBox.Show("Video device hasn't been configured in Preferences");
                vidtxCheckBox.Checked = false;
            }
            UpdateControls();
        }

        private void pttCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            //Note that it's not advised to enable hotkeys when debugging.
            //The debugger gets REALLY slow
            if (pttCheckBox.Checked)
            {
                HotKeyDlg dlg = new HotKeyDlg(ttclient);
                dlg.ShowDialog();
                ttclient.HotKey_Register((int)HotKey.HOTKEY_PUSHTOTALK, dlg.keys.ToArray());
                //OnHotKeyToggle is called when hotkey become active
            }
            else
            {
                ttclient.HotKey_Unregister((int)HotKey.HOTKEY_PUSHTOTALK);
            }
            UpdateControls();
        }

        private void voxtxCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            voiceactTrackBar.Enabled = voxtxCheckBox.Checked;
            ttclient.EnableVoiceActivation(voxtxCheckBox.Checked);
            if (!voxtxCheckBox.Checked)
                channels.ttclient_OnVoiceActivation(false);
        }

        private void mutevoiceToolStripMenuItem_Click(object sender, EventArgs e)
        {
            User user = new User();
            if(ttclient.GetUser(channels.GetSelectedUser(), ref user))
                ttclient.SetUserMute(user.nUserID, StreamType.STREAMTYPE_VOICE, !user.uUserState.HasFlag(UserState.USERSTATE_MUTE_VOICE));
        }

        private void allowVoiceTransmissionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            User user = new User();
            Channel chan = new Channel();
            if (ttclient.GetUser(channels.GetSelectedUser(), ref user) && 
                ttclient.GetChannel(user.nChannelID, ref chan))
            {
                if(allowVoiceTransmissionToolStripMenuItem.Checked)
                {
                    if (chan.GetTransmitStreamTypes(user.nUserID) == StreamType.STREAMTYPE_NONE &&
                        chan.GetTransmitUserCount() >= TeamTalk.TT_TRANSMITUSERS_MAX)
                        MessageBox.Show("Maximum users to transmit is " + TeamTalk.TT_TRANSMITUSERS_MAX.ToString());
                    else
                        chan.AddTransmitUser(user.nUserID, StreamType.STREAMTYPE_VOICE);
                }
                else
                    chan.RemoveTransmitUser(user.nUserID, StreamType.STREAMTYPE_VOICE);

                ttclient.DoUpdateChannel(chan);
            }
        }

        private void allowVideoTransmissionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            User user = new User();
            Channel chan = new Channel();
            if (ttclient.GetUser(channels.GetSelectedUser(), ref user) &&
                ttclient.GetChannel(user.nChannelID, ref chan))
            {
                if (allowVideoTransmissionToolStripMenuItem.Checked)
                {
                    if (chan.GetTransmitStreamTypes(user.nUserID) == StreamType.STREAMTYPE_NONE &&
                        chan.GetTransmitUserCount() >= TeamTalk.TT_TRANSMITUSERS_MAX)
                        MessageBox.Show("Maximum users to transmit is " + TeamTalk.TT_TRANSMITUSERS_MAX.ToString());
                    else
                        chan.AddTransmitUser(user.nUserID, StreamType.STREAMTYPE_VIDEOCAPTURE);
                }
                else
                    chan.RemoveTransmitUser(user.nUserID, StreamType.STREAMTYPE_VIDEOCAPTURE);

                ttclient.DoUpdateChannel(chan);
            }
        }

        private void serverStatisticsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            new ServerStatsDlg(ttclient).ShowDialog();
        }

        private void treeView1_ItemDrag(object sender, ItemDragEventArgs e)
        {
            TreeNode item = (TreeNode)e.Item;
            if (item.ImageIndex == (int)ChannelsView.ImageIndex.USER)
                treeView1.DoDragDrop(item, DragDropEffects.Move);
        }

        private void treeView1_DragEnter(object sender, DragEventArgs e)
        {
            e.Effect = DragDropEffects.Move;
        }

        private void treeView1_DragDrop(object sender, DragEventArgs e)
        {
            Point p = ((TreeView)sender).PointToClient(new Point(e.X, e.Y));
            TreeNode targetNode = ((TreeView)sender).GetNodeAt(p);
            if (targetNode == null)
                return;
            int chanid = 0;
            switch (targetNode.ImageIndex)
            {
                case (int)ChannelsView.ImageIndex.CHANNEL:
                    chanid = (int)targetNode.Tag;
                    break;
                case (int)ChannelsView.ImageIndex.USER:
                    User user = new User();
                    if(ttclient.GetUser((int)targetNode.Tag, ref user))
                        chanid = user.nChannelID;
                    break;
            }
            if (chanid == 0)
                return;

            TreeNode sourceNode = (TreeNode)e.Data.GetData("System.Windows.Forms.TreeNode");
            ttclient.DoMoveUser((int)sourceNode.Tag, chanid);
        }

        private void enableDesktopSharingToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (ttclient.Flags.HasFlag(ClientFlag.CLIENT_DESKTOP_ACTIVE))
            {
                ttclient.CloseDesktopWindow();
                UpdateControls();

                senddesktopTimer.Enabled = false;
                sendcursorTimer.Enabled = false;
                return;
            }

            DesktopShareDlg dlg = new DesktopShareDlg();
            if (dlg.ShowDialog() != DialogResult.OK)
                return;

            //'hShareWnd' == IntPtr.Zero means share active window
            hShareWnd = dlg.hShareWnd;
            bmpShareFormat = dlg.bmpformat;

            if (sendDesktopWindow())
            {
                if (dlg.update_interval > 0)
                {
                    senddesktopTimer.Interval = dlg.update_interval;
                    senddesktopTimer.Enabled = true;
                }
                //send desktop cursor every 50 msec
                if (dlg.share_cursor)
                {
                    sendcursorTimer.Interval = 50;
                    sendcursorTimer.Enabled = true;
                }
            }
            else
                MessageBox.Show("Failed to send shared window", "Desktop Sharing Error");

            UpdateControls();
        }

        bool sendDesktopWindow()
        {
            //figure out which window to share
            IntPtr hWnd;
            if (hShareWnd == IntPtr.Zero)
                hWnd = WindowsHelper.GetDesktopActiveHWND();
            else
                hWnd = hShareWnd;

            //don't try to send new bitmap if one is already being transmitted
            if ((ttclient.Flags & ClientFlag.CLIENT_TX_DESKTOP) == ClientFlag.CLIENT_CLOSED)
                return ttclient.SendDesktopWindowFromHWND(hWnd, bmpShareFormat,
                                                          DesktopProtocol.DESKTOPPROTOCOL_ZLIB_1)>0;
            return false;
        }

        private void senddesktopTimer_Tick(object sender, EventArgs e)
        {
            sendDesktopWindow();
        }

        private void sendcursorTimer_Tick(object sender, EventArgs e)
        {
            ttclient.SendDesktopCursorPosition((ushort)Cursor.Position.X, (ushort)Cursor.Position.Y);
        }

        private void streamMediaFileToChannelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if ((ttclient.Flags & (ClientFlag.CLIENT_STREAM_AUDIO | ClientFlag.CLIENT_STREAM_VIDEO)) != ClientFlag.CLIENT_CLOSED)
            {
                ttclient.StopStreamingMediaFileToChannel();
                streamMediaFileToChannelToolStripMenuItem.Checked = false;
            }
            else
                new MediaFileDlg(ttclient).ShowDialog();
        }

        void ttclient_OnStreamMediaFile(MediaFileInfo mediafileinfo)
        {
            switch (mediafileinfo.nStatus)
            {
                case MediaFileStatus.MFS_ABORTED:
                    AddStatusMessage("Aborted media file stream to channel");
                    break;
                case MediaFileStatus.MFS_ERROR:
                    AddStatusMessage("Error in media file stream to channel");
                    break;
                case MediaFileStatus.MFS_FINISHED:
                    AddStatusMessage("Finished media file stream to channel");
                    break;
                case MediaFileStatus.MFS_STARTED:
                    AddStatusMessage("Started media file stream to channel");
                    break;
            }
            streamMediaFileToChannelToolStripMenuItem.Checked = mediafileinfo.nStatus == MediaFileStatus.MFS_STARTED;
        }

        private void voicetxCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            ttclient.EnableVoiceTransmission(voicetxCheckBox.Checked);
        }

        //container of users' past key-down events
        // userid -> [key-code, DesktopInput]
        Dictionary<int, Dictionary<uint, DesktopInput>> desktopinputs = new Dictionary<int, Dictionary<uint, DesktopInput>>();

        void ttclient_OnUserDesktopInput(int nSrcUserID, DesktopInput desktopinput)
        {
            DesktopInput[] inputs = new DesktopInput[] { desktopinput }, trans_inputs = null;
            //assumes desktop input is received in TTKEYCODE format
            WindowsHelper.DesktopInputKeyTranslate(TTKeyTranslate.TTKEY_TTKEYCODE_TO_WINKEYCODE,
                                       inputs, out trans_inputs);
            WindowsHelper.DesktopInputExecute(trans_inputs);

            //create new (or find existing) list of desktop inputs from user
            Dictionary<uint, DesktopInput> pastinputs;
            if (!desktopinputs.TryGetValue(nSrcUserID, out pastinputs))
            {
                pastinputs = new Dictionary<uint, DesktopInput>();
                desktopinputs.Add(nSrcUserID, pastinputs);
            }

            //only store key-down events and remove previous key-down events if 
            //the keys have been released
            foreach (DesktopInput input in trans_inputs)
            {
                if (input.uKeyState == DesktopKeyState.DESKTOPKEYSTATE_DOWN)
                    pastinputs.Add(input.uKeyCode, input);
                else if (input.uKeyState == DesktopKeyState.DESKTOPKEYSTATE_UP)
                    pastinputs.Remove(input.uKeyCode);
            }

            //if no keys are held by user then remove the user
            if (pastinputs.Count == 0)
                desktopinputs.Remove(nSrcUserID);
        }

        //release keys which have been held down by user
        void closeUserDesktopInput(int nUserID)
        {
            Dictionary<uint, DesktopInput> pastinputs;
            if (!desktopinputs.TryGetValue(nUserID, out pastinputs))
                return;

            DesktopInput[] inputs = new DesktopInput[pastinputs.Count];
            int i=0;
            foreach (KeyValuePair<uint, DesktopInput> pair in pastinputs)
            {
                //invert key-down event so it's now a key up event
                //(and all keys are release by the user)
                DesktopInput input = pair.Value;
                input.uKeyState = DesktopKeyState.DESKTOPKEYSTATE_UP;
                inputs[i++] = input;
            }
            WindowsHelper.DesktopInputExecute(inputs);
        }
    }
}
