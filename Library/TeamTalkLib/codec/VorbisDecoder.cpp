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

#include "VorbisDecoder.h"

#include <cassert>
#include <cstring>

VorbisDecode::VorbisDecode()
    : m_initialized(false)
{
    vorbis_info_init(&m_info);
    vorbis_comment_init(&m_comment);
}

VorbisDecode::~VorbisDecode()
{
    Close();
}

bool VorbisDecode::Open(int sample_rate, int channels)
{
    if (m_initialized)
        return false;

    // Set basic info
    m_info.channels = channels;
    m_info.rate = sample_rate;

    int ret = vorbis_synthesis_init(&m_dsp, &m_info);
    if (ret != 0)
        return false;

    ret = vorbis_block_init(&m_dsp, &m_block);
    if (ret != 0)
    {
        vorbis_dsp_clear(&m_dsp);
        return false;
    }

    m_initialized = true;
    return true;
}

void VorbisDecode::Close()
{
    if (m_initialized)
    {
        vorbis_block_clear(&m_block);
        vorbis_dsp_clear(&m_dsp);
        vorbis_comment_clear(&m_comment);
        vorbis_info_clear(&m_info);
        m_initialized = false;
    }
}

void VorbisDecode::Reset()
{
    if (m_initialized)
    {
        vorbis_synthesis_restart(&m_dsp);
    }
}

int VorbisDecode::Decode(const char* input_buffer, int input_bufsize,
                         short* output_buffer, int output_samples)
{
    assert(m_initialized);
    assert(output_buffer);

    if (!m_initialized)
        return -1;

    // Create ogg_packet from input buffer
    ogg_packet packet;
    packet.packet = reinterpret_cast<unsigned char*>(const_cast<char*>(input_buffer));
    packet.bytes = input_bufsize;
    packet.b_o_s = 0;
    packet.e_o_s = 0;
    packet.granulepos = -1;
    packet.packetno = 0;

    // Decode the packet
    if (vorbis_synthesis(&m_block, &packet) == 0)
    {
        vorbis_synthesis_blockin(&m_dsp, &m_block);
    }

    // Extract PCM data
    float** pcm;
    int samples = vorbis_synthesis_pcmout(&m_dsp, &pcm);

    if (samples > output_samples)
        samples = output_samples;

    int channels = m_info.channels;
    int total_samples = 0;

    // Convert float samples to short
    for (int i = 0; i < samples; i++)
    {
        for (int ch = 0; ch < channels; ch++)
        {
            int val = static_cast<int>(pcm[ch][i] * 32767.0f);
            if (val > 32767) val = 32767;
            if (val < -32768) val = -32768;
            output_buffer[total_samples++] = static_cast<short>(val);
        }
    }

    // Tell vorbis how many samples we actually consumed
    vorbis_synthesis_read(&m_dsp, samples);

    return total_samples / channels;
}
