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

#include <cstring>
#include <algorithm>

#define PB_FRAMEDURATION_MSEC 40
#define PB_FRAMESIZE(samplerate) (samplerate * (PB_FRAMEDURATION_MSEC / 1000.0))

MediaPlayback::MediaPlayback(mediaplayback_status_t statusfunc,
                             int userdata,
                             soundsystem::soundsystem_t sndsys)
    : m_statusfunc(statusfunc)
    , m_userdata(userdata)
    , m_sndsys(sndsys)
{
}

MediaPlayback::~MediaPlayback()
{
    bool wait = false;
    {
        std::lock_guard<std::mutex> g(m_mutex);
        wait = m_finished;
    }

    if (wait)
    {
        // block to ensure all audio has been played
        m_drained.get(wait);
    }

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

    MediaStreamOutput outprop(inprop.audio, int(PB_FRAMESIZE(inprop.audio.samplerate)),
                              inprop.video);

    m_streamer = MakeMediaStreamer(this);
    if (m_streamer && m_streamer->OpenFile(inprop, outprop))
        return true;

    m_streamer.reset();
    return false;
}

bool MediaPlayback::OpenSoundSystem(int sndgrpid, int outputdeviceid, bool speexdsp /*= false*/)
{
    if (!m_streamer)
        return false;

    MediaFileProp inprop = m_streamer->GetMediaInput();
    if (!inprop.HasAudio())
        return false;

    int inframesize = int(PB_FRAMESIZE(inprop.audio.samplerate));
    int outframesize = inframesize;
    media::AudioFormat outformat(inprop.audio.samplerate, inprop.audio.channels);

    if (!m_sndsys->SupportsOutputFormat(outputdeviceid, inprop.audio.channels, inprop.audio.samplerate))
    {
        soundsystem::DeviceInfo devinfo;
        if (!m_sndsys->GetDevice(outputdeviceid, devinfo))
            return false;
        
        outframesize = CalcSamples(inprop.audio.samplerate, inframesize,
                                   devinfo.default_samplerate);
        int outchannels = std::min(devinfo.max_output_channels, inprop.audio.channels);
        outformat = media::AudioFormat(devinfo.default_samplerate, outchannels);

        m_resampler = MakeAudioResampler(inprop.audio, outformat, inframesize);
    }

#if defined(ENABLE_SPEEXDSP)
    if (speexdsp)
    {
        m_preprocess_left.reset(new SpeexPreprocess());
        if (!m_preprocess_left->Initialize(outformat.samplerate, outframesize))
            return false;

        m_preprocess_left->EnableDenoise(false);

        if (outformat.channels == 2)
        {
            m_preprocess_right.reset(new SpeexPreprocess());
            if(!m_preprocess_right->Initialize(outformat.samplerate, outframesize))
                return false;

            m_preprocess_right->EnableDenoise(false);
        }
    }
#endif

    return m_sndsys->OpenOutputStream(this, outputdeviceid, sndgrpid,
                                      outformat.samplerate, outformat.channels, outframesize);
}

bool MediaPlayback::PlayMedia()
{
    if (!m_streamer)
        return false;

    return m_streamer->StartStream();
}

bool MediaPlayback::Pause()
{
    if(!m_streamer)
        return false;

    return m_streamer->Pause();
}

bool MediaPlayback::Seek(ACE_UINT32 offset)
{
    if(!m_streamer)
        return false;

    m_streamer->SetOffset(offset);
    return true;
}

void MediaPlayback::MuteSound(bool leftchannel, bool rightchannel)
{
    m_stereo = ToStereoMask(leftchannel, rightchannel);
}

#if defined(ENABLE_SPEEXDSP)
bool MediaPlayback::SetupSpeexPreprocess(bool enableagc, const SpeexAGC& agc,
                                         bool enabledenoise, int denoisesuppress)
{
    if (!m_preprocess_left)
        return false;

    m_preprocess_left->EnableAGC(enableagc);
    m_preprocess_left->SetAGCSettings(agc);
    m_preprocess_left->EnableDenoise(enabledenoise);
    m_preprocess_left->SetDenoiseLevel(denoisesuppress);
    if (m_preprocess_right)
    {
        m_preprocess_right->EnableAGC(enableagc);
        m_preprocess_right->SetAGCSettings(agc);
        m_preprocess_right->EnableDenoise(enabledenoise);
        m_preprocess_right->SetDenoiseLevel(denoisesuppress);
    }
    return true;
}
#endif

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
    assert(!m_finished);
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
        if (m_sndsys->IsStreamStopped(this))
        {
            if (!m_sndsys->StartStream(this))
            {
                status = MEDIASTREAM_ERROR;
                m_drained.set(true); // ensure we don't wait for 'finished'
            }
        }
        break;
    case MEDIASTREAM_ERROR :
        break;
    case MEDIASTREAM_PAUSED :
        break;
    case MEDIASTREAM_FINISHED:
    {
        std::lock_guard<std::mutex> g(m_mutex);
        m_finished = true;
        break;
    }
    case MEDIASTREAM_NONE :
    case MEDIASTREAM_PLAYING :
        break;
    }

    if(m_statusfunc)
        m_statusfunc(m_userdata, mfp, status);
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
        else if (m_finished)
            m_drained.set(true);
    }

    MYTRACE_COND(!mb, ACE_TEXT("Media playback underflow\n"));

    if (mb)
    {
        MBGuard gmb(mb);
        media::AudioFrame frm(mb);

        if (m_resampler)
        {
            int n_resampled = m_resampler->Resample(frm.input_buffer, buffer);
            assert(n_resampled <= samples);
        }
        else
        {
            assert(streamer.framesize == samples);
            std::memcpy(buffer, frm.input_buffer, PCM16_BYTES(streamer.channels, streamer.framesize));
        }

        SOFTGAIN(buffer, streamer.framesize, streamer.channels, m_gainlevel, GAIN_NORMAL);

#if defined(ENABLE_SPEEXDSP)
        if (m_preprocess_left && streamer.channels == 1)
        {
            if (m_preprocess_left->IsDenoising() || m_preprocess_left->IsAGC())
                m_preprocess_left->Preprocess(buffer);
        }
        else if (streamer.channels == 2 && m_preprocess_left)
        {
            assert(m_preprocess_right);
            assert(m_preprocess_left->IsDenoising() == m_preprocess_right->IsDenoising());
            assert(m_preprocess_left->IsAGC() == m_preprocess_right->IsAGC());

            if(m_preprocess_left->IsDenoising() || m_preprocess_left->IsAGC())
            {
                std::vector<short> in_leftchan(streamer.framesize), in_rightchan(streamer.framesize);
                SplitStereo(buffer, streamer.framesize, in_leftchan, in_rightchan);

                m_preprocess_left->Preprocess(&in_leftchan[0]); //denoise, AGC, etc
                m_preprocess_right->Preprocess(&in_rightchan[0]); //denoise, AGC, etc
                MergeStereo(in_leftchan, in_rightchan, buffer, streamer.framesize);
            }
        }
#endif

        // mute left or right speaker (if enabled)
        if (streamer.channels == 2)
            SelectStereo(m_stereo, buffer, streamer.framesize);
    }
    else
    {
        std::memset(buffer, 0, PCM16_BYTES(streamer.channels, streamer.framesize));
    }
    
    return true;
}

