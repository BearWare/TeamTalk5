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

#if !defined(UNDER_CE)
//need x-button
#undef _WIN32_WINNT
#undef VWINVER

#define _WIN32_WINNT 0x0501
#define VWINVER 0x0501
#endif

#include <ace/OS.h>
#include <ace/Thread_Manager.h>
#include <myace/MyACE.h>
#include "HotKey.h"
#include <memory>
#include <set>
#include <assert.h>
#include <algorithm>

using namespace std;

#if defined(UNDER_CE)

/// LRESULT CALLBACK LowLevelKeyHookProc(int nCode, WPARAM wParam, LPARAM lParam);
typedef LRESULT (CALLBACK* HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);

/// HHOOK SetWindowsHookEx(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId);
typedef HHOOK (*WINHOOK) (int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId);

/// WINUSERAPI BOOL WINAPI UnhookWindowsHookEx(IN HHOOK hhk);
typedef WINUSERAPI BOOL (WINAPI* UNHOOK) (IN HHOOK hhk);

/// WINUSERAPI LRESULT WINAPI CallNextHookEx(IN HHOOK hhk, IN int nCode, IN WPARAM wParam, IN LPARAM lParam);
typedef WINUSERAPI LRESULT (WINAPI* CALLNEXT) (IN HHOOK hhk, IN int nCode, IN WPARAM wParam, IN LPARAM lParam);

#define WH_KEYBOARD_LL     20

typedef struct tagKBDLLHOOKSTRUCT {
    DWORD   vkCode;
    DWORD   scanCode;
    DWORD   flags;
    DWORD   time;
    ULONG_PTR dwExtraInfo;
} KBDLLHOOKSTRUCT, FAR *LPKBDLLHOOKSTRUCT, *PKBDLLHOOKSTRUCT;

WINHOOK SetWindowsHookEx = NULL;
UNHOOK UnhookWindowsHookEx = NULL;
CALLNEXT CallNextHookEx = NULL;

#endif

HotKey* HotKey::Instance()
{
    static HotKey hotkey;
    return &hotkey;
}

HotKey::HotKey()
{
}

HotKey::~HotKey()
{
}

void HotKey::RegisterHotKey(const HotKeyHook& hotkey)
{
    assert(hotkey.listener);
    for(size_t i=0;i<m_hotkeys.size();i++)
    {
        if(    m_hotkeys[i].hotkeyid == hotkey.hotkeyid && 
            m_hotkeys[i].listener == hotkey.listener)
            UnregisterHotKey(hotkey.listener, hotkey.hotkeyid);
    }
    m_hotkeys.push_back(hotkey);
}

void HotKey::UnregisterHotKey(HotKeyListener* listener, int hotkeyid)
{
    for(size_t i=0;i<m_hotkeys.size();)
    {
        if(m_hotkeys[i].listener == listener && m_hotkeys[i].hotkeyid == hotkeyid)
            m_hotkeys.erase(m_hotkeys.begin()+i);
        else i++;
    }
}

void HotKey::KeyToggle(UINT vkcode, BOOL keydown)
{
    if(keydown)
    {
        m_ActiveKeys.insert(vkcode);

        for(size_t i=0;i<m_hotkeys.size();i++)
        {
            if(!m_hotkeys[i].active)
            {
                intset_t res;
                set_intersection(m_hotkeys[i].keys.begin(),
                                 m_hotkeys[i].keys.end(),
                                 m_ActiveKeys.begin(),
                                 m_ActiveKeys.end(),
                                 std::inserter(res, res.begin()));
                assert(m_hotkeys[i].listener);
                if(res.size() == m_hotkeys[i].keys.size())
                {
                    m_hotkeys[i].listener->OnHotKeyActive(m_hotkeys[i].hotkeyid);
                    m_hotkeys[i].active = true;
                    MYTRACE(ACE_TEXT("HK Active %d\n"), m_hotkeys[i].hotkeyid);
                }
            }
        }

        if(m_keyTesters.size())
        {
            keytesters_t::iterator ite;
            for(ite=m_keyTesters.begin();ite!=m_keyTesters.end();ite++)
                (*ite)->OnKeyDown(vkcode);
        }
    }
    else
    {
        m_ActiveKeys.erase(vkcode);

        for(size_t i=0;i<m_hotkeys.size();i++)
        {
            assert(m_hotkeys[i].listener);
            if(m_hotkeys[i].active && 
                m_hotkeys[i].keys.find(vkcode) != m_hotkeys[i].keys.end())
            {
                MYTRACE(ACE_TEXT("HK Inactive %d\n"), m_hotkeys[i].hotkeyid);
                m_hotkeys[i].listener->OnHotKeyInactive(m_hotkeys[i].hotkeyid);
                m_hotkeys[i].active = false;
            }
        }

        if(m_keyTesters.size())
        {
            keytesters_t::iterator ite;
            for(ite=m_keyTesters.begin();ite!=m_keyTesters.end();ite++)
                (*ite)->OnKeyUp(vkcode);
        }
    }
}

int HotKey::IsHotKeyActive(HotKeyListener* listener, int hotkeyid)
{
    for(size_t i=0;i<m_hotkeys.size();i++)
    {
        if(m_hotkeys[i].listener == listener &&
            m_hotkeys[i].hotkeyid == hotkeyid)
            return m_hotkeys[i].active;
    }
    return -1;
}

void HotKey::AddKeyTester(HotKeyListener* listener)
{
    assert(listener);
    m_keyTesters.insert(listener);
}

void HotKey::RemoveKeyTester(HotKeyListener* listener)
{
    assert(listener);
    m_keyTesters.erase(listener);
}

void HotKey::ClearAll(HotKeyListener* listener)
{
    RemoveKeyTester(listener);
    for(size_t i=0;i<m_hotkeys.size();)
    {
        if(m_hotkeys[i].listener == listener)
            m_hotkeys.erase(m_hotkeys.begin()+i);
        else i++;
    }
}

BOOL HotKey::HotKeyExists(HotKeyListener* listener, int hotkeyid)
{
    for(size_t i=0;i<m_hotkeys.size();i++)
    {
        if(m_hotkeys[i].listener == listener &&
            m_hotkeys[i].hotkeyid == hotkeyid)
            return TRUE;
    }
    return FALSE;
}



//For hotkey hooks
HHOOK hKeyHook = NULL;
HHOOK hMouseHook = NULL;



//////////////////////////////////////////
/// HotKeys
//////////////////////////////////////////

LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
#if !defined(UNDER_CE)
    if(nCode == HC_ACTION)
    {
        if((lParam & 0x80000000)) //a key is up
        {
            HOTKEY->KeyToggle((UINT)wParam, FALSE);
        }
        else if(!(0x40000000 & lParam) && !(lParam & 0x80000000)) //key is down
        {
            HOTKEY->KeyToggle((UINT)wParam, TRUE);
        }
    }
#else
    assert(0);
#endif
    return ( CallNextHookEx(hKeyHook, nCode, wParam, lParam) );//pass control to next hook in the hook chain.
}

/* low level key hook for win2k - xp */

std::set<DWORD> activekeys;

LRESULT CALLBACK LowLevelKeyHookProc(  int nCode,     // hook code
                                                                         WPARAM wParam, // message identifier
                                                                         LPARAM lParam  // message data
                                                                         )
{
    if(wParam == WM_KEYDOWN || wParam == WM_KEYUP || wParam == WM_SYSKEYDOWN || wParam == WM_SYSKEYUP)
    {
        KBDLLHOOKSTRUCT* lp = reinterpret_cast<KBDLLHOOKSTRUCT*> (lParam);
        LPARAM code = lp->scanCode;
        std::set<DWORD>::iterator ite = activekeys.find(lp->vkCode);
#if !defined(UNDER_CE)
        if(lp->flags & 0x80)
        {
            if(ite != activekeys.end())
            {
                activekeys.erase(lp->vkCode);
                HOTKEY->KeyToggle(lp->vkCode, FALSE);
            }
        }
        else
        {
            if(ite == activekeys.end())
            {
                activekeys.insert(lp->vkCode);
                HOTKEY->KeyToggle(lp->vkCode, TRUE);
            }
        }
#else
        if(wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
        {
            if(ite != activekeys.end())
            {
                activekeys.erase(lp->vkCode);
                HOTKEY->KeyToggle(lp->vkCode, FALSE);
            }
        }
        else
        {
            if(ite == activekeys.end())
            {
                activekeys.insert(lp->vkCode);
                HOTKEY->KeyToggle(lp->vkCode, TRUE);
            }
        }
#endif
    }

    return CallNextHookEx(hKeyHook, nCode, wParam, lParam);
}

std::set<DWORD> activebuttons;

LRESULT CALLBACK LowLevelMouseHookProc(  int nCode,     // hook code
                                                                             WPARAM wParam, // message identifier
                                                                             LPARAM lParam  // message data
                                                                             )
{
#if !defined(UNDER_CE)
    if(nCode == HC_ACTION)
    {
        if( wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONUP ||
            wParam == WM_RBUTTONDOWN || wParam == WM_RBUTTONUP ||
            wParam == WM_MBUTTONDOWN || wParam == WM_MBUTTONUP ||
            wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONUP)
        {
            DWORD dwBtn = 0;
            if(wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONUP)
                dwBtn = VK_LBUTTON;
            else if(wParam == WM_RBUTTONDOWN || wParam == WM_RBUTTONUP)
                dwBtn = VK_RBUTTON;
            else if(wParam == WM_MBUTTONDOWN || wParam == WM_MBUTTONUP)
                dwBtn = VK_MBUTTON;
            else if(wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONUP)
                dwBtn = VK_XBUTTON1;

            MSLLHOOKSTRUCT* lp = reinterpret_cast<MSLLHOOKSTRUCT*> (lParam);
            std::set<DWORD>::iterator ite = activebuttons.find(dwBtn);
            if(wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || wParam == WM_MBUTTONUP || wParam == WM_XBUTTONUP)
            {
                if(ite != activebuttons.end())
                {
                    activebuttons.erase(dwBtn);
                    HOTKEY->KeyToggle(dwBtn, FALSE);
                }
            }
            else
            {
                if(ite == activebuttons.end())
                {
                    activebuttons.insert(dwBtn);
                    HOTKEY->KeyToggle(dwBtn, TRUE);
                }
            }
        }
    }
#else
    assert(0);
#endif
    return CallNextHookEx(hMouseHook,nCode,wParam,lParam);
}

struct HookInfo
{
    HINSTANCE hInstance;
    BOOL bLowLevel;
    int nResult;
};

static ACE_thread_t dwHookThreadID = 0;

static ACE_THR_FUNC_RETURN hook_loop (void *arg)
{
    HookInfo* hook = static_cast<HookInfo*> (arg);

#if !defined(UNDER_CE)
    hKeyHook = SetWindowsHookEx( WH_KEYBOARD_LL,
        LowLevelKeyHookProc,
        hook->hInstance,
        NULL);
    assert(hKeyHook);

    int nMouseHook = hook->bLowLevel? WH_MOUSE_LL : WH_MOUSE;
    hMouseHook = SetWindowsHookEx( nMouseHook,
        LowLevelMouseHookProc,
        hook->hInstance,
        NULL);
    assert(hMouseHook);
#endif

    hook->nResult = hMouseHook && hKeyHook;

    MSG msg = {};
    while(GetMessage(&msg, NULL, NULL, NULL) && msg.message != WM_QUIT)
        return 0;

    return 1;
}


BOOL InstallHook(HINSTANCE hInstance, BOOL bLLMouseHook)
{
    assert(dwHookThreadID == 0);
    if(dwHookThreadID)
        return FALSE;

    //detect windows version
    OSVERSIONINFO version;
    version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&version);

    if(version.dwMajorVersion >= 5 
#if defined(UNDER_CE)
        && version.dwPlatformId != VER_PLATFORM_WIN32_CE
#endif
        )    //win2K/XP
    {
        HookInfo info;
        info.hInstance = hInstance;
        info.bLowLevel = bLLMouseHook;
        info.nResult = -1;

        if(ACE_Thread_Manager::instance ()->spawn(hook_loop, &info, THR_NEW_LWP|THR_JOINABLE|THR_INHERIT_SCHED,&dwHookThreadID)<0)
            return false;

        while(info.nResult == -1)Sleep(10);

        //  hKeyHook = SetWindowsHookEx( WH_KEYBOARD_LL,
        //    LowLevelKeyHookProc,
        //    hInstance,
        //    NULL);
        //assert(hKeyHook);

        //int nMouseHook = bLLMouseHook? WH_MOUSE_LL : WH_MOUSE;
        //hMouseHook = SetWindowsHookEx( nMouseHook,
        //    LowLevelMouseHookProc,
        //    hInstance,
        //    NULL);
        //assert(hMouseHook);
    }
    else
    {
#if defined(UNDER_CE)
        HMODULE hCoreMod = NULL;
        hCoreMod = LoadLibrary(_T("coredll.dll"));
        if(hCoreMod)
        {
            SetWindowsHookEx = (WINHOOK)GetProcAddress(hCoreMod, _T("SetWindowsHookExW"));
            assert(SetWindowsHookEx);
            UnhookWindowsHookEx = (UNHOOK)GetProcAddress(hCoreMod, _T("UnhookWindowsHookEx"));
            assert(UnhookWindowsHookEx);
            CallNextHookEx = (CALLNEXT)GetProcAddress(hCoreMod, _T("CallNextHookEx"));

            hKeyHook = SetWindowsHookEx( WH_KEYBOARD_LL,
                LowLevelKeyHookProc,
                hInstance,
                NULL);
            assert(hKeyHook);
            return hKeyHook != NULL;
        }
#else
        hKeyHook = SetWindowsHookEx( WH_KEYBOARD,
            KeyHookProc,
            hInstance,
            NULL);
        assert(hKeyHook);

        hMouseHook = SetWindowsHookEx( WH_MOUSE,
            LowLevelMouseHookProc,
            hInstance,
            NULL);
        assert(hMouseHook);
#endif
    }
    return hKeyHook != NULL && hMouseHook != NULL;
}

void RemoveHook()
{
    if(dwHookThreadID)
    {
        BOOL bRet = PostThreadMessage(dwHookThreadID, WM_QUIT, 0, 0);
        assert(bRet);
        dwHookThreadID = 0;
    }

    if(hMouseHook)
        UnhookWindowsHookEx(hMouseHook);
    hMouseHook = NULL;
    if(hKeyHook)
        UnhookWindowsHookEx(hKeyHook);
    hKeyHook = NULL;
}

