/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
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
