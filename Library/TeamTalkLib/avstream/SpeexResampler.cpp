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

#include "SpeexResampler.h"

#include <cassert>
#include <cstdlib>
#include <utility>

SpeexResampler::SpeexResampler(const media::AudioFormat& informat, const media::AudioFormat& outformat,
                               int fixed_input_samples)
: AudioResampler(informat, outformat, fixed_input_samples)
, m_state(nullptr)
{
}

SpeexResampler::~SpeexResampler()
{
    Close();
}

bool SpeexResampler::Init(int quality)
{
    if (GetInputFormat().channels > 2 || GetOutputFormat().channels > 2)
        return false;

    int err = 0;
    m_state = speex_resampler_init(GetOutputFormat().channels, GetInputFormat().samplerate, 
                                   GetOutputFormat().samplerate, quality, &err);
    assert(m_state);
    return m_state != nullptr;
}

void SpeexResampler::Close()
{
    m_tmp_buffer.clear();

    if(m_state != nullptr)
    {
        speex_resampler_destroy(m_state);
        m_state = nullptr;
    }
}

int SpeexResampler::Resample(const short* input_samples, int input_samples_size, 
                             short* output_samples, int output_samples_size)
{
    int inchans = GetInputFormat().channels;
    int outchans = GetOutputFormat().channels;

    //setup buffer for mono vs. stereo conversion
    if (inchans != outchans)
    {
        int const max_channels = (inchans > outchans)? inchans : outchans;
        int const samples_total = input_samples_size * max_channels;
        if(m_tmp_buffer.size() < (size_t)samples_total)
            m_tmp_buffer.resize(samples_total);
    }

    int err = -1;
    auto output_size = (spx_uint32_t)output_samples_size;
    if (inchans == 2 && outchans == 1)//convert stereo to mono
    {
        const auto mono_sample_count = (size_t)input_samples_size;
        for(size_t i=0;i<mono_sample_count;i++)
            m_tmp_buffer[i] = ((int)((input_samples[i*2] + input_samples[(i*2)+1]))) / 2;

        auto input_size = spx_uint32_t(mono_sample_count);
        err = speex_resampler_process_int(m_state, 0, m_tmp_buffer.data(), 
                                          &input_size, output_samples, 
                                          &output_size);
        assert(err == 0);
        assert(mono_sample_count == input_size);
    }
    else if(inchans == outchans)
    {
        /* use speex_resampler_process_interleaved_int for multichannel */
        spx_uint32_t input_size = input_samples_size;
        err = speex_resampler_process_interleaved_int(m_state,
                                                      input_samples, 
                                                      &input_size, 
                                                      output_samples,
                                                      &output_size);
        assert(err == 0);
        assert(input_samples_size == input_size);
    }
    else if(inchans == 1 && outchans == 2) //convert mono to stereo
    {
        for(int i=0;i<input_samples_size;i++)
        {
            int const stereo_index = i * 2;
            m_tmp_buffer[stereo_index] = input_samples[i];
            m_tmp_buffer[stereo_index+1] = input_samples[i];
        }
        spx_uint32_t input_size = input_samples_size;
        err = speex_resampler_process_interleaved_int(m_state,
                                                      m_tmp_buffer.data(),
                                                      &input_size,
                                                      output_samples,
                                                      &output_size);
        assert(err == 0);
    }
    else { assert(0); return 0; }
    
    assert(output_size <= output_samples_size);
    if(std::cmp_less(output_size, output_samples_size))
    {
        FillOutput(outchans, output_samples, output_size, output_samples_size);
    }
    return err == 0? output_size : 0;
}
