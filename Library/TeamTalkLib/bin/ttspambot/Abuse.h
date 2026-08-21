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

#ifndef SPAMBOT_ABUSE_H
#define SPAMBOT_ABUSE_H

#include <chrono>
#include <deque>
#include <map>
#include <string>

namespace spambot {

    class Abuse
    {
    public:
        using Clock     = std::chrono::steady_clock;
        using TimePoint = Clock::time_point;
        using Duration  = std::chrono::seconds;

        Abuse(int ipLoginCount, int ipJoinCount, int ipKickCount, Duration window);

        void IncLogin(const std::string& ipaddr);
        void IncJoins(const std::string& ipaddr);
        void IncKicks(const std::string& ipaddr);

        bool CheckLoginAbuse(const std::string& ipaddr);
        bool CheckJoinAbuse(const std::string& ipaddr);
        bool CheckKickAbuse(const std::string& ipaddr);

        void RemoveIpaddr(const std::string& ipaddr);

    private:
        using History = std::map<std::string, std::deque<TimePoint>>;

        void Inc(History& h, const std::string& ipaddr);
        void Clean(History& h);
        bool CheckCount(History& h, const std::string& ipaddr, int threshold);

        History  m_iplogins, m_ipjoins, m_ipkicks;
        int      m_ipLoginCount, m_ipJoinCount, m_ipKickCount;
        Duration m_window;
    };

} // namespace spambot

#endif // SPAMBOT_ABUSE_H
