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
            FileTransfer transfer = new FileTransfer();
            if (ttclient.GetFileTransferInfo(transferid, ref transfer))
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
            FileTransfer transfer = new FileTransfer();
            if (ttclient.GetFileTransferInfo(transferid, ref transfer))
                ttclient.CancelFileTranfer(transferid);
        }
    }
}
