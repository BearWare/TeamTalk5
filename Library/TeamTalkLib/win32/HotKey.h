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

#include <vector>
#include <set>

class HotKeyListener
{
public:
    virtual void OnHotKeyActive(int hotkeyid) = 0;
    virtual void OnHotKeyInactive(int hotkeyid) = 0;
    virtual void OnKeyDown(UINT nVK) = 0;
    virtual void OnKeyUp(UINT nVK) = 0;
};

typedef std::set<int> intset_t;
struct HotKeyHook
{
    int hotkeyid;
    intset_t keys;
    bool active;
    HotKeyListener* listener;
    HotKeyHook()
    {
        hotkeyid = 0;
        active = false;
        listener = NULL;
    }
};

class HotKey
{
private:
    HotKey();
    ~HotKey();

public:
    static HotKey* Instance();

    void RegisterHotKey(const HotKeyHook& hotkey);
    void UnregisterHotKey(HotKeyListener* listener, int hotkeyid);

    void KeyToggle(UINT vkcode, BOOL keydown);

    int IsHotKeyActive(HotKeyListener* listener, int hotkeyid);

    void AddKeyTester(HotKeyListener* listener);
    void RemoveKeyTester(HotKeyListener* listener);

    void ClearAll(HotKeyListener* listener);

    BOOL HotKeyExists(HotKeyListener* listener, int hotkeyid);

private:
    intset_t m_ActiveKeys;
    typedef std::vector< HotKeyHook > hotkeys_t;
    hotkeys_t m_hotkeys;
    typedef std::set<HotKeyListener*> keytesters_t;
    keytesters_t m_keyTesters;
};

BOOL InstallHook(HINSTANCE hInstance, BOOL bLLMouseHook);
void RemoveHook();

#define HOTKEY HotKey::Instance()
