#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CStreamMediaDlg dialog

class CStreamMediaDlg : public CDialog
{
	DECLARE_DYNAMIC(CStreamMediaDlg)

public:
	CStreamMediaDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStreamMediaDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_STREAMMEDIA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

    void UpdateMediaFile();
public:
    afx_msg void OnBnClickedButtonBrowse();
    CComboBox m_wndVidCodec;
    int m_nVidCodecBitrate;
    CSpinButtonCtrl m_wndVidBitrateSpinCtrl;
    virtual BOOL OnInitDialog();
    CStatic m_wndAudioFormat;
    CStatic m_wndVideoFormat;
    CEdit m_wndVideoBitrate;
    CEdit m_wndFileName;
    CString m_szFilename;
};
