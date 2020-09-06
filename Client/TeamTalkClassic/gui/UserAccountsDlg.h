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

#include <vector>
#include "MyTabCtrl.h"
#include "ChannelOpTab.h"
#include "CodecLimitTab.h"
#include "AbuseTab.h"

// CUserAccountsDlg dialog

enum UserAccountsDisplay
{
    UAD_READWRITE,
    UAD_READONLY,
};

class CUserAccountsDlg : public CDialog
{
	DECLARE_DYNAMIC(CUserAccountsDlg)

public:
	CUserAccountsDlg(CWnd* pParent = NULL, UserAccountsDisplay uad = UAD_READWRITE);   // standard constructor
	virtual ~CUserAccountsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_USERACCOUNTS };

    std::vector<UserAccount> m_accounts;

    void UpdateControls();
    void ShowUserAccount(const UserAccount& useraccount);
    void ListAccounts();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

    UserAccountsDisplay m_uad;
public:
    CButton m_btnNew;
    CButton m_btnAdd;
    CButton m_btnDel;

    CListBox m_wndAccounts;
    CEdit m_wndUsername;
    CEdit m_wndPassword;
    CButton m_wndAdminUser;
    CButton m_wndDefaultUser;
    CButton m_wndDisabledUser;
    afx_msg void OnBnClickedButtonNew();
    afx_msg void OnBnClickedButtonAdd();
    afx_msg void OnBnClickedButtonDel();
    afx_msg void OnLbnSelchangeListAccounts();
    virtual BOOL OnInitDialog();
    afx_msg void OnEnChangeEditUsername();
    afx_msg void OnEnChangeEditPassword();
    CComboBox m_wndInitChannel;
    CEdit m_wndNote;

    CButton m_wndDoubleLogin;
    CButton m_wndTempChannels;
    CButton m_wndViewAllUsers;
    CButton m_wndUserBcast;
    CButton m_wndTransmitVoice;
    CButton m_wndTransmitVideo;
    CButton m_wndTransmitDesktops;
    CButton m_wndTransmitDesktopInput;
    afx_msg void OnBnClickedRadioDefaultuser();
    afx_msg void OnBnClickedRadioAdmin();
    CButton m_wndPermChannels;
    CButton m_wndKickUsers;
    CButton m_wndBanUsers;
    CButton m_wndMoveUsers;
    CButton m_wndChannelOp;
    CButton m_wndUploadFiles;
    CButton m_wndDownloadFiles;
    CButton m_wndSrvProp;
    CButton m_wndTransmitAudFiles;
    CButton m_wndTransmitVidFiles;
    CMyTabCtrl m_wndTabCtrl;
    CChannelOpTab m_wndChanOpTab;
    CCodecLimitTab m_wndCodecTab;
    CAbuseTab m_wndAbuseTab;
    afx_msg void OnSize(UINT nType, int cx, int cy);
    BOOL m_bResizeReady;
    CButton m_wndChangeNickname;
    CButton m_wndRecordVoice;
    afx_msg void OnBnClickedRadioDisableduser();
};
