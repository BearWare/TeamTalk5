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
#include <cstdarg>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <thread>
#include <vector>

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

bool MyFile::Open(const tstring& filename, bool readonly /*= true*/)
{
    if (m_file.is_open())
        return false;

    std::ios_base::openmode mode = std::ios::in | std::ios::binary;
    if (!readonly)
        mode |= std::ios::out;

    m_file.open(filename.c_str(), mode);
    m_readonly = m_file.good() && readonly;
    return m_file.good();
}

bool MyFile::NewFile(const tstring& filename)
{
    m_file.open(filename.c_str(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
    m_readonly = !m_file.good();
    return m_file.good();
}

void MyFile::Close()
{
    m_file.close();
}

int64_t MyFile::Read(char* buf, std::streamsize size)
{
    assert(m_file.is_open());
    if (m_file.eof())
        return 0;

    MYTRACE_COND(m_file.bad(), ACE_TEXT("File read while bad()\n"));
    MYTRACE_COND(m_file.fail(), ACE_TEXT("File read while fail()\n"));
    MYTRACE_COND(!m_file.good(), ACE_TEXT("File read while !good()\n"));

    auto pos = m_file.tellg();

    if (m_file.read(buf, size).bad())
        return -1;

    if (m_file.fail()) // happens if extracted bytes < 'size'
        m_file.clear();

    auto newpos = m_file.tellg();

    // advance write pointer
    if (!m_readonly)
    {
        if (!m_file.seekp(newpos))
        {
            assert(0 /* unable to advance write buffer */);
            return -1;
        }
    }
    assert(m_readonly || m_file.tellg() == m_file.tellp());
    return newpos - pos;
}

int64_t MyFile::Write(const char* buf, std::streamsize size)
{
    assert(m_file.is_open());

    MYTRACE_COND(m_file.bad(), ACE_TEXT("File write while bad()\n"));
    MYTRACE_COND(m_file.fail(), ACE_TEXT("File write while fail()\n"));
    MYTRACE_COND(!m_file.good(), ACE_TEXT("File write while !good()\n"));

    auto pos = m_file.tellp();
    if (!m_file.write(buf, size))
        return -1;

    // advance read pointer
    auto newpos = m_file.tellp();
    if (!m_file.seekg(newpos))
        return -1;

    assert(m_file.tellg() == m_file.tellp());

    return newpos - pos;
}

bool MyFile::Seek(int64_t size, std::ios_base::seekdir way)
{
    MYTRACE_COND(m_file.bad(), ACE_TEXT("File seek while bad()\n"));
    MYTRACE_COND(m_file.fail(), ACE_TEXT("File seek while fail()\n"));
    MYTRACE_COND(!m_file.good(), ACE_TEXT("File seek while !good()\n"));

    if (!m_file.seekg(size, way))
        return false;

    // seekg and seekp are interchangeable for file streams. Weird!

    return !!m_file;
}

int64_t MyFile::Tell()
{
    MYTRACE_COND(m_file.bad(), ACE_TEXT("File tell while bad()\n"));
    MYTRACE_COND(m_file.fail(), ACE_TEXT("File tell while fail()\n"));
    MYTRACE_COND(!m_file.good(), ACE_TEXT("File tell while !good()\n"));

    assert(m_readonly || m_file.tellg() == m_file.tellp());
    return m_file.tellg();
}

#if defined(_DEBUG)

constexpr bool MYTRACE_TIMESTAMP = true;

#if defined(__ANDROID_API__)
#include <android/log.h>
#endif

#if defined(__APPLE__)
#include <os/log.h>
#endif

#if defined(WIN32)
#include <windows.h>
#endif

void MYTRACE(const ACE_TCHAR* trace_str, ...) // NOLINT(modernize-avoid-variadic-functions)
{
    va_list args;
    va_start(args, trace_str);
#if defined(__ANDROID_API__) || defined(WIN32) || defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE || defined(TARGET_OS_SIMULATOR) && TARGET_OS_SIMULATOR
    constexpr int MAX_TRACESTRLEN = 512;
#else
    constexpr int MAX_TRACESTRLEN = 0x10000;
#endif
    std::vector<ACE_TCHAR> str_buf(MAX_TRACESTRLEN);
    ACE_OS::strncpy(str_buf.data(), ACE_TEXT("[MYTRACE buffer overflow]\n"), MAX_TRACESTRLEN);
    std::array<ACE_TCHAR, MAX_TRACESTRLEN> tmp_str;
    static uint32_t begin = GETTIMESTAMP();
    static uint32_t next;
    next = GETTIMESTAMP();
    int out_len = 0;

    if (MYTRACE_TIMESTAMP)
    {
        ACE_OS::snprintf(tmp_str.data(), MAX_TRACESTRLEN, ACE_TEXT("%08u: %s"), next - begin, trace_str);
        out_len = ACE_OS::vsnprintf(str_buf.data(), MAX_TRACESTRLEN, tmp_str.data(), args);
    }
    else
    {
        out_len = ACE_OS::vsnprintf(str_buf.data(), MAX_TRACESTRLEN, trace_str, args);
    }

#if defined(__ANDROID_API__)
    __android_log_write(ANDROID_LOG_INFO, "bearware", &str_buf[0]);
#elif defined(WIN32)
    OutputDebugString(&str_buf[0]);
#elif defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE || defined(TARGET_OS_SIMULATOR) && TARGET_OS_SIMULATOR
    os_log_info(OS_LOG_DEFAULT, "%{public}s", &str_buf[0]);
#else
    std::cout << str_buf.data();
    if (out_len >= MAX_TRACESTRLEN)
        std::cout << "[MYTRACE buffer overflow]" << std::endl << std::flush;
#endif /* __ANDROID_API__ */

    va_end(args);
}
#endif /* _DEBUG */

tstring UptimeHours(const ACE_Time_Value& value)
{
    time_t const nHour = value.sec()/3600;
    time_t const nMinutes = (value.sec()%3600) / 60;
    time_t const nSec = (value.sec()%60);
    constexpr auto BUFLEN = 512;
    std::array<ACE_TCHAR, BUFLEN> buf;
    ACE_OS::snprintf(buf.data(), BUFLEN, ACE_TEXT("%d:%.2d:%.2d"), static_cast<int>(nHour), static_cast<int>(nMinutes), static_cast<int>(nSec));
    return buf.data();
}

ACE_Time_Value ToTimeValue(int64_t msec)
{
    constexpr auto ONE_SEC = 1000;
    return ACE_Time_Value(msec / ONE_SEC, (msec % ONE_SEC) * ONE_SEC);
}

Profiler::Profiler(const ACE_TCHAR* name, const ACE_TCHAR* file, int line, 
                   bool p_start)
: m_name(name)
, m_filename(file)
, m_line(line)
{
    m_start = ACE_OS::gettimeofday();
#if defined(WIN32) || defined(__ANDROID_API__)
    auto const h = static_cast<uint64_t>(ACE_OS::thr_self());
#else
    auto const h = reinterpret_cast<uint64_t>(ACE_OS::thr_self());
#endif
    MYTRACE_COND(p_start, ACE_TEXT("Profiler: %s started at %u, %s:%d thr: 0x%X\n"),
                 m_name, static_cast<uint32_t>(0), m_filename, m_line, static_cast<unsigned>(h));
}

Profiler::~Profiler()
{
    ACE_Time_Value const tm = ACE_OS::gettimeofday() - m_start;
#if defined(WIN32) || defined(__ANDROID_API__)
    auto const h = static_cast<uint64_t>(ACE_OS::thr_self());
#else
    auto const h = reinterpret_cast<uint64_t>(ACE_OS::thr_self());
#endif
    MYTRACE(ACE_TEXT("Profiler: %s completed in %u msec, %s:%d thr: 0x%X\n"),
            m_name, static_cast<uint32_t>(tm.msec()), m_filename, m_line, static_cast<unsigned>(h));
}

// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
