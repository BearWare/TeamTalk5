// gui\WebLogin.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "WebLoginDlg.h"
#include "AppInfo.h"
#include "afxdialogex.h"
#include <Helper.h>
#include <regex>

// CWebLogin dialog

IMPLEMENT_DYNAMIC(CWebLoginDlg, CDialogEx)

CWebLoginDlg::CWebLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_WEBLOGIN, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

CWebLoginDlg::~CWebLoginDlg()
{
}

void CWebLoginDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EXPLORER1, m_wndWebBrowser);
}


BEGIN_MESSAGE_MAP(CWebLoginDlg, CDialogEx)
    ON_WM_SIZE()
END_MESSAGE_MAP()


// CWebLogin message handlers


BOOL CWebLoginDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    static CResizer::CBorderInfo s_bi[] = {

        { IDC_EXPLORER1,
        { CResizer::eFixed, IDC_MAIN, CResizer::eLeft },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop },
        { CResizer::eFixed, IDC_MAIN, CResizer::eRight },
        { CResizer::eFixed, IDC_MAIN, CResizer::eBottom } },
    };
    const int nSize = sizeof(s_bi) / sizeof(s_bi[0]);
    m_resizer.Init(m_hWnd, NULL, s_bi, nSize);

    CString szUrl = _T("https://www.facebook.com/v2.10/dialog/oauth?");
    szUrl += _T("client_id=") WEBLOGIN_FACEBOOK_CLIENTID;
    szUrl += _T("&redirect_uri=") WEBLOGIN_FACEBOOK_REDIRECT;
    szUrl += _T("&response_type=code%20token");
    m_wndWebBrowser.Navigate(szUrl, NULL, NULL, NULL, NULL);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
BEGIN_EVENTSINK_MAP(CWebLoginDlg, CDialogEx)
    ON_EVENT(CWebLoginDlg, IDC_EXPLORER1, 252, CWebLoginDlg::NavigateComplete2Explorer1, VTS_DISPATCH VTS_PVARIANT)
END_EVENTSINK_MAP()


void CWebLoginDlg::NavigateComplete2Explorer1(LPDISPATCH pDisp, VARIANT* URL)
{
    if(URL->vt != VT_BSTR)
        return;

    std::string url = STR_UTF8(URL->bstrVal, 0xFFFF);
    std::smatch sm;
    if(std::regex_search(url, sm, std::regex("&code=([A-Za-z0-9\\-_]*)")) && sm.size())
    {
        m_szPassword = STR_UTF8(sm[1]);
        TRACE(_T("Facebook URL: %s\n"), STR_UTF8(url));
    }

    if(m_szPassword.GetLength())
        OnOK();
}


void CWebLoginDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    m_resizer.Move();
}
