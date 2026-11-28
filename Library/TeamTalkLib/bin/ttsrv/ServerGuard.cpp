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

#include "AppInfo.h"
#include "ServerConfig.h"
#include "ServerXML.h"
#include "TeamTalkDefs.h"
#include "myace/MyINet.h"
#include "settings/Settings.h"
#include "teamtalk/Log.h"
#include "teamtalk/server/ServerChannel.h"
#include "teamtalk/server/ServerUser.h"
#include "teamtalk/TTAssert.h"

#include <cassert>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace teamtalk;
using namespace std;

#if defined(UNICODE)
typedef std::wostringstream tostringstream;
#else
using tostringstream = std::ostringstream;
#endif

static tstring LogPrepare(const tstring& str)
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
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" TCP address: ") << user.GetIpAddress().c_str() << TTEXT(" connected.");
    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserLogin(const ServerUser& user)
{
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if(!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("IP address: ") << user.GetIpAddress().c_str() << TTEXT(" ");
    oss << TTEXT("UDP address: ") << user.GetUdpAddress().get_host_addr() << TTEXT(":");
    oss << user.GetUdpAddress().get_port_number() << TTEXT(" logged in.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserAuthFailed(const ServerUser& user, const tstring& username)
{
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("failed to authenticate for account \"") << LogPrepare(username).c_str() << TTEXT("\" ");
    oss << TTEXT("IP address: ") << user.GetIpAddress().c_str() << TTEXT(" ");
    oss << TTEXT("UDP address: ") << user.GetUdpAddress().get_host_addr() << TTEXT(":");
    oss << user.GetUdpAddress().get_port_number() << TTEXT(".");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserLoginBanned(const ServerUser& user)
{
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if(!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("IP address: ") << user.GetIpAddress().c_str() << TTEXT(" ");
    oss << TTEXT("UDP address: ") << user.GetUdpAddress().get_host_addr() << TTEXT(":");
    oss << user.GetUdpAddress().get_port_number() << TTEXT(" denied login due to banned IP-address.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserLoggedOut(const ServerUser& user)
{
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if(!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("logged out.");

    TT_LOG(oss.str().c_str());

#if defined(ENABLE_TEAMTALKPRO)
    m_pendinglogin.erase(user.GetUserID());
#endif
}

void ServerGuard::OnUserDisconnected(const ServerUser& user)
{
    tostringstream oss;

    serverchannel_t const chan = user.GetChannel();
    if(chan)
    {
        oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
        oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
        if(!user.GetUsername().empty())
            oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
        oss << TTEXT("Channel: \"") << LogPrepare(chan->GetChannelPath()).c_str() << TTEXT("\" ");
        oss << TTEXT("disconnected.");
    }
    else
    {
        oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
        oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
        if(!user.GetUsername().empty())
            oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
        oss << TTEXT("disconnected.");
    }
    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserCryptError(const ServerUser& user, int sslerr, const tstring& msg)
{
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("encryption error no: 0x") << std::hex << sslerr << TTEXT(" ");
    oss << TTEXT("message: ") << LogPrepare(msg).c_str();
    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserDropped(const ServerUser& user)
{
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if(!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("dropped after ") << user.GetLastKeepAlive() << TTEXT(" sec for inactivity.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserKicked(const ServerUser& kickee, 
                               const ServerUser* kicker, 
                               const ServerChannel* channel)
{
    tostringstream oss;
    oss << TTEXT("User #") << kickee.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(kickee.GetNickname()).c_str() << TTEXT("\" ");
    if(!kickee.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(kickee.GetUsername()).c_str() << TTEXT("\" ");
    if(kicker != nullptr)
    {
        oss << TTEXT("kicked by #") << kicker->GetUserID() << TTEXT(" ");
        oss << TTEXT("nickname: \"") << LogPrepare(kicker->GetNickname()).c_str() << TTEXT("\" ");
        if(channel != nullptr)
            oss << TTEXT("from channel: \"") << LogPrepare(channel->GetChannelPath()).c_str() << TTEXT("\".");
        else
            oss << TTEXT("from server.");
    }
    else
    {
        if(channel != nullptr)
            oss << TTEXT("kicked from channel: \"") << LogPrepare(channel->GetChannelPath()).c_str() << TTEXT("\".");
        else
            oss << TTEXT("kick from server.");
    }

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserBanned(const ServerUser& banee, const ServerUser& banner)
{
    tostringstream oss;
    oss << TTEXT("User #") << banner.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(banner.GetNickname()).c_str() << TTEXT("\" ");
    if(!banner.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(banner.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("banned user #") << banee.GetUserID() << TTEXT(" ");
    oss << TTEXT("IP address: ") << LogPrepare(banee.GetIpAddress()).c_str() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(banee.GetNickname()).c_str() << TTEXT("\" ");
    if(!banee.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(banee.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("from server.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserBanned(const tstring& ipaddr, const ServerUser& banner)
{
    tostringstream oss;
    oss << TTEXT("User #") << banner.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(banner.GetNickname()).c_str() << TTEXT("\" ");
    if(!banner.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(banner.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("banned ");
    oss << TTEXT("IP address: ") << LogPrepare(ipaddr).c_str() << TTEXT(" ");
    oss << TTEXT("from server.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserBanned(const ServerUser& banner, const BannedUser& ban)
{
    tostringstream oss;
    oss << TTEXT("User #") << banner.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(banner.GetNickname()).c_str() << TTEXT("\" ");
    if(!banner.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(banner.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("banned ");

    if((ban.bantype & BANTYPE_IPADDR) != BANTYPE_NONE)
        oss << TTEXT("IP address: ") << LogPrepare(ban.ipaddr).c_str() << TTEXT(" ");
    if((ban.bantype & BANTYPE_USERNAME) != BANTYPE_NONE)
        oss << TTEXT("username: \"") << LogPrepare(ban.username).c_str() << TTEXT("\" ");

    if((ban.bantype & BANTYPE_CHANNEL) != BANTYPE_NONE)
        oss << TTEXT("from channel \"") << LogPrepare(ban.chanpath).c_str() << TTEXT("\".");
    else
        oss << TTEXT("from server.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserUnbanned(const ServerUser& user, const BannedUser& ban)
{
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if(!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("unbanned IP address: ") << ban.ipaddr.c_str() << TTEXT(".");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserUpdated(const ServerUser& user)
{
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if(!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("status: ") << user.GetStatusMode() << TTEXT(" ");
    oss << TTEXT("status msg: \"") << LogPrepare(user.GetStatusMessage()).c_str() << TTEXT("\" ");
    oss << TTEXT("UDP address: ") << user.GetUdpAddress().get_host_addr() << TTEXT(":");
    oss << user.GetUdpAddress().get_port_number() << TTEXT(" updated.");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserSubscribe(const ServerUser& user, const ServerUser& subscriptuser)
{
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if (!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("changed subscription to #") << subscriptuser.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(subscriptuser.GetNickname()).c_str() << TTEXT("\" ");
    if (!subscriptuser.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(subscriptuser.GetUsername()).c_str() << TTEXT("\": ");

    tostringstream oss_sub;
    auto subscriptions = user.GetSubscriptions(subscriptuser);
    if ((subscriptions & SUBSCRIBE_USER_MSG) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("um");
    if ((subscriptions & SUBSCRIBE_CHANNEL_MSG) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("cm");
    if ((subscriptions & SUBSCRIBE_BROADCAST_MSG) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("bm");
    if ((subscriptions & SUBSCRIBE_CUSTOM_MSG) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("mm");
    if ((subscriptions & SUBSCRIBE_VOICE) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("vo");
    if ((subscriptions & SUBSCRIBE_VIDEOCAPTURE) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("vi");
    if ((subscriptions & SUBSCRIBE_DESKTOP) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("de");
    if ((subscriptions & SUBSCRIBE_DESKTOPINPUT) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("di");
    if ((subscriptions & SUBSCRIBE_MEDIAFILE) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("mf");
    if ((subscriptions & SUBSCRIBE_INTERCEPT_USER_MSG) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("UM");
    if ((subscriptions & SUBSCRIBE_INTERCEPT_CHANNEL_MSG) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("CM");
    if ((subscriptions & SUBSCRIBE_INTERCEPT_CUSTOM_MSG) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("MM");
    if ((subscriptions & SUBSCRIBE_INTERCEPT_VOICE) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("VO");
    if ((subscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("VI");
    if ((subscriptions & SUBSCRIBE_INTERCEPT_DESKTOP) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("DE");
    if ((subscriptions & SUBSCRIBE_INTERCEPT_MEDIAFILE) != SUBSCRIBE_NONE)
        oss_sub << ((!oss_sub.str().empty()) ? TTEXT(",") : TTEXT("")) << TTEXT("MF");

    oss << oss_sub.str() << TTEXT(".");
    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserJoinChannel(const ServerUser& user, const ServerChannel& channel)
{
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if(!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("joined channel: \"") << LogPrepare(channel.GetChannelPath()).c_str() << TTEXT("\".");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserLeaveChannel(const ServerUser& user, const ServerChannel& channel)
{
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if(!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("left channel: \"") << LogPrepare(channel.GetChannelPath()).c_str() << TTEXT("\".");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserMoved(const ServerUser& mover, const ServerUser& movee)
{
    tostringstream oss;
    oss << TTEXT("User #") << mover.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(mover.GetNickname()).c_str() << TTEXT("\" ");
    if(!mover.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(mover.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("moved user #") << movee.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(movee.GetNickname()).c_str() << TTEXT("\" ");
    if(!movee.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(movee.GetUsername()).c_str() << TTEXT("\".");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnUserMessage(const ServerUser& from, const ServerUser& to, const teamtalk::TextMessage& msg)
{
    TTASSERT(msg.msgType == TTUserMsg);
    if(msg.msgType == TTUserMsg)
    {
        tostringstream oss;
        oss << TTEXT("User message from #") << from.GetUserID() << TTEXT(" ");
        oss << TTEXT("nickname: \"") << LogPrepare(from.GetNickname()).c_str() << TTEXT("\" ");
        if(!from.GetUsername().empty())
            oss << TTEXT("username: \"") << LogPrepare(from.GetUsername()).c_str() << TTEXT("\" ");
        oss << TTEXT("to #") << to.GetUserID() << TTEXT(" ");
        oss << TTEXT("nickname: \"") << LogPrepare(to.GetNickname()).c_str() << TTEXT("\" ");
        if(!to.GetUsername().empty())
            oss << TTEXT("username: \"") << LogPrepare(to.GetUsername()).c_str() << TTEXT("\" ");
        oss << TTEXT("content: \"") << LogPrepare(msg.content).c_str() << TTEXT("\".");

        TT_LOG(oss.str().c_str());
    }
}

void ServerGuard::OnChannelMessage(const ServerUser& from, const ServerChannel& channel, const teamtalk::TextMessage& msg)
{
    TTASSERT(msg.msgType == TTChannelMsg);
    if(msg.msgType == TTChannelMsg)
    {
        tostringstream oss;
        oss << TTEXT("Channel message from #") << from.GetUserID() << TTEXT(" ");
        oss << TTEXT("nickname: \"") << LogPrepare(from.GetNickname()).c_str() << TTEXT("\" ");
        if(!from.GetUsername().empty())
            oss << TTEXT("username: \"") << LogPrepare(from.GetUsername()).c_str() << TTEXT("\" ");
        oss << TTEXT("to channel: \"") << LogPrepare(channel.GetChannelPath()).c_str() << TTEXT("\" ");
        oss << TTEXT("content: \"") << LogPrepare(msg.content).c_str() << TTEXT("\".");

        TT_LOG(oss.str().c_str());
    }
}

void ServerGuard::OnBroadcastMessage(const ServerUser& from, const teamtalk::TextMessage& msg)
{
    TTASSERT(msg.msgType == TTBroadcastMsg);
    tostringstream oss;
    oss << TTEXT("Broadcast message from #") << from.GetUserID() << TTEXT(" ");
    oss << TTEXT("content: \"") << LogPrepare(msg.content).c_str() << TTEXT("\".");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnCustomMessage(const ServerUser& from, const ServerUser& to, const teamtalk::TextMessage& msg)
{
    //if(msg.msgType == TTCustomMsg)
    //{
    //    tostringstream oss;
    //    oss << TTEXT("User custom message from #") << from.GetUserID() << TTEXT(" ");
    //    oss << TTEXT("nickname: \"") << LogPrepare(from.GetNickname()).c_str() << TTEXT("\" ");
    //    if(from.GetUsername().length())
    //        oss << TTEXT("username: \"") << LogPrepare(from.GetUsername()).c_str() << TTEXT("\" ");
    //    oss << TTEXT("to #") << to.GetUserID() << TTEXT(" ");
    //    oss << TTEXT("nickname: \"") << LogPrepare(to.GetNickname()).c_str() << TTEXT("\" ");
    //    if(to.GetUsername().length())
    //        oss << TTEXT("username: \"") << LogPrepare(to.GetUsername()).c_str() << TTEXT("\" ");
    //    oss << TTEXT("content: \"") << LogPrepare(msg.content) << TTEXT("\".");

    //    TT_LOG(oss.str().c_str());
    //}
}

void ServerGuard::OnUserUpdateStream(const ServerUser& user, const ServerChannel& channel, StreamType stream, int streamid)
{
    tostringstream oss;
    switch (stream)
    {
    case STREAMTYPE_VOICE :
        oss << TTEXT("New voice stream ") << streamid << TTEXT(" from #") << user.GetUserID() << TTEXT(" ");
        break;
    case STREAMTYPE_DESKTOP :
        oss << TTEXT("New desktop stream ") << streamid << TTEXT(" from #") << user.GetUserID() << TTEXT(" ");
        break;
    case STREAMTYPE_DESKTOPINPUT :
        oss << TTEXT("New desktop input stream ") << streamid << TTEXT(" from #") << user.GetUserID() << TTEXT(" ");
        break;
    case STREAMTYPE_MEDIAFILE_AUDIO :
        oss << TTEXT("New audio media file stream ") << streamid << TTEXT(" from #") << user.GetUserID() << TTEXT(" ");
        break;
    case STREAMTYPE_MEDIAFILE_VIDEO :
        oss << TTEXT("New video media file stream ") << streamid << TTEXT(" from #") << user.GetUserID() << TTEXT(" ");
        break;
    case STREAMTYPE_VIDEOCAPTURE :
        oss << TTEXT("New video capture stream ") << streamid << TTEXT(" from #") << user.GetUserID() << TTEXT(" ");
        break;
    case STREAMTYPE_MEDIAFILE :
    case STREAMTYPE_CHANNELMSG :
    case STREAMTYPE_NONE :
    case STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO :
    case STREAMTYPE_ALL :
        break;
    }

    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if(!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("to channel: \"") << LogPrepare(channel.GetChannelPath()).c_str() << TTEXT("\".");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnChannelCreated(const ServerChannel& channel, 
                                   const ServerUser* user/* = NULL*/)
{
    if(user == nullptr)
        return;

    tostringstream oss;
    oss << TTEXT("Channel #") << channel.GetChannelID() << TTEXT(" ");
    if(user != nullptr)
    {
        oss << TTEXT("\"") << LogPrepare(channel.GetChannelPath()).c_str() << TTEXT("\" created by ");
        oss << TTEXT("nickname: \"") << LogPrepare(user->GetNickname()).c_str() << TTEXT("\" ");
        if(!user->GetUsername().empty())
            oss << TTEXT("username: \"") << LogPrepare(user->GetUsername()).c_str() << TTEXT("\".");
    }
    else
        oss << TTEXT("\"") << LogPrepare(channel.GetChannelPath()).c_str() << TTEXT("\" created.");

    TT_LOG(oss.str().c_str());
}

static void LogTransmitUsers(const std::set<int>& userids, ChannelTypes chantype, StreamType stream, tostringstream& oss)
{
    if (userids.empty())
        return;

    // non-classroom channels by default allow everyone to talk, so ignore those
    if (*userids.begin() == TRANSMITUSERS_FREEFORALL && userids.size() == 1 && (chantype & CHANNEL_CLASSROOM) == CHANNEL_DEFAULT)
        return;

    switch (stream)
    {
    case STREAMTYPE_VOICE :
        if ((chantype & CHANNEL_CLASSROOM) != CHANNEL_DEFAULT)
            oss << TTEXT(", can transmit voice:");
        else
            oss << TTEXT(", cannot transmit voice:");
        break;
    case STREAMTYPE_VIDEOCAPTURE :
        if ((chantype & CHANNEL_CLASSROOM) != CHANNEL_DEFAULT)
            oss << TTEXT(", can transmit video:");
        else
            oss << TTEXT(", cannot transmit video:");
        break;
    case STREAMTYPE_DESKTOP :
        if ((chantype & CHANNEL_CLASSROOM) != CHANNEL_DEFAULT)
            oss << TTEXT(", can transmit desktop:");
        else
            oss << TTEXT(", cannot transmit desktop:");
        break;
    case STREAMTYPE_MEDIAFILE :
        if ((chantype & CHANNEL_CLASSROOM) != CHANNEL_DEFAULT)
            oss << TTEXT(", can transmit media files:");
        else
            oss << TTEXT(", cannot transmit media files:");
        break;
    case STREAMTYPE_CHANNELMSG :
        if ((chantype & CHANNEL_CLASSROOM) != CHANNEL_DEFAULT)
            oss << TTEXT(", can send channel text messages:");
        else
            oss << TTEXT(", cannot send channel text messages:");
        break;
    default :
        assert(0);
        break;
    }

    for (auto id : userids)
    {
        if ((chantype & CHANNEL_CLASSROOM) == CHANNEL_CLASSROOM && id == TRANSMITUSERS_FREEFORALL)
            oss << TTEXT(" ") << TTEXT("everyone");
        else if (id != TRANSMITUSERS_FREEFORALL)
            oss << TTEXT(" ") << TTEXT("#") << id;
    }
}

void ServerGuard::OnChannelUpdated(const ServerChannel& channel, 
                                   const ServerUser* user/* = NULL*/)
{
    tostringstream oss;
    oss << TTEXT("Channel #") << channel.GetChannelID() << TTEXT(" ");
    if (user != nullptr)
    {
        oss << TTEXT("\"") << LogPrepare(channel.GetChannelPath()).c_str() << TTEXT("\" updated by ");
        oss << TTEXT("nickname: \"") << LogPrepare(user->GetNickname()).c_str() << TTEXT("\" ");
        if (!user->GetUsername().empty())
            oss << TTEXT("username: \"") << LogPrepare(user->GetUsername()).c_str() << TTEXT("\"");
    }
    else
    {
        oss << TTEXT("\"") << LogPrepare(channel.GetChannelPath()).c_str() << TTEXT("\" updated");
    }

    if (((channel.GetChannelType() & CHANNEL_SOLO_TRANSMIT) != CHANNEL_DEFAULT) && (!channel.GetTransmitQueue().empty()))
        oss << TTEXT(", transmitter: #") << *channel.GetTransmitQueue().begin();

    LogTransmitUsers(channel.GetVoiceUsers(), channel.GetChannelType(), STREAMTYPE_VOICE, oss);
    LogTransmitUsers(channel.GetVideoUsers(), channel.GetChannelType(), STREAMTYPE_VIDEOCAPTURE, oss);
    LogTransmitUsers(channel.GetMediaFileUsers(), channel.GetChannelType(), STREAMTYPE_MEDIAFILE, oss);
    LogTransmitUsers(channel.GetDesktopUsers(), channel.GetChannelType(), STREAMTYPE_DESKTOP, oss);
    LogTransmitUsers(channel.GetChannelTextMsgUsers(), channel.GetChannelType(), STREAMTYPE_CHANNELMSG, oss);

    oss << TTEXT(".");

    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnChannelRemoved(const ServerChannel& channel, 
                                   const ServerUser* user/* = NULL*/)
{
    // Log the channel removal event.
    if (user != nullptr) 
    {
        tostringstream oss;
        oss << TTEXT("Channel #") << channel.GetChannelID() << TTEXT(" ");
        oss << TTEXT("'") << LogPrepare(channel.GetChannelPath()).c_str() << TTEXT("' removed by "); 
        oss << TTEXT("nickname: '") << LogPrepare(user->GetNickname()).c_str() << TTEXT("' ");
        if(!user->GetUsername().empty())
            oss << TTEXT("username: '") << LogPrepare(user->GetUsername()).c_str() << TTEXT("'.");
        else
            oss << TTEXT(".");
        TT_LOG(oss.str().c_str());
    }
    else
    {
         // Log server-initiated removal if not already logged by operator cleanup.
         tostringstream oss_server_removed;
         oss_server_removed << TTEXT("Channel #") << channel.GetChannelID() << TTEXT(" ");
         oss_server_removed << TTEXT("'") << LogPrepare(channel.GetChannelPath()).c_str() << TTEXT("' removed (server process).");
         TT_LOG(oss_server_removed.str().c_str());
    }
}

void ServerGuard::OnFileUploaded(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file)
{
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if(!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("uploaded file: \"") << LogPrepare(file.filename).c_str() << TTEXT("\" ");
    oss << TTEXT("to channel: \"") << LogPrepare(chan.GetChannelPath()).c_str() << TTEXT("\".");
    TT_LOG(oss.str().c_str()); 
}

void ServerGuard::OnFileDownloaded(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file)
{
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if(!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("downloaded file: \"") << LogPrepare(file.filename).c_str() << TTEXT("\" ");
    oss << TTEXT("from channel: \"") << LogPrepare(chan.GetChannelPath()).c_str() << TTEXT("\".");
    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnFileDeleted(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file)
{
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if(!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("deleted file: \"") << LogPrepare(file.filename).c_str() << TTEXT("\" ");
    oss << TTEXT("from channel: \"") << LogPrepare(chan.GetChannelPath()).c_str() << TTEXT("\".");
    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnServerUpdated(const ServerUser* user, const ServerSettings&  /*srvprop*/)
{
    tostringstream oss;
    oss << TTEXT("Server properties updated");
    if (user != nullptr)
    {
        oss << TTEXT(" by user #") << user->GetUserID() << TTEXT(" ");
        oss << TTEXT("nickname: \"") << LogPrepare(user->GetNickname()).c_str() << TTEXT("\" ");
        if(!user->GetUsername().empty())
            oss << TTEXT("username: \"") << LogPrepare(user->GetUsername()).c_str() << TTEXT("\" ");
    }
    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnSaveConfiguration(const ServerUser* user/* = NULL*/)
{
    tostringstream oss;
    oss << TTEXT("Server configuration saved");
    if(user != nullptr)
    {
        oss << TTEXT(" by user #") << user->GetUserID() << TTEXT(" ");
        oss << TTEXT("nickname: \"") << LogPrepare(user->GetNickname()).c_str() << TTEXT("\" ");
        if(!user->GetUsername().empty())
            oss << TTEXT("username: \"") << LogPrepare(user->GetUsername()).c_str() << TTEXT("\"");
    }
    oss << TTEXT(".");
    TT_LOG(oss.str().c_str());
}

void ServerGuard::OnShutdown(const ServerStats& stats)
{
    constexpr auto BYTES_PER_KILOBYTE = 1024;
    tostringstream oss;
    oss << TTEXT("Data transferred - ");
    oss << TTEXT("Total TX: ") << stats.total_bytessent / BYTES_PER_KILOBYTE << TTEXT(" KBytes ");
    oss << TTEXT("Total RX: ") << stats.total_bytesreceived / BYTES_PER_KILOBYTE << TTEXT(" KBytes ");
    oss << TTEXT("Voice TX: ") << stats.voice_bytessent / BYTES_PER_KILOBYTE << TTEXT(" KBytes ");
    oss << TTEXT("Voice RX: ") << stats.voice_bytesreceived / BYTES_PER_KILOBYTE << TTEXT(" KBytes ");
    oss << TTEXT("Desktop TX: ") << stats.desktop_bytessent / BYTES_PER_KILOBYTE << TTEXT(" KBytes ");
    oss << TTEXT("Desktop RX: ") << stats.desktop_bytesreceived / BYTES_PER_KILOBYTE << TTEXT(" KBytes ");
    oss << TTEXT("Media Files TX: ") << stats.mediafile_bytessent / BYTES_PER_KILOBYTE << TTEXT(" KBytes ");
    oss << TTEXT("Media Files RX: ") << stats.mediafile_bytesreceived / BYTES_PER_KILOBYTE << TTEXT(" KBytes ");
    oss << TTEXT("Video TX: ") << stats.vidcap_bytessent / BYTES_PER_KILOBYTE << TTEXT(" KBytes/sec ");
    oss << TTEXT("Video RX: ") << stats.vidcap_bytesreceived / BYTES_PER_KILOBYTE << TTEXT(" KBytes/sec.");
    TT_LOG(oss.str().c_str());
}

#if defined(ENABLE_TEAMTALKPRO)

void ServerGuard::WebLoginBearWare(ServerNode* servernode, uint32_t userid, UserAccount useraccount)
{
    ErrorMsg err;
    std::string const authusername = UnicodeToUtf8(useraccount.username);
    std::string authtoken;

    {
        GUARD_OBJ_NAME(g, servernode, servernode->Lock());

        auto user = servernode->GetUser(userid, nullptr, false);
        if (user.get() != nullptr)
        {
            authtoken = UnicodeToUtf8(user->GetAccessToken());
        }

        if (authtoken.empty())
        {
            err = TT_CMDERR_INVALID_ACCOUNT;
            MYTRACE(TTEXT("Authentication token is empty\n"));
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
            sharedaccount.username = Utf8ToUnicode(WEBLOGIN_BEARWARE_USERNAME.data());
            if (!m_settings.AuthenticateUser(sharedaccount))
            {
                err.errorno = TT_CMDERR_INVALID_ACCOUNT;
                WebLoginComplete(servernode, userid, useraccount, err);
                MYTRACE(TTEXT("Shared 'bearware' account doesn't exist\n"));
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


    std::string url(WEBLOGIN_URL);
    url += "client=" + std::string(TEAMTALK_LIB_NAME);
    url += "&version=" + std::string(TEAMTALK_VERSION);
    url += "&service=bearware";
    url += "&action=serverauth";
    url += std::string("&username=") + authusername;
    url += std::string("&accesstoken=") + authtoken;

    MYTRACE(TTEXT("Performing HTTP web authentication of %s\n"), useraccount.username.c_str());
    std::string utf8;
    int const ret = HttpGetRequest(url, utf8);

    GUARD_OBJ_NAME(g, servernode, servernode->Lock()); // lock required by WebLoginPostAuthenticate() and WebLoginComplete()

    MYTRACE(TTEXT("HTTP response code: %d\n"), ret);
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
            std::string const nickname = xmldoc.GetValue(false, "teamtalk/bearware/nickname", "");
            std::string const username = xmldoc.GetValue(false, "teamtalk/bearware/username", "");
#if defined(UNICODE)
            useraccount.nickname = Utf8ToUnicode(nickname.c_str());
#else
            useraccount.nickname = nickname.c_str();
#endif
            TTASSERT(authusername == username);
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
    
            return useraccount.usertype == USERTYPE_NONE ?
            ErrorMsg(TT_CMDERR_INVALID_ACCOUNT) : ErrorMsg(TT_CMDERR_SUCCESS);
   
}

void ServerGuard::WebLoginComplete(ServerNode* servernode, uint32_t userid,
                                   const UserAccount& useraccount, const ErrorMsg& err)
{
    serveruser_t const user = servernode->GetUser(userid, nullptr, false);
    if(!user)
        return;

    if(!err.Success())
    {
        m_pendinglogin[userid] = UserAccount();
        user->DoError(err);
    }
    else
    {
        m_pendinglogin[userid] = useraccount;
    }

    timer_userdata usr = {};
    usr.src_userid = uint16_t(userid);

    int const ret = servernode->StartTimer(TIMERSRV_COMMAND_RESUME_ID, usr, ACE_Time_Value::zero);
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

    MYTRACE(TTEXT("Authenticating %s\n"), useraccount.username.c_str());

#if defined(ENABLE_TEAMTALKPRO)
    if (useraccount.username == Utf8ToUnicode(WEBLOGIN_BEARWARE_USERNAME.data()))
        return TT_CMDERR_INVALID_ACCOUNT;

    if (useraccount.IsWebLogin())
    {
        auto i = m_pendinglogin.find(user.GetUserID());
        if(i != m_pendinglogin.end())
        {
            useraccount = i->second;

            if((useraccount.userrights & USERRIGHT_LOCKED_NICKNAME) != USERRIGHT_NONE)
                user.SetNickname(useraccount.nickname);
            m_pendinglogin.erase(i);
            
            if(useraccount.usertype != USERTYPE_NONE)
                return TT_CMDERR_SUCCESS;
            return TT_CMDERR_IGNORE;
        }

        MYTRACE(TTEXT("Starting BearWare.dk WebLogin for %s\n"), useraccount.username.c_str());
        std::thread mythr(&ServerGuard::WebLoginBearWare, this, servernode, uint32_t(user.GetUserID()), useraccount);
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
    BannedUser testban = user.GenerateBan(BANTYPE_CHANNEL);
    testban.chanpath = chan.GetChannelPath();
    if (chan.IsBanned(testban))
        return TT_CMDERR_CHANNEL_BANNED;

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerGuard::RemoveChannel(const ServerChannel& chan, const ServerUser*  /*user*//* = nullptr */)
{
    // Clean up operator rights for the deleted channel.
    m_settings.CleanupChannelOperators(chan.GetChannelID());

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerGuard::GetUserAccount(const ServerUser&  /*user*/,
                                     UserAccount& useraccount)
{
    if(m_settings.GetUser(UnicodeToUtf8(useraccount.username), useraccount))
        return ErrorMsg(TT_CMDERR_SUCCESS);
    return ErrorMsg(TT_CMDERR_ACCOUNT_NOT_FOUND);
}

ErrorMsg ServerGuard::GetRegUsers(const ServerUser&  /*user*/, useraccounts_t& users)
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
    if( (user.GetUserType() & USERTYPE_ADMIN) == USERTYPE_NONE)
    {
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
    }

    //ensure no dublicates
    m_settings.RemoveUser(UnicodeToUtf8(useraccount.username));
    m_settings.AddNewUser(useraccount);

    //TODO: Separate log event
    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if(!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("created user account: \"") << LogPrepare(useraccount.username).c_str() << TTEXT("\".");

    TT_LOG(oss.str().c_str());

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerGuard::DeleteRegUser(const ServerUser& user, const tstring& username)
{
    if( (user.GetUserType() & USERTYPE_ADMIN) == USERTYPE_NONE)
    {
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
    }

    tostringstream oss;
    oss << TTEXT("User #") << user.GetUserID() << TTEXT(" ");
    oss << TTEXT("nickname: \"") << LogPrepare(user.GetNickname()).c_str() << TTEXT("\" ");
    if(!user.GetUsername().empty())
        oss << TTEXT("username: \"") << LogPrepare(user.GetUsername()).c_str() << TTEXT("\" ");
    oss << TTEXT("deleted user account: \"") << LogPrepare(username).c_str() << TTEXT("\".");

    TT_LOG(oss.str().c_str());

    if(m_settings.RemoveUser(UnicodeToUtf8(username)))
        return ErrorMsg(TT_CMDERR_SUCCESS);
    return ErrorMsg(TT_CMDERR_ACCOUNT_NOT_FOUND);
}

ErrorMsg ServerGuard::AddUserBan(const ServerUser& banner, const ServerUser& banee, BanTypes bantype)
{
    auto ban = banee.GenerateBan(bantype);
    ban.owner = banner.GetUsername();
    return AddUserBan(banner, ban);
}

ErrorMsg ServerGuard::AddUserBan(const ServerUser&  /*banner*/, const BannedUser& ban)
{
    // channel bans are stored in static channels
    if ((ban.bantype & BANTYPE_CHANNEL) == BANTYPE_NONE)
    {
        m_settings.AddUserBan(ban);
    }

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerGuard::RemoveUserBan(const ServerUser&  /*user*/, const BannedUser& ban)
{
    if ((ban.bantype & BANTYPE_CHANNEL) == BANTYPE_NONE)
    {
        if(!m_settings.RemoveUserBan(ban))
            return ErrorMsg(TT_CMDERR_BAN_NOT_FOUND);
    }
    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerGuard::GetUserBans(const ServerUser&  /*user*/, std::vector<BannedUser>& bans)
{
    //set banlist
    bans = m_settings.GetUserBans();
    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerGuard::ChangeNickname(const ServerUser& user, const tstring&  /*newnick*/)
{
    if((user.GetUserRights() & USERRIGHT_LOCKED_NICKNAME) == USERRIGHT_NONE)
        return ErrorMsg(TT_CMDERR_SUCCESS);
    return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
}

ErrorMsg ServerGuard::ChangeStatus(const ServerUser& user, int  /*mode*/, const tstring&  /*status*/)
{
    if((user.GetUserRights() & USERRIGHT_LOCKED_STATUS) == USERRIGHT_NONE)
        return ErrorMsg(TT_CMDERR_SUCCESS);
    return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
}

ErrorMsg ServerGuard::SaveConfiguration(const ServerUser& /*user*/, teamtalk::ServerNode& servernode)
{
    teamtalk::statchannels_t channels;
    ConvertChannels(servernode.GetRootChannel(), channels, true);
    return SaveServerProperties(m_settings, servernode.GetServerProperties(), channels) ? ErrorMsg(TT_CMDERR_SUCCESS) : ErrorMsg(TT_CMDERR_OPENFILE_FAILED);
}
