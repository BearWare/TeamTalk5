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

#include <ace/Log_Msg.h>
#include <ace/Dirent_Selector.h>
#include <ace/streams.h>
#include <ace/OS_NS_sys_stat.h>

#include <sstream>
#include <queue>
#include <regex>

#include <teamtalk/Commands.h>
#include <teamtalk/Log.h>

#include "AppInfo.h"

using namespace teamtalk;
using namespace std;

#if defined(UNICODE)
typedef wostringstream tostringstream;
#else
typedef ostringstream tostringstream;
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

#if defined(ENABLE_HTTP_AUTH)
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

#if defined(ENABLE_HTTP_AUTH)

void ServerGuard::WebLoginFacebook(ServerNode* servernode, ACE_UINT32 userid, UserAccount useraccount)
{
    int ret;
    ErrorMsg err;
    strings_t tokens = tokenize(useraccount.passwd, ACE_TEXT("="));
    useraccount.passwd.clear();
    ACE_CString url = WEBLOGIN_URL;

    {
        GUARD_OBJ_NAME(g, servernode, servernode->lock());

        // authenticate 'facebook'
        if(m_settings.AuthenticateUser(useraccount) && tokens.size() >= 2)
        {
            url += "client=" TEAMTALK_LIB_NAME;
            url += "&version=" TEAMTALK_VERSION;
            url += "&service=facebook";

            if(tokens[0] == ACE_TEXT("code"))
            {
#if defined(UNICODE)
                url += "&code=" + UnicodeToUtf8(tokens[1]);
#else
                url += "&code=" + tokens[1];
#endif
            }
            else if(tokens[0] == ACE_TEXT("token"))
            {
#if defined(UNICODE)
                url += "&token=" + UnicodeToUtf8(tokens[1]);
#else
                url += "&token=" + tokens[1];
#endif
            }
            else
            {
                err = TT_CMDERR_INVALID_ACCOUNT;
            }
        }
        else
        {
            err = TT_CMDERR_INVALID_ACCOUNT;
        }

        if (!err.success())
        {
            WebLoginComplete(servernode, userid, useraccount, err);
            return;
        }
    }

    std::string utf8;
    ret = HttpRequest(url, utf8);

    GUARD_OBJ_NAME(g, servernode, servernode->lock());

    switch(ret)
    {
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
            std::string name = xmldoc.GetValue("teamtalk/facebook/name");
            std::string id = xmldoc.GetValue("teamtalk/facebook/id");
            id += WEBLOGIN_FACEBOOK_POSTFIX;
#if defined(UNICODE)
            useraccount.username = Utf8ToUnicode(id.c_str());
            useraccount.nickname = Utf8ToUnicode(name.c_str());
#else
            useraccount.username = id.c_str();
            useraccount.nickname = name.c_str();
#endif
            err = WebLoginPostAuthenticate(useraccount);
        }
        else
        {
            err = ErrorMsg(TT_CMDERR_LOGINSERVICE_UNAVAILABLE);
        }
        break;
    } // switch

    WebLoginComplete(servernode, userid, useraccount, err);
}

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
            WebLoginComplete(servernode, userid, useraccount, err);
            return;
        }

        // authenticate 'bearware.dk'
        UserAccount sharedaccount;
        sharedaccount.username = ACE_TEXT(WEBLOGIN_BEARWARE_USERNAME);
        if(!m_settings.AuthenticateUser(sharedaccount))
        {
            err.errorno = TT_CMDERR_INVALID_ACCOUNT;
            WebLoginComplete(servernode, userid, useraccount, err);
            return;
        }

        //notice 'bearware' is now username, so swap it back
        useraccount = sharedaccount;
    }


    ACE_CString url = WEBLOGIN_URL;
    url += "client=" TEAMTALK_LIB_NAME;
    url += "&version=" TEAMTALK_VERSION;
    url += "&service=bearware";
    url += "&action=serverauth";
    url += ACE_CString("&username=") + authusername.c_str();
    url += ACE_CString("&accesstoken=") + authtoken.c_str();

    std::string utf8;
    int ret = HttpRequest(url, utf8);

    GUARD_OBJ_NAME(g, servernode, servernode->lock()); // lock required by WebLoginPostAuthenticate() and WebLoginComplete()

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
            std::string nickname = xmldoc.GetValue("teamtalk/bearware/nickname");
            std::string username = xmldoc.GetValue("teamtalk/bearware/username");
#if defined(UNICODE)
            useraccount.username = Utf8ToUnicode(username.c_str());
            useraccount.nickname = Utf8ToUnicode(nickname.c_str());
#else
            useraccount.username = username.c_str();
            useraccount.nickname = nickname.c_str();
#endif
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
    if(m_settings.IsUserBanned(ban))
    {
        return ErrorMsg(TT_CMDERR_SERVER_BANNED);
    }
    else
    {
        // authenticate if web-login username exists in db
        m_settings.AuthenticateUser(useraccount);
        return ErrorMsg(TT_CMDERR_SUCCESS);
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

    int ret = servernode->StartTimer(TIMER_COMMAND_RESUME, usr, ACE_Time_Value::zero);
    TTASSERT(ret >= 0);
}

#endif

ErrorMsg ServerGuard::AuthenticateUser(ServerNode* servernode, ServerUser& user, UserAccount& useraccount)
{
    BannedUser ban;
    ban.ipaddr = user.GetIpAddress();
    ban.username = useraccount.username;
    if(m_settings.IsUserBanned(ban))
    {
        return ErrorMsg(TT_CMDERR_SERVER_BANNED);
    }

#if defined(ENABLE_HTTP_AUTH)
    
    bool facebook = useraccount.username == ACE_TEXT(WEBLOGIN_FACEBOOK_USERNAME);
    ACE_TString fbregex = ACE_TEXT(WEBLOGIN_FACEBOOK_POSTFIX) + ACE_TString(ACE_TEXT("$"));
#if defined(UNICODE)
    facebook |= std::regex_search(useraccount.username.c_str(), std::wregex(fbregex.c_str()));
#else
    facebook |= std::regex_search(useraccount.username.c_str(), std::regex(fbregex.c_str()));
#endif

    bool bearware = false;
#if defined(ENABLE_TEAMTALKPRO)
    ACE_TString bwregex = ACE_TEXT(WEBLOGIN_BEARWARE_POSTFIX) + ACE_TString(ACE_TEXT("$"));
    if (useraccount.username == ACE_TEXT(WEBLOGIN_BEARWARE_USERNAME))
        return TT_CMDERR_INVALID_ACCOUNT;
#if defined(UNICODE)
    bearware |= std::regex_search(useraccount.username.c_str(), std::wregex(bwregex.c_str()));
#else
    bearware |= std::regex_search(useraccount.username.c_str(), std::regex(bwregex.c_str()));
#endif

#endif /* ENABLE_TEAMTALKPRO */

    if (bearware || facebook)
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

        if (facebook)
        {
            std::thread mythr(&ServerGuard::WebLoginFacebook, this, servernode, ACE_UINT32(user.GetUserID()), useraccount);
            mythr.detach();
        }
        else if (bearware)
        {
            std::thread mythr(&ServerGuard::WebLoginBearWare, this, servernode, ACE_UINT32(user.GetUserID()), useraccount);
            mythr.detach();
        }

        return TT_SRVERR_COMMAND_SUSPEND;
    }
#endif /* ENABLE_HTTP_AUTH */

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


namespace teamtalk {

    bool LoadConfig(ServerXML& xmlSettings, const ACE_TString& cfgfile)
    {
        ACE_TString settings_path;
        ACE_TCHAR buf[1024] = {};

        if(cfgfile.empty())
        {
            ACE_TString workdir = ACE_OS::getcwd(buf, 1024);
            workdir += ACE_DIRECTORY_SEPARATOR_STR;
            settings_path = workdir + ACE_TEXT(TEAMTALK_SETTINGSFILE);
        }
        else settings_path = cfgfile;

        if(ACE_OS::filesize(settings_path.c_str())>0)
        {
            if(!xmlSettings.LoadFile(UnicodeToUtf8(settings_path).c_str()))
            {
                tostringstream osErr;
                if(xmlSettings.HasErrors())
                {
                    ACE_TString errmsg = Utf8ToUnicode(xmlSettings.GetError().c_str());
                    osErr << ACE_TEXT("Unable to read ") << settings_path.c_str() << ACE_TEXT(". ") << errmsg.c_str() << endl;
                }
                else
                    osErr << ACE_TEXT("Unable to open file ") << settings_path.c_str() << ACE_TEXT(".");
                TT_SYSLOG(osErr.str().c_str());

                return false;
            }
            return true;
        }
        else
        {
            if(!xmlSettings.CreateFile(UnicodeToUtf8(settings_path).c_str()))
            {
                tostringstream osErr;
                osErr << ACE_TEXT("Unable to create settings file ") << settings_path.c_str() << ACE_TEXT(".");
                TT_SYSLOG(osErr.str().c_str());
                return false;
            }
            return true;
        }
    }


    bool ReadServerProperties(ServerXML& xmlSettings, ServerSettings& properties,
                              statchannels_t& channels)
    {
        properties.servername = Utf8ToUnicode(xmlSettings.GetServerName().c_str());
        properties.motd = Utf8ToUnicode(xmlSettings.GetMessageOfTheDay().c_str());
        properties.maxusers = xmlSettings.GetMaxUsers() == UNDEFINED? MAX_USERS : xmlSettings.GetMaxUsers();
        properties.max_logins_per_ipaddr = xmlSettings.GetMaxLoginsPerIP();
        properties.maxloginattempts = xmlSettings.GetMaxLoginAttempts();
        properties.logindelay = xmlSettings.GetLoginDelay();
        properties.usertimeout = xmlSettings.GetUserTimeout();
        properties.filesroot = Utf8ToUnicode(xmlSettings.GetFilesRoot().c_str());
        properties.diskquota = xmlSettings.GetDefaultDiskQuota();
        properties.maxdiskusage = xmlSettings.GetMaxDiskUsage();
        properties.voicetxlimit = xmlSettings.GetVoiceTxLimit();
        properties.videotxlimit = xmlSettings.GetVideoCaptureTxLimit();
        properties.mediafiletxlimit = xmlSettings.GetMediaFileTxLimit();
        properties.desktoptxlimit = xmlSettings.GetDesktopTxLimit();
        properties.totaltxlimit = xmlSettings.GetTotalTxLimit();
        properties.autosave = xmlSettings.GetAutoSave();

        u_short tcpport = xmlSettings.GetHostTcpPort() == UNDEFINED? DEFAULT_TCPPORT : xmlSettings.GetHostTcpPort();
        u_short udpport = xmlSettings.GetHostUdpPort() == UNDEFINED? DEFAULT_UDPPORT : xmlSettings.GetHostUdpPort();
        std::vector<std::string> bindips = xmlSettings.GetBindIPs();
        if (bindips.empty())
            bindips.push_back("");
        for (auto ip : bindips)
        {
            if(ip.length())
            {
#if defined(WIN32)
                ACE_INET_Addr tcpaddr(tcpport, Utf8ToUnicode(ip.c_str()).c_str());
                ACE_INET_Addr udpaddr(udpport, Utf8ToUnicode(ip.c_str()).c_str());
#else
                ACE_INET_Addr tcpaddr(tcpport, Utf8ToUnicode(ip.c_str()));
                ACE_INET_Addr udpaddr(udpport, Utf8ToUnicode(ip.c_str()));
#endif
                properties.tcpaddrs.push_back(tcpaddr);
                properties.udpaddrs.push_back(udpaddr);
            }
            else
            {
                properties.tcpaddrs.push_back(ACE_INET_Addr(tcpport));
                properties.udpaddrs.push_back(ACE_INET_Addr(udpport));
            }
        }

#if defined(ENABLE_TEAMTALKPRO)
        ACE_TString certfile, privfile;

        certfile = Utf8ToUnicode(xmlSettings.GetCertificateFile().c_str());
        privfile = Utf8ToUnicode(xmlSettings.GetPrivateKeyFile().c_str());

        ACE_SSL_Context *context = ACE_SSL_Context::instance ();
        context->set_mode(ACE_SSL_Context::SSLv23);
        if(certfile.length() && privfile.length())
        {
            if(context->certificate (UnicodeToLocal(certfile).c_str(), SSL_FILETYPE_PEM)<0)
            {
                TT_SYSLOG("Failed to load certificate file. Check the settings file.");
                return false;
            }
            if(context->private_key (UnicodeToLocal(privfile).c_str(), SSL_FILETYPE_PEM)<0)
            {
                TT_SYSLOG("Failed to load private key file. Check the settings file.");
                return false;
            }
        }
#endif

        xmlSettings.GetStaticChannels(channels);

        return true;
    }

    bool ConfigureServer(ServerNode& servernode, const ServerSettings& properties,
                         const statchannels_t& channels)
    {
        GUARD_OBJ(&servernode, servernode.lock());

        //load settings
        servernode.SetServerProperties(properties);

        servernode.SetAutoSaving(false);

        if(properties.filesroot.length() && !servernode.SetFileSharing(properties.filesroot))
            TT_SYSLOG("File sharing failed to initialize properly.");

        MakeStaticChannels(servernode, channels);

        if(!servernode.GetRootChannel())
        {
            ChannelProp chanprop;
            chanprop.channelid = 1;
            chanprop.chantype = CHANNEL_DEFAULT | CHANNEL_PERMANENT;
            chanprop.audiocodec.codec = CODEC_OPUS;
            chanprop.audiocodec.opus.samplerate = 48000;
            chanprop.audiocodec.opus.channels = 1;
            chanprop.audiocodec.opus.complexity = 10;
            chanprop.audiocodec.opus.fec = true;
            chanprop.audiocodec.opus.dtx = false;
            chanprop.audiocodec.opus.bitrate = 32000;
            chanprop.audiocodec.opus.vbr = true;
            chanprop.audiocodec.opus.vbr_constraint = false;
            chanprop.audiocodec.opus.frame_size = 1920;
            chanprop.audiocodec.opus.application = 2048; /* OPUS_APPLICATION_VOIP */
            chanprop.audiocodec.opus.frames_per_packet = 1;
#if 0
            chanprop.audiocodec.codec = CODEC_SPEEX;
            chanprop.audiocodec.speex.bandmode = 1;
            chanprop.audiocodec.speex.frames_per_packet = 2;
            chanprop.audiocodec.speex.quality = 4;
            chanprop.audiocodec.speex.sim_stereo = false;
#endif
            chanprop.diskquota = properties.diskquota;
            bool makeroot = servernode.MakeChannel(chanprop).errorno == TT_CMDERR_SUCCESS;
            TTASSERT(makeroot);
        }

        //don't enable auto-save until everything has been set in the server
        servernode.SetAutoSaving(properties.autosave);

        return true;
    }

    void ConvertChannels(const serverchannel_t& root, 
                         teamtalk::statchannels_t& channels, 
                         bool onlystatic/* = false*/)
    {
        queue< serverchannel_t > sweeper;
        queue< int > sweeperid;
        sweeper.push(root);
        sweeperid.push(0);

        int parentid = 0;
        while(!sweeper.empty())
        {
            serverchannel_t chan = sweeper.front();
            sweeper.pop();
            parentid = sweeperid.front();
            sweeperid.pop();

            ChannelProp schan = chan->GetChannelProp();
            channels[schan.channelid] = schan;

            ServerChannel::channels_t subs = chan->GetSubChannels();
            for(size_t i=0;i<subs.size();i++)
            {
                ACE_TString path = subs[i]->GetChannelPath().c_str();
                if(!onlystatic || (subs[i]->GetChannelType() & CHANNEL_PERMANENT))
                {
                    sweeper.push(subs[i]);
                    sweeperid.push(subs[i]->GetChannelID());
                }
            }
        }
    }

    void MakeStaticChannels(ServerNode& servernode, const statchannels_t& channels)
    {
        //Make static channels
        int root_id = GetRootChannelID(channels);
        if(root_id>0)
        {
            int fileid = 0;

            statchannels_t::const_iterator ite = channels.find(root_id);
            ChannelProp chan = ite->second;
            queue< ChannelProp > sweeper;
            sweeper.push(chan);

            while(!sweeper.empty())
            {
                chan = sweeper.front();
                sweeper.pop();
                bool create_chan = servernode.MakeChannel(chan).errorno == TT_CMDERR_SUCCESS;
                TTASSERT(create_chan);
                if(!create_chan)
                {
                    tostringstream os;
                    os << ACE_TEXT("Failed to create channel #") << chan.channelid << ACE_TEXT(" with name ") << chan.name.c_str();
                    TT_SYSLOG(os.str().c_str());
                    continue;
                }
                ACE_TString filesroot = servernode.GetFilesRoot().c_str() + ACE_TString(ACE_DIRECTORY_SEPARATOR_STR);
                for(size_t i=0;i<chan.files.size();i++)
                {
                    // add file, even if it doesn't exists. Otherwise
                    // a save operation will erase the files from the
                    // xml file.
                    chan.files[i].fileid = ++fileid;
                    chan.files[i].channelid = chan.channelid;
                    if(servernode.AddFileToChannel(chan.files[i]).errorno != TT_CMDERR_SUCCESS)
                    {
                        tostringstream os;
                        os << ACE_TEXT("Failed to add file ") << chan.files[i].filename.c_str() << ACE_TEXT(" to channel #") << chan.files[i].channelid;
                        TT_SYSLOG(os.str().c_str());
                    }

                    ACE_TString rootpath = filesroot + chan.files[i].internalname.c_str();
                    if(ACE_OS::filesize(rootpath.c_str())<0)
                    {
                        tostringstream os;
                        os << ACE_TEXT("Failed to open channel #") << chan.files[i].channelid << ACE_TEXT(" file: ") << rootpath.c_str();
                        TT_SYSLOG(os.str().c_str());
                    }
                    else
                    {
                        TTASSERT(ACE_OS::filesize(rootpath.c_str()) == chan.files[i].filesize);
                    }
                }

                std::for_each(chan.bans.begin(), chan.bans.end(), [&] (BannedUser ban) 
                    { servernode.AddBannedUserToChannel(ban); });

                statchannels_t subs = GetSubChannels(chan.channelid, channels);
                for(ite=subs.begin();ite!=subs.end();ite++)
                    sweeper.push(ite->second);
            }
        }
    }

    void RotateLogfile(const ACE_TString& cwd, const ACE_TString& logname, 
                       std::ofstream& logfile)
    {
        logfile.close();

        ACE_Dirent_Selector ds;
        if(ds.open(cwd.c_str())<0)
            return;

        int max = 0;
        for(int i=0;i<ds.length();i++)
        {
            ACE_TString curfile = ds[i]->d_name;
            if(curfile.length()>logname.length() && 
               curfile.substr(0, logname.length()) == logname)
            {
                strings_t tokens = tokenize(curfile, ACE_TEXT("."));
                if(tokens.empty())
                    continue;
                int val = int(string2i(tokens[tokens.size()-1]));
                if(val > max)
                    max = val;
            }
        }

        max++;

        ACE_TString tmp_logname = logname + ACE_TEXT(".") + i2string(max);

        //rename from LogFile.log to LogFile.log.1
        int ren = ACE_OS::rename(logname.c_str(), tmp_logname.c_str());
        TTASSERT(ren>=0);

        logfile.open(logname.c_str());
        if(!logfile.good())
            logfile.clear();
    }

}
