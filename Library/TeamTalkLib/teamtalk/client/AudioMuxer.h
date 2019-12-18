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

#ifndef AUDIOMUXER_H
#define AUDIOMUXER_H

#include <ace/Task.h>
#include <ace/Singleton.h>
#include <ace/Message_Queue.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <codec/WaveFile.h>
#if defined(ENABLE_MEDIAFOUNDATION)
#include <avstream/MFTransform.h>
#endif

#include <map>

#include <teamtalk/CodecCommon.h>

#if defined(ENABLE_OPUSTOOLS) && defined(ENABLE_OPUS) && defined(ENABLE_OGG)
#include <codec/OggOutput.h>
#define ENABLE_OPUSFILE 1
#endif

#if defined(ENABLE_SPEEX) && defined(ENABLE_OGG)
#include <codec/OggOutput.h>
#define ENABLE_SPEEXFILE 1
#endif

#define MUX_MYSELF_USERID 0

class AudioMuxer : private ACE_Task_Base
{
public:
    AudioMuxer();
    virtual ~AudioMuxer();
    
    bool StartThread(const ACE_TString& filename,
                     teamtalk::AudioFileFormat aff,
                     const teamtalk::AudioCodec& codec);
    void StopThread();

    void QueueUserAudio(int userid, const short* rawAudio,
                        ACE_UINT32 sample_no, bool last,
                        const teamtalk::AudioCodec& codec);
    void QueueUserAudio(int userid, const short* rawAudio,
                        ACE_UINT32 sample_no, bool last,
                        int n_samples, int n_channels);

private:
    //ACE Task
    int svc (void);

    int handle_timeout(const ACE_Time_Value &current_time, const void *act=0);

    void ProcessAudioQueues(bool flush);
    bool CanMuxUserAudio();
    void RemoveEmptyMuxUsers(); // should only be used during flush
    bool MuxUserAudio();
    void WriteAudioToFile(int cb_samples);

    bool SetupFileEncode(const ACE_TString& filename, 
                         const teamtalk::AudioCodec& codec);

    typedef std::shared_ptr< ACE_Message_Queue<ACE_MT_SYNCH> > message_queue_t;

    typedef std::map<int, message_queue_t> user_audio_queue_t;
    user_audio_queue_t m_audio_queue;
    typedef std::map<int, ACE_UINT32> user_queued_audio_t;
    user_queued_audio_t m_user_queue;
    std::vector<short> m_muxed_audio;
    ACE_Reactor m_reactor;
    ACE_Recursive_Thread_Mutex m_mutex;
    ACE_UINT32 m_last_flush_time;
    teamtalk::AudioCodec m_codec;

    wavepcmfile_t m_wavefile;
#if defined(ENABLE_MEDIAFOUNDATION)
    mftransform_t m_mp3encoder;
#endif

#if defined(ENABLE_SPEEX) && defined(ENABLE_OGG)
    speexencfile_t m_speexfile;
#endif

#if defined(ENABLE_OPUSTOOLS) && defined(ENABLE_OPUS) && defined(ENABLE_OGG)
    opusencfile_t m_opusfile;
#endif
};

typedef std::shared_ptr< AudioMuxer > audiomuxer_t;

#endif
