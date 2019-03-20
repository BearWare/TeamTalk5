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
#include "afxcmn.h"
#include "Resizer.h"

enum
{
    COLUMN_USERID,
    COLUMN_NICKNAME,
    COLUMN_STATUSMSG,
    COLUMN_USERNAME,
    COLUMN_CHANNEL,
    COLUMN_IPADDRESS,
    COLUMN_VERSION_,
    _COLUMN_LAST_COUNT
};

#define WM_ONLINEUSERSDLG_CLOSED (WM_APP+4)

// COnlineUsersDlg dialog

class COnlineUsersDlg : public CDialog
{
	DECLARE_DYNAMIC(COnlineUsersDlg)

public:
	COnlineUsersDlg(class CTeamTalkDlg* pParent = NULL);   // standard constructor
	virtual ~COnlineUsersDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_ONLINEUSERS };

    void AddUser(const User& user);
    void RemoveUser(int nUserID);

    void ResetUsers();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void PostNcDestroy();

	DECLARE_MESSAGE_MAP()
    HACCEL m_hAccel; // accelerator table

    CResizer m_resizer;

    void MenuCommand(UINT uCmd);
    class CTeamTalkDlg* m_pParent;

    std::vector<User> m_users;
public:
    CListCtrl m_wndUsers;
    virtual BOOL OnInitDialog();
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnPopupKick();
    afx_msg void OnPopupKickandban();
    afx_msg void OnPopupOp();
    afx_msg void OnPopupCopyuserinformation();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnPopupMessages();
    virtual void OnCancel();
};
