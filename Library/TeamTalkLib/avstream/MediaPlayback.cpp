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

using namespace std::placeholders;

#define DEBUG_MEDIAPLAYBACK 0

MediaPlayback::MediaPlayback(int userdata, soundsystem::soundsystem_t sndsys,
                             mediaplayback_status_t statusfunc,
                             mediaplayback_audio_t audiofunc)
    : m_statusfunc(statusfunc)
    , m_audiofunc(audiofunc)
    , m_userdata(userdata)
    , m_sndsys(sndsys)
{
    MYTRACE_COND(DEBUG_MEDIAPLAYBACK, ACE_TEXT("MediaPlayback - %p. ID: %d\n"), this, userdata);
}

MediaPlayback::~MediaPlayback()
{
    bool wait = false;
    {
        std::lock_guard<std::mutex> g(m_mutex);
        wait = (m_status == MEDIASTREAM_FINISHED);
    }

    if (wait)
    {
        // block to ensure all audio has been played
        MYTRACE_COND(DEBUG_MEDIAPLAYBACK, ACE_TEXT("~MediaPlayback - %p. Waiting. ID: %d\n"), this, m_userdata);
        m_drained.get(wait);
    }

    m_streamer.reset();

    m_sndsys->CloseOutputStream(this);

    std::lock_guard<std::mutex> g(m_mutex);
    while (m_audio_buffer.size())
    {
        m_audio_buffer.front()->release();
        m_audio_buffer.pop();
    }
    MYTRACE_COND(DEBUG_MEDIAPLAYBACK, ACE_TEXT("~MediaPlayback - %p. ID: %d\n"), this, m_userdata);
}

bool MediaPlayback::OpenFile(const ACE_TString& filename)
{
    if (m_streamer && m_streamer->GetMediaFile().IsValid())
        return false;

    MediaStreamOutput outprop(PB_FRAMEDURATION_MSEC, media::FOURCC_NONE);

    m_streamer = MakeMediaFileStreamer(filename, outprop);
    if (m_streamer && m_streamer->Open())
    {
        m_streamer->RegisterVideoCallback(std::bind(&MediaPlayback::MediaStreamVideoCallback,
                                                    this, _1, _2), true);
        m_streamer->RegisterAudioCallback(std::bind(&MediaPlayback::MediaStreamAudioCallback,
                                                    this, _1, _2), true);
        m_streamer->RegisterStatusCallback(std::bind(&MediaPlayback::MediaStreamStatusCallback,
                                                     this, _1, _2), true);
        return true;
    }

    m_streamer.reset();
    return false;
}

bool MediaPlayback::OpenSoundSystem(int sndgrpid, int outputdeviceid, bool speexdsp /*= false*/)
{
    if (!m_streamer)
        return false;

    auto inprop = m_streamer->GetMediaFile();
    if (!inprop.HasAudio())
        return false;

    int inframesize = int(PCM16_DURATION_SAMPLES(PB_FRAMEDURATION_MSEC, inprop.audio.samplerate));
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

#if defined(ENABLE_WEBRTC)
bool MediaPlayback::SetupWebRTCPreprocess(const webrtc::AudioProcessing::Config& webrtc)
{
    if (!m_apm)
        m_apm.reset(webrtc::AudioProcessingBuilder().Create());

    m_apm->ApplyConfig(webrtc);
    if (m_apm->Initialize() != webrtc::AudioProcessing::kNoError)
    {
        m_apm.reset();
        return false;
    }
    return true;
}
#endif

bool MediaPlayback::MediaStreamVideoCallback(media::VideoFrame& video_frame,
                                             ACE_Message_Block* mb_video)
{
    return false;
}

bool MediaPlayback::MediaStreamAudioCallback(media::AudioFrame& audio_frame,
                                             ACE_Message_Block* mb_audio)
{
    std::lock_guard<std::mutex> g(m_mutex);
    if (m_audio_buffer.size() > 10)
    {
        MYTRACE_COND(DEBUG_MEDIAPLAYBACK, ACE_TEXT("Media Playback buffer full. Discarding audio frame.\n"));
        return false;
    }

    m_audio_buffer.push(mb_audio);
    return true;
}

void MediaPlayback::MediaStreamStatusCallback(const MediaFileProp& mfp,
                                              MediaStreamStatus status)
{
    switch (status)
    {
    case MEDIASTREAM_STARTED :
        if (m_sndsys->IsStreamStopped(this))
        {
            if (!m_sndsys->StartStream(this))
            {
                std::lock_guard<std::mutex> g(m_mutex);
                m_status = MEDIASTREAM_ERROR;
                m_drained.set(true); // ensure we don't wait for 'finished'
            }
        }
    case MEDIASTREAM_NONE :
    case MEDIASTREAM_PLAYING :
    case MEDIASTREAM_ERROR :
    case MEDIASTREAM_FINISHED:
    case MEDIASTREAM_PAUSED :
        break;
    }

    std::lock_guard<std::mutex> g(m_mutex);
    m_progress.push(MediaFileProgress(status, mfp));
}

MediaStreamStatus MediaPlayback::GetStatus() const
{
    return m_status;
}

bool MediaPlayback::Flushed()
{
    // Give audio player time to submit and play the audio.
    // Stopping at MEDIASTREAM_FINISHED may not have played everything.
    return GetStatus() == MEDIASTREAM_FINISHED;
}

bool MediaPlayback::StreamPlayerCb(const soundsystem::OutputStreamer& streamer,
                                   short* buffer, int samples)
{
    SubmitPreProgress();

    ACE_Message_Block* mb = nullptr;
    {
        std::lock_guard<std::mutex> g(m_mutex);

        if (m_audio_buffer.size())
        {
            mb = m_audio_buffer.front();
            m_audio_buffer.pop();
        }
        else if (m_status == MEDIASTREAM_FINISHED)
        {
            /* Perform an extra callback to ensure 'm_sndsys->CloseOutputStream(this)'
             * is not called immediately after submitting the last audio frame. This
             * should already be the case since 'm_status' is now MEDIASTREAM_FINISHED
             * but on Android the last submitted frame is somehow lost. */
            if (m_last_callback)
                m_drained.set(true);
            m_last_callback = true;
        }
    }

    MYTRACE_COND(DEBUG_MEDIAPLAYBACK && !mb, ACE_TEXT("Media playback underflow\n"));

    if (mb)
    {
        MBGuard gmb(mb);
        media::AudioFrame frm(mb);

        if (m_resampler)
        {
            int n_resampled = m_resampler->Resample(frm.input_buffer, buffer);
            MYTRACE_COND(n_resampled != samples,
                         ACE_TEXT("Media playback. Unexpected number of samples returned from resampler. %d != %d\n"),
                         n_resampled, samples);
            assert(n_resampled <= samples);
        }
        else
        {
            assert(streamer.framesize == samples);
            std::memcpy(buffer, frm.input_buffer, PCM16_BYTES(streamer.channels, streamer.framesize));
        }

#if defined(ENABLE_WEBRTC)
        media::AudioFrame apm_frm(media::AudioFormat(streamer.samplerate, streamer.channels),
                                  buffer, streamer.framesize);
        if (m_apm && WebRTCPreprocess(*m_apm, apm_frm, apm_frm) != streamer.framesize)
        {
            MYTRACE(ACE_TEXT("WebRTC in media file playback failed to process audio\n"));
        }
#endif

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

        if (m_audiofunc)
        {
            assert(samples == streamer.framesize);
            media::AudioFrame cbfrm(media::AudioFormat(streamer.samplerate, streamer.channels),
                                    buffer, streamer.framesize, m_sampleindex);
            cbfrm.streamid = m_userdata;
            m_audiofunc(m_userdata, cbfrm);
        }

        m_sampleindex += samples;
    }
    else
    {
        std::memset(buffer, 0, PCM16_BYTES(streamer.channels, streamer.framesize));
    }

    // Call m_statusfunc() with MEDIASTREAM_ERROR, MEDIASTREAM_PAUSED, MEDIASTREAM_FINISHED
    SubmitPostProgress();

    return true;
}

void MediaPlayback::SubmitPreProgress()
{
    // Call m_statusfunc() with MEDIASTREAM_STARTED, MEDIASTREAM_PLAYING
    MediaFileProgress progress;
    do
    {
        progress = MediaFileProgress();
        {
            std::lock_guard<std::mutex> g(m_mutex);
            if (m_progress.size())
            {
                switch (m_progress.front().status)
                {
                case MEDIASTREAM_NONE :
                    assert(m_progress.front().status != MEDIASTREAM_NONE);
                    break;
                case MEDIASTREAM_PLAYING :
                case MEDIASTREAM_STARTED :
                    progress = m_progress.front();
                    m_progress.pop();
                    break;
                case MEDIASTREAM_FINISHED :
                case MEDIASTREAM_ERROR :
                case MEDIASTREAM_PAUSED :
                    return;
                }
            }
        }

        switch (progress.status)
        {
        case MEDIASTREAM_NONE :
            break;
        case MEDIASTREAM_PLAYING :
        case MEDIASTREAM_STARTED :
            MYTRACE_COND(DEBUG_MEDIAPLAYBACK, ACE_TEXT("MediaPlayback - %p. ID: %d. %s. Status: %d\n"),
                         this, m_userdata, progress.mfp.filename.c_str(), progress.status);
            if (m_statusfunc)
                m_statusfunc(m_userdata, progress.mfp, progress.status);
            m_status = progress.status;
            break;
        case MEDIASTREAM_FINISHED :
        case MEDIASTREAM_ERROR :
        case MEDIASTREAM_PAUSED :
            assert(0);
            break;
        }

    }
    while (progress.status != MEDIASTREAM_NONE);
}

void MediaPlayback::SubmitPostProgress()
{
    // m_statusfunc() must not be called with MEDIASTREAM_ERROR,
    // MEDIASTREAM_PAUSED or MEDIASTREAM_FINISHED
    // until m_audiofunc() has completed pumping out audio frames.
    MediaFileProgress progress;
    do
    {
        progress = MediaFileProgress();
        {
            std::lock_guard<std::mutex> g(m_mutex);
            if (m_progress.size())
            {
                switch (m_progress.front().status)
                {
                case MEDIASTREAM_NONE :
                    assert(m_progress.front().status != MEDIASTREAM_NONE);
                    break;
                case MEDIASTREAM_PLAYING :
                case MEDIASTREAM_STARTED :
                    return;
                case MEDIASTREAM_FINISHED :
                case MEDIASTREAM_ERROR :
                    assert(m_progress.size() == 1);
                case MEDIASTREAM_PAUSED :
                    if (m_audio_buffer.empty())
                    {
                        progress = m_progress.front();
                        m_progress.pop();
                    }
                    break;
                }
            }
        }

        switch (progress.status)
        {
        case MEDIASTREAM_NONE :
            break;
        case MEDIASTREAM_PLAYING :
        case MEDIASTREAM_STARTED :
            assert(0);
            break;
        case MEDIASTREAM_FINISHED :
        case MEDIASTREAM_ERROR :
        case MEDIASTREAM_PAUSED :
            m_status = progress.status;
            MYTRACE_COND(DEBUG_MEDIAPLAYBACK, ACE_TEXT("MediaPlayback - %p. ID: %d. %s. Status: %d\n"),
                         this, m_userdata, progress.mfp.filename.c_str(), progress.status);
            if (m_statusfunc)
                m_statusfunc(m_userdata, progress.mfp, progress.status);
            break;
        }
    }
    while (progress.status != MEDIASTREAM_NONE);
}
