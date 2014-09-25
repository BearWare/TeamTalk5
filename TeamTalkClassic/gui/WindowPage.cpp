/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */


#include "stdafx.h"
#include "Resource.h"
#include "WindowPage.h"
#include "Helper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CWindowPage dialog

IMPLEMENT_DYNAMIC(CWindowPage, CPropertyPage)
CWindowPage::CWindowPage()
    : CPropertyPage(CWindowPage::IDD)
    , m_bPopMsg(FALSE)
    , m_bAlwaysOnTop(FALSE)
  , m_bShowUserCount(FALSE)
    , m_bDBClickJoin(FALSE)
    , m_bQuitClearChannels(FALSE)
    , m_bTimeStamp(FALSE)
    , m_szLanguage(_T(""))
    , m_bCheckUpdates(FALSE)
{
    m_bTray = FALSE;
    m_bStartMinimized = FALSE;
    memset(&m_lf, 0, sizeof(LOGFONT) );
    m_Font.bBold = m_Font.bItalic = m_Font.bUnderline = FALSE;
    m_Font.nSize = 0;
}

CWindowPage::~CWindowPage()
{
}

void CWindowPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_STARTMINIMIZED, m_bStartMinimized);
    DDX_Check(pDX, IDC_CHECK_TRAY, m_bTray);
    DDX_Control(pDX, IDC_STATIC_FONTNAME, m_wndFontName);
    DDX_Check(pDX, IDC_CHECK_POPUPMSG, m_bPopMsg);
    DDX_Check(pDX, IDC_CHECK_ALWAYSONTOP, m_bAlwaysOnTop);
    DDX_Check(pDX, IDC_CHECK_SHOWUSERCOUNT, m_bShowUserCount);
    DDX_Check(pDX, IDC_CHECK_DBCLICKJOIN, m_bDBClickJoin);
    DDX_Check(pDX, IDC_CHECK_QUITCLEAR, m_bQuitClearChannels);
    DDX_Check(pDX, IDC_CHECK_MSGTIMESTAMP, m_bTimeStamp);
    DDX_CBString(pDX, IDC_COMBO_LANGUAGE, m_szLanguage);
    DDX_Control(pDX, IDC_COMBO_LANGUAGE, m_wndLanguages);
    DDX_Check(pDX, IDC_CHECK_CHECKUPDATES, m_bCheckUpdates);
}

void CWindowPage::ShowFont()
{
    if(m_Font.nSize==0)
    {
        CString szDefault;
        szDefault.LoadString(IDS_DEFAULT);
        m_wndFontName.SetWindowText(szDefault);
        TRANSLATE(m_wndFontName, IDS_DEFAULT);
    }
    else
    {
        CString szFontName = m_Font.szFaceName;
        if(    m_Font.bBold )
            szFontName += ", B";
        if(    m_Font.bItalic )
            szFontName += ", I";
        if(    m_Font.bUnderline )
            szFontName += ", U";
        CString s;s.Format(_T(", %d"), m_Font.nSize);
        szFontName += s;
        m_wndFontName.SetWindowText(szFontName);
    }
}

BEGIN_MESSAGE_MAP(CWindowPage, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_FONT, OnBnClickedButtonFont)
END_MESSAGE_MAP()


// CWindowPage message handlers


void CWindowPage::OnBnClickedButtonFont()
{
    CFontDialog dlg(&m_lf);

    if(dlg.DoModal() == IDOK)
    {
        LOGFONT f;
        dlg.GetCurrentFont(&f);
        m_Font.szFaceName = f.lfFaceName;
        if(dlg.GetSize()>0)
            m_Font.nSize = dlg.GetSize()/10;
        else
            m_Font.nSize = 12;
        m_Font.bBold = dlg.IsBold();
        m_Font.bItalic = dlg.IsItalic();
        m_Font.bUnderline = dlg.IsUnderline();

        ShowFont();
        //CClientDC hDC(this);
        //f.lfHeight = -MulDiv(1, GetDeviceCaps(hDC, LOGPIXELSY), 72);
        memcpy( &m_lf, &f, sizeof(LOGFONT) );
    }
}

BOOL CWindowPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    TRANSLATE(*this, IDD);

    //convert font to LOGFONT
    ConvertFont(m_Font, m_lf);

    ShowFont();

    CStringList strings;
    GetLanguageFiles(strings);
    strings.AddHead(_T(""));

    for(POSITION pos=strings.GetHeadPosition();pos!=NULL;)
        m_wndLanguages.AddString(strings.GetNext(pos));

    if(!m_szLanguage.IsEmpty())
        m_wndLanguages.SelectString(-1, m_szLanguage);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
