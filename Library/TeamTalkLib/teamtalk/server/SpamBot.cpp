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

#include "SpamBot.h"

#if defined(ENABLE_TEAMTALKPRO)

#include "myace/MyACE.h"
#include "myace/MyINet.h"
#include "teamtalk/Log.h"

#include <ace/INET_Addr.h>
#include <ace/OS.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

namespace teamtalk {

    //////////////////////////////////////
    // class BadWords
    //////////////////////////////////////

    bool BadWords::LoadFile(const std::string& language,
                            const std::string& filename)
    {
        std::ifstream ifs(filename);
        if (!ifs.is_open())
            return false;

        std::string content((std::istreambuf_iterator<char>(ifs)),
                            std::istreambuf_iterator<char>());
        ifs.close();

        LoadFromString(language, content);
        return true;
    }

    void BadWords::LoadFromString(const std::string& language,
                                  const std::string& data)
    {
        std::set<std::string>& words = m_lang_badwords[language];
        words.clear();

        std::istringstream iss(data);
        std::string line;
        while (std::getline(iss, line))
        {
            std::istringstream linestream(line);
            std::string word;
            while (std::getline(linestream, word, ','))
            {
                while (!word.empty() && std::isspace(static_cast<unsigned char>(word.front())))
                    word.erase(word.begin());
                while (!word.empty() && std::isspace(static_cast<unsigned char>(word.back())))
                    word.pop_back();

                if (word.empty())
                    continue;

                std::transform(word.begin(), word.end(), word.begin(),
                    [](unsigned char c) { return std::tolower(c); });
                words.insert(word);
            }
        }
    }

    bool BadWords::Contains(const std::string& language,
                            const std::string& sentence) const
    {
        auto it = m_lang_badwords.find(language);
        if (it == m_lang_badwords.end())
            return false;

        std::string lower = sentence;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return c == '_' ? ' ' : std::tolower(c); });

        std::regex const wordSplit(R"(\W+)");
        std::sregex_token_iterator iter(lower.begin(), lower.end(), wordSplit, -1);
        std::sregex_token_iterator const end;

        for (; iter != end; ++iter)
        {
            std::string const word = iter->str();
            if (!word.empty() && it->second.count(word) > 0)
                return true;
        }

        return false;
    }

    bool BadWords::ContainsAny(const std::string& sentence) const
    {
        for (const auto& [lang, _] : m_lang_badwords)
        {
            if (Contains(lang, sentence))
                return true;
        }
        return false;
    }

    bool BadWords::ContainsWord(const std::string& language,
                                const std::string& word) const
    {
        auto it = m_lang_badwords.find(language);
        if (it == m_lang_badwords.end())
            return false;

        std::string lower = word;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return std::tolower(c); });

        return it->second.count(lower) > 0;
    }

    void BadWords::Clear()
    {
        m_lang_badwords.clear();
    }

    std::vector<std::string> BadWords::GetLanguages() const
    {
        std::vector<std::string> langs;
        for (const auto& [lang, _] : m_lang_badwords)
            langs.push_back(lang);
        return langs;
    }

    //////////////////////////////////////
    // class VpnIpList
    //////////////////////////////////////

    bool VpnIpList::LoadFile(const std::string& filename)
    {
        std::ifstream ifs(filename);
        if (!ifs.is_open())
            return false;

        std::string content((std::istreambuf_iterator<char>(ifs)),
                            std::istreambuf_iterator<char>());
        ifs.close();

        LoadFromString(content);
        return true;
    }

    void VpnIpList::LoadFromString(const std::string& data)
    {
        m_networks.clear();

        std::istringstream iss(data);
        std::string line;
        while (std::getline(iss, line))
        {
            while (!line.empty() && std::isspace(static_cast<unsigned char>(line.front())))
                line.erase(line.begin());
            while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back())))
                line.pop_back();

            if (line.empty() || line[0] == '#')
                continue;

            m_networks.push_back(line);
        }
    }

    bool VpnIpList::IsVpnAddress(const ACE_TString& ipaddr) const
    {
        if (m_networks.empty())
            return false;

        std::string const ip = UnicodeToUtf8(ipaddr).c_str();
        if (ip.empty())
            return false;

        int const userFamily = InetAddrFamily(ipaddr);

        for (const auto& entry : m_networks)
        {
            std::string network = entry;
            int prefix = -1;
            auto slashPos = network.find('/');
            if (slashPos != std::string::npos)
            {
                prefix = std::atoi(network.c_str() + slashPos + 1);
                network = network.substr(0, slashPos);
            }

            ACE_TString const entryAddr = Utf8ToUnicode(network.c_str());
            int const entryFamily = InetAddrFamily(entryAddr);

            if (entryFamily != userFamily)
                continue;

            if (prefix < 0)
            {
                if (ip == network)
                    return true;
            }
            else
            {
                ACE_TString const userNet = INetAddrNetwork(ipaddr, prefix);
                ACE_TString const entryNet = INetAddrNetwork(entryAddr, prefix);
                if (!userNet.is_empty() && userNet == entryNet)
                    return true;
            }
        }

        return false;
    }

    void VpnIpList::Clear()
    {
        m_networks.clear();
    }

    //////////////////////////////////////
    // class AbuseTracker
    //////////////////////////////////////

    void AbuseTracker::SetThresholds(int loginCount, int joinsCount,
                                     int kicksCount, int durationSecs)
    {
        m_loginCount = loginCount;
        m_joinsCount = joinsCount;
        m_kicksCount = kicksCount;
        m_durationSecs = durationSecs;
    }

    void AbuseTracker::IncLogin(const std::string& ipaddr)
    {
        m_iplogins[ipaddr].push_back(ACE_OS::gettimeofday());
    }

    void AbuseTracker::IncJoins(const std::string& ipaddr)
    {
        m_ipjoins[ipaddr].push_back(ACE_OS::gettimeofday());
    }

    void AbuseTracker::IncKicks(const std::string& ipaddr)
    {
        m_ipkicks[ipaddr].push_back(ACE_OS::gettimeofday());
    }

    bool AbuseTracker::CheckLoginAbuse(const std::string& ipaddr)
    {
        return CheckAbuse(m_iplogins, ipaddr, m_loginCount);
    }

    bool AbuseTracker::CheckJoinAbuse(const std::string& ipaddr)
    {
        return CheckAbuse(m_ipjoins, ipaddr, m_joinsCount);
    }

    bool AbuseTracker::CheckKickAbuse(const std::string& ipaddr)
    {
        return CheckAbuse(m_ipkicks, ipaddr, m_kicksCount);
    }

    void AbuseTracker::Clean()
    {
        CleanMap(m_iplogins);
        CleanMap(m_ipjoins);
        CleanMap(m_ipkicks);
    }

    void AbuseTracker::RemoveIpAddr(const std::string& ipaddr)
    {
        m_iplogins.erase(ipaddr);
        m_ipjoins.erase(ipaddr);
        m_ipkicks.erase(ipaddr);
    }

    void AbuseTracker::CleanMap(std::map<std::string, timestamps_t>& m)
    {
        ACE_Time_Value const now = ACE_OS::gettimeofday();
        ACE_Time_Value const window(m_durationSecs, 0);
        ACE_Time_Value const cutoff = now - window;

        for (auto it = m.begin(); it != m.end(); )
        {
            auto& timestamps = it->second;
            timestamps.erase(
                std::remove_if(timestamps.begin(), timestamps.end(),
                    [&cutoff](const ACE_Time_Value& t) { return t < cutoff; }),
                timestamps.end());

            if (timestamps.empty())
                it = m.erase(it);
            else
                ++it;
        }
    }

    bool AbuseTracker::CheckAbuse(std::map<std::string, timestamps_t>& m,
                                  const std::string& ipaddr, int threshold)
    {
        auto it = m.find(ipaddr);
        if (it == m.end())
            return false;

        ACE_Time_Value const now = ACE_OS::gettimeofday();
        ACE_Time_Value const window(m_durationSecs, 0);
        ACE_Time_Value const cutoff = now - window;

        auto& timestamps = it->second;
        timestamps.erase(
            std::remove_if(timestamps.begin(), timestamps.end(),
                [&cutoff](const ACE_Time_Value& t) { return t < cutoff; }),
            timestamps.end());

        return static_cast<int>(timestamps.size()) >= threshold;
    }

    //////////////////////////////////////
    // class AbuseIPDB
    //////////////////////////////////////

    void AbuseIPDB::SetThresholds(int totalReports, int distinctUsers,
                                  int confidenceScore)
    {
        m_totalReports = totalReports;
        m_distinctUsers = distinctUsers;
        m_confidenceScore = confidenceScore;
    }

    void AbuseIPDB::AddWhiteListIPAddr(const std::string& ipaddr)
    {
        m_whitelist.insert(ipaddr);
        m_badlist.erase(ipaddr);
    }

    bool AbuseIPDB::CheckForReported(const std::string& ipaddr)
    {
        if (m_apiKey.empty())
            return false;

        if (m_whitelist.count(ipaddr) > 0)
            return false;
        if (m_badlist.count(ipaddr) > 0)
            return true;

        std::string url = "https://api.abuseipdb.com/api/v2/check?ipAddress=";
        url += URLEncode(ipaddr);
        url += "&maxAgeInDays=365";

        std::map<std::string, std::string> headers;
        headers["Key"] = m_apiKey;
        headers["Accept"] = "application/json";

        std::string result;
        ACE::HTTP::Status::Code httpCode = ACE::HTTP::Status::INVALID;
        int const ret = HttpGetRequest(url.c_str(), headers, result, &httpCode);
        if (ret <= 0)
            return false;

        try
        {
            auto getJsonInt = [&result](const std::string& key) -> int {
                auto pos = result.find("\"" + key + "\"");
                if (pos == std::string::npos)
                    return 0;
                pos = result.find(':', pos);
                if (pos == std::string::npos)
                    return 0;
                pos++;
                while (pos < result.size() && std::isspace(static_cast<unsigned char>(result[pos])))
                    pos++;
                return std::atoi(result.c_str() + pos);
            };

            auto getJsonBool = [&result](const std::string& key) -> bool {
                auto pos = result.find("\"" + key + "\"");
                if (pos == std::string::npos)
                    return false;
                pos = result.find(':', pos);
                if (pos == std::string::npos)
                    return false;
                return result.find("true", pos) == pos + 1 ||
                       result.find("true", pos) == pos + 2;
            };

            int const reports = getJsonInt("totalReports");
            int const users = getJsonInt("numDistinctUsers");
            int const score = getJsonInt("abuseConfidenceScore");
            bool const whitelisted = getJsonBool("isWhitelisted");

            if (whitelisted)
            {
                m_whitelist.insert(ipaddr);
                return false;
            }

            bool const abusive = reports >= m_totalReports &&
                                 users >= m_distinctUsers &&
                                 score >= m_confidenceScore;

            if (abusive)
                m_badlist.insert(ipaddr);
            else
                m_whitelist.insert(ipaddr);

            return abusive;
        }
        catch (...)
        {
            return false;
        }
    }

    void AbuseIPDB::Report(const std::string& ipaddr,
                           const std::string& comment)
    {
        if (m_apiKey.empty())
            return;

        std::string const url = "https://api.abuseipdb.com/api/v2/report";

        std::string postdata = "ip=" + URLEncode(ipaddr) +
            "&categories=13&comment=" + URLEncode(comment);

        std::map<std::string, std::string> headers;
        headers["Key"] = m_apiKey;
        headers["Accept"] = "application/json";
        headers["Content-Type"] = "application/x-www-form-urlencoded";

        std::string result;
        HttpPostRequest(url.c_str(), postdata.c_str(),
                        static_cast<int>(postdata.size()), headers, result);
    }

    //////////////////////////////////////
    // class SpamBot
    //////////////////////////////////////

    void SpamBot::Configure(const SpamBotSettings& settings)
    {
        m_settings = settings;

        if (m_settings.abuse_enabled)
        {
            m_abuse.SetThresholds(m_settings.ip_login_count,
                                  m_settings.ip_joins_count,
                                  m_settings.ip_kicks_count,
                                  m_settings.abuse_duration_secs);
        }

        if (m_settings.abuseipdb_enabled)
        {
            m_abuseipdb.SetApiKey(m_settings.abuseipdb_key);
            m_abuseipdb.SetThresholds(m_settings.abuseipdb_total_reports,
                                      m_settings.abuseipdb_distinct_users,
                                      m_settings.abuseipdb_confidence_score);
        }

        ReloadFiles();
    }

    void SpamBot::ReloadFiles()
    {
        if (m_settings.badwords_enabled)
        {
            m_badwords.Clear();
            for (const auto& filepath : m_settings.badwords_files)
            {
                // derive language from filename: "badwords_french.txt" -> "french"
                std::string lang;
                auto pos = filepath.rfind('/');
                std::string filename = (pos != std::string::npos) ?
                    filepath.substr(pos + 1) : filepath;
                auto dotpos = filename.rfind('.');
                if (dotpos != std::string::npos)
                    filename = filename.substr(0, dotpos);
                auto underpos = filename.find('_');
                if (underpos != std::string::npos)
                    lang = filename.substr(underpos + 1);

                if (!m_badwords.LoadFile(lang, filepath))
                {
                    TT_LOG(ACE_TEXT("SpamBot: Failed to load bad words file: %hs"),
                           filepath.c_str());
                }
            }
        }

        if (m_settings.vpnips_enabled && !m_settings.vpnips_file.empty())
        {
            if (!m_vpnips.LoadFile(m_settings.vpnips_file))
            {
                TT_LOG(ACE_TEXT("SpamBot: Failed to load VPN IPs file: %hs"),
                       m_settings.vpnips_file.c_str());
            }
        }
    }

    bool SpamBot::DownloadAndUpdate()
    {
        bool updated = false;

        if (m_settings.badwords_enabled && m_settings.badwords_auto_download)
        {
            for (size_t i = 0; i < m_settings.badwords_urls.size(); i++)
            {
                const auto& url = m_settings.badwords_urls[i];
                if (url.empty())
                    continue;

                std::string content;
                int const ret = HttpGetRequest(url.c_str(), content);
                if (ret > 0 && !content.empty())
                {
                    std::string filepath;
                    if (i < m_settings.badwords_files.size())
                        filepath = m_settings.badwords_files[i];
                    else
                        filepath = "badwords_" + std::to_string(i) + ".txt";

                    std::ofstream ofs(filepath, std::ios::trunc);
                    if (ofs.is_open())
                    {
                        ofs << content;
                        ofs.close();
                        updated = true;
                    }
                }
            }
        }

        if (m_settings.vpnips_enabled && m_settings.vpnips_auto_download &&
            !m_settings.vpnips_url.empty())
        {
            std::string content;
            int const ret = HttpGetRequest(m_settings.vpnips_url.c_str(), content);
            if (ret > 0 && !content.empty())
            {
                std::string const filepath = m_settings.vpnips_file.empty() ?
                    "vpnips.txt" : m_settings.vpnips_file;

                std::ofstream ofs(filepath, std::ios::trunc);
                if (ofs.is_open())
                {
                    ofs << content;
                    ofs.close();
                    updated = true;
                }
            }
        }

        if (updated)
            ReloadFiles();

        return updated;
    }

    bool SpamBot::ContainsBadWord(const std::string& text) const
    {
        if (!m_settings.badwords_enabled)
            return false;
        return m_badwords.ContainsAny(text);
    }

    bool SpamBot::IsVpnAddress(const ACE_TString& ipaddr) const
    {
        if (!m_settings.vpnips_enabled)
            return false;
        return m_vpnips.IsVpnAddress(ipaddr);
    }

    bool SpamBot::CheckAbuseIPDB(const std::string& ipaddr)
    {
        if (!m_settings.abuseipdb_enabled)
            return false;
        return m_abuseipdb.CheckForReported(ipaddr);
    }

    void SpamBot::ReportToAbuseIPDB(const std::string& ipaddr,
                                    const std::string& comment)
    {
        if (!m_settings.abuseipdb_enabled)
            return;
        m_abuseipdb.Report(ipaddr, comment);
    }

    void SpamBot::IncLogin(const std::string& ipaddr)
    {
        if (m_settings.abuse_enabled)
            m_abuse.IncLogin(ipaddr);
    }

    void SpamBot::IncJoins(const std::string& ipaddr)
    {
        if (m_settings.abuse_enabled)
            m_abuse.IncJoins(ipaddr);
    }

    void SpamBot::IncKicks(const std::string& ipaddr)
    {
        if (m_settings.abuse_enabled)
            m_abuse.IncKicks(ipaddr);
    }

    bool SpamBot::CheckLoginAbuse(const std::string& ipaddr)
    {
        if (!m_settings.abuse_enabled)
            return false;
        return m_abuse.CheckLoginAbuse(ipaddr);
    }

    bool SpamBot::CheckJoinAbuse(const std::string& ipaddr)
    {
        if (!m_settings.abuse_enabled)
            return false;
        return m_abuse.CheckJoinAbuse(ipaddr);
    }

    bool SpamBot::CheckKickAbuse(const std::string& ipaddr)
    {
        if (!m_settings.abuse_enabled)
            return false;
        return m_abuse.CheckKickAbuse(ipaddr);
    }

    void SpamBot::CleanAbuseTrackers()
    {
        m_abuse.Clean();
    }

    int SpamBot::GetBanPrefix(const std::string& ipaddr) const
    {
        ACE_TString const addr = Utf8ToUnicode(ipaddr.c_str());
        int const family = InetAddrFamily(addr);

        if (family == AF_INET)
            return m_settings.ipv4_ban_prefix;
        else if (family == AF_INET6)
            return m_settings.ipv6_ban_prefix;

        return 0;
    }

    std::string SpamBot::ApplyBanPrefix(const std::string& ipaddr) const
    {
        int const prefix = GetBanPrefix(ipaddr);
        ACE_TString const addr = Utf8ToUnicode(ipaddr.c_str());
        int const family = InetAddrFamily(addr);

        if (family == AF_INET && prefix < 32 && prefix > 0)
            return ipaddr + "/" + std::to_string(prefix);
        else if (family == AF_INET6 && prefix < 128 && prefix > 0)
            return ipaddr + "/" + std::to_string(prefix);

        return ipaddr;
    }

} // namespace teamtalk

#endif /* ENABLE_TEAMTALKPRO */
