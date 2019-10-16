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

// StreamMediaDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../Resource.h"
#include "StreamMediaDlg.h"

extern TTInstance* ttInst;

#define MAX_FILENAMES 10

// CStreamMediaDlg dialog

IMPLEMENT_DYNAMIC(CStreamMediaDlg, CDialog)

CStreamMediaDlg::CStreamMediaDlg(teamtalk::ClientXML& xmlSettings, CWnd* pParent /*=NULL*/)
	: CDialog(CStreamMediaDlg::IDD, pParent)
    , m_xmlSettings(xmlSettings)
    , m_nVidCodecBitrate(DEFAULT_WEBM_VP8_BITRATE)
{

}

CStreamMediaDlg::~CStreamMediaDlg()
{
    if (m_nPlaybackID)
        TT_StopLocalPlayback(ttInst, m_nPlaybackID);
}

void CStreamMediaDlg::ProcessTTMessage(const TTMessage& msg)
{
    switch (msg.nClientEvent)
    {
    case CLIENTEVENT_LOCAL_MEDIAFILE :
        m_mfi = msg.mediafileinfo;
        switch (m_mfi.nStatus)
        {
        case MFS_CLOSED :
        case MFS_ERROR :
            break;
        case MFS_FINISHED:
            m_nPlaybackID = 0;
        case MFS_STARTED:
        case MFS_PLAYING:
        {

            double percent = m_mfi.uElapsedMSec / double(m_mfi.uDurationMSec);
            m_wndOffset.SetPos(int(m_wndOffset.GetRangeMax() * percent));

            UpdateOffset();
            break;
        }
        }
        break;
    }
}

void CStreamMediaDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_VIDCODEC, m_wndVidCodec);
    DDX_Text(pDX, IDC_EDIT_VIDBITRATE, m_nVidCodecBitrate);
    DDX_Control(pDX, IDC_SPIN_VIDBITRATE, m_wndVidBitrateSpinCtrl);
    DDX_Control(pDX, IDC_STATIC_AUDIOFORMAT, m_wndAudioFormat);
    DDX_Control(pDX, IDC_STATIC_VIDEOFORMAT, m_wndVideoFormat);
    DDX_Control(pDX, IDC_EDIT_VIDBITRATE, m_wndVideoBitrate);
    DDX_Control(pDX, IDC_COMBO_FILENAME, m_wndFilename);
    DDX_Control(pDX, IDC_COMBO_AUDIOPREPROCESSOR, m_wndAudioPreprocessor);
    DDX_Control(pDX, IDC_BUTTON_AUDIOSETUP, m_wndAudioSetup);
    DDX_Control(pDX, IDC_SLIDER_OFFSET, m_wndOffset);
    DDX_Control(pDX, IDC_STATIC_TIMEOFFSET, m_wndTimeOffset);
    DDX_Control(pDX, IDC_BUTTON_STOP, m_wndStopPlayback);
    DDX_Control(pDX, IDC_BUTTON_PLAY, m_wndStartPlayback);
    DDX_Control(pDX, IDC_STATIC_DURATION, m_wndDuration);
}


BEGIN_MESSAGE_MAP(CStreamMediaDlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_AUDIOBROWSE, &CStreamMediaDlg::OnBnClickedButtonBrowse)
    ON_BN_CLICKED(IDC_BUTTON_AUDIOSETUP, &CStreamMediaDlg::OnBnClickedButtonAudiosetup)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_OFFSET, &CStreamMediaDlg::OnNMCustomdrawSliderOffset)
    ON_BN_CLICKED(IDC_BUTTON_STOP, &CStreamMediaDlg::OnBnClickedButtonStop)
    ON_BN_CLICKED(IDC_BUTTON_PLAY, &CStreamMediaDlg::OnBnClickedButtonPlay)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_OFFSET, &CStreamMediaDlg::OnNMReleasedcaptureSliderOffset)
    ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER_OFFSET, &CStreamMediaDlg::OnTRBNThumbPosChangingSliderOffset)
END_MESSAGE_MAP()


// CStreamMediaDlg message handlers

BOOL CStreamMediaDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    for(POSITION pos = m_fileList.GetHeadPosition(); pos != nullptr;)
        m_wndFilename.AddString(m_fileList.GetNext(pos));
    m_wndFilename.SetCurSel(0);

    AddString(m_wndAudioPreprocessor, _T("No Audio Preprocessor"), NO_AUDIOPREPROCESSOR);
    AddString(m_wndAudioPreprocessor, _T("TeamTalk Audio Preprocessor"), TEAMTALK_AUDIOPREPROCESSOR);
    AddString(m_wndAudioPreprocessor, _T("Speex DSP Audio Preprocessor"), SPEEXDSP_AUDIOPREPROCESSOR);
    SetCurSelItemData(m_wndAudioPreprocessor, NO_AUDIOPREPROCESSOR);

    m_wndOffset.SetRange(0, 10000);
    m_wndOffset.SetPageSize(50);

    AddString(m_wndVidCodec, _T("WebM VP8"), WEBM_VP8_CODEC);
    SetCurSelItemData(m_wndVidCodec, WEBM_VP8_CODEC);
    m_wndVidBitrateSpinCtrl.SetRange(0, 1000);

    UpdateMediaFile();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CStreamMediaDlg::UpdateMediaFile()
{
    CString szFileName;
    m_wndFilename.GetWindowText(szFileName);

    TT_GetMediaFileInfo(szFileName, &m_mfi);
    BOOL audio = m_mfi.audioFmt.nAudioFmt != AFF_NONE;
    BOOL video = m_mfi.videoFmt.picFourCC != FOURCC_NONE;

    CString szAudioFormat = _T("Unknown format");
    CString szVideoFormat = _T("Unknown format");
    CString szDuration = _T("Unspecified");

    if(audio)
        szAudioFormat.Format(_T("%d Hz, %s"), m_mfi.audioFmt.nSampleRate,
                             (m_mfi.audioFmt.nChannels == 2? _T("Stereo") : _T("Mono")));
    if(video)
    {
        double fps = (double)m_mfi.videoFmt.nFPS_Numerator / (double)m_mfi.videoFmt.nFPS_Denominator;
        szVideoFormat.Format(_T("%dx%d %d FPS"), m_mfi.videoFmt.nWidth, m_mfi.videoFmt.nHeight,
                             (int)fps);
    }

    if (m_mfi.uDurationMSec)
    {
        int nDurationSec = m_mfi.uDurationMSec / 1000;
        szDuration.Format(_T("%d:%02d:%02d"), nDurationSec / 3600, (nDurationSec % 3600) / 60, nDurationSec % 60);
    }

    m_wndAudioFormat.SetWindowText(szAudioFormat);
    m_wndVideoFormat.SetWindowText(szVideoFormat);
    m_wndDuration.SetWindowText(szDuration);

    m_wndVidBitrateSpinCtrl.EnableWindow(video);
    m_wndVideoBitrate.EnableWindow(video);
}

void CStreamMediaDlg::OnBnClickedButtonBrowse()
{
    CString filetypes = _T("Media files (*.*)|*.*|");
    CFileDialog dlg(TRUE, 0, 0, OFN_FILEMUSTEXIST| OFN_HIDEREADONLY,filetypes, this);
    TCHAR s[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, s);

    if(dlg.DoModal() == IDOK)
    {
        m_wndFilename.SetWindowText(dlg.GetPathName());
        int nIndex = m_wndFilename.FindString(-1, dlg.GetPathName());
        if (nIndex != CB_ERR)
            m_wndFilename.DeleteString(nIndex);
        m_wndFilename.InsertString(0, dlg.GetPathName());
        if (m_wndFilename.GetCount() > MAX_FILENAMES)
            m_wndFilename.DeleteString(MAX_FILENAMES);
        UpdateMediaFile();
    }

    SetCurrentDirectory(s);
}


void CStreamMediaDlg::OnOK()
{
    CDialog::OnOK();

    m_fileList.RemoveAll();
    CString szFilename;

    m_wndFilename.GetWindowText(szFilename);
    m_fileList.AddTail(szFilename);

    for (int i=0;i<m_wndFilename.GetCount();++i)
    {
        CString s;
        m_wndFilename.GetLBText(i, s);
        if (s.CompareNoCase(szFilename) != 0)
            m_fileList.AddTail(s);
    }
}

void CStreamMediaDlg::OnBnClickedButtonAudiosetup()
{
}

void CStreamMediaDlg::OnNMCustomdrawSliderOffset(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    // TODO: Add your control notification handler code here
    *pResult = 0;

    if(m_nPlaybackID)
        return;

    double percent = m_wndOffset.GetPos() / double(m_wndOffset.GetRangeMax());
    m_mfi.uElapsedMSec = UINT32(m_mfi.uDurationMSec * percent);
    UpdateOffset();
}

void CStreamMediaDlg::OnNMReleasedcaptureSliderOffset(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: Add your control notification handler code here
    *pResult = 0;
}

void CStreamMediaDlg::OnTRBNThumbPosChangingSliderOffset(NMHDR *pNMHDR, LRESULT *pResult)
{
    // This feature requires Windows Vista or greater.
    // The symbol _WIN32_WINNT must be >= 0x0600.
    NMTRBTHUMBPOSCHANGING *pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING *>(pNMHDR);
    // TODO: Add your control notification handler code here
    *pResult = 0;
}

void CStreamMediaDlg::OnBnClickedButtonStop()
{
    if (m_nPlaybackID <= 0)
        return;

    if (m_mfi.nStatus == MFS_PLAYING)
    {
        m_mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
        m_mfp.bPaused = TRUE;
        if (TT_UpdateLocalPlayback(ttInst, m_nPlaybackID, &m_mfp))
            return;
    }

    TT_StopLocalPlayback(ttInst, m_nPlaybackID);
    m_nPlaybackID = 0;

    m_mfp = {};
    m_mfi.uElapsedMSec = 0;
    m_wndOffset.SetPos(0);
    UpdateOffset();
}

void CStreamMediaDlg::OnBnClickedButtonPlay()
{
    if ((TT_GetFlags(ttInst) & (CLIENT_SNDINOUTPUT_DUPLEX | CLIENT_SNDOUTPUT_READY)) == CLIENT_CLOSED)
    {
        if (!InitSoundSystem(m_xmlSettings))
        {
            MessageBox(LoadText(IDS_SNDINITFAILED), LoadText(IDS_PLAY));
        }
    }

    if(m_mfi.nStatus == MFS_PAUSED)
    {
        double percent = m_wndOffset.GetPos() / double(m_wndOffset.GetRangeMax());
        m_mfp.uOffsetMSec = UINT32(m_mfi.uDurationMSec * percent);
        m_mfp.bPaused = FALSE;
        if (TT_UpdateLocalPlayback(ttInst, m_nPlaybackID, &m_mfp))
            return;
        else
        {
            MessageBox(_T("Failed to resume"), LoadText(IDS_PLAY));
        }
        TT_StopLocalPlayback(ttInst, m_nPlaybackID);
    }

    CString szFilename;
    m_wndFilename.GetWindowText(szFilename);

    if (szFilename.CompareNoCase(m_mfi.szFileName) != 0)
        UpdateMediaFile();

    m_mfp.bPaused = FALSE;
    m_mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    if (m_wndOffset.GetPos())
    {
        double percent = m_wndOffset.GetPos() / double(m_wndOffset.GetRangeMax());
        m_mfp.uOffsetMSec = UINT32(m_mfi.uDurationMSec * percent);
    }

    m_nPlaybackID = TT_InitLocalPlayback(ttInst, szFilename, &m_mfp);
    if (m_nPlaybackID <= 0)
    {
        MessageBox(_T("Failed to start playback"), LoadText(IDS_PLAY));
        return;
    }
}

void CStreamMediaDlg::UpdateOffset()
{
    UINT32 uHours = m_mfi.uElapsedMSec / (60 * 60 * 1000);
    UINT32 uRemain = m_mfi.uElapsedMSec - (60 * 60 * 1000 * uHours);
    UINT32 uMinutes = uRemain / (60 * 1000);
    uRemain -= 60 * 1000 * uMinutes;
    UINT32 uSeconds = uRemain / 1000;
    UINT32 uMSec = uRemain % 1000;

    CString szElapsed;
    szElapsed.Format(_T("%d:%02d:%02d.%03d"), uHours, uMinutes, uSeconds, uMSec);
    m_wndTimeOffset.SetWindowText(szElapsed);
}
