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
