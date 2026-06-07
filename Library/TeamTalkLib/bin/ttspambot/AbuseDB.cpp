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

#include "AbuseDB.h"

#include "myace/MyINet.h"
#include "teamtalk/Log.h"

#include <ace/Log_Msg.h>
#include <ace/OS_NS_stdio.h>
#include <ace/OS_NS_string.h>

#include <map>
#include <regex>
#include <string>

namespace spambot {

    namespace {
        bool ParseJsonInt(const std::string& body, const std::string& key, int& out)
        {
            std::regex re("\"" + key + "\"\\s*:\\s*(-?\\d+)");
            std::smatch m;
            if (std::regex_search(body, m, re))
            {
                try { out = std::stoi(m[1]); return true; }
                catch (...) { return false; }
            }
            return false;
        }

        bool ParseJsonBool(const std::string& body, const std::string& key, bool& out)
        {
            std::regex re("\"" + key + "\"\\s*:\\s*(true|false)");
            std::smatch m;
            if (std::regex_search(body, m, re))
            {
                out = (m[1] == "true");
                return true;
            }
            return false;
        }
    } // anonymous namespace

    AbuseDB::AbuseDB(std::string apikey, int totalReports, int distinctUsers, int confidenceScore,
                     bool lookupEnabled, bool reportEnabled)
        : m_apikey(std::move(apikey)),
          m_totalReports(totalReports), m_distinctUsers(distinctUsers),
          m_confidenceScore(confidenceScore),
          m_lookupEnabled(lookupEnabled), m_reportEnabled(reportEnabled)
    {
    }

    void AbuseDB::AddWhiteListIPAddr(const std::string& ipaddr)
    {
        m_whitelist.insert(ipaddr);
    }

    bool AbuseDB::CheckForReported(const std::string& ipaddr)
    {
        if (!m_lookupEnabled || m_apikey.empty() || ipaddr.empty())
            return false;
        if (m_whitelist.count(ipaddr))
            return false;
        if (m_badlist.count(ipaddr))
            return true;

        std::string url = "https://api.abuseipdb.com/api/v2/check?ipAddress=";
        url += URLEncode(ipaddr);
        url += "&maxAgeInDays=365";

        std::map<std::string,std::string> headers = {
            { "Accept", "application/json" },
            { "Key",    m_apikey            },
        };

        std::string body;
        int rc = HttpGetRequest(url.c_str(), headers, body);
        if (rc != 1)
        {
            ACE_TCHAR msg[512];
            ACE_OS::snprintf(msg, 512, ACE_TEXT("AbuseIPDB check failed for IP-address %C."), ipaddr.c_str());
            TT_SYSLOG(msg);
            return false;
        }

        int total = 0, distinct = 0, score = 0;
        bool whitelisted = false;
        ParseJsonInt(body, "totalReports",         total);
        ParseJsonInt(body, "numDistinctUsers",     distinct);
        ParseJsonInt(body, "abuseConfidenceScore", score);
        ParseJsonBool(body, "isWhitelisted",       whitelisted);

        if (total >= m_totalReports && distinct >= m_distinctUsers &&
            score >= m_confidenceScore && !whitelisted)
        {
            m_badlist.insert(ipaddr);
            return true;
        }
        m_whitelist.insert(ipaddr);
        return false;
    }

    void AbuseDB::Report(const std::string& ipaddr, const std::string& comment)
    {
        if (!m_reportEnabled || m_apikey.empty() || ipaddr.empty())
            return;

        std::string form = "ip=" + URLEncode(ipaddr);
        form += "&categories=13";
        form += "&comment=" + URLEncode(comment);

        std::map<std::string,std::string> headers = {
            { "Accept",       "application/json"                  },
            { "Key",          m_apikey                            },
            { "Content-Type", "application/x-www-form-urlencoded" },
        };

        std::string body;
        ACE::HTTP::Status::Code code = ACE::HTTP::Status::INVALID;
        int rc = HttpPostRequest("https://api.abuseipdb.com/api/v2/report",
                                 form.c_str(), static_cast<int>(form.size()),
                                 headers, body, &code);
        if (rc != 1)
        {
            ACE_TCHAR msg[1024];
            ACE_OS::snprintf(msg, 1024, ACE_TEXT("AbuseIPDB report failed for IP-address %C: %C"),
                             ipaddr.c_str(), body.c_str());
            TT_SYSLOG(msg);
        }
    }

} // namespace spambot
