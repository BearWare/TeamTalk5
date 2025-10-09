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

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include <PlaySoundThread.h>

// CSoundEventsPage dialog

class CSoundEventsPage : public CPropertyPage
{
    DECLARE_DYNAMIC(CSoundEventsPage)

public:
    CSoundEventsPage();
    virtual ~CSoundEventsPage();

    // Dialog Data
    enum { IDD = IDD_PROPPAGE_SOUNDEVENTSPAGE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    CString GetEventFilePath(const CString& szInitFile);
    void ToggleItem(HTREEITEM hItem);
public:
    virtual BOOL OnInitDialog();
    CTreeCtrl m_wndTree;
    int m_nClientSoundsVsVoice;
    CSliderCtrl m_wndClientSoundsVsVoice;
    CComboBox m_wndPlaybackMode;
    PlaybackMode m_nPlaybackMode;

    SoundEvents m_uSoundEvents;
    std::map<SoundEvent, CString> m_SoundFiles;
    afx_msg void OnTvnKeydownTreeSoundevents(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMClickTreeSoundevents(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnCbnSelchangeComboPBMode();
};
