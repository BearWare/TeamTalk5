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

#include "IPBan.h"

#include "teamtalk/Log.h"

#include <ace/Log_Msg.h>
#include <ace/OS_NS_stdio.h>
#include <ace/OS_NS_string.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <regex>
#include <string>

namespace spambot {

    namespace {

        std::string Tstr(const TTCHAR* p)
        {
#if defined(UNICODE)
            std::wstring ws(p);
            return std::string(ws.begin(), ws.end());
#else
            return std::string(p);
#endif
        }

        void SetTstr(TTCHAR* dst, std::size_t cap, const std::string& src)
        {
#if defined(UNICODE)
            std::wstring ws(src.begin(), src.end());
            std::size_t n = std::min(ws.size(), cap - 1);
            std::copy_n(ws.begin(), n, dst);
            dst[n] = 0;
#else
            std::size_t n = std::min(src.size(), cap - 1);
            std::copy_n(src.begin(), n, dst);
            dst[n] = 0;
#endif
        }

    } // anonymous namespace

    IPBan::IPBan(std::vector<std::string> networks, Duration banDuration)
        : m_networks(std::move(networks)), m_banDuration(banDuration)
    {
    }

    std::vector<std::string> IPBan::LoadFile(const std::string& filename)
    {
        std::vector<std::string> networks;
        if (filename.empty())
            return networks;

        std::ifstream f(filename);
        if (!f.is_open())
        {
            ACE_TCHAR msg[512];
            ACE_OS::snprintf(msg, 512, ACE_TEXT("Failed to load VPN IP file %C."), filename.c_str());
            TT_SYSLOG(msg);
            return networks;
        }

        std::string line;
        while (std::getline(f, line))
        {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            if (!line.empty())
                networks.push_back(line);
        }
        return networks;
    }

    void IPBan::SetBanTimeUTC(BannedUser& ban)
    {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_utc{};
#if defined(WIN32)
        gmtime_s(&tm_utc, &t);
#else
        gmtime_r(&t, &tm_utc);
#endif
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%04d/%02d/%02d %02d:%02d",
                      tm_utc.tm_year + 1900, tm_utc.tm_mon + 1, tm_utc.tm_mday,
                      tm_utc.tm_hour, tm_utc.tm_min);
        SetTstr(ban.szBanTime, TT_STRLEN, buf);
    }

    void IPBan::AddLocalBan(BannedUser& ban, TTInstance* ttinst)
    {
        UserAccount ua{};
        TT_GetMyUserAccount(ttinst, &ua);
        SetTstr(ban.szOwner, TT_STRLEN, Tstr(ua.szUsername));
        SetBanTimeUTC(ban);
        AddRemoteBan(ban);
    }

    void IPBan::AddRemoteBan(const BannedUser& ban)
    {
        ACE_TCHAR msg[512];
        if (Tstr(ban.szBanTime).empty())
        {
            ACE_OS::snprintf(msg, 512, ACE_TEXT("Ban time is not set, IP: %C."), Tstr(ban.szIPAddress).c_str());
            TT_SYSLOG(msg);
        }
        if (Tstr(ban.szOwner).empty())
        {
            ACE_OS::snprintf(msg, 512, ACE_TEXT("Ban owner is not set, IP: %C."), Tstr(ban.szIPAddress).c_str());
            TT_SYSLOG(msg);
        }
        if (ban.uBanTypes == BANTYPE_NONE)
        {
            ACE_OS::snprintf(msg, 512, ACE_TEXT("Ban type is not set, IP: %C."), Tstr(ban.szIPAddress).c_str());
            TT_SYSLOG(msg);
        }
        m_serverbans.push_back(ban);
    }

    void IPBan::ClearServerBans()
    {
        m_serverbans.clear();
        m_networkindex = -1;
    }

    int IPBan::SyncBans(TTInstance* ttinst)
    {
        UserAccount ua{};
        TT_GetMyUserAccount(ttinst, &ua);
        const std::string myname = Tstr(ua.szUsername);

        std::vector<BannedUser> mybans;
        std::vector<std::string> ipaddrs;
        for (const auto& b : m_serverbans)
        {
            if (b.uBanTypes == NETWORK_BAN_TYPE && Tstr(b.szOwner) == myname)
            {
                mybans.push_back(b);
                ipaddrs.push_back(Tstr(b.szIPAddress));
            }
        }

        for (const auto& b : mybans)
        {
            const std::string ip = Tstr(b.szIPAddress);
            if (std::find(m_networks.begin(), m_networks.end(), ip) == m_networks.end())
            {
                m_serverbans.erase(std::remove_if(m_serverbans.begin(), m_serverbans.end(),
                    [&](const BannedUser& x){ return Tstr(x.szIPAddress) == ip
                                                  && x.uBanTypes == b.uBanTypes
                                                  && Tstr(x.szOwner) == myname; }),
                    m_serverbans.end());
                ACE_TCHAR msg[512];
                ACE_OS::snprintf(msg, 512, ACE_TEXT("Removed missing network ban: IP %C."), ip.c_str());
                TT_LOG(msg);
                return TT_DoUnBanUserEx(ttinst, &b);
            }
        }

        for (++m_networkindex; m_networkindex < static_cast<int>(m_networks.size()); ++m_networkindex)
        {
            const std::string& ip = m_networks[m_networkindex];
            if (std::find(ipaddrs.begin(), ipaddrs.end(), ip) == ipaddrs.end())
            {
                BannedUser nb{};
                SetTstr(nb.szIPAddress, TT_STRLEN, ip);
                nb.uBanTypes = NETWORK_BAN_TYPE;
                ACE_TCHAR msg[512];
                ACE_OS::snprintf(msg, 512, ACE_TEXT("Added new network ban: IP %C."), ip.c_str());
                TT_LOG(msg);
                int cmdid = TT_DoBan(ttinst, &nb);
                if (cmdid > 0)
                    AddLocalBan(nb, ttinst);
                return cmdid;
            }
        }

        if (m_banDuration.count() > 0)
        {
            auto cutoff = std::chrono::system_clock::now() - m_banDuration;
            std::time_t cutoff_t = std::chrono::system_clock::to_time_t(cutoff);
            std::tm cutoff_tm{};
#if defined(WIN32)
            gmtime_s(&cutoff_tm, &cutoff_t);
#else
            gmtime_r(&cutoff_t, &cutoff_tm);
#endif
            std::regex re(R"(^(\d+)/(\d+)/(\d+) (\d+):(\d+)$)");
            for (auto it = m_serverbans.begin(); it != m_serverbans.end(); ++it)
            {
                if (it->uBanTypes != USER_BAN_TYPE || Tstr(it->szOwner) != myname)
                    continue;
                std::smatch m;
                std::string bantime = Tstr(it->szBanTime);
                if (!std::regex_search(bantime, m, re))
                    continue;
                std::tm bt{};
                bt.tm_year = std::stoi(m[1]) - 1900;
                bt.tm_mon  = std::stoi(m[2]) - 1;
                bt.tm_mday = std::stoi(m[3]);
                bt.tm_hour = std::stoi(m[4]);
                bt.tm_min  = std::stoi(m[5]);

                auto cmp = [](const std::tm& a, const std::tm& b) {
                    if (a.tm_year != b.tm_year) return a.tm_year < b.tm_year;
                    if (a.tm_mon  != b.tm_mon ) return a.tm_mon  < b.tm_mon;
                    if (a.tm_mday != b.tm_mday) return a.tm_mday < b.tm_mday;
                    if (a.tm_hour != b.tm_hour) return a.tm_hour < b.tm_hour;
                    return a.tm_min < b.tm_min;
                };
                if (cmp(bt, cutoff_tm))
                {
                    BannedUser b = *it;
                    m_serverbans.erase(it);
                    ACE_TCHAR msg[512];
                    ACE_OS::snprintf(msg, 512, ACE_TEXT("Removed expired ban: IP %C."), Tstr(b.szIPAddress).c_str());
                    TT_LOG(msg);
                    return TT_DoUnBanUserEx(ttinst, &b);
                }
            }
        }

        return 0;
    }

} // namespace spambot
