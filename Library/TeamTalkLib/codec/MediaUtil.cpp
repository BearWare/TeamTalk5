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

#include "MediaUtil.h"
#include <assert.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <cstring>

namespace media {

    int AudioInputFormat::GetDurationMSec() const
    {
        assert(samples > 0);
        assert(fmt.samplerate > 0);
        return PCM16_SAMPLES_DURATION(samples, fmt.samplerate);
    }

    int AudioInputFormat::GetTotalSamples() const
    {
        return samples * fmt.channels;
    }

    int AudioInputFormat::GetBytes() const
    {
        return PCM16_BYTES(samples, fmt.channels);
    }

    void AudioFrame::ApplyGain()
    {
        assert(input_buffer || input_samples == 0);
        assert(gain.denominator != 0);
        SOFTGAIN(input_buffer, input_samples, inputfmt.channels,
                 gain.numerator, gain.denominator);
        gain = Rational(1, 1);
    }    
}

void SplitStereo(const short* input_buffer, int input_samples,
                 std::vector<short>& left_chan, std::vector<short>& right_chan)
{
    left_chan.resize(input_samples);
    right_chan.resize(input_samples);

    for(int i=0;i<input_samples;i++)
        left_chan[i] = input_buffer[i*2];
    for(int i=0;i<input_samples;i++)
        right_chan[i] = input_buffer[i*2+1];
}

void MergeStereo(const std::vector<short>& left_chan, 
                 const std::vector<short>& right_chan,
                 short* output_buffer, int output_samples)
{
    for(int i=0;i<output_samples;i++)
        output_buffer[i*2] = left_chan[i];
    for(int i=0;i<output_samples;i++)
        output_buffer[i*2+1] = right_chan[i];
}

void SelectStereo(StereoMask stereo, short* buffer, int samples)
{
    switch(stereo)
    {
    case STEREO_BOTH:
        break;
    case STEREO_LEFT:
        for(int i = 2 * samples - 2; i >= 0; i -= 2)
            buffer[i + 1] = 0;
        break;
    case STEREO_RIGHT:
        for(int i = 2 * samples - 2; i >= 0; i -= 2)
            buffer[i] = 0;
        break;
    case STEREO_NONE:
        for(int i = 2 * samples - 2; i >= 0; i -= 2)
        {
            buffer[i] = 0;
            buffer[i + 1] = 0;
        }
        break;
    }
}

StereoMask ToStereoMask(bool muteleft, bool muteright)
{
    StereoMask stereo = STEREO_BOTH;
    if(muteleft)
        stereo &= ~STEREO_LEFT;
    if(muteright)
        stereo &= ~STEREO_RIGHT;
    return stereo;
}

ACE_Message_Block* VideoFrameInMsgBlock(media::VideoFrame& frm,
                                        ACE_Message_Block::ACE_Message_Type mb_type)
{
    assert(frm.frame_length);
    
    ACE_Message_Block* mb;
    ACE_NEW_RETURN(mb, 
                   ACE_Message_Block(frm.frame_length+sizeof(frm), mb_type), 
                   NULL);
    frm.frame = &mb->rd_ptr()[sizeof(frm)];
    mb->copy(reinterpret_cast<const char*> (&frm), sizeof(frm));
    return mb;
}

ACE_Message_Block* VideoFrameToMsgBlock(const media::VideoFrame& frm,
                                        ACE_Message_Block::ACE_Message_Type mb_type)
{
    assert(frm.frame);
    assert(frm.frame_length);
    media::VideoFrame tmp = frm;
    ACE_Message_Block* mb;
    ACE_NEW_RETURN(mb, ACE_Message_Block(sizeof(tmp)+frm.frame_length, mb_type), NULL);
    tmp.frame = &mb->rd_ptr()[sizeof(tmp)];

    int ret = mb->copy(reinterpret_cast<const char*>(&tmp), sizeof(tmp));
    assert(ret>=0);
    ret = mb->copy(frm.frame, frm.frame_length);
    assert(ret>=0);
    return mb;
}

media::VideoFrame* VideoFrameFromMsgBlock(ACE_Message_Block* mb)
{
    media::VideoFrame* frm = reinterpret_cast<media::VideoFrame*>(mb->base());
    return frm;
}

ACE_Message_Block* AudioFrameToMsgBlock(const media::AudioFrame& frame, bool skip_copy)
{
    ACE_Message_Block* mb;
    int frame_bytes = sizeof(frame);
    int input_bytes = PCM16_BYTES(frame.input_samples, frame.inputfmt.channels);
    int output_bytes = PCM16_BYTES(frame.output_samples, frame.outputfmt.channels);

    //assert(frame.input_buffer && frame.input_samples || !frame.input_buffer && frame.input_samples == 0);
    //assert(frame.output_buffer && frame.output_samples || !frame.output_buffer && frame.output_samples == 0);

    ACE_NEW_RETURN(mb, ACE_Message_Block(frame_bytes + input_bytes + output_bytes), nullptr);

    //assign pointers to inside ACE_Message_Block
    media::AudioFrame copy_frame = frame;
    if (input_bytes)
        copy_frame.input_buffer = reinterpret_cast<short*>(mb->rd_ptr() + frame_bytes);
    if (output_bytes)
        copy_frame.output_buffer = reinterpret_cast<short*>(mb->rd_ptr() + (frame_bytes + input_bytes));

    int ret;

    ret = mb->copy(reinterpret_cast<const char*>(&copy_frame), frame_bytes);
    assert(ret >= 0);

    if (skip_copy)
        return mb;

    if (input_bytes > 0)
    {
        ret = mb->copy(reinterpret_cast<const char*>(frame.input_buffer), input_bytes);
        assert(ret >= 0);
    }

    if (output_bytes > 0)
        ret = mb->copy(reinterpret_cast<const char*>(frame.output_buffer), output_bytes);
    assert(ret >= 0);

    return mb;
}

media::AudioFrame* AudioFrameFromMsgBlock(ACE_Message_Block* mb)
{
    media::AudioFrame* frm = reinterpret_cast<media::AudioFrame*>(mb->base());
    return frm;
}

ACE_Message_Block* AudioFramesMerge(const std::vector<ACE_Message_Block*>& mbs)
{
    assert(mbs.size());

    const int IN_CHANNELS = media::AudioFrame(mbs[0]).inputfmt.channels;
    // const int OUT_CHANNELS = media::AudioFrame(mbs[0]).outputfmt.channels;

    int64_t in_samples = 0, out_samples = 0;
    for (auto mb : mbs)
    {
        media::AudioFrame frm(mb);
        assert(frm.inputfmt == media::AudioFrame(mbs[0]).inputfmt);
        assert(frm.outputfmt == media::AudioFrame(mbs[0]).outputfmt);
        in_samples += frm.input_samples;
        out_samples += frm.output_samples;
    }

    assert(in_samples <= 0xffffffff);
    assert(out_samples <= 0xffffffff);

    media::AudioFrame frm(mbs[0]);
    frm.input_samples = int(in_samples);
    assert(out_samples == 0);
    // frm.output_samples = int(out_samples);

    ACE_Message_Block* mb = AudioFrameToMsgBlock(frm, true);
    frm = media::AudioFrame(mb);

    int in_copied = 0;
    for (auto m : mbs)
    {
        media::AudioFrame newfrm(m);
        size_t bytes = PCM16_BYTES(newfrm.input_samples, IN_CHANNELS);
        std::memcpy(&frm.input_buffer[in_copied * IN_CHANNELS], newfrm.input_buffer, bytes);
        in_copied += newfrm.input_samples;
        mb->wr_ptr(bytes);
        assert(newfrm.output_buffer == nullptr); // doesn't handle 'output_buffer'
//        std::memcpy(&frm.output_buffer[out_copied * OUT_CHANNELS], newfrm.output_buffer, PCM16_BYTES(newfrm.output_samples, OUT_CHANNELS));
//        in_copied += newfrm.input_samples;
    }
    return mb;
}

ACE_Message_Block* AudioFrameFromList(int samples_out, std::vector<ACE_Message_Block*>& mbs)
{
    if (mbs.empty())
        return nullptr;

    assert(media::AudioFrame(mbs[0]).output_buffer == nullptr);

    auto fmt = media::AudioFrame(mbs[0]).inputfmt;
    const int TOTALSAMPLES = samples_out * fmt.channels;

    // check that there's enough samples to build an AudioFrame
    int samples = 0;
    for (auto mb : mbs)
    {
        media::AudioFrame frm(mb);
        assert(frm.input_samples == 0 || frm.inputfmt == fmt);
        samples += frm.input_samples;
        if (frm.input_samples == 0 || samples >= samples_out)
        {
            samples = samples_out;
            break;
        }
    }

    if (samples != samples_out)
        return nullptr;

    // build AudioFrame
    media::AudioFrame muxfrm(mbs[0]);
    muxfrm.input_samples = 0;
    std::vector<short> buffer(TOTALSAMPLES, 0);
    muxfrm.input_buffer = &buffer[0];

    while (muxfrm.input_samples < samples_out)
    {
        assert(mbs.size());
        media::AudioFrame frm(mbs[0]);
        // exit if terminator
        if (frm.input_samples == 0)
        {
            mbs[0]->release();
            mbs.erase(mbs.begin());
            muxfrm.input_samples = samples_out;
            break;
        }

        int copied = std::min(samples_out - muxfrm.input_samples, frm.input_samples);
        size_t bytes = PCM16_BYTES(copied, fmt.channels);
        std::memcpy(&muxfrm.input_buffer[muxfrm.input_samples * fmt.channels], frm.input_buffer, bytes);
        muxfrm.input_samples += copied;

        if (copied == frm.input_samples)
        {
            // 'frm' obsolete, all copied
            mbs[0]->release();
            mbs.erase(mbs.begin());
        }
        else
        {
            // 'frm' still has data
            media::AudioFrame* rawfrm = AudioFrameFromMsgBlock(mbs[0]);
            rawfrm->input_buffer = &rawfrm->input_buffer[copied * fmt.channels];
            rawfrm->input_samples -= copied;
            assert(rawfrm->input_samples > 0);
            rawfrm->sample_no += copied;
            break;
        }
    }
    assert(muxfrm.input_samples <= samples_out);

    return AudioFrameToMsgBlock(muxfrm);
}

int GenerateTone(media::AudioFrame& audblock, int sample_index, int tone_freq, double volume /*= 8000*/)
{
    for(int i = 0; i<audblock.input_samples; i++)
    {
        double t = (double)sample_index++ / audblock.inputfmt.samplerate;
        int v = (int)(volume*sin((double)tone_freq * t * 2.0 * M_PI));
        if(v>32767)
            v = 32767;
        else if(v < -32768)
            v = -32768;

        if(audblock.inputfmt.channels == 1)
            audblock.input_buffer[i] = v;
        else
        {
            audblock.input_buffer[2 * i] = v;
            audblock.input_buffer[2 * i + 1] = v;
        }
    }
    return sample_index;
}
