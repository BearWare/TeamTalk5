/*
 * Copyright (c) 2005-2017, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


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

    SoundEvents m_uSoundEvents;
    std::map<SoundEvent, CString> m_SoundFiles;
    afx_msg void OnTvnKeydownTreeSoundevents(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMClickTreeSoundevents(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};
