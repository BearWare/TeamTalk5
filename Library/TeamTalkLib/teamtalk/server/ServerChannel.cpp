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

#include "ServerChannel.h"

#include "ServerUser.h"
#include "teamtalk/TTAssert.h"
#include "teamtalk/Common.h"
#include "myace/MyACE.h"

#include <algorithm>

#if defined(ENABLE_ENCRYPTION)
#include <openssl/rand.h>
#endif

using namespace teamtalk;

ServerChannel::ServerChannel(int channelid) 
    : PARENT(channelid)
{
    Init();
}

ServerChannel::ServerChannel(channel_t& parent, int channelid, const ACE_TString& name) 
    : PARENT(parent, channelid, name)
{
    Init();
}

ServerChannel::~ServerChannel()
{
    TTASSERT(m_lastUserPacket.empty());
}

void ServerChannel::Init()
{
#if defined(ENABLE_ENCRYPTION)
    RAND_bytes(m_cryptkey, sizeof(m_cryptkey));
#endif
    SetTransmitSwitchDelay(ACE_Time_Value(0, 500000));
}

constexpr auto STREAMKEY(int uid, int tx)
{
    return (uid << 16) | tx;
}

void ServerChannel::BlockAudioStream(int userid)
{
    int streamkey = STREAMKEY(userid, STREAMTYPE_VOICE);
    m_blockStreams[streamkey] = m_activeStreams[streamkey];
    streamkey = STREAMKEY(userid, STREAMTYPE_MEDIAFILE);
    m_blockStreams[streamkey] = m_activeStreams[streamkey];
}

bool ServerChannel::CanTransmit(int userid, StreamType txtype, int streamid, bool* modified)
{
    auto streamkey = STREAMKEY(userid, txtype);
    bool const newstreamid = m_activeStreams[streamkey] != streamid;
    m_activeStreams[streamkey] = streamid;

    if (!PARENT::CanTransmit(userid, txtype))
    {
        if ((GetChannelType() & CHANNEL_SOLO_TRANSMIT) != 0u)
        {
            // If transmitter is head we have to trigger channel update
            if (ClearFromTransmitQueue(userid) && (modified != nullptr))
                *modified = true;

            // If transmitter has been disallowed by channel update
            // then block the previous stream
            BlockAudioStream(userid);
        }
        return false;
    }

    if (newstreamid || (streamid == 0)) // !streamid handles SERVER_USERID
        m_streamstart[streamkey] = ACE_OS::gettimeofday();
    else if (GetTimeOutTimerVoice() != ACE_Time_Value::zero && (txtype & STREAMTYPE_VOICE) == STREAMTYPE_VOICE &&
             ACE_OS::gettimeofday() >= m_streamstart[streamkey] + GetTimeOutTimerVoice())
    {
        MYTRACE(ACE_TEXT("Channel %s blocked voice streamid %d from #%d after %d msec\n"), GetChannelPath().c_str(),
                streamid, userid, GetTimeOutTimerVoice().msec());
        return false;
    }
    else if (GetTimeOutTimerMediaFile() != ACE_Time_Value::zero && (txtype & STREAMTYPE_MEDIAFILE) == STREAMTYPE_MEDIAFILE &&
             ACE_OS::gettimeofday() >= m_streamstart[streamkey] + GetTimeOutTimerMediaFile())
    {
        MYTRACE(ACE_TEXT("Channel %s blocked media file streamid %d from #%d after %d msec\n"), GetChannelPath().c_str(),
                streamid, userid, GetTimeOutTimerMediaFile().msec());
        return false;
    }

    if (((GetChannelType() & CHANNEL_SOLO_TRANSMIT) != 0u) && ((txtype & (STREAMTYPE_VOICE | STREAMTYPE_MEDIAFILE)) != 0))
    {
        //MYTRACE(ACE_TEXT(" %d -> %d. StreamID: %d\n"), userid, m_blockStreams[STREAMKEY(userid, txtype)], streamid);

        /* Don't allow user to transmit until a new stream (id) is started */
        if ((userid != 0) && m_blockStreams[STREAMKEY(userid, txtype)] == streamid)
            return false;

        /* Update queue list */
        if (std::ranges::find(m_transmitqueue, userid) == m_transmitqueue.end())
        {
            if (modified != nullptr)
                *modified = true;
            m_transmitqueue.push_back(userid);
        }

        m_lastUserPacket[userid] = ACE_OS::gettimeofday();

        /* Can transmit if head of queue, transmitted within GetTransmitSwitchDelay() and started new stream */
        TTASSERT(!m_transmitqueue.empty());
        int const head_userid = *m_transmitqueue.begin();
        auto const itePkt = m_lastUserPacket.find(head_userid);
        if (itePkt->second + GetTransmitSwitchDelay() >= ACE_OS::gettimeofday())
        {
            return userid == head_userid;
        }
        
                    ClearFromTransmitQueue(head_userid);
            BlockAudioStream(head_userid);
            if (modified)
                *modified = true;
            return CanTransmit(userid, txtype, streamid, modified);
       
    }
    return true;
}

bool ServerChannel::ClearFromTransmitQueue(int userid)
{
    m_lastUserPacket.erase(userid);
    auto i = std::ranges::find(m_transmitqueue, userid);
    if (i != m_transmitqueue.end())
    {
        m_transmitqueue.erase(i);
        return true;
    }
    return false;
}

void ServerChannel::RemoveUser(int userid, bool* modified)
{
    ClearTransmitUser(userid);
    if (ClearFromTransmitQueue(userid) && (modified != nullptr))
        *modified = true;

    PARENT::RemoveUser(userid);

    m_blockStreams.erase(STREAMKEY(userid, STREAMTYPE_VOICE));
    m_blockStreams.erase(STREAMKEY(userid, STREAMTYPE_MEDIAFILE));
    m_activeStreams.erase(STREAMKEY(userid, STREAMTYPE_VOICE));
    m_activeStreams.erase(STREAMKEY(userid, STREAMTYPE_MEDIAFILE));

    m_streamstart.erase(STREAMKEY(userid, STREAMTYPE_VOICE));
    m_streamstart.erase(STREAMKEY(userid, STREAMTYPE_MEDIAFILE));
}

void ServerChannel::RemoveUser(int userid)
{
    RemoveUser(userid, nullptr);
}

void ServerChannel::UpdateChannelBans()
{
    for (auto& b : m_bans)
    {
        if ((b.bantype & BANTYPE_CHANNEL) != 0u)
            b.chanpath = GetChannelPath();
    }
}

void ServerChannel::SetOwner(const ServerUser& user)
{
    if (user.GetUserAccount().IsWebLogin())
        m_usernameOwner = user.GetUserAccount().username;
}

bool ServerChannel::IsOwner(const ServerUser& user) const
{
    return user.GetUserAccount().IsWebLogin() && m_usernameOwner == user.GetUserAccount().username;
}

bool ServerChannel::IsAutoOperator(const ServerUser& user) 
{
    return user.GetUserAccount().auto_op_channels.contains(user.GetUserID());
}

void ServerChannel::AddUserBan(const BannedUser& ban)
{
    RemoveUserBan(ban); m_bans.push_back(ban);
}

bool ServerChannel::IsBanned(const BannedUser& testban) const
{
    auto i = std::ranges::find_if(m_bans,
                          [testban](const BannedUser& ban)
                          {
                              return ban.Match(testban);
                          });
    return i != m_bans.end();
}

void ServerChannel::RemoveUserBan(const BannedUser& ban)
{
    auto i = std::ranges::find_if(m_bans,
                          [ban](const BannedUser& testban)
                          {
                              return ban.Same(testban);
                          });
    if(i != m_bans.end())
        m_bans.erase(i);
}
