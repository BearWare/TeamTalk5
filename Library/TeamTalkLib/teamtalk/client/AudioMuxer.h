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

#include <myace/MyACE.h>
#include <myace/TimerHandler.h>
#include <codec/WaveFile.h>
#if defined(ENABLE_MEDIAFOUNDATION)
#include <avstream/MFTransform.h>
#endif
#include <teamtalk/CodecCommon.h>
#include <mystd/MyStd.h>

#include <map>
#include <mutex>
#include <thread>


#if defined(ENABLE_OPUSTOOLS) && defined(ENABLE_OPUS) && defined(ENABLE_OGG)
#include <codec/OggOutput.h>
#define ENABLE_OPUSFILE 1
#endif

#if defined(ENABLE_SPEEX) && defined(ENABLE_OGG)
#include <codec/OggOutput.h>
#define ENABLE_SPEEXFILE 1
#endif

typedef std::function< void (const media::AudioFrame& frm) > audiomuxer_callback_t;

class AudioMuxer : private TimerListener
{
public:
    AudioMuxer();
    virtual ~AudioMuxer();

    bool RegisterMuxCallback(const teamtalk::AudioCodec& codec,
                             audiomuxer_callback_t cb);
    void UnregisterMuxCallback();

    bool SaveFile(const teamtalk::AudioCodec& codec,
                  const ACE_TString& filename,
                  teamtalk::AudioFileFormat aff);
    void CloseFile();

    void QueueUserAudio(int userid, const short* rawAudio,
                        ACE_UINT32 sample_no, bool last,
                        const teamtalk::AudioCodec& codec);
    void QueueUserAudio(int userid, const short* rawAudio,
                        ACE_UINT32 sample_no, bool last,
                        int n_samples, int n_channels);

private:
    bool Init(const teamtalk::AudioCodec& codec);
    bool StartThread(const teamtalk::AudioCodec& codec);
    void StopThread();
    void Run();
    int TimerEvent(ACE_UINT32 timer_event_id, long userdata);

    void ProcessAudioQueues(bool flush);
    bool CanMuxUserAudio();
    void RemoveEmptyMuxUsers(); // should only be used during flush
    bool MuxUserAudio();
    void WriteAudio(int cb_samples);
    bool FileActive();

    typedef std::shared_ptr< ACE_Message_Queue<ACE_MT_SYNCH> > message_queue_t;

    typedef std::map<int, message_queue_t> user_audio_queue_t;
    user_audio_queue_t m_audio_queue;
    typedef std::map<int, ACE_UINT32> user_queued_audio_t;
    user_queued_audio_t m_user_queue;
    std::vector<short> m_muxed_audio;

    ACE_Reactor m_reactor;
    std::recursive_mutex m_mutex;
    std::shared_ptr< std::thread > m_thread;

    ACE_UINT32 m_sample_no = 0;
    ACE_UINT32 m_last_flush_time = 0;
    teamtalk::AudioCodec m_codec;

    wavepcmfile_t m_wavefile;
#if defined(ENABLE_MEDIAFOUNDATION)
    mftransform_t m_mp3encoder;
#endif

#if defined(ENABLE_SPEEXFILE)
    speexencfile_t m_speexfile;
#endif

#if defined(ENABLE_OPUSFILE)
    opusencfile_t m_opusfile;
#endif

    audiomuxer_callback_t m_muxcallback = {};
};

typedef std::shared_ptr< AudioMuxer > audiomuxer_t;

#define FIXED_AUDIOCODEC_CHANNELID 0

class ChannelAudioMuxer
{
    // userid -> channelid
    std::map<int, int> m_userchan;
    // channel -> muxer
    std::map<int, audiomuxer_t> m_muxers;

    std::recursive_mutex m_mutex;

public:
    ChannelAudioMuxer();
    ~ChannelAudioMuxer();

    bool SaveFile(int channelid, const teamtalk::AudioCodec& codec,
                  const ACE_TString& filename,
                  teamtalk::AudioFileFormat aff);
    bool CloseFile(int channelid);

    bool AddUser(int userid, int channelid);
    bool RemoveUser(int userid);

    void QueueUserAudio(int userid, const short* rawAudio,
                        ACE_UINT32 sample_no, bool last,
                        int n_samples, int n_channels);
};
#endif
