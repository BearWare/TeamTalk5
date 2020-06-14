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

// UserDesktopDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../Resource.h"
#include "UserDesktopDlg.h"

extern TTInstance* ttInst;

// CUserDesktopDlg dialog

IMPLEMENT_DYNAMIC(CUserDesktopDlg, CDialog)

enum
{
    TIMERID_GET_DESKTOPWINDOW = 1,
    TIMERID_SEND_DESKTOPINPUT = 2
};

CUserDesktopDlg::CUserDesktopDlg(int nUserID, CWnd* pParent /*=NULL*/)
	: CDialog(CUserDesktopDlg::IDD, pParent)
    , m_nUserID(nUserID)
    , m_desktopwnd(NULL)
    , m_bRequestedAccess(FALSE)
{

}

CUserDesktopDlg::~CUserDesktopDlg()
{
    TT_ReleaseUserDesktopWindow(ttInst, m_desktopwnd);
}

void CUserDesktopDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void CUserDesktopDlg::ResizeToDesktopWindow()
{
    if(m_desktopwnd)
    {
        CRect rWnd, rClient;
        GetWindowRect(rWnd);
        GetClientRect(rClient);

        int nDiffW = m_desktopwnd->nWidth - rClient.Width();
        int nDiffH = m_desktopwnd->nHeight - rClient.Height();
        rWnd.right += nDiffW;
        rWnd.bottom += nDiffH;
        
        MoveWindow(rWnd);

        GetClientRect(rClient);
        //ASSERT(rClient.Width() == wnd.nWidth);
        //ASSERT(rClient.Height() == wnd.nHeight);
    }
}

BEGIN_MESSAGE_MAP(CUserDesktopDlg, CDialog)
    ON_WM_ERASEBKGND()
    ON_WM_TIMER()
    ON_WM_CLOSE()
    ON_WM_CONTEXTMENU()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MBUTTONDOWN()
    ON_WM_MBUTTONUP()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
END_MESSAGE_MAP()


// CUserDesktopDlg message handlers

void CUserDesktopDlg::OnOK()
{
}

void CUserDesktopDlg::OnCancel()
{
    //CDialog::OnCancel();
}

BOOL CUserDesktopDlg::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(rect);

    if(m_desktopwnd && m_desktopwnd->nSessionID)
    {
        SetStretchBltMode(pDC->m_hDC, COLORONCOLOR);
        TT_PaintDesktopWindow(ttInst, m_nUserID, pDC->m_hDC, rect.left, 
                              rect.top, rect.Width(), rect.Height());
        //const int W = 100, H = 100;
        //BOOL b;
        //b = TT_PaintDesktopWindowEx(ttInst, m_nUserID, pDC->m_hDC, rect.left, 
        //                            rect.top, rect.Width(), rect.Height(),
        //                            m_desktopwnd.nWidth-W, m_desktopwnd.nHeight-H, W, H);
        //ASSERT(b);
        return FALSE;
    }
    else
        return CDialog::OnEraseBkgnd(pDC);
}

void CUserDesktopDlg::OnTimer(UINT_PTR nIDEvent)
{
    CDialog::OnTimer(nIDEvent);

    switch(nIDEvent)
    {
    case  TIMERID_GET_DESKTOPWINDOW :
    {
        DesktopWindow* wnd = TT_AcquireUserDesktopWindow(ttInst, m_nUserID);
        if(wnd)
            TT_ReleaseUserDesktopWindow(ttInst, m_desktopwnd);
        m_desktopwnd = wnd;
        if(m_desktopwnd)
        {
            CRect rect;
            GetClientRect(rect);
            if(rect.Width() != m_desktopwnd->nWidth ||
               rect.Height() != m_desktopwnd->nHeight)
                ResizeToDesktopWindow();
        }
        else
            AfxGetMainWnd()->PostMessage(WM_USERDESKTOPDLG_ENDED, m_nUserID);
        Invalidate();
        break;
    }
    case TIMERID_SEND_DESKTOPINPUT :
    {
        User user;
        if(!TT_GetUser(ttInst, m_nUserID, &user) ||
           (user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT) == 0)
            break;

        while(m_SendInputQueue.size())
        {
            int n_send = min(int(m_SendInputQueue.size()), TT_DESKTOPINPUT_MAX);
            if(TT_SendDesktopInput(ttInst, m_nUserID, &m_SendInputQueue[0], n_send))
                m_SendInputQueue.erase(m_SendInputQueue.begin(), m_SendInputQueue.begin()+n_send);
            else break;
        }
    }
    break;
    }
}

BOOL CUserDesktopDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    if(m_nUserID == 0)
        SetWindowText(LoadText(IDS_USERDESKTOPLOCALDESKTOP, _T("Local Desktop")));
    else
    {
        User user = {};
        TT_GetUser(ttInst, m_nUserID, &user);
        SetWindowText(CString(LoadText(IDS_USERDESKTOPDESKTOP, _T("Desktop - "))) + GetDisplayName(user));
    }

    ResizeToDesktopWindow();

    SetTimer(TIMERID_GET_DESKTOPWINDOW, 100, NULL);
    SetTimer(TIMERID_SEND_DESKTOPINPUT, 50, NULL);

    return TRUE;
}

void CUserDesktopDlg::OnClose()
{
    AfxGetMainWnd()->SendMessage(WM_USERDESKTOPDLG_CLOSED, m_nUserID);
}


void CUserDesktopDlg::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    User user;
    if(!TT_GetUser(ttInst, m_nUserID, &user) ||
        (user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT))
        return;

    CMenu menu;
    menu.LoadMenu(IDR_MENU_DESKTOPINPUT);
    TRANSLATE(menu);
    CMenu* pop = menu.GetSubMenu(0);
    if(!pop)
        return;

    UINT uCmd = pop->TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN |
                                    TPM_RIGHTBUTTON, point.x, point.y,
                                    this, NULL );
    MenuCommand(uCmd);
}

void CUserDesktopDlg::MenuCommand(UINT uCmd)
{
    switch(uCmd)
    {
    case ID_POPUP_REQUESTDESKTOPACCESS :
    {
        TextMessage msg;
        ZERO_STRUCT(msg);
        msg.nMsgType = MSGTYPE_CUSTOM;
        msg.nToUserID = m_nUserID;
        if(m_bRequestedAccess)
        {
            CString szText = MakeCustomCommand(TT_INTCMD_DESKTOP_ACCESS, _T("0"));
            COPYTTSTR(msg.szMessage, szText);
        }
        else
        {
            CString szText = MakeCustomCommand(TT_INTCMD_DESKTOP_ACCESS, _T("1"));
            COPYTTSTR(msg.szMessage, szText);
        }

        TT_DoTextMessage(ttInst, &msg);
        m_bRequestedAccess = !m_bRequestedAccess;
        break;
    }
    }
}

void CUserDesktopDlg::QueueMouseEvent(const CPoint& point, UINT32 uKeyCode,
                                      DesktopKeyState keystate)
{
    DesktopInput input;
    ZERO_STRUCT(input);
    input.uKeyCode = uKeyCode;
    input.uMousePosX = UINT16(point.x);
    input.uMousePosY = UINT16(point.y);
    input.uKeyState = keystate;
    TranslateAndQueue(input);
}

void CUserDesktopDlg::QueueKeyboardEvent(UINT32 uKeyCode, DesktopKeyState keystate)
{
    DesktopInput input;
    ZERO_STRUCT(input);
    input.uKeyCode = uKeyCode;
    input.uMousePosX = TT_DESKTOPINPUT_MOUSEPOS_IGNORE;
    input.uMousePosY = TT_DESKTOPINPUT_MOUSEPOS_IGNORE;
    input.uKeyState = keystate;
    TranslateAndQueue(input);
}

void CUserDesktopDlg::TranslateAndQueue(const DesktopInput& input)
{
    DesktopInput translated;
    TTKeyTranslate key_trans = TTKEY_WINKEYCODE_TO_TTKEYCODE;

    if(TT_DesktopInput_KeyTranslate(key_trans, &input,
                                    &translated, 1))
        m_SendInputQueue.push_back(translated);
    else
        TRACE(_T("Failed to translate desktop input for transmission. KeyCode: 0x%X"),
              input.uKeyCode);
}

void CUserDesktopDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    CDialog::OnMouseMove(nFlags, point);
    QueueMouseEvent(point, TT_DESKTOPINPUT_KEYCODE_IGNORE, DESKTOPKEYSTATE_NONE);
}


void CUserDesktopDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    CDialog::OnLButtonDown(nFlags, point);
    QueueMouseEvent(point, TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN, DESKTOPKEYSTATE_DOWN);
}


void CUserDesktopDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    CDialog::OnLButtonUp(nFlags, point);
    QueueMouseEvent(point, TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN, DESKTOPKEYSTATE_UP);
}


void CUserDesktopDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
    CDialog::OnMButtonDown(nFlags, point);
    QueueMouseEvent(point, TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN, DESKTOPKEYSTATE_DOWN);
}


void CUserDesktopDlg::OnMButtonUp(UINT nFlags, CPoint point)
{
    CDialog::OnMButtonUp(nFlags, point);
    QueueMouseEvent(point, TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN, DESKTOPKEYSTATE_UP);
}


void CUserDesktopDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
    CDialog::OnRButtonDown(nFlags, point);
    QueueMouseEvent(point, TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN, DESKTOPKEYSTATE_DOWN);
}


void CUserDesktopDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
    CDialog::OnRButtonUp(nFlags, point);
    QueueMouseEvent(point, TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN, DESKTOPKEYSTATE_UP);
}

BOOL CUserDesktopDlg::PreTranslateMessage(MSG* pMsg)
{
    if(pMsg->message == WM_KEYDOWN)
        QueueKeyboardEvent((pMsg->lParam >> 16) & 0x1FF, DESKTOPKEYSTATE_DOWN);
    else if(pMsg->message == WM_KEYUP)
        QueueKeyboardEvent((pMsg->lParam >> 16) & 0x1FF, DESKTOPKEYSTATE_UP);

    return CDialog::PreTranslateMessage(pMsg);
}
