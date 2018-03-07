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

// wizard\WizMasterSheet.cpp : implementation file
//

#include "stdafx.h"
#include "TeamTalkApp.h"
#include "WizMasterSheet.h"


// CWizMasterSheet

IMPLEMENT_DYNAMIC(CWizMasterSheet, CNGWizard)
    CWizMasterSheet::CWizMasterSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
    :CNGWizard(nIDCaption, pParentWnd, iSelectPage)
{
}

CWizMasterSheet::CWizMasterSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
    :CNGWizard(pszCaption, pParentWnd, iSelectPage)
{
}

CWizMasterSheet::~CWizMasterSheet()
{
}

BEGIN_MESSAGE_MAP(CWizMasterSheet, CNGWizard)
END_MESSAGE_MAP()


// CWizMasterSheet message handlers

BOOL CWizMasterSheet::OnInitDialog()
{
    CNGWizard::OnInitDialog();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
