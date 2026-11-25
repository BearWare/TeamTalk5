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
#include "codec/MediaUtil.h"
#include "mystd/MyStd.h"

#if defined(ENABLE_MEDIAFOUNDATION)
#include "MFStreamer.h"
#endif /* ENABLE_MEDIAFOUNDATION */

#if defined(ENABLE_DSHOW)
#include "WinMedia.h"
#endif /* ENABLE_DSHOW */

#if defined(ENABLE_FFMPEG)
#include "FFmpegStreamer.h"
#endif /* ENABLE_FFMPEG*/

#if defined(WIN32) && defined(ENABLE_OPUSTOOLS) && defined(ENABLE_OPUS)
#include "OpusFileStreamer.h"
#endif /* ENABLE_OPUSTOOLS && ENABLE_OPUS */

#include <algorithm>
#include <chrono>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

using namespace media;

bool GetMediaFileProp(const tstring& filename, MediaFileProp& fileprop)
{
#if defined(WIN32) && defined(ENABLE_OPUSTOOLS) && defined(ENABLE_OPUS)
    if (GetOpusFileMediaFileProp(filename, fileprop))
        return true;
#endif

#if defined(ENABLE_FFMPEG) && defined(ENABLE_MEDIAFOUNDATION)
    if (GetAVMediaFileProp(filename, fileprop))
        return true;
    return GetMFMediaFileProp(filename, fileprop);
#elif defined(ENABLE_MEDIAFOUNDATION)
    return GetMFMediaFileProp(filename, fileprop);
#elif defined(ENABLE_DSHOW)
    return GetDSMediaFileProp(filename, fileprop);
#elif defined(ENABLE_FFMPEG)
    return GetAVMediaFileProp(filename, fileprop);
#endif
    return false;
}

mediafile_streamer_t MakeMediaFileStreamer(const tstring& filename, const MediaStreamOutput& out_prop)
{
    mediafile_streamer_t streamer;

#if defined(WIN32) && defined(ENABLE_OPUSTOOLS) && defined(ENABLE_OPUS)
    MediaFileProp fileprop;
    if (GetOpusFileMediaFileProp(filename, fileprop))
        return mediafile_streamer_t(new OpusFileStreamer(filename, out_prop));
#endif

#if defined(ENABLE_FFMPEG) && defined(ENABLE_MEDIAFOUNDATION)
    MediaFileProp ffmpegProp;
    if (GetAVMediaFileProp(filename, ffmpegProp))
        streamer = std::make_shared<FFmpegStreamer>(filename, out_prop);
    else
        streamer.reset(new MFStreamer(filename, out_prop));
#elif defined(ENABLE_MEDIAFOUNDATION)
    streamer.reset(new MFStreamer(filename, out_prop));
#elif defined(ENABLE_DSHOW)
    streamer.reset(new DSWrapperThread());
#elif defined(ENABLE_FFMPEG)
    streamer = std::make_shared<FFmpegStreamer>(filename, out_prop);
#endif

    return streamer;
}

MediaStreamer::MediaStreamer(const MediaStreamOutput& out_prop) : m_media_out(out_prop)
{
}

MediaStreamer::~MediaStreamer()
{
    // subclasses must call ::Close() manually to finish their ::Run() method
    assert(!m_thread);
    MYTRACE(TTEXT("~MediaStreamer()\n"));
}

void MediaStreamer::RegisterVideoCallback(mediastream_videocallback_t cb, bool enable)
{
    if (enable)
        m_videocallback = std::move(cb);
    else
        m_videocallback = {};
}

void MediaStreamer::RegisterAudioCallback(mediastream_audiocallback_t cb, bool enable)
{
    if (enable)
        m_audiocallback = std::move(cb);
    else
        m_audiocallback = {};
}

bool MediaStreamer::Open()
{
    assert(!m_thread);
    MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("MediaStreamer %p opening\n"), this);
    m_thread = std::make_shared<std::thread>(&MediaStreamer::Run, this);

    bool ret = false;
    m_open.get(ret);

    if (!ret)
        Close();
    
    return ret;
}

void MediaStreamer::Close()
{
    if (m_thread)
    {
        m_stop = true;
        m_run.set(false);

        m_thread->join();
        m_thread.reset();
    }

    m_open.cancel();
    m_run.cancel();
}

bool MediaStreamer::StartStream()
{
    MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("MediaStreamer %p starting\n"), this);
    m_pause = false;

    // avoid doing a double start
    if (m_run.ready() == 0)
        m_run.set(true);

    return true;
}

bool MediaStreamer::Pause()
{
    MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("MediaStreamer %p pausing\n"), this);

    m_pause = true;

    // only cancel semaphore if it's already active (set). Otherwise a double pause
    // will occur and invalidate the current semaphore (waiting thread on
    // current semaphore will not be notified)
    if (m_run.ready() != 0)
        return m_run.cancel() >= 0;

    return true;
}

bool MediaStreamer::QueueAudio(const media::AudioFrame& frame)
{
    assert(frame.inputfmt == GetMediaOutput().audio);
    auto mb = AudioFrameToMsgBlock(frame);
    assert(mb);

    if (mb == nullptr)
        return false;

    return QueueAudio(std::move(mb));
}

bool MediaStreamer::QueueAudio(std::unique_ptr<MsgBlock> mb)
{
    if(m_audio_frames.Put(std::move(mb)))
    {
        MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Queue audio frame ts: %u\n"), media::AudioFrame(mb.get()).timestamp);
        return true;
    }
    MYTRACE(TTEXT("Dropped audio frame\n"));

    return false;
}

bool MediaStreamer::QueueVideo(const media::VideoFrame& frame)
{
    auto mb = VideoFrameToMsgBlock(frame);
    assert(mb);

    if (mb == nullptr)
        return false;

    if (m_video_frames.Put(std::move(mb)))
    {
        MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Queue video frame ts: %u\n"), frame.timestamp);
        return true;
    }
    MYTRACE(TTEXT("Dropped video frame ts: %u\n"), frame.timestamp);

    return false;
}

void MediaStreamer::InitBuffers()
{
    assert(m_media_out.HasAudio() || m_media_out.HasVideo());

    if (m_media_out.HasAudio())
    {
        size_t const buffer_size = PCM16_BYTES(m_media_out.audio.samplerate *
            BUFFER_DURATION_SECONDS, m_media_out.audio.channels);

        m_audio_frames.Activate();
        m_audio_frames.SetWaterMark(buffer_size);
    }

    if (m_media_out.HasVideo())
    {
        size_t const bmp_size = RGB32_BYTES(m_media_out.video.width, m_media_out.video.height);
        size_t const media_frame_size = bmp_size + sizeof(media::VideoFrame);
        size_t const fps = m_media_out.video.fps_numerator / std::max(1, m_media_out.video.fps_denominator);

        size_t const buffer_size = fps * BUFFER_DURATION_SECONDS * media_frame_size;
        m_video_frames.Activate();
        m_video_frames.SetWaterMark(buffer_size);
    }
}

void MediaStreamer::ClearBuffers()
{
    m_audio_frames.Close();
    m_video_frames.Close();

    m_audio_frames.Activate();
    m_video_frames.Activate();
}

uint32_t MediaStreamer::GetMinimumFrameDurationMSec() const
{
    uint32_t wait_ms = 1000;
    if(m_media_out.HasAudio() && m_media_out.audio_samples > 0)
    {
        wait_ms = static_cast<uint32_t>(m_media_out.audio_samples * 1000 / m_media_out.audio.samplerate);
    }

    if(m_media_out.HasVideo())
    {
        double const fps = std::max(1, m_media_out.video.fps_numerator) / std::max(1, m_media_out.video.fps_denominator);
        wait_ms = static_cast<uint32_t>(std::min(1000. / fps, static_cast<double>(wait_ms)));
    }
    return wait_ms;
}

int MediaStreamer::GetQueuedAudioDataSize()
{
    // ensure size of m_audio_frames.BytesQueued() is correct,
    // i.e. substraction of ACE_Message_Block::length() results in
    // m_audio_frames.BytesQueued() being 0 when
    // m_audio_frames.Size() is 0.
    assert(m_audio_frames.Size() || m_audio_frames.BytesQueued() == 0);

    MsgBlock* const mb = m_audio_frames.PeekHead();
    if(mb == nullptr)
    {
        return 0;
    }

    // calculate if we can build a frame. 'message_length()' is data available
    // in message queue which has been written but hasn't been read yet.
    // 'message_bytes()' is total amount of data allocated in the message queue.
    size_t hdrs_size = m_audio_frames.Size() * sizeof(AudioFrame);
    //if we have already read some data of a block we need to substract its header
    int queued_audio_bytes = static_cast<int>(m_audio_frames.BytesQueued());
    if(mb->RdPtr() != mb->Base())
    {
        hdrs_size -= sizeof(AudioFrame);
        queued_audio_bytes -= static_cast<int>(mb->Capacity());
        queued_audio_bytes += static_cast<int>(mb->Length());
    }
    queued_audio_bytes -= static_cast<int>(hdrs_size);

    return queued_audio_bytes;
}

bool MediaStreamer::ProcessAVQueues(uint32_t starttime, uint32_t curtime, bool flush)
{
    assert(m_media_out.HasAudio() || m_media_out.HasVideo());

    bool const need_audio = ProcessAudioFrame(starttime, curtime, flush);
    bool const need_video = ProcessVideoFrame(starttime, curtime);

    //go to sleep if there is already enough data buffered
    if (!need_audio && !need_video)
    {
        uint32_t const wait_ms = GetMinimumFrameDurationMSec();
        MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Sleeping %d msec... waiting for frames\n"), wait_ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
        return true;
    }

    if (flush)
    {
        uint32_t const wait_ms = GetMinimumFrameDurationMSec();
        MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Sleeping %d msec... flushing frames\n"), wait_ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
        return (m_audio_frames.Size() != 0u) || (m_video_frames.Size() != 0u);
    }

    return false;
}

bool MediaStreamer::ProcessAudioFrame(uint32_t starttime, uint32_t curtime, bool flush)
{
    if (!m_media_out.HasAudio())
        return false;

    //see if audio block is less than time 'now'
    MsgBlock* mb = m_audio_frames.PeekHead();
    if(mb == nullptr)
    {
        MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Audio ts: %u - Queue empty\n"), curtime - starttime);
        return true;
    }

    int const queued_audio_bytes = GetQueuedAudioDataSize();
    int const required_audio_bytes = PCM16_BYTES(m_media_out.audio_samples, m_media_out.audio.channels);
    if (queued_audio_bytes < required_audio_bytes && !flush)
    {
        MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Audio ts: %u - Insufficent data\n"), curtime - starttime);
        return true;
    }

    uint32_t const queue_duration = PCM16_BYTES_DURATION(queued_audio_bytes, m_media_out.audio.channels,
                                                   m_media_out.audio.samplerate);

    // check if head is already ahead of time
    auto* first_frame = reinterpret_cast<AudioFrame*>(mb->Base());
    MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Audio ts: %u - Checking ts: %u. Queue duration: %u msec, bytes %u\n"),
        curtime - starttime, first_frame->timestamp, queue_duration,
            unsigned(m_audio_frames.BytesQueued()));

    if (W32_GT(first_frame->timestamp, curtime - starttime))
    {
        MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Audio ts: %u - Data in future ts: %u\n"),
            curtime - starttime, first_frame->timestamp);
        return false;
    }

    // ready to submit new data
    int const audio_block_size = required_audio_bytes + sizeof(AudioFrame);
    auto out_mb = std::make_unique<MsgBlock>(audio_block_size);

    auto* media_frame = reinterpret_cast<AudioFrame*>(out_mb->WrPtr());
    *media_frame = *first_frame; //use original AudioFrame as base for construction

    assert(media_frame->inputfmt == m_media_out.audio);

    //set output properties
    media_frame->timestamp = first_frame->timestamp + starttime;
    media_frame->input_buffer =
        reinterpret_cast<short*>(out_mb->WrPtr() + sizeof(AudioFrame));
    media_frame->input_samples = m_media_out.audio_samples;
    //advance write pointer past header
    out_mb->WrPtr(sizeof(*media_frame));

    int write_bytes = required_audio_bytes;
    do
    {
        //check if we should advance past header
        if(mb->RdPtr() == mb->Base())
            mb->RdPtr(sizeof(AudioFrame));

        if (std::cmp_less_equal(mb->Length(), write_bytes))
        {
            out_mb->Copy(mb->RdPtr(), mb->Length());
            write_bytes -= static_cast<int>(mb->Length());
            assert(write_bytes >= 0);
            // ensure entire message_block's length is subtracted from message queue's length
            mb->RdPtr(mb->Base());
            std::unique_ptr<MsgBlock> done;
            m_audio_frames.Get(done); //remove head (== mb)
            mb = m_audio_frames.PeekHead();
        }
        else
        {
            // advance
            auto* head_frame = reinterpret_cast<AudioFrame*>(mb->Base());
            assert(m_media_out.audio.channels);
            assert(m_media_out.audio.samplerate);
            head_frame->timestamp += PCM16_BYTES_DURATION(write_bytes,
                                                          m_media_out.audio.channels,
                                                          m_media_out.audio.samplerate);

            out_mb->Copy(mb->RdPtr(), write_bytes);
            mb->RdPtr(write_bytes);
            write_bytes -= write_bytes;
            assert(mb->RdPtr() < mb->End());
        }
        assert(write_bytes >= 0);
    }
    while(write_bytes > 0 && m_audio_frames.PeekHead() != nullptr);

    // MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Audio %u - Writebytes %u, audio size %d, q size %u, msg cnt: %u, state: %d\n"),
    //         curtime - starttime, write_bytes, GetQueuedAudioDataSize(),
    //         unsigned(m_audio_frames.BytesQueued()),
    //         unsigned(m_audio_frames.Size()), m_audio_frames.state());

    assert(flush || write_bytes == 0);

    //write bytes should only be greater than 0 if flushing
    if (write_bytes > 0)
    {
        std::memset(out_mb->WrPtr(), 0, write_bytes);
        out_mb->WrPtr(write_bytes);
        assert(out_mb->End() == out_mb->WrPtr());
    }

    uint32_t const timestamp = media_frame->timestamp;
    bool const need_more = GetQueuedAudioDataSize() < required_audio_bytes;
    MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Audio ts: %u - Submitted ts: %u. Diff ts: %d. Need more: %d\n"), curtime - starttime,
            timestamp - starttime, int((curtime - starttime) - (timestamp - starttime)),
            int(need_more));
    //MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Ejecting audio frame %u\n"), media_frame.timestamp);
    
    uint32_t const newduration = PCM16_BYTES_DURATION(GetQueuedAudioDataSize(),
                                                m_media_out.audio.channels,
                                                m_media_out.audio.samplerate);
    AudioProgress(newduration, timestamp - starttime + media_frame->InputDurationMSec());

    if (m_audiocallback)
        m_audiocallback(*media_frame, std::move(out_mb));
    //'out_mb' should now be considered dead

    return need_more;
}

bool MediaStreamer::ProcessVideoFrame(uint32_t starttime, uint32_t curtime)
{
    if (!m_media_out.HasVideo())
        return false;

    std::unique_ptr<MsgBlock> mb;
    if (m_video_frames.PeekHead() != nullptr)
    {
        auto* media_frame = reinterpret_cast<VideoFrame*>(m_video_frames.PeekHead()->RdPtr());
        MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Video ts: %u - First ts: %u\n"),
                curtime - starttime, media_frame->timestamp);

        if (W32_LEQ(media_frame->timestamp, curtime - starttime))
        {
            if (!m_video_frames.Get(mb))
            {
                return true; // should never happen
            }
            media_frame = reinterpret_cast<VideoFrame*>(mb->RdPtr());

            media_frame->timestamp = starttime + media_frame->timestamp;

            MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Video ts: %u - Submitted video frame ts: %u. Diff: %u. Queue count: %u\n"),
                    curtime - starttime,
                    media_frame->timestamp - starttime, curtime - media_frame->timestamp,
                    unsigned(m_video_frames.Size()));

            if (m_videocallback)
                m_videocallback(*media_frame, std::move(mb));
        }
        else
        {
            MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Video ts: %u - Not video ts: %u. Queue count: %u\n"),
                    curtime - starttime, media_frame->timestamp, unsigned(m_video_frames.Size()));
            return false;
        }
    }
    else
    {
        MYTRACE_COND(DEBUG_MEDIASTREAMER, TTEXT("Video ts: %u - Queue empty\n"), curtime - starttime);
    }


    return m_video_frames.Size() == 0;
}

MediaFileStreamer::MediaFileStreamer(const tstring& filename,
                                     const MediaStreamOutput& out_prop)
: MediaStreamer(out_prop)
{
    m_media_in.filename = filename;
}

void MediaFileStreamer::RegisterStatusCallback(mediastream_statuscallback_t cb, bool enable)
{
    if (enable)
        m_statuscallback = std::move(cb);
    else
        m_statuscallback = {};
}

bool MediaFileStreamer::Completed() const
{
    MYTRACE_COND(DEBUG_MEDIASTREAMER && !m_thread, TTEXT("Completed %s - thread closed\n"), GetMediaFile().filename.c_str());
    MYTRACE_COND(DEBUG_MEDIASTREAMER && m_completed, TTEXT("Completed %s - Player reported complete\n"), GetMediaFile().filename.c_str());
    return !m_thread || m_completed;
}

uint32_t MediaFileStreamer::SetOffset(uint32_t offset)
{
    std::scoped_lock const g(m_mutex);
    auto prev = m_offset;
    m_offset = offset;
    return prev;
}
