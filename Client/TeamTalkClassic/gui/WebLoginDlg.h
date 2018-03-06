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
#include "explorer1.h"


// CWebLoginDlg dialog

class CWebLoginDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWebLoginDlg)

public:
	CWebLoginDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWebLoginDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_WEBLOGIN };

    CString m_szPassword, m_szToken;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

    CResizer m_resizer;
    BOOL m_bCancelled;
public:
    virtual BOOL OnInitDialog();
    CExplorer1 m_wndWebBrowser;
    DECLARE_EVENTSINK_MAP()
    void NavigateComplete2Explorer1(LPDISPATCH pDisp, VARIANT* URL);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    virtual void OnCancel();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};
