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

#include "MediaStreamer.h"

#if defined(ENABLE_MEDIAFOUNDATION)
#include "MFStreamer.h"
#endif
#if defined(ENABLE_DSHOW)
#include "WinMedia.h"
#endif
#if defined(ENABLE_FFMPEG1)
#include "FFMpeg1Streamer.h"
#endif
#if defined(ENABLE_FFMPEG3)
#include "FFMpeg3Streamer.h"
#endif

#include <codec/MediaUtil.h>

#include <assert.h>

using namespace media;

bool GetMediaFileProp(const ACE_TString& filename, MediaFileProp& fileprop)
{
#if defined(ENABLE_MEDIAFOUNDATION)
    return GetMFMediaFileProp(filename, fileprop);
#elif defined(ENABLE_DSHOW)
    return GetDSMediaFileProp(filename, fileprop);
#elif defined(ENABLE_FFMPEG1) || defined(ENABLE_FFMPEG3)
    return GetAVMediaFileProp(filename, fileprop);
#endif
    return false;
}

media_streamer_t MakeMediaStreamer(MediaStreamListener* listener)
{
    media_streamer_t streamer;
    MediaStreamer* tmp_streamer = NULL;

#if defined(ENABLE_MEDIAFOUNDATION)
    ACE_NEW_RETURN(tmp_streamer, MFStreamer(listener), media_streamer_t());
#elif defined(ENABLE_DSHOW)
    ACE_NEW_RETURN(tmp_streamer, DSWrapperThread(listener), media_streamer_t());
#elif defined(ENABLE_FFMPEG1) || defined(ENABLE_FFMPEG3)
    ACE_NEW_RETURN(tmp_streamer, FFMpegStreamer(listener), media_streamer_t());
#endif
    streamer = media_streamer_t(tmp_streamer);

    return streamer;
}

void MediaStreamer::Reset()
{
    m_media_in = MediaFileProp();
    m_media_out = MediaStreamOutput();
    m_stop = false;
    
    m_audio_frames.close();
    m_video_frames.close();
}

void MediaStreamer::InitBuffers()
{
    assert(m_media_out.audio || m_media_out.video);

    const int BUF_SECS = 3;

    if (m_media_out.audio)
    {
        size_t buffer_size = PCM16_BYTES(m_media_out.audio_samplerate *
            BUF_SECS, m_media_out.audio_channels);

        m_audio_frames.low_water_mark(buffer_size);
        m_audio_frames.high_water_mark(buffer_size);
    }

    if (m_media_out.video)
    {
        int bmp_size = m_media_in.video_width * m_media_in.video_height * 4;
        int media_frame_size = bmp_size + sizeof(media::VideoFrame);
        size_t fps = m_media_in.video_fps_numerator / std::max(1, m_media_in.video_fps_denominator);

        size_t buffer_size = fps * BUF_SECS * media_frame_size;
        m_video_frames.low_water_mark(buffer_size);
        m_video_frames.high_water_mark(buffer_size);
    }

    int ret = m_audio_frames.activate();
    assert(ret >= 0);
    ret = m_video_frames.activate();
    assert(ret >= 0);
}

ACE_UINT32 MediaStreamer::GetMinimumFrameDurationMSec() const
{
    ACE_UINT32 wait_ms = 1000;
    if(m_media_out.audio && m_media_out.audio_samples > 0 && m_media_out.audio_samplerate > 0)
    {
        wait_ms = ACE_UINT32(m_media_out.audio_samples * 1000 / m_media_out.audio_samplerate);
    }

    if(m_media_out.video)
    {
        double fps = std::max(1, m_media_in.video_fps_numerator) / std::max(1, m_media_in.video_fps_denominator);
        wait_ms = ACE_UINT32(std::min(1000. / fps, double(wait_ms)));
    }
    return wait_ms;
}

int MediaStreamer::GetQueuedAudioDataSize()
{
    ACE_Time_Value tv;
    ACE_Message_Block* mb;
    if(m_audio_frames.peek_dequeue_head(mb, &tv) < 0)
    {
        return 0;
    }

    // calculate if we can build a frame. 'message_length()' is data available
    // in message queue which has been written but hasn't been read yet.
    // 'message_bytes()' is total amount of data allocated in the message queue.
    size_t hdrs_size = m_audio_frames.message_count() * sizeof(AudioFrame);
    //if we have already read some data of a block we need to substract its header
    int queued_audio_bytes = int(m_audio_frames.message_length());
    if(mb->rd_ptr() != mb->base())
    {
        hdrs_size -= sizeof(AudioFrame);
        queued_audio_bytes -= mb->capacity();
        queued_audio_bytes += mb->length();
    }
    queued_audio_bytes -= hdrs_size;

    return queued_audio_bytes;
}

bool MediaStreamer::ProcessAVQueues(ACE_UINT32 starttime, bool flush)
{
    assert(m_media_out.audio || m_media_out.video);

    bool need_audio = ProcessAudioFrame(starttime, flush);
    bool need_video = ProcessVideoFrame(starttime);
    
    //go to sleep if there is already enough data buffered
    if (!need_audio && !need_video)
    {
        ACE_UINT32 wait_ms = GetMinimumFrameDurationMSec();
        //wait_ms /= 2;
        MYTRACE(ACE_TEXT("Sleeping %d msec... waiting for frames\n"), wait_ms);
        ACE_OS::sleep(ACE_Time_Value(wait_ms / 1000, (wait_ms % 1000) * 1000));
        return true;
    }

    return false;
}

bool MediaStreamer::ProcessAudioFrame(ACE_UINT32 starttime, bool flush)
{
    if (!m_media_out.audio)
        return false;

    const uint32_t NOW = GETTIMESTAMP();

    //see if audio block is less than time 'now'
    ACE_Time_Value tv;
    ACE_Message_Block* mb;
    if(m_audio_frames.peek_dequeue_head(mb, &tv) < 0)
    {
        MYTRACE(ACE_TEXT("Audio %u - Queue empty\n"), NOW - starttime);
        return true;
    }

    int queued_audio_bytes = GetQueuedAudioDataSize();
    int required_audio_bytes = PCM16_BYTES(m_media_out.audio_samples, m_media_out.audio_channels);
    if (queued_audio_bytes < required_audio_bytes && !flush)
    {
        MYTRACE(ACE_TEXT("Audio %u - Insufficent data\n"), NOW - starttime);
        return true;
    }

    uint32_t queue_duration = PCM16_BYTES(m_media_out.audio_samplerate, m_media_out.audio_channels);
    queue_duration = queued_audio_bytes * 1000 / queue_duration;

    // check if head is already ahead of time
    AudioFrame* first_frame = reinterpret_cast<AudioFrame*>(mb->base());
    MYTRACE(ACE_TEXT("Audio %u - Checking %u. Queue duration: %u, bytes %u\n"), 
        NOW - starttime, first_frame->timestamp, queue_duration,
            unsigned(m_audio_frames.message_length()));

    if (W32_GT(first_frame->timestamp, NOW - starttime))
    {
        MYTRACE(ACE_TEXT("Audio %u - Data in future %u\n"),
            NOW - starttime, first_frame->timestamp);
        return false;
    }

    // ready to submit new data
    int audio_block_size = required_audio_bytes + sizeof(AudioFrame);
    ACE_Message_Block* out_mb;
    ACE_NEW_RETURN(out_mb, ACE_Message_Block(audio_block_size), 0);

    AudioFrame* media_frame = reinterpret_cast<AudioFrame*>(out_mb->wr_ptr());
    *media_frame = *first_frame; //use original AudioFrame as base for construction

    //set output properties
    media_frame->timestamp = first_frame->timestamp + starttime;
    media_frame->input_buffer = 
        reinterpret_cast<short*>(out_mb->wr_ptr() + sizeof(AudioFrame));
    media_frame->input_samples = m_media_out.audio_samples;
    //advance wr_ptr() past header
    out_mb->wr_ptr(sizeof(*media_frame));

    int write_bytes = required_audio_bytes;
    do
    {
        //check if we should advance past header
        if(mb->rd_ptr() == mb->base())
            mb->rd_ptr(sizeof(AudioFrame));
            
        if (int(mb->length()) <= write_bytes)
        {
            out_mb->copy(mb->rd_ptr(), mb->length());
            write_bytes -= int(mb->length());
            assert((int)write_bytes >= 0);
            // ensure entire message_block's length is substracted from message queue's length
            mb->rd_ptr(mb->base());
            if (m_audio_frames.dequeue(mb, &tv) >= 0)
            {
                mb->release();
                mb = NULL;
            }
            
            //assert(flush || (write_bytes == 0 && m_audio_frames.message_count() == 0 || write_bytes > 0 && m_audio_frames.message_count()));
        }
        else
        {
            // advance 
            AudioFrame* head_frame = reinterpret_cast<AudioFrame*>(mb->base());
            assert(m_media_out.audio_channels);
            assert(m_media_out.audio_samplerate);
            head_frame->timestamp += PCM16_DURATION(write_bytes, m_media_out.audio_channels, m_media_out.audio_samplerate);
            head_frame = head_frame;

            int ret = out_mb->copy(mb->rd_ptr(), write_bytes);
            assert(ret >= 0);
            mb->rd_ptr(write_bytes);
            write_bytes -= write_bytes;
            assert(mb->rd_ptr() < mb->end());
        }
        assert((int)write_bytes >= 0);
    }
    while(write_bytes > 0 && m_audio_frames.peek_dequeue_head(mb, &tv) >= 0);

    MYTRACE(ACE_TEXT("Audio %u - Writebytes %u, audio size %d, q size %u, msg cnt: %u\n"),
            NOW - starttime, write_bytes, GetQueuedAudioDataSize(),
            unsigned(m_audio_frames.message_length()),
            unsigned(m_audio_frames.message_count()));

    assert(write_bytes == 0);

    //write bytes should only be greater than 0 if flushing
    if(write_bytes)
    {
        ACE_OS::memset(out_mb->wr_ptr(), 0, write_bytes);
        out_mb->wr_ptr(write_bytes);
        assert(out_mb->end() == out_mb->wr_ptr());
    }

    uint32_t timestamp = media_frame->timestamp;
    bool need_more = GetQueuedAudioDataSize() < required_audio_bytes || W32_LEQ(timestamp, NOW);
    MYTRACE(ACE_TEXT("Audio %u - Submitted %u. Diff: %d. Need more %d\n"), NOW - starttime,
            timestamp - starttime, int((NOW - starttime) - (timestamp - starttime)),
            int(need_more));
    //MYTRACE(ACE_TEXT("Ejecting audio frame %u\n"), media_frame.timestamp);
    if (!m_listener->MediaStreamAudioCallback(this, *media_frame, out_mb))
    {
        out_mb->release();
        out_mb = NULL;
    }
    //'out_mb' should now be considered dead

    return true;
}

bool MediaStreamer::ProcessVideoFrame(ACE_UINT32 starttime)
{
    if (!m_media_out.video)
        return false;

    int ret;
    const uint32_t NOW = GETTIMESTAMP();
    //ACE_UINT32 last = -1;
    ACE_Message_Block* mb;
    ACE_Time_Value tm_zero;
    //if (m_video_frames.dequeue_tail(mb, &tm_zero) >= 0)
    //{
    //    VideoFrame last_frm(mb);
    //    last = last_frm.timestamp;

    //    ret = m_video_frames.enqueue_tail(mb, &tm_zero);
    //    if (ret < 0)
    //    {
    //        MYTRACE(ACE_TEXT("Video %u - Failed to reenqueue %u\n"), NOW - starttime, last_frm.timestamp);
    //        mb->release();
    //        return true;
    //    }

    //    uint32_t duration = NOW - starttime;
    //    if (W32_GEQ(last_frm.timestamp, duration))
    //    {
    //        MYTRACE(ACE_TEXT("Video %u - Until %u\n"), NOW - starttime, last_frm.timestamp);
    //    }
    //}

    if (m_video_frames.peek_dequeue_head(mb, &tm_zero) >= 0)
    {
        VideoFrame* media_frame = reinterpret_cast<VideoFrame*>(mb->rd_ptr());
        MYTRACE(ACE_TEXT("Video %u - First %u\n"),
                NOW - starttime, media_frame->timestamp);

        if (W32_LEQ(media_frame->timestamp, NOW - starttime))
        {
            tm_zero = ACE_Time_Value::zero;
            if ((ret = m_video_frames.dequeue(mb, &tm_zero)) < 0)
            {
                assert(ret >= 0);
                return true; // should never happen
            }

            media_frame->timestamp = starttime + media_frame->timestamp;

            MYTRACE(ACE_TEXT("Video %u - Submitted video frame %u. Diff: %u. Queue: %u\n"), 
                    NOW - starttime,
                    media_frame->timestamp - starttime, NOW - media_frame->timestamp,
                    unsigned(m_video_frames.message_count()));

            if(!m_listener->MediaStreamVideoCallback(this, *media_frame, mb))
            {
                mb->release();
                mb = NULL;
            }
        }
        else
        {
            MYTRACE(ACE_TEXT("Video - Not video time %u, duration %u. Queue: %u\n"),
                    media_frame->timestamp, NOW - starttime, unsigned(m_video_frames.message_count()));
            return false;
        }
    }

    MYTRACE(ACE_TEXT("Video %u - No video. Queue: %u\n"), 
            NOW - starttime,
            unsigned(m_video_frames.message_count()));

    return true;
}
