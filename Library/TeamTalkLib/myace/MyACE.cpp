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
#include <ace/OS.h>
#if defined(UNICODE)
#include <ace/UTF16_Encoding_Converter.h>
#include <algorithm>
#endif

#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

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
        ACE_OS::sleep(0);
    }
}

bool MyFile::Open(const ACE_TString& filename, bool readonly /*= true*/)
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

bool MyFile::NewFile(const ACE_TString& filename)
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

bool ExtractFileName(const ACE_TString& filepath, ACE_TString& filename)
{
    bool bResult = false;

    if (!filepath.empty())
    {
        if(filepath.rfind(ACE_DIRECTORY_SEPARATOR_CHAR) != ACE_TString::npos)
        {
            if(!filepath.substr(filepath.rfind(ACE_DIRECTORY_SEPARATOR_CHAR), filepath.length()+1).empty())
            {
                filename = filepath.substr(filepath.rfind(ACE_DIRECTORY_SEPARATOR_CHAR)+1, filepath.length());
                bResult = true;
            }
        }
        else
        {
            filename = filepath;
            bResult = true;
        }
    }
    return bResult;
}

ACE_TString FixFilePath(const ACE_TString& filepath)
{
    ACE_TString tmp = filepath;
    ACE_TString tofind = ACE_DIRECTORY_SEPARATOR_STR;
    tofind += tofind;
    while (tmp.find(tofind) != ACE_TString::npos)
        ReplaceAll(tmp, tofind, ACE_DIRECTORY_SEPARATOR_STR);

    if (!tmp.empty() && tmp[tmp.length()-1] == ACE_DIRECTORY_SEPARATOR_CHAR)
        tmp = tmp.substr(0, tmp.length()-1);
    return tmp;
}

void ReplaceAll(ACE_TString& target, const ACE_TString& to_find, const ACE_TString& replacement )
{
    if (to_find.empty())
        return;

    std::size_t pos1 = 0;
    std::size_t pos2 = target.find(to_find);
    if(pos2 == ACE_TString::npos)
        return;

    ACE_TString tmp;
    do
    {
        tmp += target.substr(pos1, pos2 - pos1);
        tmp += replacement;
        pos1 = pos2 + to_find.length();
    } while((pos2 = target.find(to_find, pos1)) != ACE_TString::npos);

    tmp += target.substr(pos1, target.length() - pos1);

    target = tmp;
}

ACE_TString I2String(ACE_INT64 i)
{
#if defined(UNICODE)
    return std::to_wstring(i).c_str();
#else
    return std::to_string(i).c_str();
#endif /* UNICODE */
}

ACE_INT64 String2I(const ACE_TString& int_str, int base)
{
    try
    {
        return std::stoll(int_str.c_str(), nullptr, base);
    }
    catch(...)
    {
        return 0;
    }
}

bool StringCmpNoCase(const ACE_TString& str1, const ACE_TString& str2)
{
    if(str1.length() != str2.length())
        return false;

    for(size_t i=0;i<str1.length();i++)
        if(ACE_OS::ace_tolower(str1[i]) != ACE_OS::ace_tolower(str2[i]))
            return false;

    return true;
}

ACE_TString StringToLower(const ACE_TString& str)
{
    ACE_TString sstr = str;
    for(size_t i=0;i<sstr.length();i++)
        sstr[i] = ACE_OS::ace_tolower(sstr[i]);
    return sstr;
}

#if defined(_DEBUG)

#define MYTRACE_TIMESTAMP 1

#if defined(__ANDROID_API__)
#include <android/log.h>
#endif

#if defined(__APPLE__)
#include <os/log.h>
#endif

#if defined(WIN32)
#include <windows.h>
#endif

void MYTRACE(const ACE_TCHAR* trace_str, ...)
{
    va_list args;
    va_start(args, trace_str);
#if defined(__ANDROID_API__) || defined(WIN32) || defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE || defined(TARGET_OS_SIMULATOR) && TARGET_OS_SIMULATOR
    const int MAX_TRACESTRLEN = 512;
#else
    const int MAX_TRACESTRLEN = 0x10000;
#endif
    std::vector<ACE_TCHAR> str_buf(MAX_TRACESTRLEN);
    ACE_OS::strncpy(str_buf.data(), ACE_TEXT("[MYTRACE buffer overflow]\n"), MAX_TRACESTRLEN);
    ACE_TCHAR tmp_str[MAX_TRACESTRLEN] = ACE_TEXT("");
    static ACE_UINT32 begin = GETTIMESTAMP();
    static ACE_UINT32 next;
    next = GETTIMESTAMP();
    int out_len = 0;

#if (MYTRACE_TIMESTAMP)
    ACE_OS::snprintf(tmp_str, MAX_TRACESTRLEN, ACE_TEXT("%08u: %s"), next - begin, trace_str);
    out_len = ACE_OS::vsnprintf(str_buf.data(), MAX_TRACESTRLEN, tmp_str, args);
#else
    out_len = ACE_OS::vsnprintf(&str_buf[0], MAX_TRACESTRLEN, trace_str, args);
#endif
    
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

bool VersionSameOrLater(const ACE_TString& check, const ACE_TString& against)
{
    if(check == against) return true;

    strings_t chk_tokens = Tokenize(check, ACE_TEXT("."));
    strings_t against_tokens = Tokenize(against, ACE_TEXT("."));

    std::vector<int> vec_chk;
    std::vector<int> vec_against;
    for(const auto & chk_token : chk_tokens)
        vec_chk.push_back(ACE_OS::atoi(chk_token.c_str()));
    for(const auto & against_token : against_tokens)
        vec_against.push_back(ACE_OS::atoi(against_token.c_str()));

    std::size_t const less = vec_chk.size() < vec_against.size()?vec_chk.size():vec_against.size();
    
    for(size_t i=0;i<less;i++)
        if(vec_chk[i] < vec_against[i])
            return false;
        else if(vec_chk[i] > vec_against[i])
            return true;

    return true;
}

ACE_TString KeyToHexString(const unsigned char* key, int length)
{
    ACE_TCHAR buf[3];
    int const LEN = (2*length)+1;
    auto* str = new ACE_TCHAR[LEN];
    str[LEN-1] = '\0';
    for(int i=0;i<length;i++)
    {
        unsigned int const x = key[i];
        ACE_OS::sprintf(buf, ACE_TEXT("%.2x"), x);
        ACE_OS::sprintf(str+(i*2), ACE_TEXT("%s"), buf);
    }
    ACE_TString const s = str;
    delete [] str;
    return s;
}

void HexStringToKey(const ACE_TString& crypt_key, uint8_t* key, int keylen)
{
    assert(!crypt_key.empty() && crypt_key.length() % 2 == 0);

    int pos = 0;
    ACE_TString str;
    for(size_t i=0;i<crypt_key.length() && keylen--;i+=2)
    {
        str = crypt_key.substr(i, 2);
        key[pos] = (uint8_t)ACE_OS::strtol(str.c_str(), nullptr, 16);
        pos++;
    }
}

ACE_TString UptimeHours(const ACE_Time_Value& value)
{
    time_t const nHour = value.sec()/3600;
    time_t const nMinutes = (value.sec()%3600) / 60;
    time_t const nSec = (value.sec()%60);
    ACE_TCHAR buf[512];
    ACE_OS::snprintf(buf, 512, ACE_TEXT("%d:%.2d:%.2d"), (int)nHour, (int)nMinutes, (int)nSec);
    return buf;
}

ACE_Time_Value ToTimeValue(int msec)
{
    return ACE_Time_Value(msec / 1000, (msec % 1000) * 1000);
}

strings_t Tokenize(const ACE_TString& source, const ACE_TString& delimeters)
{ 
    std::vector<ACE_TString> tokens;

    std::size_t i = 0;
    std::size_t tokenstart = 0;
    while(i<source.length())
    {
        for(size_t j=0;j<delimeters.length();j++)
        {
            if(source[i] == delimeters[j])
            {
                if(i > tokenstart && source[tokenstart] != delimeters[j])
                    tokens.push_back(source.substr(tokenstart, i - tokenstart));
                tokenstart = i + 1;
                break;
            }
        }
        i++;
    }

    if(i>tokenstart)
        tokens.push_back(source.substr(tokenstart, i - tokenstart));

    return tokens; 
}

#if defined(UNICODE)
ACE_CString UnicodeToUtf8(const wchar_t* unicode, int len/* = -1*/)
{
    ACE_UTF16_Encoding_Converter conv;
    ACE_UTF16_Encoding_Converter::Result result;
    int length = len == -1? (int)wcslen(unicode) * 4 + 1 : len * 4 + 1;
    if (length <= 0)
        return ACE_CString();

    std::vector<ACE_Byte> buffer(length, 0);
    result = conv.to_utf8(unicode, len == -1? 
            wcslen(unicode)*sizeof(wchar_t) + sizeof(wchar_t) : len * sizeof(wchar_t), &buffer[0], length);
#if defined(_DEBUG)
    if(result != ACE_UTF16_Encoding_Converter::CONVERSION_OK)
        MYTRACE(ACE_TEXT("Unicode->UTF8 conversion failed\n"));
#endif
    ACE_CString utf8((const char*)&buffer[0]);
    return utf8;
}

ACE_CString UnicodeToUtf8(const ACE_WString& str)
{
    return UnicodeToUtf8(str.c_str());
}

ACE_WString Utf8ToUnicode(const char* utf8, int len/* = -1*/)
{    
    ACE_UTF16_Encoding_Converter conv;
    ACE_UTF16_Encoding_Converter::Result result;
    int length = len == -1? (int)strlen(utf8) + 1 : len + 1;
    if(length<=0)
        return ACE_WString();

    std::vector<wchar_t> buffer(length, 0);
    result = conv.from_utf8((const ACE_Byte*)utf8, len == -1?
            strlen(utf8) + 1 : len, &buffer[0], length);
#if defined(_DEBUG)
    if(result != ACE_UTF16_Encoding_Converter::CONVERSION_OK)
        MYTRACE(ACE_TEXT("UTF8->Unicode conversion failed\n"));
#endif
    ACE_WString unicode(&buffer[0]);
    return unicode;
}

ACE_CString UnicodeToLocal(const ACE_WString& wstr)
{
    std::vector<char> str(std::max(size_t(1), wstr.length() * 4));
    int size = WideCharToMultiByte(GetConsoleCP(), 0, wstr.c_str(), int(wstr.length()),
                                      &str[0], int(str.size()), NULL, NULL);
    assert(size >= 0);
    return &str[0];
}

ACE_WString LocalToUnicode(const ACE_CString& str)
{
    std::vector<wchar_t> wstr(std::max(size_t(1), str.length() * 4));
    std::size_t size = MultiByteToWideChar(GetConsoleCP(), 0, str.c_str(), -1,
                                      &wstr[0], int(wstr.size()));
    assert(size >= 0);
    return &wstr[0];
}

ACE_CString LocalToUtf8(const char* local, int len/* = -1*/)
{
    ACE_WString unicode = LocalToUnicode(local);
    return UnicodeToUtf8(unicode.c_str());
}

ACE_CString Utf8ToLocal(const char* local, int len/* = -1*/)
{
    ACE_WString cstr = Utf8ToUnicode(local, len);
    return UnicodeToLocal(cstr.c_str());
}
/*
bool ValidUtf8(const ACE_CString& utf8_str)
{
    ACE_UTF16_Encoding_Converter conv;
    ACE_UTF16_Encoding_Converter::Result result;
    if(utf8_str.length() == 0)
        return true;
    //ensure there's no \0 in the std::string
    if(strlen(utf8_str.c_str()) != utf8_str.length())
        return false;

    std::vector<ACE_Byte> buffer((utf8_str.length()+1)*2, 0);
    result = conv.from_utf8((const ACE_Byte*)utf8_str.c_str(), 
                            utf8_str.length()+1, 
                            &buffer[0], buffer.size());
    return result == ACE_UTF16_Encoding_Converter::CONVERSION_OK;
}
*/
#endif

bool ValidUtf8(const ACE_CString& utf8_str)
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

ACE_CString LimitUtf8(const ACE_CString& utf8_str, std::size_t maxlen)
{
    if (utf8_str.length() <= maxlen)
        return utf8_str;

    const ACE_CString& trimmed = utf8_str;
    auto length = utf8_str.length();
    while (length > maxlen && length > 0)
    {
        const auto *cp = trimmed.fast_rep() + length;
        while (--cp >= trimmed.fast_rep() && (((*cp & 0b10000000) != 0) && ((*cp & 0b01000000) == 0)));
        length = cp - trimmed.fast_rep();
    }

    if (length != utf8_str.length())
        return trimmed.substr(0, length);

    return trimmed;
}

Profiler::Profiler(const ACE_TCHAR* name, const ACE_TCHAR* file, int line, 
                   bool p_start)
: m_name(name)
, m_filename(file)
, m_line(line)
{
    m_start = ACE_OS::gettimeofday();
    auto const h = (ACE_UINT64)ACE_OS::thr_self();
    MYTRACE_COND(p_start, ACE_TEXT("Profiler: %s started at %u, %s:%d thr: 0x%X\n"),
                 m_name, (ACE_UINT32)0, m_filename, m_line, (unsigned)h);
}

Profiler::~Profiler()
{
    ACE_Time_Value const tm = ACE_OS::gettimeofday() - m_start;
    auto const h = (ACE_UINT64)ACE_OS::thr_self();
    MYTRACE(ACE_TEXT("Profiler: %s completed in %u msec, %s:%d thr: 0x%X\n"),
            m_name, (ACE_UINT32)tm.msec(), m_filename, m_line, (unsigned)h);
}
