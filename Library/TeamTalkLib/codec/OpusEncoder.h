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

#ifndef OPUSENCODER_H
#define OPUSENCODER_H

#if defined(WIN32)
#include <opus.h>
#else
#include <opus/opus.h>
#endif

int OPUS_GetCbSize(int samplerate, int msec);
int OPUS_GetCbMSec(int samplerate, int cb_samples);

class OpusEncode
{
public:
    OpusEncode(const OpusEncode&) = delete;
    OpusEncode();
    ~OpusEncode();
    bool Open(int sample_rate, int channels, int app);
    void Close();
    void Reset();

    bool SetComplexity(int complex);
    bool SetFEC(bool enable);
    bool SetBitrate(int bitrate);
    bool SetVBR(bool enable);
    bool SetVBRConstraint(bool enable);
    bool SetDTX(bool enable);

    int Encode(const short* input_buffer, int input_samples,
               char* output_buffer, int output_bufsize);
private:
    OpusEncoder* m_encoder;
};

#endif
