// C:\Programming\Projects\BearWare\TeamTalk5\TeamTalkClassic\gui\AbuseTab.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "AbuseTab.h"
#include "afxdialogex.h"

// CAbuseTab dialog

IMPLEMENT_DYNAMIC(CAbuseTab, CDialog)

CAbuseTab::CAbuseTab(CWnd* pParent /*=NULL*/)
	: CMyTab(IDD_TAB_ABUSE, pParent)
{

}

CAbuseTab::~CAbuseTab()
{
}

void CAbuseTab::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_CMDLIMIT, m_wndCmdLimit);
}


BEGIN_MESSAGE_MAP(CAbuseTab, CDialog)
END_MESSAGE_MAP()


// CAbuseTab message handlers
