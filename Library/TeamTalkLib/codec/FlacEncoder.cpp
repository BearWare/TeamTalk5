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

#include "FlacEncoder.h"

#include <cassert>
#include <cstring>

FlacEncode::~FlacEncode()
{
    Close();
}

bool FlacEncode::Open(int sample_rate, int channels, int compression_level, int frame_size)
{
    if (m_encoder != nullptr)
        return false;

    m_encoder = FLAC__stream_encoder_new();
    if (m_encoder == nullptr)
        return false;

    m_samplerate = sample_rate;
    m_channels = channels;
    m_compressionlevel = compression_level;
    m_framesize = frame_size;
    m_input.resize(static_cast<size_t>(frame_size) * channels);

    if (!InitEncoder())
    {
        Close();
        return false;
    }
    FLAC__stream_encoder_finish(m_encoder);
    return true;
}

void FlacEncode::Close()
{
    if (m_encoder != nullptr)
        FLAC__stream_encoder_delete(m_encoder);
    m_encoder = nullptr;
    m_input.clear();
}

int FlacEncode::Encode(const short* input_buffer, int input_samples,
                       char* output_buffer, int output_bufsize)
{
    assert(input_buffer);
    assert(output_buffer);
    if (m_encoder == nullptr || input_samples != m_framesize)
        return -1;

    for (size_t i = 0; i < m_input.size(); ++i)
        m_input[i] = input_buffer[i];

    m_output = output_buffer;
    m_output_size = output_bufsize;
    m_output_length = 0;
    m_output_overflow = false;

    bool ok = InitEncoder();
    ok = ok && FLAC__stream_encoder_process_interleaved(m_encoder, m_input.data(), input_samples);
    ok = (FLAC__stream_encoder_finish(m_encoder) != 0) && ok;

    m_output = nullptr;
    if (!ok)
    {
        FLAC__stream_encoder_delete(m_encoder);
        m_encoder = FLAC__stream_encoder_new();
        return -1;
    }
    if (m_output_overflow)
        return -1;
    return m_output_length;
}

bool FlacEncode::InitEncoder()
{
    bool ok = FLAC__stream_encoder_set_channels(m_encoder, m_channels) != 0;
    ok = ok && FLAC__stream_encoder_set_bits_per_sample(m_encoder, 16);
    ok = ok && FLAC__stream_encoder_set_sample_rate(m_encoder, m_samplerate);
    ok = ok && FLAC__stream_encoder_set_compression_level(m_encoder, m_compressionlevel);
    ok = ok && FLAC__stream_encoder_set_blocksize(m_encoder, m_framesize);
    ok = ok && FLAC__stream_encoder_set_streamable_subset(m_encoder, false);
    if (!ok)
        return false;

    return FLAC__stream_encoder_init_stream(m_encoder, WriteCallback, nullptr, nullptr,
                                            nullptr, this) == FLAC__STREAM_ENCODER_INIT_STATUS_OK;
}

FLAC__StreamEncoderWriteStatus FlacEncode::WriteCallback(const FLAC__StreamEncoder* /*encoder*/,
                                                         const FLAC__byte buffer[], size_t bytes,
                                                         uint32_t samples, uint32_t /*current_frame*/,
                                                         void* client_data)
{
    auto* self = static_cast<FlacEncode*>(client_data);
    if (samples == 0)
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;

    if (self->m_output == nullptr ||
        static_cast<size_t>(self->m_output_length) + bytes > static_cast<size_t>(self->m_output_size))
    {
        self->m_output_overflow = true;
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    }

    std::memcpy(self->m_output + self->m_output_length, buffer, bytes);
    self->m_output_length += static_cast<int>(bytes);
    return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}
