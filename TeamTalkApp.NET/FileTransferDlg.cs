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
    public partial class FileTransferDlg : Form
    {
        TeamTalk ttclient;
        int transferid;
        public FileTransferDlg(TeamTalk tt, int transferid)
        {
            this.ttclient = tt;
            this.transferid = transferid;
            InitializeComponent();

            ttclient.OnFileTransfer += new TeamTalk.FileTransferUpdate(ttclient_OnFileTransfer);
            UpdateFileTransfer();
        }

        void ttclient_OnFileTransfer(FileTransfer filetransfer)
        {
            if (filetransfer.nTransferID == transferid)
            {
                if (filetransfer.nStatus == FileTransferStatus.FILETRANSFER_FINISHED)
                {
                    progressBar1.Value = progressBar1.Maximum;
                    timer1.Enabled = false;
                }
                else if (filetransfer.nStatus == FileTransferStatus.FILETRANSFER_ERROR)
                {
                    timer1.Enabled = false;
                }
            }
        }

        void UpdateFileTransfer()
        {
            FileTransfer transfer;
            if (ttclient.GetFileTransferInfo(transferid, out transfer))
            {
                if (transfer.nTransferred == transfer.nFileSize)
                    timer1.Enabled = false;
                filenameTextBox.Text = transfer.szRemoteFileName;
                filesizeTextBox.Text = transfer.nFileSize.ToString();

                if (transfer.nFileSize > 0)
                {
                    float percent = (float)transfer.nTransferred / (float)transfer.nFileSize;
                    progressBar1.Value = (int)(percent * 100.0f);
                }
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            UpdateFileTransfer();
        }

        private void FileTransferDlg_FormClosing(object sender, FormClosingEventArgs e)
        {
            FileTransfer transfer;
            if (ttclient.GetFileTransferInfo(transferid, out transfer))
                ttclient.CancelFileTranfer(transferid);
        }
    }
}