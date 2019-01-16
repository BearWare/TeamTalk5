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

#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

#include <ace/Task.h>
#include <ace/Message_Block.h>
#include <ace/Bound_Ptr.h> 
#include <ace/Null_Mutex.h> 

#if defined(ENABLE_VPX)
#include <codec/VpxEncoder.h>
#endif

#include <avstream/VideoCapture.h>

#include <teamtalk/Common.h>

//Get VideoFrame from ACE_Message_Block
#define GET_VIDEOFRAME_FROM_MB(video_frame, msg_block) \
    memcpy(&video_frame, msg_block->rd_ptr(), sizeof(video_frame))

#define GET_OGGPACKET_FROM_MB(ogg_pkt, msg_block) \
    memcpy(&ogg_pkt, msg_block->rd_ptr(), sizeof(ogg_pkt))

class VideoEncListener;

class VideoThread : protected ACE_Task<ACE_MT_SYNCH>
{
public:
    VideoThread();

    bool StartEncoder(VideoEncListener* listener, 
                      const media::VideoFormat& cap_format,
                      const teamtalk::VideoCodec& codec,
                      int max_frames_queued);
    void StopEncoder();

    void QueueFrame(const media::VideoFrame& video_frame);
    void QueueFrame(ACE_Message_Block* mb_video);

    const teamtalk::VideoCodec& GetCodec() const { return m_codec; }
    const media::VideoFormat& GetVideoFormat() const { return m_cap_format; }

private:
    int close(u_long);
    int svc(void);

    VideoEncListener* m_listener;
#if defined(ENABLE_VPX)
    VpxEncoder m_vpx_encoder;
#endif
    ACE_UINT32 m_packet_counter;
    media::VideoFormat m_cap_format;
    teamtalk::VideoCodec m_codec;

    int m_frames_passed, m_frames_dropped;
};

typedef ACE_Strong_Bound_Ptr< VideoThread, ACE_Null_Mutex > video_thread_t;


class VideoEncListener
{
public:
    //@return true if callee took ownership of 'org_frame'
    virtual bool EncodedVideoFrame(const VideoThread* video_encoder,
                                   ACE_Message_Block* org_frame, /* can be NULL */
                                   const char* enc_data, int enc_len,
                                   ACE_UINT32 packet_no,
                                   ACE_UINT32 timestamp) = 0;
};

#endif
