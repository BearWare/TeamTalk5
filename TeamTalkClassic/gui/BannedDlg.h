#pragma once

#include "Resizer.h"
#include "afxwin.h"
#include "ExListbox.h"

// CBannedDlg dialog

class CBannedDlg : public CDialog
{
    DECLARE_DYNAMIC(CBannedDlg)

public:
    CBannedDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CBannedDlg();

    std::vector<BannedUser> m_vecBanned;
    std::vector<BannedUser> m_vecUnBanned;

    // Dialog Data
    enum { IDD = IDD_DIALOG_BANNEDDLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    void Update();

    CResizer m_resizer;
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedButtonUnban();
    afx_msg void OnBnClickedButtonBan();
    virtual BOOL OnInitDialog();
    CExListBox m_wndBanned;
    CExListBox m_wndUnbanned;
    afx_msg void OnSize(UINT nType, int cx, int cy);
};
