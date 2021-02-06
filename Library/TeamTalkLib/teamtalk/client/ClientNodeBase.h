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

#include <ace/Reactor.h>

#define TIMERID_MASK            0x0000FFFF
#define USER_TIMER_START        0x00008000
#define USER_TIMER_USERID_MASK  0xFFFF0000 // ((userid << 16) | USER_TIMER_START) + TIMERID
#define USER_TIMER_USERID_SHIFT 16

#define USER_TIMERID(timerid, userid) ((userid << USER_TIMER_USERID_SHIFT) | timerid)
#define TIMER_USERID(timerid) ((timerid >> USER_TIMER_USERID_SHIFT) & 0xFFFF)

namespace teamtalk {

    class EventSuspender
    {
    public:
        virtual void SuspendEventHandling(bool quit = false) = 0;
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

        //the reactor associated with this client instance
        ACE_Reactor m_reactor;

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

        void SuspendEventHandling(bool quit = false) override;
        void ResumeEventHandling() override;

        ACE_Lock& reactor_lock();
        ACE_Semaphore m_reactor_wait;

        // Get reactor running timers
        ACE_Reactor* GetEventLoop() { return reactor(); }

        // Start timer running in GetEventLoop()
        virtual long StartUserTimer(uint16_t timer_id, uint16_t userid,
                                    long userdata, const ACE_Time_Value& delay,
                                    const ACE_Time_Value& interval = ACE_Time_Value::zero);
        virtual bool StopUserTimer(uint16_t timer_id, uint16_t userid);
        virtual bool TimerExists(uint32_t timer_id);
        virtual bool TimerExists(uint32_t timer_id, int userid);

        // Sound system is running is duplex mode, soundsystem::OpenDuplexStream()
        virtual bool SoundDuplexMode() = 0;

        // Get my user-ID (0 = not set)
        virtual int GetUserID() const = 0;
        // Get ID of current channel (0 = not set)
        virtual int GetChannelID() = 0;

        // Queue packet for transmission
        virtual bool QueuePacket(FieldPacket* packet) = 0;
        // Get logger for writing audio streams to disk (wav, ogg, etc)
        virtual VoiceLogger& voicelogger() = 0;

        // Callback function for teamtalk::AudioPlayer-class
        virtual void AudioUserCallback(int userid, StreamType st,
                                       const media::AudioFrame& audio_frame) = 0;

    };
}

#endif
