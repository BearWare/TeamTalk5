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

#include "AudioContainer.h"
#include "avstream/AudioResampler.h"
#include "mystd/MyStd.h"

#include <cassert>
#include <cstring>
#include <memory>

static uint32_t GenKey(int userid, teamtalk::StreamTypes sts)
{
    assert(userid < 0x10000);
    assert(sts < 0x10000);
    return (userid << 16) | sts;
}

void AudioContainer::Reset()
{
    std::scoped_lock const g(m_store_mtx);
    m_container.clear();
}

void AudioContainer::AddAudioSource(int userid, teamtalk::StreamTypes sts,
                                    const media::AudioFormat& af)
{
    std::scoped_lock const g(m_store_mtx);
    auto key = GenKey(userid, sts);
    audioentry_t const entry = std::make_shared<AudioEntry>(af);
    m_container[key] = entry;
}

void AudioContainer::RemoveAudioSource(int userid, teamtalk::StreamTypes sts)
{
    std::scoped_lock const g(m_store_mtx);
    m_container.erase(GenKey(userid, sts));
}

bool AudioContainer::AddAudio(int userid, teamtalk::StreamTypes sts,
                              const media::AudioFrame& frame)
{
    std::scoped_lock const g(m_store_mtx);

    auto const ii = m_container.find(GenKey(userid, sts));

    if (ii == m_container.end())
        return false;

    // MYTRACE(TTEXT("Add audio from #%d to container %p. Offset %u. Samples: %u. Timestamp: %u\n"),
    //         userid, this, frame.sample_no, frame.input_samples, frame.timestamp);

    // MYTRACE(TTEXT("Adding audio #%d of channels %d\n"), userid, channels);
    auto mb = AudioFrameToMsgBlock(frame);
    if (!ii->second->mq.Put(std::move(mb)))
    {
        MYTRACE(TTEXT("AudioContainer overflow for #%d, failed to insert block\n"), userid);
        return false;
    }
    return true;
}

bool AudioContainer::Exists(int userid, teamtalk::StreamTypes sts)
{
    std::scoped_lock const g(m_store_mtx);

    return m_container.contains(GenKey(userid, sts));
}

bool AudioContainer::IsEmpty(int userid, teamtalk::StreamTypes sts)
{
    std::scoped_lock const g(m_store_mtx);
    auto ii = m_container.find(GenKey(userid, sts));
    if (ii == m_container.end())
        return true;

    return ii->second->mq.Size() == 0;
}

std::unique_ptr<MsgBlock> AudioContainer::AcquireAudioFrame(int userid, teamtalk::StreamTypes sts)
{
    std::scoped_lock const g(m_store_mtx);

    auto const ii = m_container.find(GenKey(userid, sts));
    if (ii == m_container.end())
        return nullptr;

    auto& entry = ii->second;

    std::unique_ptr<MsgBlock> mb;
    if (entry->mq.Get(mb, std::chrono::milliseconds(0)))
    {
        media::AudioFrame frm(mb.get());

        // Special handling required by 'AudioMuxer'. 
        if (!frm.inputfmt.IsValid() && frm.input_samples == 0)
            return mb;

        frm.ApplyGain();

        if (entry->outfmt.IsValid() && frm.inputfmt != entry->outfmt)
        {
            // create resampler if it doesn't already exist
            if (!entry->resampler || entry->resampler->GetOutputFormat() != entry->outfmt)
            {
                entry->resampler = MakeAudioResampler(frm.inputfmt, entry->outfmt);
            }

            // setup MsgBlock for resampled output
            int const samples = CalcSamples(frm.inputfmt.samplerate, frm.input_samples,
                                      entry->outfmt.samplerate);

            size_t const audiobytes = PCM16_BYTES(samples, entry->outfmt.channels);
            size_t const totalbytes = audiobytes + sizeof(media::AudioFrame);

            auto mb_resam = std::make_unique<MsgBlock>(totalbytes);

            auto* outputptr = reinterpret_cast<short*>(mb_resam->Data() + sizeof(media::AudioFrame));
            media::AudioFrame frm_resam = frm;
            frm_resam.inputfmt = entry->outfmt;
            frm_resam.input_buffer = outputptr;
            frm_resam.input_samples = samples;
            frm_resam.sample_no = CalcSamples(frm.inputfmt.samplerate, frm.sample_no,
                                              entry->outfmt.samplerate);

            mb_resam->Copy(&frm_resam, sizeof(frm_resam));
            assert(entry->resampler);
            int const outputsamples = entry->resampler->Resample(frm.input_buffer, frm.input_samples, outputptr, samples);
            assert(outputsamples <= samples);
            MYTRACE_COND(outputsamples != samples, TTEXT("Resampled audio output doesn't match expected: %d != %d. Zeroing remaining\n"),
                         outputsamples, samples);
            if (outputsamples < samples)
                std::memset(&outputptr[outputsamples * frm_resam.inputfmt.channels], 0, PCM16_BYTES(samples - outputsamples, frm_resam.inputfmt.channels));
            mb_resam->Length(sizeof(frm_resam) + audiobytes); //include resampled output

            return mb_resam; // 'mb' auto-released
        }

        // MYTRACE("Retrieved audio for #%d type %d, index %u\n", userid, stream_type, frm.sample_no);
        return mb;
    }

    return nullptr;
}

void AudioContainer::ReleaseAllAudio()
{
    std::scoped_lock const g(m_store_mtx);

    auto ii = m_container.begin();
    while(ii != m_container.end())
    {
        MYTRACE_COND(ii->second->mq.Size() > 0,
                     TTEXT("AudioContainer clean up #%d, removing obsolete\n"),
                     ii->first >> 16);
        ii->second->mq.Flush();

        m_container.erase(ii++);
    }
}
