// MyTabCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MyTabCtrl.h"
#include ".\mytabctrl.h"
#include <afxtempl.h>
// CMyTabCtrl

IMPLEMENT_DYNAMIC(CMyTabCtrl, CXPTabCtrl)

CMyTabCtrl::CMyTabCtrl()
{
}

CMyTabCtrl::~CMyTabCtrl()
{
}

BOOL CMyTabCtrl::Init()
{
    BOOL b = m_tabChat.Create(IDD_TAB_CHAT, this);
    CString szChat = _T("Chat");
    TRANSLATE_ITEM(IDS_CHAT, szChat);
    this->AddTab(&m_tabChat, szChat, 0);
    b &= m_tabFiles.Create(IDD_TAB_FILES, this);
    CString szFiles = _T("Files");
    TRANSLATE_ITEM(IDS_FILES, szFiles);
    this->AddTab(&m_tabFiles, szFiles, 0);
    Sizing();
    
    return b;
}

void CMyTabCtrl::Translate()
{
    TCITEM item = {0};
    item.mask = TCIF_TEXT;
    this->GetItem(0, &item);
    CString szChat = _T("Chat");
    TRANSLATE_ITEM(IDS_CHAT, szChat);
    item.pszText = szChat.GetBuffer();
    this->SetItem(0, &item);
    this->GetItem(1, &item);
    CString szFiles = _T("Files");
    TRANSLATE_ITEM(IDS_FILES, szFiles);
    item.pszText = szFiles.GetBuffer();
    this->SetItem(1, &item);
    Invalidate();
}


void CMyTabCtrl::Sizing()
{
    CRect tabRect, itemRect;
    int nX, nY, nXc, nYc;

    GetClientRect(&tabRect);
    GetItemRect(0, &itemRect);

    nX=itemRect.left;
    nY=itemRect.bottom+1;
    nXc=tabRect.right-itemRect.left-1;
    nYc=tabRect.bottom-nY-1;

    CArray<CDialog*> tabs;
    tabs.Add(&m_tabChat);
    tabs.Add(&m_tabFiles);
    int sel = this->GetCurSel();
    for(int i=0;i<tabs.GetCount();i++)
    {
        int flag = sel == i? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
        tabs.GetAt(i)->SetWindowPos(&wndTop, nX+5, nY+5, nXc-13, nYc-13, flag);
    }
}

BEGIN_MESSAGE_MAP(CMyTabCtrl, CXPTabCtrl)
    ON_WM_SIZE()
ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnTcnSelchange)
ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

// CMyTabCtrl message handlers

void CMyTabCtrl::OnSize(UINT nType, int cx, int cy)
{
    CXPTabCtrl::OnSize(nType, cx, cy);

    Sizing();
}

BOOL CMyTabCtrl::PreTranslateMessage(MSG* pMsg)
{
    BOOL bPrevious = (GetAsyncKeyState(VK_SHIFT) & 0x8000)?TRUE:FALSE;
    if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB && !bPrevious)
    {
        switch(GetCurSel())
        {
        case 0 : m_tabChat.SetFocus(); return TRUE;
        case 1 : m_tabFiles.SetFocus(); return TRUE;
        }
    }

    return CXPTabCtrl::PreTranslateMessage(pMsg);
}

void CMyTabCtrl::OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult)
{
    __super::OnTabSelChanged(pNMHDR, pResult);
}

void CMyTabCtrl::OnKillFocus(CWnd* pNewWnd)
{
    CXPTabCtrl::OnKillFocus(pNewWnd);
}
