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

#ifndef SPAMBOTXML_H
#define SPAMBOTXML_H

#include "settings/Settings.h"

#include <string>
#include <vector>

constexpr auto TTSPAMBOT_XML_VERSION = "5.16";

namespace spambot {

    enum Gender
    {
        GENDER_MALE    = 1,
        GENDER_FEMALE  = 2,
        GENDER_NEUTRAL = 3,
    };

    class SpamBotXML : public teamtalk::XMLDocument
    {
    public:
        SpamBotXML();
        bool SaveFile() override;

        /* <server> */
        std::string GetHostAddress(const std::string& def = "localhost");
        void SetHostAddress(const std::string& addr);

        int GetTcpPort(int def = 10333);
        void SetTcpPort(int port);

        int GetUdpPort(int def = 10333);
        void SetUdpPort(int port);

        bool GetEncrypted(bool def = false);
        void SetEncrypted(bool enabled);

        /* <account> */
        std::string GetUsername();
        void SetUsername(const std::string& v);

        std::string GetPassword();
        void SetPassword(const std::string& v);

        std::string GetNickname(const std::string& def = "SpamBot");
        void SetNickname(const std::string& v);

        std::string GetStatusMessage();
        void SetStatusMessage(const std::string& v);

        Gender GetGender(Gender def = GENDER_NEUTRAL);
        void SetGender(Gender g);

        std::string GetInitChannel();
        void SetInitChannel(const std::string& path);

        /* <badwords> */
        bool GetBadWordsEnabled(bool def = false);
        void SetBadWordsEnabled(bool enabled);

        std::string GetBadWordsDir();
        void SetBadWordsDir(const std::string& dir);

        /* <abuse> */
        bool GetAbuseEnabled(bool def = false);
        void SetAbuseEnabled(bool enabled);

        int GetAbuseLoginCount(int def = 10);
        void SetAbuseLoginCount(int v);

        int GetAbuseJoinCount(int def = 10);
        void SetAbuseJoinCount(int v);

        int GetAbuseKickCount(int def = 10);
        void SetAbuseKickCount(int v);

        int GetAbuseDurationSecs(int def = 60);
        void SetAbuseDurationSecs(int v);

        int GetIPv4BanPrefix(int def = 32);
        void SetIPv4BanPrefix(int v);

        int GetIPv6BanPrefix(int def = 128);
        void SetIPv6BanPrefix(int v);

        int GetBanDurationSecs(int def = 0);
        void SetBanDurationSecs(int v);

        /* <ipban> */
        bool GetIPBanEnabled(bool def = false);
        void SetIPBanEnabled(bool enabled);

        std::string GetVpnIpsFile();
        void SetVpnIpsFile(const std::string& path);

        /* <abuseipdb> */
        bool GetAbuseIPDBLookupEnabled(bool def = false);
        void SetAbuseIPDBLookupEnabled(bool enabled);

        bool GetAbuseIPDBReportEnabled(bool def = false);
        void SetAbuseIPDBReportEnabled(bool enabled);

        std::string GetAbuseIPDBApiKey();
        void SetAbuseIPDBApiKey(const std::string& key);

        int GetAbuseIPDBTotalReports(int def = 2);
        void SetAbuseIPDBTotalReports(int v);

        int GetAbuseIPDBDistinctUsers(int def = 2);
        void SetAbuseIPDBDistinctUsers(int v);

        int GetAbuseIPDBConfidenceScore(int def = 2);
        void SetAbuseIPDBConfidenceScore(int v);

        /* <logging> */
        bool GetLogEnabled(bool def = true);
        void SetLogEnabled(bool enabled);

        /* <reconnect> */
        int GetReconnectMaxAttempts(int def = 0);
        void SetReconnectMaxAttempts(int v);

        int GetReconnectIntervalSecs(int def = 10);
        void SetReconnectIntervalSecs(int v);
    };

} // namespace spambot

#endif // SPAMBOTXML_H
