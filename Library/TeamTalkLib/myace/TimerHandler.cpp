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

#include "TimerHandler.h"
#include <myace/MyACE.h>

TimerHandler::TimerHandler(TimerListener& listener, ACE_UINT32 timer_event_id, long userdata)
: m_listener(listener)
{
    m_timer_event_id = timer_event_id;
    m_userdata = userdata;
    //MYTRACE(ACE_TEXT("TimerHandler() %p ID %d \n"), this, m_timer_event_id);
}

TimerHandler::~TimerHandler()
{
    //MYTRACE(ACE_TEXT("~TimerHandler() %p ID %d \n"), this, m_timer_event_id);
}

int TimerHandler::handle_timeout(const ACE_Time_Value& tv, const void* arg)
{
    return m_listener.TimerEvent(m_timer_event_id, m_userdata);
}

int TimerHandler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
    delete this;
    return -1;
}

