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

#if !defined(SPEEXRESAMPLER_H)
#define SPEEXRESAMPLER_H

#include "AudioResampler.h"
#include "codec/MediaUtil.h"

#include <speex/speex_resampler.h>

#include <vector>

class SpeexResampler : public AudioResampler
{
public:
    SpeexResampler(const SpeexResampler&) = delete;
    SpeexResampler(const media::AudioFormat& informat, const media::AudioFormat& outformat,
                   int fixed_input_samples = 0);
    ~SpeexResampler() override;
    
    bool Init(int quality);
    void Close();

    //return number of samples written to 'output_samples'
    int Resample(const short* input_samples, int input_samples_size,
                 short* output_samples, int output_samples_size) override;


private:
    SpeexResamplerState* m_state;
    std::vector<short> m_tmp_buffer;
};

#endif
