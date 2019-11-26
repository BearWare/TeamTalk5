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

AudioInputStreamer::AudioInputStreamer()
{
}

AudioInputStreamer::~AudioInputStreamer()
{
    m_resample_frames.close();

    Close();
}

bool AudioInputStreamer::InsertAudio(const media::AudioFrame& frame)
{
    ACE_Message_Block* mb = AudioFrameToMsgBlock(frame);
    ACE_Time_Value zero;
    if (m_resample_frames.enqueue(mb, &zero) < 0)
    {
        mb->release();
        return false;
    }
    return true;
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

    while (!m_stop)
    {
        ProcessResample();
        if (!ProcessAVQueues(starttime, GETTIMESTAMP(), false))
        {
            // wait for more data
            ACE_Message_Block* mb = nullptr;
            m_resample_frames.peek_dequeue_head(mb);
        }
    }
}

void AudioInputStreamer::ProcessResample()
{
    while (m_resample_frames.message_count())
    {
        ACE_Message_Block* mb = nullptr;
        int ret = m_resample_frames.dequeue(mb, nullptr);
        assert(ret >= 0);
        if (mb)
        {
            media::AudioFrame frame(mb);
            if (frame.inputfmt != GetMediaOutput().audio)
            {
                MBGuard g(mb);

                if (!m_resampler || frame.inputfmt != m_resampler->GetInputFormat())
                {
                    m_resampler = MakeAudioResampler(frame.inputfmt, GetMediaOutput().audio);
                    assert(m_resampler);
                    if (!m_resampler)
                    {
                        continue;
                    }
                }
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
        }
    }
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
