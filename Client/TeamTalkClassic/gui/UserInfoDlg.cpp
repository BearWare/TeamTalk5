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

#include "stdafx.h"
#include "Resource.h"
#include "UserInfoDlg.h"
#include <AppInfo.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern TTInstance* ttInst;

// CUserInfoDlg dialog

IMPLEMENT_DYNAMIC(CUserInfoDlg, CDialog)
CUserInfoDlg::CUserInfoDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CUserInfoDlg::IDD, pParent)
    , m_nUserID(0)
    , m_szNick(_T(""))
    , m_szPacketloss(_T(""))
    , m_nPortNumber(0)
    , m_szVersion(_T(""))
    , m_szUsername(_T(""))
    , m_szUserType(_T(""))
    , m_szIPAddr(_T(""))
    , m_szMFVideoLoss(_T(""))
    , m_szMFAudioLoss(_T(""))
    , m_szClientName(_T(""))
{
}

CUserInfoDlg::~CUserInfoDlg()
{
}

void CUserInfoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_USERID, m_nUserID);
    DDX_Text(pDX, IDC_EDIT_NICKNAME, m_szNick);
    DDX_Text(pDX, IDC_STATIC_VOICELOSS, m_szPacketloss);
    DDX_Text(pDX, IDC_EDIT_CLIENTVER, m_szVersion);
    DDX_Text(pDX, IDC_EDIT_USERNAME, m_szUsername);
    DDX_Text(pDX, IDC_EDIT_USERTYPE, m_szUserType);
    DDX_Text(pDX, IDC_EDIT_IPADDR, m_szIPAddr);
    DDX_Text(pDX, IDC_STATIC_MFVIDEOLOSS, m_szMFVideoLoss);
    DDX_Text(pDX, IDC_STATIC_MFAUDIOLOSS, m_szMFAudioLoss);
    DDX_Control(pDX, IDC_STATIC_VOICELOSS, m_wndVoiceLoss);
    DDX_Control(pDX, IDC_STATIC_MFAUDIOLOSS, m_wndMFAudioLoss);
    DDX_Control(pDX, IDC_STATIC_MFVIDEOLOSS, m_wndMFVideoLoss);
    DDX_Text(pDX, IDC_EDIT_CLIENTNAME, m_szClientName);
}

BEGIN_MESSAGE_MAP(CUserInfoDlg, CDialog)
    ON_BN_CLICKED(IDOK, &CUserInfoDlg::OnBnClickedOk)
    ON_WM_TIMER()
END_MESSAGE_MAP()

// CUserInfoDlg message handlers

BOOL CUserInfoDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    UpdateStats();

    SetTimer(0, 1000, NULL);

    return TRUE;
}

void CUserInfoDlg::OnBnClickedOk()
{
    CDialog::OnOK();
}

void CUserInfoDlg::OnTimer(UINT_PTR nIDEvent)
{
    CDialog::OnTimer(nIDEvent);

    UpdateStats();
}

void CUserInfoDlg::UpdateStats()
{
    CString s;
    UserStatistics stats;
    if(TT_GetUserStatistics(ttInst, m_nUserID, &stats))
    {
        s.Format(_T("%I64d/%I64d"), stats.nVoicePacketsLost, stats.nVoicePacketsRecv + stats.nVoicePacketsLost);
        m_wndVoiceLoss.SetWindowText(s);
        s.Format(_T("%I64d/%I64d"), stats.nMediaFileAudioPacketsLost, stats.nMediaFileAudioPacketsRecv + stats.nMediaFileAudioPacketsLost);
        m_wndMFAudioLoss.SetWindowText(s);
        s.Format(_T("%I64d/%I64d"), stats.nMediaFileVideoFramesLost, stats.nMediaFileVideoFramesRecv + stats.nMediaFileVideoFramesLost);
        m_wndMFVideoLoss.SetWindowText(s);
    }
}
