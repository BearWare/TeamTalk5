// C:\Programming\Projects\BearWare\TeamTalk5\TeamTalkClassic\gui\AbuseTab.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "AbuseTab.h"
#include "afxdialogex.h"
#include "InputDlg.h"

// CAbuseTab dialog

IMPLEMENT_DYNAMIC(CAbuseTab, CDialog)

CAbuseTab::CAbuseTab(CWnd* pParent /*=NULL*/)
	: CMyTab(IDD_TAB_ABUSE, pParent)
    , m_abuse()
{

}

CAbuseTab::~CAbuseTab()
{
}

void CAbuseTab::DoDataExchange(CDataExchange* pDX)
{
    CMyTab::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_CMDLIMIT, m_wndCmdLimit);
}


BEGIN_MESSAGE_MAP(CAbuseTab, CMyTab)
    ON_CBN_SELCHANGE(IDC_COMBO_CMDLIMIT, &CAbuseTab::OnCbnSelchangeComboCmdlimit)
END_MESSAGE_MAP()


// CAbuseTab message handlers


BOOL CAbuseTab::OnInitDialog()
{
    CMyTab::OnInitDialog();

    CString szText = _T("Disabled");
    int i = m_wndCmdLimit.AddString(szText);
    m_wndCmdLimit.SetItemData(i, LIMITCMD_DISABLED);
    szText = _T("10 commands in 10 sec.");
    i = m_wndCmdLimit.AddString(szText);
    m_wndCmdLimit.SetItemData(i, LIMITCMD_10_PER_10SEC);
    szText = _T("10 commands in 1 minute");
    i = m_wndCmdLimit.AddString(szText);
    m_wndCmdLimit.SetItemData(i, LIMITCMD_10_PER_MINUTE);
    szText = _T("60 commands in 1 minute");
    i = m_wndCmdLimit.AddString(szText);
    m_wndCmdLimit.SetItemData(i, LIMITCMD_60_PER_MINUTE);
    szText = _T("Custom");
    i = m_wndCmdLimit.AddString(szText);
    m_wndCmdLimit.SetItemData(i, LIMITCMD_CUSTOM);

    ShowAbuseInfo();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAbuseTab::ShowAbuseInfo()
{
    int result = LIMITCMD_DISABLED;
    switch(m_abuse.nCommandsLimit)
    {
    case 0 :
        result = LIMITCMD_DISABLED;
        break;
    case 10 :
        switch(m_abuse.nCommandsIntervalMSec)
        {
        case 10 * 1000 :
            result = LIMITCMD_10_PER_10SEC;
            break;
        case 60 * 1000:
            result = LIMITCMD_10_PER_MINUTE;
            break;
        default:
            result = LIMITCMD_CUSTOM;
            break;
        }
        break;
    case 60 :
        switch(m_abuse.nCommandsIntervalMSec)
        {
        case 60 * 1000 :
            result = LIMITCMD_60_PER_MINUTE;
            break;
        default :
            result = LIMITCMD_CUSTOM;
            break;
        }
        break;
    default:
        result = LIMITCMD_CUSTOM;
        break;
    }

    for(int i=0;i<m_wndCmdLimit.GetCount();++i)
    {
        if(m_wndCmdLimit.GetItemData(i) == result)
        {
            m_wndCmdLimit.SetCurSel(i);
            break;
        }
    }
}

void CAbuseTab::OnCbnSelchangeComboCmdlimit()
{
    int i = m_wndCmdLimit.GetCurSel();
    switch(m_wndCmdLimit.GetItemData(i))
    {
    case LIMITCMD_DISABLED:
        m_abuse.nCommandsIntervalMSec = m_abuse.nCommandsLimit = 0;
        break;
    case LIMITCMD_10_PER_10SEC:
        m_abuse.nCommandsIntervalMSec = 10 * 1000;
        m_abuse.nCommandsLimit = 10;
        break;
    case LIMITCMD_10_PER_MINUTE:
        m_abuse.nCommandsIntervalMSec = 60 * 1000;
        m_abuse.nCommandsLimit = 10;
        break;
    case LIMITCMD_60_PER_MINUTE:
        m_abuse.nCommandsIntervalMSec = 60 * 1000;
        m_abuse.nCommandsLimit = 60;
        break;
    case LIMITCMD_CUSTOM:
        CString szTitle = _T("Limit issued commands"),
            szMsg = _T("Number of commands to allow (0 = disabled)"),
            szInitialMsg;
        szInitialMsg.Format(_T("%d"), m_abuse.nCommandsLimit);
        CInputDlg dlg_nCmd(szTitle, szMsg, szInitialMsg, this);
        if(dlg_nCmd.DoModal() == IDOK)
        {
            int nCmd = _ttoi(dlg_nCmd.m_szInput);
            if(nCmd>0)
            {
                szMsg.Format(_T("Timeframe to allow %d commands (in seconds)"), nCmd);
                szInitialMsg.Format(_T("%d"), m_abuse.nCommandsIntervalMSec / 1000);
                CInputDlg dlg_nSec(szTitle, szMsg, szInitialMsg, this);
                if(dlg_nSec.DoModal() == IDOK)
                {
                    m_abuse.nCommandsLimit = nCmd;
                    m_abuse.nCommandsIntervalMSec = _ttoi(dlg_nSec.m_szInput) * 1000;
                }
            }
            else
            {
                m_abuse.nCommandsIntervalMSec = m_abuse.nCommandsLimit = 0;
            }
        }
        break;
    }
}
