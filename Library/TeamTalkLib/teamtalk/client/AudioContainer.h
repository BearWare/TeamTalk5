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

#ifndef AUDIOCONTAINER_H
#define AUDIOCONTAINER_H

#include <codec/MediaUtil.h>

#include <ace/Message_Queue.h>

#include <set>
#include <map>
#include <memory>
#include <mutex>

typedef union audioentry
{
    struct
    {
        ACE_UINT16 userid;
        ACE_UINT16 streamtype;
        ACE_UINT32 none;
    };
    ACE_UINT64 entryid;
    audioentry() : entryid(0) {}
    audioentry(ACE_UINT16 user_id, ACE_UINT16 stream_type)
        : userid(user_id), streamtype(stream_type), none(0) { }
} audioentry_t;

class AudioContainer
{
public:
    AudioContainer(const AudioContainer&) = delete;
    AudioContainer();
    
    void AddSoundSource(int userid, int stream_type);
    void RemoveSoundSource(int userid, int stream_type);

    bool AddAudio(int userid, int stream_type, const media::AudioFrame& frame);

    ACE_Message_Block* AcquireAudioFrame(int userid, int stream_type);
    void ReleaseAllAudio();

private:
    typedef std::shared_ptr< ACE_Message_Queue<ACE_MT_SYNCH> > msg_queue_t;
    typedef std::map<ACE_UINT64, msg_queue_t> audiostore_t;
    audiostore_t m_container;
    std::recursive_mutex m_store_mtx;
    std::set<ACE_UINT64> m_active_srcs;
};

#endif

