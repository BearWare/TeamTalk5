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

#ifndef SPAMBOT_SESSION_H
#define SPAMBOT_SESSION_H

#include "Abuse.h"
#include "AbuseDB.h"
#include "BadWords.h"
#include "IPBan.h"
#include "SpamBotXML.h"

#include <ace/OS.h>
#include <TeamTalk.h>

#include <chrono>
#include <map>
#include <memory>
#include <string>

namespace spambot {

    class SpamBotSession
    {
    public:
        explicit SpamBotSession(SpamBotXML& cfg);
        ~SpamBotSession();

        SpamBotSession(const SpamBotSession&) = delete;
        SpamBotSession& operator=(const SpamBotSession&) = delete;

        int Run();

        void Stop();

    private:
        enum class CmdComplete {
            NONE, LISTBANS, ADDBAN, ABUSE_BAN, ABUSE_KICK, ABUSE_TEXTMSG,
            LOGIN, REMOVECHANNEL, JOINCHANNEL, UNSUBSCRIBE
        };

        bool ConnectOnce();
        bool RunEventLoopOnce(int timeoutMs);
        void OnMessage(const TTMessage& msg);
        void RunTimedEvents();

        void OnConnectSuccess();
        void OnConnectFailed();
        void OnConnectionLost();

        void OnCmdMyselfLoggedIn(int userid, const UserAccount& acc);
        void OnCmdMyselfLoggedOut();
        void OnCmdUserLoggedIn(const User& u);
        void OnCmdUserLoggedOut(const User& u);
        void OnCmdUserJoinedChannel(const User& u);
        void OnCmdUserLeftChannel(int chanid, const User& u);
        void OnCmdUserUpdate(const User& u);
        void OnCmdUserTextMsg(const TextMessage& m);
        void OnCmdChannelNew(const Channel& c);
        void OnCmdChannelUpdate(const Channel& c);
        void OnCmdChannelRemove(const Channel& c);
        void OnCmdBannedUser(const BannedUser& b);
        void OnCmdProcessing(int cmdid, bool complete);
        void OnCmdError(int cmdid, const ClientErrorMsg& e);

        bool CleanUser(const User& u);
        bool CleanChannel(const Channel& c);
        bool CleanTextMessage(const TextMessage& m);

        void SendBadWordsNotify(int userid, const std::string& text);
        int  GetBanPrefix(const std::string& ipaddr) const;
        void AbuseBan(const User& u);
        void JoinInitChannel(const UserAccount& acc);
        void SyncBansOnLogin(const UserAccount& acc);

        SpamBotXML&  m_cfg;
        TTInstance*  m_tt = nullptr;
        bool         m_stopFlag = false;

        std::string  m_host;
        int          m_tcpPort = 10333, m_udpPort = 10333;
        bool         m_encrypted = false;

        std::string  m_username, m_password, m_nickname, m_statusMsg, m_initChannel;
        int          m_statusMode = 0;

        int          m_ipv4BanPrefix = 32, m_ipv6BanPrefix = 128;
        Abuse::Duration m_banDuration = std::chrono::seconds(0);

        std::unique_ptr<BadWords> m_badwords;
        std::unique_ptr<Abuse>    m_abuse;
        std::unique_ptr<IPBan>    m_ipban;
        std::unique_ptr<AbuseDB>  m_abusedb;

        std::map<int, Channel>      m_channels;
        std::map<int, User>         m_users;
        std::map<int, CmdComplete>  m_activeCmds;

        std::chrono::steady_clock::time_point m_syncTimeout;
        static constexpr std::chrono::seconds SYNC_INTERVAL{10};

        int  m_reconnectMaxAttempts = 0;
        int  m_reconnectAttempts = 0;
        std::chrono::seconds m_reconnectInterval{10};
    };

} // namespace spambot

#endif // SPAMBOT_SESSION_H
