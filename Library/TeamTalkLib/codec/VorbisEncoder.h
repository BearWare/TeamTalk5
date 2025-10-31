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

#ifndef VORBISENCODER_H
#define VORBISENCODER_H

#include "mystd/MyStd.h"
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>

class VorbisEncode : private NonCopyable
{
public:
    VorbisEncode();
    ~VorbisEncode();
    bool Open(int sample_rate, int channels, float quality);
    void Close();
    void Reset();

    bool SetBitrate(int bitrate);
    bool SetQuality(float quality);

    int Encode(const short* input_buffer, int input_samples,
               char* output_buffer, int output_bufsize);

    vorbis_info* GetInfo() { return m_initialized ? &m_info : nullptr; }

private:
    vorbis_info m_info;
    vorbis_dsp_state m_dsp;
    vorbis_block m_block;
    bool m_initialized;
};

#endif
