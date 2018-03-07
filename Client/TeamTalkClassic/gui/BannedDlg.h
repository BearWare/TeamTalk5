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
#include "afxwin.h"
#include "ExListbox.h"

// CBannedDlg dialog

class CBannedDlg : public CDialog
{
    DECLARE_DYNAMIC(CBannedDlg)

public:
    CBannedDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CBannedDlg();

    std::vector<BannedUser> m_vecBanned;
    std::vector<BannedUser> m_vecUnBanned;

    // Dialog Data
    enum { IDD = IDD_DIALOG_BANNEDDLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    void Update();

    CResizer m_resizer;
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedButtonUnban();
    afx_msg void OnBnClickedButtonBan();
    virtual BOOL OnInitDialog();
    CExListBox m_wndBanned;
    CExListBox m_wndUnbanned;
    afx_msg void OnSize(UINT nType, int cx, int cy);
};
