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
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Synch_Traits.h>
#include <ace/Time_Value.h>
#include <ace/Timer_Heap.h>
#include <ace/Timer_Queue_Adapters.h>

#include <cstdint>
#include <fstream>
#include <set>
#include <vector>

//reactor event task
ACE_THR_FUNC_RETURN EventLoop (void *arg);
void SyncReactor(ACE_Reactor& reactor);

using wguard_t = ACE_Write_Guard<ACE_Recursive_Thread_Mutex>;
using rguard_t = ACE_Read_Guard<ACE_Recursive_Thread_Mutex>;
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

class MyFile : private NonCopyable
{
public:
    bool Open(const tstring& filename, bool readonly = true);
    bool NewFile(const tstring& filename);
    void Close();

    int64_t Read(char* buf, std::streamsize size);
    int64_t Write(const char* buf, std::streamsize size);
    bool Seek(int64_t size, std::ios_base::seekdir way);
    int64_t Tell();

private:
    std::fstream m_file;
    bool m_readonly = true;
};

using intset_t = std::set<int>;
using intvec_t = std::vector<int>;

ACE_Time_Value ToTimeValue(int64_t msec);

tstring UptimeHours(const ACE_Time_Value& value);

#if defined(_DEBUG)

void MYTRACE(const tchar* trace_str, ...); /* NOLINT(cppcoreguidelines-pro-type-vararg) */

#define MYTRACE_COND(check, trace_str, ...)                       \
    do { /* NOLINT */                                             \
        if(check) MYTRACE(trace_str, ##__VA_ARGS__); /* NOLINT */ \
    } while(0)

#else

#define MYTRACE(...)           (static_cast<void>(0)) 
#define MYTRACE_COND(...)      (static_cast<void>(0))

#endif /* _DEBUG */

class Profiler
{
public:
    Profiler(const tchar* name, const tchar* file, int line, bool p_start);
    ~Profiler();
private:
    ACE_Time_Value m_start;
    const tchar* m_name;
    const tchar* m_filename;
    int m_line;
};

#define PROFILER(name)   Profiler d1d(name, ACE_TEXT( __FILE__ ), __LINE__, false)
#define PROFILER_ST(name)   Profiler d1d(name, ACE_TEXT( __FILE__ ), __LINE__, true)

#define DUP_TIMESTAMP_DELAY(tm)          \
    do {                                 \
        while((tm) == GETTIMESTAMP())    \
            std::this_thread::yield();   \
    } while(0)

constexpr auto W32_GEQ(uint32_t a, uint32_t b) { return (static_cast<int32_t>(a-b) >= 0); }
constexpr auto W32_GT(uint32_t a, uint32_t b) { return (static_cast<int32_t>(a-b) > 0); }
constexpr auto W32_LEQ(uint32_t a, uint32_t b) { return (static_cast<int32_t>(a-b) <= 0); }
constexpr auto W32_LT(uint32_t a, uint32_t b) { return (static_cast<int32_t>(a-b) < 0); }

constexpr auto W16_GEQ(uint16_t a, uint16_t b) { return (static_cast<int16_t>(a-b) >= 0); }
constexpr auto W16_GT(uint16_t a, uint16_t b) { return (static_cast<int16_t>(a-b) > 0); }
constexpr auto W16_LEQ(uint16_t a, uint16_t b) { return (static_cast<int16_t>(a-b) <= 0); }
constexpr auto W16_LT(uint16_t a, uint16_t b) { return (static_cast<int16_t>(a-b) < 0); }

constexpr auto W8_GEQ(uint8_t a, uint8_t b) { return (static_cast<int8_t>(a-b) >= 0); }
constexpr auto W8_GT(uint8_t a, uint8_t b) { return (static_cast<int8_t>(a-b) > 0); }
constexpr auto W8_LEQ(uint8_t a, uint8_t b) { return (static_cast<int8_t>(a-b) <= 0); }
constexpr auto W8_LT(uint8_t a, uint8_t b) { return (static_cast<int8_t>(a-b) < 0); }

struct W32LessComp
{
    bool operator() (const uint32_t& a, const uint32_t& b) const
        { return W32_LT(a,b); }
};

struct W16LessComp
{
    bool operator() (const uint16_t& a, const uint16_t& b) const
        { return W16_LT(a,b); }
};

#endif /* MYACE_H */
