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
#include "Resizer.h"
#include "afxwin.h"


// CChannelOpTab dialog

class CChannelOpTab : public CMyTab
{
	DECLARE_DYNAMIC(CChannelOpTab)

public:
	CChannelOpTab(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChannelOpTab();

// Dialog Data
	enum { IDD = IDD_TAB_CHANNELOP };

    CListBox m_wndAvailChannels;
    CListBox m_wndSelChannels;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

    CResizer m_resizer;

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBnClickedButtonAddchannel();
    afx_msg void OnBnClickedButtonDelchannel();

    CButton m_btnAddChan;
    CButton m_btnRmChan;
};
