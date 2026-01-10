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

#include "PacketHandler.h"

#include "PacketLayout.h"
#include "TTAssert.h"
#include "myace/MyACE.h"
#include "myace/MyINet.h"

#include <ace/Event_Handler.h>
#include <ace/Message_Block.h>
#include <ace/Reactor.h>

#if defined(ACE_HAS_IPV6) && !defined(WIN32)
#include <netinet/in.h>
#endif

#include <cstddef>
#include <cstring>
#include <queue>
#include <vector>

using namespace teamtalk;

PacketQueue::PacketQueue()
{
    this->high_water_mark(sizeof(FieldPacket*)*1000);
    this->low_water_mark(sizeof(FieldPacket*)*1000);
}

void PacketQueue::Reset()
{
    ACE_Time_Value tv;
    ACE_Message_Block* mb = nullptr;
    while(dequeue(mb, &tv)>=0)
    {
        FieldPacket* p = nullptr;
        memcpy(&p, mb->rd_ptr(), sizeof(p));
        mb->release();
        delete p;
    }
}

void PacketQueue::RemoveChannelPackets()
{
    std::queue<FieldPacket*> packets;
    packet_ptr_t p;
    while((p = GetNextPacket()))
    {
        switch (p->GetKind())
        {
        case PACKET_KIND_HELLO :
        case PACKET_KIND_KEEPALIVE :
            packets.push(p.release());
            break;
        default :
            MYTRACE(ACE_TEXT("Packet of kind %d was removed from transmit queue\n"), p->GetKind());
            break;
        }
    }

    while(!packets.empty())
    {
        QueuePacket(packets.front());
        packets.pop();
    }
}

packet_ptr_t PacketQueue::GetNextPacket()
{
    FieldPacket* p = nullptr;
    ACE_Message_Block* mb = nullptr;
    ACE_Time_Value tv;
    if(this->dequeue(mb, &tv)>=0)
    {
        memcpy(&p, mb->rd_ptr(), sizeof(p));
        mb->release();
    }
    return packet_ptr_t(p);
}


int PacketQueue::QueuePacket(FieldPacket* packet)
{
    ACE_Message_Block* mb = nullptr;
    ACE_NEW_RETURN(mb, ACE_Message_Block(sizeof(packet)), -1);
    mb->copy((const char*)&packet, sizeof(packet));
    ACE_Time_Value tv;
    return this->enqueue(mb, &tv);
}

int PacketQueue::PacketCount()
{
    return (int)this->message_count();
}



PacketHandler::PacketHandler(ACE_Reactor* r)
: ACE_Event_Handler(r, HI_PRIORITY)
{
    //MYTRACE(ACE_TEXT("%p PacketHandler()\n"), this);
    TTASSERT(r);
    constexpr auto PACKETBUFFER = 0x10000;
    m_buffer.resize(PACKETBUFFER);
}

PacketHandler::~PacketHandler()
{
    MYTRACE(ACE_TEXT("~PacketHandler()\n"));
    Close();
}

bool PacketHandler::Open(const ACE_INET_Addr &addr)
{
    // Use the address' family so IPv6 endpoints get an IPv6 socket on
    // platforms (Android) that don't auto-upgrade AF_INET to AF_INET6.
    int family = addr.get_type();
    if (family == AF_UNSPEC || family == 0)
        family = ACE_PROTOCOL_FAMILY_INET;

    int ret = Socket().open(addr, family, 0, 1);
    TTASSERT(reactor());

    if (ret == 0)
    {
        //Register the reactor to call back when incoming client connects
        ret = reactor()->register_handler(this, PacketHandler::READ_MASK);
        TTASSERT(ret != -1);
#if defined(__ANDROID__) && defined(ACE_HAS_IPV6) && defined(IPV6_V6ONLY)
        // Allow dual stack when we bind IPv6, for backwards compatibility.
        if (family == AF_INET6)
        {
            int v6only = 0;
            int v6ret = Socket().set_option(IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));
            if (v6ret != 0)
            {
                MYTRACE(ACE_TEXT("Warning: Failed to set IPV6_V6ONLY=0 for UDP socket. Dual-stack may be disabled.\n"));
            }
        }
#endif /* __ANDROID__ */

        ret = Socket().get_local_addr(m_localaddr);
        TTASSERT(ret >= 0);
    }

    return ret == 0;
}

void PacketHandler::Close()
{
    reactor()->remove_handler(this, PacketHandler::ALL_EVENTS_MASK | PacketHandler::DONT_CALL);
    m_localaddr = ACE_INET_Addr();
    Socket().close();
}

void PacketHandler::AddListener(teamtalk::PacketListener* pListener)
{
    m_setListeners.insert(pListener);
}

void PacketHandler::RemoveListener(teamtalk::PacketListener* pListener)
{
    m_setListeners.erase(pListener);
}

//Called back to handle any input received
int PacketHandler::handle_input(ACE_HANDLE /*fd*/)
{
    //TRACE(LM_DEBUG,"Reading input\r\n");
    //receive the data
    ACE_INET_Addr addr;

    ssize_t const ret = Socket().recv(m_buffer.data(), m_buffer.size(), addr);
    if(ret > 0)
    {
        packetlisteners_t::iterator ite;
        for(ite=m_setListeners.begin();ite != m_setListeners.end();ite++)
            (*ite)->ReceivedPacket(this, m_buffer.data(), (int)ret, addr);
    }
    else
    {
        int const err = ACE_OS::last_error();
        MYTRACE(ACE_TEXT("UDP receive failed from %s, errno: %d\n"), InetAddrToString(addr).c_str(), err);
    }

    return 0;
}

int PacketHandler::handle_output (ACE_HANDLE  /*fd*//* = ACE_INVALID_HANDLE*/)
{
    packetlisteners_t::iterator ite;
    for(ite=m_setListeners.begin();ite != m_setListeners.end();ite++)
        (*ite)->SendPackets();
    return 0;
}

//Used by the reactor to determine the underlying handle
ACE_HANDLE PacketHandler::get_handle() const
{
    return m_sock.get_handle();
}

//Returns a reference to the underlying socket.
ACE_SOCK_Dgram& PacketHandler::Socket()
{
    return this->m_sock;
}

namespace teamtalk {

int ToIPTOSValue(const FieldPacket& p)
{
    switch (PacketKind(p.GetKind()))
    {
    case PACKET_KIND_HELLO :
    case PACKET_KIND_KEEPALIVE :
        return IP_TOS_SIGNALING;

    case PACKET_KIND_VOICE :
    case PACKET_KIND_VOICE_CRYPT :
        return IP_TOS_VOICE;

    case PACKET_KIND_VIDEO :
    case PACKET_KIND_VIDEO_CRYPT :
        return IP_TOS_VIDEO;

    case PACKET_KIND_MEDIAFILE_AUDIO :
    case PACKET_KIND_MEDIAFILE_AUDIO_CRYPT :
        return IP_TOS_MULTIMEDIA_AUDIO;

    case PACKET_KIND_MEDIAFILE_VIDEO :
    case PACKET_KIND_MEDIAFILE_VIDEO_CRYPT :
        return IP_TOS_MULTIMEDIA_VIDEO;

    case PACKET_KIND_DESKTOP :
    case PACKET_KIND_DESKTOP_CRYPT :
        return IP_TOS_DESKTOP;

    case PACKET_KIND_DESKTOP_ACK :
    case PACKET_KIND_DESKTOP_ACK_CRYPT :
    case PACKET_KIND_DESKTOP_NAK :
    case PACKET_KIND_DESKTOP_NAK_CRYPT :
        return IP_TOS_SIGNALING;

    case PACKET_KIND_DESKTOPCURSOR :
    case PACKET_KIND_DESKTOPCURSOR_CRYPT :
        return IP_TOS_DESKTOP;

    case PACKET_KIND_DESKTOPINPUT :
    case PACKET_KIND_DESKTOPINPUT_CRYPT :
    case PACKET_KIND_DESKTOPINPUT_ACK :
    case PACKET_KIND_DESKTOPINPUT_ACK_CRYPT :
        return IP_TOS_SIGNALING;
    }
    return IP_TOS_IGNORE;
}

} // namespace teamtalk

SocketOptGuard::SocketOptGuard(ACE_SOCK_Dgram& dgram, int level, int option, int value)
: m_dgram(dgram)
{
    int valsize = sizeof(m_value);
    if (dgram.get_option(level, option, &m_value, &valsize) == 0)
    {
        m_level = level;
        m_option = option;

        if (option == IP_TOS && value == IP_TOS_IGNORE)
        {
            m_level = 0;
        }
        else
        {
            int const ret = m_dgram.set_option(m_level, m_option, &value, sizeof(value));
            MYTRACE_COND(ret, ACE_TEXT("Failed to set socket level %d option %d\n"), level, option);
        }
    }
    else
    {
        MYTRACE(ACE_TEXT("Failed to get socket level %d option %d\n"), level, option);
    }
}

SocketOptGuard::~SocketOptGuard()
{
    if (m_level != 0)
    {
        int const ret = m_dgram.set_option(m_level, m_option, &m_value, sizeof(m_value));
        MYTRACE_COND(ret, ACE_TEXT("Failed to revert socket level %d option %d\n"), m_level, m_option);
    }
}
