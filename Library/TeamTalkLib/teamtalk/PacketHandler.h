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

#if !defined(PACKETHANDLER_H)
#define PACKETHANDLER_H

#include <ace/Reactor.h>
#include <ace/Event_Handler.h>
#include <ace/Message_Queue.h>
#include <ace/SOCK_Dgram.h>

#include <myace/MyACE.h>
#include <vector>
#include <set>

#include "PacketLayout.h"

typedef unsigned char byte_t;

namespace teamtalk {

#define PACKETBUFFER 0x10000

    class PacketListener
    {
    public:
        virtual void ReceivedPacket(class PacketHandler* ph,
                                    const char* data_buf, int data_len, 
                                    const ACE_INET_Addr& addr) = 0;
        virtual void SendPackets(){}
    };

    typedef std::set<PacketListener*> packetlisteners_t;

    class PacketQueue : private ACE_Message_Queue<ACE_MT_SYNCH>
    {
    public:
        PacketQueue();
        void Reset();
        void RemovePackets(PacketKind kind);
        void RemoveChannelPackets(uint16_t chanid);
        int QueuePacket(FieldPacket* packet);
        FieldPacket* GetNextPacket();
        int PacketCount();
    };

    class PacketHandler : public ACE_Event_Handler
    {
    public:
        PacketHandler(ACE_Reactor* r);
        virtual ~PacketHandler();

        bool open(const ACE_INET_Addr &addr, int recv_buf, int send_buf);
        bool close();

        void AddListener(teamtalk::PacketListener* pListener);
        void RemoveListener(teamtalk::PacketListener* pListener);

        //Callback to handle any input received
        int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
        int handle_output (ACE_HANDLE fd = ACE_INVALID_HANDLE);
        int handle_close(ACE_HANDLE, ACE_Reactor_Mask mask);

        //Used by the reactor to determine the underlying handle
        ACE_HANDLE get_handle() const;

        //Returns a reference to the underlying dgram socket.
        ACE_SOCK_Dgram& sock_i();

        ACE_INET_Addr GetLocalAddr() const { return m_localaddr; }

    private:
        ACE_SOCK_Dgram sock_;
        ACE_INET_Addr m_localaddr;
        packetlisteners_t m_setListeners;
        std::vector<char> m_buffer;
    };
}

#endif
