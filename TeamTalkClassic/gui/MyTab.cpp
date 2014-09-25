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
                pNext = AfxGetMainWnd()->GetNextDlgTabItem(GetParent());
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
