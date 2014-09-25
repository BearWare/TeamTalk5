#pragma once
#include "afxcmn.h"

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

// COnlineUsersDlg dialog

class COnlineUsersDlg : public CDialog
{
	DECLARE_DYNAMIC(COnlineUsersDlg)

public:
	COnlineUsersDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COnlineUsersDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_ONLINEUSERS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
    HACCEL m_hAccel; // accelerator table

    void MenuCommand(UINT uCmd);
public:
    CListCtrl m_wndUsers;
    virtual BOOL OnInitDialog();
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnPopupKick();
    afx_msg void OnPopupKickandban();
    afx_msg void OnPopupOp();
    afx_msg void OnPopupCopyuserinformation();
};
