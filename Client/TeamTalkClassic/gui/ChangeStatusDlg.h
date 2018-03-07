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

// CChangeStatus dialog

class CChangeStatusDlg : public CDialog
{
    DECLARE_DYNAMIC(CChangeStatusDlg)

public:
    CChangeStatusDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CChangeStatusDlg();

// Dialog Data
    enum { IDD = IDD_DIALOG_CHANGESTATUS };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CString m_szAwayMessage;
    int m_nStatusMode;
    afx_msg void OnBnClickedRadioOnline();
    afx_msg void OnBnClickedRadioAway();
    CEdit m_wndMessage;
    CButton m_wndOnline;
    CButton m_wndAway;
    CButton m_wndQuestion;
    virtual BOOL OnInitDialog();
protected:
    virtual void OnOK();
public:
    afx_msg void OnBnClickedRadioQuestion();
};
