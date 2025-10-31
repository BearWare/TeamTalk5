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

#include "VorbisEncoder.h"

#include <cassert>
#include <cstring>

VorbisEncode::VorbisEncode()
    : m_initialized(false)
{
    vorbis_info_init(&m_info);
}

VorbisEncode::~VorbisEncode()
{
    Close();
}

bool VorbisEncode::Open(int sample_rate, int channels, float quality)
{
    if (m_initialized)
        return false;

    int ret = vorbis_encode_init_vbr(&m_info, channels, sample_rate, quality);
    if (ret != 0)
        return false;

    ret = vorbis_analysis_init(&m_dsp, &m_info);
    if (ret != 0)
    {
        vorbis_info_clear(&m_info);
        return false;
    }

    ret = vorbis_block_init(&m_dsp, &m_block);
    if (ret != 0)
    {
        vorbis_dsp_clear(&m_dsp);
        vorbis_info_clear(&m_info);
        return false;
    }

    m_initialized = true;
    return true;
}

void VorbisEncode::Close()
{
    if (m_initialized)
    {
        vorbis_block_clear(&m_block);
        vorbis_dsp_clear(&m_dsp);
        vorbis_info_clear(&m_info);
        m_initialized = false;
    }
}

void VorbisEncode::Reset()
{
    if (m_initialized)
    {
        vorbis_synthesis_restart(&m_dsp);
    }
}

bool VorbisEncode::SetBitrate(int bitrate)
{
    assert(m_initialized);
    if (!m_initialized)
        return false;

    // Note: Setting bitrate after initialization may not work well
    // Consider reinitializing with vorbis_encode_init()
    return false;
}

bool VorbisEncode::SetQuality(float quality)
{
    assert(m_initialized);
    if (!m_initialized)
        return false;

    // Note: Changing quality after initialization requires reinitialization
    return false;
}

int VorbisEncode::Encode(const short* input_buffer, int input_samples,
                          char* output_buffer, int output_bufsize)
{
    assert(m_initialized);
    assert(input_buffer);
    assert(output_buffer);

    if (!m_initialized)
        return -1;

    // Get analysis buffer
    float** buffer = vorbis_analysis_buffer(&m_dsp, input_samples);

    // Convert PCM data to float format
    int channels = m_info.channels;
    for (int i = 0; i < input_samples; i++)
    {
        for (int ch = 0; ch < channels; ch++)
        {
            buffer[ch][i] = input_buffer[i * channels + ch] / 32768.0f;
        }
    }

    // Tell the library how many samples we actually submitted
    vorbis_analysis_wrote(&m_dsp, input_samples);

    int total_bytes = 0;

    // Process blocks
    while (vorbis_analysis_blockout(&m_dsp, &m_block) == 1)
    {
        vorbis_analysis(&m_block, nullptr);
        vorbis_bitrate_addblock(&m_block);

        ogg_packet packet;
        while (vorbis_bitrate_flushpacket(&m_dsp, &packet))
        {
            if (total_bytes + packet.bytes <= output_bufsize)
            {
                std::memcpy(output_buffer + total_bytes, packet.packet, packet.bytes);
                total_bytes += static_cast<int>(packet.bytes);
            }
            else
            {
                // Buffer overflow
                return -1;
            }
        }
    }

    return total_bytes;
}
