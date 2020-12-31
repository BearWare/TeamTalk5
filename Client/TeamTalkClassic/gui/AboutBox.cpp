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

#include "stdafx.h"
#include "Resource.h"
#include "AboutBox.h"

#include "AppInfo.h"

// CAboutBox dialog

IMPLEMENT_DYNAMIC(CAboutBox, CDialog)
CAboutBox::CAboutBox(CWnd* pParent /*=NULL*/)
    : CDialog(CAboutBox::IDD, pParent)
    , m_strProgramTitle(_T(""))
    , m_strVersion(_T(""))
    , m_szCompiled(_T(""))
{
}

CAboutBox::~CAboutBox()
{
}

void CAboutBox::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_STATIC_APPTITLE, m_strProgramTitle);
    DDX_Text(pDX, IDC_STATIC_VERSION, m_strVersion);
    DDX_Control(pDX, IDC_STATIC_APPTITLE, m_wndProgramTitle);
    DDX_Control(pDX, IDC_STATIC_VERSION, m_wndVersion);
    DDX_Control(pDX, IDC_STATIC_COPYRIGHT, m_wndCopyright);
    DDX_Control(pDX, IDC_STATIC_IMAGE, m_wndImage);
    DDX_Control(pDX, IDC_STATIC_COMPILED, m_wndCompiled);
    DDX_Text(pDX, IDC_STATIC_COMPILED, m_szCompiled);
    DDX_Control(pDX, IDC_RICHEDIT2_CREDITS, m_wndCredits);
}


BEGIN_MESSAGE_MAP(CAboutBox, CDialog)
END_MESSAGE_MAP()


// CAboutBox message handlers

BOOL CAboutBox::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_wndProgramTitle.SetWindowText(APPNAME);
    m_wndVersion.SetWindowText(APPVERSION);

    VERIFY(font1.CreateFont(
       38,                        // nHeight
       0,                         // nWidth
       0,                         // nEscapement
       0,                         // nOrientation
       FW_BOLD,                 // nWeight
       TRUE,                     // bItalic
       FALSE,                     // bUnderline
       0,                         // cStrikeOut
       ANSI_CHARSET,              // nCharSet
       OUT_DEFAULT_PRECIS,        // nOutPrecision
       CLIP_DEFAULT_PRECIS,       // nClipPrecision
       ANTIALIASED_QUALITY,           // nQuality
       DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
       _T("Arial")));                 // lpszFacename

    VERIFY(font2.CreateFont(
       24,                        // nHeight
       0,                         // nWidth
       0,                         // nEscapement
       0,                         // nOrientation
       FW_BOLD,                 // nWeight
       FALSE,                     // bItalic
       FALSE,                     // bUnderline
       0,                         // cStrikeOut
       ANSI_CHARSET,              // nCharSet
       OUT_DEFAULT_PRECIS,        // nOutPrecision
       CLIP_DEFAULT_PRECIS,       // nClipPrecision
       ANTIALIASED_QUALITY,           // nQuality
       DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
       _T("Arial")));                 // lpszFacename

    m_wndProgramTitle.SetFont(&font1);
    m_wndVersion.SetFont(&font2);

    TRANSLATE(*this, IDD);
    SetAccessibleName(m_wndCredits, LoadText(IDS_ABOUT_CREDITS, _T("Credits")));

    CHARFORMAT cfDefault = {};

    cfDefault.cbSize = sizeof (CHARFORMAT);  
    cfDefault.dwMask = CFM_UNDERLINE | CFM_BOLD;
    cfDefault.dwEffects = CFE_UNDERLINE | CFE_BOLD;
    cfDefault.crTextColor = RGB(0, 0, 0);

    const CString EOL(_T("\r\n"));
    AddLine(LoadText(IDS_ABOUT_CREDITS, _T("Credits")) + EOL + EOL, cfDefault);

    cfDefault.dwEffects = CFE_BOLD;
    AddLine(LoadText(IDS_ABOUT_CONTRIB, _T("Contributors")) + EOL, cfDefault);

    cfDefault.dwEffects = 0;
    AddLine(_T("Bjørn Damstedt Rasmussen, ") + LoadText(IDS_ABOUT_DEV, _T("developer")) + EOL, cfDefault);
    AddLine(_T("Beqa Gozalishvili, ") + LoadText(IDS_ABOUT_DEV, _T("developer")) + EOL, cfDefault);
    AddLine(_T("Oreonan, ") + LoadText(IDS_ABOUT_DEV, _T("developer")) + EOL, cfDefault);

    cfDefault.dwEffects = CFE_BOLD;
    AddLine(EOL + LoadText(IDS_ABOUT_TRANSLATORS, _T("Translators")) + EOL, cfDefault);
    cfDefault.dwEffects = 0;
    AddLine(LoadText(IDC_STATIC_TRANSLATOR, _T("Translated by Bjørn Damstedt Rasmussen")) + EOL, cfDefault);

    cfDefault.dwEffects = CFE_BOLD;
    AddLine(EOL + LoadText(IDS_ABOUT_LIB, _T("Libraries")) + EOL, cfDefault);
    cfDefault.dwEffects = 0;
    AddLine(LoadText(IDS_ABOUT_LIBUSE, _T("TeamTalk uses the following libraries:")) + EOL, cfDefault);
    AddLine(_T("ACE") + EOL, cfDefault);
    AddLine(_T("FFmpeg") + EOL, cfDefault);
    AddLine(_T("OGG") + EOL, cfDefault);
    AddLine(_T("OpenSSL") + EOL, cfDefault);
    AddLine(_T("OPUS") + EOL, cfDefault);
    AddLine(_T("OPUS-tools") + EOL, cfDefault);
    AddLine(_T("PortAudio") + EOL, cfDefault);
    AddLine(_T("Qt") + EOL, cfDefault);
    AddLine(_T("Speex") + EOL, cfDefault);
    AddLine(_T("SpeexDSP") + EOL, cfDefault);
    AddLine(_T("TinyXML") + EOL, cfDefault);
    AddLine(_T("WebM") + EOL, cfDefault);
    AddLine(_T("Zlib") + EOL, cfDefault);
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CAboutBox::AddLine(const CString& szLine, CHARFORMAT& cf)
{
    m_wndCredits.SetSelectionCharFormat(cf);
    m_wndCredits.SetSel(m_wndCredits.GetTextLength(), m_wndCredits.GetTextLength());
    m_wndCredits.ReplaceSel(szLine);
}

