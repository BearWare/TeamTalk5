#pragma once
#include "afxwin.h"


// CMediaStorageDlg dialog

class CMediaStorageDlg : public CDialog
{
	DECLARE_DYNAMIC(CMediaStorageDlg)

public:
	CMediaStorageDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMediaStorageDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_STORECONVERSATIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_wndAFF;
    CEdit m_wndAudioDir;
    CString m_szAudioDir;
    BOOL m_bSingleFile;
    BOOL m_bSeparateFiles;
    UINT m_uAFF;
    afx_msg void OnBnClickedButtonBrowse();
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedButtonChanlogbrowse();
    afx_msg void OnBnClickedButtonUser2userbrowse();
    CEdit m_wndChanLogDir;
    CString m_szChanLogDir;
    CEdit m_wndUserTxtDir;
    CString m_szUserTxtDir;
};
