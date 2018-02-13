// gui\DirectConDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../Resource.h"
#include "DirectConDlg.h"
#include ".\directcondlg.h"


// CDirectConDlg dialog

IMPLEMENT_DYNAMIC(CDirectConDlg, CDialog)
CDirectConDlg::CDirectConDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CDirectConDlg::IDD, pParent)
    , m_bDisable(FALSE)
    , m_szMsg(_T(""))
{
}

CDirectConDlg::~CDirectConDlg()
{
}

void CDirectConDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_DISABLE, m_bDisable);
    DDX_Text(pDX, IDC_STATIC_MSG, m_szMsg);
}


BEGIN_MESSAGE_MAP(CDirectConDlg, CDialog)
END_MESSAGE_MAP()


// CDirectConDlg message handlers

BOOL CDirectConDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
