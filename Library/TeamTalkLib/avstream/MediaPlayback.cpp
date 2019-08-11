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

#include <codec/MediaUtil.h>

#define PB_FRAMESIZE(samplerate) (samplerate * .1)

MediaPlayback::MediaPlayback(soundsystem::SoundSystem* sndsys)
    : m_sndsys(sndsys)
{
}

MediaPlayback::~MediaPlayback()
{
    m_sndsys->CloseOutputStream(this);

    std::lock_guard<std::mutex> g(m_mutex);
    while (m_audio_buffer.size())
    {
        m_audio_buffer.front()->release();
        m_audio_buffer.pop();
    }

}

bool MediaPlayback::OpenFile(const ACE_TString& filename)
{
    if (m_streamer && m_streamer->GetMediaInput().IsValid())
        return false;

    MediaFileProp inprop;
    if (!GetMediaFileProp(filename, inprop))
        return false;

    MediaStreamOutput outprop(inprop.audio, PB_FRAMESIZE(inprop.audio.samplerate),
                              inprop.video);

    if (m_streamer->OpenFile(inprop, outprop))
        return true;

    m_streamer.reset();
    return false;
}

bool MediaPlayback::OpenSoundSystem(int sndgrpid, int outputdeviceid)
{
    if (!m_streamer)
        return false;

    MediaFileProp inprop = m_streamer->GetMediaInput();
    if (!inprop.HasAudio())
        return false;

    return m_sndsys->OpenOutputStream(this, outputdeviceid, sndgrpid,
                                      inprop.audio.samplerate, inprop.audio.channels,
                                      PB_FRAMESIZE(inprop.audio.samplerate));
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
    std::lock_guard<std::mutex> g(m_mutex);

    if (m_audio_buffer.size() > 10)
    {
        MYTRACE(ACE_TEXT("Media Playback buffer full. Discarding audio frame.\n"));
        return false;
    }

    m_audio_buffer.push(mb_audio);
    return true;
}

void MediaPlayback::MediaStreamStatusCallback(MediaStreamer* streamer,
                                              const MediaFileProp& mfp,
                                              MediaStreamStatus status)
{
    switch (status)
    {
    case MEDIASTREAM_STARTED :
        m_sndsys->StartStream(this);
        break;
    case MEDIASTREAM_ERROR :
    case MEDIASTREAM_FINISHED :
        m_sndsys->CloseOutputStream(this);
        break;
    }
}

bool MediaPlayback::StreamPlayerCb(const soundsystem::OutputStreamer& streamer, 
                                   short* buffer, int samples)
{
    ACE_Message_Block* mb = nullptr;
    {
        std::lock_guard<std::mutex> g(m_mutex);
        if (m_audio_buffer.size())
        {
            mb = m_audio_buffer.front();
            m_audio_buffer.pop();
        }
    }
    if (mb)
    {
        MBGuard gmb(mb);
        media::AudioFrame frm(mb);
        assert(streamer.framesize == samples);
        std::memcpy(buffer, frm.input_buffer, PCM16_BYTES(streamer.channels, streamer.framesize));
    }
}

void MediaPlayback::StreamPlayerCbEnded()
{
}
