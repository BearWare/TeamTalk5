#pragma once

#include "MyTab.h"
#include "afxwin.h"
#include "TeamTalkBase.h"

// CAbuseTab dialog

enum
{
    LIMITCMD_DISABLED = 0,
    LIMITCMD_10_PER_10SEC = 1,
    LIMITCMD_10_PER_MINUTE = 2,
    LIMITCMD_60_PER_MINUTE = 3,
    LIMITCMD_CUSTOM = 4
};

class CAbuseTab : public CMyTab
{
	DECLARE_DYNAMIC(CAbuseTab)

public:
	CAbuseTab(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAbuseTab();

// Dialog Data
	enum { IDD = IDD_TAB_ABUSE };
    void ShowAbuseInfo();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_wndCmdLimit;
    AbusePrevention m_abuse;
    virtual BOOL OnInitDialog();
    afx_msg void OnCbnSelchangeComboCmdlimit();
};
