#pragma once

#include <set>
#include "afxwin.h"

#include "settings/ClientXML.h"

// CKeyCompDlg dialog

class CKeyCompDlg : public CDialog
{
    DECLARE_DYNAMIC(CKeyCompDlg)

public:
    CKeyCompDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CKeyCompDlg();

    // Dialog Data
    enum { IDD = IDD_DIALOG_KEYCOMP };
    void ShowKeys();
    teamtalk::HotKey m_Hotkey;
    CEdit    m_KeyEdit;
    std::set<UINT> m_setDownKeys;
    std::set<UINT> m_setUpKeys;
    static CString GetHotkeyString(teamtalk::HotKey hotkey);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    afx_msg LRESULT OnKeyStroke(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
protected:
    virtual void OnOK();
};
