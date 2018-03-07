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


// CClientPage dialog

class CClientPage : public CPropertyPage
{
    DECLARE_DYNAMIC(CClientPage)

public:
    CClientPage();
    virtual ~CClientPage();

// Dialog Data
    enum { IDD = IDD_PROPPAGE_CLIENTPAGE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    BOOL m_bAutoConnect;
    UINT m_nClientTcpPort;
    UINT m_nClientUdpPort;
    BOOL m_bReconnect;
    CEdit m_wndHostPort;
    CEdit m_wndSoundPort;
    virtual BOOL OnKillActive();
    BOOL m_bAutoJoinRoot;
    virtual BOOL OnInitDialog();
    BOOL m_bSubUserMsg;
    BOOL m_bSubChanMsg;
    BOOL m_bSubVideo;
    BOOL m_bSubBcastMsg;
    BOOL m_bSubVoice;
    BOOL m_bSubDesktop;
    BOOL m_bSubMediaFile;
};
