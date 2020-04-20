// TTAudioPreprocessorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TTAudioPreprocessorDlg.h"
#include "afxdialogex.h"


// CTTAudioPreprocessorDlg dialog

IMPLEMENT_DYNAMIC(CTTAudioPreprocessorDlg, CDialogEx)

CTTAudioPreprocessorDlg::CTTAudioPreprocessorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_TTAUDIOPREPROCESSOR, pParent)
    , m_nGainLevel(SOUND_GAIN_DEFAULT)
    , m_bMuteLeft(FALSE)
    , m_bMuteRight(FALSE)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

CTTAudioPreprocessorDlg::~CTTAudioPreprocessorDlg()
{
}

void CTTAudioPreprocessorDlg::InitDefault()
{
    m_wndGainlevel.SetPos(SOUND_GAIN_DEFAULT);
    m_wndMuteLeft.SetCheck(BST_UNCHECKED);
    m_wndMuteRight.SetCheck(BST_UNCHECKED);
}

void CTTAudioPreprocessorDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Slider(pDX, IDC_SLIDER_GAINLEVEL, m_nGainLevel);
    DDX_Check(pDX, IDC_CHECK_MUTELEFT, m_bMuteLeft);
    DDX_Check(pDX, IDC_CHECK_MUTERIGHT, m_bMuteRight);
    DDX_Control(pDX, IDC_SLIDER_GAINLEVEL, m_wndGainlevel);
    DDX_Control(pDX, IDC_CHECK_MUTELEFT, m_wndMuteLeft);
    DDX_Control(pDX, IDC_CHECK_MUTERIGHT, m_wndMuteRight);
}


BEGIN_MESSAGE_MAP(CTTAudioPreprocessorDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_DEFAULT, &CTTAudioPreprocessorDlg::OnBnClickedButtonDefault)
END_MESSAGE_MAP()


// CTTAudioPreprocessorDlg message handlers

BOOL CTTAudioPreprocessorDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    TRANSLATE(*this, IDD);

    m_wndGainlevel.SetRange(SOUND_GAIN_MIN, SOUND_GAIN_MAX);
    m_wndGainlevel.SetPos(m_nGainLevel);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CTTAudioPreprocessorDlg::OnBnClickedButtonDefault()
{
    InitDefault();
}
