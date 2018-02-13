/********************************************************************************************
* MOD-NAME      : ControlsEx.h
* LONG-NAME     : 
*
* AUTHOR        : Martin Ziacek (Martin.Ziacek@pobox.sk)
* DEPARTMENT    : 
* TELEPHONE     : 
* CREATION-DATE : 22/02/2000 12:11:43
* SP-NO         : 
* FUNCTION      : 
* 
*********************************************************************************************/
#ifndef __CONTROLSEX_H__
#define __CONTROLSEX_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDlgToolBar
#include <afxtempl.h>

class CDlgToolBar : public CToolBar 
{   
// Construction
public:
    CDlgToolBar();
   
// Implementation
public:   
    virtual ~CDlgToolBar();

    void EnableButton(int id, BOOL bEnable);

protected:                
    CMap <int, int &, BOOL, BOOL &> m_ButtonStates;
    // Generated message map functions
    //{{AFX_MSG(CDlgToolBar)
    afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
}; 

/////////////////////////////////////////////////////////////////////////////
// CDlgStatusBar 

class CDlgStatusBar : public CStatusBar 
{   
// Construction
public:
    CDlgStatusBar();
   
// Implementation
public:   
    virtual ~CDlgStatusBar();

protected:                
    // Generated message map functions
    //{{AFX_MSG(CDlgStatusBar)
    afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif //__CONTROLSEX_H__
