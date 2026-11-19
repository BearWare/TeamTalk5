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

#if !defined(MYACE_H)
#define MYACE_H

#include "mystd/MyStd.h"

#include <ace/Guard_T.h>
#include <ace/Lock.h>
#include <ace/Message_Block.h>
#include <ace/Message_Queue_T.h>
#include <ace/Reactor.h>
#include <ace/Synch_Traits.h>
#include <ace/Time_Value.h>
#include <ace/Timer_Heap.h>
#include <ace/Timer_Queue_Adapters.h>

#include <cstdint>

//reactor event task
ACE_THR_FUNC_RETURN EventLoop (void *arg);
void SyncReactor(ACE_Reactor& reactor);

using wguard_t = std::unique_lock<std::recursive_mutex>;
using rguard_t = std::unique_lock<std::recursive_mutex>; // ACE_Read_Guard on a recursive mutex is exclusive too
using guard_t = ACE_Guard<ACE_Lock>;

using ActiveTimer = ACE_Thread_Timer_Queue_Adapter<ACE_Timer_Heap>;

using msg_queue_t = ACE_Message_Queue<ACE_MT_SYNCH>;

class MBGuard : NonCopyable
{
    ACE_Message_Block* m_mb;
public:
    explicit MBGuard(ACE_Message_Block* mb) : m_mb(mb) { }
    ~MBGuard() { m_mb->release(); }
};

ACE_Time_Value ToTimeValue(int64_t msec);

tstring UptimeHours(const ACE_Time_Value& value);

#endif /* MYACE_H */
