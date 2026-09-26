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

#ifndef SPAMBOT_ABUSEDB_H
#define SPAMBOT_ABUSEDB_H

#include <set>
#include <string>

namespace spambot {

    class AbuseDB
    {
    public:
        AbuseDB(std::string apikey,
                int totalReports, int distinctUsers, int confidenceScore,
                bool lookupEnabled, bool reportEnabled);

        void AddWhiteListIPAddr(const std::string& ipaddr);

        bool CheckForReported(const std::string& ipaddr);

        void Report(const std::string& ipaddr, const std::string& comment);

        bool LookupEnabled() const { return m_lookupEnabled; }
        bool ReportEnabled() const { return m_reportEnabled; }

    private:
        std::string    m_apikey;
        std::set<std::string> m_whitelist;
        std::set<std::string> m_badlist;
        int            m_totalReports, m_distinctUsers, m_confidenceScore;
        bool           m_lookupEnabled, m_reportEnabled;
    };

} // namespace spambot

#endif // SPAMBOT_ABUSEDB_H
