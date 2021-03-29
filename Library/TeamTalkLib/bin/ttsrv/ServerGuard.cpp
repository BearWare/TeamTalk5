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

#include "ServerGuard.h"
#include "ServerConfig.h"

#include <ace/OS_NS_sys_stat.h>

#include <sstream>
#include <regex>

#include <teamtalk/Commands.h>
#include <teamtalk/Log.h>

#include "AppInfo.h"

using namespace teamtalk;
using namespace std;

#if defined(UNICODE)
typedef std::wostringstream tostringstream;
#else
typedef std::ostringstream tostringstream;
#endif

ACE_TString LogPrepare(const ACE_TString& str)
{
    return PrepareString(str);
}

ServerGuard::ServerGuard(ServerXML& settings)
  : m_settings(settings)
{
}

void ServerGuard::OnUserConnected(const ServerUser& user)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" TCP address: ") << user.GetIpAddress().c_str() << ACE_TEXT(" connected.");
    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserLogin(const ServerUser& user)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(user.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("IP address: ") << user.GetIpAddress().c_str() << ACE_TEXT(" ");
    oss << ACE_TEXT("UDP address: ") << user.GetUdpAddress().get_host_addr() << ACE_TEXT(":");
    oss << user.GetUdpAddress().get_port_number() << ACE_TEXT(" logged in.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserAuthFailed(const ServerUser& user, const ACE_TString& username)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("failed to authenticate for account \"") << LogPrepare(username).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("IP address: ") << user.GetIpAddress().c_str() << ACE_TEXT(" ");
    oss << ACE_TEXT("UDP address: ") << user.GetUdpAddress().get_host_addr() << ACE_TEXT(":");
    oss << user.GetUdpAddress().get_port_number() << ACE_TEXT(".");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserLoginBanned(const ServerUser& user)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(user.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("IP address: ") << user.GetIpAddress().c_str() << ACE_TEXT(" ");
    oss << ACE_TEXT("UDP address: ") << user.GetUdpAddress().get_host_addr() << ACE_TEXT(":");
    oss << user.GetUdpAddress().get_port_number() << ACE_TEXT(" denied login due to banned IP-address.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserLoggedOut(const ServerUser& user)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(user.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("logged out.");

    TT_LOG(oss.str().c_str());

#if defined(ENABLE_TEAMTALKPRO)
    m_pendinglogin.erase(user.GetUserID());
#endif
}

void ServerGuard::OnUserDisconnected(const ServerUser& user)
{
    tostringstream oss;

    serverchannel_t chan = user.GetChannel();
    if(chan)
    {
        oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
        oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
        if(user.GetUsername().length())
            oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
        oss << ACE_TEXT("Channel: \"") << LogPrepare(chan->GetChannelPath()).c_str() << ACE_TEXT("\" ");
        oss << ACE_TEXT("disconnected.");
    }
    else
    {
        oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
        oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
        if(user.GetUsername().length())
            oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
        oss << ACE_TEXT("disconnected.");
    }
    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserDropped(const ServerUser& user)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(user.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("dropped after ") << user.GetLastKeepAlive() << ACE_TEXT(" sec for inactivity.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserKicked(const ServerUser& kickee, 
                               const ServerUser* kicker, 
                               const ServerChannel* channel)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << kickee.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(kickee.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(kickee.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(kickee.GetUsername()).c_str() << ACE_TEXT("\" ");
    if(kicker)
    {
        oss << ACE_TEXT("kicked by #") << kicker->GetUserID() << ACE_TEXT(" ");
        oss << ACE_TEXT("nickname: \"") << LogPrepare(kicker->GetNickname()).c_str() << ACE_TEXT("\" ");
        if(channel)
            oss << ACE_TEXT("from channel: \"") << LogPrepare(channel->GetChannelPath()).c_str() << ACE_TEXT("\".");
        else
            oss << ACE_TEXT("from server.");
    }
    else
    {
        if(channel)
            oss << ACE_TEXT("kicked from channel: \"") << LogPrepare(channel->GetChannelPath()).c_str() << ACE_TEXT("\".");
        else
            oss << ACE_TEXT("kick from server.");
    }

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserBanned(const ServerUser& banee, const ServerUser& banner)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << banner.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(banner.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(banner.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(banner.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("banned user #") << banee.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("IP address: ") << LogPrepare(banee.GetIpAddress()).c_str() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(banee.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(banee.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(banee.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("from server.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserBanned(const ACE_TString& ipaddr, const ServerUser& banner)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << banner.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(banner.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(banner.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(banner.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("banned ");
    oss << ACE_TEXT("IP address: ") << LogPrepare(ipaddr).c_str() << ACE_TEXT(" ");
    oss << ACE_TEXT("from server.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserBanned(const ServerUser& banner, const BannedUser& ban)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << banner.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(banner.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(banner.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(banner.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("banned ");

    if(ban.bantype & BANTYPE_IPADDR)
        oss << ACE_TEXT("IP address: ") << LogPrepare(ban.ipaddr).c_str() << ACE_TEXT(" ");
    if(ban.bantype & BANTYPE_USERNAME)
        oss << ACE_TEXT("username: \"") << LogPrepare(ban.username).c_str() << ACE_TEXT("\" ");

    if(ban.bantype & BANTYPE_CHANNEL)
        oss << ACE_TEXT("from channel \"") << LogPrepare(ban.chanpath).c_str() << ACE_TEXT("\".");
    else
        oss << ACE_TEXT("from server.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserUnbanned(const ServerUser& user, const BannedUser& ban)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(user.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("unbanned IP address: ") << ban.ipaddr.c_str() << ACE_TEXT(".");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserUpdated(const ServerUser& user)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(user.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("status: ") << user.GetStatusMode() << ACE_TEXT(" ");
    oss << ACE_TEXT("status msg: \"") << LogPrepare(user.GetStatusMessage()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("UDP address: ") << user.GetUdpAddress().get_host_addr() << ACE_TEXT(":");
    oss << user.GetUdpAddress().get_port_number() << ACE_TEXT(" updated.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserJoinChannel(const ServerUser& user, const ServerChannel& channel)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(user.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("joined channel: \"") << LogPrepare(channel.GetChannelPath()).c_str() << ACE_TEXT("\".");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserLeaveChannel(const ServerUser& user, const ServerChannel& channel)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(user.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("left channel: \"") << LogPrepare(channel.GetChannelPath()).c_str() << ACE_TEXT("\".");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserMoved(const ServerUser& mover, const ServerUser& movee)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << mover.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(mover.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(mover.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(mover.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("moved user #") << movee.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(movee.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(movee.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(movee.GetUsername()).c_str() << ACE_TEXT("\".");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserMessage(const ServerUser& from, const ServerUser& to, const teamtalk::TextMessage& msg)
{
    TTASSERT(msg.msgType == TTUserMsg);
    if(msg.msgType == TTUserMsg)
    {
        tostringstream oss;
        oss << ACE_TEXT("User message from #") << from.GetUserID() << ACE_TEXT(" ");
        oss << ACE_TEXT("nickname: \"") << LogPrepare(from.GetNickname()).c_str() << ACE_TEXT("\" ");
        if(from.GetUsername().length())
            oss << ACE_TEXT("username: \"") << LogPrepare(from.GetUsername()).c_str() << ACE_TEXT("\" ");
        oss << ACE_TEXT("to #") << to.GetUserID() << ACE_TEXT(" ");
        oss << ACE_TEXT("nickname: \"") << LogPrepare(to.GetNickname()).c_str() << ACE_TEXT("\" ");
        if(to.GetUsername().length())
            oss << ACE_TEXT("username: \"") << LogPrepare(to.GetUsername()).c_str() << ACE_TEXT("\" ");
        oss << ACE_TEXT("content: \"") << LogPrepare(msg.content).c_str() << ACE_TEXT("\".");

        TT_LOG(oss.str().c_str());
    }
}

void ServerGuard::OnChannelMessage(const ServerUser& from, const ServerChannel& channel, const teamtalk::TextMessage& msg)
{
    TTASSERT(msg.msgType == TTChannelMsg);
    if(msg.msgType == TTChannelMsg)
    {
        tostringstream oss;
        oss << ACE_TEXT("Channel message from #") << from.GetUserID() << ACE_TEXT(" ");
        oss << ACE_TEXT("nickname: \"") << LogPrepare(from.GetNickname()).c_str() << ACE_TEXT("\" ");
        if(from.GetUsername().length())
            oss << ACE_TEXT("username: \"") << LogPrepare(from.GetUsername()).c_str() << ACE_TEXT("\" ");
        oss << ACE_TEXT("to channel: \"") << LogPrepare(channel.GetChannelPath()).c_str() << ACE_TEXT("\" ");
        oss << ACE_TEXT("content: \"") << LogPrepare(msg.content).c_str() << ACE_TEXT("\".");

        TT_LOG(oss.str().c_str());
    }
}

void ServerGuard::OnBroadcastMessage(const ServerUser& from, const teamtalk::TextMessage& msg)
{
    TTASSERT(msg.msgType == TTBroadcastMsg);
    tostringstream oss;
    oss << ACE_TEXT("Broadcast message from #") << from.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("content: \"") << LogPrepare(msg.content).c_str() << ACE_TEXT("\".");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnCustomMessage(const ServerUser& from, const ServerUser& to, const teamtalk::TextMessage& msg)
{
    //if(msg.msgType == TTCustomMsg)
    //{
    //    tostringstream oss;
    //    oss << ACE_TEXT("User custom message from #") << from.GetUserID() << ACE_TEXT(" ");
    //    oss << ACE_TEXT("nickname: \"") << LogPrepare(from.GetNickname()).c_str() << ACE_TEXT("\" ");
    //    if(from.GetUsername().length())
    //        oss << ACE_TEXT("username: \"") << LogPrepare(from.GetUsername()).c_str() << ACE_TEXT("\" ");
    //    oss << ACE_TEXT("to #") << to.GetUserID() << ACE_TEXT(" ");
    //    oss << ACE_TEXT("nickname: \"") << LogPrepare(to.GetNickname()).c_str() << ACE_TEXT("\" ");
    //    if(to.GetUsername().length())
    //        oss << ACE_TEXT("username: \"") << LogPrepare(to.GetUsername()).c_str() << ACE_TEXT("\" ");
    //    oss << ACE_TEXT("content: \"") << LogPrepare(msg.content) << ACE_TEXT("\".");

    //    TT_LOG(oss.str().c_str());
    //}
}

void ServerGuard::OnChannelCreated(const ServerChannel& channel, 
                                   const ServerUser* user/* = NULL*/)
{
    if(!user)
        return;

    tostringstream oss;
    oss << ACE_TEXT("Channel #") << channel.GetChannelID() << ACE_TEXT(" ");
    if(user)
    {
        oss << ACE_TEXT("\"") << LogPrepare(channel.GetChannelPath()).c_str() << ACE_TEXT("\" created by ");
        oss << ACE_TEXT("nickname: \"") << LogPrepare(user->GetNickname()).c_str() << ACE_TEXT("\" ");
        if(user->GetUsername().length())
            oss << ACE_TEXT("username: \"") << LogPrepare(user->GetUsername()).c_str() << ACE_TEXT("\".");
    }
    else
        oss << ACE_TEXT("\"") << LogPrepare(channel.GetChannelPath()).c_str() << ACE_TEXT("\" created.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnChannelUpdated(const ServerChannel& channel, 
                                   const ServerUser* user/* = NULL*/)
{
    if(!user)
        return;

    tostringstream oss;
    oss << ACE_TEXT("Channel #") << channel.GetChannelID() << ACE_TEXT(" ");
    if(user)
    {
        oss << ACE_TEXT("\"") << LogPrepare(channel.GetChannelPath()).c_str() << ACE_TEXT("\" updated by ");
        oss << ACE_TEXT("nickname: \"") << LogPrepare(user->GetNickname()).c_str() << ACE_TEXT("\" ");
        if(user->GetUsername().length())
            oss << ACE_TEXT("username: \"") << LogPrepare(user->GetUsername()).c_str() << ACE_TEXT("\".");
    }
    else
        oss << ACE_TEXT("\"") << LogPrepare(channel.GetChannelPath()).c_str() << ACE_TEXT("\" updated.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnChannelRemoved(const ServerChannel& channel, 
                                   const ServerUser* user/* = NULL*/)
{
    if(!user)
        return;

    tostringstream oss;
    oss << ACE_TEXT("Channel #") << channel.GetChannelID() << ACE_TEXT(" ");
    if(user)
    {
        oss << ACE_TEXT("\"") << LogPrepare(channel.GetChannelPath()).c_str() << ACE_TEXT("\" removed by ");
        oss << ACE_TEXT("nickname: \"") << LogPrepare(user->GetNickname()).c_str() << ACE_TEXT("\" ");
        if(user->GetUsername().length())
            oss << ACE_TEXT("username: \"") << LogPrepare(user->GetUsername()).c_str() << ACE_TEXT("\".");
    }
    else
        oss << ACE_TEXT("\"") << LogPrepare(channel.GetChannelPath()).c_str() << ACE_TEXT("\" removed.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnFileUploaded(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(user.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("uploaded file: \"") << LogPrepare(file.filename).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("to channel: \"") << LogPrepare(chan.GetChannelPath()).c_str() << ACE_TEXT("\".");
    TT_LOG(oss.str().c_str()); 
}

void ServerGuard::OnFileDownloaded(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(user.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("downloaded file: \"") << LogPrepare(file.filename).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("from channel: \"") << LogPrepare(chan.GetChannelPath()).c_str() << ACE_TEXT("\".");
    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnFileDeleted(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(user.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("deleted file: \"") << LogPrepare(file.filename).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("from channel: \"") << LogPrepare(chan.GetChannelPath()).c_str() << ACE_TEXT("\".");
    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnServerUpdated(const ServerUser& user, const ServerSettings& srvprop)
{
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(user.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("updated server properties.");
    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnSaveConfiguration(ServerNode& servernode, const ServerUser* user = NULL)
{
    ServerSettings properties = servernode.GetServerProperties();

    m_settings.SetServerName(UnicodeToUtf8(properties.servername).c_str());
    m_settings.SetMessageOfTheDay(UnicodeToUtf8(properties.motd).c_str());
    m_settings.SetAutoSave(properties.autosave);
    m_settings.SetMaxUsers(properties.maxusers);
    m_settings.SetMaxLoginAttempts(properties.maxloginattempts);
    m_settings.SetMaxLoginsPerIP(properties.max_logins_per_ipaddr);
    m_settings.SetLoginDelay(properties.logindelay);
    m_settings.SetUserTimeout(properties.usertimeout);
    m_settings.SetVoiceTxLimit(properties.voicetxlimit);
    m_settings.SetVideoCaptureTxLimit(properties.videotxlimit);
    m_settings.SetMediaFileTxLimit(properties.mediafiletxlimit);
    m_settings.SetDesktopTxLimit(properties.desktoptxlimit);
    m_settings.SetTotalTxLimit(properties.totaltxlimit);
    TTASSERT(properties.tcpaddrs.size());
    if (properties.tcpaddrs.size())
        m_settings.SetHostTcpPort(properties.tcpaddrs[0].get_port_number());
    TTASSERT(properties.udpaddrs.size());
    if (properties.udpaddrs.size())
        m_settings.SetHostUdpPort(properties.udpaddrs[0].get_port_number());

    m_settings.SetMaxDiskUsage(properties.maxdiskusage);
    m_settings.SetDefaultDiskQuota(properties.diskquota);
    m_settings.SetFilesRoot(UnicodeToUtf8(properties.filesroot).c_str());

    teamtalk::statchannels_t channels;
    ConvertChannels(servernode.GetRootChannel(), channels, true);
    m_settings.SetStaticChannels(channels);

    m_settings.SaveFile();

    tostringstream oss;
    oss << ACE_TEXT("Server configuration saved");
    if(user)
    {
        oss << ACE_TEXT(" due to user #") << user->GetUserID() << ACE_TEXT(" ");
        oss << ACE_TEXT("nickname: \"") << LogPrepare(user->GetNickname()).c_str() << ACE_TEXT("\" ");
        if(user->GetUsername().length())
            oss << ACE_TEXT("username: \"") << LogPrepare(user->GetUsername()).c_str() << ACE_TEXT("\"");
    }
    oss << ACE_TEXT(".");
    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnShutdown(const ServerStats& stats)
{
    tostringstream oss;
    oss << ACE_TEXT("Data transferred - ");
    oss << ACE_TEXT("Total TX: ") << stats.total_bytessent / 1024 << ACE_TEXT(" KBytes ");
    oss << ACE_TEXT("Total RX: ") << stats.total_bytesreceived / 1024 << ACE_TEXT(" KBytes ");
    oss << ACE_TEXT("Voice TX: ") << stats.voice_bytessent / 1024 << ACE_TEXT(" KBytes ");
    oss << ACE_TEXT("Voice RX: ") << stats.voice_bytesreceived / 1024 << ACE_TEXT(" KBytes ");
    oss << ACE_TEXT("Desktop TX: ") << stats.desktop_bytessent / 1024 << ACE_TEXT(" KBytes ");
    oss << ACE_TEXT("Desktop RX: ") << stats.desktop_bytesreceived / 1024 << ACE_TEXT(" KBytes ");
    oss << ACE_TEXT("Media Files TX: ") << stats.mediafile_bytessent / 1024 << ACE_TEXT(" KBytes ");
    oss << ACE_TEXT("Media Files RX: ") << stats.mediafile_bytesreceived / 1024 << ACE_TEXT(" KBytes ");
    oss << ACE_TEXT("Video TX: ") << stats.vidcap_bytessent / 1024 << ACE_TEXT(" KBytes/sec ");
    oss << ACE_TEXT("Video RX: ") << stats.vidcap_bytesreceived / 1024 << ACE_TEXT(" KBytes/sec.");
    TT_LOG(oss.str().c_str());
}

#if defined(ENABLE_TEAMTALKPRO)

void ServerGuard::WebLoginBearWare(ServerNode* servernode, ACE_UINT32 userid, UserAccount useraccount)
{
    ErrorMsg err;
    std::string authusername = UnicodeToUtf8(useraccount.username).c_str();
    std::string authtoken;

    {
        GUARD_OBJ_NAME(g, servernode, servernode->lock());

        auto user = servernode->GetUser(userid, nullptr);
        if (user.get())
        {
            authtoken = UnicodeToUtf8(user->GetAccessToken()).c_str();
        }

        if (authtoken.empty())
        {
            err = TT_CMDERR_INVALID_ACCOUNT;
            MYTRACE(ACE_TEXT("Authentication token is empty\n"));
            WebLoginComplete(servernode, userid, useraccount, err);
            return;
        }

        // First try specific weblogin account and afterwards try
        // 'bearware' weblogin
        UserAccount copyaccount;
        copyaccount.username = useraccount.username;
        if (!m_settings.AuthenticateUser(copyaccount))
        {
            UserAccount sharedaccount;
            sharedaccount.username = ACE_TEXT(WEBLOGIN_BEARWARE_USERNAME);
            if (!m_settings.AuthenticateUser(sharedaccount))
            {
                err.errorno = TT_CMDERR_INVALID_ACCOUNT;
                WebLoginComplete(servernode, userid, useraccount, err);
                MYTRACE(ACE_TEXT("Shared 'bearware' account doesn't exist\n"));
                return;
            }

            // Apply 'sharedaccount' to 'useraccount'
            auto un = useraccount.username;
            useraccount = sharedaccount;
            useraccount.username = un;
        }
        else
        {
            // User account properties now applied to 'copyaccount'.
            useraccount = copyaccount;
        }
    }


    ACE_CString url = WEBLOGIN_URL;
    url += "client=" TEAMTALK_LIB_NAME;
    url += "&version=" TEAMTALK_VERSION;
    url += "&service=bearware";
    url += "&action=serverauth";
    url += ACE_CString("&username=") + authusername.c_str();
    url += ACE_CString("&accesstoken=") + authtoken.c_str();

    MYTRACE(ACE_TEXT("Performing HTTP web authentication of %s\n"), useraccount.username.c_str());
    std::string utf8;
    int ret = HttpRequest(url, utf8);

    GUARD_OBJ_NAME(g, servernode, servernode->lock()); // lock required by WebLoginPostAuthenticate() and WebLoginComplete()

    MYTRACE(ACE_TEXT("HTTP response code: %d\n"), ret);
    switch(ret)
    {
    default :
    case -1:
        err = ErrorMsg(TT_CMDERR_LOGINSERVICE_UNAVAILABLE);
        break;
    case 0:
         err = ErrorMsg(TT_CMDERR_INVALID_ACCOUNT);
        break;
    case 1:
        teamtalk::XMLDocument xmldoc("teamtalk", "1.0");
        if(xmldoc.Parse(utf8))
        {
            std::string nickname = xmldoc.GetValue(false, "teamtalk/bearware/nickname", "");
            std::string username = xmldoc.GetValue(false, "teamtalk/bearware/username", "");
#if defined(UNICODE)
            useraccount.nickname = Utf8ToUnicode(nickname.c_str());
#else
            useraccount.nickname = nickname.c_str();
#endif
            TTASSERT(authusername == username.c_str());
            err = WebLoginPostAuthenticate(useraccount);
        }
        else
        {
            err = ErrorMsg(TT_CMDERR_LOGINSERVICE_UNAVAILABLE);
        }
        break;
    }

    WebLoginComplete(servernode, userid, useraccount, err);
}
    
ErrorMsg ServerGuard::WebLoginPostAuthenticate(UserAccount& useraccount)
{
    // check that web-login username is not banned
    BannedUser ban;
    ban.username = useraccount.username;
    if (m_settings.IsUserBanned(ban))
    {
        return ErrorMsg(TT_CMDERR_SERVER_BANNED);
    }
    else
    {
        return useraccount.usertype == USERTYPE_NONE ?
            ErrorMsg(TT_CMDERR_INVALID_ACCOUNT) : ErrorMsg(TT_CMDERR_SUCCESS);
    }
}

void ServerGuard::WebLoginComplete(ServerNode* servernode, ACE_UINT32 userid,
                                   const UserAccount& useraccount, const ErrorMsg& err)
{
    serveruser_t user = servernode->GetUser(userid, nullptr);
    if(!user)
        return;

    if(!err.success())
    {
        m_pendinglogin[userid] = UserAccount();
        user->DoError(err);
    }
    else
    {
        m_pendinglogin[userid] = useraccount;
    }

    timer_userdata usr = {};
    usr.src_userid = ACE_UINT16(userid);

    int ret = servernode->StartTimer(TIMERSRV_COMMAND_RESUME_ID, usr, ACE_Time_Value::zero);
    TTASSERT(ret >= 0);
}

#endif /* ENABLE_TEAMTALKPRO */

ErrorMsg ServerGuard::AuthenticateUser(ServerNode* servernode, ServerUser& user, UserAccount& useraccount)
{
    BannedUser ban;
    ban.ipaddr = user.GetIpAddress();
    ban.username = useraccount.username;
    if(m_settings.IsUserBanned(ban))
    {
        return ErrorMsg(TT_CMDERR_SERVER_BANNED);
    }

    MYTRACE(ACE_TEXT("Authenticating %s\n"), useraccount.username.c_str());

#if defined(ENABLE_TEAMTALKPRO)
    bool bearware = false;
    ACE_TString bwregex = ACE_TEXT(WEBLOGIN_BEARWARE_POSTFIX) + ACE_TString(ACE_TEXT("$"));
    if (useraccount.username == ACE_TEXT(WEBLOGIN_BEARWARE_USERNAME))
        return TT_CMDERR_INVALID_ACCOUNT;
#if defined(UNICODE)
    bearware |= std::regex_search(useraccount.username.c_str(), std::wregex(bwregex.c_str()));
#else
    bearware |= std::regex_search(useraccount.username.c_str(), std::regex(bwregex.c_str()));
#endif

    if (bearware)
    {
        auto i = m_pendinglogin.find(user.GetUserID());
        if(i != m_pendinglogin.end())
        {
            useraccount = i->second;

            if((useraccount.userrights & USERRIGHT_LOCKED_NICKNAME))
                user.SetNickname(useraccount.nickname);
            m_pendinglogin.erase(i);
            
            if(useraccount.usertype != USERTYPE_NONE)
                return TT_CMDERR_SUCCESS;
            return TT_CMDERR_IGNORE;
        }

        MYTRACE(ACE_TEXT("Starting BearWare.dk WebLogin for %s\n"), useraccount.username.c_str());
        std::thread mythr(&ServerGuard::WebLoginBearWare, this, servernode, ACE_UINT32(user.GetUserID()), useraccount);
        mythr.detach();

        return TT_SRVERR_COMMAND_SUSPEND;
    }
#endif /* ENABLE_TEAMTALKPRO */

    if(m_settings.AuthenticateUser(useraccount))
    {
        return ErrorMsg(TT_CMDERR_SUCCESS);
    }

    return ErrorMsg(TT_CMDERR_INVALID_ACCOUNT);
}

ErrorMsg ServerGuard::JoinChannel(const ServerUser& user, const ServerChannel& chan)
{
    BannedUser testban = user.GetBan(BANTYPE_CHANNEL);
    testban.chanpath = chan.GetChannelPath();
    if (chan.IsBanned(testban))
        return TT_CMDERR_CHANNEL_BANNED;

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerGuard::GetUserAccount(const ServerUser& user,
                                     UserAccount& useraccount)
{
    if(m_settings.GetUser(UnicodeToUtf8(useraccount.username).c_str(), useraccount))
        return ErrorMsg(TT_CMDERR_SUCCESS);
    return ErrorMsg(TT_CMDERR_ACCOUNT_NOT_FOUND);
}

ErrorMsg ServerGuard::GetRegUsers(const ServerUser& user, useraccounts_t& users)
{
    int i = 0;
    UserAccount useracc;
    while(m_settings.GetNextUser(i, useracc))
    {
        users.push_back(useracc);
        i++;
        useracc = UserAccount(); //reset
    }
    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerGuard::AddRegUser(const ServerUser& user, const UserAccount& useraccount)
{
    if( (user.GetUserType() & USERTYPE_ADMIN) == 0)
    {
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
    }

    //ensure no dublicates
    m_settings.RemoveUser(UnicodeToUtf8(useraccount.username).c_str());
    m_settings.AddNewUser(useraccount);

    //TODO: Separate log event
    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(user.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("created user account: \"") << LogPrepare(useraccount.username).c_str() << ACE_TEXT("\".");

    TT_LOG(oss.str().c_str());

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerGuard::DeleteRegUser(const ServerUser& user, const ACE_TString& username)
{
    if( (user.GetUserType() & USERTYPE_ADMIN) == 0)
    {
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
    }

    tostringstream oss;
    oss << ACE_TEXT("User #") << user.GetUserID() << ACE_TEXT(" ");
    oss << ACE_TEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << ACE_TEXT("\" ");
    if(user.GetUsername().length())
        oss << ACE_TEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << ACE_TEXT("\" ");
    oss << ACE_TEXT("deleted user account: \"") << LogPrepare(username).c_str() << ACE_TEXT("\".");

    TT_LOG(oss.str().c_str());

    if(m_settings.RemoveUser(UnicodeToUtf8(username).c_str()))
        return ErrorMsg(TT_CMDERR_SUCCESS);
    return ErrorMsg(TT_CMDERR_ACCOUNT_NOT_FOUND);
}

ErrorMsg ServerGuard::AddUserBan(const ServerUser& banner, const ServerUser& banee, BanTypes bantype)
{
    // channel bans are stored in static channels
    if ((bantype & BANTYPE_CHANNEL) == BANTYPE_NONE)
    {
        BannedUser ban = banee.GetBan(bantype);
        m_settings.AddUserBan(ban);
    }

    return ErrorMsg(TT_CMDERR_SUCCESS);
}


ErrorMsg ServerGuard::AddUserBan(const ServerUser& banner, const BannedUser& ban)
{
    // channel bans are stored in static channels
    if ((ban.bantype & BANTYPE_CHANNEL) == BANTYPE_NONE)
    {
        m_settings.AddUserBan(ban);
    }

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerGuard::RemoveUserBan(const ServerUser& user, const BannedUser& ban)
{
    if ((ban.bantype & BANTYPE_CHANNEL) == BANTYPE_NONE)
    {
        if(!m_settings.RemoveUserBan(ban))
            return ErrorMsg(TT_CMDERR_BAN_NOT_FOUND);
    }
    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerGuard::GetUserBans(const ServerUser& user, std::vector<BannedUser>& bans)
{
    //set banlist
    bans = m_settings.GetUserBans();
    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerGuard::ChangeNickname(const ServerUser& user, const ACE_TString& newnick)
{
    if((user.GetUserRights() & USERRIGHT_LOCKED_NICKNAME) == USERRIGHT_NONE)
        return ErrorMsg(TT_CMDERR_SUCCESS);
    return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
}

ErrorMsg ServerGuard::ChangeStatus(const ServerUser& user, int mode, const ACE_TString& status)
{
    if((user.GetUserRights() & USERRIGHT_LOCKED_STATUS) == USERRIGHT_NONE)
        return ErrorMsg(TT_CMDERR_SUCCESS);
    return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
}
