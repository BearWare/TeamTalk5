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
