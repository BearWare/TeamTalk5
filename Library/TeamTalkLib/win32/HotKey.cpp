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

#include "HotKey.h"

#include "myace/MyACE.h"

#include <ace/OS_NS_Thread.h>
#include <ace/Thread_Manager.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <set>

using namespace std;

HotKey *HotKey::Instance()
{
    static HotKey hotkey;
    return &hotkey;
}

HotKey::HotKey() = default;

HotKey::~HotKey() = default;

void HotKey::RegisterHotKey(const HotKeyHook& hotkey)
{
    assert(hotkey.listener);
    for(auto & m_hotkey : m_hotkeys)
    {
        if(    m_hotkey.hotkeyid == hotkey.hotkeyid && 
            m_hotkey.listener == hotkey.listener)
            UnregisterHotKey(hotkey.listener, hotkey.hotkeyid);
    }
    m_hotkeys.push_back(hotkey);
}

void HotKey::UnregisterHotKey(HotKeyListener* listener, int hotkeyid)
{
    for(size_t i=0;i<m_hotkeys.size();)
    {
        if (m_hotkeys[i].listener == listener && m_hotkeys[i].hotkeyid == hotkeyid)
            m_hotkeys.erase(m_hotkeys.begin() + i);
        else
            i++;
    }
}

void HotKey::KeyToggle(UINT vkcode, BOOL keydown)
{
    if(keydown != 0)
    {
        m_ActiveKeys.insert(vkcode);

        for (auto &m_hotkey : m_hotkeys) {
            if (!m_hotkey.active) {
                intset_t res;
                set_intersection(m_hotkey.keys.begin(),
                                 m_hotkey.keys.end(),
                                 m_ActiveKeys.begin(),
                                 m_ActiveKeys.end(),
                                 std::inserter(res, res.begin()));
                assert(m_hotkey.listener);
                if (res.size() == m_hotkey.keys.size()) {
                    m_hotkey.listener->OnHotKeyActive(m_hotkey.hotkeyid);
                    m_hotkey.active = true;
                    MYTRACE(ACE_TEXT("HK Active %d\n"), m_hotkey.hotkeyid);
                }
            }
        }

        if(!m_keyTesters.empty())
        {
            keytesters_t::iterator ite;
            for(ite=m_keyTesters.begin();ite!=m_keyTesters.end();ite++)
                (*ite)->OnKeyDown(vkcode);
        }
    }
    else
    {
        m_ActiveKeys.erase(vkcode);

        for (size_t i = 0; i < m_hotkeys.size(); i++) {
            assert(m_hotkeys[i].listener);
            if(m_hotkeys[i].active && 
                m_hotkeys[i].keys.contains(vkcode))
            {
                MYTRACE(ACE_TEXT("HK Inactive %d\n"), m_hotkeys[i].hotkeyid);
                m_hotkeys[i].listener->OnHotKeyInactive(m_hotkeys[i].hotkeyid);
                m_hotkeys[i].active = false;
            }
        }

        if(!m_keyTesters.empty())
        {
            keytesters_t::iterator ite;
            for(ite=m_keyTesters.begin();ite!=m_keyTesters.end();ite++)
                (*ite)->OnKeyUp(vkcode);
        }
    }
}

int HotKey::IsHotKeyActive(HotKeyListener *listener, int hotkeyid)
{
    for (auto &m_hotkey : m_hotkeys) {
        if (m_hotkey.listener == listener && m_hotkey.hotkeyid == hotkeyid)
            return static_cast<int>(m_hotkey.active);
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
            m_hotkeys.erase(m_hotkeys.begin() + i);
        else
            i++;
    }
}

BOOL HotKey::HotKeyExists(HotKeyListener *listener, int hotkeyid)
{
    for(auto & m_hotkey : m_hotkeys)
    {
        if (m_hotkey.listener == listener && m_hotkey.hotkeyid == hotkeyid)
            return TRUE;
    }
    return FALSE;
}



//For hotkey hooks
static HHOOK hKeyHook = nullptr;
static HHOOK hMouseHook = nullptr;

//////////////////////////////////////////
/// HotKeys
//////////////////////////////////////////

static LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if(nCode == HC_ACTION)
    {
        if ((lParam & 0x80000000) != 0) //a key is up
        {
            HOTKEY->KeyToggle((UINT) wParam, FALSE);
        } else if (((0x40000000 & lParam) == 0) && ((lParam & 0x80000000) == 0)) //key is down
        {
            HOTKEY->KeyToggle((UINT)wParam, TRUE);
        }
    }
    return (CallNextHookEx(hKeyHook,
                           nCode,
                           wParam,
                           lParam)); //pass control to next hook in the hook chain.
}

/* low level key hook for win2k - xp */

static std::set<DWORD> activekeys;

static LRESULT CALLBACK LowLevelKeyHookProc(  int nCode,     // hook code
                                                                         WPARAM wParam, // message identifier
                                                                         LPARAM lParam  // message data
                                                                         )
{
    if (wParam == WM_KEYDOWN || wParam == WM_KEYUP || wParam == WM_SYSKEYDOWN
        || wParam == WM_SYSKEYUP) {
        auto *lp = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
        LPARAM const code = lp->scanCode;
        auto const ite = activekeys.find(lp->vkCode);
        if((lp->flags & 0x80) != 0u)
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
    }

    return CallNextHookEx(hKeyHook, nCode, wParam, lParam);
}

static std::set<DWORD> activebuttons;

static LRESULT CALLBACK LowLevelMouseHookProc(  int nCode,     // hook code
                                                                             WPARAM wParam, // message identifier
                                                                             LPARAM lParam  // message data
                                                                             )
{
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
            else if (wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONUP)
                dwBtn = VK_XBUTTON1;

            auto *lp = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
            auto const ite = activebuttons.find(dwBtn);
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
    return CallNextHookEx(hMouseHook,nCode,wParam,lParam);
}

struct HookInfo
{
    HINSTANCE hInstance;
    BOOL bLowLevel;
    int nResult;
};

static ACE_thread_t dwHookThreadID = 0;

static ACE_THR_FUNC_RETURN HookLoop(void *arg)
{
    auto* hook = static_cast<HookInfo*> (arg);

    hKeyHook = SetWindowsHookEx( WH_KEYBOARD_LL,
        LowLevelKeyHookProc,
        hook->hInstance,
        NULL);
    assert(hKeyHook);

    int const nMouseHook = (hook->bLowLevel != 0) ? WH_MOUSE_LL : WH_MOUSE;
    hMouseHook = SetWindowsHookEx( nMouseHook,
        LowLevelMouseHookProc,
        hook->hInstance,
        NULL);
    assert(hMouseHook);

    hook->nResult = (static_cast<int>(hMouseHook != nullptr) && (hKeyHook) != nullptr);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, NULL, NULL) && msg.message != WM_QUIT)
        return 0;

    return 1;
}


BOOL InstallHook(HINSTANCE hInstance, BOOL bLLMouseHook)
{
    assert(dwHookThreadID == 0);
    if(dwHookThreadID != 0u)
        return FALSE;

    HookInfo info;
    info.hInstance = hInstance;
    info.bLowLevel = bLLMouseHook;
    info.nResult = -1;

    if (ACE_Thread_Manager::instance()->spawn(HookLoop,
                                              &info,
                                              THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
                                              &dwHookThreadID)
        < 0)
        return 0;

    while(info.nResult == -1)Sleep(10);

    return static_cast<BOOL>(hKeyHook != nullptr && hMouseHook != nullptr);
}

void RemoveHook()
{
    if (dwHookThreadID != 0u) {
        BOOL const bRet = PostThreadMessage(dwHookThreadID, WM_QUIT, 0, 0);
        assert(bRet);
        dwHookThreadID = 0;
    }

    if (hMouseHook != nullptr)
        UnhookWindowsHookEx(hMouseHook);
    hMouseHook = nullptr;
    if (hKeyHook != nullptr)
        UnhookWindowsHookEx(hKeyHook);
    hKeyHook = nullptr;
}

