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

struct AudioEntry
{
    uint16_t userid = 0;
    uint16_t streamtype = 0;
    
    AudioEntry(uint16_t uid, uint16_t st) : userid(uid), streamtype(st) {}
    
    uint32_t entryid() const
    {
        return (userid << 16) | streamtype;
    }
    bool operator<(const AudioEntry& entry) const
    {
        return entryid() < entry.entryid();
    }
};

class AudioContainer
{
public:
    AudioContainer(const AudioContainer&) = delete;
    AudioContainer();
    
    void AddSoundSource(int userid, int stream_type, const media::AudioFormat& af);
    void RemoveSoundSource(int userid, int stream_type);

    bool AddAudio(int userid, int stream_type, const media::AudioFrame& frame);

    ACE_Message_Block* AcquireAudioFrame(int userid, int stream_type);
    void ReleaseAllAudio();

private:
    typedef std::shared_ptr< ACE_Message_Queue<ACE_MT_SYNCH> > msg_queue_t;
    typedef std::map<AudioEntry, msg_queue_t> audiostore_t;
    audiostore_t m_container;
    std::recursive_mutex m_store_mtx;
};

#endif

