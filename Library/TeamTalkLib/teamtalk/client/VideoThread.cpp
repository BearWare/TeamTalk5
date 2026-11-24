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

#include "VideoThread.h"
#include "mystd/MyStd.h"
#include "teamtalk/TTAssert.h"

#include <cassert>
#include <utility>

using namespace media;
using namespace teamtalk;

VideoThread::VideoThread() 
{
    m_codec.codec = CODEC_NO_CODEC;
}

VideoThread::~VideoThread()
{
    StopEncoder();
}

bool VideoThread::StartEncoder(videoencodercallback_t callback,
                               const media::VideoFormat& cap_format,
                               const teamtalk::VideoCodec& codec,
                               int max_frames_queued)
{
    TTASSERT(m_codec.codec == CODEC_NO_CODEC);
    if (m_thread.joinable())
        return false;

    size_t bytes = sizeof(VideoFrame) + RGB32_BYTES(cap_format.width, cap_format.height);
    bytes *= max_frames_queued;
    m_msg_queue.Activate();
    m_msg_queue.SetWaterMark(bytes);
    
    assert(cap_format.IsValid());

    m_callback = std::move(callback);
    m_cap_format = cap_format;
    m_codec = codec;

    switch(codec.codec)
    {
    case CODEC_NO_CODEC :
    {
        try { m_thread = std::thread([this] { Run(); }); }
        catch (const std::system_error&) {
            StopEncoder();
            return false;
        }
        return true;
    }
#if defined(ENABLE_VPX)
    case CODEC_WEBM_VP8 :
    {
        int fps = 1;
        if(cap_format.fps_denominator != 0)
            fps = cap_format.fps_numerator / cap_format.fps_denominator;

        MYTRACE(TTEXT("Launching VPX encoder %dx%d@%d bitrate %d\n"),
                         cap_format.width, cap_format.height, fps,
                         m_codec.webm_vp8.rc_target_bitrate);
        
        if(!m_vpx_encoder.Open(cap_format.width, cap_format.height, 
                               m_codec.webm_vp8.rc_target_bitrate, fps))
        {
            StopEncoder();
            return false;
        }
        try { m_thread = std::thread([this] { Run(); }); }
        catch (const std::system_error&) {
            StopEncoder();
            return false;
        }
        return true;
    }    
#endif
    default :
        return false;
    }
}

void VideoThread::StopEncoder()
{
    int ret = 0;
    m_msg_queue.Close();
    TTASSERT(ret>=0);
    if (m_thread.joinable())
        m_thread.join();

    switch(m_codec.codec)
    {
    case CODEC_NO_CODEC :
        break;
#if defined(ENABLE_VPX)
    case CODEC_WEBM_VP8 :
        m_vpx_encoder.Close();
        break;
#endif
    default : break;
    }
    m_callback = {};
    m_packet_counter = 0;
    m_cap_format = VideoFormat();
    m_codec = VideoCodec();
    m_frames_passed = m_frames_dropped = 0;
}

bool VideoThread::UpdateEncoder(const teamtalk::VideoCodec& codec)
{
    switch(m_codec.codec)
    {
    case CODEC_NO_CODEC:
        break;
#if defined(ENABLE_VPX)
    case CODEC_WEBM_VP8:
        m_codec.webm_vp8 = codec.webm_vp8;
        return m_vpx_encoder.Update(codec.webm_vp8.rc_target_bitrate);
#endif
    default: break;
    }
    return false;
}

void VideoThread::Run()
{
    std::unique_ptr<MsgBlock> mb;

    while(m_msg_queue.Get(mb))
    {
        VideoFrame const vid(mb.get());
        bool new_ownership = false;
        assert(m_callback);
        
        switch(m_codec.codec)
        {
#if defined(ENABLE_VPX)
        case CODEC_WEBM_VP8 :
        {
            vpx_codec_err_t vpxerr = VPX_CODEC_OK;
            switch (vid.fourcc)
            {
            case media::FOURCC_RGB32 :
                vpxerr = m_vpx_encoder.EncodeRGB32(vid.frame, vid.frame_length,
                                                   !vid.top_down, vid.timestamp,
                                                   m_codec.webm_vp8.encode_deadline);
                assert(vpxerr == VPX_CODEC_OK);
                break;
            case media::FOURCC_I420 :
                vpxerr = m_vpx_encoder.Encode(vid.frame, VPX_IMG_FMT_I420, 1,
                                              !vid.top_down, vid.timestamp,
                                              m_codec.webm_vp8.encode_deadline);
                assert(vpxerr == VPX_CODEC_OK);
                break;
            default :
                assert(0/* unsupported format */);
                break;
            }

            int enc_len = 0;
            const char* enc_data = m_vpx_encoder.GetEncodedData(enc_len);
            new_ownership = m_callback(mb.get(), enc_data, enc_len,
                                       m_packet_counter++, vid.timestamp);

            while((enc_data = m_vpx_encoder.GetEncodedData(enc_len)) != nullptr)
                m_callback(nullptr, enc_data, enc_len,
                           m_packet_counter++, vid.timestamp);
        }
        break;
#endif
        default : break;
        }

        // 'mb' auto-released (unique_ptr)
    }
}

void VideoThread::QueueFrame(const media::VideoFrame& video_frame)
{
    QueueFrame(VideoFrameToMsgBlock(video_frame));
}

void VideoThread::QueueFrame(std::unique_ptr<MsgBlock> mb_video)
{
    if (!m_msg_queue.Put(std::move(mb_video)))
    {
        m_frames_dropped++;
        MYTRACE(TTEXT("Dropped video frame, buffer holds %u. %d/%d\n"),
                static_cast<unsigned>(m_msg_queue.BytesQueued()),
                m_frames_dropped, m_frames_dropped + m_frames_passed);
    }
    else
    {
        m_frames_passed++;
    }
}
