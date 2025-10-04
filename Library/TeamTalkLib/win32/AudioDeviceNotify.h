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

#if !defined(AUDIODEVICENOTIFY_H)
#define AUDIODEVICENOTIFY_H

#include <ace/ace_wchar.h>

#include <functional>

enum AudioDevEvent
{
    AUDIODEVICE_ADD,
    AUDIODEVICE_REMOVE,
    AUDIODEVICE_UNPLUGGED,
    AUDIODEVICE_NEW_DEFAULT_INPUT,
    AUDIODEVICE_NEW_DEFAULT_OUTPUT,
    AUDIODEVICE_NEW_DEFAULT_INPUT_COMDEVICE,
    AUDIODEVICE_NEW_DEFAULT_OUTPUT_COMDEVICE,
};

using audio_device_change_callback_t
    = std::function<void(AudioDevEvent event, const LPCWSTR &name, const LPCWSTR &id)>;

void RegisterAudioDeviceChange(void* owner, audio_device_change_callback_t cb, bool enable);

#endif
