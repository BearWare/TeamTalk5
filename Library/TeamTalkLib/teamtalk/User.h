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

#if !defined(USER_H)
#define USER_H

#include "Common.h"
#include "PacketLayout.h"

#include <ace/SString.h>
#include <ace/INET_Addr.h>

#include <map>

namespace teamtalk {

    typedef std::map< int, TextMessage > messages_t;

    class User
    {
    public:
        User(int userid);
        virtual ~User();

        int GetUserID() const { return m_userid; }

        void SetNickname(const ACE_TString& name){m_nickname = name;}
        const ACE_TString& GetNickname() const {return m_nickname;}

        virtual const ACE_TString& GetUsername() const = 0;
        virtual UserTypes GetUserType() const = 0;
        virtual int GetUserData() const = 0;

        void SetIpAddress(const ACE_TString& ipaddr){ m_ipaddr = ipaddr; }
        const ACE_TString& GetIpAddress() const { return m_ipaddr; }

        void SetStatusMode(int nStatusMode){m_statusmode = nStatusMode;}
        int GetStatusMode() const { return m_statusmode; }

        void SetStatusMessage(const ACE_TString& msg){ m_statusmsg = msg; }
        ACE_TString GetStatusMessage() const { return m_statusmsg; }

        void SetClientVersion(const ACE_TString& version){m_version = version;}
        ACE_TString GetClientVersion() const {return m_version;}

        void SetPacketProtocol(int protocol){ m_packet_protocol = protocol; }
        int GetPacketProtocol() const { return m_packet_protocol; }

        void UpdateLastTimeStamp(ACE_UINT32 tm);
        void UpdateLastTimeStamp(PacketKind packet_kind, ACE_UINT32 tm);
        void UpdateLastTimeStamp(const FieldPacket& pkt);

        ACE_UINT32 GetLastTimeStamp(bool* is_set = NULL) const;
        ACE_UINT32 GetLastTimeStamp(const FieldPacket& pkt, bool* is_set = NULL) const;
        ACE_UINT32 GetLastTimeStamp(PacketKind packet_kind, bool* is_set = NULL) const;
        // MAX_PAYLOAD_DATA_SIZE
        void SetMaxDataChunkSize(int size) { m_mtu_data_size = size; }
        int GetMaxDataChunkSize() const { return m_mtu_data_size; }
        // MAX_PACKET_PAYLOAD_SIZE = MAX_PAYLOAD_DATA_SIZE + FIELDHEADER_PAYLOAD
        void SetMaxPayloadSize(int size) { m_mtu_max_payload_size = size; }
        int GetMaxPayloadSize() const { return m_mtu_max_payload_size; }

        void SetClientName(const ACE_TString& name) { m_clientname = name; }
        ACE_TString GetClientName() const { return m_clientname; }

    private:
        int m_userid = 0;
        int m_packet_protocol = 0;
        ACE_TString m_ipaddr;
        int m_statusmode = 0;
        ACE_TString m_statusmsg;
        bool m_tm_ok = false;
        ACE_UINT32 m_timestamp = 0;
        int m_mtu_data_size = MAX_PAYLOAD_DATA_SIZE, m_mtu_max_payload_size = MAX_PACKET_PAYLOAD_SIZE;
        typedef std::map<PacketKind, ACE_UINT32> packet_timestamps_t;
        packet_timestamps_t m_pkt_timestamps;
    protected:
        ACE_TString m_nickname;
        ACE_TString m_version;
        ACE_TString m_clientname;
    };
}
#endif
