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

#include "ClientNodeBase.h"
#include <myace/MyACE.h>

#include <ace/Select_Reactor.h>

using namespace teamtalk;

#if defined(ACE_WIN32)
#define INVALID_THREAD_ID 0
#elif defined(__APPLE__)
#define INVALID_THREAD_ID nullptr
#else
#define INVALID_THREAD_ID ACE_INVALID_HANDLE
#endif

ClientNodeBase::ClientNodeBase()
    : m_reactor(new ACE_Select_Reactor(), true) //Ensure we don't use ACE_WFMO_Reactor!!!
    , m_reactor_thread(INVALID_THREAD_ID)
{
    this->reactor(&m_reactor);
}

ClientNodeBase::~ClientNodeBase()
{
    assert(thr_count() == 0);
}

int ClientNodeBase::svc(void)
{
    {
        // ensure .wait() is called prior to .notify_all()
        std::unique_lock<std::mutex> lck(m_reactor_wait_mtx);
    }

    m_reactor_thread = ACE_OS::thr_self();
    int ret = m_reactor.owner (ACE_OS::thr_self());
    assert(ret >= 0);

    m_reactor_wait_cv.notify_all();

    m_reactor.run_reactor_event_loop ();

    m_reactor_thread = INVALID_THREAD_ID;

    MYTRACE(ACE_TEXT("ClientNodeBase reactor thread exited.\n"));

    return 0;
}

bool ClientNodeBase::CanSuspend()
{
    return m_reactor_thread == ACE_OS::thr_self();
}

void ClientNodeBase::SuspendEventHandling(bool quit)
{
    MYTRACE( (ACE_TEXT("ClientNodeBase reactor thread suspending.\n")) );

    m_reactor.end_reactor_event_loop();

    // don't wait for thread to die if SuspendEventHandling() is called from reactor loop
    assert(quit || m_reactor_thread == ACE_OS::thr_self());
    if (m_reactor_thread != ACE_OS::thr_self())
        this->wait();

    MYTRACE(ACE_TEXT("ClientNodeBase reactor thread %s.\n"), (quit ? ACE_TEXT("quit") : ACE_TEXT("suspended")));
}

void ClientNodeBase::ResumeEventHandling()
{
    MYTRACE( (ACE_TEXT("ClientNodeBase reactor thread activating.\n")) );

    ACE_thread_t thr_id = 0;
    m_reactor.owner(&thr_id);
    if(thr_id != ACE_OS::thr_self())
    {
        MYTRACE( (ACE_TEXT("ClientNodeBase reactor thread waiting.\n")) );
        this->wait();
    }
    assert(m_reactor_thread == INVALID_THREAD_ID);

    m_reactor.reset_reactor_event_loop();

    std::unique_lock<std::mutex> lck(m_reactor_wait_mtx);
    int ret = this->activate();
    assert(ret >= 0);

    if (ret >= 0)
        m_reactor_wait_cv.wait(lck);

    MYTRACE( (ACE_TEXT("ClientNodeBase reactor thread activated.\n")) );
}

ACE_Lock& ClientNodeBase::reactor_lock()
{
    // char name[100] = "";
    // if(ACE_OS::thr_id(name, sizeof(name))>0)
    // {
    //     MYTRACE("Reactor lock obtained by %s\n", name);
    // }
    // else
    // {
    //     MYTRACE("Reactor lock obtained by %p\n", ACE_OS::thr_self());
    // }
    return m_reactor.lock();
}

long ClientNodeBase::StartTimer(uint32_t timer_id, long userdata,
                                const ACE_Time_Value& delay,
                                const ACE_Time_Value& interval)
{
    TimerHandler* th;
    ACE_NEW_RETURN(th, TimerHandler(*this, timer_id, userdata), -1);

    //ensure we don't have duplicate timers
    bool stoptimer = StopTimer(timer_id);
    MYTRACE_COND(stoptimer, ACE_TEXT("Starting timer which was already active: %u\n"), timer_id);

    //make sure we don't hold reactor lock when scheduling timer
    {
        //lock timer set
        wguard_t g(lock_timers());
        TTASSERT(m_timers.find(timer_id) == m_timers.end());
        m_timers[timer_id] = th; //put in before schedule because timeout might be 0
    }

    long reactor_timerid = m_reactor.schedule_timer(th, 0, delay, interval);
    TTASSERT(reactor_timerid>=0);
    if(reactor_timerid<0)
    {
        //lock timer set
        wguard_t g(lock_timers());
        m_timers.erase(timer_id);
        delete th;
    }

    return reactor_timerid;
}

bool ClientNodeBase::StopTimer(uint32_t timer_id)
{
    wguard_t g(lock_timers());

    timer_handlers_t::iterator ii = m_timers.find(timer_id);
    if(ii != m_timers.end())
    {
        TimerHandler* th = ii->second;
        m_timers.erase(ii);
        g.release(); //don't hold reactor lock when cancelling

        if(m_reactor.cancel_timer(th, 0) != -1)
            return true;
    }
    return false;
}

void ClientNodeBase::ClearTimer(uint32_t timer_id)
{
    //lock timer set
    wguard_t g(lock_timers());

    m_timers.erase(timer_id);
}

void ClientNodeBase::ResetTimers()
{
    while (m_timers.size())
    {
        m_reactor.cancel_timer(m_timers.begin()->second, 0);
        m_timers.erase(m_timers.begin());
    }
}

//Start/stop timers handled outside ClientNode
long ClientNodeBase::StartUserTimer(uint16_t timer_id, uint16_t userid,
                                    long userdata, const ACE_Time_Value& delay,
                                    const ACE_Time_Value& interval/* = ACE_Time_Value::zero*/)
{
    TTASSERT(timer_id & USER_TIMER_START);

    return StartTimer(USER_TIMERID(timer_id, userid), userdata, delay, interval);
}

bool ClientNodeBase::StopUserTimer(uint16_t timer_id, uint16_t userid)
{
    TTASSERT(timer_id & USER_TIMER_START);

    return StopTimer(USER_TIMERID(timer_id, userid));
}

bool ClientNodeBase::TimerExists(uint32_t timer_id)
{
    TTASSERT((timer_id & USER_TIMER_START) == 0);

    //lock timer set
    rguard_t g(lock_timers());

    return m_timers.find(timer_id) != m_timers.end();
}

bool ClientNodeBase::TimerExists(uint32_t timer_id, int userid)
{
    uint32_t tm_id = USER_TIMERID(timer_id, userid);
    //lock timer set
    rguard_t g(lock_timers());

    return m_timers.find(tm_id) != m_timers.end();
}
