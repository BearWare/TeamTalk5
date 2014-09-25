#include "stdafx.h"
#include "../Resource.h"
#include "InputDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CInputDlg dialog

IMPLEMENT_DYNAMIC(CInputDlg, CDialog)
CInputDlg::CInputDlg(CString title, CString message, LPCTSTR lpszInitialInput/* = NULL */, CWnd* pParent /*=NULL*/)
    : CDialog(CInputDlg::IDD, pParent),
    m_szMessage(message),
    m_szTitle(title),
    m_szInput(lpszInitialInput)
{
}

CInputDlg::~CInputDlg()
{
}

void CInputDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_INPUT, m_szInput);
    DDX_Control(pDX, IDC_EDIT_INPUT, m_wndInput);
    DDX_Text(pDX, IDC_STATIC_MESSAGE, m_szMessage);
}


BEGIN_MESSAGE_MAP(CInputDlg, CDialog)
END_MESSAGE_MAP()


// CInputDlg message handlers

void CInputDlg::OnOK()
{
    /*
    CString s;
    m_wndInput.GetWindowText(s);
    if(s.GetLength()==0)
        AfxMessageBox("Input cannot be empty");
    else
    */
    CDialog::OnOK();
}

BOOL CInputDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    SetWindowText(m_szTitle);
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
