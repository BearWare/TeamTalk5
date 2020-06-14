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

// GenerateTTFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "GenerateTTFileDlg.h"
#include "KeyCompDlg.h"
#include "AppInfo.h"

#include <settings/ClientXML.h>

// CGenerateTTFileDlg dialog

IMPLEMENT_DYNAMIC(CGenerateTTFileDlg, CDialog)

CGenerateTTFileDlg::CGenerateTTFileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGenerateTTFileDlg::IDD, pParent)
{

}

CGenerateTTFileDlg::~CGenerateTTFileDlg()
{
}

void CGenerateTTFileDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_USERNAME, m_wndUsername);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_wndPassword);
    DDX_Control(pDX, IDC_EDIT_NICKNAME, m_wndNickname);
    DDX_Control(pDX, IDC_CHECK_PUSHTOTALK, m_wndPttChkBox);
    DDX_Control(pDX, IDC_BUTTON_SETUPKEYS, m_wndSetupKeys);
    DDX_Control(pDX, IDC_EDIT_KEYCOMB, m_wndKeyComb);
    DDX_Control(pDX, IDC_RADIO_MALE, m_wndMale);
    DDX_Control(pDX, IDC_RADIO_FEMALE, m_wndFemale);
    DDX_Control(pDX, IDC_CHECK_CLIENTOVERRIDE, m_wndOverrideClient);
    DDX_Text(pDX, IDC_EDIT_USERNAME, STR_UTF8(m_hostentry.szUsername));
    DDX_Text(pDX, IDC_EDIT_PASSWORD, STR_UTF8(m_hostentry.szPassword));
    DDX_Control(pDX, IDC_COMBO_VIDCODEC, m_wndVidCodec);
    DDX_Control(pDX, IDC_EDIT_VIDBITRATE, m_wndVidBitrate);
    DDX_Control(pDX, IDC_SPIN_VIDBITRATE, m_wndVidBitrateSpinCtrl);
    DDX_Control(pDX, IDC_CHECK_VOICEACT, m_wndVox);
    DDX_Control(pDX, IDC_COMBO_VIDRES, m_wndCapfmt);
}


BEGIN_MESSAGE_MAP(CGenerateTTFileDlg, CDialog)
    ON_BN_CLICKED(IDC_CHECK_CLIENTOVERRIDE, &CGenerateTTFileDlg::OnBnClickedCheckClientoverride)
    ON_BN_CLICKED(IDC_CHECK_PUSHTOTALK, &CGenerateTTFileDlg::OnBnClickedCheckPushtotalk)
    ON_BN_CLICKED(IDC_BUTTON_SETUPKEYS, &CGenerateTTFileDlg::OnBnClickedButtonSetupkeys)
    ON_BN_CLICKED(IDC_BUTTON_SAVETTFILE, &CGenerateTTFileDlg::OnBnClickedButtonSavettfile)
    ON_CBN_SELCHANGE(IDC_COMBO_VIDCODEC, &CGenerateTTFileDlg::OnCbnSelchangeComboVidcodec)
END_MESSAGE_MAP()


// CGenerateTTFileDlg message handlers

BOOL CGenerateTTFileDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    CString szTitle, szTmp;
    GetWindowText(szTitle);
    szTmp.Format(_T("%s - %s:%d"), szTitle, STR_UTF8(m_hostentry.szAddress),
                 m_hostentry.nTcpPort);
    SetWindowText(szTmp);

    LoadVideoFormats();

    m_wndCapfmt.AddString(LoadText(IDS_GENERATETTFILEANY, _T("Any")));

    for(size_t i=1;i<m_vidcap_fmts.size();i++)
    {
        CString s;
        s.Format(_T("%dx%d, FPS: %d"), m_vidcap_fmts[i].nWidth, m_vidcap_fmts[i].nHeight,
            m_vidcap_fmts[i].nFPS_Numerator / m_vidcap_fmts[i].nFPS_Denominator);
        m_wndCapfmt.AddString(s);
    }
    m_wndCapfmt.SetCurSel(0);

    m_wndVidCodec.SetItemData(m_wndVidCodec.AddString(LoadText(IDS_GENERATETTFILEANY, _T("Any"))), NO_CODEC);
    m_wndVidCodec.SetItemData(m_wndVidCodec.AddString(_T("WebM VP8")), WEBM_VP8_CODEC);
    m_wndVidCodec.SetCurSel(0);
    m_wndVidBitrateSpinCtrl.SetRange(0, 1000);

    return TRUE;
}

void CGenerateTTFileDlg::LoadVideoFormats()
{
    VideoFormat fmt = {};
    m_vidcap_fmts.push_back(fmt);

    fmt.picFourCC = FOURCC_RGB32;
    fmt.nFPS_Denominator = 1;

    fmt.nWidth = 160;
    fmt.nHeight = 120;

    fmt.nFPS_Numerator = 1;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 10;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 15;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 30;
    m_vidcap_fmts.push_back(fmt);
    
    fmt.nWidth = 320;
    fmt.nHeight = 240;

    fmt.nFPS_Numerator = 1;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 10;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 15;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 30;
    m_vidcap_fmts.push_back(fmt);

    fmt.nWidth = 640;
    fmt.nHeight = 480;

    fmt.nFPS_Numerator = 1;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 10;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 15;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 30;
    m_vidcap_fmts.push_back(fmt);
}


void CGenerateTTFileDlg::OnBnClickedCheckClientoverride()
{
    BOOL bEnable = m_wndOverrideClient.GetCheck() == BST_CHECKED;
    m_wndNickname.EnableWindow(bEnable);
    m_wndMale.EnableWindow(bEnable);
    m_wndFemale.EnableWindow(bEnable);
    m_wndPttChkBox.EnableWindow(bEnable);
    m_wndVox.EnableWindow(bEnable);
    m_wndCapfmt.EnableWindow(bEnable);
    m_wndVidCodec.EnableWindow(bEnable);
    m_wndVidBitrateSpinCtrl.EnableWindow(bEnable);
}

void CGenerateTTFileDlg::OnBnClickedCheckPushtotalk()
{
    BOOL bEnable = m_wndPttChkBox.GetCheck() == BST_CHECKED;
    m_wndSetupKeys.EnableWindow(bEnable);
    m_wndKeyComb.EnableWindow(bEnable);
    if(!bEnable)
    {
        m_Hotkey.clear();
        m_wndKeyComb.SetWindowText(_T(""));
    }
}


void CGenerateTTFileDlg::OnBnClickedButtonSetupkeys()
{
    CKeyCompDlg dlg;
    dlg.DoModal();
    m_Hotkey = dlg.m_Hotkey;

    m_wndKeyComb.SetWindowText(CKeyCompDlg::GetHotkeyString(m_Hotkey));
}


void CGenerateTTFileDlg::OnBnClickedButtonSavettfile()
{
    CString szUsername, szPassword;
    m_wndUsername.GetWindowText(szUsername);
    m_wndPassword.GetWindowText(szPassword);
    if(m_wndOverrideClient.GetCheck() == BST_CHECKED)
    {
        //user settings
        CString szNickname;
        m_wndNickname.GetWindowText(szNickname);
        m_hostentry.szNickname = STR_UTF8(szNickname);
        m_hostentry.nGender = GENDER_NONE;
        if(m_wndMale.GetCheck() == BST_CHECKED)
            m_hostentry.nGender = GENDER_MALE;
        if(m_wndFemale.GetCheck() == BST_CHECKED)
            m_hostentry.nGender = GENDER_FEMALE;
        //PTT
        if(m_wndPttChkBox.GetCheck() == BST_CHECKED && m_Hotkey.size())
            m_hostentry.hotkey = m_Hotkey;
        else
            m_hostentry.hotkey.clear();
        m_hostentry.nVoiceAct = m_wndVox.GetCheck() == BST_CHECKED;

        if(m_wndCapfmt.GetCurSel()>0)
            m_hostentry.capformat = m_vidcap_fmts[m_wndCapfmt.GetCurSel()];
        
        //video codec
        if(m_wndVidCodec.GetItemData(m_wndVidCodec.GetCurSel()) == WEBM_VP8_CODEC)
        {
            m_hostentry.vidcodec.nCodec = WEBM_VP8_CODEC;
            m_hostentry.vidcodec.webm_vp8.nRcTargetBitrate = GetWindowNumber(m_wndVidBitrate);
            m_hostentry.vidcodec.webm_vp8.nEncodeDeadline = DEFAULT_WEBMVP8_DEADLINE;
        }
    }
    else
    {
        m_hostentry.nGender = GENDER_NONE;
        m_hostentry.szNickname.clear();
        m_hostentry.hotkey.clear();
    }
    m_hostentry.szUsername = STR_UTF8(szUsername);
    m_hostentry.szPassword = STR_UTF8(szPassword);

    CString szWorkDir;
    GetCurrentDirectory(MAX_PATH, szWorkDir.GetBufferSetLength(MAX_PATH));

    CString szFilters = LoadText(IDS_GENERATETTFILEEXTFILE, _T(".tt Files (*.tt)|*.tt||"));
    CFileDialog fileDlg(FALSE, NULL, _T(""), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters, this);
    if(fileDlg.DoModal() == IDOK)
    {
        CString szFileName = fileDlg.GetPathName();
        if(szFileName.Right(3) != _T(".tt"))
            szFileName += _T(".tt");

        teamtalk::TTFile ttfile(TT_XML_ROOTNAME);
        if(!ttfile.CreateFile(STR_LOCAL(szFileName)))
            MessageBox(LoadText(IDS_GENERATETTFILEFAILEDTOSAVE, _T("Failed to save .tt file.")), LoadText(IDS_GENERATETTFILESAVE, _T("Save .tt File")));
        else
        {
            ttfile.SetHostEntry(m_hostentry);
            ttfile.SaveFile();
        }
    }
    SetCurrentDirectory(szWorkDir);
}


void CGenerateTTFileDlg::OnCbnSelchangeComboVidcodec()
{
    DWORD_PTR i = m_wndVidCodec.GetItemData(m_wndVidCodec.GetCurSel());
    m_wndVidBitrate.EnableWindow(i == WEBM_VP8_CODEC);
    m_wndVidBitrateSpinCtrl.EnableWindow(i == WEBM_VP8_CODEC);
}
