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

#if defined(ENABLE_SPEEXDSP)
#include <avstream/SpeexResampler.h>
#endif

#if defined(ENABLE_DMORESAMPLER)
#include "DMOResampler.h"
#elif defined(ENABLE_FFMPEG3)
#include "FFMpeg3Resampler.h"
#endif

#define ZERO_IT 0

int CalcSamples(int src_samplerate, int src_samples, int dest_samplerate)
{
    double samples = ((double)dest_samplerate / (double)src_samplerate) * (double)src_samples;
    samples += .5;
    return (int)samples;
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
            int stereo_index = output_samples_written * 2;
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


audio_resampler_t MakeAudioResampler(int input_channels, int input_samplerate, 
                                     int output_channels, int output_samplerate)
{
    audio_resampler_t resampler;
    bool ret = false;
#if defined(ENABLE_DMORESAMPLER)
    if(IsWindows6OrLater())
    {
        DMOResampler* tmp_resample;
        ACE_NEW_RETURN(tmp_resample, DMOResampler(), audio_resampler_t());
        resampler = audio_resampler_t(tmp_resample);

        ret  = tmp_resample->Init(SAMPLEFORMAT_INT16, 
                                  input_channels, 
                                  input_samplerate,
                                  SAMPLEFORMAT_INT16, 
                                  output_channels,
                                  output_samplerate);
        MYTRACE(ACE_TEXT("Launched DMOResampler\n"));
    }
#elif defined(ENABLE_FFMPEG3)
    {
        FFMPEGResampler* tmp_resample;
        ACE_NEW_RETURN(tmp_resample, FFMPEGResampler(), audio_resampler_t());
        resampler = audio_resampler_t(tmp_resample);

        ret = tmp_resample->Init(input_samplerate,
                                 input_channels,
                                 output_samplerate,
                                 output_channels);
        MYTRACE(ACE_TEXT("Launched FFMPEGResampler\n"));
    }
#elif defined(ENABLE_SPEEXDSP)
#if defined(ENABLE_DMORESAMPLER)
    else
#endif
    {
        SpeexResampler* tmp_resample;
        ACE_NEW_RETURN(tmp_resample, SpeexResampler(), audio_resampler_t());
        resampler = audio_resampler_t(tmp_resample);

        ret = tmp_resample->Init(5, input_samplerate,
                                 input_channels,
                                 output_samplerate,
                                 output_channels);
        MYTRACE(ACE_TEXT("Launched SpeexResampler\n"));
    }
#else
#pragma message("No resampler available")
#endif
    if(!ret)
        resampler.reset();

    return resampler;
}

