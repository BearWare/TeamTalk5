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

#include "AudioResampler.h"

#include "myace/MyACE.h"

#if defined(ENABLE_SPEEXDSP)
#include "avstream/SpeexResampler.h"
#endif
#if defined(ENABLE_DMORESAMPLER)
#include "DMOResampler.h"
#elif defined(ENABLE_FFMPEG)
#include "FFmpegResampler.h"
#endif

#include <cassert>
#include <cstdint>
#include <cstring>

#define ZERO_IT 0

uint32_t CalcSamples(int src_samplerate, uint32_t src_samples, int dest_samplerate)
{
    double samples = ((double)dest_samplerate / (double)src_samplerate) * (double)src_samples;
    samples += .5;
    return uint32_t(samples);
}

AudioResampler::AudioResampler(const media::AudioFormat& informat, const media::AudioFormat& outformat,
                               int fixed_input_samples/* = 0*/)
: m_infmt(informat)
, m_outfmt(outformat)
{
    if (fixed_input_samples > 0)
        SetupFixedFrameSize(informat, outformat, fixed_input_samples);

    MYTRACE(ACE_TEXT("Created resampler %d Hz, channels %d -> %d Hz, channels %d. Frame size: %d -> %d\n"),
            informat.samplerate, informat.channels, outformat.samplerate, outformat.channels,
            fixed_input_samples,
            ((fixed_input_samples != 0) ? CalcSamples(informat.samplerate, fixed_input_samples, outformat.samplerate) : 0));
}

void AudioResampler::FillOutput(int channels, short* output_samples,
                                int output_samples_written,
                                int output_samples_total)
{
    while(output_samples_written < output_samples_total)
    {
        switch(channels)
        {
        case 1 :
            output_samples[output_samples_written] = output_samples[output_samples_written-1];
#if ZERO_IT
            output_samples[output_samples_written] = 0;
#endif
            break;
        case 2 :
            int const stereo_index = output_samples_written * 2;
            output_samples[stereo_index] = output_samples[stereo_index-1];
            output_samples[stereo_index+1] = output_samples[stereo_index-2];
#if ZERO_IT
            output_samples[stereo_index] = 0;
            output_samples[stereo_index+1] = 0;
#endif
            break;
        }
        output_samples_written++;
    }
}

void AudioResampler::SetupFixedFrameSize(const media::AudioFormat& informat,
                                         const media::AudioFormat& outformat,
                                         int input_samples_size)
{
    assert(informat.IsValid());
    assert(outformat.IsValid());
    int const output_samples_size = CalcSamples(informat.samplerate, input_samples_size, outformat.samplerate);
    m_resampleoutput.resize(output_samples_size * outformat.channels);

    m_input_samples_size = input_samples_size;
    m_output_samples_size = output_samples_size;
}

short* AudioResampler::Resample(const short* input_samples, int* output_samples_size /*= nullptr*/)
{
    assert(m_resampleoutput.size());

    int const outsamples = Resample(input_samples, m_input_samples_size, m_resampleoutput.data(), m_output_samples_size);
    if (output_samples_size != nullptr)
        *output_samples_size = outsamples;
    return m_resampleoutput.data();
}

int AudioResampler::Resample(const short* input_samples, short* output_samples)
{
    int const outsamples = Resample(input_samples, m_input_samples_size, output_samples, m_output_samples_size);
    if (outsamples < m_output_samples_size)
    {
        // zero remaining on fixed size output
        std::memset(&output_samples[outsamples * GetOutputFormat().channels], 0,
                    PCM16_BYTES(m_output_samples_size - outsamples, GetOutputFormat().channels));
    }
    return outsamples;
}

audio_resampler_t MakeAudioResampler(const media::AudioFormat& informat,
                                     const media::AudioFormat& outformat,
                                     int input_samples_size/* = 0*/)
{
    assert(informat.IsValid());
    assert(outformat.IsValid());

    if(!informat.IsValid() || !outformat.IsValid())
        return {};

    audio_resampler_t resampler;
    bool ret = false;
#if defined(ENABLE_DMORESAMPLER)
    auto dmo = new DMOResampler(informat, outformat, input_samples_size);
    resampler.reset(dmo);
    ret = dmo->Init(SAMPLEFORMAT_INT16, SAMPLEFORMAT_INT16);
    MYTRACE(ACE_TEXT("Launched DMOResampler\n"));
#elif defined(ENABLE_FFMPEG)
    auto *ffmpeg = new FFMPEGResampler(informat, outformat, input_samples_size);
    resampler.reset(ffmpeg);
    ret = ffmpeg->Init();
    MYTRACE(ACE_TEXT("Launched FFMPEGResampler\n"));
#elif defined(ENABLE_SPEEXDSP)
    auto spx = new SpeexResampler(informat, outformat, input_samples_size);
    resampler.reset(spx);
    ret = spx->Init(5);
    MYTRACE(ACE_TEXT("Launched SpeexResampler\n"));
#else
#pragma message("No resampler available")
#endif
    if(!ret)
        resampler.reset();

    return resampler;
}
