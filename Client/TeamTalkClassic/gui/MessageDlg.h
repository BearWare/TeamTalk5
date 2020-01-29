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

#if !defined(AFX_SENDMESSAGEDLG_H__AC4F57A6_C994_4AE2_905C_9B728B1F6F93__INCLUDED_)
#define AFX_SENDMESSAGEDLG_H__AC4F57A6_C994_4AE2_905C_9B728B1F6F93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SendMessageDlg.h : header file
//
#define WM_MESSAGEDLG_CLOSED (WM_APP+3)

#define MAX_HISTORY_LENGTH 4000

#include "ChatRichEditCtrl.h"
#include "Resizer.h"
/////////////////////////////////////////////////////////////////////////////
// CSendMessageDlg dialog

class CMessageDlg : public CDialog
{
// Construction
public:
    CMessageDlg(CWnd* pParent, const User& myself, const User& user,
                LPCTSTR szLogFolder = NULL);   // standard constructor

    void AppendMessage(const MyTextMessage& msg, BOOL bStore = TRUE);
    void SetAlive(BOOL state);
    BOOL IsAlive();
    LOGFONT m_lf;
  //messages from previous session
    messages_t m_messages;
    BOOL m_bShowTimeStamp;

// Dialog Data
    //{{AFX_DATA(CSendMessageDlg)
    enum { IDD = IDD_DIALOG_MESSAGE };
    CButton    m_btnSend;
    CChatRichEditCtrl    m_richMessage;
    CChatRichEditCtrl    m_richHistory;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSendMessageDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void PostNcDestroy();
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CSendMessageDlg)
    virtual void OnCancel();
//    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonSend();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

private:
    virtual ~CMessageDlg(){}
    BOOL m_bUserAlive;
    User m_user;
    User m_myself;
    CWnd* m_pParent;
    CResizer m_resizer;
    CFont m_Font;
    HACCEL m_hAccel; // accelerator table
    CFile m_logFile;

public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDMESSAGEDLG_H__AC4F57A6_C994_4AE2_905C_9B728B1F6F93__INCLUDED_)
