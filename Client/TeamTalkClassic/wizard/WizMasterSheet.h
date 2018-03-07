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

#include "NGWizard.h"

#include "WizWelcomePage.h"
#include "WizGeneralPage.h"
#include "WizSoundSysPage.h"

// CWizMasterSheet

class CWizMasterSheet : public CNGWizard
{
    DECLARE_DYNAMIC(CWizMasterSheet)

public:
    CWizMasterSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CWizMasterSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    virtual ~CWizMasterSheet();

    virtual BOOL OnInitDialog();
protected:
    DECLARE_MESSAGE_MAP()
};


