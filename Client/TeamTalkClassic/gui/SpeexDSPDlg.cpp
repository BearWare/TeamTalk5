// SpeexDSPDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpeexDSPDlg.h"
#include "afxdialogex.h"


// CSpeexDSPDlg dialog

IMPLEMENT_DYNAMIC(CSpeexDSPDlg, CDialogEx)

CSpeexDSPDlg::CSpeexDSPDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_SPEEXDSP, pParent)
    , m_bAGC(DEFAULT_AGC_ENABLE)
    , m_nGainLevel(DEFAULT_AGC_GAINLEVEL)
    , m_nMaxGainLevel(DEFAULT_AGC_GAINMAXDB)
    , m_nGainInc(DEFAULT_AGC_INC_MAXDB)
    , m_nGainDec(DEFAULT_AGC_DEC_MAXDB)
    , m_nDenoiseLevel(DEFAULT_DENOISE_SUPPRESS)
    , m_bDenoise(DEFAULT_DENOISE_ENABLE)
{

}

CSpeexDSPDlg::~CSpeexDSPDlg()
{
}

void CSpeexDSPDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_AGC, m_bAGC);
    DDX_Text(pDX, IDC_EDIT_GAINLEVEL, m_nGainLevel);
    DDX_Text(pDX, IDC_EDIT_MAXGAIN, m_nMaxGainLevel);
    DDX_Text(pDX, IDC_EDIT_GAININC, m_nGainInc);
    DDX_Text(pDX, IDC_EDIT_GAINDEC, m_nGainDec);
    DDX_Text(pDX, IDC_EDIT_DENOISE, m_nDenoiseLevel);

    DDX_Control(pDX, IDC_EDIT_GAINLEVEL, m_wndGainLevel);
    DDX_Control(pDX, IDC_EDIT_MAXGAIN, m_wndGainMax);
    DDX_Control(pDX, IDC_EDIT_GAININC, m_wndGainInc);
    DDX_Control(pDX, IDC_EDIT_GAINDEC, m_wndGainDec);
    DDX_Control(pDX, IDC_EDIT_DENOISE, m_wndDenoise);

    DDX_Check(pDX, IDC_CHECK_DENOISE, m_bDenoise);
    DDX_Control(pDX, IDC_SPIN_GAINLEVEL, m_wndGainLevelSpin);
    DDX_Control(pDX, IDC_SPIN_MAXGAIN, m_wndGainMaxSpin);
    DDX_Control(pDX, IDC_SPIN_GAININC, m_wndGainIncSpin);
    DDX_Control(pDX, IDC_SPIN_GAINDEC, m_wndGainDecSpin);
    DDX_Control(pDX, IDC_SPIN_DENOISE, m_wndDenoiseSpin);
    DDX_Control(pDX, IDC_CHECK_AGC, m_wndAGC);
    DDX_Control(pDX, IDC_CHECK_DENOISE, m_wndDenoiseEnable);
}

BEGIN_MESSAGE_MAP(CSpeexDSPDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_DEFAULT, &CSpeexDSPDlg::OnBnClickedButtonDefault)
    ON_BN_CLICKED(IDC_BUTTON_DEFAULT2, &CSpeexDSPDlg::OnBnClickedButtonDefault2)
END_MESSAGE_MAP()

// CSpeexDSPDlg message handlers

BOOL CSpeexDSPDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    TRANSLATE(*this, IDD);

    m_wndGainLevelSpin.SetRange(0, 0x7fff);
    m_wndGainMaxSpin.SetRange(0, 100);
    m_wndGainIncSpin.SetRange(0, 100);
    m_wndGainDecSpin.SetRange(-100, 0);
    m_wndDenoiseSpin.SetRange(-100, 0);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CSpeexDSPDlg::OnBnClickedButtonDefault()
{
    m_wndAGC.SetCheck(DEFAULT_AGC_ENABLE ? BST_CHECKED : BST_UNCHECKED);
    SetWindowNumber(m_wndGainLevel, DEFAULT_AGC_GAINLEVEL);
    SetWindowNumber(m_wndGainMax, DEFAULT_AGC_GAINMAXDB);
    SetWindowNumber(m_wndGainInc, DEFAULT_AGC_INC_MAXDB);
    SetWindowNumber(m_wndGainDec, DEFAULT_AGC_DEC_MAXDB);
}

void CSpeexDSPDlg::OnBnClickedButtonDefault2()
{
    m_wndDenoiseEnable.SetCheck(DEFAULT_DENOISE_ENABLE ? BST_CHECKED : BST_UNCHECKED);
    SetWindowNumber(m_wndDenoise, DEFAULT_DENOISE_SUPPRESS);
}
