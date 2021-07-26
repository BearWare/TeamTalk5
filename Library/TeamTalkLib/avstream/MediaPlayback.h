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

#ifndef MEDIAPLAYBACK_H
#define MEDIAPLAYBACK_H

#include "MediaStreamer.h"
#include "SoundSystem.h"
#include "AudioResampler.h"
#include <codec/MediaUtil.h>
#if defined(ENABLE_SPEEXDSP)
#include "SpeexPreprocess.h"
#endif
#if defined(ENABLE_WEBRTC)
#include <avstream/WebRTCPreprocess.h>
#endif

#include <queue>
#include <mutex>
#include <memory>

#define PB_FRAMEDURATION_MSEC 40

typedef std::function< void(int userdata, const MediaFileProp& mfp,
                            MediaStreamStatus status) > mediaplayback_status_t;
typedef std::function< void(int userdata, const media::AudioFrame& frm) > mediaplayback_audio_t;

class MediaPlayback : public soundsystem::StreamPlayer
{
public:
    MediaPlayback(int userdata, soundsystem::soundsystem_t sndsys,
                  mediaplayback_status_t statusfunc,
                  mediaplayback_audio_t audiofunc);
    ~MediaPlayback();
    
    bool OpenFile(const ACE_TString& filename);

    bool OpenSoundSystem(int sndgrpid, int outputdeviceid, bool speexdsp = false);

    bool PlayMedia();

    bool Pause();

    bool Seek(ACE_UINT32 offset);

    // required by TeamTalkAudioPreprocessor
    void MuteSound(bool left, bool right);
    void SetGainLevel(int gainlevel = GAIN_NORMAL) { m_gainlevel = gainlevel; }

#if defined(ENABLE_SPEEXDSP)
    bool SetupSpeexPreprocess(bool enableagc, const SpeexAGC& agc,
                              bool enabledenoise, int denoisesuppress);
#endif
#if defined(ENABLE_WEBRTC)
    bool SetupWebRTCPreprocess(const webrtc::AudioProcessing::Config& webrtc);
#endif
    // MediaStreamListener
    bool MediaStreamVideoCallback(media::VideoFrame& video_frame,
                                  ACE_Message_Block* mb_video);

    bool MediaStreamAudioCallback(media::AudioFrame& audio_frame,
                                  ACE_Message_Block* mb_audio);
    
    void MediaStreamStatusCallback(const MediaFileProp& mfp,
                                   MediaStreamStatus status);

    MediaStreamStatus GetStatus() const;
    bool Flushed();

    // StreamPlayer
    bool StreamPlayerCb(const soundsystem::OutputStreamer& streamer, 
                        short* buffer, int samples);

private:
    soundsystem::soundsystem_t m_sndsys;
    mediafile_streamer_t m_streamer;
    mediaplayback_status_t m_statusfunc;
    mediaplayback_audio_t m_audiofunc;
    int m_userdata = 0;

    MediaStreamStatus m_status = MEDIASTREAM_NONE;
    struct MediaFileProgress
    {
        MediaStreamStatus status = MEDIASTREAM_NONE;
        MediaFileProp mfp;
        MediaFileProgress() {}
        MediaFileProgress(MediaStreamStatus s,
                          const MediaFileProp& m) : status(s), mfp(m) {}
    };

    /*
     * Publish status to m_statusfunc():
     * MEDIASTREAM_PLAYING
     * MEDIASTREAM_STARTED
     * */
    void SubmitPreProgress();
    /*
     * Publish status to m_statusfunc():
     * MEDIASTREAM_FINISHED
     * MEDIASTREAM_ERROR
     * MEDIASTREAM_PAUSED
     * */
    void SubmitPostProgress();

    std::queue<MediaFileProgress> m_progress;
    std::queue<ACE_Message_Block*> m_audio_buffer;
    uint32_t m_sampleindex = 0;

    int m_gainlevel = GAIN_NORMAL;
    audio_resampler_t m_resampler;
    std::mutex m_mutex;
    StereoMask m_stereo = STEREO_BOTH;
#if defined(ENABLE_SPEEXDSP)
    std::shared_ptr<SpeexPreprocess> m_preprocess_left, m_preprocess_right;
#endif
#if defined(ENABLE_WEBRTC)
    std::unique_ptr<webrtc::AudioProcessing> m_apm;
#endif
    bool m_last_callback = false;
    ACE_Future<bool> m_drained;
};

typedef std::shared_ptr<MediaPlayback> mediaplayback_t;

#endif
