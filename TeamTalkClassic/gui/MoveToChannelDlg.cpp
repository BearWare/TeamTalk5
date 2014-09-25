// gui\MoveToChannelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MoveToChannelDlg.h"


// CMoveToChannelDlg dialog

IMPLEMENT_DYNAMIC(CMoveToChannelDlg, CDialog)

CMoveToChannelDlg::CMoveToChannelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMoveToChannelDlg::IDD, pParent)
    , m_szChannel(_T(""))
{

}

CMoveToChannelDlg::~CMoveToChannelDlg()
{
}

void CMoveToChannelDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_CBString(pDX, IDC_COMBO_CHANNELS, m_szChannel);
    DDX_Control(pDX, IDC_COMBO_CHANNELS, m_wndChannels);
}


BEGIN_MESSAGE_MAP(CMoveToChannelDlg, CDialog)
END_MESSAGE_MAP()


// CMoveToChannelDlg message handlers

BOOL CMoveToChannelDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    for(POSITION pos=m_Channels.GetHeadPosition();
        pos != NULL;)
        m_wndChannels.AddString(m_Channels.GetNext(pos));
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
