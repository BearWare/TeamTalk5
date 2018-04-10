// SendLiveContentDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../Resource.h"
#include "SendLiveContentDlg.h"
#include "afxdialogex.h"

#include <AppInfo.h>
#include <regex>


// CSendLiveContentDlg dialog

IMPLEMENT_DYNAMIC(CSendLiveContentDlg, CDialog)

CSendLiveContentDlg::CSendLiveContentDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_SENDLIVECONTENT, pParent)
    , m_szUrl(_T(""))
    , m_szFileName(_T(""))
    , m_bURL(FALSE)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif
}

CSendLiveContentDlg::~CSendLiveContentDlg()
{
}

void CSendLiveContentDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_FILES, m_wndFiles);
    DDX_Control(pDX, IDC_EDIT_URL, m_wndUrl);
    DDX_Control(pDX, IDC_TREE_USERS, m_wndUsers);
    DDX_Control(pDX, IDC_RADIO_URL, m_wndUrlRadio);
    DDX_Text(pDX, IDC_EDIT_URL, m_szUrl);
    DDX_CBString(pDX, IDC_COMBO_FILES, m_szFileName);
}


BEGIN_MESSAGE_MAP(CSendLiveContentDlg, CDialog)
    ON_BN_CLICKED(IDC_RADIO_URL, &CSendLiveContentDlg::OnBnClickedRadioUrl)
    ON_BN_CLICKED(IDC_RADIO_DOWNLOAD, &CSendLiveContentDlg::OnBnClickedRadioDownload)
    ON_WM_TIMER()
END_MESSAGE_MAP()

// CSendLiveContentDlg message handlers

BOOL CSendLiveContentDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_wndUsers.ModifyStyle(TVS_CHECKBOXES, 0);
    m_wndUsers.ModifyStyle(0, TVS_CHECKBOXES);

    for(auto i : m_users)
    {
        HTREEITEM hItem = m_wndUsers.InsertItem(GetDisplayName(i.second));
        m_wndUsers.SetItemData(hItem, i.first);
        if(i.first == m_nUserID || m_nUserID == TT_USERID_MAX)
            m_wndUsers.SetCheck(hItem, TRUE);
    }


    for (auto f : m_files)
    {
        m_wndFiles.AddString(f.szFileName);
    }
    m_wndUrlRadio.SetCheck(BST_CHECKED);
    OnBnClickedRadioUrl();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CSendLiveContentDlg::OnBnClickedRadioUrl()
{
    m_wndUrl.EnableWindow(TRUE);
    m_wndFiles.EnableWindow(FALSE);
}

void CSendLiveContentDlg::OnBnClickedRadioDownload()
{
    m_wndUrl.EnableWindow(FALSE);
    m_wndFiles.EnableWindow(TRUE);
    if(m_files.size())
        m_wndFiles.SetCurSel(0);
}

void CSendLiveContentDlg::OnOK()
{
    if(m_wndUrlRadio.GetCheck() == BST_CHECKED)
    {
        CString szUrl;
        m_wndUrl.GetWindowText(szUrl);
        if (!std::regex_search(szUrl.GetBuffer(), std::wregex(_T("^http[s]?://"))) &&
            !std::regex_search(szUrl.GetBuffer(), std::wregex(_T("^") TTURL)))
        {
            m_wndUrl.SetWindowText(_T("http://") + szUrl);
        }
    }
    
    CDialog::OnOK();

    m_bURL = m_wndUrlRadio.GetCheck() == BST_CHECKED;

    for (HTREEITEM hItem=m_wndUsers.GetFirstVisibleItem();hItem != NULL;hItem = m_wndUsers.GetNextVisibleItem(hItem))
    {
        if (!m_wndUsers.GetCheck(hItem))
            m_users.erase(m_wndUsers.GetItemData(hItem));
    }
}


void CSendLiveContentDlg::OnTimer(UINT_PTR nIDEvent)
{
    CDialog::OnTimer(nIDEvent);
}
