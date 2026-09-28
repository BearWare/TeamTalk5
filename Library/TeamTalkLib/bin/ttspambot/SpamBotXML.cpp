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

#include "SpamBotXML.h"

#include <tinyxml2.h>

namespace spambot {

    SpamBotXML::SpamBotXML()
        : teamtalk::XMLDocument("tt5spambot", TTSPAMBOT_XML_VERSION)
    {
    }

    bool SpamBotXML::SaveFile()
    {
        return teamtalk::XMLDocument::SaveFile();
    }

    /* <server> */

    std::string SpamBotXML::GetHostAddress(const std::string& def)
    {
        return GetValue(true, "server/hostaddr", def);
    }
    void SpamBotXML::SetHostAddress(const std::string& addr)
    {
        SetValue("server/hostaddr", addr);
    }

    int SpamBotXML::GetTcpPort(int def)
    {
        return GetValue(true, "server/tcpport", def);
    }
    void SpamBotXML::SetTcpPort(int port)
    {
        SetValue("server/tcpport", port);
    }

    int SpamBotXML::GetUdpPort(int def)
    {
        return GetValue(true, "server/udpport", def);
    }
    void SpamBotXML::SetUdpPort(int port)
    {
        SetValue("server/udpport", port);
    }

    bool SpamBotXML::GetEncrypted(bool def)
    {
        return GetValueBool(true, "server/encrypted", def);
    }
    void SpamBotXML::SetEncrypted(bool enabled)
    {
        SetValueBool("server/encrypted", enabled);
    }

    /* <account> */

    std::string SpamBotXML::GetUsername()
    {
        return GetValue(true, "account/username", "");
    }
    void SpamBotXML::SetUsername(const std::string& v)
    {
        SetValue("account/username", v);
    }

    std::string SpamBotXML::GetPassword()
    {
        return GetValue(true, "account/password", "");
    }
    void SpamBotXML::SetPassword(const std::string& v)
    {
        SetValue("account/password", v);
    }

    std::string SpamBotXML::GetNickname(const std::string& def)
    {
        return GetValue(true, "account/nickname", def);
    }
    void SpamBotXML::SetNickname(const std::string& v)
    {
        SetValue("account/nickname", v);
    }

    std::string SpamBotXML::GetStatusMessage()
    {
        return GetValue(true, "account/statusmsg", "");
    }
    void SpamBotXML::SetStatusMessage(const std::string& v)
    {
        SetValue("account/statusmsg", v);
    }

    Gender SpamBotXML::GetGender(Gender def)
    {
        int v = GetValue(true, "account/gender", static_cast<int>(def));
        if (v == GENDER_MALE || v == GENDER_FEMALE || v == GENDER_NEUTRAL)
            return static_cast<Gender>(v);
        return def;
    }
    void SpamBotXML::SetGender(Gender g)
    {
        SetValue("account/gender", static_cast<int>(g));
    }

    std::string SpamBotXML::GetInitChannel()
    {
        return GetValue(true, "account/initchannel", "");
    }
    void SpamBotXML::SetInitChannel(const std::string& path)
    {
        SetValue("account/initchannel", path);
    }

    /* <badwords> */

    bool SpamBotXML::GetBadWordsEnabled(bool def)
    {
        return GetValueBool(true, "badwords/enabled", def);
    }
    void SpamBotXML::SetBadWordsEnabled(bool enabled)
    {
        SetValueBool("badwords/enabled", enabled);
    }

    std::string SpamBotXML::GetBadWordsDir()      { return GetValue(true, "badwords/dir", ""); }
    void SpamBotXML::SetBadWordsDir(const std::string& dir) { SetValue("badwords/dir", dir); }

    /* <abuse> */

    bool SpamBotXML::GetAbuseEnabled(bool def)        { return GetValueBool(true, "abuse/enabled", def); }
    void SpamBotXML::SetAbuseEnabled(bool enabled)    { SetValueBool("abuse/enabled", enabled); }

    int  SpamBotXML::GetAbuseLoginCount(int def)      { return GetValue(true, "abuse/login-count", def); }
    void SpamBotXML::SetAbuseLoginCount(int v)        { SetValue("abuse/login-count", v); }

    int  SpamBotXML::GetAbuseJoinCount(int def)       { return GetValue(true, "abuse/join-count", def); }
    void SpamBotXML::SetAbuseJoinCount(int v)         { SetValue("abuse/join-count", v); }

    int  SpamBotXML::GetAbuseKickCount(int def)       { return GetValue(true, "abuse/kick-count", def); }
    void SpamBotXML::SetAbuseKickCount(int v)         { SetValue("abuse/kick-count", v); }

    int  SpamBotXML::GetAbuseDurationSecs(int def)    { return GetValue(true, "abuse/duration-secs", def); }
    void SpamBotXML::SetAbuseDurationSecs(int v)      { SetValue("abuse/duration-secs", v); }

    int  SpamBotXML::GetIPv4BanPrefix(int def)        { return GetValue(true, "abuse/ipv4-ban-prefix", def); }
    void SpamBotXML::SetIPv4BanPrefix(int v)          { SetValue("abuse/ipv4-ban-prefix", v); }

    int  SpamBotXML::GetIPv6BanPrefix(int def)        { return GetValue(true, "abuse/ipv6-ban-prefix", def); }
    void SpamBotXML::SetIPv6BanPrefix(int v)          { SetValue("abuse/ipv6-ban-prefix", v); }

    int  SpamBotXML::GetBanDurationSecs(int def)      { return GetValue(true, "abuse/ban-duration-secs", def); }
    void SpamBotXML::SetBanDurationSecs(int v)        { SetValue("abuse/ban-duration-secs", v); }

    /* <ipban> */

    bool SpamBotXML::GetIPBanEnabled(bool def)        { return GetValueBool(true, "ipban/enabled", def); }
    void SpamBotXML::SetIPBanEnabled(bool enabled)    { SetValueBool("ipban/enabled", enabled); }

    std::string SpamBotXML::GetVpnIpsFile()           { return GetValue(true, "ipban/vpnips-file", ""); }
    void SpamBotXML::SetVpnIpsFile(const std::string& path) { SetValue("ipban/vpnips-file", path); }

    /* <abuseipdb> */

    bool SpamBotXML::GetAbuseIPDBLookupEnabled(bool def)     { return GetValueBool(true, "abuseipdb/lookup-enabled", def); }
    void SpamBotXML::SetAbuseIPDBLookupEnabled(bool enabled) { SetValueBool("abuseipdb/lookup-enabled", enabled); }

    bool SpamBotXML::GetAbuseIPDBReportEnabled(bool def)     { return GetValueBool(true, "abuseipdb/report-enabled", def); }
    void SpamBotXML::SetAbuseIPDBReportEnabled(bool enabled) { SetValueBool("abuseipdb/report-enabled", enabled); }

    std::string SpamBotXML::GetAbuseIPDBApiKey()             { return GetValue(true, "abuseipdb/apikey", ""); }
    void SpamBotXML::SetAbuseIPDBApiKey(const std::string& key) { SetValue("abuseipdb/apikey", key); }

    int  SpamBotXML::GetAbuseIPDBTotalReports(int def)       { return GetValue(true, "abuseipdb/total-reports", def); }
    void SpamBotXML::SetAbuseIPDBTotalReports(int v)         { SetValue("abuseipdb/total-reports", v); }

    int  SpamBotXML::GetAbuseIPDBDistinctUsers(int def)      { return GetValue(true, "abuseipdb/distinct-users", def); }
    void SpamBotXML::SetAbuseIPDBDistinctUsers(int v)        { SetValue("abuseipdb/distinct-users", v); }

    int  SpamBotXML::GetAbuseIPDBConfidenceScore(int def)    { return GetValue(true, "abuseipdb/confidence-score", def); }
    void SpamBotXML::SetAbuseIPDBConfidenceScore(int v)      { SetValue("abuseipdb/confidence-score", v); }

    /* <logging> */

    bool SpamBotXML::GetLogEnabled(bool def)                 { return GetValueBool(true, "logging/enabled", def); }
    void SpamBotXML::SetLogEnabled(bool enabled)             { SetValueBool("logging/enabled", enabled); }

    /* <reconnect> */

    int  SpamBotXML::GetReconnectMaxAttempts(int def)        { return GetValue(true, "reconnect/max-attempts", def); }
    void SpamBotXML::SetReconnectMaxAttempts(int v)          { SetValue("reconnect/max-attempts", v); }

    int  SpamBotXML::GetReconnectIntervalSecs(int def)       { return GetValue(true, "reconnect/interval-secs", def); }
    void SpamBotXML::SetReconnectIntervalSecs(int v)         { SetValue("reconnect/interval-secs", v); }

} // namespace spambot
