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
    media::VideoFrame* frm = reinterpret_cast<media::VideoFrame*>(mb->rd_ptr());
    return frm;
}

ACE_Message_Block* AudioFrameToMsgBlock(const media::AudioFrame& frame)
{
    ACE_Message_Block* mb;
    int frame_bytes = sizeof(frame);
    int input_bytes = PCM16_BYTES(frame.input_samples, frame.inputfmt.channels);
    int output_bytes = PCM16_BYTES(frame.output_samples, frame.outputfmt.channels);

    assert(frame.input_buffer && frame.input_samples || !frame.input_buffer && frame.input_samples == 0);
    assert(frame.output_buffer && frame.output_samples || !frame.output_buffer && frame.output_samples == 0);

    ACE_NEW_RETURN(mb, ACE_Message_Block(frame_bytes + input_bytes + output_bytes), nullptr);

    //assign pointers to inside ACE_Message_Block
    media::AudioFrame copy_frame = frame;
    copy_frame.input_buffer = reinterpret_cast<short*>(mb->rd_ptr() + frame_bytes);
    if (output_bytes)
        copy_frame.output_buffer = reinterpret_cast<short*>(mb->rd_ptr() + (frame_bytes + input_bytes));

    int ret;

    ret = mb->copy(reinterpret_cast<const char*>(&copy_frame), frame_bytes);
    assert(ret >= 0);

    ret = mb->copy(reinterpret_cast<const char*>(frame.input_buffer), input_bytes);
    assert(ret >= 0);

    if(output_bytes)
        ret = mb->copy(reinterpret_cast<const char*>(frame.output_buffer), output_bytes);
    assert(ret >= 0);

    return mb;
}

int GenerateTone(media::AudioFrame& audblock, int sample_index, int tone_freq)
{
    double volume = 8000;

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
