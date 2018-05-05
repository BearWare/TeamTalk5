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

#include <iostream>

#if defined(ENABLE_ENCRYPTION)
#include <openssl/rand.h>
#endif

using namespace std;
using namespace teamtalk;

ServerChannel::ServerChannel(int channelid) 
    : PARENT(channelid)
{
#if defined(ENABLE_ENCRYPTION)
    RAND_bytes(m_cryptkey, sizeof(m_cryptkey));
#endif
}

ServerChannel::ServerChannel(channel_t& parent, int channelid, const ACE_TString& name) 
    : PARENT(parent, channelid, name)
{ 
#if defined(ENABLE_ENCRYPTION)
    RAND_bytes(m_cryptkey, sizeof(m_cryptkey));
#endif
}

#define STREAMKEY(uid, tx) (((uid) << 16) | tx)

bool ServerChannel::CanTransmit(int userid, StreamType txtype, int streamid)
{
    m_activeStreams[ STREAMKEY(userid, txtype) ] = streamid;

    if(!PARENT::CanTransmit(userid, txtype))
    {
        if (m_chantype & CHANNEL_SOLO_TRANSMIT)
        {
            // If transmitter is head we have to trigger channel update
            ClearFromTransmitQueue(userid);

            // If transmitter has been disallowed by channel update
            // then block the previous stream
            int streamkey = STREAMKEY(userid, STREAMTYPE_VOICE);
            m_blockStreams[streamkey] = m_activeStreams[streamkey];
            streamkey = STREAMKEY(userid, STREAMTYPE_MEDIAFILE);
            m_blockStreams[streamkey] = m_activeStreams[streamkey];
        }
        return false;
    }

    if((m_chantype & CHANNEL_SOLO_TRANSMIT) &&
       (txtype & (STREAMTYPE_VOICE | STREAMTYPE_MEDIAFILE)))
    {
        //MYTRACE(ACE_TEXT(" %d -> %d. StreamID: %d\n"), userid, m_blockStreams[STREAMKEY(userid, txtype)], streamid);

        /* Don't allow user to transmit until a new stream (id) is started */
        if (userid && m_blockStreams[STREAMKEY(userid, txtype)] == streamid)
            return false;

        /* Update queue list */
        auto ite = std::find(m_transmitqueue.begin(), m_transmitqueue.end(), userid);
        if(ite == m_transmitqueue.end())
            m_transmitqueue.push_back(userid);

        m_lastUserPacket[userid] = ACE_OS::gettimeofday();

        /* Can transmit if head of queue, transmitted within the last 500 ms and started new stream */
        TTASSERT(m_transmitqueue.size());
        int first = *m_transmitqueue.begin();
        std::map<int, ACE_Time_Value>::const_iterator itePkt = m_lastUserPacket.find(first);
        if( itePkt->second + ACE_Time_Value(0, 500000) >= ACE_OS::gettimeofday())
        {
            return userid == first;
        }
        else
        {
            m_lastUserPacket.erase(first);

            int streamkey = STREAMKEY(first, STREAMTYPE_VOICE);
            m_blockStreams[streamkey] = m_activeStreams[streamkey];
            streamkey = STREAMKEY(first, STREAMTYPE_MEDIAFILE);
            m_blockStreams[streamkey] = m_activeStreams[streamkey];

            m_transmitqueue.erase(m_transmitqueue.begin());
            return CanTransmit(userid, txtype, streamid);
        }
    }
    return true;
}

void ServerChannel::ClearFromTransmitQueue(int userid)
{
    auto i = std::find(m_transmitqueue.begin(), m_transmitqueue.end(), userid);
    if(i != m_transmitqueue.end())
        m_transmitqueue.erase(i);
    m_lastUserPacket.erase(userid);
}

void ServerChannel::RemoveUser(int userid)
{
    PARENT::RemoveUser(userid);
    ClearFromTransmitQueue(userid);

    m_blockStreams.erase(STREAMKEY(userid, STREAMTYPE_VOICE));
    m_blockStreams.erase(STREAMKEY(userid, STREAMTYPE_MEDIAFILE));
    m_activeStreams.erase(STREAMKEY(userid, STREAMTYPE_VOICE));
    m_activeStreams.erase(STREAMKEY(userid, STREAMTYPE_MEDIAFILE));
}
