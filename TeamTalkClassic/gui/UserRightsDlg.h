#pragma once
#include "afxwin.h"


// CUserRightsDlg dialog

class CUserRightsDlg : public CDialog
{
    DECLARE_DYNAMIC(CUserRightsDlg)

public:
    CUserRightsDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CUserRightsDlg();

// Dialog Data
    enum { IDD = IDD_DIALOG_USERRIGHTS };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:

    virtual BOOL OnInitDialog();
};
