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

    MyFont m_Font;

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
    CComboBox m_wndSorting;
    int m_nSorting;
    BOOL m_bEmoji;
 };
