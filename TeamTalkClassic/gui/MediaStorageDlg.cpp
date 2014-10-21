// AudioStorageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../Resource.h"
#include "MediaStorageDlg.h"
#include <Helper.h>
#include <TeamTalkDlg.h>

// CMediaStorageDlg dialog

IMPLEMENT_DYNAMIC(CMediaStorageDlg, CDialog)

CMediaStorageDlg::CMediaStorageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMediaStorageDlg::IDD, pParent)
    , m_szAudioDir(_T(""))
    , m_bSingleFile(FALSE)
    , m_bSeparateFiles(FALSE)
    , m_uAFF(AFF_NONE)
    , m_szChanLogDir(_T(""))
    , m_szUserTxtDir(_T(""))
{

}

CMediaStorageDlg::~CMediaStorageDlg()
{
}

void CMediaStorageDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_AFF, m_wndAFF);
    DDX_Control(pDX, IDC_EDIT_AUDIOFOLDER, m_wndAudioDir);
    DDX_Text(pDX, IDC_EDIT_AUDIOFOLDER, m_szAudioDir);
    DDX_Check(pDX, IDC_CHECK_SINGLEFILE, m_bSingleFile);
    DDX_Check(pDX, IDC_CHECK_SEPARATEFILES, m_bSeparateFiles);
    DDX_Control(pDX, IDC_EDIT_CHANLOGFOLDER, m_wndChanLogDir);
    DDX_Text(pDX, IDC_EDIT_CHANLOGFOLDER, m_szChanLogDir);
    DDX_Control(pDX, IDC_EDIT_TEXTLOGFOLDER, m_wndUserTxtDir);
    DDX_Text(pDX, IDC_EDIT_TEXTLOGFOLDER, m_szUserTxtDir);
}


BEGIN_MESSAGE_MAP(CMediaStorageDlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_AUDIOBROWSE, &CMediaStorageDlg::OnBnClickedButtonBrowse)
    ON_BN_CLICKED(IDOK, &CMediaStorageDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BUTTON_CHANLOGBROWSE, &CMediaStorageDlg::OnBnClickedButtonChanlogbrowse)
    ON_BN_CLICKED(IDC_BUTTON_USER2USERBROWSE, &CMediaStorageDlg::OnBnClickedButtonUser2userbrowse)
END_MESSAGE_MAP()


// CMediaStorageDlg message handlers

BOOL CMediaStorageDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    if(FileExists(MP3ENCDLL_FILENAME))
    {
        int i;
        i = m_wndAFF.AddString(_T("MP3-format (16 kbit)"));
        m_wndAFF.SetItemData(i, AFF_MP3_16KBIT_FORMAT);
        i = m_wndAFF.AddString(_T("MP3-format (32 kbit)"));
        m_wndAFF.SetItemData(i, AFF_MP3_32KBIT_FORMAT);
        i = m_wndAFF.AddString(_T("MP3-format (64 kbit)"));
        m_wndAFF.SetItemData(i, AFF_MP3_64KBIT_FORMAT);
        i = m_wndAFF.AddString(_T("MP3-format (128 kbit)"));
        m_wndAFF.SetItemData(i, AFF_MP3_128KBIT_FORMAT);
        i = m_wndAFF.AddString(_T("MP3-format (256 kbit)"));
        m_wndAFF.SetItemData(i, AFF_MP3_256KBIT_FORMAT);
    }
    int i = m_wndAFF.AddString(_T("Wave-format"));
    m_wndAFF.SetItemData(i, AFF_WAVE_FORMAT);
    m_wndAFF.SetCurSel(0);


    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CMediaStorageDlg::OnBnClickedButtonBrowse()
{
    TCHAR szSaveDir[MAX_PATH];
    if(BrowseForFolder(m_hWnd, m_szAudioDir, szSaveDir))
        m_wndAudioDir.SetWindowText(szSaveDir);
}

void CMediaStorageDlg::OnBnClickedOk()
{
    m_uAFF = m_wndAFF.GetItemData(m_wndAFF.GetCurSel());
    OnOK();
}

void CMediaStorageDlg::OnBnClickedButtonChanlogbrowse()
{
    TCHAR szSaveDir[MAX_PATH];
    if(BrowseForFolder(m_hWnd, m_szChanLogDir, szSaveDir))
        m_wndChanLogDir.SetWindowText(szSaveDir);
}


void CMediaStorageDlg::OnBnClickedButtonUser2userbrowse()
{
    TCHAR szSaveDir[MAX_PATH];
    if(BrowseForFolder(m_hWnd, m_szUserTxtDir, szSaveDir))
        m_wndUserTxtDir.SetWindowText(szSaveDir);
}
