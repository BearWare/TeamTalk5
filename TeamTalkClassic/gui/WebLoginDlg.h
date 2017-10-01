#pragma once
#include "Resizer.h"
#include "explorer1.h"


// CWebLoginDlg dialog

class CWebLoginDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWebLoginDlg)

public:
	CWebLoginDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWebLoginDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_WEBLOGIN };

    CString m_szPassword;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

    CResizer m_resizer;
public:
    virtual BOOL OnInitDialog();
    CExplorer1 m_wndWebBrowser;
    DECLARE_EVENTSINK_MAP()
    void NavigateComplete2Explorer1(LPDISPATCH pDisp, VARIANT* URL);
    afx_msg void OnSize(UINT nType, int cx, int cy);
};
