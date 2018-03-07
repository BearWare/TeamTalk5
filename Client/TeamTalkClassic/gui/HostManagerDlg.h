/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#if !defined(AFX_HOSTMANAGERDLG_H__D07010E5_5BF8_4705_AB4B_E3858BB4FD68__INCLUDED_)
#define AFX_HOSTMANAGERDLG_H__D07010E5_5BF8_4705_AB4B_E3858BB4FD68__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HostManagerDlg.h : header file
//


#include "settings/ClientXML.h"
#include "HttpRequest.h"

#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// CHostManagerDlg dialog

class CHostManagerDlg : public CDialog
{
// Construction
public:
    CHostManagerDlg(teamtalk::ClientXML* xmlSettings, CWnd* pParent = NULL);   // standard constructor
    ~CHostManagerDlg();

    std::vector<teamtalk::HostEntry> m_vecHosts, m_delHosts;

// Dialog Data
    //{{AFX_DATA(CHostManagerDlg)
    enum { IDD = IDD_DIALOG_HOSTMANAGER };
    CButton    m_wndDelete;
    CListBox    m_wndHosts;
    CEdit    m_wndHostUdpPort;
    CEdit    m_wndHostPort;
    CEdit    m_wndEntryName;
    CString    m_szEntryName;
    UINT    m_nTcpPort;
    UINT    m_nUdpPort;
    CComboBox    m_wndUsername;
    CEdit    m_wndPassword;
    CString m_szUsername;
    CString m_szPassword;
    CString m_szHostAddress;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CHostManagerDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CHostManagerDlg)
    afx_msg void OnButtonNew();
    afx_msg void OnButtonAdd();
    afx_msg void OnButtonDelete();
    afx_msg void OnSelchangeListHosts();
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    void ShowPublicServers();
    BOOL GetHostEntry(teamtalk::HostEntry& entry);

    void DisplayHosts();

private:
    teamtalk::ClientXML* m_pSettings;
    CHttpRequest* m_pHttpRequest;
    std::vector<teamtalk::HostEntry> m_pubservers;
    void ShowFieldError();

public:
    afx_msg void OnBnClickedOk();
    CString m_szChannel;
    CString m_szChPassword;
    CEdit m_wndChannel;
    CEdit m_wndChPasswd;
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    CButton m_btnPubServers;
    BOOL m_bPubServers;
    afx_msg void OnBnClickedCheckPublicservers();
    afx_msg void OnLbnDblclkListHosts();
    afx_msg void OnBnClickedButtonGentt();
    CButton m_wndEncrypted;
    BOOL m_bEncrypted;
    afx_msg void OnBnClickedButtonDelentry();
    CComboBox m_wndHostAddress;
    afx_msg void OnCbnSelchangeComboHostaddress();
    afx_msg void OnBnClickedButtonImportttile();
    CButton m_wndImportBtn;
    afx_msg void OnCbnEditchangeComboUsername();
    afx_msg void OnCbnKillfocusComboUsername();
    afx_msg void OnEnSetfocusEditEntryname();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HOSTMANAGERDLG_H__D07010E5_5BF8_4705_AB4B_E3858BB4FD68__INCLUDED_)
