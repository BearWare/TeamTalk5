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
