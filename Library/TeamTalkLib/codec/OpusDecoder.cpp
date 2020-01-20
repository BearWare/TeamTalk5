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

#include "OpusDecoder.h"
#include <string.h>
#include <assert.h>

OpusDecode::OpusDecode()
    : m_decoder(NULL)
{
}

OpusDecode::~OpusDecode()
{
    Close();
}

bool OpusDecode::Open(int sample_rate, int channels)
{
    if(m_decoder)
        return false;

    int err = 0;
    m_decoder = opus_decoder_create(sample_rate, channels, &err);
    assert(err == 0);
    return m_decoder != NULL;
}

void OpusDecode::Close()
{
    if(m_decoder)
        opus_decoder_destroy(m_decoder);
    m_decoder = NULL;
}

void OpusDecode::Reset()
{
    if (m_decoder)
    {
        int ret = opus_decoder_ctl(m_decoder, OPUS_RESET_STATE);
        assert(ret == OPUS_OK);
    }
}

int OpusDecode::Decode(const char* input_buffer, int input_bufsize, 
                       short* output_buffer, int output_samples)
{
    assert(m_decoder);
    assert(output_buffer);
    return opus_decode(m_decoder, 
                       reinterpret_cast<const unsigned char*>(input_buffer?input_buffer:NULL),
                       input_buffer?input_bufsize:0, output_buffer, 
                       output_samples, input_buffer?0:1);
}
