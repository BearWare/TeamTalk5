#if !defined(AFX_SYSCOLSTATIC_H__664DE301_4F7B_11D1_9E3D_00A0245800CF__INCLUDED_)
#define AFX_SYSCOLSTATIC_H__664DE301_4F7B_11D1_9E3D_00A0245800CF__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SysColStatic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSysColStatic window

class CSysColStatic : public CStatic
{
// Construction
public:
    CSysColStatic();
    void ReloadBitmap(int nImageID);
// Attributes
public:

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSysColStatic)
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CSysColStatic();

    // Generated message map functions
protected:
    int m_nImageID;
    //{{AFX_MSG(CSysColStatic)
    afx_msg void OnSysColorChange();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
public:
//    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSCOLSTATIC_H__664DE301_4F7B_11D1_9E3D_00A0245800CF__INCLUDED_)