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

#include "ChatRichEditCtrl.h"
#include "MyEdit.h"
#include "afxcmn.h"

#include "MyTab.h"

// CChatTab dialog

class CChatTab : public CMyTab
{
    DECLARE_DYNAMIC(CChatTab)

public:
    CChatTab(CWnd* pParent = NULL);   // standard constructor
    virtual ~CChatTab();

// Dialog Data
    enum { IDD = IDD_TAB_CHAT };

    HACCEL m_hAccel; // accelerator table

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    CResizer m_resizer;

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    CChatRichEditCtrl m_wndRichEdit;
    CMyEdit m_wndChanMessage;
protected:
    virtual void OnOK();
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};
