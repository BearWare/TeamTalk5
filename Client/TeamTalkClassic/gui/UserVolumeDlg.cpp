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
#include "UserVolumeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
extern TTInstance* ttInst;

// CUserVolumeDlg dialog

IMPLEMENT_DYNAMIC(CUserVolumeDlg, CDialog)
    CUserVolumeDlg::CUserVolumeDlg(const User& user, CWnd* pParent /*=NULL*/)
    : CDialog(CUserVolumeDlg::IDD, pParent)
    , m_user(user)
{
}

CUserVolumeDlg::~CUserVolumeDlg()
{
}

void CUserVolumeDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SLIDER_MEDIAFILEVOL, m_wndMediaFileVol);
    DDX_Control(pDX, IDC_CHECK_MUTELEFTMEDIAFILE, m_wndMediaFileMuteLeft);
    DDX_Control(pDX, IDC_CHECK_MUTERIGHTMEDIAFILE, m_wndMediaFileMuteRight);
    DDX_Control(pDX, IDC_CHECK_MUTELEFTVOICE, m_wndVoiceMuteLeft);
    DDX_Control(pDX, IDC_CHECK_MUTERIGHTVOICE, m_wndVoiceMuteRight);
    DDX_Control(pDX, IDC_SLIDER_VOICEVOL, m_wndVoiceVol);
}

void CUserVolumeDlg::UpdateProperties()
{
    TT_SetUserVolume(ttInst, m_user.nUserID, STREAMTYPE_VOICE,
                     RefVolume(m_wndVoiceVol.GetPos()));
    TT_SetUserVolume(ttInst, m_user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO,
                     RefVolume(m_wndMediaFileVol.GetPos()));
    
    TT_SetUserStereo(ttInst, m_user.nUserID, STREAMTYPE_VOICE,
                     m_wndVoiceMuteLeft.GetCheck() != BST_CHECKED,
                     m_wndVoiceMuteRight.GetCheck() != BST_CHECKED);
    TT_SetUserStereo(ttInst, m_user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO,
                     m_wndMediaFileMuteLeft.GetCheck() != BST_CHECKED,
                     m_wndMediaFileMuteRight.GetCheck() != BST_CHECKED);
}

BEGIN_MESSAGE_MAP(CUserVolumeDlg, CDialog)
    ON_BN_CLICKED(IDC_CHECK_MUTERIGHTVOICE, &CUserVolumeDlg::OnBnClickedCheckMuterightvoice)
    ON_BN_CLICKED(IDC_CHECK_MUTELEFTVOICE, &CUserVolumeDlg::OnBnClickedCheckMuteleftvoice)
    ON_BN_CLICKED(IDC_CHECK_MUTELEFTMEDIAFILE, &CUserVolumeDlg::OnBnClickedCheckMuteleftmediafile)
    ON_BN_CLICKED(IDC_CHECK_MUTERIGHTMEDIAFILE, &CUserVolumeDlg::OnBnClickedCheckMuterightmediafile)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_VOICEVOL, &CUserVolumeDlg::OnNMCustomdrawSliderVoicevol)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_MEDIAFILEVOL, &CUserVolumeDlg::OnNMCustomdrawSliderMediafilevol)
    ON_BN_CLICKED(IDCANCEL, &CUserVolumeDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BUTTON_DEFAULT, &CUserVolumeDlg::OnBnClickedButtonDefault)
END_MESSAGE_MAP()


// CUserVolumeDlg message handlers
BOOL CUserVolumeDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    m_wndVoiceVol.SetRange(0, 100, TRUE);
    m_wndVoiceVol.SetPos(RefVolumeToPercent(m_user.nVolumeVoice));
    m_wndVoiceVol.SetPageSize(m_wndVoiceVol.GetRangeMax() / 20);

    m_wndMediaFileVol.SetRange(0, 100, TRUE);
    m_wndMediaFileVol.SetPos(RefVolumeToPercent(m_user.nVolumeMediaFile));
    m_wndMediaFileVol.SetPageSize(m_wndMediaFileVol.GetRangeMax() / 20);

    m_wndVoiceMuteLeft.SetCheck(!m_user.stereoPlaybackVoice[0]?BST_CHECKED:BST_UNCHECKED);
    m_wndVoiceMuteRight.SetCheck(!m_user.stereoPlaybackVoice[1]?BST_CHECKED:BST_UNCHECKED);

    m_wndMediaFileMuteLeft.SetCheck(!m_user.stereoPlaybackMediaFile[0]?BST_CHECKED:BST_UNCHECKED);
    m_wndMediaFileMuteRight.SetCheck(!m_user.stereoPlaybackMediaFile[1]?BST_CHECKED:BST_UNCHECKED);

    SetWindowText(CString(LoadText(IDS_USERVOLUMETITLEWINDOW, _T("Volume for "))) + GetDisplayName(m_user));
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CUserVolumeDlg::OnNMCustomdrawSliderUservol(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    *pResult = 0;
}

void CUserVolumeDlg::OnBnClickedCheckMuterightvoice()
{
    UpdateProperties();
}


void CUserVolumeDlg::OnBnClickedCheckMuteleftvoice()
{
    UpdateProperties();
}


void CUserVolumeDlg::OnBnClickedCheckMuteleftmediafile()
{
    UpdateProperties();
}


void CUserVolumeDlg::OnBnClickedCheckMuterightmediafile()
{
    UpdateProperties();
}

void CUserVolumeDlg::OnNMCustomdrawSliderVoicevol(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    *pResult = 0;
    UpdateProperties();
}


void CUserVolumeDlg::OnNMCustomdrawSliderMediafilevol(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    *pResult = 0;
    UpdateProperties();
}


void CUserVolumeDlg::OnBnClickedCancel()
{
    CDialog::OnCancel();
}


void CUserVolumeDlg::OnBnClickedButtonDefault()
{
    m_wndVoiceVol.SetPos(RefVolumeToPercent(SOUND_VOLUME_DEFAULT));
    m_wndMediaFileVol.SetPos(RefVolumeToPercent(SOUND_VOLUME_DEFAULT));
    UpdateProperties();
}
