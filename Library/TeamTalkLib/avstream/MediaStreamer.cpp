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
#elif defined(ENABLE_FFMPEG3)
    return GetAVMediaFileProp(filename, fileprop);
#endif
    return false;
}

mediafile_streamer_t MakeMediaFileStreamer()
{
    mediafile_streamer_t streamer;

#if defined(ENABLE_MEDIAFOUNDATION)
    streamer.reset(new MFStreamer());
#elif defined(ENABLE_DSHOW)
    streamer.reset(new DSWrapperThread());
#elif defined(ENABLE_FFMPEG3)
    streamer.reset(new FFMpegStreamer());
#endif

    return streamer;
}

MediaStreamer::~MediaStreamer()
{
    // subclasses must call ::Close() manually to finish their ::Run() method
    assert(!m_thread);
    MYTRACE(ACE_TEXT("~MediaStreamer()\n"));
}

void MediaStreamer::RegisterVideoCallback(mediastream_videocallback_t cb, bool enable)
{
    if (enable)
        m_videocallback = cb;
    else
        m_videocallback = {};
}

void MediaStreamer::RegisterAudioCallback(mediastream_audiocallback_t cb, bool enable)
{
    if (enable)
        m_audiocallback = cb;
    else
        m_audiocallback = {};
}

bool MediaStreamer::Open(const MediaStreamOutput& out_prop)
{
    if (GetMediaOutput().IsValid())
        return false;

    m_media_out = out_prop;

    m_thread.reset(new std::thread(&MediaStreamer::Run, this));

    bool ret = false;
    m_open.get(ret);

    if (!ret)
        Close();
    
    return ret;
}

void MediaStreamer::Close()
{
    if (m_thread.get())
    {
        m_stop = true;
        m_run.set(false);

        m_thread->join();
        m_thread.reset();
    }
    Reset();

    m_open.cancel();
    m_run.cancel();
}

bool MediaStreamer::StartStream()
{
    m_pause = false;

    // avoid doing a double start
    if (!m_run.ready())
        m_run.set(true);

    return true;
}

bool MediaStreamer::Pause()
{
    MYTRACE(ACE_TEXT("MediaStreamer pausing\n"));

    m_pause = true;

    // only cancel semaphore if it's already active (set). Otherwise a double pause
    // will occur and invalidate the current semaphore (waiting thread on
    // current semaphore will not be notified)
    if (m_run.ready())
        return m_run.cancel() >= 0;

    return true;
}

void MediaStreamer::Reset()
{
    m_media_out = MediaStreamOutput();
    m_stop = m_pause = false;

    m_audio_frames.close();
    m_video_frames.close();
}

bool MediaStreamer::QueueAudio(const media::AudioFrame& frame)
{
    assert(frame.inputfmt == GetMediaOutput().audio);
    ACE_Message_Block* mb = AudioFrameToMsgBlock(frame);
    assert(mb);

    if (!mb)
        return false;

    if (!QueueAudio(mb))
    {
        mb->release();
        return false;
    }
    return true;
}

bool MediaStreamer::QueueAudio(ACE_Message_Block* mb)
{
    ACE_Time_Value zero;
    if(m_audio_frames.enqueue(mb, &zero) >= 0)
        return true;

    MYTRACE(ACE_TEXT("Dropped audio frame %u\n"), media::AudioFrame(mb).timestamp);

    return false;
}

bool MediaStreamer::QueueVideo(const media::VideoFrame& frame)
{
    ACE_Message_Block* mb = VideoFrameToMsgBlock(frame);
    assert(mb);

    if (!mb)
        return false;
    
    ACE_Time_Value zero;
    if (m_video_frames.enqueue(mb, &zero) >= 0)
        return true;
    
    MYTRACE(ACE_TEXT("Dropped video frame %u\n"), frame.timestamp);
    
    mb->release();
    return false;
}

void MediaStreamer::InitBuffers()
{
    assert(m_media_out.HasAudio() || m_media_out.HasVideo());

    if (m_media_out.HasAudio())
    {
        size_t buffer_size = PCM16_BYTES(m_media_out.audio.samplerate *
            BUF_SECS, m_media_out.audio.channels);

        m_audio_frames.low_water_mark(buffer_size);
        m_audio_frames.high_water_mark(buffer_size);
    }

    if (m_media_out.HasVideo())
    {
        int bmp_size = RGB32_BYTES(m_media_out.video.width, m_media_out.video.height);
        int media_frame_size = bmp_size + sizeof(media::VideoFrame);
        size_t fps = m_media_out.video.fps_numerator / std::max(1, m_media_out.video.fps_denominator);

        size_t buffer_size = fps * BUF_SECS * media_frame_size;
        m_video_frames.low_water_mark(buffer_size);
        m_video_frames.high_water_mark(buffer_size);
    }

    int ret = m_audio_frames.activate();
    assert(ret >= 0);
    ret = m_video_frames.activate();
    assert(ret >= 0);
}

void MediaStreamer::ClearBuffers()
{
    m_audio_frames.close();
    m_video_frames.close();
    
    // message_queue::close() doesn't reset message_queue::message_length()
    m_audio_frames.message_length(0);

    int ret = m_audio_frames.activate();
    assert(ret >= 0);
    ret = m_video_frames.activate();
    assert(ret >= 0);
}

ACE_UINT32 MediaStreamer::GetMinimumFrameDurationMSec() const
{
    ACE_UINT32 wait_ms = 1000;
    if(m_media_out.HasAudio() && m_media_out.audio_samples > 0)
    {
        wait_ms = ACE_UINT32(m_media_out.audio_samples * 1000 / m_media_out.audio.samplerate);
    }

    if(m_media_out.HasVideo())
    {
        double fps = std::max(1, m_media_out.video.fps_numerator) / std::max(1, m_media_out.video.fps_denominator);
        wait_ms = ACE_UINT32(std::min(1000. / fps, double(wait_ms)));
    }
    return wait_ms;
}

int MediaStreamer::GetQueuedAudioDataSize()
{
    // ensure size of m_audio_frames.message_length() is correct,
    // i.e. substraction of ACE_Message_Block::length() results in
    // m_audio_frames.message_length() being 0 when
    // m_audio_frames.message_count() is 0.
    assert(m_audio_frames.message_count() || m_audio_frames.message_length() == 0);

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
        queued_audio_bytes -= int(mb->capacity());
        queued_audio_bytes += int(mb->length());
    }
    queued_audio_bytes -= int(hdrs_size);

    return queued_audio_bytes;
}

bool MediaStreamer::ProcessAVQueues(ACE_UINT32 starttime, ACE_UINT32 curtime, bool flush)
{
    assert(m_media_out.HasAudio() || m_media_out.HasVideo());

    bool need_audio = ProcessAudioFrame(starttime, curtime, flush);
    bool need_video = ProcessVideoFrame(starttime, curtime);

    //go to sleep if there is already enough data buffered
    if (!need_audio && !need_video)
    {
        ACE_UINT32 wait_ms = GetMinimumFrameDurationMSec();
        MYTRACE(ACE_TEXT("Sleeping %d msec... waiting for frames\n"), wait_ms);
        ACE_OS::sleep(ACE_Time_Value(wait_ms / 1000, (wait_ms % 1000) * 1000));
        return true;
    }

    if (flush)
    {
        ACE_UINT32 wait_ms = GetMinimumFrameDurationMSec();
        MYTRACE(ACE_TEXT("Sleeping %d msec... flushing frames\n"), wait_ms);
        ACE_OS::sleep(ACE_Time_Value(wait_ms / 1000, (wait_ms % 1000) * 1000));
        return m_audio_frames.message_count() || m_video_frames.message_count();
    }

    return false;
}

bool MediaStreamer::ProcessAudioFrame(ACE_UINT32 starttime, ACE_UINT32 curtime, bool flush)
{
    if (!m_media_out.HasAudio())
        return false;

    //see if audio block is less than time 'now'
    ACE_Time_Value tv;
    ACE_Message_Block* mb;
    if(m_audio_frames.peek_dequeue_head(mb, &tv) < 0)
    {
        MYTRACE(ACE_TEXT("Audio %u - Queue empty\n"), curtime - starttime);
        return true;
    }

    int queued_audio_bytes = GetQueuedAudioDataSize();
    int required_audio_bytes = PCM16_BYTES(m_media_out.audio_samples, m_media_out.audio.channels);
    if (queued_audio_bytes < required_audio_bytes && !flush)
    {
        MYTRACE(ACE_TEXT("Audio %u - Insufficent data\n"), curtime - starttime);
        return true;
    }

    uint32_t queue_duration = PCM16_BYTES_DURATION(queued_audio_bytes, m_media_out.audio.channels,
                                                   m_media_out.audio.samplerate);

    // check if head is already ahead of time
    AudioFrame* first_frame = reinterpret_cast<AudioFrame*>(mb->base());
    MYTRACE(ACE_TEXT("Audio %u - Checking %u. Queue duration: %u, bytes %u\n"),
        curtime - starttime, first_frame->timestamp, queue_duration,
            unsigned(m_audio_frames.message_length()));

    if (W32_GT(first_frame->timestamp, curtime - starttime))
    {
        MYTRACE(ACE_TEXT("Audio %u - Data in future %u\n"),
            curtime - starttime, first_frame->timestamp);
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
                mb = nullptr;
            }

            assert(mb == nullptr);

            //assert(flush || (write_bytes == 0 && m_audio_frames.message_count() == 0 || write_bytes > 0 && m_audio_frames.message_count()));
        }
        else
        {
            // advance
            AudioFrame* head_frame = reinterpret_cast<AudioFrame*>(mb->base());
            assert(m_media_out.audio.channels);
            assert(m_media_out.audio.samplerate);
            head_frame->timestamp += PCM16_BYTES_DURATION(write_bytes,
                                                          m_media_out.audio.channels,
                                                          m_media_out.audio.samplerate);

            int ret = out_mb->copy(mb->rd_ptr(), write_bytes);
            assert(ret >= 0);
            mb->rd_ptr(write_bytes);
            write_bytes -= write_bytes;
            assert(mb->rd_ptr() < mb->end());
        }
        assert((int)write_bytes >= 0);
    }
    while(write_bytes > 0 && m_audio_frames.peek_dequeue_head(mb, &tv) >= 0);

    // MYTRACE(ACE_TEXT("Audio %u - Writebytes %u, audio size %d, q size %u, msg cnt: %u, state: %d\n"),
    //         curtime - starttime, write_bytes, GetQueuedAudioDataSize(),
    //         unsigned(m_audio_frames.message_length()),
    //         unsigned(m_audio_frames.message_count()), m_audio_frames.state());

    assert(flush || write_bytes == 0);

    //write bytes should only be greater than 0 if flushing
    if (write_bytes > 0)
    {
        ACE_OS::memset(out_mb->wr_ptr(), 0, write_bytes);
        out_mb->wr_ptr(write_bytes);
        assert(out_mb->end() == out_mb->wr_ptr());
    }

    uint32_t timestamp = media_frame->timestamp;
    bool need_more = GetQueuedAudioDataSize() < required_audio_bytes;
    MYTRACE(ACE_TEXT("Audio %u - Submitted %u. Diff: %d. Need more %d\n"), curtime - starttime,
            timestamp - starttime, int((curtime - starttime) - (timestamp - starttime)),
            int(need_more));
    //MYTRACE(ACE_TEXT("Ejecting audio frame %u\n"), media_frame.timestamp);
    
    uint32_t newduration = PCM16_BYTES_DURATION(GetQueuedAudioDataSize(),
                                                m_media_out.audio.channels,
                                                m_media_out.audio.samplerate);
    AudioProgress(newduration, timestamp - starttime + media_frame->InputDurationMSec());

    if (!m_audiocallback || !m_audiocallback(*media_frame, out_mb))
    {
        out_mb->release();
        out_mb = NULL;
    }
    //'out_mb' should now be considered dead

    return need_more;
}

bool MediaStreamer::ProcessVideoFrame(ACE_UINT32 starttime, ACE_UINT32 curtime)
{
    if (!m_media_out.HasVideo())
        return false;

    int ret;
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
                curtime - starttime, media_frame->timestamp);

        if (W32_LEQ(media_frame->timestamp, curtime - starttime))
        {
            tm_zero = ACE_Time_Value::zero;
            if ((ret = m_video_frames.dequeue(mb, &tm_zero)) < 0)
            {
                assert(ret >= 0);
                return true; // should never happen
            }

            media_frame->timestamp = starttime + media_frame->timestamp;

            MYTRACE(ACE_TEXT("Video %u - Submitted video frame %u. Diff: %u. Queue: %u\n"),
                    curtime - starttime,
                    media_frame->timestamp - starttime, curtime - media_frame->timestamp,
                    unsigned(m_video_frames.message_count()));

            if (!m_videocallback || !m_videocallback(*media_frame, mb))
            {
                mb->release();
                mb = NULL;
            }
        }
        else
        {
            MYTRACE(ACE_TEXT("Video %u - Not video time %u. Queue: %u\n"),
                    curtime - starttime, media_frame->timestamp, unsigned(m_video_frames.message_count()));
            return false;
        }
    }
    else
    {
        MYTRACE(ACE_TEXT("Video %u - Queue empty\n"), curtime - starttime);
    }


    return m_video_frames.message_count() == 0;
}


void MediaFileStreamer::Reset()
{
    MediaStreamer::Reset();

    m_media_in = MediaFileProp();
    m_offset = MEDIASTREAMER_OFFSET_IGNORE;
}

bool MediaFileStreamer::OpenFile(const ACE_TString& filename,
                                 const MediaStreamOutput& out_prop)
{
    Close();

    m_media_in.filename = filename;

    if (!Open(out_prop))
    {
        Close();
        return false;
    }
    
    return true;
}

void MediaFileStreamer::RegisterStatusCallback(mediastream_statuscallback_t cb, bool enable)
{
    if (enable)
        m_statuscallback = cb;
    else
        m_statuscallback = {};
}

ACE_UINT32 MediaFileStreamer::SetOffset(ACE_UINT32 offset)
{
    std::lock_guard<std::mutex> g(m_mutex);
    auto prev = m_offset;
    m_offset = offset;
    return prev;
}

