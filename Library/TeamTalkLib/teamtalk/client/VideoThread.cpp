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
#include <teamtalk/ttassert.h>
#include <codec/MediaUtil.h>

using namespace media;
using namespace teamtalk;

VideoThread::VideoThread()
: m_packet_counter(0)
, m_codec()
, m_frames_passed(0)
, m_frames_dropped(0)
{
    m_codec.codec = CODEC_NO_CODEC;
}

bool VideoThread::StartEncoder(videoencodercallback_t callback,
                               const media::VideoFormat& cap_format,
                               const teamtalk::VideoCodec& codec,
                               int max_frames_queued)
{
    TTASSERT(m_codec.codec == CODEC_NO_CODEC);
    if(this->thr_count() != 0)
        return false;

    int bytes = sizeof(VideoFrame) + RGB32_BYTES(cap_format.width, cap_format.height);
    bytes *= max_frames_queued;
    this->msg_queue()->activate();
    this->msg_queue()->high_water_mark(bytes);
    this->msg_queue()->low_water_mark(bytes);
    
    assert(cap_format.IsValid());

    m_callback = callback;
    m_cap_format = cap_format;
    m_codec = codec;

    switch(codec.codec)
    {
    case CODEC_NO_CODEC :
    {
        if(this->activate()<0)
        {
            StopEncoder();
            return false;
        }
        return true;
    }
#if defined(ENABLE_VPX)
    case CODEC_WEBM_VP8 :
    {
        int fps = 1;
        if(cap_format.fps_denominator)
            fps = cap_format.fps_numerator / cap_format.fps_denominator;

        MYTRACE(ACE_TEXT("Launching VPX encoder %dx%d@%d bitrate %d\n"),
                         cap_format.width, cap_format.height, fps,
                         m_codec.webm_vp8.rc_target_bitrate);
        
        if(!m_vpx_encoder.Open(cap_format.width, cap_format.height, 
                               m_codec.webm_vp8.rc_target_bitrate, fps))
        {
            StopEncoder();
            return false;
        }
        if(this->activate()<0)
        {
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
    int ret;
    ret = this->msg_queue()->close();
    TTASSERT(ret>=0);
    ret = wait();
    TTASSERT(ret>=0);

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

int VideoThread::close(u_long)
{
    MYTRACE( ACE_TEXT("Video Encoder thread closed\n") );
    return 0;
}

int VideoThread::svc(void)
{
    ACE_Message_Block* mb;

    while(getq(mb) >= 0)
    {
        VideoFrame vid(mb);
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

            int enc_len;
            const char* enc_data = m_vpx_encoder.GetEncodedData(enc_len);
            new_ownership = m_callback(mb, enc_data, enc_len,
                                       m_packet_counter++, vid.timestamp);

            while((enc_data = m_vpx_encoder.GetEncodedData(enc_len)))
                m_callback(NULL, enc_data, enc_len,
                           m_packet_counter++, vid.timestamp);
        }
        break;
#endif
        default : break;
        }

        if(!new_ownership)
            mb->release();
    }
    return 0;
}

void VideoThread::QueueFrame(const media::VideoFrame& video_frame)
{
    ACE_Message_Block* mb = VideoFrameToMsgBlock(video_frame);
    if(mb)
        QueueFrame(mb);
}

void VideoThread::QueueFrame(ACE_Message_Block* mb_video)
{
    assert(mb_video->rd_ptr() == mb_video->base());
    ACE_Time_Value tm_zero;
    if (this->msg_queue()->enqueue(mb_video, &tm_zero)<0)
    {
        m_frames_dropped++;
        MYTRACE(ACE_TEXT("Dropped video frame of size %d, buffer holds %u. %d/%d\n"),
                (int)mb_video->length(), (int)this->msg_queue()->message_bytes(), 
                m_frames_dropped, m_frames_dropped + m_frames_passed);
        mb_video->release();
    }
    else
    {
        m_frames_passed++;
    }
    //MYTRACE(ACE_TEXT("Encoder queue size: %d\n"), this->msg_queue()->message_count());
}
