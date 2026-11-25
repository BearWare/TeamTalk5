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

#include "codec/MediaUtil.h"
#include "mystd/MyQueue.h"
#include "teamtalk/Common.h"

#if defined(ENABLE_VPX)
#include "codec/VpxEncoder.h"
#endif

#include <functional>
#include <memory>
#include <thread>

//Get VideoFrame from MsgBlock
#define GET_VIDEOFRAME_FROM_MB(video_frame, msg_block) \
    memcpy(&(video_frame), (msg_block)->rd_ptr(), sizeof(video_frame))

#define GET_OGGPACKET_FROM_MB(ogg_pkt, msg_block) \
    memcpy(&(ogg_pkt), (msg_block)->rd_ptr(), sizeof(ogg_pkt))

using videoencodercallback_t = std::function< void (msgblock_t org_frame, /* can be NULL */
                             const char* enc_data, int enc_len,
                             uint32_t packet_no,
                             uint32_t timestamp) >;

class VideoThread
{
public:
    VideoThread();
    ~VideoThread();

    bool StartEncoder(videoencodercallback_t m_callback,
                      const media::VideoFormat& cap_format,
                      const teamtalk::VideoCodec& codec,
                      int max_frames_queued);
    void StopEncoder();

    bool UpdateEncoder(const teamtalk::VideoCodec& codec);

    void QueueFrame(const media::VideoFrame& video_frame);
    void QueueFrame(std::unique_ptr<MsgBlock> mb_video);

    const teamtalk::VideoCodec& GetCodec() const { return m_codec; }
    const media::VideoFormat& GetVideoFormat() const { return m_cap_format; }

private:
    void Run();
    MsgQueue m_msg_queue;
    std::thread m_thread;

    videoencodercallback_t m_callback;
    
#if defined(ENABLE_VPX)
    VpxEncoder m_vpx_encoder;
#endif
    uint32_t m_packet_counter = 0;
    media::VideoFormat m_cap_format;
    teamtalk::VideoCodec m_codec;

    int m_frames_passed = 0, m_frames_dropped = 0;
};

using video_thread_t = std::shared_ptr< VideoThread >;


class VideoEncListener
{
public:
    virtual ~VideoEncListener() = default;
    //@return true if callee took ownership of 'org_frame'
    virtual bool EncodedVideoFrame(const VideoThread* video_encoder,
                                   msgblock_t org_frame, /* can be NULL */
                                   const char* enc_data, int enc_len,
                                   uint32_t packet_no,
                                   uint32_t timestamp) = 0;
};

#endif
