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

#include "Abuse.h"

namespace spambot {

    Abuse::Abuse(int ipLoginCount, int ipJoinCount, int ipKickCount, Duration window)
        : m_ipLoginCount(ipLoginCount), m_ipJoinCount(ipJoinCount),
          m_ipKickCount(ipKickCount), m_window(window)
    {
    }

    void Abuse::Inc(History& h, const std::string& ipaddr)
    {
        h[ipaddr].push_back(Clock::now());
    }

    void Abuse::IncLogin(const std::string& ipaddr) { Inc(m_iplogins, ipaddr); }
    void Abuse::IncJoins(const std::string& ipaddr) { Inc(m_ipjoins, ipaddr); }
    void Abuse::IncKicks(const std::string& ipaddr) { Inc(m_ipkicks, ipaddr); }

    void Abuse::Clean(History& h)
    {
        auto const now = Clock::now();
        for (auto it = h.begin(); it != h.end(); )
        {
            auto& stamps = it->second;
            while (!stamps.empty() && (now - stamps.front()) > m_window)
                stamps.pop_front();
            if (stamps.empty())
                it = h.erase(it);
            else
                ++it;
        }
    }

    bool Abuse::CheckCount(History& h, const std::string& ipaddr, int threshold)
    {
        Clean(h);
        auto it = h.find(ipaddr);
        if (it == h.end())
            return false;
        return static_cast<int>(it->second.size()) >= threshold;
    }

    bool Abuse::CheckLoginAbuse(const std::string& ipaddr) { return CheckCount(m_iplogins, ipaddr, m_ipLoginCount); }
    bool Abuse::CheckJoinAbuse (const std::string& ipaddr) { return CheckCount(m_ipjoins,  ipaddr, m_ipJoinCount);  }
    bool Abuse::CheckKickAbuse (const std::string& ipaddr) { return CheckCount(m_ipkicks,  ipaddr, m_ipKickCount);  }

    void Abuse::RemoveIpaddr(const std::string& ipaddr)
    {
        m_iplogins.erase(ipaddr);
        m_ipjoins.erase(ipaddr);
        m_ipkicks.erase(ipaddr);
    }

} // namespace spambot
