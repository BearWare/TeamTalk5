// BearWareLoginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BearWareLoginDlg.h"
#include "afxdialogex.h"
#include <WinInet.h>

// CBearWareLoginDlg dialog

IMPLEMENT_DYNAMIC(CBearWareLoginDlg, CDialogEx)

CBearWareLoginDlg::CBearWareLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_BEARWARELOGIN, pParent)
    , m_szUsername(_T(""))
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif
}

CBearWareLoginDlg::~CBearWareLoginDlg()
{
}

void CBearWareLoginDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_BEARWAREID, m_wndUsername);
    DDX_Text(pDX, IDC_EDIT_BEARWAREID, m_szUsername);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_wndPassword);
    DDX_Control(pDX, IDOK, m_wndOK);
}

BOOL CBearWareLoginDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    TRANSLATE(*this, IDD);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CBearWareLoginDlg, CDialogEx)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON_BEARWARECREATE, &CBearWareLoginDlg::OnBnClickedButtonBearwarecreate)
END_MESSAGE_MAP()

enum
{
    TIMER_URL_CHECK = 1,
    TIMER_URL_TIMEOUT,
};
// CBearWareLoginDlg message handlers

void CBearWareLoginDlg::OnTimer(UINT_PTR nIDEvent)
{
    CDialogEx::OnTimer(nIDEvent);

    switch (nIDEvent)
    {
    case TIMER_URL_CHECK :
    {
        if (m_webLogin->SendReady())
        {
            m_webLogin->Send(_T(""));
        }
        else if (m_webLogin->ResponseReady())
        {
            KillTimer(TIMER_URL_CHECK);
            KillTimer(TIMER_URL_TIMEOUT);

            CString szResponse = m_webLogin->GetResponse(), szNickname;
            TRACE(_T("\n") + szResponse + _T("\n"));

            std::string xml = STR_UTF8(szResponse, szResponse.GetLength() * 4);
            teamtalk::XMLDocument xmlDoc(TT_XML_ROOTNAME, TEAMTALK_XML_VERSION);
            if(xmlDoc.Parse(xml))
            {
                szNickname = STR_UTF8(xmlDoc.GetValue(false, "teamtalk/bearware/nickname").c_str());
                m_szUsername = STR_UTF8(xmlDoc.GetValue(false, "teamtalk/bearware/username").c_str());
                m_szToken = STR_UTF8(xmlDoc.GetValue(false, "teamtalk/bearware/token").c_str());
            }

            CString szCaption = LoadText(IDD);
            if (m_szUsername.GetLength())
            {
                CString szMessage, szCaption = LoadText(IDD_DIALOG_BEARWARELOGIN);
                szMessage.Format(LoadText(IDS_BEARWAREAUTHSUCCESS), szNickname, m_szUsername);
                MessageBox(szMessage, szCaption);

                m_wndUsername.SetWindowText(m_szUsername);

                CDialogEx::OnOK();
            }
            else
            {
                MessageBox(LoadText(IDS_BEARWAREAUTHFAIL), LoadText(IDD), MB_OK);
                m_wndUsername.EnableWindow(TRUE);
                m_wndPassword.EnableWindow(TRUE);
                m_wndOK.EnableWindow(TRUE);
            }
        }
        break;
    }
    case TIMER_URL_TIMEOUT :
        KillTimer(TIMER_URL_CHECK);
        KillTimer(TIMER_URL_TIMEOUT);

        m_wndUsername.EnableWindow(TRUE);
        m_wndPassword.EnableWindow(TRUE);
        m_wndOK.EnableWindow(TRUE);

        MessageBox(LoadText(IDS_BEARWARETIMEOUT), LoadText(IDD), MB_OK);
        break;
    }
}

void CBearWareLoginDlg::OnOK()
{
    m_wndUsername.EnableWindow(FALSE);
    m_wndPassword.EnableWindow(FALSE);
    m_wndOK.EnableWindow(FALSE);

    CString szUsername, szPassword;
    m_wndUsername.GetWindowText(szUsername);
    m_wndPassword.GetWindowText(szPassword);

    szUsername.Trim();
    szUsername.MakeLower();
    auto POSTFIXLEN = _tcslen(_T(WEBLOGIN_BEARWARE_USERNAMEPOSTFIX));
    if (szUsername.Right(POSTFIXLEN) == WEBLOGIN_BEARWARE_USERNAMEPOSTFIX)
        szUsername = szUsername.Left(szUsername.GetLength() - POSTFIXLEN);
    m_wndUsername.SetWindowText(szUsername);

    TCHAR szUrlUsername[INTERNET_MAX_URL_LENGTH] = _T("");
    TCHAR szUrlPassword[INTERNET_MAX_URL_LENGTH] = _T("");
    DWORD dwNewLen = INTERNET_MAX_URL_LENGTH;
    UrlEscape(szUsername, szUrlUsername, &dwNewLen, URL_ESCAPE_PERCENT | URL_ESCAPE_AS_UTF8);
    dwNewLen = INTERNET_MAX_URL_LENGTH;
    UrlEscape(szPassword, szUrlPassword, &dwNewLen, URL_ESCAPE_PERCENT | URL_ESCAPE_AS_UTF8);

    CString szUrl = WEBLOGIN_BEARWARE_URLAUTH(szUrlUsername, szUrlPassword);

    m_webLogin.reset(new CHttpRequest(szUrl));

    SetTimer(TIMER_URL_CHECK, 500, NULL);
    SetTimer(TIMER_URL_TIMEOUT, 10000, NULL);
}


void CBearWareLoginDlg::OnBnClickedButtonBearwarecreate()
{
}
