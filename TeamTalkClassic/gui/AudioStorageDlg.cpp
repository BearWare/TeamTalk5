// AudioStorageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../Resource.h"
#include "AudioStorageDlg.h"
#include <Helper.h>
#include <TeamTalkDlg.h>

// CAudioStorageDlg dialog

IMPLEMENT_DYNAMIC(CAudioStorageDlg, CDialog)

CAudioStorageDlg::CAudioStorageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAudioStorageDlg::IDD, pParent)
    , m_szStorageDir(_T(""))
    , m_bSingleFile(FALSE)
    , m_bSeparateFiles(FALSE)
    , m_uAFF(AFF_NONE)
{

}

CAudioStorageDlg::~CAudioStorageDlg()
{
}

void CAudioStorageDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_AFF, m_wndAFF);
    DDX_Control(pDX, IDC_EDIT_STORAGEDIR, m_wndStorageDir);
    DDX_Text(pDX, IDC_EDIT_STORAGEDIR, m_szStorageDir);
    DDX_Check(pDX, IDC_CHECK_SINGLEFILE, m_bSingleFile);
    DDX_Check(pDX, IDC_CHECK_SEPARATEFILES, m_bSeparateFiles);
}


BEGIN_MESSAGE_MAP(CAudioStorageDlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CAudioStorageDlg::OnBnClickedButtonBrowse)
    ON_BN_CLICKED(IDOK, &CAudioStorageDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAudioStorageDlg message handlers

BOOL CAudioStorageDlg::OnInitDialog()
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

void CAudioStorageDlg::OnBnClickedButtonBrowse()
{
    TCHAR szSaveDir[MAX_PATH];
    if(BrowseForFolder(m_hWnd, m_szStorageDir, szSaveDir))
        m_wndStorageDir.SetWindowText(szSaveDir);
}

void CAudioStorageDlg::OnBnClickedOk()
{
    m_uAFF = m_wndAFF.GetItemData(m_wndAFF.GetCurSel());
    OnOK();
}
