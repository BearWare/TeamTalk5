// D:\Testing-C\TeamTalk2\GUI\UserStatusDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../Resource.h"
#include "UserStatusDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CUserStatusDlg dialog

IMPLEMENT_DYNAMIC(CUserStatusDlg, CDialog)
CUserStatusDlg::CUserStatusDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUserStatusDlg::IDD, pParent)
	, m_szAwayMessage(_T(""))
	, m_szStatusMode(_T(""))
	, m_nStatusMode(USERSTATUS_AVAILABLE)
{
}

CUserStatusDlg::~CUserStatusDlg()
{
}

void CUserStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_MESSAGE, m_szAwayMessage);
	DDX_Text(pDX, IDC_STATIC_STATUSMODE, m_szStatusMode);
}


BEGIN_MESSAGE_MAP(CUserStatusDlg, CDialog)
END_MESSAGE_MAP()


// CUserStatusDlg message handlers

BOOL CUserStatusDlg::OnInitDialog()
{
	switch(m_nStatusMode)
	{
	case USERSTATUS_AVAILABLE : m_szStatusMode = "Online";break;
	case USERSTATUS_AWAY : m_szStatusMode = "Away";break;
	}
	CDialog::OnInitDialog();

	TRANSLATE(*this, IDD);

	CString s;
	s.Format(_T("Status for %s"), m_szNick);
	SetWindowText(s);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
