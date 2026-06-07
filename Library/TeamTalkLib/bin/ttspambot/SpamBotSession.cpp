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

#include "SpamBotSession.h"

#include "teamtalk/Log.h"

#include <ace/Log_Msg.h>
#include <ace/OS_NS_stdio.h>
#include <ace/OS_NS_string.h>

#include <algorithm>
#include <cstring>
#include <sstream>
#include <thread>

namespace spambot {

    namespace {

        std::string Tstr(const TTCHAR* p)
        {
            if (!p) return std::string();
#if defined(UNICODE)
            std::wstring ws(p);
            return std::string(ws.begin(), ws.end());
#else
            return std::string(p);
#endif
        }

        void SetTstr(TTCHAR* dst, std::size_t cap, const std::string& src)
        {
#if defined(UNICODE)
            std::wstring ws(src.begin(), src.end());
            std::size_t n = std::min(ws.size(), cap - 1);
            std::copy_n(ws.begin(), n, dst);
            dst[n] = 0;
#else
            std::size_t n = std::min(src.size(), cap - 1);
            std::copy_n(src.begin(), n, dst);
            dst[n] = 0;
#endif
        }

        bool IsIPv6(const std::string& s)
        {
            return s.find(':') != std::string::npos;
        }

    } // anonymous namespace

    SpamBotSession::SpamBotSession(SpamBotXML& cfg) : m_cfg(cfg)
    {
        m_host        = cfg.GetHostAddress();
        m_tcpPort     = cfg.GetTcpPort();
        m_udpPort     = cfg.GetUdpPort();
        m_encrypted   = cfg.GetEncrypted();

        m_username    = cfg.GetUsername();
        m_password    = cfg.GetPassword();
        m_nickname    = cfg.GetNickname();
        m_statusMsg   = cfg.GetStatusMessage();
        m_initChannel = cfg.GetInitChannel();

        constexpr int STATUSMODE_FEMALE_BIT = 0x00000100;
        if (cfg.GetGender() == GENDER_FEMALE)
            m_statusMode = STATUSMODE_FEMALE_BIT;
        else
            m_statusMode = 0;

        m_ipv4BanPrefix = cfg.GetIPv4BanPrefix();
        m_ipv6BanPrefix = cfg.GetIPv6BanPrefix();
        m_banDuration   = Abuse::Duration(cfg.GetBanDurationSecs());

        if (cfg.GetBadWordsEnabled())
        {
            m_badwords = std::make_unique<BadWords>();
            m_badwords->LoadDirectory(cfg.GetBadWordsDir());
        }

        if (cfg.GetAbuseEnabled())
        {
            m_abuse = std::make_unique<Abuse>(cfg.GetAbuseLoginCount(),
                                              cfg.GetAbuseJoinCount(),
                                              cfg.GetAbuseKickCount(),
                                              Abuse::Duration(cfg.GetAbuseDurationSecs()));
        }

        if (cfg.GetIPBanEnabled())
        {
            auto networks = IPBan::LoadFile(cfg.GetVpnIpsFile());
            m_ipban = std::make_unique<IPBan>(std::move(networks), m_banDuration);
        }

        m_abusedb = std::make_unique<AbuseDB>(cfg.GetAbuseIPDBApiKey(),
                                              cfg.GetAbuseIPDBTotalReports(),
                                              cfg.GetAbuseIPDBDistinctUsers(),
                                              cfg.GetAbuseIPDBConfidenceScore(),
                                              cfg.GetAbuseIPDBLookupEnabled(),
                                              cfg.GetAbuseIPDBReportEnabled());

        m_reconnectMaxAttempts = cfg.GetReconnectMaxAttempts();
        m_reconnectInterval    = std::chrono::seconds(cfg.GetReconnectIntervalSecs());

        m_syncTimeout = std::chrono::steady_clock::now();
    }

    SpamBotSession::~SpamBotSession()
    {
        if (m_tt)
        {
            TT_Disconnect(m_tt);
            TT_CloseTeamTalk(m_tt);
            m_tt = nullptr;
        }
    }

    void SpamBotSession::Stop()
    {
        m_stopFlag = true;
    }

    int SpamBotSession::Run()
    {
        while (!m_stopFlag)
        {
            if (!ConnectOnce())
            {
                ++m_reconnectAttempts;
                if (m_reconnectMaxAttempts > 0 && m_reconnectAttempts >= m_reconnectMaxAttempts)
                {
                    ACE_TCHAR msg[256];
                    ACE_OS::snprintf(msg, 256, ACE_TEXT("Reconnect attempts exhausted (%d). Exiting."), m_reconnectAttempts);
                    TT_SYSLOG(msg);
                    return 1;
                }
                ACE_TCHAR msg[256];
                ACE_OS::snprintf(msg, 256, ACE_TEXT("Reconnect attempt %d in %d seconds."),
                                 m_reconnectAttempts, static_cast<int>(m_reconnectInterval.count()));
                TT_LOG(msg);
                std::this_thread::sleep_for(m_reconnectInterval);
                continue;
            }
            m_reconnectAttempts = 0;

            while (!m_stopFlag && RunEventLoopOnce(100))
            {
                RunTimedEvents();
            }
        }
        return 0;
    }

    bool SpamBotSession::ConnectOnce()
    {
        if (!m_tt)
            m_tt = TT_InitTeamTalkPoll();
        if (!m_tt)
        {
            TT_SYSLOG(ACE_TEXT("Failed to initialise TeamTalk client instance."));
            return false;
        }

        m_channels.clear();
        m_users.clear();
        m_activeCmds.clear();
        if (m_ipban)
            m_ipban->ClearServerBans();

#if defined(UNICODE)
        std::wstring whost(m_host.begin(), m_host.end());
        const TTCHAR* host = whost.c_str();
#else
        const TTCHAR* host = m_host.c_str();
#endif
        ACE_TCHAR msg[512];
        if (!TT_Connect(m_tt, host, m_tcpPort, m_udpPort, 0, 0, m_encrypted ? TRUE : FALSE))
        {
            ACE_OS::snprintf(msg, 512, ACE_TEXT("Failed to issue connect to %s:%d."), host, m_tcpPort);
            TT_SYSLOG(msg);
            return false;
        }
        ACE_OS::snprintf(msg, 512, ACE_TEXT("Connecting to %s:%d."), host, m_tcpPort);
        TT_LOG(msg);
        return true;
    }

    bool SpamBotSession::RunEventLoopOnce(int timeoutMs)
    {
        TTMessage msg{};
        INT32 wait = timeoutMs;
        if (TT_GetMessage(m_tt, &msg, &wait))
        {
            OnMessage(msg);
        }
        ClientFlags flags = TT_GetFlags(m_tt);
        if ((flags & CLIENT_CONNECTION) == CLIENT_CLOSED)
        {
            TT_LOG(ACE_TEXT("Connection closed. Reconnecting."));
            return false;
        }
        return true;
    }

    void SpamBotSession::OnMessage(const TTMessage& msg)
    {
        switch (msg.nClientEvent)
        {
        case CLIENTEVENT_CON_SUCCESS:           OnConnectSuccess(); break;
        case CLIENTEVENT_CON_FAILED:            OnConnectFailed();  break;
        case CLIENTEVENT_CON_LOST:              OnConnectionLost(); break;
        case CLIENTEVENT_CMD_MYSELF_LOGGEDIN:   OnCmdMyselfLoggedIn(msg.nSource, msg.useraccount); break;
        case CLIENTEVENT_CMD_MYSELF_LOGGEDOUT:  OnCmdMyselfLoggedOut(); break;
        case CLIENTEVENT_CMD_USER_LOGGEDIN:     OnCmdUserLoggedIn(msg.user); break;
        case CLIENTEVENT_CMD_USER_LOGGEDOUT:    OnCmdUserLoggedOut(msg.user); break;
        case CLIENTEVENT_CMD_USER_JOINED:       OnCmdUserJoinedChannel(msg.user); break;
        case CLIENTEVENT_CMD_USER_LEFT:         OnCmdUserLeftChannel(msg.nSource, msg.user); break;
        case CLIENTEVENT_CMD_USER_UPDATE:       OnCmdUserUpdate(msg.user); break;
        case CLIENTEVENT_CMD_USER_TEXTMSG:      OnCmdUserTextMsg(msg.textmessage); break;
        case CLIENTEVENT_CMD_CHANNEL_NEW:       OnCmdChannelNew(msg.channel); break;
        case CLIENTEVENT_CMD_CHANNEL_UPDATE:    OnCmdChannelUpdate(msg.channel); break;
        case CLIENTEVENT_CMD_CHANNEL_REMOVE:    OnCmdChannelRemove(msg.channel); break;
        case CLIENTEVENT_CMD_BANNEDUSER:        OnCmdBannedUser(msg.banneduser); break;
        case CLIENTEVENT_CMD_PROCESSING:        OnCmdProcessing(msg.nSource, msg.bActive == FALSE); break;
        case CLIENTEVENT_CMD_ERROR:             OnCmdError(msg.nSource, msg.clienterrormsg); break;
        default: break;
        }
    }

    void SpamBotSession::RunTimedEvents()
    {
        if ((TT_GetFlags(m_tt) & CLIENT_AUTHORIZED) == CLIENT_CLOSED)
            return;
        if (!m_activeCmds.empty())
            return;
        if (!m_ipban)
            return;

        auto now = std::chrono::steady_clock::now();
        if (now < m_syncTimeout)
            return;

        int bancmdid = m_ipban->SyncBans(m_tt);
        if (bancmdid > 0)
            m_activeCmds[bancmdid] = CmdComplete::ADDBAN;
        else
            m_syncTimeout = now + SYNC_INTERVAL;
    }

    void SpamBotSession::OnConnectSuccess()
    {
        ACE_TCHAR msg[512];
        ACE_OS::snprintf(msg, 512, ACE_TEXT("Connected to %C:%d."), m_host.c_str(), m_tcpPort);
        TT_LOG(msg);

#if defined(UNICODE)
        std::wstring wnick(m_nickname.begin(), m_nickname.end());
        std::wstring wuser(m_username.begin(), m_username.end());
        std::wstring wpwd(m_password.begin(), m_password.end());
        int cmd = TT_DoLogin(m_tt, wnick.c_str(), wuser.c_str(), wpwd.c_str());
#else
        int cmd = TT_DoLogin(m_tt, m_nickname.c_str(), m_username.c_str(), m_password.c_str());
#endif
        if (cmd > 0)
            m_activeCmds[cmd] = CmdComplete::LOGIN;
    }

    void SpamBotSession::OnConnectFailed()
    {
        ACE_TCHAR msg[512];
        ACE_OS::snprintf(msg, 512, ACE_TEXT("Failed to connect to %C:%d."), m_host.c_str(), m_tcpPort);
        TT_SYSLOG(msg);
        TT_Disconnect(m_tt);
    }

    void SpamBotSession::OnConnectionLost()
    {
        ACE_TCHAR msg[512];
        ACE_OS::snprintf(msg, 512, ACE_TEXT("Lost connection to %C:%d."), m_host.c_str(), m_tcpPort);
        TT_SYSLOG(msg);
        TT_Disconnect(m_tt);
    }

    void SpamBotSession::OnCmdMyselfLoggedIn(int userid, const UserAccount& acc)
    {
        ACE_TCHAR msg[256];
        ACE_OS::snprintf(msg, 256, ACE_TEXT("Logged in as user #%d."), userid);
        TT_LOG(msg);

        TT_DoChangeStatus(m_tt, m_statusMode,
#if defined(UNICODE)
            std::wstring(m_statusMsg.begin(), m_statusMsg.end()).c_str()
#else
            m_statusMsg.c_str()
#endif
        );

        JoinInitChannel(acc);
        SyncBansOnLogin(acc);
    }

    void SpamBotSession::JoinInitChannel(const UserAccount& acc)
    {
        std::string path = m_initChannel.empty() ? Tstr(acc.szInitChannel) : m_initChannel;
        if (path.empty())
            return;

#if defined(UNICODE)
        std::wstring wpath(path.begin(), path.end());
        int chanid = TT_GetChannelIDFromPath(m_tt, wpath.c_str());
#else
        int chanid = TT_GetChannelIDFromPath(m_tt, path.c_str());
#endif
        if (chanid <= 0)
        {
            ACE_TCHAR msg[512];
            ACE_OS::snprintf(msg, 512, ACE_TEXT("Initial channel not found: %C"), path.c_str());
            TT_SYSLOG(msg);
            return;
        }
        int cmd = TT_DoJoinChannelByID(m_tt, chanid, ACE_TEXT(""));
        if (cmd > 0)
            m_activeCmds[cmd] = CmdComplete::JOINCHANNEL;
    }

    void SpamBotSession::SyncBansOnLogin(const UserAccount& acc)
    {
        if (!m_ipban)
            return;
        if ((acc.uUserRights & USERRIGHT_BAN_USERS) == 0)
        {
            TT_SYSLOG(ACE_TEXT("Bot account lacks USERRIGHT_BAN_USERS. Ban synchronisation disabled."));
            return;
        }
        int cmd = TT_DoListBans(m_tt, 0, 0, 1000000);
        if (cmd > 0)
            m_activeCmds[cmd] = CmdComplete::LISTBANS;
    }

    void SpamBotSession::OnCmdMyselfLoggedOut()
    {
        TT_Disconnect(m_tt);
    }

    bool SpamBotSession::CleanUser(const User& u)
    {
        if (!m_badwords)
            return true;
        if (m_badwords->ContainsAny(Tstr(u.szNickname)))
            return false;
        if (m_badwords->ContainsAny(Tstr(u.szStatusMsg)))
            return false;
        return true;
    }

    bool SpamBotSession::CleanChannel(const Channel& c)
    {
        if (!m_badwords)
            return true;
        if (m_badwords->ContainsAny(Tstr(c.szName)))
            return false;
        if (m_badwords->ContainsAny(Tstr(c.szTopic)))
            return false;
        return true;
    }

    bool SpamBotSession::CleanTextMessage(const TextMessage& m)
    {
        if (!m_badwords)
            return true;
        if (m_badwords->ContainsAny(Tstr(m.szMessage)))
            return false;
        return true;
    }

    void SpamBotSession::SendBadWordsNotify(int userid, const std::string& text)
    {
        TextMessage tm{};
        tm.nMsgType = MSGTYPE_USER;
        tm.nToUserID = userid;
        SetTstr(tm.szMessage, TT_STRLEN, text);
        int cmd = TT_DoTextMessage(m_tt, &tm);
        if (cmd > 0)
            m_activeCmds[cmd] = CmdComplete::ABUSE_TEXTMSG;
    }

    int SpamBotSession::GetBanPrefix(const std::string& ipaddr) const
    {
        if (ipaddr.empty())
            return 0;
        if (IsIPv6(ipaddr))
            return m_ipv6BanPrefix == 128 ? 0 : m_ipv6BanPrefix;
        return m_ipv4BanPrefix == 32 ? 0 : m_ipv4BanPrefix;
    }

    void SpamBotSession::AbuseBan(const User& u)
    {
        BannedUser b{};
        b.uBanTypes = BANTYPE_IPADDR | BANTYPE_USERNAME;
        std::memcpy(b.szNickname,  u.szNickname,  sizeof(b.szNickname));
        std::memcpy(b.szUsername,  u.szUsername,  sizeof(b.szUsername));
        std::memcpy(b.szIPAddress, u.szIPAddress, sizeof(b.szIPAddress));

        std::string ip = Tstr(u.szIPAddress);
        int prefix = GetBanPrefix(ip);
        if (prefix > 0)
        {
            std::string withPrefix = ip + "/" + std::to_string(prefix);
            SetTstr(b.szIPAddress, TT_STRLEN, withPrefix);
        }

        int cmd = TT_DoBan(m_tt, &b);
        if (cmd > 0)
        {
            m_activeCmds[cmd] = CmdComplete::ABUSE_BAN;
            if (m_ipban)
                m_ipban->AddLocalBan(b, m_tt);
        }

        SendBadWordsNotify(u.nUserID, "You have been banned due to abuse");

        cmd = TT_DoKickUser(m_tt, u.nUserID, 0);
        if (cmd > 0)
            m_activeCmds[cmd] = CmdComplete::ABUSE_KICK;

        if (m_abusedb)
            m_abusedb->Report(Tstr(u.szIPAddress), "Spam");
    }

    void SpamBotSession::OnCmdUserLoggedIn(const User& u)
    {
        m_users[u.nUserID] = u;
        std::string ip = Tstr(u.szIPAddress);

        if (u.nUserID == TT_GetMyUserID(m_tt))
        {
            if (m_abusedb)
                m_abusedb->AddWhiteListIPAddr(ip);
            return;
        }

        if (!ip.empty() && m_abusedb && m_abusedb->CheckForReported(ip))
        {
            SendBadWordsNotify(u.nUserID, "Your IP-address is listed as a spammer");
            int cmd = TT_DoKickUser(m_tt, u.nUserID, 0);
            if (cmd > 0) m_activeCmds[cmd] = CmdComplete::ABUSE_KICK;
            ACE_TCHAR msg[512];
            ACE_OS::snprintf(msg, 512, ACE_TEXT("Kicking %C (%C) as listed spammer."),
                             Tstr(u.szNickname).c_str(), ip.c_str());
            TT_LOG(msg);
            if (m_abuse) m_abuse->IncKicks(ip);
        }
        else if (!CleanUser(u))
        {
            SendBadWordsNotify(u.nUserID, "Your nick name and/or status message contains foul language");
            int cmd = TT_DoKickUser(m_tt, u.nUserID, 0);
            if (cmd > 0) m_activeCmds[cmd] = CmdComplete::ABUSE_KICK;
            if (m_abuse) m_abuse->IncKicks(ip);
        }

        if (m_abuse)
        {
            m_abuse->IncLogin(ip);
            if (m_abuse->CheckLoginAbuse(ip))
                AbuseBan(u);
            else if (m_abuse->CheckKickAbuse(ip))
                AbuseBan(u);
        }
    }

    void SpamBotSession::OnCmdUserLoggedOut(const User& u)
    {
        m_users.erase(u.nUserID);
    }

    void SpamBotSession::OnCmdUserJoinedChannel(const User& u)
    {
        m_users[u.nUserID] = u;
        std::string ip = Tstr(u.szIPAddress);

        if (!CleanUser(u))
        {
            SendBadWordsNotify(u.nUserID, "Your nick name and/or status message contains foul language");
            int cmd = TT_DoKickUser(m_tt, u.nUserID, 0);
            if (cmd > 0) m_activeCmds[cmd] = CmdComplete::ABUSE_KICK;
            if (m_abuse) m_abuse->IncKicks(ip);
        }

        if (u.nChannelID == TT_GetMyChannelID(m_tt))
        {
            int cmd = TT_DoUnsubscribe(m_tt, u.nUserID,
                SUBSCRIBE_VOICE | SUBSCRIBE_DESKTOP | SUBSCRIBE_VIDEOCAPTURE | SUBSCRIBE_MEDIAFILE);
            if (cmd > 0) m_activeCmds[cmd] = CmdComplete::UNSUBSCRIBE;
        }

        if (m_abuse)
        {
            m_abuse->IncJoins(ip);
            if (m_abuse->CheckJoinAbuse(ip))
                AbuseBan(u);
        }
    }

    void SpamBotSession::OnCmdUserLeftChannel(int chanid, const User& u)
    {
        m_users[u.nUserID] = u;
    }

    void SpamBotSession::OnCmdUserUpdate(const User& u)
    {
        m_users[u.nUserID] = u;
        if (!CleanUser(u))
        {
            SendBadWordsNotify(u.nUserID, "Your nick name and/or status message contains foul language");
            int cmd = TT_DoKickUser(m_tt, u.nUserID, 0);
            if (cmd > 0) m_activeCmds[cmd] = CmdComplete::ABUSE_KICK;
            if (m_abuse) m_abuse->IncKicks(Tstr(u.szIPAddress));
        }
    }

    void SpamBotSession::OnCmdUserTextMsg(const TextMessage& m)
    {
        if (!CleanTextMessage(m))
        {
            SendBadWordsNotify(m.nFromUserID, "Your text message contains foul language");
            int cmd = TT_DoKickUser(m_tt, m.nFromUserID, 0);
            if (cmd > 0) m_activeCmds[cmd] = CmdComplete::ABUSE_KICK;

            auto it = m_users.find(m.nFromUserID);
            if (it != m_users.end() && m_abuse)
                m_abuse->IncKicks(Tstr(it->second.szIPAddress));
        }
    }

    void SpamBotSession::OnCmdChannelNew(const Channel& c)
    {
        m_channels[c.nChannelID] = c;
        if (!CleanChannel(c))
        {
            int cmd = TT_DoRemoveChannel(m_tt, c.nChannelID);
            if (cmd > 0) m_activeCmds[cmd] = CmdComplete::REMOVECHANNEL;
        }
    }

    void SpamBotSession::OnCmdChannelUpdate(const Channel& c)
    {
        m_channels[c.nChannelID] = c;
        if (!CleanChannel(c) && c.nParentID != 0)
        {
            int cmd = TT_DoRemoveChannel(m_tt, c.nChannelID);
            if (cmd > 0) m_activeCmds[cmd] = CmdComplete::REMOVECHANNEL;
        }
    }

    void SpamBotSession::OnCmdChannelRemove(const Channel& c)
    {
        m_channels.erase(c.nChannelID);
    }

    void SpamBotSession::OnCmdBannedUser(const BannedUser& b)
    {
        if (m_ipban)
            m_ipban->AddRemoteBan(b);
    }

    void SpamBotSession::OnCmdProcessing(int cmdid, bool complete)
    {
        if (!complete)
            return;
        auto it = m_activeCmds.find(cmdid);
        if (it == m_activeCmds.end())
            return;
        switch (it->second)
        {
        case CmdComplete::LISTBANS:
        case CmdComplete::ADDBAN:
            if (m_ipban)
            {
                int bancmdid = m_ipban->SyncBans(m_tt);
                if (bancmdid > 0)
                    m_activeCmds[bancmdid] = CmdComplete::ADDBAN;
            }
            break;
        default:
            break;
        }
        m_activeCmds.erase(it);
    }

    void SpamBotSession::OnCmdError(int cmdid, const ClientErrorMsg& e)
    {
        ACE_TCHAR msg[512];
        ACE_OS::snprintf(msg, 512, ACE_TEXT("Command #%d failed: %C"), cmdid, Tstr(e.szErrorMsg).c_str());
        TT_SYSLOG(msg);
        m_activeCmds.erase(cmdid);
    }

} // namespace spambot
