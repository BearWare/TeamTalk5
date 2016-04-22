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

#if !defined(HELPER_H)
#define HELPER_H

#include "settings/ClientXML.h"

typedef struct
{
    CString szFaceName;
    int nSize;
    BOOL bBold;
    BOOL bItalic;
    BOOL bUnderline;
} Font;


BOOL ConvertFont( const Font font, LOGFONT& destination);

BOOL FileExists(LPCTSTR szFileName);

BOOL VersionSameOrLater(const CString& szCheck, const CString& szAgainst);

#if defined(UNICODE) || defined(_UNICODE)
//Unicode to UTF-8 conversion and back
CString STR_UTF8(LPCSTR str, int max_str_len = TT_STRLEN);
CString STR_UTF8(const std::string& str);
std::string STR_UTF8(LPCWSTR str, int max_str_len = TT_STRLEN);
std::string STR_LOCAL(LPCWSTR str, int max_str_len = TT_STRLEN);
void COPYSTR(LPWSTR lpTarget, LPCSTR lpSource, int nLength);
void COPYSTR(LPSTR lpTarget, LPCWSTR lpSource, int nLength);

#define COPYTTSTR(dst,src)\
    do {\
    _tcsncpy(dst, src, TT_STRLEN);\
    } while(0)

#else
#define STR_UTF8
#define STR_LOCAL
#define COPYSTR strncpy
#endif

#define ZERO_STRUCT(a) memset(&a, 0, sizeof(a))

BOOL BrowseForFolder(HWND hwnd, LPCTSTR szCurrent, LPTSTR szPath);

void SetWindowNumber(CWnd& wnd, int nNumber);
int GetWindowNumber(CWnd& wnd);
void AddString(CComboBox& wnd, LPCTSTR szText, DWORD_PTR nItemData);
int GetItemData(CComboBox& wnd, BOOL* pError = NULL);
void SetCurSelItemData(CComboBox& wnd, DWORD_PTR nItemData);
void PlayWaveFile(LPCTSTR szFilePath);
CString LimitText(const CString& szName);
CString StripAmpersand(const CString& szText);
void UpdateAllowTransmitMenuItem(int nUserID, StreamTypes uStreamType, CCmdUI *pCmdUI);
CString GetDisplayName(const User& user);

CString GetLogTimeStamp();
CString GetLogFileName(LPCTSTR szFolder, LPCTSTR szName);
CString TrimForPath(LPCTSTR szPath);
BOOL OpenLogFile(CFile& file, LPCTSTR szFolder, LPCTSTR szName, CString& szLogFileName = CString());
void CloseLogFile(CFile& file);
void WriteLogMsg(CFile& file, LPCTSTR szMsg);
void SetAccessibleName(CWnd& wnd, LPCTSTR szHint);
#endif
