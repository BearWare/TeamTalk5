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
};

class MediaStreamer;

class MediaStreamListener
{
public:
    virtual ~MediaStreamListener() {}
    virtual bool MediaStreamVideoCallback(MediaStreamer* streamer,
                                          media::VideoFrame& video_frame,
                                          ACE_Message_Block* mb_video) = 0;
    //'audio_frame' is a reference to mb_audio->rd_ptr()
    //@return if false 'mb_audio' will be deleted
    virtual bool MediaStreamAudioCallback(MediaStreamer* streamer,
                                          media::AudioFrame& audio_frame,
                                          ACE_Message_Block* mb_audio) = 0;
    virtual void MediaStreamStatusCallback(MediaStreamer* streamer,
                                           const MediaFileProp& mfp,
                                           MediaStreamStatus status) = 0;
};

#define MEDIASTREAMER_OFFSET_IGNORE (0xFFFFFFFF)

class MediaStreamer
{
public:
    MediaStreamer(MediaStreamListener* listener) 
        : m_listener(listener) { }
    virtual ~MediaStreamer();
    
    bool OpenFile(const MediaFileProp& in_prop,
                  const MediaStreamOutput& out_prop);
    void Close();

    bool StartStream();

    bool Pause();

    void SetOffset(ACE_UINT32 offset) { m_offset = offset; }

    const MediaFileProp& GetMediaInput() const { return m_media_in; }
    const MediaStreamOutput& GetMediaOutput() const { return m_media_out; }

protected:
    virtual void Run() = 0;
    void Reset();
    void InitBuffers();
    void ClearBuffers();
    ACE_UINT32 GetMinimumFrameDurationMSec() const;
    int GetQueuedAudioDataSize();

    MediaFileProp m_media_in;
    ACE_UINT32 m_offset = MEDIASTREAMER_OFFSET_IGNORE;
    MediaStreamOutput m_media_out;
    MediaStreamListener* m_listener;

    std::shared_ptr< std::thread > m_thread;
    ACE_Future<bool> m_open, m_run;
    bool m_pause = false;
    bool m_stop = false;
    
    //return 'true' if it should be called again
    bool ProcessAVQueues(ACE_UINT32 starttime, ACE_UINT32 curtime, bool flush);

    msg_queue_t m_audio_frames;
    msg_queue_t m_video_frames;

private:
    bool ProcessAudioFrame(ACE_UINT32 starttime, ACE_UINT32 curtime, bool flush);
    bool ProcessVideoFrame(ACE_UINT32 starttime, ACE_UINT32 curtime);
};

typedef std::shared_ptr< MediaStreamer > media_streamer_t;

media_streamer_t MakeMediaStreamer(MediaStreamListener* listener);

#endif
