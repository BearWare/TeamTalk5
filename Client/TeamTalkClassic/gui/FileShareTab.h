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

#include "Resizer.h"
#include "afxcmn.h"

// CFileShareTab dialog

class CFileShareTab : public CDialog
{
	DECLARE_DYNAMIC(CFileShareTab)

public:
	CFileShareTab(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFileShareTab();
	void SetNotifier(CWnd* pWnd);

// Dialog Data
	enum { IDD = IDD_TAB_FILESHARE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CResizer m_resizer;
	CWnd* m_pNotifyWnd;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnCancel();
public:
	CListCtrl m_wndFileList;
};
