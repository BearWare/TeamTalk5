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

#include "AudioInputStreamer.h"
#include <assert.h>

AudioInputStreamer::AudioInputStreamer(int streamid)
: m_streamid(streamid)
{
}

AudioInputStreamer::~AudioInputStreamer()
{
    m_resample_frames.close();

    Close();
}

void AudioInputStreamer::RegisterAudioInputStatusCallback(audioinput_statuscallback_t cb, bool enable)
{
    if (enable)
        m_statuscb = cb;
    else
        m_statuscb = {};
}

bool AudioInputStreamer::InsertAudio(const media::AudioFrame& frame)
{
    if (!m_inputfmt.IsValid())
    {
        if (!frame.inputfmt.IsValid())
            return false;

        m_inputfmt = frame.inputfmt;
        m_resample_frames.low_water_mark(PCM16_BYTES(m_inputfmt.samplerate, m_inputfmt.channels) * BUF_SECS);
        m_resample_frames.high_water_mark(PCM16_BYTES(m_inputfmt.samplerate, m_inputfmt.channels) * BUF_SECS);
    }
    else if (frame.inputfmt != m_inputfmt) // don't allow input format to change
        return false;

    if (frame.inputfmt != GetMediaOutput().audio)
    {
        if (!m_resampler)
        {
            m_resampler = MakeAudioResampler(frame.inputfmt, GetMediaOutput().audio);
            assert(m_resampler);
            if (!m_resampler)
            {
                return false;
            }
        }
    }

    ACE_Message_Block* mb = AudioFrameToMsgBlock(frame);
    ACE_Time_Value zero;
    if (m_resample_frames.enqueue(mb, &zero) < 0)
    {
        mb->release();
        return false;
    }
    return true;
}

bool AudioInputStreamer::Flush()
{
    ACE_Message_Block* mb = AudioFrameToMsgBlock(media::AudioFrame());
    ACE_Time_Value zero;
    if (m_resample_frames.enqueue(mb, &zero) < 0)
    {
        mb->release();
        return false;
    }
    return true;
}

void AudioInputStreamer::AudioProgress(uint32_t queuedmsec, uint32_t elapsedmsec)
{
    if (m_statuscb)
        m_statuscb(AudioInputStatus(queuedmsec, elapsedmsec, m_streamid));
}

void AudioInputStreamer::Run()
{
    bool ready = GetMediaOutput().audio.IsValid();
    m_open.set(ready);

    if (!ready)
        return;

    bool start = true;
    m_run.get(start);

    if (!start)
        return;

    InitBuffers();

    ACE_UINT32 starttime = GETTIMESTAMP();

    bool flush = false;
    while (!m_stop)
    {
        if (!ProcessAVQueues(starttime, GETTIMESTAMP(), flush))
        {
            if (flush)
                break;

            // wait for more data
            flush = ProcessResample();
        }
    }

    if (!m_stop && flush && m_statuscb)
        m_statuscb(AudioInputStatus(0, 0, m_streamid));
}

bool AudioInputStreamer::ProcessResample()
{
    ACE_Message_Block* mb = nullptr;
    int ret = m_resample_frames.dequeue(mb, nullptr);
    if (!mb)
        return false;

    media::AudioFrame frame(mb);

    if (frame.input_samples == 0)
    {
        mb->release();
        return true;
    }

    if (frame.inputfmt != GetMediaOutput().audio)
    {
        MBGuard g(mb);
        assert(frame.inputfmt == m_inputfmt);
        assert(m_resampler);
        int osamples = CalcSamples(frame.inputfmt.samplerate, frame.input_samples, GetMediaOutput().audio.samplerate);
        if (m_resamplebuffer.size() != osamples * GetMediaOutput().audio.channels)
            m_resamplebuffer.resize(osamples * GetMediaOutput().audio.channels);
        ret = m_resampler->Resample(frame.input_buffer, frame.input_samples, &m_resamplebuffer[0], osamples);
        assert(ret > 0);
        media::AudioFrame resam_frame(GetMediaOutput().audio, &m_resamplebuffer[0], osamples);
        Submit(resam_frame);
    }
    else
    {
        if (!Submit(mb))
            mb->release();
    }

    return false; // no flush
}

bool AudioInputStreamer::Submit(const media::AudioFrame& frame)
{
    if (!GetMediaOutput().audio.IsValid())
        return false;

    media::AudioFrame copy = frame;
    UpdateTimeStamp(copy);
    return QueueAudio(copy);
}

bool AudioInputStreamer::Submit(ACE_Message_Block* mb)
{
    if(!GetMediaOutput().audio.IsValid())
        return false;

    media::AudioFrame* frame = AudioFrameFromMsgBlock(mb);
    UpdateTimeStamp(*frame);
    return QueueAudio(*frame);
}

void AudioInputStreamer::UpdateTimeStamp(media::AudioFrame& frame)
{
    assert(frame.inputfmt == GetMediaOutput().audio);
    frame.timestamp = ACE_UINT32( (m_sampleindex * 1000) / GetMediaOutput().audio.samplerate );
    m_sampleindex += frame.input_samples;
}
