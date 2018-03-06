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

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        // main symbols

#define UNDEFINED -1

extern "C"
{
  typedef BOOL (WINAPI *LASTINPUT) (LASTINPUTINFO* pInput);

  DWORD GetLastInput();
}

BOOL IsWin2kPlus();

/* Message used for Inter-process comm. */

struct MsgCmdLine
{
    TCHAR        szPath[MAX_PATH];
};

class MyCommandLineInfo : public CCommandLineInfo
{
public:
    virtual void ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast );
public:
    CStringList m_args;
};

// CTeamTalkApp:
// See TeamTalk.cpp for the implementation of this class
//

class CTeamTalkApp : public CWinApp
{
public:
    CTeamTalkApp();

// Overrides
    public:
    virtual BOOL InitInstance();

// Implementation

    DECLARE_MESSAGE_MAP()
    virtual int ExitInstance();
};

extern CTeamTalkApp theApp;
