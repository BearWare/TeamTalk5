// WizMasterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../TeamTalk3.h"
#include "WizMasterDlg.h"
#include ".\wizmasterdlg.h"


// CWizMasterDlg dialog

CWizMasterDlg::CWizMasterDlg(CWnd* pParent /*=NULL*/)
	: CNewWizDialog(CWizMasterDlg::IDD, pParent)
{
}

CWizMasterDlg::~CWizMasterDlg()
{
}

void CWizMasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CNewWizDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CWizMasterDlg, CNewWizDialog)
END_MESSAGE_MAP()


// CWizMasterDlg message handlers

BOOL CWizMasterDlg::OnInitDialog()
{
  SetPlaceholderID(IDC_STATIC_SHEET);
  
  CNewWizDialog::OnInitDialog();

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}
