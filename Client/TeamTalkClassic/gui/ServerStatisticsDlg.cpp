#include "stdafx.h"
#include "../Resource.h"
#include "ServerStatisticsDlg.h"

IMPLEMENT_DYNAMIC(CServerStatisticsDlg, CDialog)

CServerStatisticsDlg::CServerStatisticsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerStatisticsDlg::IDD, pParent)
{
}

CServerStatisticsDlg::~CServerStatisticsDlg()
{
}

void CServerStatisticsDlg::DoDataExchange(CDataExchange* pDX)
{
    DDX_Text(pDX, IDC_EDIT_TOTAL, m_szTotalRxTx);
    DDX_Text(pDX, IDC_EDIT_VOICE, m_szVoiceRxTx);
    DDX_Text(pDX, IDC_EDIT_VIDEO, m_szVideoRxTx);
    DDX_Text(pDX, IDC_EDIT_MEDIAFILERXTX, m_szMediaFileRXTX);
    DDX_Text(pDX, IDC_EDIT_DESKTOPRXTX, m_szDesktopRXTX);
    DDX_Text(pDX, IDC_EDIT_FILESRXTX, m_szFilesRxTx);
    DDX_Text(pDX, IDC_EDIT_USERSSERVED, m_szUsersServed);
    DDX_Text(pDX, IDC_EDIT_USERSPEAK, m_szUsersPeak);
}

BEGIN_MESSAGE_MAP(CServerStatisticsDlg, CDialog)
END_MESSAGE_MAP()


BOOL CServerStatisticsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
