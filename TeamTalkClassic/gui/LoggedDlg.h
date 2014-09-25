#if !defined(AFX_LOGGEDDLG_H__34ED1AAD_0573_489D_A889_0DABDD773EF2__INCLUDED_)
#define AFX_LOGGEDDLG_H__34ED1AAD_0573_489D_A889_0DABDD773EF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoggedDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLoggedDlg dialog

class CLoggedDlg : public CDialog
{
// Construction
public:
	virtual void WriteLine(CString str);
	CLoggedDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLoggedDlg)
	enum { IDD = IDD_DIALOG_LOGGED };
	CListBox	m_wndList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoggedDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLoggedDlg)
	afx_msg void OnButton1();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGGEDDLG_H__34ED1AAD_0573_489D_A889_0DABDD773EF2__INCLUDED_)
