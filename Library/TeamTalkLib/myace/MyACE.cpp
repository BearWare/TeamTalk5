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
#include <ace/ACE.h>
#include <ace/OS.h>
#include <ace/UTF16_Encoding_Converter.h>
#include <ace/OS_NS_ctype.h>
#include <ace/Version.h>

#include <ace/INet/HTTP_URL.h>
#include <ace/INet/HTTP_ClientRequestHandler.h>

#if defined(ENABLE_ENCRYPTION)
#include <ace/INet/HTTPS_URL.h>
#include <ace/INet/HTTPS_SessionFactory.h>
#endif

#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <assert.h>

using namespace std;

/*******************************************************************/
/************************* Helper functions ************************/
/*******************************************************************/
ACE_THR_FUNC_RETURN event_loop (void *arg)
{
    ACE_Reactor *reactor = static_cast<ACE_Reactor *> (arg);
    assert(reactor);

    reactor->owner (ACE_OS::thr_self ());
    reactor->run_reactor_event_loop ();
    return 0;
}

void SyncReactor(ACE_Reactor& reactor)
{
    ACE_thread_t tid;
    while(reactor.owner(&tid) < 0 || tid == ACE_OS::thr_self())
    {
        ACE_OS::sleep(0);
    }
}

bool ExtractFileName(const ACE_TString& filepath, ACE_TString& filename)
{
    bool bResult = false;

    if(filepath.length())
    {
        if(filepath.rfind(ACE_DIRECTORY_SEPARATOR_CHAR) != ACE_TString::npos)
        {
            if(filepath.substr(filepath.rfind(ACE_DIRECTORY_SEPARATOR_CHAR), filepath.length()+1).length())
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
    while(tmp.find(tofind) != ACE_TString::npos)
        replace_all(tmp, tofind, ACE_DIRECTORY_SEPARATOR_STR);
    if(tmp.length() && tmp[tmp.length()-1] == ACE_DIRECTORY_SEPARATOR_CHAR)
        tmp = tmp.substr(0, tmp.length()-1);
    return tmp;
}

void replace_all(ACE_TString& target, const ACE_TString& to_find, const ACE_TString& replacement )
{
    if (to_find.length() == 0)
        return;

    size_t pos1 = 0, pos2 = target.find(to_find);
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

ACE_TString i2string(ACE_INT64 i)
{
#if defined(__ANDROID_API__)
    std::ostringstream os ;
    os << i;
    return os.str().c_str();
#else
    
#if defined(UNICODE)
    return std::to_wstring(i).c_str();
#else
    return std::to_string(i).c_str();
#endif /* UNICODE */
    
#endif
}

ACE_INT64 string2i(const ACE_TString& int_str, int base)
{
#if defined(__ANDROID_API__)
    ACE_INT64 ret = 0;
    istringstream is(int_str.c_str());
    is >> std::setbase(base);
    is >> ret;
    return ret;
#else
    try
    {
        return std::stoll(int_str.c_str(), 0, base);
    }
    catch(...)
    {
        return 0;
    }
#endif
}

bool stringcmpnocase(const ACE_TString& str1, const ACE_TString& str2)
{
    if(str1.length() != str2.length())
        return false;

    for(size_t i=0;i<str1.length();i++)
        if(ACE_OS::ace_tolower(str1[i]) != ACE_OS::ace_tolower(str2[i]))
            return false;

    return true;
}

ACE_TString stringtolower(const ACE_TString& str)
{
    ACE_TString sstr = str;
    for(size_t i=0;i<sstr.length();i++)
        sstr[i] = ACE_OS::ace_tolower(sstr[i]);
    return sstr;
}

#if defined(_DEBUG)

#if defined(WIN32)

#include <windows.h>

#define MYTRACE_TIMESTAMP 1

void MYTRACE(const ACE_TCHAR* trace_str, ...)
{
    va_list args;
    va_start(args, trace_str);
    static ACE_UINT32 begin = GETTIMESTAMP(), next;
    next = GETTIMESTAMP();
    ACE_TCHAR str_buf[512] = ACE_TEXT(""), tmp_str[512] = ACE_TEXT("");

#if (MYTRACE_TIMESTAMP)
    ACE_OS::snprintf(tmp_str, 512, ACE_TEXT("%08u: %s"), next - begin, trace_str);
    int nBuf = ACE_OS::vsnprintf(str_buf, 512, tmp_str, args);
#else
    int nBuf = ACE_OS::vsnprintf(str_buf, 512, trace_str, args);
#endif

    if (nBuf > -1)
        OutputDebugString(str_buf);
    else
        OutputDebugString( ACE_TEXT("MYTRACE buffer overflow\n") );

    va_end(args);
}

#elif defined(__ANDROID_API__)

#include <android/log.h>

void MYTRACE(const ACE_TCHAR* trace_str, ...)
{
    va_list args;
    va_start(args, trace_str);
    ACE_TCHAR str_buf[1024];
    int nBuf = ACE_OS::vsnprintf(str_buf, 1024, trace_str, args);

    if (nBuf > -1)
        __android_log_write(ANDROID_LOG_INFO, "bearware", str_buf);
    else
        __android_log_write(ANDROID_LOG_INFO, "bearware",
                            ACE_TEXT("MYTRACE buffer overflow\n"));
    va_end(args);
}

#endif /* WIN32 */

#endif /* _DEBUG */

bool VersionSameOrLater(const ACE_TString& check, const ACE_TString& against)
{
    if(check == against) return true;

    strings_t chk_tokens = tokenize(check, ACE_TEXT("."));
    strings_t against_tokens = tokenize(against, ACE_TEXT("."));

    vector<int> vec_chk, vec_against;
    for(size_t i=0;i<chk_tokens.size();i++)
        vec_chk.push_back(ACE_OS::atoi(chk_tokens[i].c_str()));
    for(size_t i=0;i<against_tokens.size();i++)
        vec_against.push_back(ACE_OS::atoi(against_tokens[i].c_str()));

    size_t less = vec_chk.size() < vec_against.size()?vec_chk.size():vec_against.size();
    
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
    int LEN = 2*length+1;
    ACE_TCHAR* str = new ACE_TCHAR[LEN];
    str[LEN-1] = '\0';
    for(int i=0;i<length;i++)
    {
        unsigned int x = key[i];
        ACE_OS::sprintf(buf, ACE_TEXT("%.2x"), x);
        ACE_OS::sprintf(str+i*2, ACE_TEXT("%s"), buf);
    }
    ACE_TString s = str;
    delete [] str;
    return s;
}

void HexStringToKey(const ACE_TString& crypt_key, unsigned char* key)
{
    assert(crypt_key.length() && crypt_key.length() % 2 == 0);

    int pos = 0;
    ACE_TString str;
    for(size_t i=0;i<crypt_key.length();i+=2)
    {
        str = crypt_key.substr(i, 2);
        key[pos] = (unsigned char)ACE_OS::strtol(str.c_str(), (ACE_TCHAR **)NULL, 16);
        pos++;
    }
}

ACE_TString UptimeHours(const ACE_Time_Value& value)
{
    time_t nHour = value.sec()/3600;
    time_t nMinutes = (value.sec()%3600) / 60;
    time_t nSec = (value.sec()%60);
    ACE_TCHAR buf[512];
    ACE_OS::snprintf(buf, 512, ACE_TEXT("%d:%.2d:%.2d"), (int)nHour, (int)nMinutes, (int)nSec);
    return buf;
}

ACE_Time_Value ToTimeValue(int msec)
{
    return ACE_Time_Value(msec / 1000, (msec % 1000) * 1000);
}

strings_t tokenize(const ACE_TString& source, const ACE_TString& delimeters) 
{ 
    vector<ACE_TString> tokens; 

    size_t i = 0, tokenstart = 0;
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
    if(length<=0)
        return ACE_CString();

    vector<ACE_Byte> buffer(length, 0);
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

    vector<wchar_t> buffer(length, 0);
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
    size_t size = MultiByteToWideChar(GetConsoleCP(), 0, str.c_str(), -1,
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
    //ensure there's no \0 in the string
    if(strlen(utf8_str.c_str()) != utf8_str.length())
        return false;

    vector<ACE_Byte> buffer((utf8_str.length()+1)*2, 0);
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
    const unsigned char * bytes = reinterpret_cast<const unsigned char*>(utf8_str.c_str());
    while(*bytes)
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


Profiler::Profiler(const ACE_TCHAR* name, const ACE_TCHAR* file, int line, 
                   bool p_start)
: m_name(name)
, m_filename(file)
, m_line(line)
{
    m_start = ACE_OS::gettimeofday();
    ACE_UINT64 h = (ACE_UINT64)ACE_OS::thr_self();
    MYTRACE_COND(p_start, ACE_TEXT("Profiler: %s started at %u, %s:%d thr: 0x%X\n"),
                 m_name, (ACE_UINT32)0, m_filename, m_line, (unsigned)h);
}

Profiler::~Profiler()
{
    ACE_Time_Value tm = ACE_OS::gettimeofday() - m_start;
    ACE_UINT64 h = (ACE_UINT64)ACE_OS::thr_self();
    MYTRACE(ACE_TEXT("Profiler: %s completed in %u msec, %s:%d thr: 0x%X\n"),
            m_name, (ACE_UINT32)tm.msec(), m_filename, m_line, (unsigned)h);
}

std::vector<ACE_INET_Addr> DetermineHostAddress(const ACE_TString& host, int port)
{
    std::vector<ACE_INET_Addr> result;
    
#if ACE_MAJOR_VERSION < 6 || (ACE_MAJOR_VERSION == 6 && ACE_MINOR_VERSION < 4)
    result.resize(1);

    int address_family = AF_INET;
    result[0] = ACE_INET_Addr(port, host.c_str(), address_family);
    if (result[0].is_any())
    {
        address_family = AF_INET6;
        result[0] = ACE_INET_Addr(port, host.c_str(), address_family);
    }
    
#else
    bool encode = true;
    addrinfo hints;
    ACE_OS::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    // The ai_flags used to contain AI_ADDRCONFIG as well but that prevented
    // lookups from completing if there is no, or only a loopback, IPv6
    // interface configured. See Bugzilla 4211 for more info.

    hints.ai_flags = AI_V4MAPPED;
#if defined(ACE_HAS_IPV6) && defined(AI_ALL)
    // Without AI_ALL, Windows machines exhibit inconsistent behaviors on
    // difference machines we have tested.
    hints.ai_flags |= AI_ALL;
#endif

    // Note - specify the socktype here to avoid getting multiple entries
    // returned with the same address for different socket types or
    // protocols. If this causes a problem for some reason (an address that's
    // available for TCP but not UDP, or vice-versa) this will need to change
    // back to unrestricted hints and weed out the duplicate addresses by
    // searching this->inet_addrs_ which would slow things down.
    hints.ai_socktype = SOCK_STREAM;

    addrinfo *res = 0;

    const int error = ACE_OS::getaddrinfo(UnicodeToUtf8(host).c_str(), 0, &hints, &res);

    if(error)
    {
        errno = error;
        return std::vector<ACE_INET_Addr>();
    }


    for(addrinfo *curr = res; curr; curr = curr->ai_next)
    {
        union ip46
        {
            sockaddr_in  in4_;
#if defined (ACE_HAS_IPV6)
            sockaddr_in6 in6_;
#endif /* ACE_HAS_IPV6 */
        };

        ip46 addr;
        ACE_OS::memcpy(&addr, curr->ai_addr, curr->ai_addrlen);
#ifdef ACE_HAS_IPV6
        if(curr->ai_family == AF_INET6)
        {
            addr.in6_.sin6_port = encode ? ACE_NTOHS(port) : port;
            result.push_back(ACE_INET_Addr(reinterpret_cast<const sockaddr_in*>(&addr.in6_), sizeof(addr.in6_)));
        }
        else
#endif
        {
            addr.in4_.sin_port = encode ? ACE_NTOHS(port) : port;
            result.push_back(ACE_INET_Addr(reinterpret_cast<const sockaddr_in*>(&addr.in4_), sizeof(addr.in4_)));
        }
    }

    ACE_OS::freeaddrinfo(res);

#endif /* ACE_MAJOR_VERSION */
    
    return result;
}

int HttpRequest(const ACE_CString& url, std::string& doc)
{
#if defined(ENABLE_ENCRYPTION)
#if defined(ENABLE_TEAMTALKACE)
    // Enable SNI enabled HTTPS sessions
    ACE::HTTPS::SessionFactory_Impl::registerHTTPS();
#else
    // HTTPS session factory is not instantiated unless specified explicitly
    ACE_Singleton<ACE::HTTPS::SessionFactory_Impl, ACE_SYNCH::NULL_MUTEX>::instance();
#endif /* ENABLE_TEAMTALKACE */
#endif /* ENABLE_ENCRYPTION */

    ACE_Auto_Ptr<ACE::INet::URL_Base> url_safe(ACE::INet::URL_Base::create_from_string(url));
    if(url_safe.get() == 0)
        return -1;

    ACE::HTTP::ClientRequestHandler http;
    ACE::INet::URLStream urlin = url_safe.get()->open(http);

    ostringstream oss;
    oss << urlin->rdbuf();
    doc = oss.str();

    ACE::HTTP::Status status = http.response().get_status();
#if defined(UNICODE)
    MYTRACE_COND(!status.is_ok(), ACE_TEXT("HTTP request failed:\n%s\n"),
                 Utf8ToUnicode(doc.c_str()).c_str());
#else
    MYTRACE_COND(!status.is_ok(), ACE_TEXT("HTTP request failed:\n%s\n"), doc.c_str());
#endif
    
    return status.is_ok() ? 1 : 0;
}
