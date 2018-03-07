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


// CServerPropertiesDlg dialog

class CServerPropertiesDlg : public CDialog
{
	DECLARE_DYNAMIC(CServerPropertiesDlg)

public:
	CServerPropertiesDlg(BOOL bReadOnly, CWnd* pParent = NULL);   // standard constructor
	virtual ~CServerPropertiesDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SERVERPROPERTIES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
    BOOL m_bReadOnly;
public:
    CString m_szSrvName;
    int m_nMaxUsers;
    CString m_szMOTD;
    int m_nTcpPort;
    int m_nUdpPort;
    int m_nUserTimeout;
    BOOL m_bAutoSave;
    CButton m_btnOk;
    CButton m_btnCancel;
    virtual BOOL OnInitDialog();
    CEdit m_wndSrvName;
    CEdit m_wndMaxUsers;
    CEdit m_wndMOTD;
    CEdit m_wndTcpPort;
    CEdit m_wndUdpPort;
    CEdit m_wndUserTimeout;
    CButton m_wndAutoSave;
    CString m_szVersion;
    int m_nAudioTx;
    int m_nVideoTx;
    int m_nTotalTx;
    CEdit m_wndAudioTX;
    CEdit m_wndVideoTX;
    CEdit m_wndTotalTX;
    CEdit m_wndDesktopTx;
    int m_nDesktopTxMax;
    BOOL m_bShowMOTD;
    CButton m_wndShowMOTD;
    CString m_szMOTDRaw;
    afx_msg void OnBnClickedCheckShowmotd();
    CEdit m_wndMediaFileTx;
    int m_nMediaFileTx;
    int m_nMaxIPLogins;
    int m_nLoginsBan;
    CEdit m_wndLoginBan;
    CEdit m_wndIPLogins;
};
