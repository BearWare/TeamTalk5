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
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.Diagnostics;
using BearWare;

namespace TeamTalkApp.NET
{
    class ChannelsView
    {
        TeamTalkBase ttclient;
        TreeView treeview;

        public enum ImageIndex
        {
            CHANNEL,
            USER
        }

        public ChannelsView(TeamTalkBase tt, TreeView tree)
        {
            ttclient = tt;
            treeview = tree;

            ttclient.OnCmdChannelNew += new TeamTalkBase.ChannelUpdate(ttclient_OnCmdChannelNew);
            ttclient.OnCmdChannelUpdate += new TeamTalkBase.ChannelUpdate(ttclient_OnCmdChannelUpdate);
            ttclient.OnCmdChannelRemove += new TeamTalkBase.ChannelUpdate(ttclient_OnCmdChannelRemove);

            ttclient.OnCmdUserJoinedChannel += new TeamTalkBase.UserUpdate(ttclient_OnCmdUserJoinedChannel);
            ttclient.OnCmdUserUpdate += new TeamTalkBase.UserUpdate(ttclient_OnCmdUserUpdate);
            ttclient.OnCmdUserLeftChannel += new TeamTalkBase.UserUpdate(ttclient_OnCmdUserLeftChannel);

            ttclient.OnUserStateChange += new TeamTalkBase.UserUpdate(ttclient_OnUserStateChange);
            ttclient.OnVoiceActivation += new TeamTalkBase.VoiceActivation(ttclient_OnVoiceActivation);
        }

        public TreeNode GetTreeNode(int id, int img_type)
        {
            Queue<TreeNode> queue = new Queue<TreeNode>();
            foreach (TreeNode node in treeview.Nodes)
                queue.Enqueue(node);

            while (queue.Count > 0)
            {
                TreeNode node = queue.Dequeue();
                if (node.ImageIndex == (int)img_type)
                    if ((int)node.Tag == id)
                        return node;

                foreach (TreeNode n in node.Nodes)
                {
                    queue.Enqueue(n);
                }
            }
            return null;
        }

        public TreeNode GetChannel(int channelid)
        {
            return GetTreeNode(channelid, (int)ImageIndex.CHANNEL);
        }

        public TreeNode GetUser(int userid)
        {
            return GetTreeNode(userid, (int)ImageIndex.USER);
        }

        public int GetSelectedChannel()
        {
            if (treeview.SelectedNode != null &&
               treeview.SelectedNode.ImageIndex == (int)ImageIndex.CHANNEL)
                return (int)treeview.SelectedNode.Tag;
            return 0;
        }

        public int GetSelectedUser()
        {
            if (treeview.SelectedNode != null &&
               treeview.SelectedNode.ImageIndex == (int)ImageIndex.USER)
                return (int)treeview.SelectedNode.Tag;
            return 0;
        }

        void ttclient_OnCmdChannelNew(Channel chan)
        {
            TreeNode parent = GetChannel(chan.nParentID);
            if (parent == null)
            {
                //it's the root channel

                //user servername as root channel name
                ServerProperties prop = new ServerProperties();
                ttclient.GetServerProperties(ref prop);
                TreeNode newnode = new TreeNode(prop.szServerName, (int)ImageIndex.CHANNEL, (int)ImageIndex.CHANNEL);
                newnode.Tag = chan.nChannelID;
                treeview.Nodes.Add(newnode);
            }
            else
            {
                TreeNode newnode = new TreeNode(chan.szName, (int)ImageIndex.CHANNEL, (int)ImageIndex.CHANNEL);
                newnode.Tag = chan.nChannelID;
                parent.Nodes.Add(newnode);
            }
        }

        void ttclient_OnCmdChannelUpdate(Channel chan)
        {
            if ((chan.uChannelType & ChannelType.CHANNEL_CLASSROOM) == ChannelType.CHANNEL_DEFAULT)
                return;

            TreeNode chanNode = GetChannel(chan.nChannelID);
            Debug.Assert(chanNode != null);

            List<TreeNode> users = new List<TreeNode>();
            foreach (TreeNode userNode in chanNode.Nodes)
            {
                if (userNode.ImageIndex == (int)ImageIndex.USER)
                    users.Add(userNode);
            }
            foreach (TreeNode userNode in users)
            {
                User user = new User();
                if (ttclient.GetUser((int)userNode.Tag, ref user))
                {
                    userNode.Text = String.Format("{0} [Voice={1}, Video={2}, Desktop={3}, MediaFile={4}]",
                        user.szNickname,
                        chan.GetTransmitStreamTypes(user.nUserID).HasFlag(StreamType.STREAMTYPE_VOICE),
                        chan.GetTransmitStreamTypes(user.nUserID).HasFlag(StreamType.STREAMTYPE_VIDEOCAPTURE),
                        chan.GetTransmitStreamTypes(user.nUserID).HasFlag(StreamType.STREAMTYPE_DESKTOP),
                        chan.GetTransmitStreamTypes(user.nUserID).HasFlag(StreamType.STREAMTYPE_MEDIAFILE_AUDIO | StreamType.STREAMTYPE_MEDIAFILE_VIDEO));
                }
            }
        }

        void ttclient_OnCmdChannelRemove(Channel chan)
        {
            treeview.Nodes.Remove(GetChannel(chan.nChannelID));
        }

        void ttclient_OnCmdUserJoinedChannel(User user)
        {
            TreeNode node = GetChannel(user.nChannelID);
            if (node == null)
                return;
            TreeNode newnode = new TreeNode(user.szNickname, (int)ImageIndex.USER, (int)ImageIndex.USER);
            newnode.Tag = user.nUserID;
            node.Nodes.Add(newnode);
        }

        void ttclient_OnCmdUserUpdate(User user)
        {
            TreeNode node = GetUser(user.nUserID);
            if (node == null)
                return;
            node.Text = user.szNickname;
        }

        void ttclient_OnCmdUserLeftChannel(User user)
        {
            treeview.Nodes.Remove(GetUser(user.nUserID));
        }

        void ttclient_OnUserStateChange(User user)
        {
            TreeNode node = GetUser(user.nUserID);
            if(node != null)
                node.BackColor = user.uUserState.HasFlag(UserState.USERSTATE_VOICE | UserState.USERSTATE_MEDIAFILE_AUDIO) ? Color.Green : Color.Transparent;
        }

        public void ttclient_OnVoiceActivation(bool bVoiceActive)
        {
            TreeNode node = GetUser(ttclient.GetMyUserID());
            if (node != null)
            {
                if (bVoiceActive || ttclient.Flags.HasFlag(ClientFlag.CLIENT_TX_VOICE))
                    node.BackColor = Color.Green;
                else
                    node.BackColor = Color.Transparent;
            }
        }
    }
}
