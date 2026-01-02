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

#include <ace/INET_Addr.h>
#include <ace/INet/HTTP_ClientRequestHandler.h>
#include <ace/INet/HTTP_URL.h>
#include <ace/INet/URLBase.h>

#if defined(ENABLE_ENCRYPTION)
#include <ace/INet/HTTPS_SessionFactory.h>
#endif

#include <ace/OS.h>
#if defined(WIN32)
#else
#include <arpa/inet.h>
#if defined(ACE_HAS_IPV6)
#include <net/if.h>
#endif
#endif

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <iomanip>
#include <memory>
#include <sstream>

std::vector<ACE_INET_Addr> DetermineHostAddress(const ACE_TString& host, uint16_t port)
{
    std::vector<ACE_INET_Addr> result;

    // Fast path for numeric literals (avoids platform resolver quirks).
    // getaddrinfo() should handle numeric hosts, but on some Android devices
    // it can still fail transiently (EAI_AGAIN) even for IPv6 literals.
    const auto host_utf8_full = UnicodeToUtf8(host);
    const char* host_utf8_cstr = host_utf8_full.c_str();
    std::string host_utf8(host_utf8_cstr != nullptr ? host_utf8_cstr : "");

    // IPv6 literals may include a zone index, e.g. "fe80::1%wlan0".
    std::string v6addr = host_utf8;
    std::string v6scope;
    auto percent = host_utf8.find('%');
    if (percent != std::string::npos)
    {
        v6addr = host_utf8.substr(0, percent);
        v6scope = host_utf8.substr(percent + 1);
    }

    sockaddr_in6 sa6 {};
    if (!v6addr.empty() && inet_pton(AF_INET6, v6addr.c_str(), &sa6.sin6_addr) == 1)
    {
        bool ok = true;
        if (!v6scope.empty())
        {
            unsigned int scope_id = 0;
            bool digits = std::all_of(v6scope.begin(), v6scope.end(),
                                      [](unsigned char c) { return std::isdigit(c) != 0; });
            if (digits)
            {
                scope_id = static_cast<unsigned int>(std::strtoul(v6scope.c_str(), nullptr, 10));
            }
            else
            {
                scope_id = if_nametoindex(v6scope.c_str());
            }
            if (scope_id == 0)
                ok = false;
            else
                sa6.sin6_scope_id = scope_id;
        }
        if (ok)
        {
#if defined(ACE_HAS_SOCKADDR_IN6_SIN6_LEN)
            sa6.sin6_len = sizeof(sa6);
#endif
            sa6.sin6_family = AF_INET6;
            sa6.sin6_port = ACE_HTONS(port);
            // ACE_INET_Addr::set() takes a `sockaddr_in*` even for IPv6; the `len`
            // parameter determines whether it is interpreted as IPv4 or IPv6.
            ACE_INET_Addr addr6;
            addr6.set(reinterpret_cast<const sockaddr_in*>(static_cast<const void*>(&sa6)), sizeof(sa6));
            result.push_back(addr6);
            return result;
        }
    }

    sockaddr_in sa4 {};
    if (!host_utf8.empty() && inet_pton(AF_INET, host_utf8.c_str(), &sa4.sin_addr) == 1)
    {
#if defined(ACE_HAS_SOCKADDR_IN_SIN_LEN)
        sa4.sin_len = sizeof(sa4);
#endif
        sa4.sin_family = AF_INET;
        sa4.sin_port = ACE_HTONS(port);
        ACE_INET_Addr addr4;
        addr4.set(&sa4, sizeof(sa4));
        result.push_back(addr4);
        return result;
    }

    addrinfo hints;
    ACE_OS::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    // Avoid AI_ALL / AI_V4MAPPED with AF_UNSPEC: on some libc implementations
    // (notably Android's bionic) this combination returns EAI_BADFLAGS for
    // IPv6‑only hosts, which leaves us with an empty address list. Stick to
    // plain lookups here so both IPv4 and IPv6 results are returned.
    hints.ai_flags = 0;

    // Do not constrain ai_socktype here. Android's resolver can fail in some
    // network configurations when ai_socktype is set, and TeamTalk also uses
    // UDP sockets so we want results for both transport types.
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;

    addrinfo* res = nullptr;

    // Android's resolver may return EAI_AGAIN transiently even when DNS works
    // (e.g. during network transitions). Retry a few times before giving up.
    int addrinfoError = 0;
    for (int attempt = 0; attempt < 3; ++attempt)
    {
        addrinfoError = ACE_OS::getaddrinfo(host_utf8_full.c_str(), nullptr, &hints, &res);
        if (addrinfoError == 0 || addrinfoError != EAI_AGAIN)
            break;
        ACE_OS::sleep(ACE_Time_Value(0, 200 * 1000)); // 200ms
    }

    if (addrinfoError != 0)
    {
        errno = addrinfoError;
#if defined(UNICODE)
        MYTRACE(ACE_TEXT("getaddrinfo failed host=%s err=%d (%s) flags=0x%x family=%d\n"),
                Utf8ToUnicode(host_utf8_full.c_str()).c_str(), addrinfoError,
                Utf8ToUnicode(ACE_OS::strerror(addrinfoError)).c_str(), hints.ai_flags, hints.ai_family);
#else
        MYTRACE(ACE_TEXT("getaddrinfo failed host=%s err=%d (%s) flags=0x%x family=%d\n"),
                host_utf8_full.c_str(), addrinfoError, ACE_OS::strerror(addrinfoError),
                hints.ai_flags, hints.ai_family);
#endif
        return {};
    }

    for (addrinfo* curr = res; curr != nullptr; curr = curr->ai_next)
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
            addr.in6_.sin6_port = htons(port);
            result.emplace_back(reinterpret_cast<const sockaddr_in*>(&addr.in6_), sizeof(addr.in6_));
        }
        else
#endif
        {
            addr.in4_.sin_port = htons(port);
            result.emplace_back(reinterpret_cast<const sockaddr_in*>(&addr.in4_), sizeof(addr.in4_));
        }
    }

    ACE_OS::freeaddrinfo(res);

    return result;
}

static void AceSingletons()
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
    AceSingletons();

    std::unique_ptr<ACE::INet::URL_Base> url_safe(ACE::INet::URL_Base::create_from_string(url));
    if (!url_safe)
        return -1;

    ACE::HTTP::ClientRequestHandler http;
    ACE::INet::URLStream urlin = url_safe->open(http);

    std::ostringstream oss;
    oss << urlin->rdbuf();
    result = oss.str();

    ACE::HTTP::Status const httpCode = http.response().get_status();
#if defined(UNICODE)
    MYTRACE_COND(!httpCode.is_ok(), ACE_TEXT("HTTP request failed:\n%s\n"),
        Utf8ToUnicode(result.c_str()).c_str());
#else
    MYTRACE_COND(!httpCode.is_ok(), ACE_TEXT("HTTP request failed:\n%s\n"), result.c_str());
#endif
    if (statusCode != nullptr)
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
    AceSingletons();

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
        std::istream& handle_open_request(const ACE::INet::URL_Base& url) override
        {
            if (request().get_method() == ACE::HTTP::Request::HTTP_GET)
            {
                return ACE::HTTP::ClientRequestHandler::handle_open_request(url);
            }
            if (request().get_method() == ACE::HTTP::Request::HTTP_POST)
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

                for (const auto& v : m_headers)
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
    std::ostringstream oss;
    oss << urlin->rdbuf();
    result = oss.str();
    auto httpCode = http.response().get_status();
    if (statusCode != nullptr)
        *statusCode = httpCode.get_status();

    return httpCode.is_ok() ? 1 : 0;
}

int HttpPostRequest(const ACE_CString& url, const std::map<std::string,std::string>& unencodedformdata,
                    std::string& result, ACE::HTTP::Status::Code* statusCode/* = nullptr*/)
{
    std::string content;
    auto count = unencodedformdata.size();
    for (const auto& vp : unencodedformdata)
    {
        content += URLEncode(vp.first) + "=" + URLEncode(vp.second);
        if (--count > 0)
            content += "&";
    }
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = "application/x-www-form-urlencoded";
    return HttpPostRequest(url, content.c_str(), content.size(), headers, result, statusCode);
}

std::string URLEncode(const std::string& utf8)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : utf8)
    {
        // Keep alphanumeric and other accepted characters intact
        if ((isalnum(c) != 0) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char)c);
        escaped << std::nouppercase;
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
    int const af = InetAddrFamily(ipaddr);
    switch (af)
    {
    case AF_INET :
    {
        struct sockaddr_in ipv4addr;
        if (inet_pton(AF_INET, UnicodeToUtf8(ipaddr).c_str(), &(ipv4addr.sin_addr)) <= 0)
            return {};
        prefix = std::min(prefix, uint32_t(32));
        uint32_t const shift = 32-prefix;
        ipv4addr.sin_addr.s_addr = ntohl(ipv4addr.sin_addr.s_addr);
        ipv4addr.sin_addr.s_addr >>= shift;
        ipv4addr.sin_addr.s_addr <<= shift;
        ipv4addr.sin_addr.s_addr = ntohl(ipv4addr.sin_addr.s_addr);
        char strnet[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &(ipv4addr.sin_addr.s_addr), strnet, INET_ADDRSTRLEN) != nullptr)
            return Utf8ToUnicode(strnet);
    }
    case AF_INET6 :
    {
        struct sockaddr_in6 ipv6addr;
        if (inet_pton(AF_INET6, UnicodeToUtf8(ipaddr).c_str(), &(ipv6addr.sin6_addr)) <= 0)
            return {};
        prefix = std::min(prefix, uint32_t(128));
        for (int i = 0; i < 16; ++i)
        {
            uint32_t const bits = std::min(uint32_t(8), prefix);
            ipv6addr.sin6_addr.s6_addr[i] >>= 8 - bits;
            ipv6addr.sin6_addr.s6_addr[i] <<= 8 - bits;
            prefix -= bits;
        }

        char strnet[INET6_ADDRSTRLEN];
        if (inet_ntop(AF_INET6, &(ipv6addr.sin6_addr), strnet, INET6_ADDRSTRLEN) != nullptr)
            return Utf8ToUnicode(strnet);
    }
    }
    return {};
}
