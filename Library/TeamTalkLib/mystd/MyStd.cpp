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

#include "MyStd.h"

#if defined(UNICODE)
#if defined(WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include <cwchar>
#include <cwctype>
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <chrono>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <regex>
#include <string>
#include <thread>
#include <vector>

/*******************************************************************/
/************************* Helper functions ************************/
/*******************************************************************/

tstring FixFilePath(const tstring& filepath)
{
    tstring tmp = filepath;
    tstring tofind(2, DIRECTORY_SEPARATOR_CHAR);
    while (tmp.find(tofind) != tstring::npos)
        ReplaceAll(tmp, tofind, tstring(1, DIRECTORY_SEPARATOR_CHAR));

    if (!tmp.empty() && tmp[tmp.length()-1] == DIRECTORY_SEPARATOR_CHAR)
        tmp = tmp.substr(0, tmp.length()-1);
    return tmp;
}

void ReplaceAll(tstring& target, const tstring& to_find, const tstring& replacement )
{
    if (to_find.empty())
        return;

    //replace in-place; advance past the inserted replacement so a
    //replacement containing 'to_find' is not replaced again
    for (tstring::size_type pos = target.find(to_find);
         pos != tstring::npos;
         pos = target.find(to_find, pos + replacement.length()))
    {
        target.replace(pos, to_find.length(), replacement);
    }
}

tstring I2String(int64_t i)
{
#if defined(UNICODE)
    return std::to_wstring(i);
#else
    return std::to_string(i);
#endif /* UNICODE */
}

int64_t String2I(const tstring& int_str, int base, int64_t errorValue)
{
    try
    {
        return std::stoll(int_str, nullptr, base);
    }
    catch(...)
    {
        return errorValue;
    }
}

bool StringCmpNoCase(const tstring& str1, const tstring& str2)
{
    return StringToLower(str1) == StringToLower(str2);
}

tstring StringToLower(const tstring& str)
{
    tstring sstr = str;
    for(size_t i=0;i<sstr.length();i++)
    {
#if defined(UNICODE)
        sstr[i] = static_cast<tstring::value_type>(std::towlower(sstr[i]));
#else
        sstr[i] = static_cast<tstring::value_type>(std::tolower(static_cast<unsigned char>(sstr[i])));
#endif
    }
    return sstr;
}

strings_t Tokenize(const tstring& source, const tstring& delimeters)
{
    strings_t tokens;
    tstring::size_type start = 0;
    //skip leading and repeated delimiters, i.e. no empty tokens
    while ((start = source.find_first_not_of(delimeters, start)) != tstring::npos)
    {
        tstring::size_type const end = source.find_first_of(delimeters, start);
        tokens.push_back(source.substr(start, end - start)); //end == npos -> rest of string
        start = end;
    }
    return tokens;
}

tstring KeyToHexString(const unsigned char* key, std::size_t length)
{
    //no iostreams/locale here: libTeamTalk5 is linked with a static libstdc++
    //and numeric stream insertion crashes if the host process (e.g. a JVM)
    //also loads the shared libstdc++ (mismatching locale facet tables)
    auto const hexdigit = [](unsigned int v) -> tstring::value_type {
        return static_cast<tstring::value_type>(v < 10 ? '0' + v : 'a' + (v - 10));
    };
    tstring result;
    result.reserve(length * 2);
    for (std::size_t i = 0; i < length; i++)
    {
        result += hexdigit(key[i] >> 4);
        result += hexdigit(key[i] & 0x0F);
    }
    return result;
}

void HexStringToKey(const tstring& crypt_key, uint8_t* key, std::size_t keylen)
{
    assert(!crypt_key.empty() && crypt_key.length() % 2 == 0);

    int pos = 0;
    for(size_t i=0;i<crypt_key.length() && ((keylen--) != 0);i+=2)
    {
        tstring str = crypt_key.substr(i, 2);
        constexpr auto HEXBASE = 16;
#if defined(UNICODE)
        key[pos] = static_cast<uint8_t>(std::wcstoul(str.c_str(), nullptr, HEXBASE));
#else
        key[pos] = static_cast<uint8_t>(std::strtoul(str.c_str(), nullptr, HEXBASE));
#endif
        pos++;
    }
}

bool VersionSameOrLater(const tstring& check, const tstring& against)
{
    if(check == against) return true;

    strings_t chk_tokens = Tokenize(check, tstring(1, '.'));
    strings_t against_tokens = Tokenize(against, tstring(1, '.'));

    std::vector<int> vec_chk;
    std::vector<int> vec_against;
    for(const auto & chk_token : chk_tokens)
        vec_chk.push_back(static_cast<int>(String2I(chk_token)));
    for(const auto & against_token : against_tokens)
        vec_against.push_back(static_cast<int>(String2I(against_token)));

    std::size_t const less = vec_chk.size() < vec_against.size()?vec_chk.size():vec_against.size();
    
    for(size_t i=0;i<less;i++)
    {
        if(vec_chk[i] < vec_against[i])
            return false;
        if(vec_chk[i] > vec_against[i])
            return true;
    }
    return true;
}

#if defined(UNICODE)
std::string UnicodeToUtf8(const wchar_t* unicode, int len/* = -1*/)
{
    int srclen = len == -1? int(wcslen(unicode)) : len;
    if (srclen <= 0)
        return {};

    std::vector<char> buffer(srclen * 4 + 1, 0);
    int size = WideCharToMultiByte(CP_UTF8, 0, unicode, srclen,
                                   &buffer[0], int(buffer.size()), nullptr, nullptr);
    assert(size >= 0);
    if (size <= 0)
        return {};
    return std::string(&buffer[0], size);
}

std::string UnicodeToUtf8(const tstring& str)
{
    return UnicodeToUtf8(str.c_str());
}

tstring Utf8ToUnicode(const char* utf8, int len/* = -1*/)
{    
    int srclen = len == -1? int(strlen(utf8)) : len;
    if (srclen <= 0)
        return {};

    std::vector<wchar_t> buffer(srclen + 1, 0);
    int size = MultiByteToWideChar(CP_UTF8, 0, utf8, srclen,
                                   &buffer[0], int(buffer.size()));
    assert(size >= 0);
    if (size <= 0)
        return {};
    return tstring(&buffer[0], size);
}

std::string UnicodeToLocal(const tstring& wstr)
{
    std::vector<char> str(std::max(size_t(1), wstr.length() * 4));
    int size = WideCharToMultiByte(GetConsoleCP(), 0, wstr.c_str(), int(wstr.length()),
                                      &str[0], int(str.size()), NULL, NULL);
    assert(size >= 0);
    return &str[0];
}

tstring LocalToUnicode(const std::string& str)
{
    std::vector<wchar_t> wstr(std::max(size_t(1), str.length() * 4));
    std::size_t size = MultiByteToWideChar(GetConsoleCP(), 0, str.c_str(), -1,
                                      &wstr[0], int(wstr.size()));
    assert(size >= 0);
    return &wstr[0];
}

std::string Utf8ToLocal(const char* local, int len/* = -1*/)
{
    tstring cstr = Utf8ToUnicode(local, len);
    return UnicodeToLocal(cstr);
}
#endif

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-pointer-arithmetic)
bool ValidUtf8(const std::string& utf8_str)
{
    //ensure there's no \0 in the string
    if(strlen(utf8_str.c_str()) != utf8_str.length())
        return false;

    //http://stackoverflow.com/questions/1031645/how-to-detect-utf-8-in-plain-c
    const auto * bytes = reinterpret_cast<const unsigned char*>(utf8_str.c_str());
    while (*bytes != 0u)
    {
        if(     (// ASCII
            bytes[0] == 0x09 ||
            bytes[0] == 0x0A ||
            bytes[0] == 0x0D ||
            (0x20 <= bytes[0] && bytes[0] <= 0x7E)
            )
            ) {
                bytes += 1;
                continue;
        }

        if(     (// non-overlong 2-byte
            (0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
            (0x80 <= bytes[1] && bytes[1] <= 0xBF)
            )
            ) {
                bytes += 2;
                continue;
        }

        if(     (// excluding overlongs
            bytes[0] == 0xE0 &&
            (0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            ) ||
            (// straight 3-byte
            ((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
            bytes[0] == 0xEE ||
            bytes[0] == 0xEF) &&
            (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            ) ||
            (// excluding surrogates
            bytes[0] == 0xED &&
            (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            )
            ) {
                bytes += 3;
                continue;
        }

        if(     (// planes 1-3
            bytes[0] == 0xF0 &&
            (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
            (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
            (// planes 4-15
            (0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
            (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
            (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
            (// plane 16
            bytes[0] == 0xF4 &&
            (0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
            (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            )
            ) {
                bytes += 4;
                continue;
        }

        return false;
    }

    return true;
}

std::string LimitUtf8(const std::string& utf8_str, std::size_t maxlen)
{
    if (utf8_str.length() <= maxlen)
        return utf8_str;

    const std::string& trimmed = utf8_str;
    auto length = utf8_str.length();
    while (length > maxlen && length > 0)
    {
        const auto *cp = trimmed.data() + length;
        while (--cp >= trimmed.data() && (((*cp & 0b10000000) != 0) && ((*cp & 0b01000000) == 0))); // NOLINT(bugprone-inc-dec-in-conditions)
        length = cp - trimmed.data();
    }

    if (length != utf8_str.length())
        return trimmed.substr(0, length);

    return trimmed;
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-pointer-arithmetic)

#if defined(UNICODE)
std::wregex BuildRegex(const std::wstring& regexstr)
{
    try
    {
        return std::wregex(regexstr);
    }
    catch (const std::regex_error& )
    {
        return std::wregex();
    }
}
#else
std::regex BuildRegex(const std::string& regexstr)
{
    try
    {
        return std::regex(regexstr);
    }
    catch (const std::regex_error& )
    {
        return {};
    }
}
#endif


uint32_t GETTIMESTAMP()
{
    using namespace std::chrono;
    steady_clock::time_point const NOW = steady_clock::now();
    auto now_ms = time_point_cast<milliseconds>(NOW);
    auto duration = now_ms.time_since_epoch();
    return static_cast<uint32_t>(duration.count());
}

void DUP_TIMESTAMP_DELAY(uint32_t tm)
{
    while(tm == GETTIMESTAMP())
        std::this_thread::yield();
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
#include <cwchar>
#endif

void MYTRACE(const tchar* trace_str, ...) // NOLINT(modernize-avoid-variadic-functions)
{
    va_list args;
    va_start(args, trace_str);
#if defined(__ANDROID_API__) || defined(WIN32) || defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE || defined(TARGET_OS_SIMULATOR) && TARGET_OS_SIMULATOR
    constexpr int MAX_TRACESTRLEN = 512;
#else
    constexpr int MAX_TRACESTRLEN = 0x10000;
#endif
    std::vector<tchar> str_buf(MAX_TRACESTRLEN);
#if defined(UNICODE)
    std::wcsncpy(str_buf.data(), TTEXT("[MYTRACE buffer overflow]\n"), MAX_TRACESTRLEN);
#else
    std::strncpy(str_buf.data(), TTEXT("[MYTRACE buffer overflow]\n"), MAX_TRACESTRLEN);
#endif
    std::array<tchar, MAX_TRACESTRLEN> tmp_str;
    static uint32_t begin = GETTIMESTAMP();
    static uint32_t next;
    next = GETTIMESTAMP();
    int out_len = 0;

    if (MYTRACE_TIMESTAMP)
    {
#if defined(UNICODE)
        std::swprintf(tmp_str.data(), MAX_TRACESTRLEN, TTEXT("%08u: %s"), next - begin, trace_str);
        out_len = std::vswprintf(str_buf.data(), MAX_TRACESTRLEN, tmp_str.data(), args);
#else
        std::snprintf(tmp_str.data(), MAX_TRACESTRLEN, "%08u: %s", next - begin, trace_str);
        out_len = std::vsnprintf(str_buf.data(), MAX_TRACESTRLEN, tmp_str.data(), args);
#endif
    }
    else
    {
#if defined(UNICODE)
        out_len = std::vswprintf(str_buf.data(), MAX_TRACESTRLEN, trace_str, args);
#else
        out_len = std::vsnprintf(str_buf.data(), MAX_TRACESTRLEN, trace_str, args);
#endif
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


Profiler::Profiler(const tchar* name, const tchar* file, int line, bool p_start)
: m_name(name)
, m_filename(file)
, m_line(line)
{
    m_start = std::chrono::steady_clock::now();
    auto const h = std::hash<std::thread::id>{}(std::this_thread::get_id());
    MYTRACE_COND(p_start, TTEXT("Profiler: %s started at %u, %s:%d thr: 0x%llX\n"),
                 m_name, static_cast<uint32_t>(0), m_filename, m_line,
                 static_cast<unsigned long long>(h));
}

Profiler::~Profiler()
{
    auto const tm = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - m_start).count();
    auto const h = std::hash<std::thread::id>{}(std::this_thread::get_id());
    MYTRACE(TTEXT("Profiler: %s completed in %u msec, %s:%d thr: 0x%llX\n"),
            m_name, static_cast<uint32_t>(tm), m_filename, m_line,
            static_cast<unsigned long long>(h));
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

    MYTRACE_COND(m_file.bad(), TTEXT("File read while bad()\n"));
    MYTRACE_COND(m_file.fail(), TTEXT("File read while fail()\n"));
    MYTRACE_COND(!m_file.good(), TTEXT("File read while !good()\n"));

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

    MYTRACE_COND(m_file.bad(), TTEXT("File write while bad()\n"));
    MYTRACE_COND(m_file.fail(), TTEXT("File write while fail()\n"));
    MYTRACE_COND(!m_file.good(), TTEXT("File write while !good()\n"));

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
    MYTRACE_COND(m_file.bad(), TTEXT("File seek while bad()\n"));
    MYTRACE_COND(m_file.fail(), TTEXT("File seek while fail()\n"));
    MYTRACE_COND(!m_file.good(), TTEXT("File seek while !good()\n"));

    if (!m_file.seekg(size, way))
        return false;

    // seekg and seekp are interchangeable for file streams. Weird!

    return !!m_file;
}

int64_t MyFile::Tell()
{
    MYTRACE_COND(m_file.bad(), TTEXT("File tell while bad()\n"));
    MYTRACE_COND(m_file.fail(), TTEXT("File tell while fail()\n"));
    MYTRACE_COND(!m_file.good(), TTEXT("File tell while !good()\n"));

    assert(m_readonly || m_file.tellg() == m_file.tellp());
    return m_file.tellg();
}


