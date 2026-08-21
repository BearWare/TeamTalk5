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

#include "SpamBotConfig.h"
#include "AppInfo.h"
#include "ServerUtil.h"
#include "TeamTalkDefs.h"

#include "myace/MyACE.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#if defined(WIN32) && defined(CreateFile)
#undef CreateFile
#endif

using namespace std;

namespace spambot {

    bool LoadConfig(SpamBotXML& xml, const ACE_TString& cfgfile)
    {
        std::string const path = UnicodeToUtf8(cfgfile).c_str();
        if (!xml.LoadFile(path))
            return xml.CreateFile(path);
        return true;
    }

    static const char* GenderName(Gender g)
    {
        switch (g)
        {
        case GENDER_MALE:    return "male";
        case GENDER_FEMALE:  return "female";
        case GENDER_NEUTRAL: return "neutral";
        }
        return "neutral";
    }

    static Gender PromptGender(Gender current)
    {
        cout << "Bot gender (1=male, 2=female, 3=neutral): ";
        int v = PrintGetInt(static_cast<int>(current));
        if (v != GENDER_MALE && v != GENDER_FEMALE && v != GENDER_NEUTRAL)
            v = GENDER_NEUTRAL;
        return static_cast<Gender>(v);
    }

    void RunWizard(SpamBotXML& xml)
    {
        cout << TEAMTALK_NAME << " " << TEAMTALK_VERSION_FRIENDLY << " configurator" << endl;
        cout << endl;

        cout << "Do you want to configure your " TEAMTALK_NAME "? ";
        if (!PrintGetBool(true))
            return;

        cout << endl;
        cout << "Configuring file: " << xml.GetFileName() << endl;

        std::string hostaddr   = xml.GetHostAddress();
        int         tcpport    = xml.GetTcpPort();
        int         udpport    = xml.GetUdpPort();
        bool        encrypted  = xml.GetEncrypted();

        std::string username   = xml.GetUsername();
        std::string password   = xml.GetPassword();
        std::string nickname   = xml.GetNickname();
        std::string statusmsg  = xml.GetStatusMessage();
        Gender      gender     = xml.GetGender();
        std::string initchan   = xml.GetInitChannel();

        bool        bw_enabled = xml.GetBadWordsEnabled();
        std::string bw_dir     = xml.GetBadWordsDir();

        bool        ab_enabled = xml.GetAbuseEnabled();
        int         ab_login   = xml.GetAbuseLoginCount();
        int         ab_join    = xml.GetAbuseJoinCount();
        int         ab_kick    = xml.GetAbuseKickCount();
        int         ab_window  = xml.GetAbuseDurationSecs();
        int         ipv4pref   = xml.GetIPv4BanPrefix();
        int         ipv6pref   = xml.GetIPv6BanPrefix();
        int         banduration = xml.GetBanDurationSecs();

        bool        ipban_enabled = xml.GetIPBanEnabled();
        std::string vpnips        = xml.GetVpnIpsFile();

        bool        adb_lookup    = xml.GetAbuseIPDBLookupEnabled();
        bool        adb_report    = xml.GetAbuseIPDBReportEnabled();
        std::string adb_key       = xml.GetAbuseIPDBApiKey();
        int         adb_total     = xml.GetAbuseIPDBTotalReports();
        int         adb_distinct  = xml.GetAbuseIPDBDistinctUsers();
        int         adb_score     = xml.GetAbuseIPDBConfidenceScore();

        int         rc_max        = xml.GetReconnectMaxAttempts();
        int         rc_interval   = xml.GetReconnectIntervalSecs();

        cout << endl;
        cout << "Ready to configure " << TEAMTALK_NAME << " settings." << endl;

#if !defined(WIN32)
        ACE_TCHAR* s = nullptr;
        int utf8_mode = 0;
        if ((((s = getenv("LC_ALL"))   != nullptr) && (*s != 0)) ||
            (((s = getenv("LC_CTYPE")) != nullptr) && (*s != 0)) ||
            (((s = getenv("LANG"))     != nullptr) && (*s != 0)))
        {
            if (strstr(s, "UTF-8") != nullptr)
                utf8_mode = 1;
        }
        if (utf8_mode == 0)
            cout << "Warning: UTF-8 not enabled. Please stick to English characters!" << endl;
#endif

        cout << endl;
        cout << "Target " TEAMTALK_NAME " host address: ";
        hostaddr = PrintGetString(hostaddr);
        cout << "Target " TEAMTALK_NAME " TCP port: ";
        tcpport = PrintGetInt(tcpport);
        cout << "Target " TEAMTALK_NAME " UDP port: ";
        udpport = PrintGetInt(udpport);
        cout << "Connect using encrypted connection: ";
        encrypted = PrintGetBool(encrypted);

        cout << endl << "Bot account credentials." << endl;

        cout << "Username for bot login: ";
        username = PrintGetString(username);
        cout << "Password for bot login: ";
        {
            std::string newpwd = PrintGetPassword(password);
            if (!newpwd.empty())
                password = newpwd;
        }
        cout << "Nickname displayed by bot: ";
        nickname = PrintGetString(nickname);
        cout << "Status message displayed by bot: ";
        statusmsg = PrintGetString(statusmsg);
        gender = PromptGender(gender);
        cout << "Initial channel path (leave empty to honor server account's init channel): ";
        initchan = PrintGetString(initchan);

        cout << endl << "Anti-spam features." << endl;

        cout << "Enable bad word filtering: ";
        bw_enabled = PrintGetBool(bw_enabled);
        if (bw_enabled)
        {
            cout << "Directory to scan for badwords*.txt files (leave empty for current directory): ";
            bw_dir = PrintGetString(bw_dir);
        }

        cout << "Enable abuse detection (login, join and kick rate per IP-address): ";
        ab_enabled = PrintGetBool(ab_enabled);
        if (ab_enabled)
        {
            cout << "Number of logins per IP-address before banning: ";
            ab_login = PrintGetInt(ab_login);
            cout << "Number of channel joins per IP-address before banning: ";
            ab_join = PrintGetInt(ab_join);
            cout << "Number of kicks per IP-address before banning: ";
            ab_kick = PrintGetInt(ab_kick);
            cout << "Time window in seconds for abuse counters: ";
            ab_window = PrintGetInt(ab_window);
            cout << "IPv4 ban CIDR prefix (32 = single host): ";
            ipv4pref = PrintGetInt(ipv4pref);
            cout << "IPv6 ban CIDR prefix (128 = single host): ";
            ipv6pref = PrintGetInt(ipv6pref);
            cout << "Duration of issued bans in seconds (0 = permanent): ";
            banduration = PrintGetInt(banduration);
        }

        cout << "Enable VPN IP-address blocking: ";
        ipban_enabled = PrintGetBool(ipban_enabled);
        if (ipban_enabled)
        {
            cout << "Path to file with VPN IP-addresses (one address or CIDR network per line): ";
            vpnips = PrintGetString(vpnips);
            if (vpnips.empty())
            {
                cout << "No VPN IP file specified. VPN blocking disabled." << endl;
                ipban_enabled = false;
            }
        }

        cout << "Enable AbuseIPDB IP-address reputation lookup: ";
        adb_lookup = PrintGetBool(adb_lookup);
        cout << "Enable AbuseIPDB auto-report of banned users: ";
        adb_report = PrintGetBool(adb_report);
        if (adb_lookup || adb_report)
        {
            cout << "AbuseIPDB API key: ";
            std::string newkey = PrintGetPassword(adb_key);
            if (!newkey.empty())
                adb_key = newkey;
        }
        if (adb_lookup)
        {
            cout << "Minimum total reports to flag IP-address: ";
            adb_total = PrintGetInt(adb_total);
            cout << "Minimum distinct users to flag IP-address: ";
            adb_distinct = PrintGetInt(adb_distinct);
            cout << "Minimum confidence score to flag IP-address: ";
            adb_score = PrintGetInt(adb_score);
        }

        cout << endl << "Connection stability." << endl;

        cout << "Maximum reconnect attempts (0 = retry forever): ";
        rc_max = PrintGetInt(rc_max);
        cout << "Seconds between reconnect attempts: ";
        rc_interval = PrintGetInt(rc_interval);

        cout << endl;
        cout << "Summary of " << TEAMTALK_NAME << " settings." << endl;
        cout << "Target server: " << hostaddr << " (TCP " << tcpport << ", UDP " << udpport << ")" << endl;
        cout << "Encrypted connection: " << (encrypted ? "true" : "false") << endl;
        cout << "Bot login: username \"" << username << "\", nickname \"" << nickname << "\", gender " << GenderName(gender) << endl;
        cout << "Initial channel: " << (initchan.empty() ? "(server account default)" : initchan) << endl;
        cout << "Bad word filtering: " << (bw_enabled ? "enabled" : "disabled");
        if (bw_enabled)
            cout << " (directory: " << (bw_dir.empty() ? "current directory" : bw_dir) << ")";
        cout << endl;
        cout << "Abuse detection: " << (ab_enabled ? "enabled" : "disabled");
        if (ab_enabled)
            cout << " (logins " << ab_login << ", joins " << ab_join << ", kicks " << ab_kick
                 << ", window " << ab_window << "s, ban " << banduration << "s)";
        cout << endl;
        cout << "VPN IP blocking: " << (ipban_enabled ? "enabled" : "disabled");
        if (ipban_enabled)
            cout << " (file: " << vpnips << ")";
        cout << endl;
        cout << "AbuseIPDB lookup: " << (adb_lookup ? "enabled" : "disabled") << endl;
        cout << "AbuseIPDB auto-report: " << (adb_report ? "enabled" : "disabled") << endl;
        cout << "Reconnect: " << (rc_max == 0 ? std::string("forever") : std::to_string(rc_max) + " attempts")
             << ", interval " << rc_interval << "s" << endl;

        cout << endl;
        cout << "Save these settings? ";
        if (PrintGetBool(true))
        {
            xml.SetHostAddress(hostaddr);
            xml.SetTcpPort(tcpport);
            xml.SetUdpPort(udpport);
            xml.SetEncrypted(encrypted);
            xml.SetUsername(username);
            xml.SetPassword(password);
            xml.SetNickname(nickname);
            xml.SetStatusMessage(statusmsg);
            xml.SetGender(gender);
            xml.SetInitChannel(initchan);
            xml.SetBadWordsEnabled(bw_enabled);
            xml.SetBadWordsDir(bw_dir);
            xml.SetAbuseEnabled(ab_enabled);
            xml.SetAbuseLoginCount(ab_login);
            xml.SetAbuseJoinCount(ab_join);
            xml.SetAbuseKickCount(ab_kick);
            xml.SetAbuseDurationSecs(ab_window);
            xml.SetIPv4BanPrefix(ipv4pref);
            xml.SetIPv6BanPrefix(ipv6pref);
            xml.SetBanDurationSecs(banduration);
            xml.SetIPBanEnabled(ipban_enabled);
            xml.SetVpnIpsFile(vpnips);
            xml.SetAbuseIPDBLookupEnabled(adb_lookup);
            xml.SetAbuseIPDBReportEnabled(adb_report);
            xml.SetAbuseIPDBApiKey(adb_key);
            xml.SetAbuseIPDBTotalReports(adb_total);
            xml.SetAbuseIPDBDistinctUsers(adb_distinct);
            xml.SetAbuseIPDBConfidenceScore(adb_score);
            xml.SetReconnectMaxAttempts(rc_max);
            xml.SetReconnectIntervalSecs(rc_interval);
            xml.SaveFile();

            cout << "Changes saved." << endl;
            cout << endl;
            cout << "If your " << TEAMTALK_NAME << " is currently running the settings will not take" << endl;
            cout << "effect until you restart it." << endl;
            cout << endl;
        }
        else
        {
            cout << "Changes discarded." << endl;
        }
    }

} // namespace spambot
