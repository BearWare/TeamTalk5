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

#include "FFmpegResampler.h"
#include <assert.h>

#include "FFmpegStreamer.h" // need InitAVConv()

extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/channel_layout.h>
}

#if !defined(SWR_CH_MAX)
#define SWR_CH_MAX 32
#endif

FFMPEGResampler::FFMPEGResampler(const media::AudioFormat& informat,
                                 const media::AudioFormat& outformat,
                                 int fixed_input_samples)
: AudioResampler(informat, outformat, fixed_input_samples)
, m_ctx(NULL)
{
    InitAVConv();
}

FFMPEGResampler::~FFMPEGResampler()
{
    Close();
}

bool FFMPEGResampler::Init()
{
    if(m_ctx)
        return false;

    m_ctx = swr_alloc_set_opts(NULL,
                               GetOutputFormat().channels == 2?
                               AV_CH_LAYOUT_STEREO :
                               AV_CH_LAYOUT_MONO,
                               AV_SAMPLE_FMT_S16,
                               GetOutputFormat().samplerate,
                               GetInputFormat().channels == 2?
                               AV_CH_LAYOUT_STEREO :
                               AV_CH_LAYOUT_MONO,
                               AV_SAMPLE_FMT_S16,
                               GetInputFormat().samplerate,
                               0,
                               0);
    if(!m_ctx)
        return false;

    return swr_init(m_ctx) >= 0;
}

void FFMPEGResampler::Close()
{
    if(m_ctx)
        swr_free(&m_ctx);
    m_ctx = NULL;
}

int FFMPEGResampler::Resample(const short* input_samples, int input_samples_size,
                              short* output_samples, int output_samples_size)
{
    const uint8_t* in_ptr[SWR_CH_MAX] = {};
    in_ptr[0] = (uint8_t*)input_samples;
    uint8_t* out_ptr[SWR_CH_MAX] = {};
    out_ptr[0] = (uint8_t*)output_samples;

    int ret = swr_convert(m_ctx,
                          out_ptr,
                          output_samples_size,
                          in_ptr,
                          input_samples_size);

    assert(ret <= output_samples_size);
    return ret;
}
