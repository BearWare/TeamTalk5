#pragma once
#include "afxwin.h"


// CAudioStorageDlg dialog

class CAudioStorageDlg : public CDialog
{
	DECLARE_DYNAMIC(CAudioStorageDlg)

public:
	CAudioStorageDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAudioStorageDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_AUDIOSTORAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_wndAFF;
    CEdit m_wndStorageDir;
    CString m_szStorageDir;
    BOOL m_bSingleFile;
    BOOL m_bSeparateFiles;
    UINT m_uAFF;
    afx_msg void OnBnClickedButtonBrowse();
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
};
