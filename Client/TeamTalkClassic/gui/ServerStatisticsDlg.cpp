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
#include "../Resource.h"
#include "ServerStatisticsDlg.h"

IMPLEMENT_DYNAMIC(CServerStatisticsDlg, CDialog)

CServerStatisticsDlg::CServerStatisticsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerStatisticsDlg::IDD, pParent)
{
}

CServerStatisticsDlg::~CServerStatisticsDlg()
{
}

void CServerStatisticsDlg::DoDataExchange(CDataExchange* pDX)
{
    DDX_Text(pDX, IDC_EDIT_TOTAL, m_szTotalRxTx);
    DDX_Text(pDX, IDC_EDIT_VOICE, m_szVoiceRxTx);
    DDX_Text(pDX, IDC_EDIT_VIDEO, m_szVideoRxTx);
    DDX_Text(pDX, IDC_EDIT_MEDIAFILERXTX, m_szMediaFileRXTX);
    DDX_Text(pDX, IDC_EDIT_DESKTOPRXTX, m_szDesktopRXTX);
    DDX_Text(pDX, IDC_EDIT_FILESRXTX, m_szFilesRxTx);
    DDX_Text(pDX, IDC_EDIT_USERSSERVED, m_szUsersServed);
    DDX_Text(pDX, IDC_EDIT_USERSPEAK, m_szUsersPeak);
}

BEGIN_MESSAGE_MAP(CServerStatisticsDlg, CDialog)
END_MESSAGE_MAP()


BOOL CServerStatisticsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
