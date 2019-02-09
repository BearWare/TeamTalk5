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
#include "Advancedpage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CAdvancedPage dialog

IMPLEMENT_DYNAMIC(CAdvancedPage, CPropertyPage)
CAdvancedPage::CAdvancedPage()
    : CPropertyPage(CAdvancedPage::IDD)
    , m_bMixerAutoSelect(FALSE)
    , m_nMixerIndex(UNDEFINED)
    , m_bBoostBug(FALSE)
{
}

CAdvancedPage::~CAdvancedPage()
{
}

void CAdvancedPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_MIXERAUTOSELECT, m_bMixerAutoSelect);
    DDX_Control(pDX, IDC_CHECK_MIXERAUTOSELECT, m_wndMixerAutoSelect);
    DDX_CBIndex(pDX, IDC_COMBO_MIXERINPUTS, m_nMixerIndex);
    DDX_Control(pDX, IDC_COMBO_MIXERINPUTS, m_wndMixerInputs);
    DDX_Check(pDX, IDC_CHECK_BOOSTBUG, m_bBoostBug);
}

void CAdvancedPage::UpdateControls()
{
    if(m_bMixerAutoSelect)
    {
        m_wndMixerInputs.ResetContent();
        CStringList list;
        int count = TT_Mixer_GetWaveInControlCount(0);
        int nSelectedIndex = -1;
        for(int i=0;i<count;i++)
        {
            TCHAR buff[TT_STRLEN] = {};
            TT_Mixer_GetWaveInControlName(0, i, buff);
            list.AddTail(buff);
            if(TT_Mixer_GetWaveInControlSelected(0, i))
                nSelectedIndex = i;
        }

        if(list.GetCount())
        {
            for(POSITION pos=list.GetHeadPosition(); pos!= NULL;)
                m_wndMixerInputs.AddString(list.GetNext(pos));
            int index = -1;
                m_wndMixerInputs.SetCurSel(m_nMixerIndex == -1? nSelectedIndex : m_nMixerIndex);
        }
        else
        {
            AfxMessageBox(_T("Failed to initialize Windows' mixer"));
        }
    }
    m_wndMixerInputs.EnableWindow(m_bMixerAutoSelect);
}


BEGIN_MESSAGE_MAP(CAdvancedPage, CPropertyPage)
    ON_BN_CLICKED(IDC_CHECK_MIXERAUTOSELECT, OnBnClickedCheckMixerautoselect)
END_MESSAGE_MAP()


// CAdvancedPage message handlers

BOOL CAdvancedPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    TRANSLATE(*this, IDD);

    UpdateControls();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CAdvancedPage::OnBnClickedCheckMixerautoselect()
{
    m_bMixerAutoSelect = !m_bMixerAutoSelect;
    UpdateControls();
}
