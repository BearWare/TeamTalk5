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

#include <myace/TimerHandler.h>
#include <avstream/SoundSystem.h>
#include <codec/MediaUtil.h>
#include <teamtalk/PacketLayout.h>

#include <ace/High_Res_Timer.h>
#include <ace/Reactor.h>
#include <ace/Timer_Heap.h>

#include <condition_variable>
#include <mutex>

#define TIMERID_MASK            0x0000FFFF
#define USER_TIMER_START        0x00008000
#define USER_TIMER_USERID_MASK  0xFFFF0000 // ((userid << 16) | USER_TIMER_START) + TIMERID
#define USER_TIMER_USERID_SHIFT 16

#define USER_TIMERID(timerid, userid) ((userid << USER_TIMER_USERID_SHIFT) | timerid)
#define TIMER_USERID(timerid) ((timerid >> USER_TIMER_USERID_SHIFT) & 0xFFFF)

#define SOUNDDEVICE_IGNORE_ID -1

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
        int inputdeviceid;
        int outputdeviceid;
        //sound group for current instance
        int soundgroupid;
        // AGC, AEC and denoise settings
        AudioPreprocessor preprocessor;
        //dereverb
        bool dereverb;
        //count transmitted samples
        uint32_t samples_transmitted;
        //total samples recorded
        uint32_t samples_recorded;
        uint32_t samples_delay_msec;
        SoundDeviceEffects effects;

        SoundProperties()
        {
            inputdeviceid = outputdeviceid = SOUNDDEVICE_IGNORE_ID;
            soundgroupid = 0;
            dereverb = true;
            samples_transmitted = 0;
            samples_recorded = 0;
            samples_delay_msec = 0;
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
        int svc(void) override;

        //timer queue we can change to high-resolution timer (monotomic)
        ACE_Timer_Heap m_timer_queue;
        //the reactor associated with this client instance
        ACE_Reactor m_reactor;
        ACE_thread_t m_reactor_thread;

        // sync reactor thread start/stop
        std::condition_variable m_reactor_wait_cv;
        std::mutex m_reactor_wait_mtx;

        ACE_Recursive_Thread_Mutex m_timers_lock; //mutexes must be the last to be destroyed

        //set of timers currently in use. Protected by lock_timers().
        timer_handlers_t m_timers;

        ACE_Recursive_Thread_Mutex& lock_timers() { return m_timers_lock; }

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
        virtual ~ClientNodeBase();

        bool CanSuspend() override;
        void SuspendEventHandling(bool quit) override;
        void ResumeEventHandling() override;

        ACE_Lock& reactor_lock();

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
        virtual class VoiceLogger& voicelogger() = 0;

        // Callback function for teamtalk::AudioPlayer-class
        virtual void AudioUserCallback(int userid, StreamType st,
                                       const media::AudioFrame& audio_frame) = 0;

    };
}

#endif
