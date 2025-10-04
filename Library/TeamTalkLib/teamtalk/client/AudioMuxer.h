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

#include "AudioContainer.h"

#include "codec/MediaUtil.h"
#include "codec/WaveFile.h"
#include "myace/TimerHandler.h"
#include "mystd/MyStd.h"
#include "teamtalk/Common.h"

#if defined(ENABLE_MEDIAFOUNDATION)
#include "avstream/MFTransform.h"
#endif

#if defined(ENABLE_OGG)
#include "codec/OggFileIO.h"

#if defined(ENABLE_OPUSTOOLS) && defined(ENABLE_OPUS)
#define ENABLE_OPUSFILE
#endif
#if defined(ENABLE_SPEEX)
#define ENABLE_SPEEXFILE
#endif

#endif /* ENABLE_OGG */

#include <ace/Message_Block.h>
#include <ace/Message_Queue_T.h>
#include <ace/Reactor.h>
#include <ace/SString.h>
#include <ace/Time_Value.h>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

using audiomuxer_callback_t = std::function< void (teamtalk::StreamTypes sts, const media::AudioFrame& frm) >;

using audiomuxer_tick_t = std::function< void (teamtalk::StreamTypes sts, uint32_t sample_no) >;

class AudioMuxer : private TimerListener, NonCopyable
{
public:
    AudioMuxer(teamtalk::StreamTypes sts);
    ~AudioMuxer() override;

    bool RegisterMuxCallback(const media::AudioInputFormat& fmt,
                             audiomuxer_callback_t cb);
    void UnregisterMuxCallback();

    // For debugging when ProcessAudioQueues() has been running
    void RegisterMuxTick(audiomuxer_tick_t cb);

    bool SaveFile(const teamtalk::AudioCodec& codec,
                  const ACE_TString& filename,
                  teamtalk::AudioFileFormat aff);
    void CloseFile();

    bool QueueUserAudio(int userid, teamtalk::StreamType st, const media::AudioFrame& frm);

    void SetMuxInterval(int msec);
    
private:
    bool Init(const media::AudioInputFormat& fmt);
    bool StartThread(const media::AudioInputFormat& fmt);
    void StopThread();
    void Run();
    int TimerEvent(ACE_UINT32 timer_event_id, long userdata) override;

    void ProcessAudioQueues(bool flush);
    bool CanMuxUserAudio();
    void RemoveEmptyMuxUsers(); // should only be used during flush
    teamtalk::StreamTypes MuxUserAudio();
    void WriteAudio(int cb_samples, teamtalk::StreamTypes sts);
    bool FileActive();

    using message_queue_t = std::shared_ptr< ACE_Message_Queue<ACE_MT_SYNCH> >;
    message_queue_t GetMuxQueue(int key);
    // return number of media::AudioFrame in queue
    int SubmitMuxAudioFrame(int key, const media::AudioFrame& frm);
    // drain everything in 'm_preprocess_queue'
    void SubmitPreprocessQueue();
    ACE_Message_Block* BuildMuxAudioFrame(std::vector<ACE_Message_Block*>& mbs) const;

    // preprocess queue (audio that cannot yet to into 'm_usermux_queue')
    AudioContainer m_preprocess_queue;
    std::recursive_mutex m_mutex1_preprocess;
    // key -> media::AudioFrame. Audio block storing the remainder of 'm_preprocess_queue'
    std::map<int, ACE_Message_Block*> m_preprocess_block;

    // raw audio data from a user ID
    using user_audio_queue_t = std::map<int, message_queue_t>;
    user_audio_queue_t m_usermux_queue;
    // next sample number to expect for muxing from a user ID
    using user_muxprogress_t = std::map<int, uint32_t>;
    user_muxprogress_t m_usermux_progress;
    std::vector<short> m_muxed_buffer;

    ACE_Reactor m_reactor;
    ACE_Time_Value m_mux_interval;
    std::recursive_mutex m_mutex2_mux;
    std::shared_ptr< std::thread > m_thread;

    uint32_t m_sample_no = 0;
    uint32_t m_last_flush_time = 0;
    media::AudioInputFormat m_inputformat;
    teamtalk::StreamTypes m_streamtypes = teamtalk::STREAMTYPE_NONE;

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

    audiomuxer_callback_t m_muxcallback;
    audiomuxer_tick_t m_tickcallback;
};

using audiomuxer_t = std::shared_ptr< AudioMuxer >;

constexpr auto FIXED_AUDIOCODEC_CHANNELID = 0;

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
                  teamtalk::StreamTypes sts,
                  const ACE_TString& filename,
                  teamtalk::AudioFileFormat aff);
    bool CloseFile(int channelid);

    bool AddUser(int userid, teamtalk::StreamType st, int channelid);
    bool RemoveUser(int userid, teamtalk::StreamType st);

    void QueueUserAudio(int userid, teamtalk::StreamType st, const media::AudioFrame& frm);
};
#endif
