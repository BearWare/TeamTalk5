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
#include "ttassert.h"
#include "Commands.h"
#include <vector>
#include <queue>

#include <ace/OS_NS_sys_socket.h>
using namespace std;
using namespace teamtalk;

PacketQueue::PacketQueue()
{
    this->high_water_mark(sizeof(FieldPacket*)*1000);
    this->low_water_mark(sizeof(FieldPacket*)*1000);
}

void PacketQueue::Reset()
{
    ACE_Time_Value tv;
    ACE_Message_Block* mb;
    while(dequeue(mb, &tv)>=0)
    {
        FieldPacket* p;
        memcpy(&p, mb->rd_ptr(), sizeof(p));
        mb->release();
        delete p;
    }
}

void PacketQueue::RemovePackets(PacketKind kind)
{
    std::queue<FieldPacket*> packets;
    FieldPacket* p;
    while((p = GetNextPacket()))
    {
        if(p->GetKind() == kind)
            delete p;
        else
            packets.push(p);
    }
    while(packets.size())
    {
        QueuePacket(packets.front());
        packets.pop();
    }
}

void PacketQueue::RemoveChannelPackets(uint16_t chanid)
{
    std::queue<FieldPacket*> packets;
    FieldPacket* p;
    while((p = GetNextPacket()))
    {
        if(p->GetChannel() == chanid)
            delete p;
        else
            packets.push(p);
    }
    while(packets.size())
    {
        QueuePacket(packets.front());
        packets.pop();
    }
}

FieldPacket* PacketQueue::GetNextPacket()
{
    FieldPacket* p = NULL;
    ACE_Message_Block* mb;
    ACE_Time_Value tv;
    if(this->dequeue(mb, &tv)>=0)
    {
        memcpy(&p, mb->rd_ptr(), sizeof(p));
        mb->release();
    }
    return p;
}


int PacketQueue::QueuePacket(FieldPacket* packet)
{
    ACE_Message_Block* mb;
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
    TTASSERT(r);
    m_buffer = new char[PACKETBUFFER];
}

PacketHandler::~PacketHandler()
{
    delete [] m_buffer;
}

bool PacketHandler::open(const ACE_INET_Addr &addr, int recv_buf, int send_buf)
{
    int ret = sock_.open(addr);

    TTASSERT(reactor());

    if(ret == 0 && reactor())
    {
        //Register the reactor to call back when incoming client connects
        ret = reactor()->register_handler(this, PacketHandler::READ_MASK);
        TTASSERT(ret != -1);
        priority(HI_PRIORITY);
        TTASSERT(ret != -1);
        //MYTRACE("PacketHandler %d opened successfully\n", get_handle());
        int ret = 0;
        ret = ACE_OS::setsockopt(sock_.get_handle(), SOL_SOCKET, SO_RCVBUF, 
            reinterpret_cast<const char*>(&recv_buf), sizeof(recv_buf));
        TTASSERT(ret == 0);
        ret = ACE_OS::setsockopt(sock_.get_handle(), SOL_SOCKET, SO_SNDBUF, 
            reinterpret_cast<const char*>(&send_buf), sizeof(send_buf));
        TTASSERT(ret == 0);
    }

    return ret == 0;
}

bool PacketHandler::close()
{
    if(reactor())
    {
        reactor()->remove_handler(this, PacketHandler::ALL_EVENTS_MASK | PacketHandler::DONT_CALL);
        //MYTRACE("PacketHandler %d closed\n", get_handle());
        int ret = sock_.close();
        return ret == 0;
    }
    else
        return false;
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
int PacketHandler::handle_input(ACE_HANDLE)
{
    //TRACE(LM_DEBUG,"Reading input\r\n");
    //receive the data
    ACE_INET_Addr addr;

    ssize_t ret = sock_i().recv(m_buffer, PACKETBUFFER, addr);
    if(ret > 0)
    {    
        packetlisteners_t::iterator ite;
        for(ite=m_setListeners.begin();ite != m_setListeners.end();ite++)
            (*ite)->ReceivedPacket(m_buffer, (int)ret, addr);
    }
    else
    {
        int err = ACE_OS::last_error();
        MYTRACE(ACE_TEXT("UDP receive failed from %s, errno: %d\n"), InetAddrToString(addr).c_str(), err);
    }

    return 0;
}

int PacketHandler::handle_output (ACE_HANDLE fd/* = ACE_INVALID_HANDLE*/)
{
    packetlisteners_t::iterator ite;
    for(ite=m_setListeners.begin();ite != m_setListeners.end();ite++)
        (*ite)->SendPackets();
    return 0;
}

int PacketHandler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
    TTASSERT(false);
    return 0;
}

//Used by the reactor to determine the underlying handle
ACE_HANDLE PacketHandler::get_handle() const 
{
    return this->sock_.get_handle();
}

//Returns a reference to the underlying socket.
ACE_SOCK_Dgram& PacketHandler::sock_i()
{
    return this->sock_;
}


