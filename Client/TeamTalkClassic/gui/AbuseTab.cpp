/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

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

    TRANSLATE(*this, IDD);

    CString szText; szText.LoadString(IDS_DISABLED);
    TRANSLATE_ITEM(IDS_DISABLED, szText);
    int i = m_wndCmdLimit.AddString(szText);
    m_wndCmdLimit.SetItemData(i, LIMITCMD_DISABLED);
    szText.LoadString(IDS_CMD10IN10);
    TRANSLATE_ITEM(IDS_CMD10IN10, szText);
    i = m_wndCmdLimit.AddString(szText);
    m_wndCmdLimit.SetItemData(i, LIMITCMD_10_PER_10SEC);
    szText.LoadString(IDS_CMD10IN60);
    TRANSLATE_ITEM(IDS_CMD10IN60, szText);
    i = m_wndCmdLimit.AddString(szText);
    m_wndCmdLimit.SetItemData(i, LIMITCMD_10_PER_MINUTE);
    szText.LoadString(IDS_CMD60IN60);
    TRANSLATE_ITEM(IDS_CMD60IN60, szText);
    i = m_wndCmdLimit.AddString(szText);
    m_wndCmdLimit.SetItemData(i, LIMITCMD_60_PER_MINUTE);
    szText.LoadString(IDS_CMDCUSTOM);
    TRANSLATE_ITEM(IDS_CMDCUSTOM, szText);
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
        CString szTitle; szTitle.LoadString(IDS_CMDLIMIT);
        TRANSLATE_ITEM(IDS_CMDLIMIT, szTitle);
        CString szMsg; szMsg.LoadString(IDS_CMDNO);
        TRANSLATE_ITEM(IDS_CMDNO, szMsg);
        CString szInitialMsg;
        szInitialMsg.Format(_T("%d"), m_abuse.nCommandsLimit);
        CInputDlg dlg_nCmd(szTitle, szMsg, szInitialMsg, this);
        if(dlg_nCmd.DoModal() == IDOK)
        {
            int nCmd = _ttoi(dlg_nCmd.m_szInput);
            if(nCmd>0)
            {
                CString szFmt; szFmt.LoadString(IDS_CMDTIMEFRAME);
                TRANSLATE_ITEM(IDS_CMDTIMEFRAME, szFmt);
                szMsg.Format(szFmt, nCmd);
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
