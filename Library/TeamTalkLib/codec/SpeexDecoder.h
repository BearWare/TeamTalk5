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

#if !defined(SPEEXDECODER_H)
#define SPEEXDECODER_H

#include <speex/speex.h>
#include <vector>

class SpeexDecoder
{
public:
    SpeexDecoder(const SpeexDecoder&) = delete;
    SpeexDecoder();
    ~SpeexDecoder();

    bool Initialize(int bandmode);
    void Close();
    void Reset();

    int GetOption(int opt, spx_int32_t& v);
    int GetOption(int opt, float& v);

    int Decode(const char* enc_data, int enc_len, short* samples_out);
    void DecodeMultiple(const char* enc_data, 
                        const std::vector<int>& encframe_sizes, 
                        short* samples_out);
private:
    //speex stuff
    SpeexBits m_DecBits;
    void* m_decstate;
    int m_framesize;
};
#endif
