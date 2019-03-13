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

#ifndef CODECCOMMON_H
#define CODECCOMMON_H

#include "Common.h"

#include <codec/MediaUtil.h>

namespace teamtalk
{
    bool ValidAudioCodec(const AudioCodec& codec);
    int GetAudioCodecSampleRate(const AudioCodec& codec);
    int GetAudioCodecCbSamples(const AudioCodec& codec);
    int GetAudioCodecCbBytes(const AudioCodec& codec);
    int GetAudioCodecCbMillis(const AudioCodec& codec);
    int GetAudioCodecCbTotalSamples(const AudioCodec& codec); // includes * channels
    int GetAudioCodecChannels(const AudioCodec& codec);
    int GetAudioCodecEncSize(const AudioCodec& codec); //fpp * enc_frame
    int GetAudioCodecEncFrameSize(const AudioCodec& codec);
    int GetAudioCodecFrameSize(const AudioCodec& codec);
    int GetAudioCodecFramesPerPacket(const AudioCodec& codec);
    bool GetAudioCodecVBRMode(const AudioCodec& codec);
    bool GetAudioCodecSimulateStereo(const AudioCodec& codec);
    int GetAudioCodecBitRate(const AudioCodec& codec);
    media::AudioFormat GetAudioCodecAudioFormat(const AudioCodec& codec);

    int GetSpeexBandMode(const AudioCodec& codec);
    int GetSpeexQuality(const AudioCodec& codec);
    int GetSpeexBandModeSampleRate(int bandmode);
    int GetSpeexBandModeFrameSize(int bandmode);

    //returns number of milliseconds 
    int GetSpeexFramesDuration(int bandmode, int framecount);
    int GetSpeexSamplesCount(int bandmode, int framecount);
}

#endif
