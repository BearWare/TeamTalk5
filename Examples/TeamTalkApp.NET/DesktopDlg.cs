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
using BearWare;

namespace TeamTalkApp.NET
{
    public partial class DesktopDlg : Form
    {
        TeamTalk ttclient;
        int userid;
        Bitmap bmp;
        int unsubscribe_cmdid = 0;

        public DesktopDlg(TeamTalk tt, int userid)
        {
            ttclient = tt;
            this.userid = userid;

            InitializeComponent();
            this.CenterToScreen();
            
            ttclient.OnUserDesktopWindow += new TeamTalk.NewDesktopWindow(ttclient_OnUserDesktopWindow);
        }

        private void DesktopDlg_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (unsubscribe_cmdid == 0)
            {
                if (MessageBox.Show("Do you wish to stop receiving desktops from this user?", "Close Desktop",
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
                        if ((user.uLocalSubscriptions & Subscription.SUBSCRIBE_DESKTOP) == Subscription.SUBSCRIBE_DESKTOP)
                            subtype = Subscription.SUBSCRIBE_DESKTOP;
                        else if ((user.uLocalSubscriptions & Subscription.SUBSCRIBE_INTERCEPT_DESKTOP) == Subscription.SUBSCRIBE_INTERCEPT_DESKTOP)
                            subtype = Subscription.SUBSCRIBE_INTERCEPT_DESKTOP;
                    }

                    //store the command ID we get back from the client instance
                    unsubscribe_cmdid = ttclient.DoUnsubscribe(this.userid, subtype);
                    e.Cancel = true;
                }
                else
                    e.Cancel = true;
            }
            else
                ttclient.OnUserDesktopWindow -= ttclient_OnUserDesktopWindow;
        }

        void ttclient_OnCmdProcessing(int nCmdID, bool bActive)
        {
            if (nCmdID == unsubscribe_cmdid && !bActive)
            {
                //now that the 'DoUnsubscribe' command has completed we can close the dialog.
                this.Close();
            }
        }

        void ttclient_OnUserDesktopWindow(int nUserID, int nSessionID)
        {
            if (nUserID != userid)
                return;
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

        DesktopWindow deskwnd = new DesktopWindow();

        private void timer1_Tick(object sender, EventArgs e)
        {
            DesktopWindow tmp_wnd = ttclient.AcquireUserDesktopWindow(userid);
            if (tmp_wnd.nSessionID <= 0)
            {
                ttclient.ReleaseUserDesktopWindow(deskwnd);
                this.Close();
                return;
            }

            ttclient.ReleaseUserDesktopWindow(deskwnd);

            deskwnd = tmp_wnd;

            if (ClientSize.Height != deskwnd.nHeight || ClientSize.Width != deskwnd.nWidth)
            {
                ClientSize = new Size(deskwnd.nWidth, deskwnd.nHeight);
                //int width_diff = deskwnd.nWidth - ClientSize.Width;
                //int height_diff = deskwnd.nHeight - ClientSize.Height;
                //this.Size.Width += width_diff;
                //this.Size.Height += height_diff;
            }

            if (bmp != null)
                bmp.Dispose();
            bmp = null;

            switch(deskwnd.bmpFormat)
            {
                case BitmapFormat.BMP_RGB8_PALETTE:
                    bmp = new Bitmap(deskwnd.nWidth, deskwnd.nHeight, deskwnd.nBytesPerLine,
                                     PixelFormat.Format8bppIndexed, deskwnd.frameBuffer);
                    ColorPalette pal = bmp.Palette;
                    for (int i = 0; i < 256; i++)
                        pal.Entries[i] = TeamTalk.Palette_GetColorTable(BitmapFormat.BMP_RGB8_PALETTE, i);
                    bmp.Palette = pal;
                    break;
                case BitmapFormat.BMP_RGB16_555:
                    bmp = new Bitmap(deskwnd.nWidth, deskwnd.nHeight, 
                                     deskwnd.nBytesPerLine, PixelFormat.Format16bppRgb555,
                                     deskwnd.frameBuffer);
                    break;
                case BitmapFormat.BMP_RGB24 :
                    bmp = new Bitmap(deskwnd.nWidth, deskwnd.nHeight, deskwnd.nBytesPerLine,
                                     PixelFormat.Format24bppRgb, deskwnd.frameBuffer);
                    break;
                case BitmapFormat.BMP_RGB32:
                    bmp = new Bitmap(deskwnd.nWidth, deskwnd.nHeight, deskwnd.nBytesPerLine,
                                     PixelFormat.Format32bppRgb, deskwnd.frameBuffer);
                    break;
            }

            Invalidate();
        }

        DesktopInput ToTTKeyCode(MouseEventArgs e, DesktopKeyState state)
        {
            DesktopInput input;
            switch (e.Button)
            {
                case MouseButtons.Left:
                    input.uKeyCode = DesktopInputConstants.DESKTOPINPUT_KEYCODE_LMOUSEBTN;
                    break;
                case MouseButtons.Right:
                    input.uKeyCode = DesktopInputConstants.DESKTOPINPUT_KEYCODE_RMOUSEBTN;
                    break;
                case MouseButtons.Middle:
                    input.uKeyCode = DesktopInputConstants.DESKTOPINPUT_KEYCODE_MMOUSEBTN;
                    break;
                default :
                    input.uKeyCode = DesktopInputConstants.DESKTOPINPUT_KEYCODE_IGNORE;
                    break;
            }
            input.uKeyState = state;
            input.uMousePosX = (ushort)e.X;
            input.uMousePosY = (ushort)e.Y;
            return input;
        }

        DesktopInput ToTTKeyCode(uint scancode, DesktopKeyState state)
        {
            DesktopInput input;
            input.uKeyCode = scancode;
            input.uKeyState = state;
            input.uMousePosX = input.uMousePosY = DesktopInputConstants.DESKTOPINPUT_MOUSEPOS_IGNORE;
            return input;
        }

        DesktopInput[] ToTTKey(DesktopInput input)
        {
            DesktopInput[] inputs = new DesktopInput[1];
            inputs[0] = input;
            DesktopInput[] translated;
            WindowsHelper.DesktopInputKeyTranslate(TTKeyTranslate.TTKEY_WINKEYCODE_TO_TTKEYCODE,
                                       inputs, out translated);
            return translated;
        }

        private void DesktopDlg_MouseDown(object sender, MouseEventArgs e)
        {
            DesktopInput input = ToTTKeyCode(e, DesktopKeyState.DESKTOPKEYSTATE_DOWN);
            ttclient.SendDesktopInput(userid, ToTTKey(input));
        }

        private void DesktopDlg_MouseMove(object sender, MouseEventArgs e)
        {
            DesktopInput input = ToTTKeyCode(e, DesktopKeyState.DESKTOPKEYSTATE_NONE);
            ttclient.SendDesktopInput(userid, ToTTKey(input));
        }

        private void DesktopDlg_MouseUp(object sender, MouseEventArgs e)
        {
            DesktopInput input = ToTTKeyCode(e, DesktopKeyState.DESKTOPKEYSTATE_UP);
            ttclient.SendDesktopInput(userid, ToTTKey(input));
        }

        protected override void WndProc(ref Message message)
        {
            const int WM_KEYDOWN = 0x0100;
            const int WM_KEYUP = 0x0101;

            switch (message.Msg)
            {
                case WM_KEYDOWN:
                    {
                        uint scancode = (((uint)message.LParam) >> 16) & 0x1FF;
                        DesktopInput input = ToTTKeyCode(scancode, DesktopKeyState.DESKTOPKEYSTATE_DOWN);
                        ttclient.SendDesktopInput(userid, ToTTKey(input));
                        break;
                    }
                case WM_KEYUP:
                    {
                        uint scancode = (((uint)message.LParam) >> 16) & 0x1FF;
                        DesktopInput input = ToTTKeyCode(scancode, DesktopKeyState.DESKTOPKEYSTATE_UP);
                        ttclient.SendDesktopInput(userid, ToTTKey(input));
                        break;
                    }
            }
            base.WndProc(ref message);
        }
    }
}
