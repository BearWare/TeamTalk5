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

#ifndef MEDIASTREAMER_H
#define MEDIASTREAMER_H

#include <ace/ACE.h>
#include <ace/SString.h>
#include <ace/Future.h>

#include <myace/MyACE.h>
#include <codec/MediaUtil.h>

#include <memory>
#include <thread>
#include <mutex>

struct MediaStream
{
    media::AudioFormat audio;
    media::VideoFormat video;

    bool IsValid() const { return HasAudio() || HasVideo(); }
    bool HasAudio() const { return audio.IsValid(); }
    // some video format don't have frame rate information, but they're still valid...
    bool HasVideo() const { return video.IsValid(); /*video.width > 0 && video.height > 0 && video.fps_numerator > 0 && video.fps_denominator > 0;*/ }
};

struct MediaFileProp : public MediaStream
{
    ACE_UINT32 duration_ms = 0, elapsed_ms = 0;
    ACE_TString filename;

    MediaFileProp() { }
    MediaFileProp(const ACE_TString& fname) : filename(fname) { }
};

struct MediaStreamOutput : public MediaStream
{
    int audio_samples = 0;
    MediaStreamOutput() {}

    MediaStreamOutput(const media::AudioFormat& afmt, int asamples) : audio_samples(asamples) { audio = afmt;}
    MediaStreamOutput(const media::VideoFormat& vfmt) { video = vfmt; }
    MediaStreamOutput(const media::AudioFormat& afmt, int asamples, const media::VideoFormat& vfmt)
    : audio_samples(asamples) { audio = afmt; video = vfmt; }
    MediaStreamOutput(const media::AudioFormat& afmt, int asamples, media::FourCC fourcc)
        : audio_samples(asamples)
    {
        audio = afmt;
        // scaling is currently not supported so just set FourCC
        video.fourcc = fourcc;
    }
};

bool GetMediaFileProp(const ACE_TString& filename, MediaFileProp& fileprop);

enum MediaStreamStatus
{
    MEDIASTREAM_NONE        = 0,
    MEDIASTREAM_STARTED     = 1,
    MEDIASTREAM_ERROR       = 2,
    MEDIASTREAM_FINISHED    = 3,
    MEDIASTREAM_PAUSED      = 4,
    MEDIASTREAM_PLAYING     = 5,
};

typedef std::function< bool (media::VideoFrame& video_frame,
                             ACE_Message_Block* mb_video) > mediastream_videocallback_t;

//'audio_frame' is a reference to mb_audio->rd_ptr()
//@return if false 'mb_audio' will be deleted
typedef std::function< bool (media::AudioFrame& audio_frame,
                             ACE_Message_Block* mb_audio) > mediastream_audiocallback_t;

typedef std::function< void (const MediaFileProp& mfp,
                             MediaStreamStatus status) > mediastream_statuscallback_t;


#define MEDIASTREAMER_OFFSET_IGNORE (0xFFFFFFFF)

class MediaStreamer
{
public:
    MediaStreamer() { }
    virtual ~MediaStreamer();

    void RegisterVideoCallback(mediastream_videocallback_t cb, bool enable);
    void RegisterAudioCallback(mediastream_audiocallback_t cb, bool enable);
    
    bool Open(const MediaStreamOutput& out_prop);
    void Close();

    bool StartStream();

    bool Pause();

    virtual const MediaStream& GetMediaInput() const = 0;
    const MediaStreamOutput& GetMediaOutput() const { return m_media_out; }
    
protected:
    virtual void Run() = 0;
    virtual void Reset();

    bool QueueAudio(const media::AudioFrame& frame);
    bool QueueVideo(const media::VideoFrame& frame);
    
    void InitBuffers();
    void ClearBuffers();
    ACE_UINT32 GetMinimumFrameDurationMSec() const;
    int GetQueuedAudioDataSize();

    MediaStreamOutput m_media_out;
    
    std::shared_ptr< std::thread > m_thread;
    ACE_Future<bool> m_open, m_run;
    bool m_pause = false;
    bool m_stop = false;
    
    //return 'true' if it should be called again
    bool ProcessAVQueues(ACE_UINT32 starttime, ACE_UINT32 curtime, bool flush);

private:

    mediastream_videocallback_t m_videocallback;
    mediastream_audiocallback_t m_audiocallback;

    bool ProcessAudioFrame(ACE_UINT32 starttime, ACE_UINT32 curtime, bool flush);
    bool ProcessVideoFrame(ACE_UINT32 starttime, ACE_UINT32 curtime);
    
    msg_queue_t m_audio_frames;
    msg_queue_t m_video_frames;
};

class MediaFileStreamer : public MediaStreamer
{
public:
    bool OpenFile(const ACE_TString& filename,
                  const MediaStreamOutput& out_prop);
    
    void RegisterStatusCallback(mediastream_statuscallback_t cb, bool enable);
    
    // return previous offset (was)
    ACE_UINT32 SetOffset(ACE_UINT32 offset);

    const MediaStream& GetMediaInput() const { return m_media_in; }
    const MediaFileProp& GetMediaFile() const { return m_media_in; }
    
protected:
    virtual void Reset();

    MediaFileProp m_media_in;
    
    mediastream_statuscallback_t m_statuscallback;

    std::mutex m_mutex;
    ACE_UINT32 m_offset = MEDIASTREAMER_OFFSET_IGNORE;
    
};

typedef std::shared_ptr< MediaFileStreamer > mediafile_streamer_t;

mediafile_streamer_t MakeMediaFileStreamer();

#endif
