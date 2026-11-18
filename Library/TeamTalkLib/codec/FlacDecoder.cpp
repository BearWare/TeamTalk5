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

#include "FlacDecoder.h"

#include <algorithm>
#include <cassert>
#include <cstring>

FlacDecode::~FlacDecode()
{
    Close();
}

bool FlacDecode::Open(int sample_rate, int channels)
{
    if (m_decoder != nullptr)
        return false;

    m_decoder = FLAC__stream_decoder_new();
    if (m_decoder == nullptr)
        return false;

    m_samplerate = sample_rate;
    m_channels = channels;

    FLAC__stream_decoder_set_md5_checking(m_decoder, false);
    if (FLAC__stream_decoder_init_stream(m_decoder, ReadCallback, nullptr, nullptr, nullptr, nullptr,
                                         WriteCallback, nullptr, ErrorCallback,
                                         this) != FLAC__STREAM_DECODER_INIT_STATUS_OK)
    {
        Close();
        return false;
    }
    return true;
}

void FlacDecode::Close()
{
    if (m_decoder != nullptr)
        FLAC__stream_decoder_delete(m_decoder);
    m_decoder = nullptr;
}

int FlacDecode::Decode(const char* input_buffer, int input_bufsize,
                       short* output_buffer, int output_samples)
{
    assert(m_decoder);
    assert(output_buffer);
    if (m_decoder == nullptr || input_buffer == nullptr || input_bufsize <= 0)
        return -1;

    m_input = input_buffer;
    m_input_size = input_bufsize;
    m_input_pos = 0;
    m_output = output_buffer;
    m_output_samples = output_samples;
    m_decoded_samples = 0;
    m_error = false;

    bool const ok = FLAC__stream_decoder_process_single(m_decoder) != 0;
    bool const valid = ok && !m_error && m_input_pos == m_input_size && m_decoded_samples > 0;

    m_input = nullptr;
    m_output = nullptr;

    if (!valid || FLAC__stream_decoder_get_state(m_decoder) == FLAC__STREAM_DECODER_END_OF_STREAM)
        FLAC__stream_decoder_flush(m_decoder);

    if (!valid)
        return -1;
    return m_decoded_samples;
}

FLAC__StreamDecoderReadStatus FlacDecode::ReadCallback(const FLAC__StreamDecoder* /*decoder*/,
                                                       FLAC__byte buffer[], size_t* bytes,
                                                       void* client_data)
{
    auto* self = static_cast<FlacDecode*>(client_data);
    size_t const remain = static_cast<size_t>(self->m_input_size - self->m_input_pos);
    if (self->m_input == nullptr || remain == 0)
    {
        *bytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }

    *bytes = std::min(*bytes, remain);
    std::memcpy(buffer, self->m_input + self->m_input_pos, *bytes);
    self->m_input_pos += static_cast<int>(*bytes);
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderWriteStatus FlacDecode::WriteCallback(const FLAC__StreamDecoder* /*decoder*/,
                                                         const FLAC__Frame* frame,
                                                         const FLAC__int32* const buffer[],
                                                         void* client_data)
{
    auto* self = static_cast<FlacDecode*>(client_data);
    int const blocksize = static_cast<int>(frame->header.blocksize);
    if (self->m_output == nullptr ||
        static_cast<int>(frame->header.channels) != self->m_channels ||
        static_cast<int>(frame->header.sample_rate) != self->m_samplerate ||
        frame->header.bits_per_sample != 16 ||
        self->m_decoded_samples + blocksize > self->m_output_samples)
    {
        self->m_error = true;
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    }

    short* output = self->m_output + (static_cast<size_t>(self->m_decoded_samples) * self->m_channels);
    for (int i = 0; i < blocksize; ++i)
    {
        for (int c = 0; c < self->m_channels; ++c)
            output[(i * self->m_channels) + c] = static_cast<short>(buffer[c][i]);
    }
    self->m_decoded_samples += blocksize;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FlacDecode::ErrorCallback(const FLAC__StreamDecoder* /*decoder*/,
                               FLAC__StreamDecoderErrorStatus /*status*/,
                               void* client_data)
{
    auto* self = static_cast<FlacDecode*>(client_data);
    self->m_error = true;
}
