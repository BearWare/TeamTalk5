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
#include <Mmsystem.h>
#include <queue>
#include <WinInet.h>

extern TTInstance* ttInst;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;
using namespace teamtalk;

BOOL ConvertFont( const MyFont font, LOGFONT& destination)
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

CString GetExecutableFolder()
{
    // get folder of executing file
    CString path;
    GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH), MAX_PATH);
    return path.Left(path.ReverseFind('\\')+1);
}

BOOL DirectoryExists(LPCTSTR szPath)
{
    DWORD dwAttrib = GetFileAttributes(szPath);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
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
        int ret = WideCharToMultiByte(CP_UTF8, 0, str, -1, &buff[0], int(buff.size()-1), NULL, NULL);
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

DWORD_PTR GetItemData(CComboBox& wnd, DWORD_PTR dwDefault)
{
    int nCur = wnd.GetCurSel();
    if(nCur != CB_ERR)
        return wnd.GetItemData(nCur);
    return dwDefault;
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

DWORD_PTR GetItemData(CListBox& wnd, DWORD_PTR dwDefault)
{
    int nCur = wnd.GetCurSel();
    if(nCur != LB_ERR)
        return wnd.GetItemData(nCur);
    return dwDefault;
}

HTREEITEM GetItemDataItem(CTreeCtrl& wnd, DWORD_PTR dwItemData)
{
    HTREEITEM hItem = wnd.GetRootItem();
    HTREEITEM hResult = 0;
    std::queue<HTREEITEM> items;

    items.push(hItem);

    while(items.size() && hResult == 0)
    {
        hItem = items.front();
        items.pop();

        if(dwItemData == wnd.GetItemData(hItem))
        {
            hResult = hItem;
            break;
        }
        else if(wnd.ItemHasChildren(hItem))
            items.push(wnd.GetChildItem(hItem));
        else if(wnd.GetNextSiblingItem(hItem))
            items.push(wnd.GetNextSiblingItem(hItem));
    }
    return hResult;
}

void PlayWaveFile(LPCTSTR szFilePath, BOOL bAsync)
{
    ::PlaySound(szFilePath, NULL, SND_FILENAME | (bAsync ? SND_ASYNC : SND_SYNC));
    TRACE(_T("PLAY %s\n"), szFilePath);
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

CString ExtractMenuText(int nID, CString szText)
{
    TRANSLATE_ITEM(nID, szText);
    szText.Replace(_T("&"), _T(""));
    int i = szText.ReverseFind('\t');
    if(i >= 0)
        szText = szText.Left(i);
    return szText;
}

CString LoadText(int nID, CString szInitial)
{
    szInitial.LoadString(nID);
    TRANSLATE_ITEM(nID, szInitial);
    return szInitial;
}

void RemoveString(CStringList& strList, const CString& szStr)
{
    POSITION pos;
    while((pos = strList.Find(szStr)) != NULL)
        strList.RemoveAt(pos);
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

void UpdateAllowTransmitMenuItem(int nUserID, const Channel& chan, 
                                 StreamTypes uStreamType, CCmdUI *pCmdUI)
{
    BOOL b = FALSE;
    
    transmitusers_t users;
    b = (GetTransmitUsers(chan, users)[nUserID] & uStreamType);

    if (chan.uChannelType & CHANNEL_CLASSROOM)
    {
        pCmdUI->Enable(CanToggleTransmitUsers(chan.nChannelID));
        pCmdUI->SetCheck(b);
    }
    else
    {
        pCmdUI->Enable(CanToggleTransmitUsers(chan.nChannelID) && nUserID != TT_CLASSROOM_FREEFORALL);
        pCmdUI->SetCheck(!b && nUserID != TT_CLASSROOM_FREEFORALL);
    }
}

CString GetDisplayName(const User& user)
{
    if(bShowUsernames)
        return LimitText(user.szUsername);

    CString szNickname = LimitText(user.szNickname);
    if (szNickname.IsEmpty())
    {
        szNickname.Format(_T("%s - #%d"), LoadText(IDS_DEFAULTNICKNAME, DEFAULT_NICKNAME), user.nUserID);
    }
    return LimitText(szNickname);
}

BOOL EndsWith(const CString& szText, LPCTSTR szEnd, BOOL bCaseSensitive)
{
    return bCaseSensitive? szText.Right(int(_tcslen(szEnd))) == szEnd : szText.Right(int(_tcslen(szEnd))).CompareNoCase(szEnd) == 0;
}

BOOL StartsWith(const CString& szText, LPCTSTR szStart, BOOL bCaseSensitive)
{
    return bCaseSensitive ? szText.Left(int(_tcslen(szStart))) == szStart : szText.Left(int(_tcslen(szStart))).CompareNoCase(szStart) == 0;
}

BOOL IsWebLogin(const CString& szUsername)
{
    return szUsername == WEBLOGIN_BEARWARE_USERNAME || EndsWith(szUsername, _T(WEBLOGIN_BEARWARE_USERNAMEPOSTFIX));
}

CString URLEncode(const CString& szText)
{
    TCHAR szEncodedText[INTERNET_MAX_URL_LENGTH] = _T("");
    DWORD dwNewLen = INTERNET_MAX_URL_LENGTH;
    UrlEscape(szText, szEncodedText, &dwNewLen, URL_ESCAPE_PERCENT | URL_ESCAPE_AS_UTF8);
    return szEncodedText;
}

CString URLDecode(const CString& szUrlText)
{
    TCHAR szDecodedText[INTERNET_MAX_URL_LENGTH] = _T("");
    _tcsncpy(szDecodedText, szUrlText, INTERNET_MAX_URL_LENGTH);
    DWORD dwNewLen = INTERNET_MAX_URL_LENGTH;
    HRESULT hr = UrlUnescape(szDecodedText, szDecodedText, &dwNewLen, URL_UNESCAPE_INPLACE);
    return szDecodedText;
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

int GetSoundInputDevice(teamtalk::ClientXML& xmlSettings, SoundDevice* pSoundDev/* = NULL*/)
{
    int nInputDevice = xmlSettings.GetSoundInputDevice(-1);
    if(nInputDevice == -1)
        TT_GetDefaultSoundDevices(&nInputDevice, NULL);
    CString szInputDevice = STR_UTF8(xmlSettings.GetSoundInputDevice());
    SoundDevice dev;
    if(!pSoundDev)
        pSoundDev = &dev;
    if(GetSoundDevice(nInputDevice, szInputDevice, TRUE, *pSoundDev))
        return pSoundDev->nDeviceID;
    return nInputDevice;
}

int GetSoundOutputDevice(teamtalk::ClientXML& xmlSettings, SoundDevice* pSoundDev/* = NULL*/)
{
    int nOutputDevice = xmlSettings.GetSoundOutputDevice(-1);
    if(nOutputDevice == -1)
        TT_GetDefaultSoundDevices(NULL, &nOutputDevice);
    CString szOutputDevice = STR_UTF8(xmlSettings.GetSoundOutputDevice());
    SoundDevice dev;
    if(!pSoundDev)
        pSoundDev = &dev;
    if(GetSoundDevice(nOutputDevice, szOutputDevice, FALSE, *pSoundDev))
        return pSoundDev->nDeviceID;
    return nOutputDevice;
}

BOOL GetSoundDevice(int nSoundDeviceID, const CString& szDeviceID, BOOL bInput, SoundDevice& dev)
{
    int count = 25;
    std::vector<SoundDevice> devices(count);
    TT_GetSoundDevices(&devices[0], &count);
    if (count == 25)
    {
        TT_GetSoundDevices(NULL, &count);
        devices.resize(count);
        TT_GetSoundDevices(&devices[0], &count);
    }
    devices.resize(count);
    size_t i;
    for (i = 0; i < devices.size() && szDeviceID.GetLength(); i++)
    {
        if (devices[i].szDeviceID == szDeviceID &&
            ((bInput && devices[i].nMaxInputChannels > 0) || (!bInput && devices[i].nMaxOutputChannels > 0)))
        {
            dev = devices[i];
            return true;
        }
    }
    for (i = 0; i < devices.size(); i++)
    {
        if (devices[i].nDeviceID == nSoundDeviceID)
        {
            dev = devices[i];
            return true;
        }
    }
    return false;
}

int GetSoundDuplexSampleRate(const SoundDevice& indev, const SoundDevice& outdev)
{
    if ((indev.uSoundDeviceFeatures & SOUNDDEVICEFEATURE_DUPLEXMODE) == SOUNDDEVICEFEATURE_NONE)
        return 0;

    auto isend = indev.inputSampleRates + sizeof(indev.inputSampleRates);
    auto isr = std::find_if(indev.inputSampleRates, isend,
        [outdev](int sr)
        {
            return sr == outdev.nDefaultSampleRate && 
                   (outdev.uSoundDeviceFeatures & SOUNDDEVICEFEATURE_DUPLEXMODE) == SOUNDDEVICEFEATURE_DUPLEXMODE;
        });

    return isr != isend ? outdev.nDefaultSampleRate : 0;
}

BOOL IsSoundDeviceEchoCapable(const SoundDevice& indev, const SoundDevice& outdev)
{
    return GetSoundDuplexSampleRate(indev, outdev) > 0 || (indev.uSoundDeviceFeatures & SOUNDDEVICEFEATURE_AEC);
}

BOOL InitSoundSystem(teamtalk::ClientXML& xmlSettings, SoundDevice& indev, SoundDevice& outdev)
{
    TT_CloseSoundInputDevice(ttInst);
    TT_CloseSoundOutputDevice(ttInst);
    TT_CloseSoundDuplexDevices(ttInst);

    //Restart sound system so we have the latest sound devices
    TT_RestartSoundSystem();

    int nInputDevice = GetSoundInputDevice(xmlSettings, &indev);
    int nOutputDevice = GetSoundOutputDevice(xmlSettings, &outdev);

    BOOL bEchoCancel = xmlSettings.GetEchoCancel(DEFAULT_ECHO_ENABLE);
    BOOL bDuplex = GetSoundDuplexSampleRate(indev, outdev) > 0;

    SoundDeviceEffects effects = {};

    if (!bDuplex && bEchoCancel && (indev.uSoundDeviceFeatures & SOUNDDEVICEFEATURE_AEC))
        effects.bEnableEchoCancellation = bEchoCancel;
    TT_SetSoundDeviceEffects(ttInst, &effects);

    BOOL bSuccess = FALSE;

    if (bDuplex || (effects.bEnableEchoCancellation && (indev.nSoundSystem == SOUNDSYSTEM_WASAPI)))
    {
        bSuccess = TT_InitSoundDuplexDevices(ttInst, nInputDevice, nOutputDevice);
    }
    else
    {
        bSuccess = TT_InitSoundInputDevice(ttInst, nInputDevice) &&
            TT_InitSoundOutputDevice(ttInst, nOutputDevice);
    }

    if (!bSuccess)
    {
        TT_CloseSoundInputDevice(ttInst);
        TT_CloseSoundOutputDevice(ttInst);
        TT_CloseSoundDuplexDevices(ttInst);

        indev = {};
        outdev = {};

        if (TT_GetDefaultSoundDevices(&nInputDevice, &nOutputDevice))
        {
            if (TT_InitSoundInputDevice(ttInst, nInputDevice))
                GetSoundDevice(nInputDevice, _T(""), TRUE, indev);

            if (TT_InitSoundOutputDevice(ttInst, nOutputDevice))
                GetSoundDevice(nOutputDevice, _T(""), FALSE, outdev);
        }
    }
    return bSuccess;
}

extern BOOL g_bSpeech;

void AddTextToSpeechMessage(const CString& szMsg)
{
#if defined(ENABLE_TOLK)
    if(g_bSpeech)
        Tolk_Output(szMsg);
#endif
}
