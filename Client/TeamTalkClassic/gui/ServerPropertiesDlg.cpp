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

// ServerPropertiesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../Resource.h"
#include "ServerPropertiesDlg.h"


// CServerPropertiesDlg dialog

IMPLEMENT_DYNAMIC(CServerPropertiesDlg, CDialog)

CServerPropertiesDlg::CServerPropertiesDlg(BOOL bReadOnly, CWnd* pParent /*=NULL*/)
	: CDialog(CServerPropertiesDlg::IDD, pParent)
    , m_bReadOnly(bReadOnly)
    , m_szSrvName(_T(""))
    , m_nMaxUsers(0)
    , m_szMOTD(_T(""))
    , m_nTcpPort(0)
    , m_nUdpPort(0)
    , m_nUserTimeout(0)
    , m_bAutoSave(FALSE)
    , m_szVersion(_T(""))
    , m_nAudioTx(0)
    , m_nVideoTx(0)
    , m_nTotalTx(0)
    , m_nDesktopTxMax(0)
    , m_bShowMOTD(FALSE)
    , m_nMediaFileTx(0)
    , m_nMaxIPLogins(0)
    , m_nLoginsBan(0)
{
}

CServerPropertiesDlg::~CServerPropertiesDlg()
{
}

void CServerPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_SRVNAME, m_szSrvName);
    DDX_Text(pDX, IDC_EDIT_MAXUSERS, m_nMaxUsers);
    DDX_Text(pDX, IDC_EDIT_MOTD, m_szMOTD);
    DDX_Text(pDX, IDC_EDIT_TCPPORT, m_nTcpPort);
    DDV_MinMaxInt(pDX, m_nTcpPort, 1, 65535);
    DDX_Text(pDX, IDC_EDIT_UDPPORT, m_nUdpPort);
    DDV_MinMaxInt(pDX, m_nUdpPort, 1, 65535);
    DDX_Text(pDX, IDC_EDIT_USERTIMEOUT, m_nUserTimeout);
    DDV_MinMaxInt(pDX, m_nUserTimeout, 1, 900);
    DDX_Check(pDX, IDC_CHECK_AUTOSAVE, m_bAutoSave);
    DDX_Control(pDX, IDOK, m_btnOk);
    DDX_Control(pDX, IDCANCEL, m_btnCancel);
    DDX_Control(pDX, IDC_EDIT_SRVNAME, m_wndSrvName);
    DDX_Control(pDX, IDC_EDIT_MAXUSERS, m_wndMaxUsers);
    DDX_Control(pDX, IDC_EDIT_MOTD, m_wndMOTD);
    DDX_Control(pDX, IDC_EDIT_TCPPORT, m_wndTcpPort);
    DDX_Control(pDX, IDC_EDIT_UDPPORT, m_wndUdpPort);
    DDX_Control(pDX, IDC_EDIT_USERTIMEOUT, m_wndUserTimeout);
    DDX_Control(pDX, IDC_CHECK_AUTOSAVE, m_wndAutoSave);
    DDX_Text(pDX, IDC_EDIT_SRVVERSION, m_szVersion);
    DDX_Text(pDX, IDC_EDIT_AUDIOTX, m_nAudioTx);
    DDV_MinMaxInt(pDX, m_nAudioTx, 0, 1000000);
    DDX_Text(pDX, IDC_EDIT_VIDEOTX, m_nVideoTx);
    DDV_MinMaxInt(pDX, m_nVideoTx, 0, 1000000);
    DDX_Text(pDX, IDC_EDIT_TOTALTX, m_nTotalTx);
    DDV_MinMaxInt(pDX, m_nTotalTx, 0, 1000000);
    DDX_Control(pDX, IDC_EDIT_AUDIOTX, m_wndAudioTX);
    DDX_Control(pDX, IDC_EDIT_VIDEOTX, m_wndVideoTX);
    DDX_Control(pDX, IDC_EDIT_TOTALTX, m_wndTotalTX);
    DDX_Control(pDX, IDC_EDIT_DESKTOPTX, m_wndDesktopTx);
    DDX_Text(pDX, IDC_EDIT_DESKTOPTX, m_nDesktopTxMax);
    DDX_Check(pDX, IDC_CHECK_SHOWMOTD, m_bShowMOTD);
    DDX_Control(pDX, IDC_CHECK_SHOWMOTD, m_wndShowMOTD);
    DDX_Control(pDX, IDC_EDIT_MEDIAFILETX, m_wndMediaFileTx);
    DDX_Text(pDX, IDC_EDIT_MEDIAFILETX, m_nMediaFileTx);
    DDX_Text(pDX, IDC_EDIT_IPLOGINS, m_nMaxIPLogins);
    DDX_Text(pDX, IDC_EDIT_LOGINBAN, m_nLoginsBan);
    DDX_Control(pDX, IDC_EDIT_LOGINBAN, m_wndLoginBan);
    DDX_Control(pDX, IDC_EDIT_IPLOGINS, m_wndIPLogins);
}


BEGIN_MESSAGE_MAP(CServerPropertiesDlg, CDialog)
    ON_BN_CLICKED(IDC_CHECK_SHOWMOTD, &CServerPropertiesDlg::OnBnClickedCheckShowmotd)
END_MESSAGE_MAP()


// CServerPropertiesDlg message handlers

BOOL CServerPropertiesDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    m_wndSrvName.SetReadOnly(m_bReadOnly);
    m_wndMaxUsers.SetReadOnly(m_bReadOnly);
    m_wndMOTD.SetReadOnly(m_bReadOnly);
    m_wndShowMOTD.EnableWindow(!m_bReadOnly);
    m_wndTcpPort.SetReadOnly(m_bReadOnly);
    m_wndUdpPort.SetReadOnly(m_bReadOnly);
    m_wndUserTimeout.SetReadOnly(m_bReadOnly);
    m_wndAutoSave.EnableWindow(!m_bReadOnly);
    m_wndAudioTX.SetReadOnly(m_bReadOnly);
    m_wndVideoTX.SetReadOnly(m_bReadOnly);
    m_wndDesktopTx.SetReadOnly(m_bReadOnly);
    m_wndMediaFileTx.SetReadOnly(m_bReadOnly);
    m_wndTotalTX.SetReadOnly(m_bReadOnly);
    m_wndIPLogins.SetReadOnly(m_bReadOnly);
    m_wndLoginBan.SetReadOnly(m_bReadOnly);

    if(m_szMOTDRaw.GetLength() && !m_bReadOnly)
    {
        m_wndShowMOTD.SetCheck(BST_CHECKED);
        OnBnClickedCheckShowmotd();
    }
    
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CServerPropertiesDlg::OnBnClickedCheckShowmotd()
{
    if(m_wndShowMOTD.GetCheck() == BST_CHECKED)
        m_wndMOTD.SetWindowText(m_szMOTDRaw);
    else
        m_wndMOTD.SetWindowText(m_szMOTD);
}
