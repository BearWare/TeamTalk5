using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

using BearWare;

namespace TeamTalkApp.NET
{
    class FilesView
    {
        TeamTalk ttclient;
        ListView listview;

        public FilesView(TeamTalk tt, ListView list)
        {
            this.ttclient = tt;
            this.listview = list;

            listview.Columns.Add("File ID");
            listview.Columns.Add("Filename");
            listview.Columns.Add("Size");
            listview.Columns.Add("Owner");

            ttclient.OnCmdFileNew += new TeamTalk.FileUpdate(ttclient_OnCmdFileNew);
            ttclient.OnCmdFileRemove += new TeamTalk.FileUpdate(ttclient_OnCmdFileRemove);
            ttclient.OnCmdUserJoinedChannel += new TeamTalk.UserUpdate(ttclient_OnCmdUserJoinedChannel);
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
