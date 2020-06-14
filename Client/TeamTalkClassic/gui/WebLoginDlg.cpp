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
    , m_bCancelled(FALSE)
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
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CWebLogin message handlers


BOOL CWebLoginDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    TRANSLATE(*this, IDD);

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
        m_szPassword = WEBLOGIN_FACEBOOK_PASSWDPREFIX + STR_UTF8(sm[1]);
    }

    if(std::regex_search(url, sm, std::regex("#access_token=([A-Za-z0-9\\-_]*)")) && sm.size())
    {
        m_szToken = STR_UTF8(sm[1]);
    }
    
    if(m_szPassword.GetLength() && !m_bCancelled)
    {
        SetTimer(1, 1000, NULL);
    }

    if(m_bCancelled && CString(URL->bstrVal) == WEBLOGIN_FACEBOOK_LOGOUT_REDIRECT)
    {
        CDialogEx::OnCancel();
    }
}


void CWebLoginDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    m_resizer.Move();
}


void CWebLoginDlg::OnCancel()
{
    KillTimer(1);

    if(m_bCancelled)
    {
        CDialogEx::OnCancel();
        return;
    }

    m_bCancelled = TRUE;

    if(MessageBox(LoadText(IDS_WEBLOGINFBLOGOUT, _T("Do Facebook logout?")), LoadText(IDS_WEBLOGINFBLOGIN, _T("Facebook Login")), MB_YESNO) == IDYES)
    {
        CString szUrl = WEBLOGIN_FACEBOOK_LOGOUT_URL;
        szUrl += _T("next=") WEBLOGIN_FACEBOOK_LOGOUT_REDIRECT;
        szUrl += _T("&access_token=") + m_szToken;
        m_wndWebBrowser.Navigate(szUrl, NULL, NULL, NULL, NULL);
    }
    else
    {
        if(m_szPassword.GetLength())
            OnOK();
        else
            CDialogEx::OnCancel();
    }
}


void CWebLoginDlg::OnTimer(UINT_PTR nIDEvent)
{
    CDialogEx::OnTimer(nIDEvent);
    OnOK();
}
