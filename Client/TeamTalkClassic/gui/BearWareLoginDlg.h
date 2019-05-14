#pragma once

#include "HttpRequest.h"
#include <memory>
#include "afxwin.h"

// CBearWareLoginDlg dialog

class CBearWareLoginDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CBearWareLoginDlg)

public:
	CBearWareLoginDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBearWareLoginDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_BEARWARELOGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

    std::unique_ptr<CHttpRequest> m_webLogin;
public:
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    virtual void OnOK();
    CEdit m_wndUsername;
    CString m_szUsername;
    CEdit m_wndPassword;
    CString m_szToken;
    virtual BOOL OnInitDialog();
    CButton m_wndOK;
};
