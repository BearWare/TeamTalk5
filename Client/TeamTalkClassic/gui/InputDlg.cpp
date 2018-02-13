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
CInputDlg::CInputDlg(CString szTitle, CString szMessage, 
                     LPCTSTR lpszInitialInput/* = NULL */,
                     CWnd* pParent /*=NULL*/)
    : CDialog(CInputDlg::IDD, pParent),
    m_szMessage(szMessage),
    m_szTitle(szTitle),
    m_szInput(lpszInitialInput)
{
}

CInputDlg::~CInputDlg()
{
}

void CInputDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    if(m_inputList.GetSize())
        DDX_Text(pDX, IDC_COMBO_INPUT, m_szInput);
    else
        DDX_Text(pDX, IDC_EDIT_INPUT, m_szInput);
    DDX_Control(pDX, IDC_EDIT_INPUT, m_wndInput);
    DDX_Text(pDX, IDC_STATIC_MESSAGE, m_szMessage);
    DDX_Control(pDX, IDC_COMBO_INPUT, m_wndInputCombo);
}


BEGIN_MESSAGE_MAP(CInputDlg, CDialog)
END_MESSAGE_MAP()


// CInputDlg message handlers

void CInputDlg::OnOK()
{
    CDialog::OnOK();
}

BOOL CInputDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    if(m_inputList.GetSize())
    {
        CRect newRect, oldRect;
        m_wndInput.GetWindowRect(newRect);
        m_wndInputCombo.GetWindowRect(oldRect);
        ScreenToClient(newRect);
        ScreenToClient(oldRect);
        m_wndInput.MoveWindow(oldRect);
        m_wndInputCombo.MoveWindow(newRect);
        for(POSITION pos=m_inputList.GetHeadPosition();pos != NULL;)
            m_wndInputCombo.AddString(m_inputList.GetNext(pos));
        if(m_szInput.GetLength())
            m_wndInputCombo.SelectString(-1, m_szInput);
        else
            m_wndInputCombo.SetCurSel(0);
        m_wndInputCombo.ShowWindow(SW_SHOW);
        m_wndInput.ShowWindow(SW_HIDE);
    }
    else
    {
        m_wndInputCombo.ShowWindow(SW_HIDE);
    }

    SetWindowText(m_szTitle);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
