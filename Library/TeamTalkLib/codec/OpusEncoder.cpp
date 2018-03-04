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

#include "OpusEncoder.h"
#include <string.h>
#include <assert.h>

int OPUS_GetCbSize(int samplerate, int msec)
{
    if(!msec)
        return 0;

    //OPUS frame must be divided by 2.5ms
    int cb_samples = (int)(samplerate * 0.0025);

    int opus_frames = msec * 1000 / 2500;

    while(opus_frames>=2)
    {
        cb_samples *= 2;
        opus_frames /= 2;
    }
    if(msec == 60)
        cb_samples = (cb_samples / 2) + cb_samples;

    return cb_samples;
}

int OPUS_GetCbMSec(int samplerate, int cb_samples)
{
    if(!samplerate)
        return 0;

    int msec = cb_samples * 1000 / samplerate;
    int samples = OPUS_GetCbSize(samplerate, msec);
    if(!samples)
        return 0;
    
    return samples * 1000 / samplerate;
}

OpusEncode::OpusEncode()
    : m_encoder(NULL)
{
}

OpusEncode::~OpusEncode()
{
    Close();
}

bool OpusEncode::Open(int sample_rate, int channels, int app)
{
    if(m_encoder)
        return false;

    int err = 0;
    m_encoder = opus_encoder_create(sample_rate, channels, app, &err);
    assert(err == 0);

    return m_encoder != NULL;
}

void OpusEncode::Close()
{
    if(m_encoder)
        opus_encoder_destroy(m_encoder);
    m_encoder = NULL;
}

bool OpusEncode::SetComplexity(int complex)
{
    assert(m_encoder);
    if(!m_encoder)
        return false;
    
    int err = opus_encoder_ctl(m_encoder, OPUS_SET_COMPLEXITY(complex));
    assert(err == 0);
    return err == 0;
}

bool OpusEncode::SetFEC(bool enable)
{
    assert(m_encoder);
    if(!m_encoder)
        return false;

    int value = enable;
    int err = opus_encoder_ctl(m_encoder, OPUS_SET_INBAND_FEC(value));
    assert(err == 0);
    return err == 0;
}

bool OpusEncode::SetBitrate(int bitrate)
{
    assert(m_encoder);
    if(!m_encoder)
        return false;

    int err = opus_encoder_ctl(m_encoder, OPUS_SET_BITRATE(bitrate));
    assert(err == 0);    
    return err == 0;
}

bool OpusEncode::SetVBR(bool enable)
{
    assert(m_encoder);
    if(!m_encoder)
        return false;

    int value = enable;
    int err = opus_encoder_ctl(m_encoder, OPUS_SET_VBR(value));
    assert(err == 0);
    return err == 0;
}

bool OpusEncode::SetVBRConstraint(bool enable)
{
    assert(m_encoder);
    if(!m_encoder)
        return false;

    int value = enable;
    int err = opus_encoder_ctl(m_encoder, OPUS_SET_VBR_CONSTRAINT(value));
    assert(err == 0);
    return err == 0;
}

bool OpusEncode::SetDTX(bool enable)
{
    assert(m_encoder);
    if(!m_encoder)
        return false;

    int value = enable;
    int err = opus_encoder_ctl(m_encoder, OPUS_SET_DTX(value));
    assert(err == 0);
    return err == 0;
}

int OpusEncode::Encode(const short* input_buffer, int input_samples,
                        char* output_buffer, int output_bufsize)
{
    assert(m_encoder);
    assert(input_buffer);
    assert(output_buffer);
    return opus_encode(m_encoder, input_buffer, input_samples, 
                       reinterpret_cast<unsigned char*>(output_buffer),
                       output_bufsize);
}
