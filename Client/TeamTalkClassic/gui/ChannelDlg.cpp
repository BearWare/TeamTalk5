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
#include "ChannelDlg.h"
#include "../Helper.h"

extern TTInstance* ttInst;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CCreateChannelDlg dialog

IMPLEMENT_DYNAMIC(CChannelDlg, CDialog)
CChannelDlg::CChannelDlg(CChannelDlg::ChannelDlgType dlgType, CWnd* pParent /*=NULL*/)
: CDialog(CChannelDlg::IDD, pParent)
, m_szChannelname(_T(""))
, m_szChannelPassword(_T(""))
, m_szChannelTopic(_T(""))
, m_bStaticChannel(FALSE)
, m_nType(dlgType)
, m_nDiskQuota(0)
, m_nMaxUsers(0)
, m_bSingleTxChannel(FALSE)
, m_bClassRoom(FALSE)
, m_bEnableAGC(DEFAULT_CHANNEL_AUDIOCONFIG)
, m_nGainLevel(DEFAULT_AGC_GAINLEVEL)
, m_nTxDelay(DEFAULT_MSEC_PER_PACKET)
, m_bOpRecvOnly(FALSE)
, m_bNoVoiceAct(FALSE)
, m_bNoRecord(FALSE)
, m_bVBR(FALSE)
{
    InitDefaultAudioCodec(m_codec);
}

CChannelDlg::~CChannelDlg()
{
}

void CChannelDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_CHANNAME, m_szChannelname);
    DDX_Text(pDX, IDC_EDIT_CHANPASSWORD, m_szChannelPassword);
    DDX_Text(pDX, IDC_EDIT_CHANTOPIC, m_szChannelTopic);
    DDX_Text(pDX, IDC_EDIT_OPPASSWD, m_szOpPasswd);
    DDX_Control(pDX, IDC_EDIT_CHANPASSWORD, m_wndPassword);
    DDX_Control(pDX, IDC_STATIC_GRPCHANNEL, m_wndGrp);
    DDX_Control(pDX, IDC_EDIT_CHANNAME, m_wndChannelName);
    DDX_Control(pDX, IDC_EDIT_CHANTOPIC, m_wndChanTopic);
    DDX_Control(pDX, IDC_SLIDER_QUALITY, m_wndQuality);
    DDX_Text(pDX, IDC_EDIT_DISKQUOTA, m_nDiskQuota);
    DDX_Text(pDX, IDC_EDIT_MAXUSERS, m_nMaxUsers);
    DDX_Control(pDX, IDC_EDIT_DISKQUOTA, m_wndDiskQuota);
    DDX_Control(pDX, IDC_EDIT_MAXUSERS, m_wndMaxUsers);
    DDX_Check(pDX, IDC_CHECK_SINGLETXCHANNEL, m_bSingleTxChannel);
    DDX_Check(pDX, IDC_CHECK_CHANNELPERMANENT, m_bStaticChannel);
    DDX_Control(pDX, IDC_EDIT_OPPASSWD, m_wndOpPasswd);
    DDX_Control(pDX, IDC_CHECK_CHANNELPERMANENT, m_btnStaticChannel);
    DDX_Control(pDX, IDC_CHECK_SINGLETXCHANNEL, m_btnSingleChannel);
    DDX_Control(pDX, IDC_COMBO_AUDIOCHANNELS, m_wndAudioChannels);
    DDX_Check(pDX, IDC_CHECK_CLASSROOM, m_bClassRoom);
    DDX_Control(pDX, IDC_CHECK_CLASSROOM, m_btnClassRoom);
    DDX_Control(pDX, IDC_CHECK_FORCEAGC, m_btnForceAGC);
    DDX_Control(pDX, IDC_SLIDER_GAINLEVEL, m_wndGainSlider);
    DDX_Check(pDX, IDC_CHECK_FORCEAGC, m_bEnableAGC);
    DDX_Slider(pDX, IDC_SLIDER_GAINLEVEL, m_nGainLevel);
    DDX_Control(pDX, IDC_STATIC_GAINLEVEL, m_wndGainLabel);
    DDX_Control(pDX, IDC_COMBO_CODEC, m_wndCodec);
    DDX_Control(pDX, IDC_EDIT_BITRATE, m_wndBitrate);
    DDX_Control(pDX, IDC_EDIT_MAXBITRATE, m_wndMaxBitrate);
    DDX_Control(pDX, IDC_CHECK_DTX, m_wndDtx);
    DDX_Control(pDX, IDC_STATIC_RECORDERFREQ, m_wndSRLabel);
    DDX_Control(pDX, IDC_STATIC_AUDCHANNELS, m_wndACLabel);
    DDX_Control(pDX, IDC_STATIC_VOICEQUALITY, m_wndQualityLabel);
    DDX_Control(pDX, IDC_STATIC_BITRATE, m_wndBpsLabel);
    DDX_Control(pDX, IDC_STATIC_MAXBITRATE, m_wndMaxBpsLabel);
    DDX_Control(pDX, IDC_STATIC_BPS1, m_wndBpsLabel1);
    DDX_Control(pDX, IDC_STATIC_BPS2, m_wndBpsLabel2);
    DDX_Control(pDX, IDC_EDIT_TXDELAY, m_wndTxDelay);
    DDX_Text(pDX, IDC_EDIT_TXDELAY, m_nTxDelay);
    DDV_MinMaxInt(pDX, m_nTxDelay, AUDIOCODEC_MIN_TXINTERVALMSEC, AUDIOCODEC_MAX_TXINTERVALMSEC);
    DDX_Control(pDX, IDC_SPIN_DELAY, m_wndTxDelaySpin);
    DDX_Control(pDX, IDC_STATIC_TXINTERVAL, m_wndTxDelayLabel);
    DDX_Control(pDX, IDC_STATIC_TXI1, m_wndTxDelayLabel1);
    DDX_Check(pDX, IDC_CHECK_OPRECVONLY, m_bOpRecvOnly);
    DDX_Control(pDX, IDC_CHECK_OPRECVONLY, m_wndOpRecvOnly);
    DDX_Control(pDX, IDC_COMBO_SRATE, m_wndSampleRate);
    DDX_Control(pDX, IDC_COMBO_CODECAPP, m_wndCodecApp);
    DDX_Control(pDX, IDC_STATIC_APPLICATION, m_wndAppLabel);
    DDX_Control(pDX, IDC_CHECK_NOVOICEACT, m_wndNoVoiceAct);
    DDX_Check(pDX, IDC_CHECK_NOVOICEACT, m_bNoVoiceAct);
    DDX_Control(pDX, IDC_CHECK_NORECORD, m_wndNoRecord);
    DDX_Check(pDX, IDC_CHECK_NORECORD, m_bNoRecord);
    DDX_Check(pDX, IDC_CHECK_VBR, m_bVBR);
    DDX_Control(pDX, IDC_CHECK_VBR, m_wndVBR);
    DDX_Control(pDX, IDC_COMBO_OPUSFRAMESIZES, m_wndOpusFrameSizes);
    DDX_Control(pDX, IDC_STATIC_OPUSFRMSIZE, m_wndFrmSizeStatic);
}


BEGIN_MESSAGE_MAP(CChannelDlg, CDialog)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_QUALITY, OnNMCustomdrawSliderQuality)
    ON_CBN_SELCHANGE(IDC_COMBO_AUDIOCHANNELS, &CChannelDlg::OnCbnSelchangeComboAudiochannels)
    ON_BN_CLICKED(IDC_CHECK_FORCEAGC, &CChannelDlg::OnBnClickedCheckForceagc)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_GAINLEVEL, &CChannelDlg::OnNMCustomdrawSliderGainlevel)
    ON_CBN_SELCHANGE(IDC_COMBO_CODEC, &CChannelDlg::OnCbnSelchangeComboCodec)
    ON_EN_CHANGE(IDC_EDIT_BITRATE, &CChannelDlg::OnEnChangeEditBitrate)
    ON_EN_CHANGE(IDC_EDIT_MAXBITRATE, &CChannelDlg::OnEnChangeEditMaxbitrate)
    ON_BN_CLICKED(IDC_CHECK_DTX, &CChannelDlg::OnBnClickedCheckDtx)
    ON_EN_KILLFOCUS(IDC_EDIT_BITRATE, &CChannelDlg::OnEnKillfocusEditBitrate)
    ON_EN_KILLFOCUS(IDC_EDIT_MAXBITRATE, &CChannelDlg::OnEnKillfocusEditMaxbitrate)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_DELAY, &CChannelDlg::OnDeltaposSpinDelay)
    ON_EN_KILLFOCUS(IDC_EDIT_TXDELAY, &CChannelDlg::OnEnKillfocusEditTxdelay)
END_MESSAGE_MAP()

// CCreateChannelDlg message handlers

BOOL CChannelDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    ServerProperties prop;
    TT_GetServerProperties(ttInst, &prop);

    AddString(m_wndCodec, _T("No Audio"), NO_CODEC);
    AddString(m_wndCodec, _T("Speex"), SPEEX_CODEC);
    AddString(m_wndCodec, _T("Speex Variable Bitrate"), SPEEX_VBR_CODEC);
    AddString(m_wndCodec, _T("OPUS"), OPUS_CODEC);
    SetCurSelItemData(m_wndCodec, m_codec.nCodec);

    m_wndGainSlider.SetRange(1, 32, TRUE);

    m_btnStaticChannel.EnableWindow(TT_GetMyUserRights(ttInst) & USERRIGHT_MODIFY_CHANNELS);
    
    BOOL bEnableChan = (TT_GetMyUserRights(ttInst) & USERRIGHT_MODIFY_CHANNELS) != USERRIGHT_NONE;
    if(!bEnableChan)
    {
        m_wndDiskQuota.EnableWindow(FALSE);
        m_wndMaxUsers.EnableWindow(FALSE);
    }

    if(m_nType == CREATE_CHANNEL)
    {
        CString szCh, szNewCh;
        szCh.LoadString(IDS_CREATECHANNEL);
        SetWindowText(szCh);
        TRANSLATE(*this, IDS_CREATECHANNEL);

        szNewCh.LoadString(IDS_CREATENEWCHANNEL);
        m_wndGrp.SetWindowText(szNewCh);
        TRANSLATE(m_wndGrp, IDS_CREATENEWCHANNEL);

        OnBnClickedCheckForceagc();
    }
    else if(m_nType == UPDATE_CHANNEL)
    {
        CString szCh, szUpdCh;
        szCh.LoadString(IDS_UPDATECHANNEL);
        SetWindowText(szCh);
        TRANSLATE(*this, IDS_UPDATECHANNEL);

        szUpdCh.LoadString(IDS_UPDEXISTINGCHANNEL);
        m_wndGrp.SetWindowText(szUpdCh);
        TRANSLATE(m_wndGrp, IDS_UPDEXISTINGCHANNEL);

        TRANSLATE(m_wndGrp, IDS_UPDEXISTINGCHANNEL);

        OnBnClickedCheckForceagc();
    }
    else if(m_nType == READONLY_CHANNEL)
    {
        m_wndChannelName.SetReadOnly(TRUE);
        m_wndChanTopic.SetReadOnly(TRUE);
        m_wndPassword.SetReadOnly(TRUE);
        m_wndDiskQuota.SetReadOnly(TRUE);
        m_wndMaxUsers.SetReadOnly(TRUE);
        m_wndOpPasswd.SetReadOnly(TRUE);
        m_btnStaticChannel.EnableWindow(FALSE);
        m_btnSingleChannel.EnableWindow(FALSE);
        m_btnClassRoom.EnableWindow(FALSE);
        m_wndAudioChannels.EnableWindow(FALSE);
        m_btnForceAGC.EnableWindow(FALSE);
        m_wndGainSlider.EnableWindow(FALSE);
        m_wndCodec.EnableWindow(FALSE);
        m_wndSampleRate.EnableWindow(FALSE);
        m_wndCodecApp.EnableWindow(FALSE);
        m_wndQuality.EnableWindow(FALSE);
        m_wndBitrate.EnableWindow(FALSE);
        m_wndMaxBitrate.EnableWindow(FALSE);
        m_wndVBR.EnableWindow(FALSE);
        m_wndDtx.EnableWindow(FALSE);
        m_wndTxDelay.EnableWindow(FALSE);
        m_wndTxDelaySpin.EnableWindow(FALSE);
        m_wndOpusFrameSizes.EnableWindow(FALSE);
        m_wndOpRecvOnly.EnableWindow(FALSE);
        m_wndNoVoiceAct.EnableWindow(FALSE);
        m_wndNoRecord.EnableWindow(FALSE);
    }

    DisplayCodecControls(m_codec.nCodec); //show the correct controls for specified codec

    ShowCurrentCodec();
    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CChannelDlg::OnOK()
{
    AudioCodec codec = m_codec;
    UpdateCodec();

    CString s;
    m_wndChannelName.GetWindowText(s);
    if(s.GetLength() == 0 && m_nType == CREATE_CHANNEL)
    {
        AfxMessageBox(_T("Enter a channel name"));
        m_wndChannelName.SetFocus();
    }
    else if(s.Find('/') != -1)
    {
        AfxMessageBox(_T("A channel name cannot contain a '/'"));
        m_wndChannelName.SetFocus();
    }
    else
        CDialog::OnOK();
}

void CChannelDlg::OnNMCustomdrawSliderQuality(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    *pResult = 0;
}

void CChannelDlg::OnCbnSelchangeComboAudiochannels()
{
}

void CChannelDlg::OnEnChangeEditBitrate()
{
}

void CChannelDlg::OnEnChangeEditMaxbitrate()
{
}

void CChannelDlg::OnBnClickedCheckDtx()
{
}

void CChannelDlg::OnBnClickedCheckForceagc()
{
    m_wndGainSlider.EnableWindow(m_btnForceAGC.GetCheck() == BST_CHECKED);
}

void CChannelDlg::OnNMCustomdrawSliderGainlevel(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    *pResult = 0;
    SetWindowNumber(m_wndGainLabel, m_wndGainSlider.GetPos()*1000);
}

void CChannelDlg::DisplayCodecControls(Codec nCodec)
{
    int bpsMin = 0, bpsMax = 0, bpsDefault = 0;
    GetBitrateLimits(nCodec, bpsMin, bpsMax, bpsDefault);

    switch(nCodec)
    {
    case SPEEX_CODEC :
        //sample rate
        m_wndSRLabel.ShowWindow(SW_SHOW);
        m_wndSampleRate.ShowWindow(SW_SHOW);
        m_wndSampleRate.ResetContent();
        AddString(m_wndSampleRate, _T("8 KHz"), SPEEX_MODEID_NB);
        AddString(m_wndSampleRate, _T("16 KHz"), SPEEX_MODEID_WB);
        AddString(m_wndSampleRate, _T("32 KHz"), SPEEX_MODEID_UWB);
        SetCurSelItemData(m_wndSampleRate, DEFAULT_SPEEX_BANDMODE);
        //channels
        m_wndACLabel.ShowWindow(SW_HIDE);
        m_wndAudioChannels.ShowWindow(SW_HIDE);
        m_wndAudioChannels.ResetContent();
        AddString(m_wndAudioChannels, _T("Mono"), 1);
        AddString(m_wndAudioChannels, _T("Stereo"), 2);
        SetCurSelItemData(m_wndAudioChannels, DEFAULT_SPEEX_SIMSTEREO ? 2 : 1);
        //quality
        m_wndQuality.SetRange(0, 10, TRUE);
        m_wndQuality.SetPos(DEFAULT_SPEEX_QUALITY);
        m_wndQualityLabel.ShowWindow(SW_SHOW);
        m_wndQuality.ShowWindow(SW_SHOW);
        //tx delay
        m_wndTxDelayLabel.ShowWindow(SW_SHOW);
        m_wndTxDelay.ShowWindow(SW_SHOW);
        m_wndTxDelaySpin.ShowWindow(SW_SHOW);
        m_wndTxDelayLabel1.ShowWindow(SW_SHOW);
        m_wndTxDelaySpin.SetRange(SPEEX_MIN_TXINTERVALMSEC,
                                  SPEEX_MAX_TXINTERVALMSEC);
        SetWindowNumber(m_wndTxDelay, DEFAULT_SPEEX_DELAY);

        m_wndAppLabel.ShowWindow(SW_HIDE);
        m_wndCodecApp.ResetContent();
        m_wndCodecApp.ShowWindow(SW_HIDE);
        m_wndBpsLabel.ShowWindow(SW_HIDE);
        m_wndBpsLabel1.ShowWindow(SW_HIDE);
        m_wndBitrate.ShowWindow(SW_HIDE);
        m_wndMaxBpsLabel.ShowWindow(SW_HIDE);
        m_wndBpsLabel1.ShowWindow(SW_HIDE);
        m_wndMaxBitrate.ShowWindow(SW_HIDE);
        m_wndBpsLabel2.ShowWindow(SW_HIDE);
        m_wndDtx.ShowWindow(SW_HIDE);
        m_wndVBR.ShowWindow(SW_HIDE);
        m_wndFrmSizeStatic.ShowWindow(SW_HIDE);
        m_wndOpusFrameSizes.ShowWindow(SW_HIDE);
        break;
    case SPEEX_VBR_CODEC :
        //sample rate
        m_wndSRLabel.ShowWindow(SW_SHOW);
        m_wndSampleRate.ShowWindow(SW_SHOW);
        m_wndSampleRate.ResetContent();
        AddString(m_wndSampleRate, _T("8 KHz"), SPEEX_MODEID_NB);
        AddString(m_wndSampleRate, _T("16 KHz"), SPEEX_MODEID_WB);
        AddString(m_wndSampleRate, _T("32 KHz"), SPEEX_MODEID_UWB);
        SetCurSelItemData(m_wndSampleRate, DEFAULT_SPEEX_VBR_BANDMODE);
        //channels
        m_wndACLabel.ShowWindow(SW_HIDE);
        m_wndAudioChannels.ShowWindow(SW_HIDE);
        AddString(m_wndAudioChannels, _T("Mono"), 1);
        AddString(m_wndAudioChannels, _T("Stereo"), 2);
        SetCurSelItemData(m_wndAudioChannels, DEFAULT_SPEEX_VBR_SIMSTEREO ? 2 : 1);
        //quality        
        m_wndQualityLabel.ShowWindow(SW_SHOW);
        m_wndQuality.ShowWindow(SW_SHOW);
        m_wndQuality.SetRange(0, 10, TRUE);
        m_wndQuality.SetPos(DEFAULT_SPEEX_VBR_QUALITY);
        //max bitrate
        m_wndMaxBpsLabel.ShowWindow(SW_SHOW);
        m_wndMaxBitrate.ShowWindow(SW_SHOW);
        SetWindowNumber(m_wndMaxBitrate, DEFAULT_SPEEX_VBR_MAXBITRATE);
        m_wndBpsLabel2.ShowWindow(SW_SHOW);
        //DTX
        m_wndDtx.ShowWindow(SW_SHOW);
        m_wndDtx.SetCheck(DEFAULT_SPEEX_VBR_DTX? BST_CHECKED : BST_UNCHECKED);
        //tx delay
        m_wndTxDelayLabel.ShowWindow(SW_SHOW);
        m_wndTxDelay.ShowWindow(SW_SHOW);
        m_wndTxDelaySpin.ShowWindow(SW_SHOW);
        m_wndTxDelayLabel1.ShowWindow(SW_SHOW);
        m_wndTxDelaySpin.SetRange(SPEEX_MIN_TXINTERVALMSEC,
                                  SPEEX_MAX_TXINTERVALMSEC);
        SetWindowNumber(m_wndTxDelay, DEFAULT_SPEEX_VBR_DELAY);

        m_wndAppLabel.ShowWindow(SW_HIDE);
        m_wndCodecApp.ShowWindow(SW_HIDE);
        m_wndBpsLabel.ShowWindow(SW_HIDE);
        m_wndBpsLabel1.ShowWindow(SW_HIDE);
        m_wndBitrate.ShowWindow(SW_HIDE);
        SetWindowNumber(m_wndBitrate, bpsDefault);
        m_wndBpsLabel1.ShowWindow(SW_HIDE);
        m_wndVBR.ShowWindow(SW_HIDE);
        m_wndFrmSizeStatic.ShowWindow(SW_HIDE);
        m_wndOpusFrameSizes.ShowWindow(SW_HIDE);
        break;
    case OPUS_CODEC :
        //application
        m_wndAppLabel.ShowWindow(SW_SHOW);
        m_wndCodecApp.ShowWindow(SW_SHOW);
        m_wndCodecApp.ResetContent();
        AddString(m_wndCodecApp, _T("VoIP"), OPUS_APPLICATION_VOIP);
        AddString(m_wndCodecApp, _T("Music"), OPUS_APPLICATION_AUDIO);
        SetCurSelItemData(m_wndCodecApp, DEFAULT_OPUS_APPLICATION);
        //samplerate
        m_wndSRLabel.ShowWindow(SW_SHOW);
        m_wndSampleRate.ShowWindow(SW_SHOW);
        m_wndSampleRate.ResetContent();
        AddString(m_wndSampleRate, _T("8 KHz"), 8000);
        AddString(m_wndSampleRate, _T("12 KHz"), 12000);
        AddString(m_wndSampleRate, _T("16 KHz"), 16000);
        AddString(m_wndSampleRate, _T("24 KHz"), 24000);
        AddString(m_wndSampleRate, _T("48 KHz"), 48000);
        SetCurSelItemData(m_wndSampleRate, DEFAULT_OPUS_SAMPLERATE);
        //channels
        m_wndACLabel.ShowWindow(SW_SHOW);
        m_wndAudioChannels.ShowWindow(SW_SHOW);
        m_wndAudioChannels.ResetContent();
        AddString(m_wndAudioChannels, _T("Mono"), 1);
        AddString(m_wndAudioChannels, _T("Stereo"), 2);
        SetCurSelItemData(m_wndAudioChannels, DEFAULT_OPUS_CHANNELS);
        //bitrate
        m_wndBpsLabel.ShowWindow(SW_SHOW);
        m_wndBpsLabel1.ShowWindow(SW_SHOW);
        m_wndBitrate.ShowWindow(SW_SHOW);
        SetWindowNumber(m_wndBitrate, bpsDefault);
        m_wndMaxBitrate.ShowWindow(SW_SHOW);
        m_wndVBR.ShowWindow(SW_SHOW);
        m_wndVBR.SetCheck(DEFAULT_OPUS_VBR? BST_CHECKED : BST_UNCHECKED);
        //DTX
        m_wndDtx.ShowWindow(SW_SHOW);
        //TX delay
        m_wndTxDelayLabel.ShowWindow(SW_SHOW);
        m_wndTxDelay.ShowWindow(SW_SHOW);
        m_wndTxDelaySpin.ShowWindow(SW_SHOW);
        m_wndTxDelayLabel1.ShowWindow(SW_SHOW);
        m_wndTxDelaySpin.SetRange(OPUS_MIN_TXINTERVALMSEC,
                                  OPUS_MAX_TXINTERVALMSEC);
        SetWindowNumber(m_wndTxDelay, DEFAULT_OPUS_DELAY);
        m_wndOpusFrameSizes.ShowWindow(SW_SHOW);
        m_wndFrmSizeStatic.ShowWindow(SW_SHOW);
        AddString(m_wndOpusFrameSizes, _T("0 msec"), 0);
        AddString(m_wndOpusFrameSizes, _T("2.5 msec"), OPUS_MIN_FRAMESIZE);
        AddString(m_wndOpusFrameSizes, _T("5 msec"), 5);
        AddString(m_wndOpusFrameSizes, _T("10 msec"), 10);
        AddString(m_wndOpusFrameSizes, _T("20 msec"), 20);
        AddString(m_wndOpusFrameSizes, _T("40 msec"), 40);
        AddString(m_wndOpusFrameSizes, _T("60 msec"), OPUS_MAX_FRAMESIZE);
        AddString(m_wndOpusFrameSizes, _T("80 msec"), 80);
        AddString(m_wndOpusFrameSizes, _T("100 msec"), 100);
        AddString(m_wndOpusFrameSizes, _T("120 msec"), OPUS_REALMAX_FRAMESIZE);
        m_wndOpusFrameSizes.SetCurSel(0);
        SetCurSelItemData(m_wndOpusFrameSizes, DEFAULT_OPUS_FRAMESIZE);

        m_wndQualityLabel.ShowWindow(SW_HIDE);
        m_wndQuality.ShowWindow(SW_HIDE);
        m_wndMaxBpsLabel.ShowWindow(SW_HIDE);
        m_wndMaxBitrate.ShowWindow(SW_HIDE);
        m_wndBpsLabel2.ShowWindow(SW_HIDE);
        break;
    case NO_CODEC :
    default:
        m_wndAppLabel.ShowWindow(SW_HIDE);
        m_wndCodecApp.ShowWindow(SW_HIDE);
        m_wndSRLabel.ShowWindow(SW_HIDE);
        m_wndSampleRate.ShowWindow(SW_HIDE);
        m_wndACLabel.ShowWindow(SW_HIDE);
        m_wndAudioChannels.ShowWindow(SW_HIDE);
        m_wndQualityLabel.ShowWindow(SW_HIDE);
        m_wndQuality.ShowWindow(SW_HIDE);
        m_wndBpsLabel.ShowWindow(SW_HIDE);
        m_wndBitrate.ShowWindow(SW_HIDE);
        m_wndBpsLabel1.ShowWindow(SW_HIDE);
        m_wndMaxBpsLabel.ShowWindow(SW_HIDE);
        m_wndMaxBitrate.ShowWindow(SW_HIDE);
        m_wndBpsLabel2.ShowWindow(SW_HIDE);
        m_wndVBR.ShowWindow(SW_HIDE);
        m_wndDtx.ShowWindow(SW_HIDE);
        m_wndTxDelayLabel.ShowWindow(SW_HIDE);
        m_wndTxDelay.ShowWindow(SW_HIDE);
        m_wndTxDelaySpin.ShowWindow(SW_HIDE);
        m_wndTxDelayLabel1.ShowWindow(SW_HIDE);
        m_wndFrmSizeStatic.ShowWindow(SW_HIDE);
        m_wndOpusFrameSizes.ShowWindow(SW_HIDE);
        break;
    }

    if(nCodec == m_codec.nCodec)
        ShowCurrentCodec();
}

void CChannelDlg::UpdateCodec()
{
    int bitrate = 0, maxbitrate = 0;
    CString s;
    m_wndBitrate.GetWindowText(s);
    bitrate = _ttoi(s);
    m_wndMaxBitrate.GetWindowText(s);
    maxbitrate = _ttoi(s);

    m_codec.nCodec = GetCurrentCodec();
    switch(m_codec.nCodec)
    {
    case SPEEX_CODEC :
        m_codec.speex.nBandmode = GetItemData(m_wndSampleRate);
        m_codec.speex.nQuality = m_wndQuality.GetPos();
        m_codec.speex.bStereoPlayback = DEFAULT_SPEEX_SIMSTEREO;
        m_codec.speex.nTxIntervalMSec = GetWindowNumber(m_wndTxDelay);
        break;
    case SPEEX_VBR_CODEC :
        m_codec.speex_vbr.nBandmode = GetItemData(m_wndSampleRate);
        m_codec.speex_vbr.nQuality = m_wndQuality.GetPos();
        m_codec.speex_vbr.nBitRate = bitrate;
        m_codec.speex_vbr.nMaxBitRate = maxbitrate;
        m_codec.speex_vbr.bDTX = m_wndDtx.GetCheck() == BST_CHECKED;
        m_codec.speex_vbr.bStereoPlayback = DEFAULT_SPEEX_SIMSTEREO;
        m_codec.speex_vbr.nTxIntervalMSec = GetWindowNumber(m_wndTxDelay);
        break;
    case OPUS_CODEC :
        m_codec.opus.nSampleRate = GetItemData(m_wndSampleRate);
        m_codec.opus.nChannels = GetItemData(m_wndAudioChannels);
        m_codec.opus.nApplication = GetItemData(m_wndCodecApp);
        m_codec.opus.nComplexity = DEFAULT_OPUS_COMPLEXITY;
        m_codec.opus.bFEC = DEFAULT_OPUS_FEC;
        m_codec.opus.bDTX = m_wndDtx.GetCheck() == BST_CHECKED;
        m_codec.opus.nBitRate = bitrate;
        m_codec.opus.bVBR = m_wndVBR.GetCheck() == BST_CHECKED;
        m_codec.opus.bVBRConstraint = DEFAULT_OPUS_VBRCONSTRAINT;
        m_codec.opus.nTxIntervalMSec = GetWindowNumber(m_wndTxDelay);
        m_codec.opus.nFrameSizeMSec = GetItemData(m_wndOpusFrameSizes);
        break;
    }
}

Codec CChannelDlg::GetCurrentCodec()
{
    return (Codec)m_wndCodec.GetItemData(m_wndCodec.GetCurSel());
}

void CChannelDlg::ShowCurrentCodec()
{
    switch(m_codec.nCodec)
    {
    case SPEEX_CODEC :
        SetCurSelItemData(m_wndSampleRate, m_codec.speex.nBandmode);
        m_wndQuality.SetPos(m_codec.speex.nQuality);
        m_wndAudioChannels.SetCurSel(0);
        SetWindowNumber(m_wndTxDelay, m_codec.speex.nTxIntervalMSec);
        break;
    case SPEEX_VBR_CODEC :
        SetCurSelItemData(m_wndSampleRate, m_codec.speex_vbr.nBandmode);
        SetWindowNumber(m_wndBitrate, m_codec.speex_vbr.nBitRate);
        SetWindowNumber(m_wndMaxBitrate, m_codec.speex_vbr.nMaxBitRate);
        m_wndAudioChannels.SetCurSel(0);
        m_wndQuality.SetPos(m_codec.speex_vbr.nQuality);
        m_wndDtx.SetCheck(m_codec.speex_vbr.bDTX);
        SetWindowNumber(m_wndTxDelay, m_codec.speex_vbr.nTxIntervalMSec);
        break;
    case OPUS_CODEC :
        SetCurSelItemData(m_wndCodecApp, m_codec.opus.nApplication);
        SetCurSelItemData(m_wndSampleRate, m_codec.opus.nSampleRate);
        SetCurSelItemData(m_wndAudioChannels, m_codec.opus.nChannels);
        SetWindowNumber(m_wndBitrate, m_codec.opus.nBitRate);
        m_wndVBR.SetCheck(m_codec.opus.bVBR? BST_CHECKED : BST_UNCHECKED);
        m_wndDtx.SetCheck(m_codec.opus.bDTX? BST_CHECKED : BST_UNCHECKED);
        SetWindowNumber(m_wndTxDelay, m_codec.opus.nTxIntervalMSec);
        SetCurSelItemData(m_wndOpusFrameSizes, m_codec.opus.nFrameSizeMSec);
        break;
    }
}

void CChannelDlg::GetBitrateLimits(Codec nCodec, int& nMinBps, 
                                   int& nMaxBps, int& nDefaultBps)
{
    switch(nCodec)
    {
    case SPEEX_VBR_CODEC :
        switch(GetItemData(m_wndSampleRate))
        {
        case SPEEX_MODEID_NB :
            nMinBps = SPEEX_NB_MIN_BITRATE;
            nMaxBps = SPEEX_NB_MAX_BITRATE;
            nDefaultBps = DEFAULT_SPEEX_VBR_BITRATE;
            break;
        case SPEEX_MODEID_WB :
            nMinBps = SPEEX_WB_MIN_BITRATE;
            nMaxBps = SPEEX_WB_MAX_BITRATE;
            nDefaultBps = DEFAULT_SPEEX_VBR_BITRATE;
            break;
        case SPEEX_MODEID_UWB :
            nMinBps = SPEEX_UWB_MIN_BITRATE;
            nMaxBps = SPEEX_UWB_MAX_BITRATE;
            nDefaultBps = DEFAULT_SPEEX_VBR_BITRATE;
            break;
        }
    case OPUS_CODEC :
        nMinBps = OPUS_MIN_BITRATE;
        nMaxBps = OPUS_MAX_BITRATE;
        nDefaultBps = DEFAULT_OPUS_BITRATE;
        break;
    }
}

void CChannelDlg::OnCbnSelchangeComboCodec()
{
    DisplayCodecControls(GetCurrentCodec());
}

void CChannelDlg::OnEnKillfocusEditBitrate()
{
    int bps_low = 0, bps_high = 0, bps_default = 0;
    GetBitrateLimits(GetCurrentCodec(), bps_low, 
        bps_high, bps_default);

    CString s1;
    m_wndBitrate.GetWindowText(s1);
    int bps = _ttoi(s1);
    if(bps == 0)
        SetWindowNumber(m_wndBitrate, 0);
    else if(bps<bps_low)
        SetWindowNumber(m_wndBitrate, bps_low);
    if(bps>bps_high)
        SetWindowNumber(m_wndBitrate, bps_high);
}

void CChannelDlg::OnEnKillfocusEditMaxbitrate()
{
    int bps_low, bps_high, bps_default;
    GetBitrateLimits(GetCurrentCodec(), bps_low, 
        bps_high, bps_default);

    CString s1;
    m_wndMaxBitrate.GetWindowText(s1);
    int maxbps = _ttoi(s1);

    if(maxbps<bps_low)
        SetWindowNumber(m_wndMaxBitrate, 0);
    if(maxbps>bps_high)
        SetWindowNumber(m_wndMaxBitrate, bps_high);
}

void CChannelDlg::OnDeltaposSpinDelay(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    *pResult = 0;
//    int n = GetWindowNumber(m_wndTxDelay);
//    if(pNMUpDown->iDelta>0)
//        n += 20;
//    else
//        n -= 20;
//    SetWindowNumber(m_wndTxDelay, n);
//    DelayBounds();
}

void CChannelDlg::OnEnKillfocusEditTxdelay()
{
    DelayBounds();
}
    
void CChannelDlg::DelayBounds()
{
    //int n = GetWindowNumber(m_wndTxDelay);
    //n = min(DEFAULT_MAX_MSEC_PER_PACKET, n);
    //n = max(DEFAULT_MIN_MSEC_PER_PACKET, n);
    //n = (n / 20) * 20;
    //SetWindowNumber(m_wndTxDelay, n);
}
