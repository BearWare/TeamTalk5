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

#include "NGWizardPage.h"
#include "afxwin.h"

// CWizWelcomePage dialog

class CWizWelcomePage : public CNGWizardPage
{
    DECLARE_DYNAMIC(CWizWelcomePage)
public:
    CWizWelcomePage();   // standard constructor
    virtual ~CWizWelcomePage();

    virtual BOOL OnSetActive();
    // Dialog Data
    enum { IDD = IDD_WIZARD_WELCOMEPAGE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    CStatic m_wndIntro;
    CStatic m_wndTitle;
    CFont font1;
public:
    virtual BOOL OnInitDialog();
    BOOL m_bLanguage;
    CComboBox m_wndLanguage;
    CString m_szLanguage;
    afx_msg void OnCbnSelchangeComboLanguage();
    afx_msg void OnBnClickedCheckUilanguage();
    CButton m_wndCheckLanguage;
};
