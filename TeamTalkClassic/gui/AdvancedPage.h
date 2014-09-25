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

#ifndef UNDEFINED
#define UNDEFINED -1
#endif

// CAdvancedPage dialog

class CAdvancedPage : public CPropertyPage
{
    DECLARE_DYNAMIC(CAdvancedPage)

public:
    CAdvancedPage();
    virtual ~CAdvancedPage();

// Dialog Data
    enum { IDD = IDD_PROPPAGE_ADVANCEDPAGE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    BOOL m_bMixerAutoSelect;
    CButton m_wndMixerAutoSelect;
    int m_nMixerIndex;
    afx_msg void OnBnClickedCheckMixerautoselect();
    CComboBox m_wndMixerInputs;
    virtual BOOL OnInitDialog();
    void UpdateControls();
    BOOL m_bBoostBug;
};
