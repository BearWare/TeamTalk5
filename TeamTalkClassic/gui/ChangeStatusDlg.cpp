
#include "stdafx.h"
#include "Resource.h"
#include "ChangeStatusDlg.h"

//need status mode information

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CChangeStatus dialog

IMPLEMENT_DYNAMIC(CChangeStatusDlg, CDialog)
CChangeStatusDlg::CChangeStatusDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CChangeStatusDlg::IDD, pParent)
    , m_szAwayMessage(_T("")),
    m_nStatusMode(STATUSMODE_AVAILABLE)
{
}

CChangeStatusDlg::~CChangeStatusDlg()
{
}

void CChangeStatusDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_AWAYMESSAGE, m_szAwayMessage);
    DDX_Control(pDX, IDC_EDIT_AWAYMESSAGE, m_wndMessage);
    DDX_Control(pDX, IDC_RADIO_AWAY, m_wndAway);
    DDX_Control(pDX, IDC_RADIO_ONLINE, m_wndOnline);
    DDX_Control(pDX, IDC_RADIO_QUESTION, m_wndQuestion);
}


BEGIN_MESSAGE_MAP(CChangeStatusDlg, CDialog)
    ON_BN_CLICKED(IDC_RADIO_ONLINE, OnBnClickedRadioOnline)
    ON_BN_CLICKED(IDC_RADIO_AWAY, OnBnClickedRadioAway)
    ON_BN_CLICKED(IDC_RADIO_QUESTION, &CChangeStatusDlg::OnBnClickedRadioQuestion)
END_MESSAGE_MAP()


// CChangeStatus message handlers

void CChangeStatusDlg::OnBnClickedRadioOnline()
{
    m_nStatusMode &= ~STATUSMODE_MASK;
    m_nStatusMode |= STATUSMODE_AVAILABLE;
}

void CChangeStatusDlg::OnBnClickedRadioAway()
{
    m_nStatusMode &= ~STATUSMODE_MASK;
    m_nStatusMode |= STATUSMODE_AWAY;
}

void CChangeStatusDlg::OnBnClickedRadioQuestion()
{
    m_nStatusMode &= ~STATUSMODE_MASK;
    m_nStatusMode |= STATUSMODE_QUESTION;
}

BOOL CChangeStatusDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    switch(m_nStatusMode & STATUSMODE_MASK)
    {
    case STATUSMODE_AVAILABLE : 
        m_wndOnline.SetCheck(TRUE);
        OnBnClickedRadioOnline();
        break;
    case STATUSMODE_AWAY : 
        m_wndAway.SetCheck(TRUE);
        OnBnClickedRadioAway();
        break;
    case STATUSMODE_QUESTION :
        m_wndQuestion.SetCheck(TRUE);
        OnBnClickedRadioQuestion();
        break;
    }

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CChangeStatusDlg::OnOK()
{
    CDialog::OnOK();
}
