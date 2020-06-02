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
} MyFont;

BOOL ConvertFont(const MyFont font, LOGFONT& destination);

BOOL FileExists(LPCTSTR szFileName);
CString GetExecutableFolder();
BOOL DirectoryExists(LPCTSTR szPath);

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
DWORD_PTR GetItemData(CComboBox& wnd, DWORD_PTR dwDefault);
void SetCurSelItemData(CComboBox& wnd, DWORD_PTR nItemData);
DWORD_PTR GetItemData(CListBox& wnd, DWORD_PTR dwDefault);
HTREEITEM GetItemDataItem(CTreeCtrl& wnd, DWORD_PTR dwItemData);
void PlayWaveFile(LPCTSTR szFilePath, BOOL bAsync);
CString LimitText(const CString& szName);
CString StripAmpersand(const CString& szText);
CString ExtractMenuText(int nID, CString szText);
CString LoadText(int nID, CString szInitial = _T(""));
void RemoveString(CStringList& strList, const CString& szStr);
void UpdateAllowTransmitMenuItem(int nUserID, const Channel& chan, StreamTypes uStreamType, CCmdUI *pCmdUI);
CString GetDisplayName(const User& user);
BOOL EndsWith(const CString& szText, LPCTSTR szEnd, BOOL bCaseSensitive = TRUE);
BOOL StartsWith(const CString& szText, LPCTSTR szStart, BOOL bCaseSensitive = TRUE);

CString GetLogTimeStamp();
CString GetLogFileName(LPCTSTR szFolder, LPCTSTR szName);
CString TrimForPath(LPCTSTR szPath);
BOOL OpenLogFile(CFile& file, LPCTSTR szFolder, LPCTSTR szName, CString& szLogFileName = CString());
void CloseLogFile(CFile& file);
void WriteLogMsg(CFile& file, LPCTSTR szMsg);
void SetAccessibleName(CWnd& wnd, LPCTSTR szHint);

int GetSoundInputDevice(teamtalk::ClientXML& xmlSettings, SoundDevice* pSoundDev = NULL);
int GetSoundOutputDevice(teamtalk::ClientXML& xmlSettings, SoundDevice* pSoundDev = NULL);
BOOL InitSoundSystem(teamtalk::ClientXML& xmlSettings);

#endif
