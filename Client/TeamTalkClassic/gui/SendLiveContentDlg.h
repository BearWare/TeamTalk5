#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CSendLiveContentDlg dialog

class CSendLiveContentDlg : public CDialog
{
	DECLARE_DYNAMIC(CSendLiveContentDlg)

public:
	CSendLiveContentDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSendLiveContentDlg();

    users_t m_users;
    int m_nUserID = 0;
    std::vector<RemoteFile> m_files;
    // Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SENDLIVECONTENT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_wndFiles;
    CEdit m_wndUrl;
    CTreeCtrl m_wndUsers;
    CButton m_wndUrlRadio;
    afx_msg void OnBnClickedRadioUrl();
    afx_msg void OnBnClickedRadioDownload();
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    CString m_szUrl;
    CString m_szFileName;
    BOOL m_bURL;
};
