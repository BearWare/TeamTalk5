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
#include "ClientPage.h"
#include ".\clientpage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CClientPage dialog

IMPLEMENT_DYNAMIC(CClientPage, CPropertyPage)
CClientPage::CClientPage()
    : CPropertyPage(CClientPage::IDD)
    , m_bAutoConnect(FALSE)
    , m_nClientTcpPort(0)
    , m_nClientUdpPort(0)
    , m_bReconnect(FALSE)
    , m_bAutoJoinRoot(FALSE)
    , m_bSubUserMsg(FALSE)
    , m_bSubChanMsg(FALSE)
    , m_bSubVideo(FALSE)
    , m_bSubBcastMsg(FALSE)
    , m_bSubVoice(FALSE)
    , m_bSubDesktop(FALSE)
    , m_bSubMediaFile(FALSE)
{
}

CClientPage::~CClientPage()
{
}

void CClientPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_AUTOCONNECT, m_bAutoConnect);
    DDX_Text(pDX, IDC_EDIT_CCLIENTPORT, m_nClientTcpPort);
    DDV_MinMaxUInt(pDX, m_nClientTcpPort, 0, 65535);
    DDX_Text(pDX, IDC_EDIT_CSOUNDPORT, m_nClientUdpPort);
    DDV_MinMaxUInt(pDX, m_nClientUdpPort, 0, 65535);
    DDX_Check(pDX, IDC_CHECK_RECONNECT, m_bReconnect);
    DDX_Control(pDX, IDC_EDIT_CCLIENTPORT, m_wndHostPort);
    DDX_Control(pDX, IDC_EDIT_CSOUNDPORT, m_wndSoundPort);
    DDX_Check(pDX, IDC_CHECK_AUTOJOINROOT, m_bAutoJoinRoot);
    DDX_Check(pDX, IDC_CHECK_SUBUSERMSG, m_bSubUserMsg);
    DDX_Check(pDX, IDC_CHECK_SUBCHANMSG, m_bSubChanMsg);
    DDX_Check(pDX, IDC_CHECK_SUBVIDEO, m_bSubVideo);
    DDX_Check(pDX, IDC_CHECK_SUBBCAST, m_bSubBcastMsg);
    DDX_Check(pDX, IDC_CHECK_SUBVOICE, m_bSubVoice);
    DDX_Check(pDX, IDC_CHECK_SUBDESKTOP, m_bSubDesktop);
    DDX_Check(pDX, IDC_CHECK_SUBMEDIAFILE, m_bSubMediaFile);
}


BEGIN_MESSAGE_MAP(CClientPage, CPropertyPage)
END_MESSAGE_MAP()


// CClientPage message handlers

BOOL CClientPage::OnKillActive()
{
    CString szHost, szSound;
    m_wndHostPort.GetWindowText(szHost);
    m_wndSoundPort.GetWindowText(szSound);
    UINT nHost = _ttoi(szHost);
    UINT nSound = _ttoi(szSound);
    if( szHost == szSound && nHost > 0)
    {
        AfxMessageBox(_T("You cannot use the same ports"));
        return FALSE;
    }

    return CPropertyPage::OnKillActive();
}

BOOL CClientPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    TRANSLATE(*this, IDD);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
