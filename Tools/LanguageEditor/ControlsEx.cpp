/********************************************************************************************
* MOD-NAME      : ControlsEx.cpp
* LONG-NAME     : 
*
* AUTHOR        : Martin Ziacek (Martin.Ziacek@pobox.sk)
* DEPARTMENT    : 
* TELEPHONE     : 
* CREATION-DATE : 22/02/2000 12:12:39
* SP-NO         : 
* FUNCTION      : 
* 
*********************************************************************************************/

#include "stdafx.h"
#include <afxpriv.h>
#include "ControlsEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgStatusBar

BEGIN_MESSAGE_MAP(CDlgStatusBar, CStatusBar)
    //{{AFX_MSG_MAP(CDlgStatusBar)
    ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
                 
/////////////////////////////////////////////////////////////////////////////
// CDlgStatusBar Construction/Destruction

CDlgStatusBar::CDlgStatusBar()
{
}

CDlgStatusBar::~CDlgStatusBar()
{
} 

LRESULT CDlgStatusBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM) 
{
    if (IsWindowVisible()) {
        CFrameWnd *pParent = (CFrameWnd *)GetParent();
        if (pParent)
            OnUpdateCmdUI(pParent, (BOOL)wParam);
    }
    return 0L;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgToolBar

BEGIN_MESSAGE_MAP(CDlgToolBar, CToolBar)
    //{{AFX_MSG_MAP(CDlgToolBar)
    ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
        // NOTE - the ClassWizard will add and remove mapping macros here.
        //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
                 
/////////////////////////////////////////////////////////////////////////////
// CDlgToolBar Construction/Destruction

CDlgToolBar::CDlgToolBar()
{
}

CDlgToolBar::~CDlgToolBar()
{
} 

LRESULT CDlgToolBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM) 
{
    if (!m_ButtonStates.GetCount()) {
        if (IsWindowVisible()) {
            CFrameWnd *pParent = (CFrameWnd *)GetParent();
            if (pParent)
                OnUpdateCmdUI(pParent, (BOOL)wParam);
        }
    }

    if (IsWindowVisible()) {
        POSITION pos = m_ButtonStates.GetStartPosition();
        int id;
        BOOL val;

        while (pos != NULL) {
            m_ButtonStates.GetNextAssoc(pos,id,val);
            GetToolBarCtrl().EnableButton(id,val);
        }
    }
        
    return 0L;
}

void CDlgToolBar::EnableButton(int id, BOOL bEnable)
{
    GetToolBarCtrl().EnableButton(id,bEnable);

    if (bEnable) m_ButtonStates.RemoveKey(id);
    else m_ButtonStates.SetAt(id,bEnable);
}
