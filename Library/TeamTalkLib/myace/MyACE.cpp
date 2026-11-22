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

#include "MyACE.h"
#include "mystd/MyStd.h"

#include <ace/ACE.h>
#include <ace/OS.h> // NOLINT(misc-include-cleaner)

#include <array>
#include <cassert>
#include <cstdint>
#include <ctime>
#include <thread>

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-pointer-arithmetic)

/*******************************************************************/
/************************* Helper functions ************************/
/*******************************************************************/
ACE_THR_FUNC_RETURN EventLoop (void *arg)
{
    auto *reactor = static_cast<ACE_Reactor *> (arg);
    assert(reactor);

    reactor->owner (ACE_OS::thr_self ());
    reactor->run_reactor_event_loop ();
    return {};
}

void SyncReactor(ACE_Reactor& reactor)
{
    ACE_thread_t tid{};
    while(reactor.owner(&tid) < 0 || tid == ACE_OS::thr_self())
    {
        std::this_thread::yield();
    }
}

ACE_Time_Value ToTimeValue(int64_t msec)
{
    constexpr auto ONE_SEC = 1000;
    return ACE_Time_Value(msec / ONE_SEC, (msec % ONE_SEC) * ONE_SEC);
}

