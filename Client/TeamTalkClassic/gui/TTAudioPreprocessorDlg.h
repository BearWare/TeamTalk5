#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CTTAudioPreprocessorDlg dialog

class CTTAudioPreprocessorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTTAudioPreprocessorDlg)

public:
	CTTAudioPreprocessorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTTAudioPreprocessorDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_TTAUDIOPREPROCESSOR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

    void InitDefault();
public:
    int m_nGainLevel;
    BOOL m_bMuteLeft;
    BOOL m_bMuteRight;
    virtual BOOL OnInitDialog();
    CSliderCtrl m_wndGainlevel;
    afx_msg void OnBnClickedButtonDefault();
    CButton m_wndMuteLeft;
    CButton m_wndMuteRight;
};
