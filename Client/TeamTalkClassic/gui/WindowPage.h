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
#include "Helper.h"
#include "afxcmn.h"

// CWindowPage dialog

class CWindowPage : public CPropertyPage
{
    DECLARE_DYNAMIC(CWindowPage)

public:
    CWindowPage();
    virtual ~CWindowPage();
    BOOL    m_bTray;
    BOOL m_bStartMinimized;
    LOGFONT m_lf;

    Font m_Font;

// Dialog Data
    enum { IDD = IDD_PROPPAGE_WINDOWPAGE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonFont();
    CStatic m_wndFontName;
    void ShowFont();
    virtual BOOL OnInitDialog();
    BOOL m_bPopMsg;
    BOOL m_bAlwaysOnTop;
  BOOL m_bShowUserCount;
    BOOL m_bDBClickJoin;
    BOOL m_bQuitClearChannels;
    BOOL m_bTimeStamp;
    CString m_szLanguage;
    CComboBox m_wndLanguages;
    BOOL m_bCheckUpdates;
    BOOL m_bVuMeter;
    int m_nTextLen;
    CSpinButtonCtrl m_wndSpinTextLen;
    BOOL m_bShowUsername;
};
