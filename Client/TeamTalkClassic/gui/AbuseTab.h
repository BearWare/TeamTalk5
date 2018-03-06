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
