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
#include <teamtalk/ttassert.h>

AudioContainer::AudioContainer()
{
}

void AudioContainer::AddSoundSource(int userid, int stream_type)
{
    std::lock_guard<std::recursive_mutex> g(m_store_mtx);
    m_active_srcs.insert(audioentry(userid, stream_type).entryid);
}

void AudioContainer::RemoveSoundSource(int userid, int stream_type)
{
    std::lock_guard<std::recursive_mutex> g(m_store_mtx);

    audioentry entry(userid, stream_type);
    m_active_srcs.erase(entry.entryid);
    m_container.erase(entry.entryid);
}

bool AudioContainer::AddAudio(int userid, int stream_type,
                              const media::AudioFrame& frame)
{
    TTASSERT(userid<=0xFFFF);
    TTASSERT(stream_type<=0xFFFF);

    std::lock_guard<std::recursive_mutex> g(m_store_mtx);

    audioentry_t entry(userid, stream_type);
    if(m_active_srcs.find(entry.entryid) == m_active_srcs.end())
        return false;

    // MYTRACE(ACE_TEXT("Adding audio #%d of channels %d\n"), userid, channels);
    ACE_Message_Block* mb = AudioFrameToMsgBlock(frame);

    audiostore_t::iterator ii = m_container.find(entry.entryid);
    if(ii != m_container.end())
    {
        ACE_Time_Value tm;
        if (ii->second->enqueue(mb, &tm) < 0)
        {
            MYTRACE(ACE_TEXT("AudioContainer overflow for #%d, failed to insert block\n"), userid);
            mb->release();

            return false;
        }
    }
    else
    {
        ACE_Time_Value tm;

        ACE_Message_Queue<ACE_MT_SYNCH>* q;
        ACE_NEW_NORETURN(q, ACE_Message_Queue<ACE_MT_SYNCH>());
        if(!q)
        {
            mb->release();
            return false;
        }
        
        q->high_water_mark(1024*128);
        q->low_water_mark(1024*128);

        msg_queue_t mq(q);
        if (mq->enqueue(mb, &tm) < 0)
        {
            MYTRACE(ACE_TEXT("AudioContainer initial overflow for #%d, removing block\n"), userid);
            mb->release();
            return false;
        }
        else
        {
            m_container[entry.entryid] = mq;
        }
    }
    return true;
}

ACE_Message_Block* AudioContainer::AcquireAudioFrame(int userid, int stream_type)
{
    std::lock_guard<std::recursive_mutex> g(m_store_mtx);

    audioentry_t entry(userid, stream_type);

    audiostore_t::iterator ii = m_container.find(entry.entryid);
    if(ii != m_container.end())
    {
        ACE_Time_Value tm;
        ACE_Message_Block* mb;

        if (ii->second->dequeue_head(mb, &tm) >= 0)
        {
            media::AudioFrame frm(mb);
            // MYTRACE("Retrieved audio for #%d type %d, index %u\n", userid, stream_type, frm.sample_no);
            return mb;
        }
    }
    // MYTRACE("Didn't find audio for #%d type %d\n", userid, stream_type);
    return nullptr;
}

void AudioContainer::ReleaseAllAudio()
{
    std::lock_guard<std::recursive_mutex> g(m_store_mtx);
    
    audiostore_t::iterator ii = m_container.begin();
    while(ii != m_container.end())
    {
        audioentry_t entry;
        entry.entryid = ii->first;

        ACE_Time_Value tm;
        ACE_Message_Block* mb;

        while(ii->second->dequeue(mb, &tm) >= 0)
        {
            mb->release();
            MYTRACE(ACE_TEXT("AudioContainer clean up #%d, removing obsolete\n"),
                    entry.userid);
        }

        m_container.erase(ii++);
    }
}
