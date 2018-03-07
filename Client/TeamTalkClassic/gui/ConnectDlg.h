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

#include "afxwin.h"
#if !defined(AFX_CONNECTDLG_H__C11D2435_7B0C_40E4_806D_A91D63FE50D7__INCLUDED_)
#define AFX_CONNECTDLG_H__C11D2435_7B0C_40E4_806D_A91D63FE50D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConnectDlg.h : header file
//

#include "settings/ClientXML.h"
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CConnectDlg dialog

class CConnectDlg : public CDialog
{
// Construction
public:
    CConnectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CConnectDlg)
    enum { IDD = IDD_DIALOG_CONNECT };
    //}}AFX_DATA

    void DisplayHosts();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CConnectDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CConnectDlg)
        // NOTE: the ClassWizard will add member functions here
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
    CStatic m_wndGroupBox;
    CEdit m_wndHostUdpPort;
    CStatic m_wndDefHostText;
    CStatic m_wndDefSoundText;
    CStatic m_wndHostPortText;
    CEdit m_wndHostPort;
    CStatic m_wndHostUdpPortText;
    CButton m_btnOK;
    CButton m_btnCancel;
    //lastest
    std::vector<teamtalk::HostEntry> m_vecHosts, m_delHosts;
    UINT m_nTcpPort;
    UINT m_nUdpPort;
    virtual BOOL OnInitDialog();
    CComboBox m_wndHostAddress;
    afx_msg void OnCbnSelchangeComboHostaddress();
    CString m_szHostAddress;
    afx_msg void OnStnClickedStaticDefsound();
    afx_msg void OnBnClickedOk();
    afx_msg void OnCbnEditupdateComboHostaddress();
    CStatic m_wndGrpJoinChan;
    CEdit m_wndUsername;
    CString m_szUsername;
    CEdit m_wndPassword;
    CString m_szPassword;
    CString m_szChPasswd;
    CString m_szChannel;
    CEdit m_wndChannel;
    CEdit m_wndChPasswd;
    afx_msg void OnBnClickedButtonDelentry();
    BOOL m_bEncrypted;
    CButton m_wndEncrypted;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONNECTDLG_H__C11D2435_7B0C_40E4_806D_A91D63FE50D7__INCLUDED_)
