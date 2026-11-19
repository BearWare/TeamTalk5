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

#if !defined(MYSTD_H)
#define MYSTD_H

#include <chrono>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <fstream>
#include <regex>
#include <set>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

//tstring is std::wstring on Windows and std::string on other platforms.
#if defined(UNICODE)
using tstring = std::wstring;
using tostringstream = std::wostringstream;
using tchar = wchar_t;
#define TTEXT(x) L##x
#else
using tstring = std::string;
using tostringstream = std::ostringstream;
using tchar = char;
#define TTEXT(x) x
#endif

using strings_t = std::vector<tstring>;
using intset_t = std::set<int>;
using intvec_t = std::vector<int>;

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

//directory separator used in tstring file paths
#if defined(WIN32)
constexpr tstring::value_type DIRECTORY_SEPARATOR_CHAR = '\\';
#else
constexpr tstring::value_type DIRECTORY_SEPARATOR_CHAR = '/';
#endif

tstring FixFilePath(const tstring& filepath);

void ReplaceAll(tstring& target, const tstring& to_find, const tstring& replacement);

tstring I2String(int64_t i);
int64_t String2I(const tstring& int_str, int base = 10, int64_t errorValue = 0);

bool StringCmpNoCase(const tstring& str1, const tstring& str2);
tstring StringToLower(const tstring& str);
strings_t Tokenize(const tstring& source, const tstring& delimeters);

tstring KeyToHexString(const unsigned char* key, std::size_t length);
void HexStringToKey(const tstring& crypt_key, uint8_t* key, std::size_t keylen);

bool VersionSameOrLater(const tstring& check, const tstring& against);

#if defined(UNICODE)
std::string UnicodeToUtf8(const wchar_t* unicode, int len = -1);
std::string UnicodeToUtf8(const tstring& str);
tstring Utf8ToUnicode(const char* utf8, int len = -1);
std::string UnicodeToLocal(const tstring& str);
tstring LocalToUnicode(const std::string& str);
std::string Utf8ToLocal(const char* local, int len = -1);
#else
#define UnicodeToUtf8(str)      (str)
#define Utf8ToUnicode(utf8)     (utf8)
#define UnicodeToLocal(str)     (str)
#define LocalToUnicode(str)     (str)
#define Utf8ToLocal(local)      (local)
#endif

bool ValidUtf8(const std::string& utf8_str);
std::string LimitUtf8(const std::string& utf8_str, size_t maxlen);

#if defined(UNICODE)
std::wregex BuildRegex(const std::wstring& regexstr);
#else
std::regex BuildRegex(const std::string& regexstr);
#endif

uint32_t GETTIMESTAMP();

void DUP_TIMESTAMP_DELAY(uint32_t tm);

//tchar versions of tstrncpy, tsnprintf and tstrsncpy
inline tchar* tstrncpy(tchar* dest, const tchar* src, size_t n)
{
#if defined(UNICODE)
    return std::wcsncpy(dest, src, n);
#else
    return std::strncpy(dest, src, n);
#endif
}

inline int tvsnprintf(tchar* buf, size_t n, const tchar* fmt, va_list args)
{
#if defined(UNICODE)
    return std::vswprintf(buf, n, fmt, args);
#else
    return std::vsnprintf(buf, n, fmt, args);
#endif
}

inline int tsnprintf(tchar* buf, size_t n, const tchar* fmt, ...) // NOLINT(cert-dcl50-cpp)
{
    va_list args;
    va_start(args, fmt);
    int const ret = tvsnprintf(buf, n, fmt, args);
    va_end(args);
    return ret;
}

//like tstrsncpy: copies at most maxlen-1 chars and always
//null-terminates (if maxlen > 0); returns dest
inline tchar* tstrsncpy(tchar* dest, const tchar* src, size_t maxlen)
{
    if (maxlen == 0)
        return dest;
    if (src == nullptr)
        src = TTEXT("");
    size_t const len = std::char_traits<tchar>::length(src);
    size_t const n = len >= maxlen ? maxlen - 1 : len;
    std::char_traits<tchar>::copy(dest, src, n);
    dest[n] = tchar();
    return dest;
}

//tchar version of ACE_OS::strrchr: returns pointer to last occurrence
//of 'c' in 's', or nullptr
inline const tchar* tstrrchr(const tchar* s, int c)
{
#if defined(UNICODE)
    return std::wcsrchr(s, static_cast<wchar_t>(c));
#else
    return std::strrchr(s, c);
#endif
}

class NonCopyable
{
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
public:
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable& operator=(const NonCopyable &) = delete;
    NonCopyable(NonCopyable&&) = delete;
    NonCopyable& operator=(NonCopyable&&) = delete;
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

struct Rational
{
    int numerator;
    int denominator;
    Rational(int n, int d) : numerator(n), denominator(d) { }
    bool IsOne() const { return numerator == denominator; }
};

class Profiler
{
public:
    Profiler(const tchar* name, const tchar* file, int line, bool p_start);
    ~Profiler();
private:
    std::chrono::steady_clock::time_point m_start;
    const tchar* m_name;
    const tchar* m_filename;
    int m_line;
};

#define PROFILER(name)      Profiler d1d(name, TTEXT( __FILE__ ), __LINE__, false)
#define PROFILER_ST(name)   Profiler d1d(name, TTEXT( __FILE__ ), __LINE__, true)

//wraparound-safe comparisons of unsigned 32/16/8-bit counters
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

#endif

