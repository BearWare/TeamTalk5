/*
 * Copyright (c) 2005-2016, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
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
using BearWare;

namespace TeamTalkApp.NET
{
    public partial class VideoDlg : Form
    {
        TeamTalk ttclient;
        int userid;
        Bitmap bmp;
        int unsubscribe_cmdid = 0;

        public VideoDlg(TeamTalk tt, int userid)
        {
            ttclient = tt;
            this.userid = userid;
            InitializeComponent();
            this.CenterToScreen();

            User user = new User();
            if (userid > 0 && ttclient.GetUser(userid, ref user))
                this.Text = "Video - " + user.szNickname;
            else
                this.Text = "Local Video";

            ttclient.OnUserVideoCapture += new TeamTalk.UserVideoFrame(ttclient_OnUserVideoFrame);
        }

        private void VideoDlg_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (this.userid > 0 && unsubscribe_cmdid == 0)
            {
                if (MessageBox.Show("Do you wish to stop receiving video from this user?", "Close Video",
                    MessageBoxButtons.YesNo) == DialogResult.Yes)
                {
                    //we wait for the server to repond to us that we'll no 
                    //longer receive video from this user.
                    ttclient.OnCmdProcessing += new TeamTalk.CommandProcessing(ttclient_OnCmdProcessing);

                    //figure out how we're subscribing to video
                    Subscription subtype = Subscription.SUBSCRIBE_NONE;
                    User user = new User();
                    if (ttclient.GetUser(this.userid, ref user))
                    {
                        if (user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_VIDEOCAPTURE))
                            subtype = Subscription.SUBSCRIBE_VIDEOCAPTURE;
                        else if (user.uLocalSubscriptions.HasFlag(Subscription.SUBSCRIBE_INTERCEPT_VIDEOCAPTURE))
                            subtype = Subscription.SUBSCRIBE_INTERCEPT_VIDEOCAPTURE;
                    }

                    //store the command ID we get back from the client instance
                    unsubscribe_cmdid = ttclient.DoUnsubscribe(this.userid, subtype);
                    e.Cancel = true;
                }
                else
                    e.Cancel = true;
            }
            else
                ttclient.OnUserVideoCapture -= ttclient_OnUserVideoFrame;
        }

        void ttclient_OnCmdProcessing(int nCmdID, bool bActive)
        {
            if (nCmdID == unsubscribe_cmdid && !bActive)
            {
                //now that the 'DoUnsubscribe' command has completed we can close the dialog.
                this.Close();
            }
        }

        VideoFrame vidframe;

        void ttclient_OnUserVideoFrame(int nUserID, int nStreamID)
        {
            if (userid != nUserID)
                return;

            if (bmp != null)
                bmp.Dispose();
            bmp = null;

            //Release shared memory
            ttclient.ReleaseUserVideoCaptureFrame(vidframe);

            vidframe = ttclient.AcquireUserVideoCaptureFrame(nUserID, out bmp);

            Invalidate();
        }

        protected override void OnPaintBackground(PaintEventArgs e)
        {
            if (bmp != null)
            {
                e.Graphics.DrawImage(bmp, 0, 0, this.ClientRectangle.Width,
                    this.ClientRectangle.Height);
            }
            else
            {
                base.OnPaintBackground(e);
            }
        }
        
        //hm, this doesn't work... :(
        //protected override void OnPaint(PaintEventArgs e)
        //{
        //    if (bmp != null)
        //    {
        //        ttclient.PaintVideoFrame(userid, e.Graphics.GetHdc(), 0, 0,  
        //            this.ClientRectangle.Width, this.ClientRectangle.Height);
        //        e.Graphics.ReleaseHdc();
        //    }
        //    else
        //        base.OnPaint(e);
        //}
    }
}
