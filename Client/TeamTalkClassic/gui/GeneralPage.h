/*
 * Copyright (c) 2005-2017, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#pragma once

#include "afxwin.h"
#include "afxcmn.h"

#include "KeyCompDlg.h"

// CGeneralPage dialog

class CGeneralPage : public CPropertyPage
{
    DECLARE_DYNAMIC(CGeneralPage)

public:
    CGeneralPage();
    virtual ~CGeneralPage();

    teamtalk::HotKey m_Hotkey;
    BOOL    m_bPush;
    BOOL    m_bVoiceAct;
    CString    m_sNickname;
    BOOL m_bFemale;
  int m_nInactivity;
  BOOL m_bIdleVox;

    CEdit    m_KeyEdit;
    CButton    m_btnPush;
    CButton    m_btnVoice;

    CButton m_wndKeysBtn;
    CStatic m_wndKeyComp;
  CEdit m_wndInactivity;
  CButton m_wndIdleVox;

// Dialog Data
    enum { IDD = IDD_PROPPAGE_GENERALPAGE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedCheckPushtotalk();
    virtual BOOL OnInitDialog();
public:
    afx_msg void OnBnClickedSetupKeys();
  afx_msg void OnEnChangeEditInactivity();
};
