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
#include <codec/MediaUtil.h>

#include <queue>
#include <mutex>
#include <memory>

typedef std::function< void(int userdata, const MediaFileProp& mfp,
                            MediaStreamStatus status) > mediaplayback_status_t;

class MediaPlayback : public MediaStreamListener
                    , public soundsystem::StreamPlayer
{
public:
    MediaPlayback(mediaplayback_status_t statusfunc,
                  int userdata,
                  soundsystem::SoundSystem* sndsys);
    ~MediaPlayback();
    
    bool OpenFile(const ACE_TString& filename);

    bool OpenSoundSystem(int sndgrpid, int outputdeviceid);

    bool PlayMedia();

    void MuteSound(bool left, bool right);
    void SetGainLevel(int gainlevel = GAIN_NORMAL) { m_gainlevel = gainlevel; }

    // MediaStreamListener
    bool MediaStreamVideoCallback(MediaStreamer* streamer,
                                  media::VideoFrame& video_frame,
                                  ACE_Message_Block* mb_video);

    bool MediaStreamAudioCallback(MediaStreamer* streamer,
                                  media::AudioFrame& audio_frame,
                                  ACE_Message_Block* mb_audio);
    
    void MediaStreamStatusCallback(MediaStreamer* streamer,
                                   const MediaFileProp& mfp,
                                   MediaStreamStatus status);

    // StreamPlayer
    bool StreamPlayerCb(const soundsystem::OutputStreamer& streamer, 
                        short* buffer, int samples);
    void StreamPlayerCbEnded();
    
private:
    media_streamer_t m_streamer;
    mediaplayback_status_t m_statusfunc;
    int m_userdata = 0;
    int m_gainlevel = GAIN_NORMAL;
    soundsystem::SoundSystem* m_sndsys;
    std::queue<ACE_Message_Block*> m_audio_buffer;
    std::mutex m_mutex;
    StereoMask m_stereo = STEREO_BOTH;
};

typedef std::shared_ptr<MediaPlayback> mediaplayback_t;

#endif
