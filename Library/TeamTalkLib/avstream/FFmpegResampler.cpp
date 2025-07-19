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
#include <libavutil/opt.h>
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

    AVChannelLayout in_ch_layout;
    AVChannelLayout out_ch_layout;
    int ret = 0;

    av_channel_layout_default(&in_ch_layout, GetInputFormat().channels);
    av_channel_layout_default(&out_ch_layout, GetOutputFormat().channels);

    // For older FFmpeg versions, swr_alloc_set_opts2 returns an int and sets the context via the first parameter.
    ret = swr_alloc_set_opts2(&m_ctx,
                              &out_ch_layout,
                              AV_SAMPLE_FMT_S16,
                              GetOutputFormat().samplerate,
                              &in_ch_layout,
                              AV_SAMPLE_FMT_S16,
                              GetInputFormat().samplerate,
                              0,
                              NULL);

    // Clean up the channel layout structs after they've been used.
    av_channel_layout_uninit(&in_ch_layout);
    av_channel_layout_uninit(&out_ch_layout);

    if (ret < 0)
    {
        swr_free(&m_ctx);
        m_ctx = NULL;
        return false;
    }

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
    if (!m_ctx)
        return 0;

    const uint8_t* in_ptr[SWR_CH_MAX] = {};
    in_ptr[0] = (const uint8_t*)input_samples;
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