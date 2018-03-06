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
#include "afxcmn.h"


// CTextToSpeechPage dialog

class CTextToSpeechPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CTextToSpeechPage)

public:
	CTextToSpeechPage();
	virtual ~CTextToSpeechPage();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_TEXTTOSPEECHPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

    void Sync();
public:
    CTreeCtrl m_wndTree;
    virtual BOOL OnInitDialog();
    virtual BOOL OnKillActive();
    afx_msg void OnNMClickTreeTts(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnTvnKeydownTreeTts(NMHDR *pNMHDR, LRESULT *pResult);
    
    TTSEvents m_uTTSEvents;
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};
