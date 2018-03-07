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

// MyTab.cpp : implementation file
//

#include "stdafx.h"
#include "MyTab.h"
#include ".\mytab.h"


// CMyTab

IMPLEMENT_DYNAMIC(CMyTab, CDialog)
CMyTab::CMyTab(UINT nIDTemplate, CWnd* pParent/* = NULL*/)
: CDialog(nIDTemplate, pParent)
{
}

CMyTab::~CMyTab()
{
}


BEGIN_MESSAGE_MAP(CMyTab, CDialog)
//    ON_WM_KILLFOCUS()
END_MESSAGE_MAP()



// CMyTab message handlers


BOOL CMyTab::PreTranslateMessage(MSG* pMsg)
{
    if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
    {
        BOOL bPrevious = (GetAsyncKeyState(VK_SHIFT) & 0x8000)?TRUE:FALSE;
        CWnd* pDummy = NULL;
        CWnd* pFirst = this->GetNextDlgTabItem(pDummy);
        CWnd* pLast = this->GetNextDlgTabItem(pFirst, TRUE);
        CWnd* pActive = this->GetFocus();
        CWnd* pNext = this->GetNextDlgTabItem(pActive, bPrevious);
        if( ( ( (pNext == pFirst && !bPrevious) || (bPrevious && pNext == pLast)) || pActive == pNext ) && GetParent())
        {
            if(bPrevious)
                GetParent()->SetFocus();
            else
            {
                // pNext = AfxGetMainWnd()->GetNextDlgTabItem(GetParent()); //This only works if tab-ctrl is on main dialog
                CWnd* pParent = GetParent()->GetParent(); // tab ctrl (parent), dialog (parent-parent)
                pNext = pParent->GetNextDlgTabItem(GetParent());
                if(pNext)
                    pNext->SetFocus();
            }
            return TRUE;
        }
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void CMyTab::OnOK()
{
}

void CMyTab::OnCancel()
{
}
