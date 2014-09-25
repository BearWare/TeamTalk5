// gui\UserRightsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../Resource.h"
#include "UserRightsDlg.h"
#include ".\userrightsdlg.h"


// CUserRightsDlg dialog

IMPLEMENT_DYNAMIC(CUserRightsDlg, CDialog)
CUserRightsDlg::CUserRightsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUserRightsDlg::IDD, pParent)

{
}

CUserRightsDlg::~CUserRightsDlg()
{
}

void CUserRightsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

}


BEGIN_MESSAGE_MAP(CUserRightsDlg, CDialog)
END_MESSAGE_MAP()


// CUserRightsDlg message handlers

BOOL CUserRightsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	TRANSLATE(*this, IDD);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
