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
using System.Text;
using System.Windows.Forms;

using BearWare;

namespace TeamTalkApp.NET
{
    class FilesView
    {
        TeamTalkBase ttclient;
        ListView listview;

        public FilesView(TeamTalkBase tt, ListView list)
        {
            this.ttclient = tt;
            this.listview = list;

            listview.Columns.Add("File ID");
            listview.Columns.Add("Filename");
            listview.Columns.Add("Size");
            listview.Columns.Add("Owner");

            ttclient.OnCmdFileNew += new TeamTalkBase.FileUpdate(ttclient_OnCmdFileNew);
            ttclient.OnCmdFileRemove += new TeamTalkBase.FileUpdate(ttclient_OnCmdFileRemove);
            ttclient.OnCmdUserJoinedChannel += new TeamTalkBase.UserUpdate(ttclient_OnCmdUserJoinedChannel);
        }

        public int GetSelectedFile()
        {
            if (listview.SelectedItems.Count == 0)
                return 0;
            return (int)listview.SelectedItems[0].Tag;
        }

        public int GetSelectedChannel()
        {
            return ttclient.GetMyChannelID(); //for now just "my" channel
        }

        void ttclient_OnCmdUserJoinedChannel(User user)
        {
            if(user.nUserID == ttclient.UserID)
                listview.Items.Clear(); //ensure we don't get dublicates if joining again
        }

        void ttclient_OnCmdFileNew(RemoteFile remotefile)
        {
            if (ttclient.GetMyChannelID() != remotefile.nChannelID)
                return;//an admin can see all files but we don't show it here (for now)

            ListViewItem item = new ListViewItem();
            item.Tag = remotefile.nFileID;
            item.Text = remotefile.nFileID.ToString();
            item.SubItems.Add(remotefile.szFileName);
            item.SubItems.Add(remotefile.nFileSize.ToString());
            item.SubItems.Add(remotefile.szUsername);
            listview.Items.Add(item);
        }

        void ttclient_OnCmdFileRemove(RemoteFile remotefile)
        {
            if (ttclient.GetMyChannelID() != remotefile.nChannelID)
                return;

            foreach (ListViewItem item in listview.Items)
                if ((int)item.Tag == remotefile.nFileID)
                    listview.Items.Remove(item);
        }
    }
}
