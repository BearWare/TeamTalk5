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

#include "MediaPlayback.h"

MediaPlayback::MediaPlayback()
{
}

bool MediaPlayback::OpenFile(const ACE_TString& filename)
{
    if (m_streamer && m_streamer->GetMediaInput().IsValid())
        return false;
}

bool MediaPlayback::OpenSoundSystem(soundsystem::SoundSystem& sndsys,
                                    int sndgrpid, int outputdeviceid)
{
    return false;
}

bool MediaPlayback::PlayMedia()
{
    return false;
}

bool MediaPlayback::MediaStreamVideoCallback(MediaStreamer* streamer,
                                             media::VideoFrame& video_frame,
                                             ACE_Message_Block* mb_video)
{
    return false;
}

bool MediaPlayback::MediaStreamAudioCallback(MediaStreamer* streamer,
                                             media::AudioFrame& audio_frame,
                                             ACE_Message_Block* mb_audio)
{
    return false;
}
    
void MediaPlayback::MediaStreamStatusCallback(MediaStreamer* streamer,
                                              const MediaFileProp& mfp,
                                              MediaStreamStatus status)
{
}
