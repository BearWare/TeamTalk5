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

#ifndef DMORESAMPLER_H
#define DMORESAMPLER_H

#include <ace/OS.h>

#include <wmcodecdsp.h>
#include <dmort.h>

#include <codec/AudioResampler.h>

enum SampleFormat
{
    SAMPLEFORMAT_INT16 = 0x00000008, //compatible with PaSampleFormat
    SAMPLEFORMAT_INT24 = 0x00000004,
};

class DMOResampler : public AudioResampler
{
public:
    DMOResampler();
    virtual ~DMOResampler();

    bool Init(SampleFormat inputSampleFmt, int input_channels,
              int input_samplerate, SampleFormat outputSampleFmt,
              int output_channels, int output_samplerate);
    void Close();

    //returns no. of samples in output
    int Resample(const short* input_samples, int input_samples_cnt,
                 short* output_samples, int output_samples_cnt);
private:
    IMediaObject* m_pDMO;
    DMO_MEDIA_TYPE m_mt_input, m_mt_output;
};

#endif
