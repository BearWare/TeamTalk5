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

#if !defined(SPEEXENCODER_H)
#define SPEEXENCODER_H

#include <speex/speex.h>

#include <cstddef>

/****************************/
/******* Win32/Linux ********/
/****************************/

#define DEFAULT_SPEEX_COMPLEXITY 2

#define SPEEX_COMPLEXITY_MIN 0
#define SPEEX_COMPLEXITY_MAX 10

class SpeexEncoder
{
public:
    SpeexEncoder(const SpeexEncoder&) = delete;
    SpeexEncoder();
    ~SpeexEncoder();

    bool Initialize(int bandmode, int complexity, int quality);
    bool Initialize(int bandmode, int complexity, float vbr_quality,
                    int bitrate, int vbr_maxbitrate, bool dtx);
    void Close();

    void Reset();

    int Encode(const short* in_buf, char* out_buf, int nMaxBytes); //num bytes encoded

    int GetOption(int opt, spx_int32_t& v);
    int GetOption(int opt, float& v);

private:
    bool InitCommon(int bandmode, int complexity);
    //speex encoder variables
    void* m_encstate;
    SpeexBits m_EncBits;
};

#endif
