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

#include "User.h"

#include "PacketLayout.h"
#include "myace/MyACE.h"


namespace teamtalk {

    User::User(int userid)
        : m_userid(userid)
    {
    }

    User::~User()
    {
        MYTRACE(ACE_TEXT("~User() - \"%s\"\n"), m_nickname.c_str());
    }

    void User::UpdateLastTimeStamp(ACE_UINT32 tm)
    {
        if(!m_tm_ok || W32_GEQ(GetLastTimeStamp(), m_timestamp))
        {
            m_timestamp = tm;
            m_tm_ok = true;
        }
    }

    void User::UpdateLastTimeStamp(PacketKind packet_kind, ACE_UINT32 tm)
    {
        UpdateLastTimeStamp(tm);
        auto const ii = m_pkt_timestamps.find(packet_kind);
        if(ii == m_pkt_timestamps.end() || W32_GEQ(tm, ii->second))
            m_pkt_timestamps[packet_kind] = tm;
    }

    void User::UpdateLastTimeStamp(const FieldPacket& pkt)
    {
        UpdateLastTimeStamp((PacketKind)pkt.GetKind(), pkt.GetTime());
    }

    ACE_UINT32 User::GetLastTimeStamp(bool* is_set/* = NULL*/) const
    {
        if(is_set != nullptr)
            *is_set = m_tm_ok;
        return m_timestamp;
    }

    ACE_UINT32 User::GetLastTimeStamp(const FieldPacket& pkt, bool* is_set/* = NULL*/) const
    {
        return GetLastTimeStamp((PacketKind)pkt.GetKind(), is_set);
    }

    ACE_UINT32 User::GetLastTimeStamp(PacketKind packet_kind, bool* is_set/* = NULL*/) const
    {
        auto const ii = m_pkt_timestamps.find(packet_kind);
        if(ii != m_pkt_timestamps.end())
        {
            if(is_set != nullptr)
                *is_set = true;
            return ii->second;
        }

        if(is_set != nullptr)
            *is_set = false;
        return 0;
        //return GetLastTimeStamp(is_set);
    }

} // namespace teamtalk
