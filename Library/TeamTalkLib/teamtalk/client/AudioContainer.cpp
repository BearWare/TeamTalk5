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
#include <teamtalk/Common.h>

using namespace teamtalk;

RawAudio::RawAudio()
: samplerate(0)
, channels(0)
, rawAudio(0)
, samples(0)
, start_sample_no(0)
{
    create_time = GETTIMESTAMP();
}

AudioContainer::AudioContainer()
{
}

void AudioContainer::AddSoundSource(int sndgrp_id, int userid, int stream_type)
{
    wguard_t g(m_store_mtx);
    m_active_srcs.insert(audioentry(sndgrp_id, userid, stream_type).entryid);
}

void AudioContainer::RemoveSoundSource(int sndgrp_id, int userid, int stream_type)
{
    wguard_t g(m_store_mtx);

    ReleaseAllAudio(sndgrp_id);
    m_active_srcs.erase(audioentry(sndgrp_id, userid, stream_type).entryid);
}

bool AudioContainer::AddAudio(int sndgrpid, int userid, int stream_type, 
                              int stream_id, int samplerate, int channels,
                              const short* rawaudio, int samples, 
                              ACE_UINT32 sample_index)
{
    TTASSERT(sndgrpid<=0xFFFF);
    TTASSERT(userid<=0xFFFF);
    TTASSERT(stream_type<=0xFFFF);

    wguard_t g(m_store_mtx);

    audioentry_t entry(sndgrpid, userid, stream_type);
    if(m_active_srcs.find(entry.entryid) == m_active_srcs.end())
        return false;

    // MYTRACE(ACE_TEXT("Adding audio #%d of channels %d\n"), userid, channels);
    int ret;

    ACE_Message_Block* mb;

    RawAudio aud;
    aud.stream_id = stream_id;
    aud.samplerate = samplerate;
    aud.channels = channels;
    aud.samples = samples;
    aud.start_sample_no = sample_index;
    
    int datasize = sizeof(short) * channels * samples;

    ACE_NEW_RETURN(mb, 
                   ACE_Message_Block(datasize + sizeof(RawAudio)),
                   false);
    
    if(datasize>0)
        aud.rawAudio = reinterpret_cast<short*>(&mb->rd_ptr()[sizeof(aud)]);
    else
        aud.rawAudio = NULL;

    ret = mb->copy(reinterpret_cast<const char*> (&aud), sizeof(aud));
    TTASSERT(ret >= 0);
    if(datasize>0)
    {
        ret = mb->copy(reinterpret_cast<const char*> (rawaudio), datasize);
        TTASSERT(ret >= 0);
    }

    audiostore_t::iterator ii = m_container.find(entry.entryid);
    if(ii != m_container.end())
    {
        ACE_Time_Value tm;
        if(ii->second->enqueue(mb, &tm)<0)
        {
            MYTRACE(ACE_TEXT("AudioContainer overflow for #%d, failed to insert block\n"), userid);
            // ACE_Message_Block* mb_tmp;
            // if(ii->second->dequeue(mb_tmp, &tm) >= 0)
            // {
            //     MYTRACE(ACE_TEXT("AudioContainer overflow for #%d, removing block\n"), userid);
            //     mb_tmp->release();
            // }
            // if(ii->second->enqueue(mb, &tm)<0)
            // {
            //     MYTRACE(ACE_TEXT("AudioContainer overflow for #%d, failed to insert block\n"), userid);
            //     mb->release();
            // }

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
        if(mq->enqueue(mb, &tm)<0)
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

ACE_Message_Block* AudioContainer::AcquireRawAudio(int sndgrpid, int userid, 
                                                   int stream_type, RawAudio& aud)
{
    wguard_t g(m_store_mtx);

    audioentry_t entry(sndgrpid, userid, stream_type);

    audiostore_t::iterator ii = m_container.find(entry.entryid);
    if(ii != m_container.end())
    {
        ACE_Time_Value tm;
        ACE_Message_Block* mb;

        if(ii->second->dequeue_head(mb, &tm) < 0)
            return NULL;
        memcpy(&aud, mb->rd_ptr(), sizeof(aud));
        assert(aud.stream_id || userid == 0);
        return mb;
    }
    return NULL;
}

void AudioContainer::ReleaseAllAudio(int sndgrpid)
{
    wguard_t g(m_store_mtx);
    
    audiostore_t::iterator ii = m_container.begin();
    while(ii != m_container.end())
    {
        audioentry_t entry;
        entry.entryid = ii->first;

        if(entry.sndgrpid != sndgrpid)
        {
            ii++;
            continue;
        }

        ACE_Time_Value tm;
        ACE_Message_Block* mb;

        RawAudio aud;
        while(ii->second->dequeue(mb, &tm) >= 0)
        {
            mb->release();
            MYTRACE(ACE_TEXT("AudioContainer clean up #%d, removing obsolete\n"),
                    entry.userid);
        }

        m_container.erase(ii++);
    }
}
