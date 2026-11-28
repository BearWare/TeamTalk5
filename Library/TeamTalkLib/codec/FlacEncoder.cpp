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

bool FlacEncode::Open(int sample_rate, int channels, int compression_level, int frame_size)
{
    if (m_open)
        return false;

    m_samplerate = sample_rate;
    m_channels = channels;
    m_compressionlevel = compression_level;
    m_framesize = frame_size;

    FLAC__StreamEncoder* encoder = CreateEncoder();
    if (encoder == nullptr)
        return false;
    FLAC__stream_encoder_finish(encoder);
    FLAC__stream_encoder_delete(encoder);

    m_input.resize(static_cast<size_t>(frame_size) * channels);
    m_open = true;
    return true;
}

void FlacEncode::Close()
{
    m_open = false;
    m_input.clear();
}

int FlacEncode::Encode(const short* input_buffer, int input_samples,
                       char* output_buffer, int output_bufsize)
{
    assert(input_buffer);
    assert(output_buffer);
    if (!m_open || input_samples != m_framesize)
        return -1;

    for (size_t i = 0; i < m_input.size(); ++i)
        m_input[i] = input_buffer[i];

    m_output = output_buffer;
    m_output_size = output_bufsize;
    m_output_length = 0;
    m_output_overflow = false;

    FLAC__StreamEncoder* encoder = CreateEncoder();
    bool ok = encoder != nullptr;
    if (ok)
    {
        ok = FLAC__stream_encoder_process_interleaved(encoder, m_input.data(), input_samples) != 0;
        ok = (FLAC__stream_encoder_finish(encoder) != 0) && ok;
        FLAC__stream_encoder_delete(encoder);
    }

    m_output = nullptr;
    if (!ok || m_output_overflow)
        return -1;
    return m_output_length;
}

FLAC__StreamEncoder* FlacEncode::CreateEncoder()
{
    FLAC__StreamEncoder* encoder = FLAC__stream_encoder_new();
    if (encoder == nullptr)
        return nullptr;

    bool ok = FLAC__stream_encoder_set_channels(encoder, m_channels) != 0;
    ok = ok && FLAC__stream_encoder_set_bits_per_sample(encoder, 16);
    ok = ok && FLAC__stream_encoder_set_sample_rate(encoder, m_samplerate);
    ok = ok && FLAC__stream_encoder_set_compression_level(encoder, m_compressionlevel);
    ok = ok && FLAC__stream_encoder_set_blocksize(encoder, m_framesize);
    ok = ok && FLAC__stream_encoder_set_streamable_subset(encoder, false);
    ok = ok && FLAC__stream_encoder_init_stream(encoder, WriteCallback, nullptr, nullptr,
                                                nullptr, this) == FLAC__STREAM_ENCODER_INIT_STATUS_OK;
    if (!ok)
    {
        FLAC__stream_encoder_delete(encoder);
        return nullptr;
    }
    return encoder;
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
