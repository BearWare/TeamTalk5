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

// ChatRichEditCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ChatRichEditCtrl.h"
#include ".\chatricheditctrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern TTInstance* ttInst;

// CChatRichEditCtrl

IMPLEMENT_DYNAMIC(CChatRichEditCtrl, CRichEditCtrl)
CChatRichEditCtrl::CChatRichEditCtrl()
: m_bMinimized(FALSE)
, m_bShowTimeStamp(FALSE)
{
}

CChatRichEditCtrl::~CChatRichEditCtrl()
{
}


BEGIN_MESSAGE_MAP(CChatRichEditCtrl, CRichEditCtrl)
    ON_NOTIFY_REFLECT(EN_LINK, OnEnLink)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_POPUP_UNDO, OnPopupUndo)
    ON_COMMAND(ID_POPUP_SELECTALL, OnPopupSelectall)
    ON_COMMAND(ID_POPUP_COPY, OnPopupCopy)
    ON_COMMAND(ID_POPUP_CLEARALL, OnPopupClearall)
    ON_COMMAND(ID_POPUP_DELETE, OnPopupDelete)
    ON_COMMAND(ID_POPUP_PASTE, OnPopupPaste)
    ON_COMMAND(ID_POPUP_CUT, OnPopupCut)
    ON_WM_KEYDOWN()
    ON_WM_SIZE()
END_MESSAGE_MAP()



// CChatRichEditCtrl message handlers

void CChatRichEditCtrl::OnEnLink(NMHDR *pNMHDR, LRESULT *pResult)
{
    ENLINK      *pENLink;
    CString      URL ;
    *pResult = 0;

    switch (pNMHDR->code)
    {
    case EN_LINK:
        pENLink = reinterpret_cast<ENLINK*> (pNMHDR);

        switch (pENLink->msg)
        {
        case WM_LBUTTONDOWN:
            GetTextRange(pENLink->chrg.cpMin, pENLink->chrg.cpMax, URL);
            {
                CWaitCursor WaitCursor;
                HWND hWnd = m_hWnd;
                hWnd = AfxGetMainWnd()->m_hWnd;
                HINSTANCE hInstance = ShellExecute(hWnd, _T("open"), URL, NULL, NULL, SW_SHOWNORMAL);
                *pResult = 1;
            }
            break;

        case WM_LBUTTONUP:
            *pResult = 1;
            break ;
        }

        break;
    }
}

void CChatRichEditCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
    CMenu menu;
    VERIFY(menu.LoadMenu(IDR_MENU_MSGPOPUP));
    TRANSLATE(menu);
    long n1 = 0, n2 = 0;
    GetSel(n1, n2);
    UINT nEn = n1 != n2? MF_ENABLED : MF_DISABLED | MF_GRAYED;

    menu.EnableMenuItem(ID_POPUP_UNDO, CanUndo()? MF_ENABLED : MF_DISABLED | MF_GRAYED);
    menu.EnableMenuItem(ID_POPUP_COPY, nEn);
    menu.EnableMenuItem(ID_POPUP_CUT, nEn);
    menu.EnableMenuItem(ID_POPUP_DELETE, nEn);
    menu.EnableMenuItem(ID_POPUP_PASTE, CanPaste()? MF_ENABLED : MF_DISABLED | MF_GRAYED);
    menu.EnableMenuItem(ID_POPUP_CLEARALL, MF_ENABLED);
    menu.EnableMenuItem(ID_POPUP_SELECTALL, MF_ENABLED);

    menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y, this);  
}

void CChatRichEditCtrl::OnPopupUndo()
{
    if(CanUndo())
        Undo();
}

void CChatRichEditCtrl::OnPopupSelectall()
{
    if(GetTextLength()>0)
        SetSel(0,GetTextLength()-1);
}

void CChatRichEditCtrl::OnPopupCopy()
{
    long n1 = 0, n2 = 0;
    GetSel(n1, n2);
    if(n1 != n2)
        Copy();
}

void CChatRichEditCtrl::OnPopupClearall()
{
    SetWindowText(_T(""));
}

void CChatRichEditCtrl::OnPopupDelete()
{
    long n1 = 0, n2 = 0;
    GetSel(n1, n2);
    if(n1 != n2)
        ReplaceSel(_T(""));
}

void CChatRichEditCtrl::OnPopupPaste()
{
    Paste();
}

void CChatRichEditCtrl::OnPopupCut()
{
    long n1 = 0, n2 = 0;
    GetSel(n1, n2);
    if(n1 != n2)
        Cut();
}

int CChatRichEditCtrl::GetVisibleLinesCount()
{
    //this is used for ensuring for filling out the 
    //window before scrolling down

    int num = 0;            //this is the number of visible lines 

    ASSERT(m_bMinimized == FALSE);
    CRect rect;
    GetRect(rect);
    int height = rect.Height();

    for(int i = GetFirstVisibleLine(); 
        i < GetLineCount() && 
        GetCharPos(LineIndex(i)).y < height; 
    i++) 
        num++; 

    return num;
}

void CChatRichEditCtrl::ScrollDown(int nOldLineCount)
{
    int nNewLines = GetLineCount() - nOldLineCount;
    int nLeft = 0;
    if(m_bMinimized)
        nLeft = 0;
    else
        nLeft = GetVisibleLinesCount() - GetLineCount();
    int nScroll = nLeft<=0? nNewLines : nLeft + nNewLines;
    LineScroll(nScroll,0);
}

CString CChatRichEditCtrl::GetChatTime()
{
    CTime tm = CTime::GetCurrentTime();
    CString szTime;
    szTime.Format(_T("%.2d:%.2d"), tm.GetHour(), tm.GetMinute());
    return szTime;
}


CString CChatRichEditCtrl::AddMessage(CString szNick, CString szMessage)
{
    CHARFORMAT cf;
    memset(&cf,0,sizeof (CHARFORMAT));
    cf.cbSize        = sizeof (CHARFORMAT);  
    cf.dwMask        = CFM_COLOR | CFM_UNDERLINE | CFM_BOLD;
    cf.dwEffects    = (unsigned long)~(CFE_AUTOCOLOR | CFE_UNDERLINE | CFE_BOLD);
    cf.crTextColor    = RGB(0, 0, 0); 

    int nOldLineCount = GetLineCount();
    CString name;
    if(m_bShowTimeStamp)
        name.Format(_T("\r\n%s <%s> "), GetChatTime(), szNick);
    else
        name.Format(_T("\r\n<%s> "), szNick);
    //insert name
    SetSel(GetTextLength(),GetTextLength());
    SetSelectionCharFormat(cf); 
    ReplaceSel(name);

    //insert msg
    SetSel(GetTextLength(), GetTextLength());
    ReplaceSel(szMessage);

    ScrollDown(nOldLineCount);

    return name + szMessage;
}

void CChatRichEditCtrl::AddBroadcastMessage(CString szMessage)
{
    CHARFORMAT cf;
    memset(&cf,0,sizeof (CHARFORMAT));
    cf.cbSize        = sizeof (CHARFORMAT);  
    cf.dwMask        = CFM_COLOR | CFM_UNDERLINE | CFM_BOLD;
    cf.dwEffects    = (unsigned long)~(CFE_AUTOCOLOR | CFE_UNDERLINE );
    cf.crTextColor    = RGB(0, 0, 0); 

    int nOldLineCount = GetLineCount();
    //insert ServerInfo
    CString szMsg;
    if(GetLineCount()<=1)
        szMsg.Format(LoadText(IDS_CHATRICHBROADCAST, _T("Broadcast: %s\r\n")), szMessage);
    else
        szMsg.Format(LoadText(IDS_CHATRICHBROADCASTT, _T("\r\nBroadcast: %s")), szMessage);
    SetSel(GetTextLength(),GetTextLength());
    cf.crTextColor    = RGB(255, 117, 5);
    SetSelectionCharFormat(cf);
    ReplaceSel(szMsg);

    //insert enter
    SetSel(GetTextLength(), GetTextLength());
    ReplaceSel(_T("\r\n"));
    HideSelection(TRUE, FALSE);

    ScrollDown(nOldLineCount);
}

void CChatRichEditCtrl::AddLogMesage(CString szMsg)
{
    CHARFORMAT cf;
    memset(&cf,0,sizeof (CHARFORMAT));
    cf.cbSize        = sizeof (CHARFORMAT);  
    cf.dwMask        = CFM_COLOR | CFM_UNDERLINE | CFM_BOLD;
    cf.dwEffects    = (unsigned long)~(CFE_AUTOCOLOR | CFE_UNDERLINE | CFE_BOLD);
    cf.crTextColor    = RGB(127, 127, 127); 

    int nOldLineCount = GetLineCount();
    CString szLine;
    if(m_bShowTimeStamp)
        szLine.Format(_T("\r\n%s * %s"), GetChatTime(), szMsg);
    else
        szLine.Format(_T("\r\n* %s"), szMsg);

    //insert msg
    SetSel(GetTextLength(), GetTextLength());
    SetSelectionCharFormat(cf); 
    ReplaceSel(szLine);

    ScrollDown(nOldLineCount);
}

void CChatRichEditCtrl::SetServerInfo(CString szServerName, CString szMOTD)
{
    CHARFORMAT cf;
    memset(&cf,0,sizeof (CHARFORMAT));
    cf.cbSize        = sizeof (CHARFORMAT);  
    cf.dwMask        = CFM_COLOR | CFM_UNDERLINE | CFM_BOLD;
    cf.dwEffects    = (unsigned long)~(CFE_AUTOCOLOR | CFE_UNDERLINE );
    cf.crTextColor    = RGB(0, 0, 0); 

    int nOldLineCount = GetLineCount();
    //insert ServerInfo
    CString szMsg;
    if(GetLineCount()<=1)
        szMsg.Format(LoadText(IDS_CHATRICHSERVERNAMEMOTD, _T("Server name: %s\r\nMOTD: %s\r\n")), szServerName, szMOTD);
    else
        szMsg.Format(LoadText(IDS_CHATRICHSERVERNAMEMOTDD, _T("\r\nServer name: %s\r\nMOTD: %s")), szServerName, szMOTD);
    SetSel(GetTextLength(),GetTextLength());
    cf.crTextColor    = RGB(0, 0, 0);
    SetSelectionCharFormat(cf);
    ReplaceSel(szMsg);

    //insert enter
    SetSel(GetTextLength(), GetTextLength());
    ReplaceSel(_T("\r\n"));
    HideSelection(TRUE, FALSE);

    ScrollDown(nOldLineCount);
}

void CChatRichEditCtrl::SetChannelInfo(int nChannelID)
{
    CString szChannelName, szTopic, szDiskQuota;
    Channel chan = {};
    TCHAR szChanPath[TT_STRLEN] = {};
    TT_GetChannel(ttInst, nChannelID, &chan);
    TT_GetChannelPath(ttInst, nChannelID, szChanPath);

    szChannelName = szChanPath;
    szTopic = chan.szTopic;
    szDiskQuota.Format(_T("%I64d"), chan.nDiskQuota/1024);

    CHARFORMAT cf;
    memset(&cf,0,sizeof (CHARFORMAT));
    cf.cbSize        = sizeof (CHARFORMAT);  
    cf.dwMask        = CFM_COLOR | CFM_UNDERLINE | CFM_BOLD;
    cf.dwEffects    = (unsigned long)~(CFE_AUTOCOLOR | CFE_UNDERLINE | CFE_BOLD);
    cf.crTextColor    = RGB(0, 0, 0); 

    int nOldLineCount = GetLineCount();
    //insert join text
    SetSel(GetTextLength(),GetTextLength());
    cf.crTextColor    = RGB(0, 0, 0);
    cf.dwEffects    = (unsigned long)~(CFE_AUTOCOLOR | CFE_UNDERLINE);
    SetSelectionCharFormat(cf);

    ReplaceSel(LoadText(IDS_CHATRICHJOINEDNEWCHAN, _T("\r\nJoined new channel.")));

    //insert Channel name
    SetSel(GetTextLength(),GetTextLength());
    cf.crTextColor    = RGB(0, 123, 36);
    cf.dwEffects    = (unsigned long)~(CFE_AUTOCOLOR | CFE_UNDERLINE | CFE_BOLD);
    SetSelectionCharFormat(cf);
    ReplaceSel(LoadText(IDS_CHATRICHCHAN, _T("\r\nChannel: "))+ szChannelName);

    //insert topic
    SetSel(GetTextLength(),GetTextLength());
    cf.crTextColor    = RGB(0, 105, 221);
    SetSelectionCharFormat(cf);
    ReplaceSel(LoadText(IDS_CHATRICHTOPIC, _T("\r\nTopic: ")) + szTopic);

    //insert disk quota
    SetSel(GetTextLength(),GetTextLength());
    cf.crTextColor    = RGB(175, 51, 59);
    SetSelectionCharFormat(cf);
    ReplaceSel(LoadText(IDS_CHATRICHDISKQUOTA, _T("\r\nDisk quota: ")) + szDiskQuota + LoadText(IDS_CHATRICHKBYTES, _T(" KBytes")));

    //insert enter
    SetSel(GetTextLength(), GetTextLength());
    ReplaceSel(_T("\r\n"));
    HideSelection(TRUE, FALSE);

    ScrollDown(nOldLineCount);
}

void CChatRichEditCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    //for some odd reason ESC makes the CDialog close although 
    //OnCancel() is overwritten
    //if(nChar != VK_ESCAPE)
        CRichEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CChatRichEditCtrl::OnSize(UINT nType, int cx, int cy)
{
    CRichEditCtrl::OnSize(nType, cx, cy);

    CWnd* pParent = AfxGetApp()->m_pMainWnd;
    if(pParent)
    {
        WINDOWPLACEMENT wndpl;
        pParent->GetWindowPlacement(&wndpl);
        if(wndpl.showCmd != SW_SHOWMINIMIZED && m_bMinimized)
        {
            //an ugly hack to ensure the last line is at the bottom of the window
            SendMessage(EM_SCROLL,SB_PAGEDOWN,NULL);
        }
        m_bMinimized = wndpl.showCmd == SW_SHOWMINIMIZED;
    }
}
