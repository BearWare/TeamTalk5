// GenerateTTFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "GenerateTTFileDlg.h"
#include "TeamTalkDlg.h"
#include "KeyCompDlg.h"
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
}


BEGIN_MESSAGE_MAP(CGenerateTTFileDlg, CDialog)
    ON_BN_CLICKED(IDC_CHECK_CLIENTOVERRIDE, &CGenerateTTFileDlg::OnBnClickedCheckClientoverride)
    ON_BN_CLICKED(IDC_CHECK_PUSHTOTALK, &CGenerateTTFileDlg::OnBnClickedCheckPushtotalk)
    ON_BN_CLICKED(IDC_BUTTON_SETUPKEYS, &CGenerateTTFileDlg::OnBnClickedButtonSetupkeys)
    ON_BN_CLICKED(IDC_BUTTON_SAVETTFILE, &CGenerateTTFileDlg::OnBnClickedButtonSavettfile)
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

    return TRUE;
}

void CGenerateTTFileDlg::OnBnClickedCheckClientoverride()
{
    BOOL bEnable = m_wndOverrideClient.GetCheck() == BST_CHECKED;
    m_wndNickname.EnableWindow(bEnable);
    m_wndPttChkBox.EnableWindow(bEnable);
    m_wndMale.EnableWindow(bEnable);
    m_wndFemale.EnableWindow(bEnable);
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
        CString szNickname;
        m_wndNickname.GetWindowText(szNickname);
        m_hostentry.szNickname = STR_UTF8(szNickname);
        if(m_wndPttChkBox.GetCheck() == BST_CHECKED && m_Hotkey.size())
            m_hostentry.hotkey = m_Hotkey;
        else
            m_hostentry.hotkey.clear();
        m_hostentry.nGender = GENDER_NONE;
        if(m_wndMale.GetCheck() == BST_CHECKED)
            m_hostentry.nGender = GENDER_MALE;
        if(m_wndFemale.GetCheck() == BST_CHECKED)
            m_hostentry.nGender = GENDER_FEMALE;
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

    TCHAR szFilters[] = _T(".tt Files (*.tt)|*.tt||");
    CFileDialog fileDlg(FALSE, NULL, _T(""), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters, this);
    if(fileDlg.DoModal() == IDOK)
    {
        CString szFileName = fileDlg.GetPathName();
        if(szFileName.Right(3) != _T(".tt"))
            szFileName += _T(".tt");

        teamtalk::TTFile ttfile(TT_XML_ROOTNAME);
        if(!ttfile.CreateFile(STR_LOCAL(szFileName)))
            MessageBox(_T("Failed to save .tt file."), _T("Save .tt File"));
        else
        {
            ttfile.SetHostEntry(m_hostentry);
            ttfile.SaveFile();
        }
    }
    SetCurrentDirectory(szWorkDir);
}
