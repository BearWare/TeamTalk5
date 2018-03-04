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

#ifndef _TTASSERT_H
#define _TTASSERT_H

void tt_assert(const char* assertion, const char* file, int line);

#if defined(NDEBUG)

#define TTASSERT(_EX) ((_EX) ? (void)0 : tt_assert(#_EX, __FILE__, __LINE__))

#else  // !NDEBUG

#include <assert.h>

#define TTASSERT(_EX) do { (_EX)? (void)0 : tt_assert(#_EX, __FILE__, __LINE__);  assert(_EX); } while(0)

#endif


#ifdef _DEBUG

//ensure that it's the thread owner of the reactor which is executing
#define ASSERT_REACTOR_THREAD(reactor)                                  \
    do {                                                                \
        ACE_thread_t tid;                                               \
        (reactor).owner(&tid);                                          \
        TTASSERT(ACE_Thread::self() == tid);                            \
    } while(0)

#define ASSERT_NOT_REACTOR_THREAD(reactor)                              \
    do {                                                                \
        ACE_thread_t tid;                                               \
        (reactor).owner(&tid);                                          \
        TTASSERT(ACE_Thread::self() != tid);                            \
    } while(0)

#if defined(ACE_WIN32) /* Win32 cannot see who holds a lock */
#define ASSERT_MUTEX_LOCKED(lock)                                       \
    TTASSERT((lock).get_nesting_level()>0)
#else
#define ASSERT_MUTEX_LOCKED(lock)                                       \
    TTASSERT(ACE_Thread::self() == (lock).get_thread_id())
#endif /* ACE_WIN32 */

#else
#define ASSERT_REACTOR_THREAD(...)             (void)0
#define ASSERT_NOT_REACTOR_THREAD(...)         (void)0
#define ASSERT_MUTEX_LOCKED(...)               (void)0
#endif

#endif // _TTASSERT_H
