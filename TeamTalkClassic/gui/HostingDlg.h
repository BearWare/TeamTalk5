#pragma once
#include "afxcmn.h"

#include "ChannelDlg.h"
#include "SessionTreeCtrl.h"

// CHostingDlg dialog

class CHostingDlg : public CDialog
{
    DECLARE_DYNAMIC(CHostingDlg)

public:
    CHostingDlg(const channels_t& channels, CWnd* pParent = NULL);   // standard constructor
    virtual ~CHostingDlg();
    CString GetChannelPath(HTREEITEM hItem);
    CImageList m_ImageList;
    channels_t m_channels;
    CString m_szServerName, m_szPassword;

    //users rights
    BOOL m_bCreateChannels;
    BOOL m_bOperators;

    //remote admin
    BOOL m_bRemoteAdmin;
    CButton m_wndRAdminButton;
    CString m_szAdminUsername;
    CEdit m_wndAdminPassword;
    CEdit m_wndAdminUsername;
    CString m_szAdminPassword;

// Dialog Data
    enum { IDD = IDD_DIALOG_HOSTINGADVANCED };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CSessionTreeCtrl m_wndTree;
    afx_msg void OnBnClickedButtonNew();
    afx_msg void OnBnClickedButtonUpdate();
    afx_msg void OnBnClickedButtonDelete();
    afx_msg void OnBnClickedCheckRemoteadmin();
    virtual BOOL OnInitDialog();
};
