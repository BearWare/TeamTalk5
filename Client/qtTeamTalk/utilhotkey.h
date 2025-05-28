/*
 * Copyright (C) 2023, Bjørn D. Rasmussen, BearWare.dk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UTILHOTKEY_H
#define UTILHOTKEY_H

#include "common.h"
#include <QMap>

enum HotKeyID
{
    HOTKEY_NONE = 0x0,
    HOTKEY_PUSHTOTALK = qulonglong(1) << 0,
    HOTKEY_VOICEACTIVATION = qulonglong(1) << 1,
    HOTKEY_INCVOLUME = qulonglong(1) << 2,
    HOTKEY_DECVOLUME = qulonglong(1) << 3,
    HOTKEY_MUTEALL = qulonglong(1) << 4,
    HOTKEY_MICROPHONEGAIN_INC = qulonglong(1) << 5,
    HOTKEY_MICROPHONEGAIN_DEC = qulonglong(1) << 6,
    HOTKEY_VIDEOTX = qulonglong(1) << 7,
    HOTKEY_REINITSOUNDDEVS = qulonglong(1) << 8,
    HOTKEY_SHOWHIDE_WINDOW = qulonglong(1) << 9,
    HOTKEY_SPEAK_PING = qulonglong(1) << 10,

    HOTKEY_FIRST = HOTKEY_PUSHTOTALK,
    HOTKEY_NEXT_UNUSED = qulonglong(1) << 10,
};

typedef qulonglong Hotkeys;
typedef QMap<HotKeyID, hotkey_t> hotkeys_t;

#if defined(Q_OS_DARWIN)
#define MAC_HOTKEY_SIZE 2
#define MAC_NO_KEY 0xFFFFFFFF
#endif

QString getHotKeyName(HotKeyID id);

QString getHotKeyText(const hotkey_t& hotkey);

void saveHotKeySettings(HotKeyID hotkeyid, const hotkey_t& hotkey);

bool loadHotKeySettings(HotKeyID hotkeyid, hotkey_t& hotkey);

void deleteHotKeySettings(HotKeyID hotkeyid);

#endif
