#pragma once
#include "afxwin.h"


// CInputDlg dialog

class CInputDlg : public CDialog
{
    DECLARE_DYNAMIC(CInputDlg)

public:
    CInputDlg(CString title, CString message, LPCTSTR lpszInitialInput = NULL, CWnd* pParent = NULL);   // standard constructor
    virtual ~CInputDlg();

// Dialog Data
    enum { IDD = IDD_DIALOG_INPUT };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
    CString m_szTitle;
    CString m_szMessage;
public:
    CString m_szInput;
    CString GetInputString(){return m_szInput;}
protected:
    virtual void OnOK();
public:
    CEdit m_wndInput;
    virtual BOOL OnInitDialog();
};
