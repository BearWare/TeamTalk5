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

// ChannelOpTab.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ChannelOpTab.h"
#include "afxdialogex.h"


// CChannelOpTab dialog

IMPLEMENT_DYNAMIC(CChannelOpTab, CMyTab)

CChannelOpTab::CChannelOpTab(CWnd* pParent /*=NULL*/)
	: CMyTab(IDD, pParent)
{
}

CChannelOpTab::~CChannelOpTab()
{
}

void CChannelOpTab::DoDataExchange(CDataExchange* pDX)
{
    CMyTab::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_AVAILCHANNELS, m_wndAvailChannels);
    DDX_Control(pDX, IDC_LIST_SELECTEDCHANNELS, m_wndSelChannels);
    DDX_Control(pDX, IDC_BUTTON_ADDCHANNEL, m_btnAddChan);
    DDX_Control(pDX, IDC_BUTTON_DELCHANNEL, m_btnRmChan);
}


BEGIN_MESSAGE_MAP(CChannelOpTab, CMyTab)
    ON_BN_CLICKED(IDC_BUTTON_ADDCHANNEL, &CChannelOpTab::OnBnClickedButtonAddchannel)
    ON_BN_CLICKED(IDC_BUTTON_DELCHANNEL, &CChannelOpTab::OnBnClickedButtonDelchannel)
    ON_WM_SIZE()
END_MESSAGE_MAP()


// CChannelOpTab message handlers


BOOL CChannelOpTab::OnInitDialog()
{
    CMyTab::OnInitDialog();

    TRANSLATE(*this, IDD);

    static CResizer::CBorderInfo s_bi[] = {

        { IDC_STATIC_GRPAUTOOP,
        { CResizer::eFixed, IDC_MAIN, CResizer::eLeft },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop },
        { CResizer::eFixed, IDC_MAIN, CResizer::eRight },
        { CResizer::eFixed, IDC_MAIN, CResizer::eBottom } },

        { IDC_STATIC_AVAILCHANNELS,
        { CResizer::eFixed, IDC_MAIN, CResizer::eLeft },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop },
        { CResizer::eProportional, IDC_MAIN, CResizer::eRight },
        { CResizer::eHeight, IDC_MAIN, CResizer::eBottom } },

        { IDC_LIST_AVAILCHANNELS,
        { CResizer::eFixed, IDC_MAIN, CResizer::eLeft },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop },
        { CResizer::eProportional, IDC_MAIN, CResizer::eRight },
        { CResizer::eFixed, IDC_MAIN, CResizer::eBottom } },

        { IDC_BUTTON_ADDCHANNEL,
        { CResizer::eFixed, IDC_MAIN, CResizer::eXCenter },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop },
        { CResizer::eFixed, IDC_MAIN, CResizer::eXCenter },
        { CResizer::eFixed, IDC_MAIN, CResizer::eBottom } },

        { IDC_BUTTON_DELCHANNEL,
        { CResizer::eFixed, IDC_MAIN, CResizer::eXCenter },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop },
        { CResizer::eFixed, IDC_MAIN, CResizer::eXCenter },
        { CResizer::eFixed, IDC_MAIN, CResizer::eBottom } },

        { IDC_STATIC_SELCHANNELS,
        { CResizer::eProportional, IDC_MAIN, CResizer::eLeft },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop },
        { CResizer::eFixed, IDC_MAIN, CResizer::eRight },
        { CResizer::eHeight, IDC_MAIN, CResizer::eBottom } },

        { IDC_LIST_SELECTEDCHANNELS,
        { CResizer::eProportional, IDC_MAIN, CResizer::eLeft },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop },
        { CResizer::eFixed, IDC_MAIN, CResizer::eRight },
        { CResizer::eFixed, IDC_MAIN, CResizer::eBottom } },

        
    };
    const int nSize = sizeof(s_bi) / sizeof(s_bi[0]);
    m_resizer.Init(m_hWnd, NULL, s_bi, nSize);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CChannelOpTab::OnBnClickedButtonAddchannel()
{
    int i = m_wndAvailChannels.GetCurSel();
    if(i == LB_ERR)
        return;
    DWORD_PTR nItemData = m_wndAvailChannels.GetItemData(i);
    CString szChan;
    m_wndAvailChannels.GetText(i, szChan);
    i = m_wndSelChannels.AddString(szChan);
    m_wndSelChannels.SetItemData(i, nItemData);
}

void CChannelOpTab::OnBnClickedButtonDelchannel()
{
    int i = m_wndSelChannels.GetCurSel();
    if(i == LB_ERR)
        return;
    m_wndSelChannels.DeleteString(i);
}

void CChannelOpTab::OnSize(UINT nType, int cx, int cy)
{
    CMyTab::OnSize(nType, cx, cy);

    m_resizer.Move();
}
