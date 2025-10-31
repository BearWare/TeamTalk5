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

#include "VorbisFileStreamer.h"

#include "codec/OggFileIO.h"
#include "myace/MyACE.h"
#include "mystd/MyStd.h"

#include <cassert>
#include <cstdint>
#include <vector>

bool GetVorbisFileMediaFileProp(const ACE_TString& filename, MediaFileProp& mfp)
{
    VorbisDecFile vdf;
    if (vdf.Open(filename))
    {
        mfp.audio = media::AudioFormat(vdf.GetSampleRate(), vdf.GetChannels());
        mfp.duration_ms = vdf.GetDurationMSec();
        mfp.filename = filename;
        return true;
    }
    return false;
}

VorbisFileStreamer::VorbisFileStreamer(const ACE_TString& filename, const MediaStreamOutput& out_prop)
    : MediaFileStreamer(filename, out_prop)
{
}

VorbisFileStreamer::~VorbisFileStreamer()
{
    Close();
}

void VorbisFileStreamer::Run()
{
    if (!m_decoder.Open(m_media_in.filename))
    {
        m_open.set(false);
        return;
    }

    media::AudioFormat const infmt = media::AudioFormat(m_decoder.GetSampleRate(), m_decoder.GetChannels());

    if (infmt.IsValid() && !m_media_out.HasAudio() && (m_media_out.audio_duration_ms != 0u))
    {
        int const audio_samples = PCM16_DURATION_SAMPLES(m_media_out.audio_duration_ms, infmt.samplerate);
        MediaStreamOutput const newoutput(infmt, audio_samples, m_media_out.video);
        m_media_out = newoutput;
    }

    // allocate buffer for decoded samples
    std::vector<short> framebuf(m_decoder.GetSampleRate() * m_decoder.GetChannels());
    std::vector<short> resample_framebuf;

    if (infmt != m_media_out.audio)
    {
        m_resampler = MakeAudioResampler(infmt, m_media_out.audio);
        if (!m_resampler)
        {
            m_open.set(false);
            return;
        }
        resample_framebuf.resize(m_media_out.audio.samplerate * m_media_out.audio.channels);
    }

    // setup the audio format of input file
    m_media_in.audio = infmt;
    m_media_in.duration_ms = m_decoder.GetDurationMSec();

    // notify ready
    m_open.set(true);

    InitBuffers();

    //wait for start signal
    MYTRACE(ACE_TEXT("VorbisFileStreamer waiting to start streaming: %s\n"), m_media_in.filename.c_str());
    bool start = false;
    m_run.get(start);

    if (!start)
        return; // user doesn't want to start stream

    MediaStreamStatus status = MEDIASTREAM_STARTED;
    uint32_t sampleindex = 0;
    uint32_t starttime = GETTIMESTAMP();
    uint32_t totalpausetime = 0;
    uint32_t startoffset = 0;

    while (!m_stop)
    {
        // check if user requested pause
        if (m_pause)
        {
            if (m_statuscallback)
                m_statuscallback(m_media_in, MEDIASTREAM_PAUSED);

            auto pausetime = GETTIMESTAMP();
            if ((m_run.get(start) >= 0 && !start) || m_stop)
            {
                MYTRACE(ACE_TEXT("Media playback aborted during pause\n"));
                break;
            }

            // ensure we don't submit MEDIASTREAM_STARTED twice (also for seek)
            status = MEDIASTREAM_STARTED;

            pausetime = GETTIMESTAMP() - pausetime;
            MYTRACE_COND(pausetime > 0, ACE_TEXT("Paused %s for %u msec\n"), m_media_in.filename.c_str(), pausetime);

            totalpausetime += pausetime;
        }

        // check if we need to seek into stream
        auto newoffset = SetOffset(MEDIASTREAMER_OFFSET_IGNORE);
        if (newoffset != MEDIASTREAMER_OFFSET_IGNORE)
        {
            if (m_decoder.Seek(newoffset))
            {
                ClearBuffers();

                m_media_in.elapsed_ms = m_decoder.GetElapsedMSec();
                startoffset = m_media_in.elapsed_ms;
                starttime = GETTIMESTAMP();
                totalpausetime = 0;
                status = MEDIASTREAM_STARTED;
            }
            else
            {
                if (m_statuscallback && !m_stop)
                    m_statuscallback(m_media_in, MEDIASTREAM_ERROR);
                return;
            }
        }

        // notify MEDIASTREAM_STARTED
        if (status != MEDIASTREAM_NONE)
        {
            if (m_statuscallback)
                m_statuscallback(m_media_in, status);

            status = MEDIASTREAM_NONE;
        }

        int const framesize = m_decoder.Decode(framebuf.data(), m_decoder.GetSampleRate());
        if (framesize <= 0)
        {
            break; // eof
        }

        // submit raw decoded audio
        bool submitted = false;
        if (m_resampler)
        {
            assert(resample_framebuf.size());
            int const outsamples = m_resampler->Resample(framebuf.data(), framesize, resample_framebuf.data(), m_media_out.audio.samplerate);
            assert(outsamples > 0);
            auto resampleindex = CalcSamples(infmt.samplerate, sampleindex, m_media_out.audio.samplerate);
            media::AudioFrame frm(m_media_out.audio, resample_framebuf.data(), outsamples, resampleindex);
            frm.timestamp = m_media_in.elapsed_ms - startoffset;
            submitted = QueueAudio(frm);
        }
        else
        {
            media::AudioFrame frm(infmt, framebuf.data(), framesize, sampleindex);
            frm.timestamp = m_media_in.elapsed_ms - startoffset;
            submitted = QueueAudio(frm);
        }

        MYTRACE_COND(!submitted, ACE_TEXT("Failed to submit VORBIS audio from media streamer. File: %s. Elapsed: %u\n"),
                     m_media_in.filename.c_str(), m_media_in.elapsed_ms);

        // update elapsed
        sampleindex += framesize;
        m_media_in.elapsed_ms += PCM16_SAMPLES_DURATION(framesize, infmt.samplerate);

        // Notify progress
        if (m_statuscallback)
            m_statuscallback(m_media_in, MEDIASTREAM_PLAYING);

        while (!m_stop && ProcessAVQueues(starttime, GETTIMESTAMP() - totalpausetime, false));
    }

    while(!m_stop && ProcessAVQueues(starttime, GETTIMESTAMP() - totalpausetime, true));

    // don't do callback if thread is asked to quit
    if (m_statuscallback && !m_stop)
        m_statuscallback(m_media_in, MEDIASTREAM_FINISHED);
}
