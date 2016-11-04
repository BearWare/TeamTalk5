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
using BearWare;

namespace TeamTalkApp.NET
{
    public partial class UserAccountsDlg : Form
    {
        TeamTalk ttclient;
        int create_cmdid;
        int list_cmdid;
        int del_cmdid;

        UserAccount account_create;
        List<UserAccount> all_accounts = new List<UserAccount>();

        const UserRight DEFAULT_USERRIGHTS = (UserRight.USERRIGHT_MULTI_LOGIN |   
                            UserRight.USERRIGHT_VIEW_ALL_USERS |            
                            UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL |  
                            UserRight.USERRIGHT_UPLOAD_FILES |              
                            UserRight.USERRIGHT_DOWNLOAD_FILES |            
                            UserRight.USERRIGHT_TRANSMIT_VOICE |            
                            UserRight.USERRIGHT_TRANSMIT_VIDEOCAPTURE |            
                            UserRight.USERRIGHT_TRANSMIT_DESKTOP |          
                            UserRight.USERRIGHT_TRANSMIT_DESKTOPINPUT |     
                            UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO |  
                            UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO);

        public UserAccountsDlg(TeamTalk tt)
        {
            ttclient = tt;
            create_cmdid = 0;
            del_cmdid = 0;

            InitializeComponent();
            this.CenterToScreen();

            listView1.Columns.Add("Username");
            listView1.Columns.Add("Password");
            listView1.Columns.Add("User Type");
            listView1.Columns.Add("UserData");
            listView1.Columns.Add("Note");

            Channel[] channels;
            if (ttclient.GetServerChannels(out channels))
            {
                foreach (Channel chan in channels)
                {
                    string channel = "";
                    //auto-op only works for static channels
                    if (chan.uChannelType.HasFlag(ChannelType.CHANNEL_PERMANENT) &&
                        ttclient.GetChannelPath(chan.nChannelID, ref channel))
                        availchanListBox.Items.Add(channel);
                }
            }

            ttclient.OnCmdProcessing += new TeamTalk.CommandProcessing(ttclient_OnCmdProcessing);
            ttclient.OnCmdError += new TeamTalk.CommandError(ttclient_OnCmdError);
            ttclient.OnCmdSuccess += new TeamTalk.CommandSuccess(ttclient_OnCmdSuccess);
            ttclient.OnCmdUserAccount += new TeamTalk.ListUserAccount(ttclient_OnCmdUserAccount);

            //hopefully you have less than 100000 accounts
            list_cmdid = ttclient.DoListUserAccounts(0, 100000);
        }

        void ttclient_OnCmdUserAccount(UserAccount useraccount)
        {
            AddAccount(useraccount);
        }

        void ClearControls()
        {
            usernameTextBox.Text = "";
            passwordTextBox.Text = "";
            defuserRadioButton.Checked = true;
            userdataTextBox.Text = "0";
            noteTextBox.Text = "";
            initchanTextBox.Text = "";
            selchanListBox.Items.Clear();
            ShowUserRights(DEFAULT_USERRIGHTS);
        }

        void EnableControls(bool enable)
        {
            createButton.Enabled = enable;
            usernameTextBox.Enabled = enable;
            passwordTextBox.Enabled = enable;
            defuserRadioButton.Enabled = enable;
            adminRadioButton.Enabled = enable;
            noteTextBox.Enabled = enable;
            userdataTextBox.Enabled = enable;
            initchanTextBox.Enabled = enable;
            availchanListBox.Enabled = enable;
            selchanListBox.Enabled = enable;
            userrightsGroupBox.Enabled = enable;
        }

        void ShowUserRights(UserRight rights)
        {
            multiloginCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_MULTI_LOGIN);
            viewallCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_VIEW_ALL_USERS);
            permchannelsCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_MODIFY_CHANNELS);
            tempchanCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL);
            bcastCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_TEXTMESSAGE_BROADCAST);
            kickCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_KICK_USERS);
            banCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_BAN_USERS);
            moveuserCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_MOVE_USERS);
            operatorenableCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_OPERATOR_ENABLE);
            uploadCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_UPLOAD_FILES);
            downloadCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_DOWNLOAD_FILES);
            srvupdCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_UPDATE_SERVERPROPERTIES);
            voicetxCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_TRANSMIT_VOICE);
            vidcapCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_TRANSMIT_VIDEOCAPTURE);
            audfilesCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO);
            vidfileCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO);
            desktopCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_TRANSMIT_DESKTOP);
            desktopinputCheckBox.Checked = rights.HasFlag(UserRight.USERRIGHT_TRANSMIT_DESKTOPINPUT);
        }

        void ttclient_OnCmdProcessing(int nCmdID, bool bActive)
        {
            if (create_cmdid == nCmdID && !bActive)
            {
                EnableControls(true);
                ClearControls();
            }
            else if (list_cmdid == nCmdID && !bActive)
            {
            }
            else if (del_cmdid == nCmdID && !bActive)
            {
                listView1.Enabled = true;
                if (listView1.SelectedItems.Count > 0)
                {
                    listView1.Items.Remove(listView1.SelectedItems[0]);
                }
            }
        }

        void ttclient_OnCmdSuccess(int nCmdID)
        {
            if (nCmdID == create_cmdid)
            {
                RemoveAccount(account_create.szUsername);
                AddAccount(account_create);
            }
            if (nCmdID == del_cmdid)
            {
                if(listView1.SelectedItems.Count>0)
                    RemoveAccount(listView1.SelectedItems[0].SubItems[0].Text);
            }
        }

        void ttclient_OnCmdError(int nCmdID, ClientErrorMsg clienterrormsg)
        {
            //clear create account if unsuccessful
            if (nCmdID == create_cmdid)
            {
                account_create = new UserAccount();
                MessageBox.Show(clienterrormsg.szErrorMsg);
            }
        }

        private void createButton_Click(object sender, EventArgs e)
        {
            account_create = new UserAccount(); ;
            account_create.szUsername = usernameTextBox.Text;
            account_create.szPassword = passwordTextBox.Text;
            if(adminRadioButton.Checked)
                account_create.uUserType = UserType.USERTYPE_ADMIN;
            else
                account_create.uUserType = UserType.USERTYPE_DEFAULT;

            if (multiloginCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_MULTI_LOGIN;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_MULTI_LOGIN;

            if (viewallCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_VIEW_ALL_USERS;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_VIEW_ALL_USERS;

            if (permchannelsCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_MODIFY_CHANNELS;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_MODIFY_CHANNELS;

            if (tempchanCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL;

            if (bcastCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_TEXTMESSAGE_BROADCAST;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_TEXTMESSAGE_BROADCAST;

            if (kickCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_KICK_USERS;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_KICK_USERS;

            if(banCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_BAN_USERS;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_BAN_USERS;

            if(moveuserCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_MOVE_USERS;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_MOVE_USERS;

            if (operatorenableCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_OPERATOR_ENABLE;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_OPERATOR_ENABLE;

            if (uploadCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_UPLOAD_FILES;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_UPLOAD_FILES;

            if (downloadCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_DOWNLOAD_FILES;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_DOWNLOAD_FILES;

            if (srvupdCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_UPDATE_SERVERPROPERTIES;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_UPDATE_SERVERPROPERTIES;

            if (voicetxCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_TRANSMIT_VOICE;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_TRANSMIT_VOICE;

            if (vidcapCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_TRANSMIT_VIDEOCAPTURE;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_TRANSMIT_VIDEOCAPTURE;

            if (audfilesCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO;

            if (vidfileCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO;
                
            if (desktopCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_TRANSMIT_DESKTOP;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_TRANSMIT_DESKTOP;

            if(desktopinputCheckBox.Checked)
                account_create.uUserRights |= UserRight.USERRIGHT_TRANSMIT_DESKTOPINPUT;
            else
                account_create.uUserRights &= ~UserRight.USERRIGHT_TRANSMIT_DESKTOPINPUT;

            account_create.szNote = noteTextBox.Text;
            account_create.nUserData = int.Parse(userdataTextBox.Text);
            account_create.szInitChannel = initchanTextBox.Text;
            account_create.nAudioCodecBpsLimit = (int)bitrateNumericUpDown.Value * 1000;
            
            account_create.autoOperatorChannels = new int[TeamTalk.TT_CHANNELS_OPERATOR_MAX];
            int c = 0;
            //process auto-operator channels
            foreach (string channel in selchanListBox.Items)
            {
                int id = ttclient.GetChannelIDFromPath(channel);
                if(id>0)
                    account_create.autoOperatorChannels[c++] = id;
            }


            create_cmdid = ttclient.DoNewUserAccount(account_create);
            if (create_cmdid > 0)
            {
                EnableControls(false);
            }
        }

        private void deleteAccountToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (listView1.SelectedItems.Count > 0)
            {
                del_cmdid = ttclient.DoDeleteUserAccount(listView1.SelectedItems[0].Text);
                listView1.Enabled = false;
            }
        }

        void AddAccount(UserAccount account)
        {
            ListViewItem item = new ListViewItem();
            item.Text = account.szUsername;
            item.SubItems.Add(account.szPassword);
            item.SubItems.Add((account.uUserType & UserType.USERTYPE_ADMIN) == UserType.USERTYPE_ADMIN ? "Admin" : "Default");
            item.SubItems.Add(account.nUserData.ToString());
            item.SubItems.Add(account.szNote);
            listView1.Items.Add(item);

            all_accounts.Add(account);
        }


        void RemoveAccount(string username)
        {
            for (int i = 0; i < all_accounts.Count; i++)
            {
                if (all_accounts[i].szUsername == username)
                {
                    all_accounts.RemoveAt(i);
                    break;
                }
            }
            foreach (ListViewItem item in listView1.Items)
            {
                if (item.SubItems[0].Text == username)
                    listView1.Items.Remove(item);
            }
        }

        private void addChanButton_Click(object sender, EventArgs e)
        {
            if(!selchanListBox.Items.Contains(availchanListBox.SelectedItem))
                selchanListBox.Items.Add(availchanListBox.SelectedItem);
        }

        private void delChanButton_Click(object sender, EventArgs e)
        {
            selchanListBox.Items.Remove(selchanListBox.SelectedItem);
        }

        private void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if(listView1.SelectedItems.Count == 0)
                return;
            ClearControls();

            string username = listView1.SelectedItems[0].SubItems[0].Text;
            foreach (UserAccount acc in all_accounts)
            {
                if (acc.szUsername == username)
                {
                    usernameTextBox.Text = acc.szUsername;
                    passwordTextBox.Text = acc.szPassword;
                    defuserRadioButton.Checked = (acc.uUserType & UserType.USERTYPE_DEFAULT) == UserType.USERTYPE_DEFAULT;
                    adminRadioButton.Checked = (acc.uUserType & UserType.USERTYPE_ADMIN) == UserType.USERTYPE_ADMIN;
                    userdataTextBox.Text = acc.nUserData.ToString();
                    noteTextBox.Text = acc.szNote;
                    initchanTextBox.Text = acc.szInitChannel;
                    bitrateNumericUpDown.Value = acc.nAudioCodecBpsLimit / 1000;
                    foreach (int id in acc.autoOperatorChannels)
                    {
                        string channel = "";
                        if (ttclient.GetChannelPath(id, ref channel))
                            selchanListBox.Items.Add(channel);
                    }


                    UserRight rights = acc.uUserRights;
                    ShowUserRights(rights);
                    break;
                }
            }
        }

        private void clrButton_Click(object sender, EventArgs e)
        {
            ClearControls();
        }

        private void adminRadioButton_CheckedChanged(object sender, EventArgs e)
        {
            userrightsGroupBox.Enabled = !adminRadioButton.Checked;
        }
    }
}
