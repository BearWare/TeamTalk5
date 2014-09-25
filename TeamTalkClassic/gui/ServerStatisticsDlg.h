#pragma once


// CServerStatisticsDlg dialog

class CServerStatisticsDlg : public CDialog
{
	DECLARE_DYNAMIC(CServerStatisticsDlg)

public:
	CServerStatisticsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CServerStatisticsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SERVERSTATISTICS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CString m_szTotalRxTx;
    CString m_szVoiceRxTx;
    CString m_szVideoRxTx;
    virtual BOOL OnInitDialog();
    CString m_szMediaFileRXTX;
    CString m_szDesktopRXTX;
    CString m_szFilesRxTx;
    CString m_szUsersServed;
    CString m_szUsersPeak;
};
