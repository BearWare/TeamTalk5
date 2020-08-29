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
    DDX_Control(pDX, IDC_STATIC_AUTHOR, m_wndAuthor);
    DDX_Control(pDX, IDC_STATIC_COPYRIGHT, m_wndCopyright);
    DDX_Control(pDX, IDC_STATIC_IMAGE, m_wndImage);
    DDX_Control(pDX, IDC_STATIC_COMPILED, m_wndCompiled);
    DDX_Text(pDX, IDC_STATIC_COMPILED, m_szCompiled);
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

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
