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

#include "stdafx.h"
#include "Resource.h"
#include "Advancedpage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CAdvancedPage dialog

IMPLEMENT_DYNAMIC(CAdvancedPage, CPropertyPage)
CAdvancedPage::CAdvancedPage()
    : CPropertyPage(CAdvancedPage::IDD)
{
}

CAdvancedPage::~CAdvancedPage()
{
}

void CAdvancedPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAdvancedPage, CPropertyPage)
END_MESSAGE_MAP()


// CAdvancedPage message handlers

BOOL CAdvancedPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    TRANSLATE(*this, IDD);
    return TRUE;
}
