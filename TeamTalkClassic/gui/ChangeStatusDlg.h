#pragma once

// CChangeStatus dialog

class CChangeStatusDlg : public CDialog
{
    DECLARE_DYNAMIC(CChangeStatusDlg)

public:
    CChangeStatusDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CChangeStatusDlg();

// Dialog Data
    enum { IDD = IDD_DIALOG_CHANGESTATUS };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CString m_szAwayMessage;
    int m_nStatusMode;
    afx_msg void OnBnClickedRadioOnline();
    afx_msg void OnBnClickedRadioAway();
    CEdit m_wndMessage;
    CButton m_wndOnline;
    CButton m_wndAway;
    CButton m_wndQuestion;
    virtual BOOL OnInitDialog();
protected:
    virtual void OnOK();
public:
    afx_msg void OnBnClickedRadioQuestion();
};
