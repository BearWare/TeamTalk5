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

#if !defined(WINMIXER_H)
#define WINMIXER_H

#include <tchar.h>

enum
{
    MIXER_WAVEOUT_SET = 0x1,
    MIXER_WAVEOUT_GET = 0x2,

    MIXER_WAVEOUT_MUTE = 0x10,
    MIXER_WAVEOUT_VOLUME = 0x20,

    MIXER_WAVEOUT_MASTER = 0x100,
    MIXER_WAVEOUT_WAVE = 0x200,
    MIXER_WAVEOUT_MICROPHONE = 0x400,
};

enum
{
    MIXER_WAVEIN_SET        = 0x0001,
    MIXER_WAVEIN_GET        = 0x0002,
    MIXER_WAVEIN_GETCOUNT   = 0x0004,    //get the number of WaveIn devices

    MIXER_WAVEIN_VOLUME     = 0x0010,
    MIXER_WAVEIN_SELECTED   = 0x0020,
    MIXER_WAVEIN_NAME       = 0x0040,
    MIXER_WAVEIN_BOOST      = 0x0080,
    MIXER_WAVEIN_MUTE       = 0x0100,

    MIXER_WAVEIN_MICROPHONE = 0x1000,
    MIXER_WAVEIN_LINEIN     = 0x2000,
    MIXER_WAVEIN_WAVEOUT    = 0x4000,
    MIXER_WAVEIN_BYINDEX    = 0x8000,
};

#define MIXER_STR_LEN 512

union InOutValue
{
    TCHAR name[MIXER_STR_LEN];
    int value;
};

bool mixerWaveOut(int nWaveDeviceID, int mixerMask, InOutValue& inoutValue);

bool mixerWaveIn(int nWaveDeviceID, int mixerMask, InOutValue& inoutValue);

int mixerGetCount();
bool mixerGetName(int nMixerIndex, TCHAR name[MIXER_STR_LEN]);

bool mixerGetWaveInName(int nWaveDeviceID, TCHAR name[MIXER_STR_LEN]);
bool mixerGetWaveOutName(int nWaveDeviceID, TCHAR name[MIXER_STR_LEN]);

#endif
