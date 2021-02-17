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

#include "OpusFileStreamer.h"
#include <assert.h>

OpusFileStreamer::OpusFileStreamer(const ACE_TString& filename, const MediaStreamOutput& out_prop)
    : MediaFileStreamer(filename, out_prop)
{
}

void OpusFileStreamer::Run()
{
    if (!m_decoder.Open(m_media_in.filename))
    {
        m_open.set(false);
        return;
    }

    media::AudioFormat infmt = media::AudioFormat(m_decoder.GetSampleRate(), m_decoder.GetChannels());
    const int FRAMESIZE = m_decoder.GetFrameSize();
    std::vector<short> framebuf(FRAMESIZE * m_decoder.GetChannels());

    if (infmt != m_media_out.audio)
    {
        m_resampler = MakeAudioResampler(infmt, m_media_out.audio, FRAMESIZE);
        if (!m_resampler)
        {
            m_open.set(false);
            return;
        }
    }

    // setup the audio format of input file
    m_media_in.audio = infmt;
    m_media_in.duration_ms = m_decoder.GetDurationMSec();

    // notify ready
    m_open.set(true);

    InitBuffers();

    //wait for start signal
    MYTRACE(ACE_TEXT("OpusFileStreamer waiting to start streaming: %s\n"), m_media_in.filename.c_str());
    bool start = false;
    m_run.get(start);

    if (!start)
        return; // user doesn't want to start stream

    MediaStreamStatus status = MEDIASTREAM_STARTED;
    uint32_t sampleindex = 0;
    uint32_t starttime = GETTIMESTAMP(), totalpausetime = 0;

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
                sampleindex = m_media_in.elapsed_ms / PCM16_SAMPLES_DURATION(FRAMESIZE, infmt.samplerate);
                sampleindex *= FRAMESIZE;
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

        if (m_decoder.Decode(&framebuf[0], FRAMESIZE) != FRAMESIZE)
        {
            break; // eof
        }

        // submit raw decoded audio
        bool submitted;
        if (m_resampler)
        {
            int outsamples = 0;
            short* resampled = m_resampler->Resample(&framebuf[0], &outsamples);
            assert(resampled);
            media::AudioFrame frm(m_media_out.audio, resampled, outsamples,
                                  outsamples * (sampleindex / FRAMESIZE));
            submitted = QueueAudio(frm);
        }
        else
        {
            media::AudioFrame frm(infmt, &framebuf[0], FRAMESIZE, sampleindex);
            submitted = QueueAudio(frm);
        }

        MYTRACE_COND(!submitted, ACE_TEXT("Failed to submit OPUS audio from media streamer. File: %s. Elapsed: %u\n"),
                     m_media_in.filename.c_str(), m_media_in.elapsed_ms);

        // update elapsed
        sampleindex += FRAMESIZE;
        m_media_in.elapsed_ms += PCM16_SAMPLES_DURATION(FRAMESIZE, infmt.samplerate);

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
