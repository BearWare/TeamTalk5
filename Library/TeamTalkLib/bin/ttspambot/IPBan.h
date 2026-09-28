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

#ifndef SPAMBOT_IPBAN_H
#define SPAMBOT_IPBAN_H

#include <ace/OS.h>
#include <TeamTalk.h>

#include <chrono>
#include <string>
#include <vector>

namespace spambot {

    class IPBan
    {
    public:
        using Duration = std::chrono::seconds;

        IPBan(std::vector<std::string> networks, Duration banDuration);

        static std::vector<std::string> LoadFile(const std::string& filename);
        static void SetBanTimeUTC(BannedUser& ban);

        void AddLocalBan(BannedUser& ban, TTInstance* ttinst);
        void AddRemoteBan(const BannedUser& ban);
        void ClearServerBans();

        int  SyncBans(TTInstance* ttinst);

        static constexpr unsigned NETWORK_BAN_TYPE = BANTYPE_IPADDR;
        static constexpr unsigned USER_BAN_TYPE    = BANTYPE_IPADDR | BANTYPE_USERNAME;

    private:
        std::vector<BannedUser> m_serverbans;
        std::vector<std::string> m_networks;
        int  m_networkindex = -1;
        Duration m_banDuration;
    };

} // namespace spambot

#endif // SPAMBOT_IPBAN_H
