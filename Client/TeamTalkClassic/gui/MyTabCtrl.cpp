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

// MyTabCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "MyTabCtrl.h"
#include <afxtempl.h>
// CMyTabCtrl

IMPLEMENT_DYNAMIC(CMyTabCtrl, CXPTabCtrl)

CMyTabCtrl::CMyTabCtrl()
{
}

CMyTabCtrl::~CMyTabCtrl()
{
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

    int sel = this->GetCurSel();
    for(size_t i=0;i<m_vecTabs.size();i++)
    {
        int flag = sel == i? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
        m_vecTabs[i]->SetWindowPos(&wndTop, nX+5, nY+5, nXc-13, nYc-13, flag);
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
        if(GetCurSel() >= 0)
        {
            m_vecTabs[GetCurSel()]->SetFocus();
            return TRUE;
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
