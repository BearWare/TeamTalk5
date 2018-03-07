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

// C:\Programming\Projects\BearWare\TeamTalk5\TeamTalkClassic\gui\CodecLimitTab.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "CodecLimitTab.h"
#include "afxdialogex.h"


// CCodecLimitTab dialog

IMPLEMENT_DYNAMIC(CCodecLimitTab, CMyTab)

CCodecLimitTab::CCodecLimitTab(CWnd* pParent /*=NULL*/)
	: CMyTab(IDD_TAB_CODECLIMIT, pParent)
    , m_nBitrate(DEFAULT_AUDIOCODEC_BPS_LIMIT)

{
}

CCodecLimitTab::~CCodecLimitTab()
{
}

void CCodecLimitTab::DoDataExchange(CDataExchange* pDX)
{
    CMyTab::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_BITRATE, m_wndBitrate);
    DDX_Text(pDX, IDC_EDIT_BITRATE, m_nBitrate);
}


BEGIN_MESSAGE_MAP(CCodecLimitTab, CDialog)
    ON_WM_SIZE()
END_MESSAGE_MAP()


// CCodecLimitTab message handlers


BOOL CCodecLimitTab::OnInitDialog()
{
    CMyTab::OnInitDialog();

    TRANSLATE(*this, IDD);

    static CResizer::CBorderInfo s_bi[] = {

        { IDC_STATIC_AUDCODECLIMIT,
        { CResizer::eFixed, IDC_MAIN, CResizer::eLeft },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop },
        { CResizer::eFixed, IDC_MAIN, CResizer::eRight },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop } },

        { IDC_STATIC_MAXABPS,
        { CResizer::eFixed, IDC_MAIN, CResizer::eLeft },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop },
        { CResizer::eProportional, IDC_MAIN, CResizer::eRight },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop } },

        { IDC_EDIT_BITRATE,
        { CResizer::eFixed, IDC_MAIN, CResizer::eRight },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop },
        { CResizer::eWidth, IDC_MAIN, CResizer::eRight },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop } },

        { IDC_STATIC_KBPS,
        { CResizer::eProportional, IDC_MAIN, CResizer::eRight },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop },
        { CResizer::eFixed, IDC_MAIN, CResizer::eRight },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop } },

    };
    const int nSize = sizeof(s_bi) / sizeof(s_bi[0]);
    m_resizer.Init(m_hWnd, NULL, s_bi, nSize);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}


void CCodecLimitTab::OnSize(UINT nType, int cx, int cy)
{
    CMyTab::OnSize(nType, cx, cy);

    m_resizer.Move();

}
