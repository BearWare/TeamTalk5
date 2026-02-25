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

#include "Trial.h"
#include "TeamTalkDefs.h"
#include "myace/MyINet.h"

#include <ace/INet/HTTP_ClientRequestHandler.h>
#include <ace/INet/HTTP_Status.h>
#include <ace/INet/HTTP_URL.h>
#include <ace/Thread_Manager.h>

#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>

#if defined(WIN32)
#include <tchar.h>
#endif

ACE_TString g_lpszRegName;
ACE_TString g_lpszRegKey;
bool g_LicenseValid = true;

void LicenseCheck()
{
    static std::atomic<uint64_t> checks{0};

    if (++checks == 1)
    {
        if (g_lpszRegName.empty() || g_lpszRegKey.empty())
        {
            std::cerr << "----------------------------------------------------" << std::endl;
            std::cerr << "TeamTalk 5 SDK. Copyright (c) 2005-2026, BearWare.dk" << std::endl;
            std::cerr << "----------------------------------------------------" << std::endl;
            std::cerr << "TeamTalk 5 DLL running in TRAIL MODE." << std::endl;
            std::cerr << "Check out the \"License TeamTalk 5 SDK\" section in the SDK's documentation" << std::endl;
            std::cerr << "for licensing information!" << std::endl;
        }
        ValidTeamTalkSDK();
    }
}

#if defined(WIN32)
ACE_TString GetProcessName()
{
    ACE_TCHAR buff[MAX_PATH] = ACE_TEXT(""), *p;
    GetModuleFileName(NULL, buff, MAX_PATH);

    p = ACE_OS::strrchr(buff, ACE_DIRECTORY_SEPARATOR_CHAR);
    if(p)
        return ++p;

    return buff;
}
#elif defined(ACE_ANDROID)
extern char *__progname;
ACE_TString GetProcessName()
{
    if(__progname)
    {
        ACE_TCHAR* p = ACE_OS::strrchr(__progname,
                                       ACE_DIRECTORY_SEPARATOR_CHAR);
        if(p)
            return ++p;
        return __progname;
    }
    return ACE_TString();
}
#elif !defined(__APPLE__)
ACE_TString GetProcessName()
{
    if(program_invocation_short_name)
    {
        ACE_TCHAR* p = ACE_OS::strrchr(program_invocation_short_name,
                                       ACE_DIRECTORY_SEPARATOR_CHAR);
        if(p)
            return ++p;
        return program_invocation_short_name;
    }
    return ACE_TString();
}
#elif defined(__APPLE__)
/* process name found in TrialObjC.mm */
#else
#error "Cannot get process name"
#endif

using http_prop_t = std::map<ACE_CString, ACE_CString>;

ACE_THR_FUNC_RETURN perform_check(void *arg)
{
    ACE_CString url = TEAMTALK_SDK_URL;

    http_prop_t http_prop;

#if defined(ACE_WIN32)
    http_prop["os"] =  (sizeof(void*) == sizeof(uint64_t)? "win64" : "win32");
#elif defined(ACE_HAS_IPHONE)
    http_prop["os"] =  (sizeof(void*) == sizeof(uint64_t)? "ios64" : "ios32");
#elif defined(__APPLE__)
    http_prop["os"] =  (sizeof(void*) == sizeof(uint64_t)? "mac64" : "mac32");
#elif defined(ACE_ANDROID)
    http_prop["os"] =  (sizeof(void*) == sizeof(uint64_t)? "android64" : "android32");
#else
    http_prop["os"] =  (sizeof(void*) == sizeof(uint64_t)? "linux64" : "linux32");
#endif
    http_prop["client"] = TEAMTALK_LIB_NAME;
    http_prop["version"] = TEAMTALK_VERSION;

    ACE_CString process = UnicodeToUtf8(GetProcessName().c_str());
    http_prop["program"] = URLEncode(process.c_str()).c_str();
    if (!g_lpszRegName.empty())
    {
        ACE_CString regname = UnicodeToUtf8(g_lpszRegName).c_str();
        http_prop["regname"] = URLEncode(regname.c_str()).c_str();
    }
    if (!g_lpszRegKey.empty())
    {
        ACE_CString regkey = UnicodeToUtf8(g_lpszRegKey).c_str();
        http_prop["regkey"] = URLEncode(regkey.c_str()).c_str();
    }

    for(http_prop_t::const_iterator i=http_prop.begin();i!=http_prop.end();)
    {
        url += i->first;
        url += "=";
        url += i->second;
        i++;
        if(i != http_prop.end())
            url += "&";
    }

    ACE::HTTP::Status::Code statusCode;
    std::string response;
    if (HttpGetRequest(url, response, &statusCode) >= 0)
    {
        switch (statusCode)
        {
        case ACE::HTTP::Status::HTTP_PAYMENT_REQUIRED:
            g_LicenseValid = false;
            break;
        default:
            break;
        }
    }

    return {};
}

void ValidTeamTalkSDK()
{
    ACE_Thread_Manager::instance ()->spawn(perform_check, nullptr);
}
