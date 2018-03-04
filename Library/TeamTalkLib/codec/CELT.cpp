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

#include "CELT.h"
#include <memory>
#include <assert.h>
#include <stdio.h>

CeltEncoder::CeltEncoder() : m_mode(NULL), m_encoder(NULL) {}
CeltEncoder::~CeltEncoder() { Close(); }

bool CeltEncoder::Initialize(int sample_rate, int channels, int frame_size)
{
    if(m_encoder)
        return false;

    int err = 0;
    m_mode = celt_mode_create(sample_rate, frame_size, &err);
    assert(err == CELT_OK);
    if(err != CELT_OK)
        goto error;

    m_encoder = celt_encoder_create_custom(m_mode, channels, &err);
    assert(err == CELT_OK);
    if(err != CELT_OK)
        goto error;

    m_framesize = frame_size;
    m_channels = channels;
    return true;

error:
    Close();
    return false;
}

bool CeltEncoder::Initialize(int sample_rate, int channels, int frame_size,
                             int vbr)
{
    if(!Initialize(sample_rate, channels, frame_size))
        return false;
    
    int err = CELT_OK;
    if(vbr)
    {
        err = celt_encoder_ctl(m_encoder, CELT_SET_BITRATE(vbr));
        if(err != CELT_OK)
            goto error;

        //this is as a bool val
        vbr = 1;
        err = celt_encoder_ctl(m_encoder, CELT_SET_VBR(vbr));
        if(err != CELT_OK)
            goto error;
    }
    //if(max_vbr)
    //{
    //    err = celt_encoder_ctl(m_encoder, CELT_SET_VBR_CONSTRAINT(max_vbr));
    //    if(err != CELT_OK)
    //        goto error;
    //}
    return err == CELT_OK;

error:
    Close();
    return false;
}

void CeltEncoder::Close()
{
    if(m_encoder)
        celt_encoder_destroy(m_encoder);
    if(m_mode)
        celt_mode_destroy(m_mode);
    m_encoder = NULL;
    m_mode = NULL;
    m_framesize = 0;
    m_channels = 0;
}

void CeltEncoder::Reset()
{
    assert(m_encoder);
    int v = CELT_RESET_STATE;
    v = celt_encoder_ctl(m_encoder, CELT_RESET_STATE_REQUEST, &v);
    assert(v == CELT_OK);
}

int CeltEncoder::Encode(const short* sample_buffer, char* out_buffer, int out_buf_len)
{
    int ret = celt_encode(m_encoder, sample_buffer, m_framesize, 
                          reinterpret_cast<unsigned char*>(out_buffer), 
                          out_buf_len);
    return ret;
}


int CELT_GetPreferredCbSize(int samplerate, int ms)
{
    int cb_size = samplerate * ms / 1000;
    if(cb_size % CELT_FRAMESIZE_DEFAULT != 0)
        cb_size += CELT_FRAMESIZE_DEFAULT - (cb_size % CELT_FRAMESIZE_DEFAULT);
    return cb_size;
}


CeltDecoder::CeltDecoder() : m_mode(NULL), m_decoder(NULL) {}
CeltDecoder::~CeltDecoder() { Close(); }

bool CeltDecoder::Initialize(int sample_rate, int channels, int frame_size)
{
    int err = 0;
    m_mode = celt_mode_create(sample_rate, frame_size, &err);
    assert(err == CELT_OK);
    if(err != CELT_OK)
        goto error;
    m_decoder = celt_decoder_create_custom(m_mode, channels, &err);
    assert(err == CELT_OK);
    if(err != CELT_OK)
        goto error;
    m_framesize = frame_size;
    m_channels = channels;
    return true;

error:
    Close();
    return false;
}

void CeltDecoder::Close()
{
    if(m_decoder)
        celt_decoder_destroy(m_decoder);
    if(m_mode)
        celt_mode_destroy(m_mode);
    m_decoder = NULL;
    m_mode = NULL;
    m_framesize = 0;
    m_channels = 0;
}

void CeltDecoder::Reset()
{
    assert(m_decoder);
    int v = CELT_RESET_STATE;
    v = celt_decoder_ctl(m_decoder, CELT_RESET_STATE_REQUEST, &v);
    assert(v == CELT_OK);
}

int CeltDecoder::Decode(const char* in_buf, int in_buf_len, short* out_sample_buffer)
{
    assert(m_decoder);
    assert(m_mode);
    int ret = celt_decode(m_decoder, 
        reinterpret_cast<const unsigned char*>(in_buf), 
        in_buf_len, out_sample_buffer, m_framesize);
    assert(ret == CELT_OK);
    return ret;
}

int CeltDecoder::DecodeMultiple(const char* in_buf, 
                                const std::vector<int>& encframe_sizes, 
                                short* out_sample_buffer)
{
    assert(m_decoder);
    assert(m_mode);
    int ret = 0, pos = 0;
    for(size_t i=0;i<encframe_sizes.size();i++)
    {
        if(in_buf)
            ret = Decode(&in_buf[pos], encframe_sizes[i], 
                         &out_sample_buffer[m_framesize*i*m_channels]);
        else
            ret = Decode(NULL, 0, 
                         &out_sample_buffer[m_framesize*i*m_channels]);
        pos += encframe_sizes[i];
    }
    return ret;
}
