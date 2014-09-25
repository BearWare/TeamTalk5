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


