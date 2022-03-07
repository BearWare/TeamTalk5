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

#ifndef UTILHOTKEY_H
#define UTILHOTKEY_H

#include "common.h"

enum HotKeyID
{
    HOTKEY_PUSHTOTALK,
    HOTKEY_VOICEACTIVATION,
    HOTKEY_INCVOLUME,
    HOTKEY_DECVOLUME,
    HOTKEY_MUTEALL,
    HOTKEY_MICROPHONEGAIN_INC,
    HOTKEY_MICROPHONEGAIN_DEC,
    HOTKEY_VIDEOTX,
    HOTKEY_REINITSOUNDDEVS,
    HOTKEY_SHOWHIDE_WINDOW,
};

#if defined(Q_OS_DARWIN)
#define MAC_HOTKEY_SIZE 2
#define MAC_NO_KEY 0xFFFFFFFF
#endif

QString getHotKeyText(const hotkey_t& hotkey);

void saveHotKeySettings(HotKeyID hotkeyid, const hotkey_t& hotkey);

bool loadHotKeySettings(HotKeyID hotkeyid, hotkey_t& hotkey);

void deleteHotKeySettings(HotKeyID hotkeyid);

#endif
