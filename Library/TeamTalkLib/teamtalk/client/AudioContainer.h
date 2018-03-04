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

#include <ace/Singleton.h>
#include <ace/Message_Queue.h>
#include <ace/Bound_Ptr.h>

#include <ace/Recursive_Thread_Mutex.h>

#include <set>
#include <map>

struct RawAudio
{
    int stream_id;
    int samplerate;
    int channels;
    short* rawAudio;
    int samples;
    ACE_UINT32 create_time;
    ACE_UINT32 start_sample_no;
    RawAudio();
};

typedef union audioentry
{
    struct
    {
        ACE_UINT16 sndgrpid;
        ACE_UINT16 userid;
        ACE_UINT16 streamtype;
        ACE_UINT16 none;
    };
    ACE_UINT64 entryid;
    audioentry() : entryid(0) {}
    audioentry(ACE_UINT16 sndgrp_id, ACE_UINT16 user_id, ACE_UINT16 stream_type)
        : userid(user_id), sndgrpid(sndgrp_id), streamtype(stream_type), none(0) { }
} audioentry_t;

class AudioContainer
{
    friend class ACE_Singleton<AudioContainer, ACE_Null_Mutex>;
    AudioContainer();
public:
    void AddSoundSource(int sndgrp_id, int userid, int stream_type);
    void RemoveSoundSource(int sndgrp_id, int userid, int stream_type);

    bool AddAudio(int sndgrpid, int userid, int stream_type, int stream_id, 
                  int samplerate, int channels, const short* rawaudio, int samples, 
                  ACE_UINT32 sample_index);

    ACE_Message_Block* AcquireRawAudio(int sndgrpid, int userid, int stream_type, RawAudio& aud);

    void ReleaseAllAudio(int sndgrpid);

private:
    typedef ACE_Strong_Bound_Ptr< ACE_Message_Queue<ACE_MT_SYNCH>, ACE_Null_Mutex > msg_queue_t;
    typedef std::map<ACE_UINT64, msg_queue_t> audiostore_t;
    audiostore_t m_container;
    ACE_Recursive_Thread_Mutex m_store_mtx;
    std::set<ACE_UINT64> m_active_srcs;
};

typedef ACE_Singleton<AudioContainer, ACE_Null_Mutex> AUDIOCONTAINER;

#endif

