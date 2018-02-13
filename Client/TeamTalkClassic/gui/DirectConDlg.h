#pragma once


// CDirectConDlg dialog

class CDirectConDlg : public CDialog
{
    DECLARE_DYNAMIC(CDirectConDlg)

public:
    CDirectConDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CDirectConDlg();

// Dialog Data
    enum { IDD = IDD_DIALOG_DIRECTCONDLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    BOOL m_bDisable;
    CString m_szMsg;
    virtual BOOL OnInitDialog();
};
