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

// CAboutBox dialog

class CAboutBox : public CDialog
{
    DECLARE_DYNAMIC(CAboutBox)

public:
    CAboutBox(CWnd* pParent = NULL);   // standard constructor
    virtual ~CAboutBox();

// Dialog Data
    enum { IDD = IDD_DIALOG_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CString m_strProgramTitle;
    CString m_strVersion;
    CStatic m_wndProgramTitle;
    CStatic m_wndVersion;
    virtual BOOL OnInitDialog();
    CFont font1;
    CFont font2;

    CStatic m_wndAuthor;
    CStatic m_wndCopyright;
    CStatic m_wndImage;
    CStatic m_wndCompiled;
    CString m_szCompiled;
};
