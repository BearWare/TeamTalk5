#pragma once

// CUserStatusDlg dialog

class CUserStatusDlg : public CDialog
{
    DECLARE_DYNAMIC(CUserStatusDlg)

public:
    CUserStatusDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CUserStatusDlg();

// Dialog Data
    enum { IDD = IDD_DIALOG_USERSTATUS };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CString m_szAwayMessage;
    CString m_szStatusMode;
    CString m_szNick;
    int m_nStatusMode;
    virtual BOOL OnInitDialog();
};
