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

#include "stdafx.h"
#include <Mmsystem.h>

extern TTInstance* ttInst;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;
using namespace teamtalk;

BOOL ConvertFont( const Font font, LOGFONT& destination)
{
    if( font.szFaceName.IsEmpty() || font.nSize <= 0)
        return FALSE;

    memset(&destination, 0, sizeof(LOGFONT));
    _tcscpy(destination.lfFaceName, font.szFaceName);
    destination.lfItalic = font.bItalic;
    destination.lfUnderline = font.bUnderline;
    if( font.bBold )
        destination.lfWeight = destination.lfWeight | FW_BOLD;

    HDC hdc = ::GetDC(NULL);
    POINT pt;
    pt.y = MulDiv(::GetDeviceCaps(hdc, LOGPIXELSY), font.nSize, 72);
    ::DPtoLP(hdc, &pt, 1);
    destination.lfHeight = -pt.y;
    ::ReleaseDC(NULL, hdc);
    return TRUE;
}

BOOL FileExists(LPCTSTR szFileName)
{
    FILE* pFile = _tfopen(szFileName, _T("r"));
    if(pFile == NULL)
        return FALSE;
    else
    {
        fclose(pFile);
        return TRUE;
    }
}

BOOL VersionSameOrLater(const CString& szCheck, const CString& szAgainst)
{
    if(szCheck == szAgainst) return true;

    int i = 0;
    CStringList chk_tokens;
    CString szToken = szCheck.Tokenize(_T("."), i);
    while(!szToken.IsEmpty())
    {
        chk_tokens.AddTail(szToken);
        szToken = szCheck.Tokenize(_T("."), i);
    }

    i = 0;
    CStringList against_tokens;
    szToken = szAgainst.Tokenize(_T("."), i);
    while(!szToken.IsEmpty())
    {
        against_tokens.AddTail(szToken);
        szToken = szAgainst.Tokenize(_T("."), i);
    }

    vector<int> vec_chk, vec_against;
    for(POSITION pos = chk_tokens.GetHeadPosition();pos != NULL;)
        vec_chk.push_back(_ttoi(chk_tokens.GetNext(pos)));
    for(POSITION pos = against_tokens.GetHeadPosition();pos != NULL;)
        vec_against.push_back(_ttoi(against_tokens.GetNext(pos)));

    size_t less = vec_chk.size() < vec_against.size()?vec_chk.size():vec_against.size();
    
    for(size_t i=0;i<less;i++)
        if(vec_chk[i] < vec_against[i])
            return false;
        else if(vec_chk[i] > vec_against[i])
            return true;

    return true;
}

#if defined(UNICODE) || defined(_UNICODE)
CString STR_UTF8(LPCSTR str, int max_str_len/* = TT_STRLEN*/)
{
    ASSERT(str);
    wstring buff;
    buff.resize(max_str_len);
    if(buff.size())
    {
        int ret = MultiByteToWideChar(CP_UTF8, 0, str, -1, &buff[0], TT_STRLEN);
        //int ret = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str, -1, buff, TT_STRLEN);
        //if(ret == 0 && GetLastError() == ERROR_NO_UNICODE_TRANSLATION)
        //    mbstowcs(buff, str, TT_STRLEN); 
    }
    return buff.c_str();
}

CString STR_UTF8(const std::string& str) { return STR_UTF8(str.c_str()); }

std::string STR_UTF8(LPCWSTR str, int max_str_len/* = TT_STRLEN*/)
{
    ASSERT(str);
    string buff;
    buff.resize(max_str_len);
    if(buff.size())
    {
        int ret = WideCharToMultiByte(CP_UTF8, 0, str, -1, &buff[0], buff.size()-1, NULL, NULL);
        ASSERT(ret>0);
    }
    return buff.c_str();
}

std::string STR_LOCAL(LPCWSTR str, int max_str_len/* = TT_STRLEN*/)
{
    ASSERT(str);
    string buff;
    buff.resize(max_str_len);
    if(buff.size())
        wcstombs(&buff[0], str, buff.size()-1);
    return buff.c_str();
}

void COPYSTR(LPWSTR lpTarget, LPCSTR lpSource, int nLength)
{
    ASSERT(lpTarget);
    ASSERT(lpSource);
    wcsncpy(lpTarget, STR_UTF8(lpSource).GetBuffer(), nLength);
}

void COPYSTR(LPSTR lpTarget, LPCWSTR lpSource, int nLength)
{
    ASSERT(lpTarget);
    ASSERT(lpSource);
    strncpy(lpTarget, STR_UTF8(lpSource).c_str(), nLength);
}

#endif

static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	// If the BFFM_INITIALIZED message is received
	// set the path to the start path.
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
		{
			if (NULL != lpData)
			{
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
			}
		}
	}

	return 0; // The function should always return 0.
}

// HWND is the parent window.
// szCurrent is an optional start folder. Can be NULL.
// szPath receives the selected path on success. Must be MAX_PATH characters in length.
BOOL BrowseForFolder(HWND hwnd, LPCTSTR szCurrent, LPTSTR szPath)
{
	BROWSEINFO   bi = { 0 };
	LPITEMIDLIST pidl;
	TCHAR        szDisplay[MAX_PATH];
	BOOL         retval;

	//CoInitialize(NULL);

	bi.hwndOwner      = hwnd;
	bi.pszDisplayName = szDisplay;
	bi.lpszTitle      = TEXT("Please choose a folder.");
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn           = BrowseCallbackProc;
	bi.lParam         = (LPARAM) szCurrent;

	pidl = SHBrowseForFolder(&bi);

	if (NULL != pidl)
	{
		retval = SHGetPathFromIDList(pidl, szPath);
		CoTaskMemFree(pidl);
	}
	else
	{
		retval = FALSE;
	}

	if (!retval)
	{
		szPath[0] = TEXT('\0');
	}

	//CoUninitialize();
	return retval;
}

void SetWindowNumber(CWnd& wnd, int nNumber)
{
    CString s;
    s.Format(_T("%d"), nNumber);
    wnd.SetWindowText(s);
}

int GetWindowNumber(CWnd& wnd)
{
    CString s;
    wnd.GetWindowText(s);
    return _ttoi(s);
}

void AddString(CComboBox& wnd, LPCTSTR szText, DWORD_PTR nItemData)
{
    int nIndex = wnd.AddString(szText);
    wnd.SetItemData(nIndex, nItemData);
}

int GetItemData(CComboBox& wnd, BOOL* pError)
{
    int nCur = wnd.GetCurSel();
    if(pError)
        *pError = nCur>=0;
    if(nCur>=0)
        return wnd.GetItemData(nCur);
    return 0;
}

void SetCurSelItemData(CComboBox& wnd, DWORD_PTR nItemData)
{
    int nItems = wnd.GetCount();
    for(int i=0;i<nItems;i++)
    {
        if(wnd.GetItemData(i) == nItemData)
        {
            wnd.SetCurSel(i);
            return;
        }
    }
}

void PlayWaveFile(LPCTSTR szFilePath)
{
    if(szFilePath && *szFilePath && FileExists( szFilePath ))
        ::PlaySound(szFilePath, NULL, SND_FILENAME | SND_ASYNC);
}

int nTextLimit = TT_STRLEN;
BOOL bShowUsernames = FALSE;

CString LimitText(const CString& szName)
{
    if(szName.GetLength() <= nTextLimit)
        return szName;
    return szName.Left(nTextLimit) + _T("...");
}

CString StripAmpersand(const CString& szText)
{
    CString szResult = szText;
    szResult.Replace(_T("&"), _T(""));
    return szResult;
}

CString GetLogTimeStamp()
{
    CTime tm = CTime::GetCurrentTime();
    CString szTime;
    szTime.Format(_T("%.4d%.2d%.2d-%.2d%.2d%.2d"), tm.GetYear(), tm.GetMonth(),
                  tm.GetDay(), tm.GetHour(), tm.GetMinute(), tm.GetSecond());
    return szTime;
}

CString GetLogFileName(LPCTSTR szFolder, LPCTSTR szName)
{
    CString szPath = szFolder;
    szPath.Format(_T("%s\\%s - %s"), szFolder, GetLogTimeStamp(),
                  TrimForPath(szName).GetBuffer());
    return szPath;
}

CString TrimForPath(LPCTSTR szPath)
{
    CString szTmp = szPath;
    szTmp.Trim(_T("\\/:?\"<>|"));
    return szTmp;
}

BOOL OpenLogFile(CFile& file, LPCTSTR szFolder, LPCTSTR szName, CString& szLogFileName)
{
    CloseLogFile(file);

    CString szFileName = GetLogFileName(szFolder, szName);
    szLogFileName = szFileName;
    return file.Open(szFileName,
                     CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate);
}

void CloseLogFile(CFile& file)
{
    if(file.m_hFile != CFile::hFileNull)
    {
        CString szFileName = file.GetFilePath();
        ULONGLONG uSize = file.GetLength();
        file.Close();
        if(uSize == 0)
            CFile::Remove(szFileName);
    }
}

void WriteLogMsg(CFile& file, LPCTSTR szMsg)
{
    if(file.m_hFile == CFile::hFileNull)
        return;

    std::string utf8 = STR_UTF8(szMsg);
    file.Write(utf8.c_str(), (UINT)utf8.size());
}

void UpdateAllowTransmitMenuItem(int nUserID, StreamTypes uStreamType, CCmdUI *pCmdUI)
{
    User user;
    Channel chan;
    BOOL b = FALSE;
    if(TT_GetUser(ttInst, nUserID, &user) && 
       TT_GetChannel(ttInst, user.nChannelID, &chan) && 
       (chan.uChannelType & CHANNEL_CLASSROOM) )
    {
        transmitusers_t users;
        b = (GetTransmitUsers(chan, users)[nUserID] & uStreamType); 
        pCmdUI->Enable(CanToggleTransmitUsers(user.nChannelID));
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
    pCmdUI->SetCheck(b);
}

CString GetDisplayName(const User& user)
{
    if(bShowUsernames)
        return LimitText(user.szUsername);
    return LimitText(user.szNickname);
}

// The horror... initguid.h must be included before oleacc.h but oleacc.h is included
// by afxwin.h which has to be the first include file in a MFC project...
#undef INITGUID
#include <InitGuid.h>
DEFINE_GUID(CLSID_AccPropServices,   0xb5f8350b, 0x0548, 0x48b1, 0xa6, 0xee, 0x88, 0xbd, 0x00, 0xb4, 0xa5, 0xe7);
DEFINE_GUID( PROPID_ACC_NAME             , 0x608d3df8, 0x8128, 0x4aa7, 0xa4, 0x28, 0xf5, 0x5e, 0x49, 0x26, 0x72, 0x91);

void SetAccessibleName(CWnd& wnd, LPCTSTR szHint)
{
    // COM is assumed to be initialized...
    IAccPropServices* pAccPropServices = NULL;
    HRESULT hr = CoCreateInstance(CLSID_AccPropServices,
        NULL, CLSCTX_SERVER, IID_IAccPropServices, 
        (void**)&pAccPropServices);

    if (SUCCEEDED(hr))
    {
        hr = pAccPropServices->SetHwndPropStr(wnd, OBJID_CLIENT, CHILDID_SELF, PROPID_ACC_NAME, szHint);
        pAccPropServices->Release();
    }
}
