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

#include "MyINet.h"
#include "MyACE.h"

#include <ace/OS.h>

#include <ace/INet/HTTP_URL.h>
#include <ace/INet/HTTP_ClientRequestHandler.h>

#if defined(ENABLE_ENCRYPTION)
#include <ace/INet/HTTPS_URL.h>
#include <ace/INet/HTTPS_SessionFactory.h>
#endif

#include <iomanip>
#include <sstream>

using namespace std;

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

    addrinfo* res = 0;

    const int error = ACE_OS::getaddrinfo(UnicodeToUtf8(host).c_str(), 0, &hints, &res);

    if (error)
    {
        errno = error;
        return std::vector<ACE_INET_Addr>();
    }


    for (addrinfo* curr = res; curr; curr = curr->ai_next)
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
        if (curr->ai_family == AF_INET6)
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

int HttpGetRequest(const ACE_CString& url, std::string& doc)
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
    if (url_safe.get() == 0)
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

std::string URLEncode(const std::string& utf8)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (std::string::const_iterator i = utf8.begin(), n = utf8.end(); i != n; ++i)
    {
        std::string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char)c);
        escaped << nouppercase;
    }

    return escaped.str();
}
