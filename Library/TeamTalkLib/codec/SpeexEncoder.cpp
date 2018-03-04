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

#include "SpeexEncoder.h"
#include <assert.h>

SpeexEncoder::SpeexEncoder()
: m_encstate(NULL)
{
}

SpeexEncoder::~SpeexEncoder()
{
    Close();
}

bool SpeexEncoder::InitCommon(int bandmode, int complexity)
{
    assert(!m_encstate);
    if(m_encstate)
        return false;

    bool success = true;

    //init SpeeX
    speex_bits_init(&m_EncBits);
    switch(bandmode)
    {
    case SPEEX_MODEID_NB : 
        m_encstate = speex_encoder_init(&speex_nb_mode); 
        break;
    case SPEEX_MODEID_WB : 
        m_encstate = speex_encoder_init(&speex_wb_mode); 
        break;
    case SPEEX_MODEID_UWB : 
        m_encstate = speex_encoder_init(&speex_uwb_mode); 
        break;
    default :
        assert(false);
        success = false;
        break;
    }

    if(success)
        success &= speex_encoder_ctl(m_encstate, SPEEX_SET_COMPLEXITY, &complexity) == 0;

    if(!success)
        Close();

    return success;
}

bool SpeexEncoder::Initialize(int bandmode, int complexity, int quality)
{
    if(InitCommon(bandmode, complexity))
    {
        if(speex_encoder_ctl(m_encstate, SPEEX_SET_QUALITY, &quality) == 0)
            return true;
    }
    Close();
    return false;
}

bool SpeexEncoder::Initialize(int bandmode, int complexity, float vbr_quality,
                              int bitrate, int vbr_maxbitrate, bool dtx)
{
    int dtx_enable = 1;
    int vbr_enable = 1;

    if(!InitCommon(bandmode, complexity))
        return false;

    if(vbr_maxbitrate && speex_encoder_ctl(m_encstate, SPEEX_SET_VBR_MAX_BITRATE, &vbr_maxbitrate) != 0)
        goto error;

    if(bitrate)
    {
        if(speex_encoder_ctl(m_encstate, SPEEX_SET_BITRATE, &bitrate) != 0)
            goto error;
    }
    else if(speex_encoder_ctl(m_encstate, SPEEX_SET_VBR_QUALITY, &vbr_quality) != 0)
        goto error;

    if(dtx && speex_encoder_ctl(m_encstate, SPEEX_SET_DTX, &dtx_enable) != 0)
        goto error;

    if(speex_encoder_ctl(m_encstate, SPEEX_SET_VBR, &vbr_enable) != 0)
        goto error;

    return true;

error:
    Close();
    return false;
}


void SpeexEncoder::Close()
{
    if(m_encstate)
    {
        speex_bits_destroy(&m_EncBits);
        speex_encoder_destroy(m_encstate);
        m_encstate = NULL;
    }
}

void SpeexEncoder::Reset()
{
    if(m_encstate)
    {
        int v = 1;
        int ret = speex_encoder_ctl(m_encstate, SPEEX_RESET_STATE, &v);
        assert(ret == 0);
    }
}

int SpeexEncoder::Encode(const short* in_buf, char* out_buf, int nMaxBytes)
{
    assert(m_encstate);

    speex_bits_reset(&m_EncBits);
    speex_encode_int(m_encstate, const_cast<short*>(in_buf), &m_EncBits);
    return speex_bits_write(&m_EncBits, out_buf, nMaxBytes);
}

int SpeexEncoder::GetOption(int opt, spx_int32_t& v)
{
    assert(m_encstate);
    return speex_encoder_ctl(m_encstate, opt, &v);
}

int SpeexEncoder::GetOption(int opt, float& v)
{
    assert(m_encstate);
    return speex_encoder_ctl(m_encstate, opt, &v);
}

