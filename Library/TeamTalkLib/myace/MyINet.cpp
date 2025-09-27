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

#include <ace/INet/HTTP_ClientRequestHandler.h>
#include <ace/INet/HTTP_URL.h>
#include <ace/OS.h>
#include <ace/ace_wchar.h>

#if defined(ENABLE_ENCRYPTION)
#include <ace/INet/HTTPS_SessionFactory.h>
#include <ace/INet/HTTPS_URL.h>
#endif

#if !defined(WIN32)
#include <arpa/inet.h>
#endif
#include <assert.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <memory>

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

void aceSingletons()
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
}

int HttpGetRequest(const ACE_CString& url, std::string& result, ACE::HTTP::Status::Code* statusCode /*= nullptr*/)
{
    aceSingletons();

    std::unique_ptr<ACE::INet::URL_Base> url_safe(ACE::INet::URL_Base::create_from_string(url));
    if (!url_safe)
        return -1;

    ACE::HTTP::ClientRequestHandler http;
    ACE::INet::URLStream urlin = url_safe->open(http);

    ostringstream oss;
    oss << urlin->rdbuf();
    result = oss.str();

    ACE::HTTP::Status httpCode = http.response().get_status();
#if defined(UNICODE)
    MYTRACE_COND(!status.is_ok(), ACE_TEXT("HTTP request failed:\n%s\n"),
        Utf8ToUnicode(result.c_str()).c_str());
#else
    MYTRACE_COND(!httpCode.is_ok(), ACE_TEXT("HTTP request failed:\n%s\n"), result.c_str());
#endif
    if (statusCode)
        *statusCode = httpCode.get_status();

    if (!httpCode.is_valid() || httpCode.get_status() == ACE::HTTP::Status::HTTP_NONE ||
        httpCode.get_status() >= ACE::HTTP::Status::HTTP_INTERNAL_SERVER_ERROR)
        return -1;

    return httpCode.is_ok() ? 1 : 0;
}

int HttpPostRequest(const ACE_CString& url, const char* data, int len,
                    const std::map<std::string, std::string>& headers,
                    std::string& result, ACE::HTTP::Status::Code* statusCode /*= nullptr*/)
{
    aceSingletons();

    std::unique_ptr<ACE::INet::URL_Base> url_safe(ACE::INet::URL_Base::create_from_string(url));
    if (!url_safe)
        return -1;

    class MyRequest : public ACE::HTTP::ClientRequestHandler
    {
        const std::map<std::string, std::string>& m_headers;
        const char* m_content;
        int m_contentlen;
    public:
        MyRequest(const char* content, int contentlen, const std::map<std::string, std::string>& headers) : m_content(content), m_contentlen(contentlen), m_headers(headers)
        {
            request().set_method(ACE::HTTP::Request::HTTP_POST);
        }

    protected:

        // 10% copy-paste from ClientRequestHandler::handle_open_request()
        std::istream& handle_open_request(const ACE::INet::URL_Base& url)
        {
            if (request().get_method() == ACE::HTTP::Request::HTTP_GET)
            {
                return ACE::HTTP::ClientRequestHandler::handle_open_request(url);
            }
            else if (request().get_method() == ACE::HTTP::Request::HTTP_POST)
            {
                const ACE::HTTP::URL& http_url = dynamic_cast<const ACE::HTTP::URL&> (url);
                return handle_post_request(http_url);
            }
            else
            {
                return ACE::HTTP::ClientRequestHandler::handle_open_request(url);
            }
        }

        // 90% copy-paste from ClientRequestHandler::handle_get_request()
        std::istream& handle_post_request(const ACE::HTTP::URL& http_url)
        {
            bool connected = false;
            if (http_url.has_proxy())
                connected = this->initialize_connection(http_url.get_scheme(),
                    http_url.get_host(),
                    http_url.get_port(),
                    true,
                    http_url.get_proxy_host(),
                    http_url.get_proxy_port());
            else
                connected = this->initialize_connection(http_url.get_scheme(),
                    http_url.get_host(),
                    http_url.get_port());

            if (connected)
            {
                request().reset(request().get_method(), http_url.get_request_uri(), request().get_version());

                this->initialize_request(http_url, this->request());

                for (auto v : m_headers)
                {
                    request().set(v.first.c_str(), v.second.c_str());
                }

                auto& os = this->session()->send_request(this->request());
                if (os)
                {
                    if (m_contentlen > 0)
                    {
                        request().set_content_length(m_contentlen);
                        os << m_content;
                    }

                    if (this->session()->receive_response(this->response()))
                        return this->response_stream();
                }

                this->close_connection();
                this->handle_request_error(http_url);
            }
            else
            {
                this->handle_connection_error(http_url);
            }

            return this->response_stream();
        }

    } http(data, len, headers);

    ACE::INet::URLStream urlin = url_safe->open(http);
    ostringstream oss;
    oss << urlin->rdbuf();
    result = oss.str();
    auto httpCode = http.response().get_status();
    if (statusCode)
        *statusCode = httpCode.get_status();

    return httpCode.is_ok() ? 1 : 0;
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

ACE_TString InetAddrToString(const ACE_INET_Addr& addr)
{
    ACE_TCHAR buf[INET6_ADDRSTRLEN+1] = {};
    addr.addr_to_string(buf, INET6_ADDRSTRLEN);
    return buf;
}

int InetAddrFamily(const ACE_TString& addr_str)
{
    char buf[INET6_ADDRSTRLEN];
    if (ACE_OS::inet_pton(AF_INET, UnicodeToUtf8(addr_str).c_str(), buf) > 0)
        return AF_INET;
    if (ACE_OS::inet_pton(AF_INET6, UnicodeToUtf8(addr_str).c_str(), buf) > 0)
        return AF_INET6;
    return -1;
}

ACE_TString INetAddrNetwork(const ACE_TString& ipaddr, uint32_t prefix)
{
    int af = InetAddrFamily(ipaddr);
    switch (af)
    {
    case AF_INET :
    {
        struct sockaddr_in ipv4addr;
        if (inet_pton(AF_INET, UnicodeToUtf8(ipaddr).c_str(), &(ipv4addr.sin_addr)) <= 0)
            return ACE_TString();
        prefix = std::min(prefix, uint32_t(32));
        uint32_t shift = 32-prefix;
        ipv4addr.sin_addr.s_addr = ntohl(ipv4addr.sin_addr.s_addr);
        ipv4addr.sin_addr.s_addr >>= shift;
        ipv4addr.sin_addr.s_addr <<= shift;
        ipv4addr.sin_addr.s_addr = ntohl(ipv4addr.sin_addr.s_addr);
        char strnet[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &(ipv4addr.sin_addr.s_addr), strnet, INET_ADDRSTRLEN))
            return Utf8ToUnicode(strnet);
    }
    case AF_INET6 :
    {
        struct sockaddr_in6 ipv6addr;
        if (inet_pton(AF_INET6, UnicodeToUtf8(ipaddr).c_str(), &(ipv6addr.sin6_addr)) <= 0)
            return ACE_TString();
        prefix = std::min(prefix, uint32_t(128));
        for (int i = 0; i < 16; ++i)
        {
            uint32_t bits = std::min(uint32_t(8), prefix);
            ipv6addr.sin6_addr.s6_addr[i] >>= 8 - bits;
            ipv6addr.sin6_addr.s6_addr[i] <<= 8 - bits;
            prefix -= bits;
        }

        char strnet[INET6_ADDRSTRLEN];
        if (inet_ntop(AF_INET6, &(ipv6addr.sin6_addr), strnet, INET6_ADDRSTRLEN))
            return Utf8ToUnicode(strnet);
    }
    }
    return ACE_TString();
}
