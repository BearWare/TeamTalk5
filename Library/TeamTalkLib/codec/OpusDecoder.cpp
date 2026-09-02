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

#include <cassert>

OpusDecode::OpusDecode()
    : m_decoder(nullptr)
{
}

OpusDecode::~OpusDecode()
{
    Close();
}

bool OpusDecode::Open(int sample_rate, int channels)
{
    if(m_decoder != nullptr)
        return false;

    int err = 0;
    m_decoder = opus_decoder_create(sample_rate, channels, &err);
    assert(err == 0);
    return m_decoder != nullptr;
}

void OpusDecode::Close()
{
    if(m_decoder != nullptr)
        opus_decoder_destroy(m_decoder);
    m_decoder = nullptr;
}

void OpusDecode::Reset()
{
    if (m_decoder != nullptr)
    {
        int const ret = opus_decoder_ctl(m_decoder, OPUS_RESET_STATE);
        assert(ret == OPUS_OK);
    }
}

int OpusDecode::Decode(const char* input_buffer, int input_bufsize,
                       short* output_buffer, int output_samples, bool fec)
{
    assert(m_decoder);
    assert(output_buffer);

    auto const data = reinterpret_cast<const unsigned char*>(input_buffer);
    int const len = (input_buffer != nullptr) ? input_bufsize : 0;
    int const decode_fec = (fec && input_buffer != nullptr) ? 1 : 0;
    return opus_decode(m_decoder, (input_buffer != nullptr) ? data : nullptr,
                       len, output_buffer, output_samples, decode_fec);
}
