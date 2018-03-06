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

#include <set>
#include "afxwin.h"

#include "settings/ClientXML.h"

// CKeyCompDlg dialog

class CKeyCompDlg : public CDialog
{
    DECLARE_DYNAMIC(CKeyCompDlg)

public:
    CKeyCompDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CKeyCompDlg();

    // Dialog Data
    enum { IDD = IDD_DIALOG_KEYCOMP };
    void ShowKeys();
    teamtalk::HotKey m_Hotkey;
    CEdit    m_KeyEdit;
    std::set<UINT> m_setDownKeys;
    std::set<UINT> m_setUpKeys;
    static CString GetHotkeyString(teamtalk::HotKey hotkey);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    afx_msg LRESULT OnKeyStroke(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
protected:
    virtual void OnOK();
};
