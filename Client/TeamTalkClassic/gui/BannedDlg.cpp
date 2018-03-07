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

// gui\BannedDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "BannedDlg.h"


static CResizer::CBorderInfo s_bi[] = {

    {IDC_LIST_BANNED,	
    {CResizer::eFixed, IDC_MAIN, CResizer::eLeft}, 
    {CResizer::eFixed, IDC_MAIN, CResizer::eTop},  
    {CResizer::eProportional, IDC_MAIN, CResizer::eRight},
    {CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 

    {IDC_LIST_UNBANNED,	
    {CResizer::eFixed, IDC_MAIN, CResizer::eLeft}, 
    {CResizer::eFixed, IDC_MAIN, CResizer::eTop},  
    {CResizer::eProportional, IDC_MAIN, CResizer::eRight},
    {CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 

    {IDC_STATIC_BANNED,	
    {CResizer::eFixed, IDC_MAIN, CResizer::eLeft}, 
    {CResizer::eFixed, IDC_MAIN, CResizer::eTop},  
    {CResizer::eProportional, IDC_MAIN, CResizer::eRight},
    {CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 

    {IDC_STATIC_UNBANNED,	
    {CResizer::eFixed, IDC_STATIC_BANNED, CResizer::eLeft}, 
    {CResizer::eFixed, IDC_MAIN, CResizer::eTop},  
    {CResizer::eProportional, IDC_MAIN, CResizer::eRight},
    {CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 

    {IDC_BUTTON_BAN,	
    {CResizer::eFixed, IDC_STATIC_BANNED, CResizer::eLeft}, 
    {CResizer::eFixed, IDC_MAIN, CResizer::eBottom},  
    {CResizer::eFixed, IDC_STATIC_BANNED, CResizer::eRight},
    {CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 

    {IDC_BUTTON_UNBAN,	
    {CResizer::eFixed, IDC_STATIC_BANNED, CResizer::eLeft}, 
    {CResizer::eFixed, IDC_MAIN, CResizer::eBottom},  
    {CResizer::eFixed, IDC_STATIC_BANNED, CResizer::eRight},
    {CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 

};

// CBannedDlg dialog

IMPLEMENT_DYNAMIC(CBannedDlg, CDialog)
CBannedDlg::CBannedDlg(CWnd* pParent /*=NULL*/)
: CDialog(CBannedDlg::IDD, pParent)
{
}

CBannedDlg::~CBannedDlg()
{
}

void CBannedDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_BANNED, m_wndBanned);
    DDX_Control(pDX, IDC_LIST_UNBANNED, m_wndUnbanned);
}

BOOL CBannedDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    Update();

    const int nSize = sizeof(s_bi)/sizeof(s_bi[0]);
    m_resizer.Init(m_hWnd, NULL, s_bi, nSize);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CBannedDlg::Update()
{
    m_wndBanned.ResetContent();
    m_wndUnbanned.ResetContent();

    CString szUser;

    for(size_t i=0;i<m_vecBanned.size();i++)
    {
        BannedUser& ban = m_vecBanned[i];
        szUser.Format(_T("%s, %s, %s, %s"), ban.szNickname, ban.szUsername, 
            ban.szIPAddress, ban.szBanTime);
        m_wndBanned.AddString(szUser);
    }

    for(size_t i=0;i<m_vecUnBanned.size();i++)
    {
        BannedUser& ban = m_vecUnBanned[i];
        szUser.Format(_T("%s, %s, %s, %s"), ban.szNickname, ban.szUsername, 
            ban.szIPAddress, ban.szBanTime);
        m_wndUnbanned.AddString(szUser);
    }
}

BEGIN_MESSAGE_MAP(CBannedDlg, CDialog)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_BN_CLICKED(IDC_BUTTON_UNBAN, OnBnClickedButtonUnban)
    ON_BN_CLICKED(IDC_BUTTON_BAN, OnBnClickedButtonBan)
    ON_WM_SIZE()
END_MESSAGE_MAP()


// CBannedDlg message handlers

void CBannedDlg::OnBnClickedOk()
{
    OnOK();
}

void CBannedDlg::OnBnClickedButtonUnban()
{
    for(int i=m_wndBanned.GetCount()-1;i>=0;i--)
        if(m_wndBanned.GetSel(i) > 0)
        {
            m_vecUnBanned.push_back(m_vecBanned[i]);
            m_vecBanned.erase(m_vecBanned.begin()+i);
        }

        Update();
}

void CBannedDlg::OnBnClickedButtonBan()
{
    for(int i=m_wndUnbanned.GetCount()-1;i>=0;i--)
        if(m_wndUnbanned.GetSel(i) > 0)
        {
            m_vecBanned.push_back(m_vecUnBanned[i]);
            m_vecUnBanned.erase(m_vecUnBanned.begin()+i);
        }

        Update();
}

void CBannedDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    m_resizer.Move();
}
