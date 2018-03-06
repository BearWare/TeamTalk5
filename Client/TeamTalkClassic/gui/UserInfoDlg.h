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

#pragma once
#include "afxwin.h"


// CUserInfoDlg dialog

class CUserInfoDlg : public CDialog
{
    DECLARE_DYNAMIC(CUserInfoDlg)

public:
    CUserInfoDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CUserInfoDlg();

// Dialog Data
    enum { IDD = IDD_DIALOG_USERINFO };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    void UpdateStats();
public:
    UINT m_nUserID;
    CString m_szNick;
    CString m_szPacketloss;
    UINT m_nPortNumber;
    virtual BOOL OnInitDialog();
    CString m_szVersion;
    CString m_szUsername;
    CString m_szUserType;
    CString m_szIPAddr;
    afx_msg void OnBnClickedOk();
    CString m_szMFVideoLoss;
    CString m_szMFAudioLoss;
    CStatic m_wndVoiceLoss;
    CStatic m_wndMFAudioLoss;
    CStatic m_wndMFVideoLoss;
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    CString m_szClientName;
    CButton m_wndProfileBtn;
    afx_msg void OnBnClickedButtonProfile();
};
