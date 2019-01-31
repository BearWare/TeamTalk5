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

#if !defined(TIMERHANDLER_H)
#define TIMERHANDLER_H

#include <ace/Event_Handler.h>

#include <map>

class TimerListener
{
public:
    virtual ~TimerListener() {}
    virtual int TimerEvent(ACE_UINT32 timer_event_id, long userdata) = 0; //returning -1 will call 'delete this'
};

typedef std::map<ACE_UINT32, long> timers_t;

class TimerHandler : public ACE_Event_Handler
{
public:
    TimerHandler(TimerListener& listener, ACE_UINT32 timer_event_id, long userdata = 0);
    virtual ~TimerHandler();

    int handle_timeout(const ACE_Time_Value& tv, const void* arg);
    int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask);
private:
    TimerListener& m_listener;
    ACE_UINT32 m_timer_event_id;
    long m_userdata;
};

typedef std::map<ACE_UINT32, TimerHandler*> timer_handlers_t;

#endif
