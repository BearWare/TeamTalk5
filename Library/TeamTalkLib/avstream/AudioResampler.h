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

#include <myace/MyACE.h>
#include <ace/Bound_Ptr.h>

//with a callback of 'src_samples' and sample rate of
//'src_samplerate', how many samples should be provided given a
//samplerate of 'dest_samplerate'
int CalcSamples(int src_samplerate, int src_samples, int dest_samplerate);

class AudioResampler
{
public:
    virtual ~AudioResampler() {}

    virtual int Resample(const short* input_samples, int input_samples_size,
                         short* output_samples, int output_samples_size) = 0;

protected:
    void FillOutput(int channels, short* output_samples,
                    int output_samples_written,
                    int output_samples_total);
};

typedef ACE_Strong_Bound_Ptr< AudioResampler, ACE_Null_Mutex > audio_resampler_t;

audio_resampler_t MakeAudioResampler(int input_channels, 
                                     int input_samplerate, 
                                     int output_channels, 
                                     int output_samplerate);

#endif
