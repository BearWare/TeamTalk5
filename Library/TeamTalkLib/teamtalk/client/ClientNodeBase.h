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

namespace teamtalk {

    // subclass must implement TimerEvent()
    class ClientNodeBase : public TimerListener, public soundsystem::StreamDuplex
    {
    public:
        virtual ~ClientNodeBase() {}

        // Get reactor running timers
        virtual ACE_Reactor* GetEventLoop() = 0;
        // Start timer running in GetEventLoop()
        virtual long StartUserTimer(uint16_t timer_id, uint16_t userid, 
                                    long userdata, const ACE_Time_Value& delay, 
                                    const ACE_Time_Value& interval = ACE_Time_Value::zero) = 0;
        virtual bool StopUserTimer(uint16_t timer_id, uint16_t userid) = 0;
        virtual bool TimerExists(ACE_UINT32 timer_id) = 0;
        virtual bool TimerExists(ACE_UINT32 timer_id, int userid) = 0;

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
