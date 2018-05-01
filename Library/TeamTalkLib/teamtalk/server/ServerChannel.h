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

#ifndef SERVERCHANNEL_H
#define SERVERCHANNEL_H

#include <teamtalk/Channel.h>
#include "ServerUser.h"

namespace teamtalk { 

    class ServerUser;

    class ServerChannel : public Channel<ServerChannel, ServerUser>
    {
    public:
        typedef Channel<ServerChannel, ServerUser> PARENT;
        ServerChannel(int channelid);
        ServerChannel(channel_t& parent, int channelid, const ACE_TString& name);
        //Used for channels with CHANNEL_SOLO_TRANSMIT 
        bool CanTransmit(int userid, StreamType txtype, int streamid);
        void RemoveUser(int userid);
        void ClearFromTransmitQueue(int userid);
    private:
        // userid -> last transmit time
        std::map<int, ACE_Time_Value> m_lastUserPacket;
        // userid -> stream id
        std::map<int, int> m_blockStreams, m_activeStreams;
    };
}

#endif /* SERVERCHANNEL_H */
