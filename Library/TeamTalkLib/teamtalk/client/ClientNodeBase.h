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

#if !defined(CLIENTNODEBASE_H)
#define CLIENTNODEBASE_H

#include "VoiceLogger.h"
#include "avstream/SoundSystem.h"
#include "codec/MediaUtil.h"
#include "myace/TimerHandler.h"
#include "teamtalk/Common.h"
#include "teamtalk/PacketLayout.h"
#include "teamtalk/TTAssert.h"

#include <ace/Lock.h>
#include <ace/Reactor.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Task_T.h>
#include <ace/Thread.h>
#include <ace/Time_Value.h>
#include <ace/Timer_Heap.h>

#include <condition_variable>
#include <cstdint>
#include <mutex>

constexpr auto TIMERID_MASK             = 0x0000FFFF;
constexpr auto USER_TIMER_START         = 0x00008000;
constexpr auto USER_TIMER_USERID_MASK   = 0xFFFF0000; // ((userid << 16) | USER_TIMER_START) + TIMERID
constexpr auto USER_TIMER_USERID_SHIFT  = 16;

constexpr auto USER_TIMERID(int timerid, int userid) { return (((userid) << USER_TIMER_USERID_SHIFT) | (timerid)); }
constexpr auto TIMER_USERID(int timerid) { return (((timerid) >> USER_TIMER_USERID_SHIFT) & 0xFFFF); }

constexpr auto SOUNDDEVICE_IGNORE_ID    = (-1);

namespace teamtalk {

    enum ClientTimer
    {
        TIMER_ONE_SECOND_ID                     = 1, //timer for checking things every second
        TIMER_TCPKEEPALIVE_ID                   = 2,
        TIMER_UDPCONNECT_ID                     = 3, //connect to server with UDP
        TIMER_UDPKEEPALIVE_ID                   = 4,
        TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID      = 8,
        TIMER_DESKTOPNAKPACKET_TIMEOUT_ID       = 9,
        TIMER_BUILD_DESKTOPPACKETS_ID           = 10,
        TIMER_QUERY_MTU_ID                      = 11,
        TIMER_STOP_AUDIOINPUT                   = 12,
        TIMER_REMOVE_LOCALPLAYBACK              = 13,
        TIMER_STOP_STREAM_MEDIAFILE_ID          = 14,

        //User instance timers (termination not handled by ClientNode::StopTimer())
        USER_TIMER_MASK                         = USER_TIMER_START,

        USER_TIMER_VOICE_PLAYBACK_ID            = USER_TIMER_MASK + 2,
        USER_TIMER_MEDIAFILE_AUDIO_PLAYBACK_ID  = USER_TIMER_MASK + 3,
        USER_TIMER_MEDIAFILE_VIDEO_PLAYBACK_ID  = USER_TIMER_MASK + 4,
        USER_TIMER_DESKTOPACKPACKET_ID          = USER_TIMER_MASK + 5,
        USER_TIMER_DESKTOPINPUT_RTX_ID          = USER_TIMER_MASK + 7,
        USER_TIMER_DESKTOPINPUT_ACK_ID          = USER_TIMER_MASK + 8,
        USER_TIMER_REMOVE_FILETRANSFER_ID       = USER_TIMER_MASK + 9,
        USER_TIMER_UPDATE_USER                  = USER_TIMER_MASK + 10,
        USER_TIMER_JITTER_BUFFER_ID             = USER_TIMER_MASK + 11,
    };

    struct SoundProperties
    {
        int inputdeviceid = SOUNDDEVICE_IGNORE_ID;
        int outputdeviceid = SOUNDDEVICE_IGNORE_ID;
        //sound group for current instance
        int soundgroupid = 0;
        // AGC, AEC and denoise settings
        AudioPreprocessor preprocessor;
        //dereverb
        bool dereverb = true;
        //count transmitted samples
        uint32_t samples_transmitted = 0;
        //total samples recorded
        uint32_t samples_recorded = 0;
        uint32_t samples_delay_msec = 0;
        SoundDeviceEffects effects;

        SoundProperties()
        {
            // default to TT Audio preprocessor to be compatible with
            // SetVoiceGainLevel()
            preprocessor.preprocessor = AUDIOPREPROCESSOR_TEAMTALK;
            preprocessor.ttpreprocessor.gainlevel = GAIN_NORMAL;
            preprocessor.ttpreprocessor.muteleft = preprocessor.ttpreprocessor.muteright = false;
        }
    };

    class EventSuspender
    {
    public:
        virtual ~EventSuspender() = default;
        virtual bool CanSuspend() = 0;
        virtual void SuspendEventHandling(bool quit) = 0;
        virtual void ResumeEventHandling() = 0;
    };

    // subclass must implement TimerEvent()
    class ClientNodeBase : protected ACE_Task<ACE_MT_SYNCH>
                         , public TimerListener
                         , public soundsystem::StreamDuplex
                         , public EventSuspender
    {
        // Thread func running ClientNode's main event-loop
        int svc() override;

        //timer queue we can change to high-resolution timer (monotomic)
        ACE_Timer_Heap m_timer_queue;
        //the reactor associated with this client instance
        ACE_Reactor m_reactor;
        ACE_thread_t m_reactor_thread = ACE_thread_t();

        // sync reactor thread start/stop
        std::condition_variable m_reactor_wait_cv;
        std::mutex m_reactor_wait_mtx;

        ACE_Recursive_Thread_Mutex m_timers_lock; //mutexes must be the last to be destroyed

        //set of timers currently in use. Protected by lock_timers().
        timer_handlers_t m_timers;

        ACE_Recursive_Thread_Mutex& LockTimers() { return m_timers_lock; }

    protected:
        //Start/stop timers handled by ClientNode
        long StartTimer(uint32_t timer_id, long userdata,
                        const ACE_Time_Value& delay,
                        const ACE_Time_Value& interval = ACE_Time_Value::zero);
        bool StopTimer(uint32_t timer_id);
        //remove timer from timer set (without stopping it)
        void ClearTimer(uint32_t timer_id);
        // Clear all timers in 'm_timers'
        void ResetTimers();

    public:

        ClientNodeBase();
        ~ClientNodeBase() override;

        bool CanSuspend() override;
        void SuspendEventHandling(bool quit) override;
        void ResumeEventHandling() override;

        ACE_Lock& ReactorLock();
#if defined(_DEBUG)
        ACE_thread_t m_reactorlock_thr_id = ACE_Thread::self();
#endif

        // Get reactor running timers
        ACE_Reactor* GetEventLoop() { return reactor(); }

        // Start timer running in GetEventLoop()
        virtual long StartUserTimer(uint16_t timer_id, uint16_t userid,
                                    long userdata, const ACE_Time_Value& delay,
                                    const ACE_Time_Value& interval = ACE_Time_Value::zero);
        virtual bool StopUserTimer(uint16_t timer_id, uint16_t userid);
        virtual bool TimerExists(uint32_t timer_id);
        virtual bool TimerExists(uint32_t timer_id, int userid);

        // Whether sound system is running is duplex mode, soundsystem::OpenDuplexStream()
        virtual bool SoundDuplexMode() = 0;

        // Get my user-ID (0 = not set)
        virtual int GetUserID() const = 0;
        // Get ID of current channel (0 = not set)
        virtual int GetChannelID() = 0;

        // Queue packet for transmission
        virtual bool QueuePacket(FieldPacket* packet) = 0;
        // Get logger for writing audio streams to disk (wav, ogg, etc)
        virtual class VoiceLogger& GetVoiceLogger() = 0;

        // Callback function for teamtalk::AudioPlayer-class
        virtual void AudioUserCallback(int userid, StreamType st,
                                       const media::AudioFrame& audio_frame) = 0;

    };


#if defined(_DEBUG)

#define ASSERT_CLIENTNODE_LOCKED(clientnode) do { /* NOLINT */ \
    TTASSERT(clientnode->m_reactorlock_thr_id == ACE_Thread::self());   \
    } while(0)

#define GUARD_REACTOR(this_obj)                         \
    guard_t g( this_obj->ReactorLock() );               \
    /*PROFILER_ST(ACE_TEXT("Thread"));*/                \
    this_obj->m_reactorlock_thr_id = ACE_Thread::self()
#else

#define ASSERT_CLIENTNODE_LOCKED(...)    (void)0

#define GUARD_REACTOR(this_obj)                 \
    guard_t g(this_obj->ReactorLock())

#endif /* _DEBUG */

} // namespace teamtalk

#endif
