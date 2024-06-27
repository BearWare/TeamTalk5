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
    HOTKEY_DEC_STREAMVOLUME,
    HOTKEY_ENC_STREAMVOLUME,
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
