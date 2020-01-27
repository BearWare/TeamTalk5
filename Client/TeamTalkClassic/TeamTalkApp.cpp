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
#include "TeamTalkApp.h"
#include "TeamTalkDlg.h"
#include "AppInfo.h"
#include "License.h"
#include <VersionHelpers.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////
/// C functions for libraries

extern "C"
{

    LASTINPUT GetLastInputInfo_ = NULL;

    DWORD GetLastInput()
    {
        DWORD dwResult = 0;

        if(GetLastInputInfo_)
        {
            LASTINPUTINFO info;
            memset(&info, 0, sizeof(info));
            info.cbSize = sizeof(LASTINPUTINFO);
            if( GetLastInputInfo_(&info))
                dwResult = info.dwTime;
            else
            {
                ASSERT(FALSE);
                dwResult = ::GetTickCount();
            }
        }
        return dwResult;
    }

}

///////////////////////////////////////////////////////////////////////
// helper functions for IPC

typedef struct
{
    HWND    hwnd;
    LPCTSTR    title;
} FindWnd;


BOOL CALLBACK CheckWindowTitle( HWND hwnd, LPARAM lParam )
{
    TCHAR    buffer[MAX_PATH];
    GetWindowText( hwnd, buffer, MAX_PATH );

    FindWnd * fw = (FindWnd *)lParam;
    if( _tcsncmp( buffer, fw->title, MAX_PATH ) == 0 )
    {
        fw->hwnd = hwnd;
        return FALSE;
    }
    return TRUE;
}


HWND FindWinTitle( LPCTSTR title )
{
    FindWnd    fw;
    fw.hwnd = 0;
    fw.title = title;

    EnumWindows( (WNDENUMPROC) CheckWindowTitle, (LPARAM) &fw );
    return fw.hwnd;
}

BOOL IsWin2kPlus()
{
    //detect windows version
    return IsWindowsXPOrGreater();
}

void MyCommandLineInfo::ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast )
{
    bFlag = bFlag;
    m_args.AddTail(lpszParam);
}

// CTeamTalkApp

BEGIN_MESSAGE_MAP(CTeamTalkApp, CWinApp)
    //ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CTeamTalkApp construction

CTeamTalkApp::CTeamTalkApp()
{
    //EnableHtmlHelp();

    // Place all significant initialization in InitInstance
}


// The one and only CTeamTalkApp object

CTeamTalkApp theApp;

const GUID CDECL BASED_CODE _tlid =
{ 0xBC4E1CC0, 0x8B80, 0x4BD4, { 0x85, 0xC3, 0xD1, 0xAC, 0x38, 0x6A, 0x29, 0xB8 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


// CTeamTalkApp initialization

BOOL CTeamTalkApp::InitInstance()
{
	AfxOleInit();
#if defined(USE_MINIDUMP)
    MiniDumper dump( _T("TeamTalk") );
#endif
    // InitCommonControls() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    InitCommonControls();

    CWinApp::InitInstance();

    AfxEnableControlContainer();

    HMODULE hCoreMod = 0;
    // Load keyboard hook
    if(IsWin2kPlus())
    {
        hCoreMod = LoadLibrary(_T("USER32.dll"));
        GetLastInputInfo_ = (LASTINPUT)GetProcAddress(hCoreMod, "GetLastInputInfo");
        ASSERT(GetLastInputInfo_);
    }

    //load richedit
    if(!AfxInitRichEdit2( ))
        AfxMessageBox(_T("Failed to initialize RichEdit component"));

    SetCurrentDirectory(GetExecutableFolder());

    /* Set license information before creating the first client instance */
    TT_SetLicenseInformation(REGISTRATION_NAME, REGISTRATION_KEY);

    // check whether an existing instance of TT is running an whether
    // this instance has been passed a .tt file
    HWND hRunningTT = FindWinTitle(APPTITLE);

    MyCommandLineInfo info;
    ParseCommandLine(info);

    CString szArg;
    //trim command line params for "
    if(hRunningTT && info.m_args.GetCount() == 1)
    {
        BOOL bTTUrl = FALSE;
        MsgCmdLine msg = {};
        for(POSITION pos = info.m_args.GetHeadPosition(); pos != NULL;)
        {
            szArg = info.m_args.GetNext(pos);
            _tcsncat(msg.szPath, szArg.GetBuffer(), MAX_PATH);
            _tcsncat(msg.szPath, _T("¤"), MAX_PATH);
            bTTUrl |= StartsWith(szArg, TTURL, FALSE);
            bTTUrl |= EndsWith(szArg, _T( TTFILE_EXT ), FALSE);
        }

        if(bTTUrl)
        {
            COPYDATASTRUCT    cds;
            cds.dwData = 0;
            cds.cbData = sizeof(msg);
            cds.lpData = &msg;
            ::SendMessage(hRunningTT, WM_COPYDATA, 0, (LPARAM)&cds);
            return FALSE;
        }
    }

    szArg.Empty();
    CTeamTalkDlg dlg;
    dlg.m_cmdArgs.AddHead(&info.m_args);

    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK)
    {
        //  dismissed with OK
    }
    else if (nResponse == IDCANCEL)
    {
        //  dismissed with Cancel
    }

    if(hCoreMod)
        FreeLibrary(hCoreMod);
    hCoreMod = 0;
    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}

int CTeamTalkApp::ExitInstance()
{
    return CWinApp::ExitInstance();
}
