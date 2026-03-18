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

#if !defined(SPAMBOT_H)
#define SPAMBOT_H

#if defined(ENABLE_TEAMTALKPRO)

#include <ace/SString.h>
#include <ace/Time_Value.h>

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace teamtalk {

    struct SpamBotSettings
    {
        bool enabled = false;

        bool badwords_enabled = false;
        std::vector<std::string> badwords_files;
        bool badwords_auto_download = false;
        std::vector<std::string> badwords_urls;
        int badwords_update_interval_mins = 60;

        bool vpnips_enabled = false;
        std::string vpnips_file;
        bool vpnips_auto_download = false;
        std::string vpnips_url;
        int vpnips_update_interval_mins = 60;

        bool abuse_enabled = false;
        int ip_login_count = 10;
        int ip_joins_count = 10;
        int ip_kicks_count = 10;
        int abuse_duration_secs = 60;
        int ban_duration_secs = 0;
        int ipv4_ban_prefix = 32;
        int ipv6_ban_prefix = 128;

        bool abuseipdb_enabled = false;
        std::string abuseipdb_key;
        int abuseipdb_total_reports = 2;
        int abuseipdb_distinct_users = 2;
        int abuseipdb_confidence_score = 2;

        SpamBotSettings() = default;
    };

    class BadWords
    {
    public:
        BadWords() = default;

        bool LoadFile(const std::string& language, const std::string& filename);
        void LoadFromString(const std::string& language, const std::string& data);
        bool Contains(const std::string& language, const std::string& sentence) const;
        bool ContainsAny(const std::string& sentence) const;
        bool ContainsWord(const std::string& language, const std::string& word) const;
        void Clear();
        std::vector<std::string> GetLanguages() const;

    private:
        std::map<std::string, std::set<std::string>> m_lang_badwords;
    };

    class VpnIpList
    {
    public:
        VpnIpList() = default;

        bool LoadFile(const std::string& filename);
        void LoadFromString(const std::string& data);
        bool IsVpnAddress(const ACE_TString& ipaddr) const;
        void Clear();
        size_t GetCount() const { return m_networks.size(); }
        const std::vector<std::string>& GetNetworks() const { return m_networks; }

    private:
        std::vector<std::string> m_networks;
    };

    class AbuseTracker
    {
    public:
        AbuseTracker() = default;

        void SetThresholds(int loginCount, int joinsCount, int kicksCount,
                           int durationSecs);

        void IncLogin(const std::string& ipaddr);
        void IncJoins(const std::string& ipaddr);
        void IncKicks(const std::string& ipaddr);

        bool CheckLoginAbuse(const std::string& ipaddr);
        bool CheckJoinAbuse(const std::string& ipaddr);
        bool CheckKickAbuse(const std::string& ipaddr);

        void Clean();
        void RemoveIpAddr(const std::string& ipaddr);

    private:
        using timestamps_t = std::vector<ACE_Time_Value>;

        void CleanMap(std::map<std::string, timestamps_t>& m);
        bool CheckAbuse(std::map<std::string, timestamps_t>& m,
                        const std::string& ipaddr, int threshold);

        std::map<std::string, timestamps_t> m_iplogins;
        std::map<std::string, timestamps_t> m_ipjoins;
        std::map<std::string, timestamps_t> m_ipkicks;

        int m_loginCount = 10;
        int m_joinsCount = 10;
        int m_kicksCount = 10;
        int m_durationSecs = 60;
    };

    class AbuseIPDB
    {
    public:
        AbuseIPDB() = default;

        void SetApiKey(const std::string& key) { m_apiKey = key; }
        void SetThresholds(int totalReports, int distinctUsers,
                           int confidenceScore);

        bool CheckForReported(const std::string& ipaddr);
        void Report(const std::string& ipaddr, const std::string& comment);
        void AddWhiteListIPAddr(const std::string& ipaddr);

    private:
        std::string m_apiKey;
        int m_totalReports = 2;
        int m_distinctUsers = 2;
        int m_confidenceScore = 2;

        std::set<std::string> m_whitelist;
        std::set<std::string> m_badlist;
    };

    class SpamBot
    {
    public:
        SpamBot() = default;

        void Configure(const SpamBotSettings& settings);
        void ReloadFiles();
        bool DownloadAndUpdate();

        bool ContainsBadWord(const std::string& text) const;
        bool IsVpnAddress(const ACE_TString& ipaddr) const;
        bool CheckAbuseIPDB(const std::string& ipaddr);
        void ReportToAbuseIPDB(const std::string& ipaddr,
                               const std::string& comment);

        void IncLogin(const std::string& ipaddr);
        void IncJoins(const std::string& ipaddr);
        void IncKicks(const std::string& ipaddr);
        bool CheckLoginAbuse(const std::string& ipaddr);
        bool CheckJoinAbuse(const std::string& ipaddr);
        bool CheckKickAbuse(const std::string& ipaddr);
        void CleanAbuseTrackers();

        int GetBanPrefix(const std::string& ipaddr) const;
        std::string ApplyBanPrefix(const std::string& ipaddr) const;

        const SpamBotSettings& GetSettings() const { return m_settings; }
        bool IsEnabled() const { return m_settings.enabled; }

    private:
        SpamBotSettings m_settings;
        BadWords m_badwords;
        VpnIpList m_vpnips;
        AbuseTracker m_abuse;
        AbuseIPDB m_abuseipdb;
    };

} // namespace teamtalk

#endif /* ENABLE_TEAMTALKPRO */

#endif /* SPAMBOT_H */
