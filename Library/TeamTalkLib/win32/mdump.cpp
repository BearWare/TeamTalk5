#define _CRT_NON_CONFORMING_SWPRINTFS 1
#include <windows.h>
#include <assert.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "mdump.h"

#include <array>

LPCTSTR MiniDumper::m_szAppName;

MiniDumper::MiniDumper( LPCTSTR szAppName )
{
    // if this assert fires then you have two instances of MiniDumper
    // which is not allowed
    assert( m_szAppName==NULL );

    m_szAppName = szAppName ? _tcsdup(szAppName) : _T("Application");

    ::SetUnhandledExceptionFilter( TopLevelFilter );
}

LONG MiniDumper::TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
    LONG retval = EXCEPTION_CONTINUE_SEARCH;
    HWND hParent = NULL;                        // find a better value for your app

    // firstly see if dbghelp.dll is around and has the function we need
    // look next to the EXE first, as the one in System32 might be old 
    // (e.g. Windows 2000)
    HMODULE hDll = NULL;
    std::array<TCHAR, _MAX_PATH> szDbgHelpPath{};

    if (GetModuleFileName( NULL, szDbgHelpPath.data(), _MAX_PATH ))
    {
        TCHAR *pSlash = _tcsrchr( szDbgHelpPath.data(), '\\' );
        if (pSlash)
        {
            _tcscpy( pSlash+1, _T("DBGHELP.DLL") );
            hDll = ::LoadLibrary( szDbgHelpPath.data() );
        }
    }

    if (hDll==NULL)
    {
        // load any version we can
        hDll = ::LoadLibrary( _T("DBGHELP.DLL") );
    }

    LPCTSTR szResult = NULL;

    if (hDll)
    {
        MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
        if (pDump)
        {
            std::array<TCHAR, _MAX_PATH> szDumpPath{};
            std::array<TCHAR, _MAX_PATH> szScratch{};

            // work out a good place for the dump file
            if (!GetTempPath( _MAX_PATH, szDumpPath.data() ))
                _tcscpy( szDumpPath.data(), _T("c:\\temp\\") );

            _tcscat( szDumpPath.data(), m_szAppName );
            _tcscat( szDumpPath.data(), _T(".dmp") );

            // ask the user if they want to save a dump file
            if (::MessageBox( NULL, _T("A fatal error occurred. Would you like to save an error report? Please send the file to contact@bearware.dk"), m_szAppName, MB_YESNO )==IDYES)
            {
                // create the file
                HANDLE hFile = ::CreateFile( szDumpPath.data(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL, NULL );

                if (hFile!=INVALID_HANDLE_VALUE)
                {
                    _MINIDUMP_EXCEPTION_INFORMATION ExInfo;

                    ExInfo.ThreadId = ::GetCurrentThreadId();
                    ExInfo.ExceptionPointers = pExceptionInfo;
                    ExInfo.ClientPointers = NULL;

                    // write the dump
                    BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
                    if (bOK)
                    {
                        _stprintf( szScratch.data(), _T("Saved dump file to '%s'"), szDumpPath.data() );
                        szResult = szScratch.data();
                        retval = EXCEPTION_EXECUTE_HANDLER;
                    }
                    else
                    {
                        _stprintf( szScratch.data(), _T("Failed to save dump file to '%s' (error %d)"), szDumpPath.data(), GetLastError() );
                        szResult = szScratch.data();
                    }
                    ::CloseHandle(hFile);
                }
                else
                {
                    _stprintf( szScratch.data(), _T("Failed to create dump file '%s' (error %d)"), szDumpPath.data(), GetLastError() );
                    szResult = szScratch.data();
                }
            }
        }
        else
        {
            szResult = _T("DBGHELP.DLL too old");
        }
    }
    else
    {
        szResult = _T("DBGHELP.DLL not found");
    }

    if (szResult)
        ::MessageBox( NULL, szResult, m_szAppName, MB_OK );

    return retval;
}
