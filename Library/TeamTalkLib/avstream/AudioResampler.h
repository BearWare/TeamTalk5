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

#ifndef AUDIORESAMPLER_H
#define AUDIORESAMPLER_H

#include <memory>
#include <codec/MediaUtil.h>

//with a callback of 'src_samples' and sample rate of
//'src_samplerate', how many samples should be provided given a
//samplerate of 'dest_samplerate'
uint32_t CalcSamples(int src_samplerate, uint32_t src_samples, int dest_samplerate);

class AudioResampler
{
public:
    AudioResampler(const media::AudioFormat& informat, const media::AudioFormat& outformat,
                   int fixed_input_samples = 0);
    virtual ~AudioResampler() {}

    // resample with varying 'input_sample_size'
    virtual int Resample(const short* input_samples, int input_samples_size,
                         short* output_samples, int output_samples_size) = 0;

    // resample with fixed 'input_sample_size'. Returns 'm_resampleoutput'
    short* Resample(const short* input_samples, int* output_samples_size = nullptr);
    // resample with fixed 'input_sample_size'
    int Resample(const short* input_samples, short* output_samples);

    const media::AudioFormat& GetInputFormat() const { return m_infmt; }
    const media::AudioFormat& GetOutputFormat() const { return m_outfmt; }

protected:
    void SetupFixedFrameSize(const media::AudioFormat& informat,
                             const media::AudioFormat& outformat,
                             int input_samples_size);

    void FillOutput(int channels, short* output_samples,
                    int output_samples_written,
                    int output_samples_total);
private:
    media::AudioFormat m_infmt, m_outfmt;
    std::vector<short> m_resampleoutput;
    int m_input_samples_size = 0, m_output_samples_size = 0;
};

typedef std::shared_ptr< AudioResampler > audio_resampler_t;

audio_resampler_t MakeAudioResampler(const media::AudioFormat& informat,
                                     const media::AudioFormat& outformat,
                                     int input_samples_size = 0);

#endif
