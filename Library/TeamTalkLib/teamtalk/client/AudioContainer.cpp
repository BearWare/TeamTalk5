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
#include <assert.h>

uint32_t GenKey(int userid, int streamtype)
{
    assert(userid < 0x10000);
    assert(streamtype < 0x10000);
    return (userid << 16) | streamtype;
}

AudioContainer::AudioContainer()
{
}

void AudioContainer::AddSoundSource(int userid, int stream_type,
                                    const media::AudioFormat& af)
{
    std::lock_guard<std::recursive_mutex> g(m_store_mtx);
    auto key = GenKey(userid, stream_type);
    audioentry_t entry(new AudioEntry(af));
    m_container[key] = entry;
}

void AudioContainer::RemoveSoundSource(int userid, int stream_type)
{
    std::lock_guard<std::recursive_mutex> g(m_store_mtx);
    m_container.erase(GenKey(userid, stream_type));
}

bool AudioContainer::AddAudio(int userid, int stream_type,
                              const media::AudioFrame& frame)
{
    std::lock_guard<std::recursive_mutex> g(m_store_mtx);

    audiostore_t::iterator ii = m_container.find(GenKey(userid, stream_type));

    if (ii == m_container.end())
        return false;

    // MYTRACE(ACE_TEXT("Add audio from #%d to container %p. Offset %u. Samples: %u. Timestamp: %u\n"),
    //         userid, this, frame.sample_no, frame.input_samples, frame.timestamp);

    // MYTRACE(ACE_TEXT("Adding audio #%d of channels %d\n"), userid, channels);
    ACE_Message_Block* mb = AudioFrameToMsgBlock(frame);
    ACE_Time_Value tm;
    if (ii->second->mq.enqueue(mb, &tm) < 0)
    {
        MYTRACE(ACE_TEXT("AudioContainer overflow for #%d, failed to insert block\n"), userid);
        mb->release();

        return false;
    }
    return true;
}

ACE_Message_Block* AudioContainer::AcquireAudioFrame(int userid, int stream_type)
{
    std::lock_guard<std::recursive_mutex> g(m_store_mtx);

    audiostore_t::iterator ii = m_container.find(GenKey(userid, stream_type));
    if (ii == m_container.end())
        return nullptr;

    auto& entry = ii->second;

    ACE_Time_Value tm;
    ACE_Message_Block* mb;
    if (entry->mq.dequeue_head(mb, &tm) >= 0)
    {
        media::AudioFrame frm(mb);

        if (entry->outfmt.IsValid() && frm.inputfmt != entry->outfmt)
        {
            // create resampler if it doesn't already exist
            if (!entry->resampler || entry->resampler->GetOutputFormat() != entry->outfmt)
            {
                entry->resampler = MakeAudioResampler(frm.inputfmt, entry->outfmt);
            }

            // setup ACE_Message_Block for resampled output
            int samples = CalcSamples(frm.inputfmt.samplerate, frm.input_samples,
                                      entry->outfmt.samplerate);

            size_t audiobytes = PCM16_BYTES(samples, entry->outfmt.channels);
            size_t totalbytes = audiobytes + sizeof(media::AudioFrame);

            ACE_Message_Block* mb_resam;
            ACE_NEW_NORETURN(mb_resam, ACE_Message_Block(totalbytes));
            if (!mb_resam)
            {
                mb->release();
                return nullptr;
            }

            short* outputptr = reinterpret_cast<short*>(mb_resam->wr_ptr() + sizeof(media::AudioFrame));
            media::AudioFrame frm_resam = frm;
            frm_resam.inputfmt = entry->outfmt;
            frm_resam.input_buffer = outputptr;
            frm_resam.input_samples = samples;
            frm_resam.sample_no = CalcSamples(frm.inputfmt.samplerate, frm.sample_no,
                                              entry->outfmt.samplerate);

            mb_resam->copy(reinterpret_cast<const char*>(&frm_resam), sizeof(frm_resam));
            int outputsamples = entry->resampler->Resample(frm.input_buffer, frm.input_samples, outputptr, samples);
            assert(outputsamples <= samples);
            MYTRACE_COND(outputsamples != samples, ACE_TEXT("Resampled audio output doesn't match expected: %d != %d\n"),
                         outputsamples, samples);
            mb_resam->wr_ptr(audiobytes); //advance past resampled output

            mb->release();
            return mb_resam;
        }

        // MYTRACE("Retrieved audio for #%d type %d, index %u\n", userid, stream_type, frm.sample_no);
        return mb;
    }

    return nullptr;
}

void AudioContainer::ReleaseAllAudio()
{
    std::lock_guard<std::recursive_mutex> g(m_store_mtx);

    audiostore_t::iterator ii = m_container.begin();
    while(ii != m_container.end())
    {
        ACE_Time_Value tm;
        ACE_Message_Block* mb;

        while(ii->second->mq.dequeue(mb, &tm) >= 0)
        {
            mb->release();
            MYTRACE(ACE_TEXT("AudioContainer clean up #%d, removing obsolete\n"),
                    ii->first >> 16);
        }

        m_container.erase(ii++);
    }
}
