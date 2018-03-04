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

#ifndef CELTCODEC_H
#define CELTCODEC_H

#include <celt.h>
#include <vector>

#define CELT_FRAMESIZE_DEFAULT 256

class CeltEncoder
{
public:
    CeltEncoder();
    ~CeltEncoder();
    //CBR init
    bool Initialize(int sample_rate, int channels, int frame_size);
    //VBR init
    bool Initialize(int sample_rate, int channels, int frame_size,
                    int vbr);
    void Close();
    void Reset();

    int Encode(const short* sample_buffer, char* out_buffer, int out_buf_len);

private:
    CELTMode* m_mode;
    CELTEncoder* m_encoder;
    int m_framesize;
    int m_channels;
};

int CELT_GetPreferredCbSize(int samplerate, int ms);

class CeltDecoder
{
public:
    CeltDecoder();
    ~CeltDecoder();
    bool Initialize(int sample_rate, int channels, int frame_size);
    void Close();
    void Reset();

    int Decode(const char* in_buf, int in_buf_len, short* out_sample_buffer);
    int DecodeMultiple(const char* in_buf, 
                       const std::vector<int>& encframe_sizes, 
                       short* out_sample_buffer);

private:
    CELTMode* m_mode;
    CELTDecoder* m_decoder;
    int m_framesize;
    int m_channels;
};

#endif
