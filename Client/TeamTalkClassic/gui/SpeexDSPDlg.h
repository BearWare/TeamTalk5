#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CSpeexDSPDlg dialog

class CSpeexDSPDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSpeexDSPDlg)

public:
	CSpeexDSPDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSpeexDSPDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SPEEXDSP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonDefault();
    afx_msg void OnBnClickedButtonDefault2();
    BOOL m_bAGC;
    int m_nGainLevel;
    int m_nMaxGainLevel;
    int m_nGainInc;
    int m_nGainDec;
    int m_nDenoiseLevel;
    BOOL m_bDenoise;
    virtual BOOL OnInitDialog();
    CEdit m_wndGainLevel;
    CEdit m_wndGainMax;
    CEdit m_wndGainInc;
    CEdit m_wndGainDec;
    CEdit m_wndDenoise;
    CSpinButtonCtrl m_wndGainLevelSpin;
    CSpinButtonCtrl m_wndGainMaxSpin;
    CSpinButtonCtrl m_wndGainIncSpin;
    CSpinButtonCtrl m_wndGainDecSpin;
    CSpinButtonCtrl m_wndDenoiseSpin;
    CButton m_wndAGC;
    CButton m_wndDenoiseEnable;
};
