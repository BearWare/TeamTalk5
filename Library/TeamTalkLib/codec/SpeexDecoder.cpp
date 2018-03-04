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

#include "SpeexDecoder.h"
#include <assert.h>
#include <cstddef>

SpeexDecoder::SpeexDecoder()
: m_decstate(NULL)
, m_framesize(0)
{
}

SpeexDecoder::~SpeexDecoder()
{
    if(m_decstate)
        Close();
}

bool SpeexDecoder::Initialize(int bandmode)
{
    assert(m_decstate == NULL);
    if(m_decstate)
        return false;

    //initialize speex decoder
    speex_bits_init(&m_DecBits);
    switch(bandmode)
    {
    case SPEEX_MODEID_NB :
        m_decstate = speex_decoder_init(&speex_nb_mode);
        speex_mode_query(&speex_nb_mode, SPEEX_MODE_FRAME_SIZE, &m_framesize);
        break;
    case SPEEX_MODEID_WB :
        m_decstate = speex_decoder_init(&speex_wb_mode);
        speex_mode_query(&speex_wb_mode, SPEEX_MODE_FRAME_SIZE, &m_framesize);
        break;
    case SPEEX_MODEID_UWB :
        m_decstate = speex_decoder_init(&speex_uwb_mode);
        speex_mode_query(&speex_uwb_mode, SPEEX_MODE_FRAME_SIZE, &m_framesize);
        break;
    default :
        assert(false);
        return false;
    }

    return true;
}

void SpeexDecoder::Close()
{
    //shutdown decoder
    if(m_decstate)
    {
        speex_bits_destroy(&m_DecBits);
        speex_decoder_destroy(m_decstate);
        m_decstate = NULL;
        m_framesize = 0;
    }
}

void SpeexDecoder::Reset()
{
    if(m_decstate)
    {
        int v = 1;
        int ret = speex_decoder_ctl(m_decstate, SPEEX_RESET_STATE, &v);
        ret = ret;
    }
}

int SpeexDecoder::GetOption(int opt, spx_int32_t& v)
{
    assert(m_decstate);
    return speex_decoder_ctl(m_decstate, opt, &v);
}

int SpeexDecoder::GetOption(int opt, float& v)
{
    assert(m_decstate);
    return speex_decoder_ctl(m_decstate, opt, &v);
}

int SpeexDecoder::Decode(const char* enc_data, int enc_len, 
                         short* samples_out)
{
    if(!m_decstate)
        return -1;

    if(enc_data)
    {
        speex_bits_read_from(&m_DecBits, const_cast<char*> (enc_data), enc_len);
        return speex_decode_int(m_decstate, &m_DecBits, samples_out);
    }

    //decode lost
    return speex_decode_int(m_decstate, NULL, samples_out);
}

void SpeexDecoder::DecodeMultiple(const char* enc_data, 
                                  const std::vector<int>& encframe_sizes, 
                                  short* samples_out)
{
    int pos = 0;
    for(size_t i=0;i<encframe_sizes.size();i++)
    {
        if(enc_data)
            Decode(&enc_data[pos], encframe_sizes[i], &samples_out[i*m_framesize]);
        else
            Decode(NULL, encframe_sizes[i], &samples_out[i*m_framesize]);
        pos += encframe_sizes[i];
    }
}
