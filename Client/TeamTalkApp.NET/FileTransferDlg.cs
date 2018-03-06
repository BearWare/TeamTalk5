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
        TeamTalkBase ttclient;
        int transferid;
        public FileTransferDlg(TeamTalkBase tt, int transferid)
        {
            this.ttclient = tt;
            this.transferid = transferid;
            InitializeComponent();

            ttclient.OnFileTransfer += new TeamTalkBase.FileTransferUpdate(ttclient_OnFileTransfer);
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
