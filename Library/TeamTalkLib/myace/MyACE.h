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

#include <mystd/MyStd.h>

#include <ace/Reactor.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Lock.h>
#include <ace/Guard_T.h>
#include <ace/Timer_Queue_Adapters.h>
#include <ace/Timer_Heap.h>
#include <ace/SString.h>
#include <vector>
#include <set>
#include <fstream>

//reactor event task
ACE_THR_FUNC_RETURN event_loop (void *arg);
void SyncReactor(ACE_Reactor& reactor);

typedef ACE_Write_Guard<ACE_Recursive_Thread_Mutex> wguard_t;
typedef ACE_Read_Guard<ACE_Recursive_Thread_Mutex> rguard_t;
typedef ACE_Guard<ACE_Lock> guard_t;

typedef ACE_Thread_Timer_Queue_Adapter<ACE_Timer_Heap> ActiveTimer;

typedef ACE_Message_Queue<ACE_MT_SYNCH> msg_queue_t;

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
    bool Open(const ACE_TString& filename, bool readonly = true);
    bool NewFile(const ACE_TString& filename);
    void Close();

    int64_t Read(char* buf, std::streamsize size);
    int64_t Write(const char* buf, std::streamsize size);
    bool Seek(int64_t size, std::ios_base::seekdir way);
    int64_t Tell();

private:
    std::fstream m_file;
    bool m_readonly = true;
};

typedef std::vector< ACE_TString > strings_t;
typedef std::set<int> intset_t;
typedef std::vector<int> intvec_t;

bool ExtractFileName(const ACE_TString& filepath, ACE_TString& filename);
ACE_TString FixFilePath(const ACE_TString& filepath);

ACE_TString stringtolower(const ACE_TString& str);

void replace_all(ACE_TString& target, const ACE_TString& to_find, const ACE_TString& replacement );

ACE_TString i2string(ACE_INT64 i);
ACE_INT64 string2i(const ACE_TString& int_str, int base = 10);

bool stringcmpnocase(const ACE_TString& str1, const ACE_TString& str2);
strings_t tokenize(const ACE_TString& source, const ACE_TString& delimeters);

ACE_TString KeyToHexString(const unsigned char* key, int length);
void HexStringToKey(const ACE_TString& crypt_key, unsigned char* key);

ACE_Time_Value ToTimeValue(int msec);

ACE_TString UptimeHours(const ACE_Time_Value& value);

#if defined(UNICODE)
ACE_CString UnicodeToUtf8(const wchar_t* unicode, int len = -1);
ACE_CString UnicodeToUtf8(const ACE_WString& str);
ACE_WString Utf8ToUnicode(const char* utf8, int len = -1);
ACE_CString UnicodeToLocal(const ACE_WString& str);
ACE_WString LocalToUnicode(const ACE_CString& str);
ACE_CString LocalToUtf8(const char* local, int len = -1);
ACE_CString Utf8ToLocal(const char* local, int len = -1);
//bool ValidUtf8(const ACE_CString& utf8_str);
#else
#define UnicodeToUtf8(str)      (str)
#define Utf8ToUnicode(utf8)     (utf8)
#define UnicodeToLocal(str)     (str)
#define LocalToUnicode(str)     (str)
#define LocalToUtf8(local)      (local)
#define Utf8ToLocal(local)      (local)
#endif

bool ValidUtf8(const ACE_CString& utf8_str);
ACE_CString LimitUtf8(const ACE_CString& utf8_str, size_t maxlen);

#if !defined(MYTRACE)

#if defined(_DEBUG)

void MYTRACE(const ACE_TCHAR* trace_str, ...);

#define MYTRACE_COND(check, trace_str, ...)         \
    do {                                            \
        if(check) MYTRACE(trace_str, ##__VA_ARGS__);  \
    } while(0)

#else

#define MYTRACE(...)           ((void)0) 
#define MYTRACE_COND(...)      ((void)0)

#endif /* _DEBUG */

#endif /* MYTRACE */

class Profiler
{
public:
    Profiler(const ACE_TCHAR* name, const ACE_TCHAR* file, int line, bool p_start);
    ~Profiler();
private:
    ACE_Time_Value m_start;
    const ACE_TCHAR* m_name;
    const ACE_TCHAR* m_filename;
    int m_line;
};

#define PROFILER(name)   Profiler d1d(name, ACE_TEXT( __FILE__ ), __LINE__, false)
#define PROFILER_ST(name)   Profiler d1d(name, ACE_TEXT( __FILE__ ), __LINE__, true)

bool VersionSameOrLater(const ACE_TString& check, const ACE_TString& against);

#define DUP_TIMESTAMP_DELAY(tm)          \
    do {                                 \
        while((tm) == GETTIMESTAMP())    \
            ACE_OS::sleep(0);            \
    } while(0)

constexpr auto W32_GEQ(ACE_UINT32 a, ACE_UINT32 b) { return ((ACE_INT32)((a)-(b)) >= 0); }
constexpr auto W32_GT(ACE_UINT32 a, ACE_UINT32 b) { return ((ACE_INT32)((a)-(b)) > 0); }
constexpr auto W32_LEQ(ACE_UINT32 a, ACE_UINT32 b) { return ((ACE_INT32)((a)-(b)) <= 0); }
constexpr auto W32_LT(ACE_UINT32 a, ACE_UINT32 b) { return ((ACE_INT32)((a)-(b)) < 0); }

constexpr auto W16_GEQ(ACE_UINT16 a, ACE_UINT16 b) { return ((ACE_INT16)((a)-(b)) >= 0); }
constexpr auto W16_GT(ACE_UINT16 a, ACE_UINT16 b) { return ((ACE_INT16)((a)-(b)) > 0); }
constexpr auto W16_LEQ(ACE_UINT16 a, ACE_UINT16 b) { return ((ACE_INT16)((a)-(b)) <= 0); }
constexpr auto W16_LT(ACE_UINT16 a, ACE_UINT16 b) { return ((ACE_INT16)((a)-(b)) < 0); }

constexpr auto W8_GEQ(ACE_UINT8 a, ACE_UINT8 b) { return ((ACE_INT8)((a)-(b)) >= 0); }
constexpr auto W8_GT(ACE_UINT8 a, ACE_UINT8 b) { return ((ACE_INT8)((a)-(b)) > 0); }
constexpr auto W8_LEQ(ACE_UINT8 a, ACE_UINT8 b) { return ((ACE_INT8)((a)-(b)) <= 0); }
constexpr auto W8_LT(ACE_UINT8 a, ACE_UINT8 b) { return ((ACE_INT8)((a)-(b)) < 0); }

struct w32_less_comp
{
    bool operator() (const ACE_UINT32& a, const ACE_UINT32& b) const
        { return W32_LT(a,b); }
};

struct w16_less_comp
{
    bool operator() (const ACE_UINT16& a, const ACE_UINT16& b) const
        { return W16_LT(a,b); }
};

#endif /* MYACE_H */
