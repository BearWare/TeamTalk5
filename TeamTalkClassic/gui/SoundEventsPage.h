/*
 * Copyright (c) 2005-2016, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
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

    CString GetEventFilePath();
public:
    CString m_szNewMessagePath;
    CString m_szNewUserPath;
    CString m_szUserRemovedPath;
    CString m_szHotKeyPath;
    CString m_szServerLostPath;
    CEdit m_wndHotKey;
    CEdit m_wndNewMessage;
    CEdit m_wndNewUser;
    CEdit m_wndServerLost;
    CEdit m_wndUserRemoved;
    CEdit m_wndUserReturned;
    afx_msg void OnBnClickedButtonEventsNewuser();
    afx_msg void OnBnClickedButtonEventsUserremoved();
    afx_msg void OnBnClickedButtonEventsServerlost();
    afx_msg void OnBnClickedButtonEventsNewmessage();
    afx_msg void OnBnClickedButtonEventsHotkey();
    afx_msg void OnBnClickedButtonEventsChanmessage();
    CEdit m_wndChanMsgEdit;
    CString m_szChanMsg;
    afx_msg void OnBnClickedButtonEventsStoppedtalk();
    CEdit m_wndStopTalkEdit;
    CString m_szStopTalk;
    virtual BOOL OnInitDialog();
    CEdit m_wndFilesUpd;
    CString m_szFilesUpd;
    CEdit m_wndTransferEnd;
    CString m_szTransferEnd;
    afx_msg void OnBnClickedButtonEventsFilesupd();
    afx_msg void OnBnClickedButtonEventsTransferend();
    afx_msg void OnBnClickedButtonEventsVideosession();
    afx_msg void OnBnClickedButtonEventsDesktopsession();
    afx_msg void OnBnClickedButtonEventsQuestionmode();
    CEdit m_wndVideoSession;
    CEdit m_wndDesktopSession;
    CEdit m_wndQuestionMode;
    CString m_szNewVideoSession;
    CString m_szNewDesktopSession;
    CString m_szQuestionMode;
    afx_msg void OnBnClickedButtonEventsDesktopaccessreq();
    CString m_szDesktopAccessReq;
    CEdit m_wndDesktopAccessReq;
};
