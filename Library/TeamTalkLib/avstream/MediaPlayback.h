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

#include <queue>
#include <mutex>
#include <memory>

typedef std::function< void(int userdata, const MediaFileProp& mfp,
                            MediaStreamStatus status) > mediaplayback_status_t;

class MediaPlayback : public soundsystem::StreamPlayer
{
public:
    MediaPlayback(mediaplayback_status_t statusfunc,
                  int userdata,
                  soundsystem::soundsystem_t sndsys);
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

    // MediaStreamListener
    bool MediaStreamVideoCallback(media::VideoFrame& video_frame,
                                  ACE_Message_Block* mb_video);

    bool MediaStreamAudioCallback(media::AudioFrame& audio_frame,
                                  ACE_Message_Block* mb_audio);
    
    void MediaStreamStatusCallback(const MediaFileProp& mfp,
                                   MediaStreamStatus status);

    // StreamPlayer
    bool StreamPlayerCb(const soundsystem::OutputStreamer& streamer, 
                        short* buffer, int samples);
    
private:
    mediafile_streamer_t m_streamer;
    mediaplayback_status_t m_statusfunc;
    int m_userdata = 0;
    int m_gainlevel = GAIN_NORMAL;
    soundsystem::soundsystem_t m_sndsys;
    audio_resampler_t m_resampler;
    std::queue<ACE_Message_Block*> m_audio_buffer;
    std::mutex m_mutex;
    StereoMask m_stereo = STEREO_BOTH;
#if defined(ENABLE_SPEEXDSP)
    std::shared_ptr<SpeexPreprocess> m_preprocess_left, m_preprocess_right;
#endif
    bool m_finished = false;
    ACE_Future<bool> m_drained;
};

typedef std::shared_ptr<MediaPlayback> mediaplayback_t;

#endif
