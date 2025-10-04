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

#include "ClientNode.h"

#include "myace/MyACE.h"
#include "myace/MyINet.h"
#include "mystd/MyStd.h"
#include "teamtalk/CodecCommon.h"
#include "teamtalk/DesktopSession.h"
#include "teamtalk/ttassert.h"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <ctime>

#include <ace/OS.h>

#if defined(ENABLE_ENCRYPTION)
#include <openssl/rand.h>
#endif

using namespace teamtalk;
using namespace media;
using namespace soundsystem;
using namespace vidcap;

#define GEN_NEXT_ID(id) (++(id) == 0 ? ++(id) : (id))
#define GEN_NEXT_ID_MAX(id, max) ((id) + 1 > (max) ? (id) = 1 : (++(id) == 0 ? ++(id) : (id)))

constexpr auto VIDEOFILE_ENCODER_FRAMES_MAX         = 3;
constexpr auto VIDEOCAPTURE_ENCODER_FRAMES_MAX      = 3;
constexpr auto VIDEOCAPTURE_LOCAL_FRAMES_MAX        = 10;
constexpr auto UDP_SOCKET_RECV_BUF_SIZE             = 0x20000;
constexpr auto UDP_SOCKET_SEND_BUF_SIZE             = 0x20000;

constexpr auto LOCAL_USERID                         = 0; // Local user recording
constexpr auto MUX_USERID                           = 0x1001; // User ID for recording muxed stream
constexpr auto LOCAL_TX_USERID                      = 0x1002; // User ID for local user transmitting
constexpr auto LOCAL_MEDIAPLAYBACK_SESSIONID_MAX    = 16; // Max session id. Should be small because we
                                               // use all on STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO
                                               // in EnableAudioBlockCallback()

#define SIMULATE_RX_PACKETLOSS 0
#define SIMULATE_TX_PACKETLOSS 0

#if (SIMULATE_RX_PACKETLOSS || SIMULATE_TX_PACKETLOSS) && defined(NDEBUG)
#error Packetloss in release mode
#endif

ClientNode::ClientNode(const ACE_TString& version, ClientListener* listener)
                       : m_flags(CLIENT_CLOSED)
                       , m_connector(GetEventLoop(), ACE_NONBLOCK)
#if defined(ENABLE_ENCRYPTION)
                       , m_crypt_connector(GetEventLoop(), ACE_NONBLOCK)
#endif
                       , m_packethandler(GetEventLoop())
                       , m_version(version)
                       , m_listener(listener)
{
    MYTRACE(ACE_TEXT("ClientNode %p\n"), this);

    m_listener->RegisterEventSuspender(this);

    m_soundsystem = soundsystem::GetInstance();
    m_soundprop.soundgroupid = m_soundsystem->OpenSoundGroup();

    m_local_voicelog = std::make_shared<ClientUser>(LOCAL_USERID, this,
                                          m_listener, m_soundsystem);

    ResumeEventHandling();
}

ClientNode::~ClientNode()
{
    {
        //guard needed for disconnect since Logout and LeaveChannel are called
        GUARD_REACTOR(this);

        Disconnect();
        StopStreamingMediaFile();
        StopRecordingMuxedAudioFile();
        CloseVideoCapture();
        m_mediaplayback_streams.clear(); //clear all players before removing sound group
        CloseSoundInputDevice();
        CloseSoundOutputDevice();
        CloseSoundDuplexDevices();
    }

    m_soundsystem->RemoveSoundGroup(m_soundprop.soundgroupid);

#if defined(ENABLE_ENCRYPTION)
    CryptStreamHandler::RemoveSSLContext(GetEventLoop());
#endif

    //close reactor so no one can register new handlers
    SuspendEventHandling(true);

    MYTRACE(ACE_TEXT("~ClientNode %p\n"), this);
}

VoiceLogger& ClientNode::GetVoiceLogger()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if(!m_voicelogger)
        m_voicelogger = std::make_shared<VoiceLogger>(m_listener);

    return *m_voicelogger;
}

AudioContainer& ClientNode::GetAudioContainer()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    return m_audiocontainer;
}

bool ClientNode::GetServerInfo(ServerInfo& info)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if((m_flags & CLIENT_CONNECTED) != 0u)
    {
        info = m_serverinfo;
        return true;
    }
    return false;
}

bool ClientNode::GetClientStatistics(ClientStats& stats)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if((m_flags & CLIENT_CONNECTED) != 0u)
    {
        stats = m_clientstats;

        if(m_clientstats.udp_ping_dirty)
            stats.udpping_time = -1;
        else
            m_clientstats.udp_ping_dirty = true;

        if(m_clientstats.tcp_ping_dirty)
            stats.tcpping_time = -1;
        else
            m_clientstats.tcp_ping_dirty = true;

        return true;
    }
    return false;
}

clientchannel_t ClientNode::GetRootChannel()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    return m_rootchannel;
}

clientchannel_t ClientNode::GetMyChannel()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    return m_mychannel;
}

int ClientNode::GetChannelID()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if(m_mychannel)
        return m_mychannel->GetChannelID();
    return 0;
}

clientchannel_t ClientNode::GetChannel(int channelid)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    clientchannel_t c = GetRootChannel();
    if (c)
    {
        if((m_mychannel.get() != nullptr) && m_mychannel->GetChannelID() == channelid)
            return m_mychannel; //most likely scenario
        if(c->GetChannelID() == channelid)
            return c;
        else
            return c->GetSubChannel(channelid, true);//recursive (SLOW)
    }
    return {};
}

ACE_TString ClientNode::GetChannelPath(int channelid)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    ACE_TString chanpath;
    clientchannel_t const chan = GetChannel(channelid);
    if (chan)
        chanpath = chan->GetChannelPath();
    return chanpath;
}

bool ClientNode::GetChannelProp(int channelid, ChannelProp& prop)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    clientchannel_t const chan = GetChannel(channelid);
    if (chan)
    {
        prop = chan->GetChannelProp();
        return true;
    }
    return false;
}

clientuser_t ClientNode::GetUser(int userid, bool include_local/* = false*/)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    //fast search
    auto const ite = m_users.find(userid);
    if(ite != m_users.end())
        return ite->second;
    //most likely search
    clientuser_t suser;
    if(m_mychannel.get() != nullptr)
        suser = m_mychannel->GetUser(userid, false);
    //slow search
    if (!suser && m_rootchannel)
        suser = m_rootchannel->GetUser(userid, true);
    //special case for local user properties
    if(userid == LOCAL_USERID && include_local)
        return m_local_voicelog;

    return suser;
}

clientuser_t ClientNode::GetUserByUsername(const ACE_TString& username)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    //fast search
    auto ite = m_users.begin();
    while(ite != m_users.end())
    {
        if(ite->second->GetUsername() == username)
            return ite->second;
        ite++;
    }

    //with view-all-users disabled we need to check channels
    if((m_rootchannel.get() != nullptr) && 
        (m_myuseraccount.userrights & USERRIGHT_VIEW_ALL_USERS) == 0)
    {
        ClientChannel::users_t users;
        m_rootchannel->GetUsers(users, true);
        for(const auto & user : users)
        {
            if(user->GetUsername() == username)
                return user;
        }
    }
    return {};
}

void ClientNode::GetUsers(std::set<int>& userids)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    //fast search
    auto ite = m_users.begin();
    while(ite != m_users.end())
    {
        userids.insert(ite->first);
        ite++;
    }
    //with view-all-users disabled we need to check channels
    if((m_rootchannel.get() != nullptr) && 
       (m_myuseraccount.userrights & USERRIGHT_VIEW_ALL_USERS) == 0)
    {
        ClientChannel::users_t users;
        m_rootchannel->GetUsers(users, true);
        for(const auto & user : users)
            userids.insert(user->GetUserID());
    }
}

void ClientNode::UpdateKeepAlive(const ClientKeepAlive& keepalive)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    
    m_keepalive = keepalive;
    
    // set TCP keepalive (DoPing) to half of usertimeout
    m_keepalive.tcp_keepalive_interval = std::max(ACE_Time_Value(m_serverinfo.usertimeout / 2, 0),
                                                  ACE_Time_Value(1, 0));
    // if server has not "seen us" for twice the user timeout then
    // we've been disconnected anyway
    m_keepalive.connection_lost_timeout = ACE_Time_Value(m_serverinfo.usertimeout * 2, 0);

    bool restarttcp = false;
    bool restartudp = false;
    if (m_keepalive.tcp_keepalive_interval != keepalive.tcp_keepalive_interval)
    {
        if (TimerExists(TIMER_TCPKEEPALIVE_ID))
        {
            restarttcp = true;
            StopTimer(TIMER_TCPKEEPALIVE_ID);
        }
    }

    if (m_keepalive.udp_keepalive_interval != keepalive.udp_keepalive_interval ||
        m_keepalive.udp_keepalive_rtx != keepalive.udp_keepalive_rtx)
    {
        if (TimerExists(TIMER_UDPKEEPALIVE_ID))
        {
            restartudp = true;
            StopTimer(TIMER_UDPKEEPALIVE_ID);
        }
    }

    //reset keep alive counters (otherwise we might disconnect by mistake)
    m_clientstats.tcp_silence_sec = 0;
    m_clientstats.udp_silence_sec = 0;

    //only start keep alive timer if HandleWelcome has been called
    TTASSERT((m_serverinfo.usertimeout &&
              (m_flags & (CLIENT_CONNECTING | CLIENT_CONNECTED))) || 
             m_serverinfo.usertimeout == 0);
    
    if (restarttcp && (m_serverinfo.usertimeout != 0))
    {
        StartTimer(TIMER_TCPKEEPALIVE_ID, 0, ACE_Time_Value::zero,
                   m_keepalive.tcp_keepalive_interval);
    }

    // restart keep alive timer
    if (restartudp)
    {
        TTASSERT(!TimerExists(TIMER_UDPKEEPALIVE_ID));
        StartTimer(TIMER_UDPKEEPALIVE_ID, 0, ACE_Time_Value::zero,
                   m_keepalive.udp_keepalive_rtx);
    }
}

ClientKeepAlive ClientNode::GetKeepAlive()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    return m_keepalive;
}

//TimerListener
int ClientNode::TimerEvent(ACE_UINT32 timer_event_id, long userdata)
{
    GUARD_REACTOR(this);

#if defined(_DEBUG)
    //ensure timer isn't delete from inside a callback
    m_active_timerid = timer_event_id;
#endif

    int ret = -1;
    int const userid = TIMER_USERID(timer_event_id);

    TTASSERT(TimerExists(timer_event_id, userid));

    switch(timer_event_id & TIMERID_MASK)
    {
    case TIMER_UDPCONNECT_ID :
    {
        ACE_Time_Value const duration(m_clientstats.udp_silence_sec, 0);
        if (duration >= m_keepalive.udp_connect_timeout)
        {
            if ((m_flags & CLIENT_CONNECTING) != 0u)
                OnClosed();
            ret = -1;
        }
        else
        {
            SendPacket(HelloPacket(m_myuserid, GETTIMESTAMP()),
                       m_serverinfo.udpaddr);
            ret = 0;
        }
    }
    break;
    case TIMER_ONE_SECOND_ID :
        ret = TimerOneSecond();
        break;
    case TIMER_TCPKEEPALIVE_ID :
    {
        //send tcp keepalive
        DoPing(false);
        ret = 0;
    }
    break;
    case TIMER_UDPKEEPALIVE_ID :
        ret = TimerUdpKeepAlive();
        break;
    case TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID :
        ret = TimerDesktopPacketRtx();
        break;
    case TIMER_DESKTOPNAKPACKET_TIMEOUT_ID :
        ret = TimerDesktopNakPacket();
        break;
    case TIMER_BUILD_DESKTOPPACKETS_ID :
        ret = TimerBuildDesktopPackets();
        break;
    case TIMER_QUERY_MTU_ID :
        ret = TimerQueryMtu(userdata);
        break;
    case TIMER_STOP_AUDIOINPUT :
        m_audioinput_voice.reset();
        ret = -1;
        break;
    case TIMER_REMOVE_LOCALPLAYBACK :
        assert(m_mediaplayback_streams.size() <= LOCAL_MEDIAPLAYBACK_SESSIONID_MAX);
        for (auto i=m_mediaplayback_streams.begin(); i != m_mediaplayback_streams.end();)
        {
            switch (i->second->GetStatus())
            {
            case MEDIASTREAM_ERROR :
                m_mediaplayback_streams.erase(i++);
                break;
            case MEDIASTREAM_FINISHED :
                if (i->second->Flushed())
                    m_mediaplayback_streams.erase(i++);
                break;
            default :
                ++i;
                break;
            }
        }
        ret = m_mediaplayback_streams.empty() ? -1 : 0;
        break;
    case TIMER_STOP_STREAM_MEDIAFILE_ID :
        if (!m_mediafile_streamer)
            ret = -1;
        else if (m_mediafile_streamer->Completed())
        {
            StopStreamingMediaFile(false);
            ret = -1;
        }
        else
        {
            ret = 0;
        }
        break;
    case USER_TIMER_VOICE_PLAYBACK_ID :
    {
        clientuser_t const user = GetUser(userid);
        if(user.get() != nullptr)
            ret = user->TimerMonitorVoicePlayback();
        else
            ret = -1;
    }
    break;
    case USER_TIMER_JITTER_BUFFER_ID :
    {
        clientuser_t const user = GetUser(userid);
        if (user.get() != nullptr)
            ret = user->TimerVoiceJitterBuffer();
        else
            ret = -1;
    }
    break;
    case USER_TIMER_MEDIAFILE_AUDIO_PLAYBACK_ID :
    {
        clientuser_t const user = GetUser(userid);
        if(user.get() != nullptr)
            ret = user->TimerMonitorAudioFilePlayback();
        else
            ret = -1;
    }
    break;
    case USER_TIMER_MEDIAFILE_VIDEO_PLAYBACK_ID :
    {
        clientuser_t const user = GetUser(userid);
        if(user.get() != nullptr)
            ret = user->TimerMonitorVideoFilePlayback();
        else
            ret = -1;
    }
    break;
    case USER_TIMER_DESKTOPACKPACKET_ID :
    {
        clientuser_t const user = GetUser(userid);
        if(user.get() != nullptr)
            ret = user->TimerDesktopDelayedAck();
        else
            ret = -1;
    }
    break;
    case USER_TIMER_DESKTOPINPUT_RTX_ID :
    {
        clientuser_t const user = GetUser(userid);
        if (!user)
        {
            ret = -1;
            break;
        }

        //if user closed desktop session then just return
        desktop_viewer_t const desktop = user->GetDesktopSession();
        if (!desktop)
        {
            ret = -1;
            break;
        }
        //process desktop input RTX queue
        while(!user->GetDesktopInputRtxQueue().empty())
        {
            const DesktopInputPacket& p = *user->GetDesktopInputRtxQueue().front();
            //quit sending desktop input if user doesn't subscribe to it.
            if(!user->PeerSubscribes(p) ||
               (m_myuseraccount.userrights & USERRIGHT_TRANSMIT_DESKTOPINPUT) == USERRIGHT_NONE)
            {
                //clear everything since user won't ack our desktop input packets
                user->ResetDesktopInputTx();

                ret = -1;
                break;
            }
            //skip desktop input if it's for the wrong session
            if(desktop->GetSessionID() != p.GetSessionID())
            {
                user->GetDesktopInputRtxQueue().pop_front();
                continue;
            }

            MYTRACE(ACE_TEXT("Sending desktop input RTX. Session %d, pkt: %d\n"),
                    p.GetSessionID(), p.GetPacketNo());
            //queue desktop input packet for RTX
            DesktopInputPacket* packet = nullptr;
            ACE_NEW_RETURN(packet,
                           DesktopInputPacket(p),
                           0);
            if(!QueuePacket(packet))
                delete packet;
            ret = 0;
            break;
        }
        break;
    }
    case USER_TIMER_DESKTOPINPUT_ACK_ID :
    {
        ret = -1;

        clientuser_t const user = GetUser(userid);
        if (!user || !m_desktop || !m_mychannel)
            break;

        DesktopInputAckPacket* ack_pkt = nullptr;
        ACE_NEW_NORETURN(ack_pkt,
                         DesktopInputAckPacket(m_myuserid,
                                               GETTIMESTAMP(),
                                               m_desktop->GetSessionID(),
                                               user->GetNextDesktopInputRxPacketNo()-1));
        if(ack_pkt == nullptr)
            break;
        ack_pkt->SetChannel(m_mychannel->GetChannelID());
        ack_pkt->SetDestUser(userid);

        MYTRACE(ACE_TEXT("Sending desktop input ACK. Session %d. Pkt: %d\n"),
                ack_pkt->GetSessionID(), ack_pkt->GetPacketNo());
                
        if(!QueuePacket(ack_pkt))
            delete ack_pkt;

        break;
    }
    case USER_TIMER_REMOVE_FILETRANSFER_ID :
        m_filetransfers.erase(userdata);
        ret = -1;
        break;
    case USER_TIMER_UPDATE_USER :
    {
        clientuser_t const user = GetUser(userdata);
        if(user.get() != nullptr)
            m_listener->OnUserStateChange(*user);
        ret = -1;
    }
    break;
    default:
        TTASSERT(0);
        ret = -1;
        break;
    }

    //ensure timer is still there otherwise there will be a double delete
    TTASSERT(TimerExists(timer_event_id, userid));

#if defined(_DEBUG)
    //ensure timer isn't delete from inside a callback
    m_active_timerid = 0;
#endif

    if(ret < 0)
    {
        ClearTimer(timer_event_id);
    }

    return ret;
}

int ClientNode::TimerOneSecond()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    //Check server has replied to TCP keep alives and disconnect if inactive
    m_clientstats.tcp_silence_sec += 1;
    m_clientstats.udp_silence_sec += 1;

    //check whether server has replied within the timeout limit on TCP and within
    //TIMER_UDPKEEPALIVE_INTERVAL_MS * 3 on UDP
    if (ACE_Time_Value(m_clientstats.tcp_silence_sec, 0) >= m_keepalive.connection_lost_timeout ||
        (ACE_Time_Value(m_clientstats.udp_silence_sec, 0) >= m_keepalive.connection_lost_timeout &&
         m_serverinfo.udpaddr != ACE_INET_Addr()))
    {

        m_packethandler.Close();

#if defined(ENABLE_ENCRYPTION)
        if(m_crypt_stream != nullptr)
        {
            m_crypt_stream->close(); // automatically calls OnClosed()
        }
#endif
        if(m_def_stream != nullptr)
        {
            m_def_stream->close();
        }
    }

    return 0;
}

int ClientNode::TimerUdpKeepAlive()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    // try reconnecting on UDP
    if (ACE_Time_Value(m_clientstats.udp_silence_sec, 0) >= m_keepalive.udp_keepalive_interval * 2)
    {
        MYTRACE(ACE_TEXT("Recreating UDP socket due to connectivity problems\n"));
        RecreateUdpSocket();

        if (TimerExists(TIMER_UDPCONNECT_ID))
            StopTimer(TIMER_UDPCONNECT_ID);

        m_clientstats.udp_silence_sec = 0; // reset UDP timeout since we switch from keepalive to connect
        StartTimer(TIMER_UDPCONNECT_ID, 0, ACE_Time_Value::zero, m_keepalive.udp_connect_interval);
        
        return -1; // TIMER_UDPCONNECT_ID will take over
    }
    
    KeepAlivePacket const p(m_myuserid, GETTIMESTAMP());
    SendPacket(p, m_serverinfo.udpaddr);
    return 0;
}

int ClientNode::TimerBuildDesktopPackets()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    TTASSERT(m_desktop);
    if (!m_desktop)
        return -1;

    if(m_desktop->thr_count() != 0u)
        return 0;

    TTASSERT(m_desktop_tx);
    if (!m_desktop_tx)
        return -1;

    desktoppackets_t packets;
    m_desktop->GetDesktopPackets(packets);
    if(packets.empty())
        return -1;

    //calculate bytes to send
    int total_size = 0;
    int pk_size = 0;
    auto ii = packets.begin();
    while(ii != packets.end())
    {
        desktoppacket_t dp(*ii);
        m_desktop_tx->AddDesktopPacketToQueue(dp);

        total_size += (*ii)->GetPacketSize();
        pk_size = std::max<int>(pk_size, (*ii)->GetPacketSize());
        ii++;
    }
    //MYTRACE(ACE_TEXT("Packets ready: %u, Total size: %d, Biggest packet: %d\n"), 
    //        packets.size(), total_size, pk_size);

    int const session_id = m_desktop->GetSessionID();

    //get packets to send
    desktoppackets_t tx_packets;
    m_desktop_tx->GetNextDesktopPackets(tx_packets);

    auto dpi = tx_packets.begin();
    while(dpi != tx_packets.end())
    {
        DesktopPacket* dp = nullptr;
        ACE_NEW_RETURN(dp, DesktopPacket(*(*dpi)), true);
        if(!QueuePacket(dp))
        {
            delete dp;
            break;
        }
        //MYTRACE(ACE_TEXT("Queued desktop packet %d, %u\n"), 
        //    (*dpi)->GetPacketIndex(), GETTIMESTAMP());
        dpi++;
    }

    if(!TimerExists(TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID))
    {
        //start RTX timer
        ACE_Time_Value const rtx_interval(DESKTOP_RTX_TIMER_INTERVAL);
        
        if(StartTimer(TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID, 0,
                      rtx_interval, rtx_interval) < 0)
        {
            CloseDesktopSession(false);
            m_listener->OnDesktopTransferUpdate(0, 0);
            return 0; //CloseDesktopSession() will cancel TIMER_BUILD_DESKTOPPACKETS_ID
        }
    }

    //notify of bytes to send
    m_listener->OnDesktopTransferUpdate(session_id, total_size);

    return -1;
}

int ClientNode::TimerDesktopPacketRtx()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    //retransmit lost desktop packets
    TTASSERT(m_desktop_tx);
    if (m_desktop_tx)
    {
        ACE_Time_Value const rtx_timeout(DESKTOP_DEFAULT_RTX_TIMEOUT);
            /*GetDesktopPacketRTxTimeout(m_clientstats.udpping_time);*/
        desktoppackets_t rtx_packets;
        m_desktop_tx->GetLostDesktopPackets(rtx_timeout, rtx_packets, 1);
        auto dpi = rtx_packets.begin();
        while(dpi != rtx_packets.end())
        {
            DesktopPacket* dp = nullptr;
            ACE_NEW_RETURN(dp, DesktopPacket(*(*dpi)), 0);
            if(!QueuePacket(dp))
                delete dp;

            dpi++;
        }
    }
    return 0;
}

int ClientNode::TimerDesktopNakPacket()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if (m_desktop_nak_tx && m_mychannel)
    {
        DesktopNakPacket* nak_pkt = nullptr;
        ACE_NEW_RETURN(nak_pkt, DesktopNakPacket(GetUserID(), 
                                                 m_desktop_nak_tx->GetUpdateID(),
                                                 m_desktop_nak_tx->GetSessionID()), 0);

        nak_pkt->SetChannel(m_mychannel->GetChannelID());

        MYTRACE(ACE_TEXT("Sending NAK for session %d:%u\n"), 
                m_desktop_nak_tx->GetSessionID(),
                m_desktop_nak_tx->GetUpdateID());

        if(!QueuePacket(nak_pkt))
            delete nak_pkt;
    }
    return 0;
}

int ClientNode::TimerQueryMtu(int mtu_index)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    TTASSERT(mtu_index < MTU_QUERY_SIZES_COUNT);

    if(m_mtu_packets.size() >= MTU_QUERY_RETRY_COUNT)
    {
        m_mtu_packets.clear();
        //MTU query failed
        if(mtu_index == 0)
            m_listener->OnMTUQueryComplete(0);
        else
            m_listener->OnMTUQueryComplete(m_mtu_max_payload_size);
            
        return -1;
    }

    KeepAlivePacket* ka_pkt = nullptr;

    uint32_t const tm = GETTIMESTAMP();
    ACE_NEW_RETURN(ka_pkt, KeepAlivePacket(GetUserID(), tm, 
                                              MTU_QUERY_SIZES[mtu_index]), 0);

    ka_mtu_packet_t const pkt(ka_pkt);

    if(SendPacket(*pkt, m_serverinfo.udpaddr)>0)
        m_mtu_packets[tm] = pkt;

    return 0;
}

void ClientNode::RecreateUdpSocket()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    //recreate the UDP socket which has the server connection
    m_packethandler.Close();
    auto localaddr = GetLocalAddr();
    m_packethandler.Open(localaddr, UDP_SOCKET_RECV_BUF_SIZE, UDP_SOCKET_SEND_BUF_SIZE);
}

void ClientNode::OpenAudioCapture(const AudioCodec& codec)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    int const codec_samplerate = GetAudioCodecSampleRate(codec);
    int const codec_samples = GetAudioCodecCbSamples(codec);
    int const codec_channels = GetAudioCodecChannels(codec);

    rguard_t const g_snd(LockSndprop());

    if(codec_samples <= 0 || codec_samplerate <= 0 || codec_channels == 0 ||
       m_soundprop.inputdeviceid == SOUNDDEVICE_IGNORE_ID)
        return;
    
    // track duration of initial audio frame
    m_clientstats.streamcapture_delay_msec = 0;
    m_soundprop.samples_delay_msec = GETTIMESTAMP();

    bool opened = false;
    if ((m_flags & CLIENT_SNDINOUTPUT_DUPLEX) != 0u)
    {
        int samplerate = codec_samplerate;
        int output_channels = GetAudioCodecSimulateStereo(codec) ? 2 : codec_channels;

        // In order to support Windows AEC we let output device 
        // determine what resampler settings should be used. Input sample rate
        // is fixed in Windows AEC so we should let output device choose sample rate.
        // Also duplex mode in PortAudioWrapper is hacked on Windows' to make a fake
        // duplex device.
        DeviceInfo outdev;
        m_soundsystem->GetDevice(m_soundprop.outputdeviceid, outdev);
        if (!outdev.SupportsOutputFormat(output_channels, codec_samplerate))
        {
            output_channels = outdev.GetSupportedOutputChannels(output_channels);
            samplerate = outdev.default_samplerate; // duplex callback should use output device's sample rate
            media::AudioFormat infmt(samplerate, output_channels);
            media::AudioFormat outfmt(codec_samplerate, codec_channels);
            m_playback_resampler = MakeAudioResampler(infmt, outfmt);  //sample rate shared in dpx mode
            if (!m_playback_resampler)
            {
                m_playback_resampler.reset();
                m_listener->OnInternalError(TT_INTERR_SNDOUTPUT_FAILURE,
                                            ACE_TEXT("Cannot create resampler for sound output device"));
                return;
            }
            m_playback_buffer.resize(codec_samples * codec_channels);
        }

        // resample if output device already forced resampling or 
        // if input device doesn't support number of channels
        DeviceInfo indev;
        m_soundsystem->GetDevice(m_soundprop.inputdeviceid, indev);
        int const input_channels = indev.GetSupportedInputChannels(codec_channels);
        int samples = codec_samples;

        if (samplerate != codec_samplerate || input_channels != codec_channels)
        {
            media::AudioFormat infmt(samplerate, input_channels);
            media::AudioFormat outfmt(codec_samplerate, codec_channels);
            m_capture_resampler = MakeAudioResampler(infmt, outfmt);

            if (!m_capture_resampler)
            {
                m_capture_resampler.reset();
                m_listener->OnInternalError(TT_INTERR_SNDINPUT_FAILURE,
                    ACE_TEXT("Cannot create resampler for sound input device."));
                return;
            }
            m_capture_buffer.resize(codec_samples * codec_channels);

            samples = CalcSamples(codec_samplerate, codec_samples, samplerate);
        }


        opened = m_soundsystem->OpenDuplexStream(this, m_soundprop.inputdeviceid,
                                                 m_soundprop.outputdeviceid,
                                                 m_soundprop.soundgroupid, 
                                                 samplerate, input_channels,
                                                 output_channels, samples);
    }
    else
    {
        int input_samplerate = 0;
        int input_channels = 0;
        int input_samples = 0;

        if(!m_soundsystem->SupportsInputFormat(m_soundprop.inputdeviceid,
            codec_channels, codec_samplerate))
        {
            DeviceInfo dev;
            if(!m_soundsystem->GetDevice(m_soundprop.inputdeviceid, dev) ||
                dev.default_samplerate == 0)
            {
                m_listener->OnInternalError(TT_INTERR_SNDINPUT_FAILURE,
                    ACE_TEXT("Cannot open sound input device."));
                return;
            }

            //choose highest sample rate supported by device
            input_samplerate = dev.default_samplerate;
            //choose channels supported by device
            input_channels = dev.GetSupportedInputChannels(codec_channels);
            //get callback size for new samplerate
            input_samples = CalcSamples(codec_samplerate, codec_samples,
                input_samplerate);
            media::AudioFormat infmt(input_samplerate, input_channels);
            media::AudioFormat outfmt(codec_samplerate, codec_channels);
            m_capture_resampler = MakeAudioResampler(infmt, outfmt);

            if(!m_capture_resampler)
            {
                m_capture_resampler.reset();
                m_listener->OnInternalError(TT_INTERR_SNDINPUT_FAILURE,
                    ACE_TEXT("Cannot create resampler for sound input device."));
                return;
            }
            m_capture_buffer.resize(codec_samples * codec_channels);
        }
        else
        {
            input_samplerate = codec_samplerate;
            input_channels = codec_channels;
            input_samples = codec_samples;
        }

        opened = m_soundsystem->OpenInputStream(this, m_soundprop.inputdeviceid, 
                                                m_soundprop.soundgroupid,
                                                input_samplerate, input_channels, 
                                                input_samples);
    }

    if (!opened)
    {
        if (m_listener != nullptr)
            m_listener->OnInternalError(TT_INTERR_SNDINPUT_FAILURE,
                                        GetErrorDescription(TT_INTERR_SNDINPUT_FAILURE));
    }
}

void ClientNode::CloseAudioCapture()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if((m_flags & CLIENT_SNDINOUTPUT_DUPLEX) != 0u)
        m_soundsystem->CloseDuplexStream(this);
    else
        m_soundsystem->CloseInputStream(this);

    // if sending to AudioMuxer then submit 'end' frame
    media::AudioFrame frm;
    frm.sample_no = m_soundprop.samples_transmitted;
    frm.streamid = m_voice_stream_id;
    frm.userdata = STREAMTYPE_VOICE;
    AudioUserCallback(LOCAL_TX_USERID, STREAMTYPE_VOICE, frm);

    // submit ending of recording
    frm.sample_no = m_soundprop.samples_recorded;
    if (m_audiocontainer.AddAudio(LOCAL_USERID, STREAMTYPE_VOICE, frm))
    {
        m_listener->OnUserAudioBlock(LOCAL_USERID, STREAMTYPE_VOICE);
    }
    
    m_soundprop.samples_transmitted = 0;
    m_soundprop.samples_recorded = 0;

    // reset time of initial audio frame
    m_soundprop.samples_delay_msec = 0;
    m_clientstats.streamcapture_delay_msec = 0;

    //clear capture resampler if initiated (in duplex mode)
    m_capture_resampler.reset();
    m_capture_buffer.clear();
    //clear playback resampler if initiated (in duplex mode)
    m_playback_resampler.reset();
    m_playback_buffer.clear();
}

// Separate thread
void ClientNode::QueueAudioCapture(media::AudioFrame& audframe)
{
    bool const ptt_close = m_voice_tx_closed.exchange(false);
    audframe.force_enc = (((m_flags & CLIENT_TX_VOICE) != 0u) || ptt_close);
    audframe.voiceact_enc = ((m_flags & CLIENT_SNDINPUT_VOICEACTIVATED) != 0u);
    audframe.sample_no = m_soundprop.samples_recorded;
    m_soundprop.samples_recorded += audframe.input_samples;

    MYTRACE_COND(m_soundprop.samples_delay_msec, ACE_TEXT("%p Audio recorder delay: %u msec\n"),
                 this, GETTIMESTAMP() - m_soundprop.samples_delay_msec);

    if (m_soundprop.samples_delay_msec != 0u)
    {
        int delay = int(GETTIMESTAMP() - m_soundprop.samples_delay_msec);
        delay -= PCM16_SAMPLES_DURATION(audframe.input_samples, audframe.inputfmt.samplerate);
        m_clientstats.streamcapture_delay_msec = std::max(delay, 1); // put minimum 1 to indicate it was set
        m_soundprop.samples_delay_msec = 0;
    }

    if (!m_audioinput_voice)
        QueueVoiceFrame(audframe);
}

// Separate thread
void ClientNode::QueueVoiceFrame(media::AudioFrame& audframe,
                                 ACE_Message_Block* mb_audio)
{
    audframe.soundgrpid = m_soundprop.soundgroupid;
    audframe.userdata = STREAMTYPE_VOICE;
    audframe.streamid = m_voice_stream_id;

    if (m_audiocontainer.AddAudio(LOCAL_USERID, STREAMTYPE_VOICE, audframe))
    {
        m_listener->OnUserAudioBlock(LOCAL_USERID, STREAMTYPE_VOICE);
    }
    
    if (mb_audio != nullptr)
    {
        m_voice_thread.QueueAudio(mb_audio);
        // don't touch 'mb_audio' after this
    }
    else
        m_voice_thread.QueueAudio(audframe);
    }

void ClientNode::SendVoicePacket(const VoicePacket& packet)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    TTASSERT(packet.Finalized());

#ifdef ENABLE_ENCRYPTION
    if(m_crypt_stream != nullptr)
    {
        clientchannel_t const chan = GetChannel(packet.GetChannel());
        if (!chan)
            return;

        CryptVoicePacket const crypt_pkt(packet, chan->GetEncryptKey());
        if((m_myuseraccount.userrights & USERRIGHT_TRANSMIT_VOICE) != 0u)
            SendPacket(crypt_pkt, m_serverinfo.udpaddr);
        TTASSERT(crypt_pkt.ValidatePacket());
    }
    else
#endif
    {
        if((m_myuseraccount.userrights & USERRIGHT_TRANSMIT_VOICE) != 0u)
            SendPacket(packet, m_serverinfo.udpaddr);
        TTASSERT(packet.ValidatePacket());
    }

    //int ss = tx_pkt->GetPacketSize();
    //MYTRACE(ACE_TEXT("Packet no %d became %d bytes\n"),
    //    packet.GetPacketNumber(), packet.GetPacketSize());
}

void ClientNode::SendAudioFilePacket(const AudioFilePacket& packet)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    TTASSERT(packet.Finalized());
    TTASSERT(packet.GetKind() == PACKET_KIND_MEDIAFILE_AUDIO);

#ifdef ENABLE_ENCRYPTION
    if(m_crypt_stream != nullptr)
    {
        clientchannel_t const chan = GetChannel(packet.GetChannel());
        if (!chan)
            return;

        CryptAudioFilePacket const crypt_pkt(packet, chan->GetEncryptKey());
        if((m_myuseraccount.userrights & USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO) != 0u)
            SendPacket(crypt_pkt, m_serverinfo.udpaddr);
        TTASSERT(crypt_pkt.ValidatePacket());
    }
    else
#endif
    {
        MYTRACE_COND(!m_def_stream, ACE_TEXT("Sending UDP data on closed connected\n"));
        if((m_myuseraccount.userrights & USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO) != 0u)
            SendPacket(packet, m_serverinfo.udpaddr);
        TTASSERT(packet.ValidatePacket());
    }

    //int ss = tx_pkt->GetPacketSize();
    //MYTRACE(ACE_TEXT("Packet no %d became %d bytes\n"),
    //    packet.GetPacketNumber(), packet.GetPacketSize());
}


void ClientNode::EncodedAudioVoiceFrame(const teamtalk::AudioCodec& codec, 
                                        const char* enc_data, int enc_length,
                                        const std::vector<int>& enc_frame_sizes,
                                        const media::AudioFrame& org_frame)
{
    TTASSERT(org_frame.userdata);
    TTASSERT(org_frame.userdata == STREAMTYPE_VOICE);
    
    // Can't hold reactor lock here because it can cause a deadlock

    if(enc_length == 0)
    {
        if(((m_flags & CLIENT_SNDINPUT_VOICEACTIVE) != 0u) &&
           ((m_flags & CLIENT_SNDINPUT_VOICEACTIVATED) != 0u))
        {
            m_flags &= ~CLIENT_SNDINPUT_VOICEACTIVE;
            m_listener->OnVoiceActivated(false);
        }

        // submit 'end' frame to audio muxer
        media::AudioFrame frm;
        frm.sample_no = m_soundprop.samples_transmitted;
        frm.streamid = m_voice_stream_id;
        frm.userdata = STREAMTYPE_VOICE;
        AudioUserCallback(LOCAL_TX_USERID, STREAMTYPE_VOICE, frm);
        
        return;
    }
    if((m_flags & CLIENT_SNDINPUT_VOICEACTIVE) == 0 &&
            (m_flags & CLIENT_SNDINPUT_VOICEACTIVATED))
    {
        m_flags |= CLIENT_SNDINPUT_VOICEACTIVE;
        m_listener->OnVoiceActivated(true);

        if((m_flags & CLIENT_TX_VOICE) == 0)
            GEN_NEXT_ID(m_voice_stream_id);
    }

    // submit to audio muxer with sample index matching transmitted
    // (not recorded)
    media::AudioFrame cpyframe = org_frame;
    cpyframe.streamid = m_voice_stream_id;
    cpyframe.sample_no = m_soundprop.samples_transmitted;
    cpyframe.userdata = STREAMTYPE_VOICE;
    AudioUserCallback(LOCAL_TX_USERID, STREAMTYPE_VOICE, cpyframe);
    
    m_soundprop.samples_transmitted += org_frame.input_samples;

    MYTRACE_COND(enc_length > MAX_ENC_FRAMESIZE,
                 ACE_TEXT("Queue voice packet #%d at TS: %u, pkt time: %u, size: %d\n"),
                 m_voice_pkt_counter, GETTIMESTAMP(), org_frame.timestamp, enc_length);
    
    assert(enc_length <= MAX_ENC_FRAMESIZE);

    VoicePacket* newpacket = nullptr;
    if (GetAudioCodecFramesPerPacket(codec) > 1 && GetAudioCodecVariableFrameSizes(codec))
    {
        ACE_NEW(newpacket, 
                VoicePacket(PACKET_KIND_VOICE, m_myuserid, 
                            org_frame.timestamp, m_voice_stream_id, 
                            m_voice_pkt_counter++, enc_data, enc_length,
                            ConvertFrameSizes(enc_frame_sizes)));
    }
    else
    {
        ACE_NEW(newpacket, 
                VoicePacket(PACKET_KIND_VOICE, m_myuserid, 
                            org_frame.timestamp,  m_voice_stream_id, 
                            m_voice_pkt_counter++, enc_data, enc_length));
    }

    if(!QueuePacket(newpacket))
        delete newpacket;
}

void ClientNode::EncodedAudioFileFrame(const teamtalk::AudioCodec& codec, 
                                       const char* enc_data, int enc_length,
                                       const std::vector<int>& enc_frame_sizes,
                                       const media::AudioFrame& org_frame)
{
    assert(GetFlags() & CLIENT_STREAM_AUDIOFILE);
    TTASSERT(org_frame.userdata == STREAMTYPE_MEDIAFILE_AUDIO);
    
    AudioFilePacket* newpacket = nullptr;
    if (GetAudioCodecFramesPerPacket(codec) > 1 && GetAudioCodecVariableFrameSizes(codec))
    {
        ACE_NEW(newpacket, 
                AudioFilePacket(PACKET_KIND_MEDIAFILE_AUDIO, m_myuserid, 
                                org_frame.timestamp, m_mediafile_stream_id, 
                                m_audiofile_pkt_counter++, enc_data, enc_length,
                                ConvertFrameSizes(enc_frame_sizes)));
    }
    else
    {
        ACE_NEW(newpacket, 
                AudioFilePacket(PACKET_KIND_MEDIAFILE_AUDIO, m_myuserid, 
                                org_frame.timestamp,  m_mediafile_stream_id, 
                                m_audiofile_pkt_counter++, enc_data, enc_length));
    }

    if(!QueuePacket(newpacket))
        delete newpacket;
}


void ClientNode::StreamCaptureCb(const soundsystem::InputStreamer& /*streamer*/,
                                 const short* buffer, int n_samples)
{
    rguard_t const g_snd(LockSndprop());

    int const codec_samplerate = GetAudioCodecSampleRate(m_voice_thread.Codec());
    int const codec_samples = GetAudioCodecCbSamples(m_voice_thread.Codec());
    int const codec_channels = GetAudioCodecChannels(m_voice_thread.Codec());

    const short* capture_buffer = nullptr;
    if (m_capture_resampler)
    {
        assert((int)m_capture_buffer.size() == codec_samples * codec_channels);
        int const ret = m_capture_resampler->Resample(buffer, n_samples, 
                                                m_capture_buffer.data(),
                                                codec_samples);
        assert(ret > 0);
        assert(ret <= codec_samples);
        MYTRACE_COND(ret != codec_samples,
                     ACE_TEXT("Resampler output incorrect no. samples, expect %d, got %d\n"),
                     codec_samples, ret);
        capture_buffer = m_capture_buffer.data();
    }
    else
        capture_buffer = buffer;

    AudioFrame audframe(AudioFormat(codec_samplerate, codec_channels),
                        const_cast<short*>(capture_buffer), codec_samples);
    
    QueueAudioCapture(audframe);
}

void ClientNode::StreamDuplexCb(const soundsystem::DuplexStreamer& streamer,
                                    const short* input_buffer, 
                                    short* output_buffer, int n_samples)
{
    rguard_t const g_snd(LockSndprop());

    int const codec_samplerate = GetAudioCodecSampleRate(m_voice_thread.Codec());
    int const codec_samples = GetAudioCodecCbSamples(m_voice_thread.Codec());
    int const codec_channels = GetAudioCodecChannels(m_voice_thread.Codec());

    const short* capture_buffer = nullptr;
    if (m_capture_resampler)
    {
        assert((int)m_capture_buffer.size() == codec_samples * codec_channels);
        int const ret = m_capture_resampler->Resample(input_buffer, n_samples, 
                                                m_capture_buffer.data(),
                                                codec_samples);
        assert(ret > 0);
        assert(ret <= codec_samples);
        MYTRACE_COND(ret != codec_samples,
                     ACE_TEXT("Resampler output incorrect no. samples, expect %d, got %d\n"),
                     codec_samples, ret);

        capture_buffer = m_capture_buffer.data();
    }
    else
        capture_buffer = input_buffer;

    const short* playback_buffer = nullptr;
    if (m_playback_resampler)
    {
        assert((int)m_playback_buffer.size() == codec_samples * codec_channels);
        int const ret = m_playback_resampler->Resample(output_buffer, n_samples, 
                                                 m_playback_buffer.data(),
                                                 codec_samples);
        assert(ret>0);
        assert(ret <= codec_samples);
        MYTRACE_COND(ret != codec_samples,
                     ACE_TEXT("Resampler output incorrect no. samples, expect %d, got %d\n"),
                     codec_samples, ret);

        playback_buffer = m_playback_buffer.data();
    }
    else
        playback_buffer = output_buffer;

    AudioFrame audframe(AudioFormat(codec_samplerate, codec_channels),
                        const_cast<short*>(capture_buffer), codec_samples);
    
    audframe.outputfmt = AudioFormat(codec_samplerate, codec_channels);
    audframe.output_buffer = playback_buffer;
    audframe.output_samples = codec_samples;
    // Copy the last capture/render delay from the streamer will be used by WebRTC AEC
    audframe.duplex_callback_delay = streamer.last_duplex_callback_delay;

    QueueAudioCapture(audframe);
}

namespace teamtalk {
    SoundDeviceFeatures GetSoundDeviceFeatures(const SoundDeviceEffects& effects)
    {
        SoundDeviceFeatures features = SOUNDDEVICEFEATURE_NONE;

        if (effects.enable_aec)
            features |= SOUNDDEVICEFEATURE_AEC;
        if (effects.enable_agc)
            features |= SOUNDDEVICEFEATURE_AGC;
        if (effects.enable_denoise)
            features |= SOUNDDEVICEFEATURE_DENOISE;

        return features;
    }
} // namespace teamtalk

SoundDeviceFeatures ClientNode::GetCaptureFeatures()
{
    return GetSoundDeviceFeatures(m_soundprop.effects);
}

SoundDeviceFeatures ClientNode::GetDuplexFeatures()
{
    return GetCaptureFeatures();
}

bool ClientNode::VideoCaptureRGB32Callback(media::VideoFrame& video_frame,
                                           ACE_Message_Block* mb_video)
{
    if (mb_video == nullptr)
        mb_video = VideoFrameToMsgBlock(video_frame);

    ACE_Time_Value tm_zero;
    if (m_local_vidcapframes.enqueue(mb_video, &tm_zero) < 0)
    {
        //make room by deleting oldest video frame
        ACE_Message_Block* mb = nullptr;
        if(m_local_vidcapframes.dequeue(mb, &tm_zero) >= 0)
            mb->release();
        if(m_local_vidcapframes.enqueue(mb_video, &tm_zero) < 0)
            return false;
    }

    m_listener->OnUserVideoCaptureFrame(0, m_vidcap_stream_id);

    return true; //took ownership of 'org_frame'
}

bool ClientNode::VideoCaptureEncodeCallback(media::VideoFrame& video_frame,
                                            ACE_Message_Block* mb_video)
{
    // FOURCC of 'video_frame' must be RGB32 or I420

    if ((m_flags & CLIENT_TX_VIDEOCAPTURE) == CLIENT_CLOSED)
        return false;

    if(mb_video != nullptr)
    {
        m_vidcap_thread.QueueFrame(mb_video);
        return true;
    }
    
            m_vidcap_thread.QueueFrame(video_frame);
   
    return false;
}

bool ClientNode::VideoCaptureDualCallback(media::VideoFrame& video_frame,
                                          ACE_Message_Block* mb_video)
{
    if ((m_flags & CLIENT_TX_VIDEOCAPTURE) != 0u)
    {
        return VideoCaptureEncodeCallback(video_frame, mb_video);
    }
    
            return VideoCaptureRGB32Callback(video_frame, mb_video);
   
}

bool ClientNode::MediaStreamVideoCallback(media::VideoFrame& /*video_frame*/,
                                          ACE_Message_Block* mb_video)
{
    TTASSERT(m_flags & CLIENT_STREAM_VIDEOFILE);
    TTASSERT(m_videofile_thread);
    if (!m_videofile_thread)
        return false;

//    VideoFormat cap_format = m_videofile_thread->GetVideoFormat();
//     static int x = 1;
//     if((x++ % 200) == 0)
//         WriteBitmap(i2string(x) + ACE_TString(".bmp"), video_frame.width, video_frame.height, 4,
//                     video_frame.frame, video_frame.frame_length);
    m_videofile_thread->QueueFrame(mb_video);

    return true; //m_video_thread always takes ownership
}

bool ClientNode::MediaStreamAudioCallback(AudioFrame& audio_frame,
                                          ACE_Message_Block* mb_audio)
{
    TTASSERT(m_flags & CLIENT_STREAM_AUDIOFILE);
/*
    static int x = 0;
    static ACE_UINT32 start = GETTIMESTAMP();
    ACE_UINT32 now = GETTIMESTAMP();
    MYTRACE(ACE_TEXT("Sent %d msec at %u. Duration: %u. Frame time: %u\n"), 
        x += GetAudioCodecCbMillis(m_audiofile_thread.Codec()),
        GETTIMESTAMP(), now - start, audio_frame.timestamp);
    assert(audio_frame.timestamp < now + 2000);
*/
    AudioCodec const codec = m_audiofile_thread.Codec();

    TTASSERT(audio_frame.input_samples == GetAudioCodecCbSamples(codec));
    TTASSERT(audio_frame.inputfmt.channels);
    TTASSERT(audio_frame.input_buffer);
    TTASSERT(audio_frame.inputfmt.samplerate);
    TTASSERT(audio_frame.input_samples);

    audio_frame.force_enc = true;
    audio_frame.userdata = STREAMTYPE_MEDIAFILE_AUDIO;
    audio_frame.streamid = m_mediafile_stream_id;

    AudioUserCallback(LOCAL_USERID, STREAMTYPE_MEDIAFILE_AUDIO, audio_frame);

    m_audiofile_thread.QueueAudio(mb_audio);
    // don't touch 'mb_audio' after this

    return true;  //m_audiofile_thread always takes ownership
}

void ClientNode::MediaStreamStatusCallback(const MediaFileProp& mfp,
                                           MediaStreamStatus status)
{
    TTASSERT(m_flags & (CLIENT_STREAM_AUDIOFILE | CLIENT_STREAM_VIDEOFILE));

    MediaFileStatus mfs = MFS_CLOSED;

    switch(status)
    {
    case MEDIASTREAM_STARTED :
        mfs = MFS_STARTED;
        break;
    case MEDIASTREAM_PLAYING :
        mfs = MFS_PLAYING;
        break;
    case MEDIASTREAM_ERROR :
        mfs = MFS_ERROR;
        AudioUserCallback(LOCAL_USERID, STREAMTYPE_MEDIAFILE_AUDIO, media::AudioFrame());
        break;
    case MEDIASTREAM_FINISHED :
        mfs = MFS_FINISHED;
        AudioUserCallback(LOCAL_USERID, STREAMTYPE_MEDIAFILE_AUDIO, media::AudioFrame());
        break;
    case MEDIASTREAM_PAUSED :
        mfs = MFS_PAUSED;
        break;
    case MEDIASTREAM_NONE :
        assert(status != MEDIASTREAM_NONE);
        break;
    }

    m_listener->OnChannelStreamMediaFile(mfp, mfs);
}

bool ClientNode::AudioInputCallback(media::AudioFrame& audio_frame,
                                    ACE_Message_Block* mb_audio)
{
    assert(mb_audio);
    audio_frame.force_enc = true;
    QueueVoiceFrame(audio_frame);

    return false; // delete 'mb_audio'
}

void ClientNode::AudioInputStatusCallback(const AudioInputStatus& ais)
{
    if (ais.elapsed_msec == 0 && ais.queueduration_msec == 0)
    {
        //schedule timer to kill 'm_audioinput_voice'
        long const ret = StartTimer(TIMER_STOP_AUDIOINPUT, ais.streamid, ACE_Time_Value::zero);
        assert(ret >= 0);
    }

    m_listener->OnAudioInputStatus(m_voice_stream_id, ais);
}

void ClientNode::AudioMuxCallback(teamtalk::StreamTypes sts, const media::AudioFrame& audio_frame)
{
    if (m_audiocontainer.AddAudio(MUX_USERID, sts, audio_frame))
    {
        m_listener->OnUserAudioBlock(MUX_USERID, sts);
    }
}

void ClientNode::AudioUserCallback(int userid, StreamType st,
                                   const media::AudioFrame& audio_frame)
{
    m_channelrecord.QueueUserAudio(userid, st, audio_frame);

    // make copy since we're in a separate thread
    auto streammuxer = m_audiomuxer_stream;
    if (streammuxer)
        streammuxer->QueueUserAudio(userid, st, audio_frame);

    // ignore "terminate" audio frames (i.e. null buffers) and "not
    // talking" audio frames (i.e. streamid=0)
    if ((audio_frame.input_buffer != nullptr) && audio_frame.streamid != 0)
    {
        if (m_audiocontainer.AddAudio(userid, st, audio_frame))
        {
            // MYTRACE("Reporting #%d has stream %d type %d at %u\n",
            //         userid, audio_frame.streamid, st, audio_frame.sample_no);
            m_listener->OnUserAudioBlock(userid, st);
        }
    }
}

void ClientNode::OnFileTransferStatus(const teamtalk::FileTransfer& transfer)
{
    switch(transfer.status)
    {
    case FILETRANSFER_ERROR :
    case FILETRANSFER_FINISHED :
        //using transfer id as user id
        StartUserTimer(USER_TIMER_REMOVE_FILETRANSFER_ID, transfer.transferid,
                       transfer.transferid, ACE_Time_Value::zero);
        break;
    case FILETRANSFER_CLOSED :
    case FILETRANSFER_ACTIVE :
        break;
    }
    m_listener->OnFileTransferStatus(transfer);
}

bool ClientNode::GetTransferInfo(int transferid, FileTransfer& transfer)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    
    auto const ite = m_filetransfers.find(transferid);
    if(ite != m_filetransfers.end())
        transfer = ite->second->GetFileTransferInfo();
    return ite != m_filetransfers.end();
}

bool ClientNode::CancelFileTransfer(int transferid)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    
    auto const ite = m_filetransfers.find(transferid);
    if(ite != m_filetransfers.end())
    {
        m_filetransfers.erase(ite);
        return true;
    }
    return false;
}

void ClientNode::ReceivedPacket(PacketHandler* /*ph*/,
                                const char* packet_data, int packet_size, 
                                const ACE_INET_Addr& addr)
{
    ASSERT_REACTOR_THREAD(*GetEventLoop());

    GUARD_REACTOR(this);

    m_clientstats.udpbytes_recv += packet_size;

    MYTRACE_COND(m_serverinfo.udpaddr != addr, 
                 ACE_TEXT("Received packet not from server: Was %s != %s\n"),
                 InetAddrToString(addr).c_str(), InetAddrToString(m_serverinfo.udpaddr).c_str());

    if(m_serverinfo.udpaddr != addr)
        return;

    //used for validating that the UDP connections is still open
    FieldPacket const packet(packet_data, packet_size);

    // MYTRACE("Packet type %.2X with timestamp %u\n", packet.GetKind(), packet.GetTime());
                 
#if SIMULATE_RX_PACKETLOSS
    static int dropped = 0, received = 0;
    received++;
    if((ACE_OS::rand() % SIMULATE_RX_PACKETLOSS) == 0)
    {
        dropped++;
        MYTRACE(ACE_TEXT("Dropped RX packet kind %d from #%d, dropped %d/%d\n"), 
            (int)packet.GetKind(),(int)packet.GetSrcUserID(), dropped, received);
        return;
    }
#endif


#ifdef _DEBUG
    TTASSERT(packet.ValidatePacket());
#endif
    if(!packet.ValidatePacket())
        return;

    clientuser_t const user = GetUser(packet.GetSrcUserID());

    switch(packet.GetKind())
    {
    case PACKET_KIND_HELLO :
        ReceivedHelloAckPacket(HelloPacket(packet_data, packet_size), addr);
        return;
    case PACKET_KIND_KEEPALIVE :
        ReceivedKeepAliveReplyPacket(KeepAlivePacket(packet_data, packet_size), addr);
        return;
    }

    //FieldPackets are destined for channels
    FieldPacket const chanpacket(packet_data, packet_size);

    clientchannel_t const chan = GetChannel(chanpacket.GetChannel());
    if (!chan)
    {
        MYTRACE(ACE_TEXT("Received packet kind %d without a specified channel\n"), int(packet.GetKind()));
        return;
    }
    
    switch(chanpacket.GetKind())
    {
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_VOICE_CRYPT :
    {
        CryptVoicePacket const crypt_pkt(packet_data, packet_size);
        auto decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        MYTRACE_COND(!decrypt_pkt, ACE_TEXT("Failed to decrypted voice packet from #%d\n"),
                     crypt_pkt.GetSrcUserID());
        if(!decrypt_pkt)
            return;

        MYTRACE_COND(!user,
                     ACE_TEXT("Received crypt voice packet from unknown user #%d\n"),
                     packet.GetSrcUserID());
        m_clientstats.voicebytes_recv += packet_size;
        bool const no_record = ((chan->GetChannelType() & CHANNEL_NO_RECORDING) != 0u) &&
            (GetMyUserAccount().userrights & USERRIGHT_RECORD_VOICE) == USERRIGHT_NONE;
        if (user)
            user->AddVoicePacket(*decrypt_pkt, m_soundprop, !no_record);
    }
    break;
#endif
    case PACKET_KIND_VOICE :
    {
        VoicePacket const audio_pkt(packet_data, packet_size);
        MYTRACE_COND(!user,
                     ACE_TEXT("Received voice packet from unknown user #%d\n"),
                     packet.GetSrcUserID());
        m_clientstats.voicebytes_recv += packet_size;
        bool const no_record = ((chan->GetChannelType() & CHANNEL_NO_RECORDING) != 0u) &&
            (GetMyUserAccount().userrights & USERRIGHT_RECORD_VOICE) == USERRIGHT_NONE;
        if (user)
            user->AddVoicePacket(audio_pkt, m_soundprop, !no_record);
        break;
    }
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_MEDIAFILE_AUDIO_CRYPT :
    {
        CryptAudioFilePacket const crypt_pkt(packet_data, packet_size);
        auto decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        MYTRACE_COND(!decrypt_pkt, ACE_TEXT("Failed to decrypted audio packet from #%d\n"),
                     crypt_pkt.GetSrcUserID());
        if(!decrypt_pkt)
            return;
        MYTRACE_COND(!user,
                     ACE_TEXT("Received CryptAudioFilePacket from unknown user #%d"),
                     packet.GetSrcUserID());
        m_clientstats.mediafile_audio_bytes_recv += packet_size;
        if (user)
            user->AddAudioFilePacket(*decrypt_pkt, m_soundprop);
    }
    break;
#endif
    case PACKET_KIND_MEDIAFILE_AUDIO :
    {
        AudioFilePacket const audio_pkt(packet_data, packet_size);
        MYTRACE_COND(!user,
                     ACE_TEXT("Received AudioFilePacket from unknown user #%d"),
                     packet.GetSrcUserID());
        m_clientstats.mediafile_audio_bytes_recv += packet_size;
        if (user)
            user->AddAudioFilePacket(audio_pkt, m_soundprop);
        break;
    }
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_VIDEO_CRYPT :
    {
        CryptVideoCapturePacket const crypt_pkt(packet_data, packet_size);
        auto decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        MYTRACE_COND(!user,
                     ACE_TEXT("Received CryptVideoCapturePacket from unknown user #%d"),
                     packet.GetSrcUserID());
        m_clientstats.vidcapbytes_recv += packet_size;
        if (user)
            user->AddVideoCapturePacket(*decrypt_pkt, *chan);
    }
    break;
#endif
    case PACKET_KIND_VIDEO :
    {
        VideoCapturePacket const video_pkt(packet_data, packet_size);
        MYTRACE_COND(!user,
                     ACE_TEXT("Received VideoCapturePacket from unknown user #%d\n"),
                     packet.GetSrcUserID());
        m_clientstats.vidcapbytes_recv += packet_size;
        if (user)
            user->AddVideoCapturePacket(video_pkt, *chan);
    }
    break;
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_MEDIAFILE_VIDEO_CRYPT :
    {
        CryptVideoFilePacket const crypt_pkt(packet_data, packet_size);
        auto decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        MYTRACE_COND(!user,
                     ACE_TEXT("Received CryptVideoCapturePacket from unknown user #%d"),
                     packet.GetSrcUserID());
        m_clientstats.mediafile_video_bytes_recv += packet_size;
        if (user)
            user->AddVideoFilePacket(*decrypt_pkt, *chan);
    }
    break;
#endif
    case PACKET_KIND_MEDIAFILE_VIDEO :
    {
        VideoFilePacket const video_pkt(packet_data, packet_size);
        MYTRACE_COND(!user,
                     ACE_TEXT("Received VideoFilePacket from unknown user #%d\n"),
                     packet.GetSrcUserID());
        m_clientstats.mediafile_video_bytes_recv += packet_size;
        if (user)
            user->AddVideoFilePacket(video_pkt, *chan);
    }
    break;
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_DESKTOP_CRYPT :
    {
        CryptDesktopPacket const crypt_pkt(packet_data, packet_size);
        auto decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        MYTRACE_COND(!user,
                     ACE_TEXT("Received CryptDesktopPacket from unknown user #%d"),
                     packet.GetSrcUserID());

        m_clientstats.desktopbytes_recv += packet_size;
        if (user)
            ReceivedDesktopPacket(*user, *chan, *decrypt_pkt);
    }
    break;
#endif
    case PACKET_KIND_DESKTOP :
    {
        DesktopPacket const desktop_pkt(packet_data, packet_size);
        MYTRACE_COND(!user,
                     ACE_TEXT("Received DesktopPacket from unknown user #%d"),
                     packet.GetSrcUserID());

        m_clientstats.desktopbytes_recv += packet_size;
        if (user)
            ReceivedDesktopPacket(*user, *chan, desktop_pkt);
    }
    break;
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_DESKTOP_ACK_CRYPT :
    {
        TTASSERT(chan.get());
        CryptDesktopAckPacket const crypt_pkt(packet_data, packet_size);
        auto decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;

        ReceivedDesktopAckPacket(*decrypt_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
#endif
    case PACKET_KIND_DESKTOP_ACK :
    {
        DesktopAckPacket const ack_pkt(packet_data, packet_size);
        ReceivedDesktopAckPacket(ack_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_DESKTOP_NAK_CRYPT :
    {
        TTASSERT(chan.get());
        CryptDesktopNakPacket const crypt_pkt(packet_data, packet_size);
        auto decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        ReceivedDesktopNakPacket(*decrypt_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
#endif
    case PACKET_KIND_DESKTOP_NAK :
    {
        DesktopNakPacket const nak_pkt(packet_data, packet_size);
        ReceivedDesktopNakPacket(nak_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_DESKTOPCURSOR_CRYPT :
    {
        TTASSERT(chan.get());
        CryptDesktopCursorPacket const crypt_pkt(packet_data, packet_size);
        auto decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        ReceivedDesktopCursorPacket(*decrypt_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
#endif
    case PACKET_KIND_DESKTOPCURSOR :
    {
        DesktopCursorPacket const csr_pkt(packet_data, packet_size);
        ReceivedDesktopCursorPacket(csr_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
        break;
    }
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_DESKTOPINPUT_CRYPT :
    {
        TTASSERT(chan.get());
        CryptDesktopInputPacket const crypt_pkt(packet_data, packet_size);
        auto decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        ReceivedDesktopInputPacket(*decrypt_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
#endif
    case PACKET_KIND_DESKTOPINPUT :
    {
        DesktopInputPacket const csr_pkt(packet_data, packet_size);
        ReceivedDesktopInputPacket(csr_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
        break;
    }
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_DESKTOPINPUT_ACK_CRYPT :
    {
        CryptDesktopInputAckPacket const crypt_pkt(packet_data, packet_size);
        auto decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        ReceivedDesktopInputAckPacket(*decrypt_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
#endif
    case PACKET_KIND_DESKTOPINPUT_ACK :
    {
        DesktopInputAckPacket const ack_pkt(packet_data, packet_size);
        ReceivedDesktopInputAckPacket(ack_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
    default :
        MYTRACE_COND(user.get(),
                     ACE_TEXT("Received unknown packet type %d from #%d, %s\n"), 
                     (int)packet.GetKind(), user->GetUserID(), user->GetNickname().c_str());
        break;
    }
}


void ClientNode::ReceivedHelloAckPacket(const HelloPacket& packet,
                                        const ACE_INET_Addr& /*addr*/)
{
    ASSERT_REACTOR_THREAD(*GetEventLoop());
    ASSERT_CLIENTNODE_LOCKED(this);

    if( (m_flags & CLIENT_CONNECTING) != 0u)//server ACK
    {
        m_flags &= ~CLIENT_CONNECTING;
        m_flags |= CLIENT_CONNECTED;

        m_clientstats.udpping_time = GETTIMESTAMP() - packet.GetTime();
        m_clientstats.udp_ping_dirty = false;
        m_clientstats.udp_silence_sec = 0;

        m_serverinfo.packetprotocol = packet.GetProtocol();

        if(TimerExists(TIMER_UDPCONNECT_ID))
            StopTimer(TIMER_UDPCONNECT_ID);

        TTASSERT(!TimerExists(TIMER_UDPKEEPALIVE_ID));

        StartTimer(TIMER_UDPKEEPALIVE_ID, 0, m_keepalive.udp_keepalive_interval,
                   m_keepalive.udp_keepalive_rtx);
        
        //notify parent application
        if(m_listener != nullptr)
            m_listener->OnConnectSuccess();
    }
    else if ((m_flags & CLIENT_CONNECTED) != 0u)//server ACK
    {
        if(TimerExists(TIMER_UDPCONNECT_ID))
            StopTimer(TIMER_UDPCONNECT_ID);

        // reset UDP silence period
        m_clientstats.udp_silence_sec = 0;

        // Hello packet might come from recreated UDP connection
        if (!TimerExists(TIMER_UDPKEEPALIVE_ID))
        {
            StartTimer(TIMER_UDPKEEPALIVE_ID, 0, m_keepalive.udp_keepalive_interval,
                       m_keepalive.udp_keepalive_rtx);
        }
    }
}

void ClientNode::ReceivedKeepAliveReplyPacket(const KeepAlivePacket& packet,
                                              const ACE_INET_Addr& addr)
{
    ASSERT_REACTOR_THREAD(*GetEventLoop());
    ASSERT_CLIENTNODE_LOCKED(this);

    int const userid = packet.GetSrcUserID();
    MYTRACE( ACE_TEXT("Received UDP keepalive reply from #%d %s\n"), 
        userid, InetAddrToString(addr).c_str());

    //reset server keep aliver timeout
    m_clientstats.udp_silence_sec = 0;
    m_clientstats.udpping_time = GETTIMESTAMP() - packet.GetTime();
    m_clientstats.udp_ping_dirty = false;

    //this is a reply to a MTU query
    auto const ii = m_mtu_packets.find(packet.GetTime());
    if(ii != m_mtu_packets.end())
    {
        TTASSERT(ii->second->GetPayloadSize()>0);

        uint16_t const payload_size = ii->second->GetPayloadSize();
        MYTRACE(ACE_TEXT("MTU query %u reported reply to %d payload\n"),
                ii->second->GetTime(), payload_size);
        StopTimer(TIMER_QUERY_MTU_ID);

        m_mtu_packets.clear();

        m_mtu_data_size = payload_size;
        m_mtu_max_payload_size = m_mtu_data_size + FIELDHEADER_PAYLOAD;

        if(payload_size < MTU_QUERY_SIZES[MTU_QUERY_SIZES_COUNT-1])
        {
            size_t i = 0;
            for(i=1;i<MTU_QUERY_SIZES_COUNT;i++)
            {
                if(MTU_QUERY_SIZES[i] > payload_size)
                    break;
            }
            StartTimer(TIMER_QUERY_MTU_ID, long(i), ACE_Time_Value::zero,
                        CLIENT_QUERY_MTU_INTERVAL);
        }
        else
        {
            m_listener->OnMTUQueryComplete(m_mtu_max_payload_size);
        }
    }

    // restart keep alive timer
    if (TimerExists(TIMER_UDPKEEPALIVE_ID))
        StopTimer(TIMER_UDPKEEPALIVE_ID);
    
    StartTimer(TIMER_UDPKEEPALIVE_ID, 0, m_keepalive.udp_keepalive_interval,
               m_keepalive.udp_keepalive_rtx);
}

void ClientNode::ReceivedDesktopPacket(ClientUser& user,
                                       const ClientChannel& chan,
                                       const DesktopPacket& desktop_pkt)
{
    m_clientstats.desktopbytes_recv += desktop_pkt.GetPacketSize();
    user.AddPacket(desktop_pkt, chan); //ClientUser will schedule delayed ACK
}

void ClientNode::ReceivedDesktopAckPacket(const DesktopAckPacket& ack_pkt)
{
    uint8_t session_id = 0;
    uint32_t time_ack = 0;
    if(!ack_pkt.GetSessionInfo(nullptr, &session_id, &time_ack))
        return;

    if (m_desktop_tx &&
        m_desktop_tx->GetSessionID() == session_id &&
        m_desktop_tx->GetUpdateID() == time_ack)
    {
        uint16_t const packets_remain = m_desktop_tx->GetRemainingPacketsCount();
        m_desktop_tx->ProcessDesktopAckPacket(ack_pkt);

        //transmit new packets
        desktoppackets_t tx_packets;
        m_desktop_tx->GetDupAckLostDesktopPackets(tx_packets);
        m_desktop_tx->GetNextDesktopPackets(tx_packets);

        auto dpi = tx_packets.begin();
        while(dpi != tx_packets.end())
        {
            DesktopPacket* dp = nullptr;
            ACE_NEW(dp, DesktopPacket(*(*dpi)));
            if(!QueuePacket(dp))
            {
                delete dp;
                break;
            }

            dpi++;
        }

        int const new_remain_packets = m_desktop_tx->GetRemainingPacketsCount();
        int const new_remain_bytes = m_desktop_tx->GetRemainingBytes();

        if(m_desktop_tx->Done())
        {
            if(TimerExists(TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID))
                StopTimer(TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID);
            MYTRACE(ACE_TEXT("Desktop update %d:%u completed, duration %u msec\n"),
                    m_desktop_tx->GetSessionID(), m_desktop_tx->GetUpdateID(),
                    GETTIMESTAMP()-m_desktop_tx->GetUpdateID());
            m_desktop_tx.reset();

            m_flags &= ~CLIENT_TX_DESKTOP;
        }
        //send notify AFTER clearing of desktop tx
        if(std::cmp_not_equal(packets_remain , new_remain_packets))
            m_listener->OnDesktopTransferUpdate(session_id, new_remain_bytes);
    }
    else if (m_desktop_nak_tx)
    {
        //may be an ACK to a NAK
        if(TimerExists(TIMER_DESKTOPNAKPACKET_TIMEOUT_ID) &&
           session_id == m_desktop_nak_tx->GetSessionID() &&
           time_ack == m_desktop_nak_tx->GetUpdateID())
        {
           StopTimer(TIMER_DESKTOPNAKPACKET_TIMEOUT_ID);
           m_desktop_nak_tx.reset();

           MYTRACE(ACE_TEXT("Server ack'ed NAK to close desktop session %d:%u\n"),
                   session_id, time_ack);
        }
    }
}

void ClientNode::ReceivedDesktopNakPacket(const DesktopNakPacket& nak_pkt)
{
    ASSERT_REACTOR_THREAD(*GetEventLoop());

    clientuser_t const user = GetUser(nak_pkt.GetSrcUserID());
    MYTRACE_COND(!user, ACE_TEXT("Asked to delete desktop session #%d ")
                 ACE_TEXT("from user who doesn't exist\n"), nak_pkt.GetSessionID());
    if(!user)
        return;

    desktop_viewer_t const viewer = user->GetDesktopSession();
    MYTRACE_COND(!viewer, ACE_TEXT("Asked to delete desktop session #%d ")
                 ACE_TEXT("which doesn't exist from user #%d\n"), 
                 nak_pkt.GetSessionID(), nak_pkt.GetSrcUserID());

    if (!viewer)
        return;

    if(viewer->GetSessionID() == nak_pkt.GetSessionID())
    {
        MYTRACE(ACE_TEXT("Closing desktop session %d:%u for user #%d due to NAK\n"),
            nak_pkt.GetSessionID(), nak_pkt.GetTime(), user->GetUserID());
        user->CloseDesktopSession();
    }
}

void ClientNode::ReceivedDesktopCursorPacket(const DesktopCursorPacket& csr_pkt)
{
    clientuser_t const src_user = GetUser(csr_pkt.GetSrcUserID());
    clientchannel_t const chan = GetChannel(csr_pkt.GetChannel());
    uint16_t dest_userid = 0;
    uint8_t session_id = 0;
    if (chan && csr_pkt.GetSessionCursor(&dest_userid, &session_id, nullptr, nullptr))
    {
        if(dest_userid == 0)
        {
            if (src_user)
                src_user->AddPacket(csr_pkt, *chan);
        }
        else
        {
            clientuser_t const dest_user = GetUser(dest_userid);
            if (dest_user)
                dest_user->AddPacket(csr_pkt, *chan);
        }
    }
}

void ClientNode::ReceivedDesktopInputPacket(const DesktopInputPacket& di_pkt)
{
    ASSERT_REACTOR_THREAD(*GetEventLoop());

    clientuser_t const src_user = GetUser(di_pkt.GetSrcUserID());
    clientchannel_t const chan = GetChannel(di_pkt.GetChannel());
    if (!chan)
        return;

    if (!m_desktop || m_desktop->GetSessionID() != di_pkt.GetSessionID())
        return;

    MYTRACE(ACE_TEXT("Received desktop input from #%d session: %d, pktno: %u\n"),
            di_pkt.GetSrcUserID(), di_pkt.GetSessionID(),
            (ACE_UINT32)di_pkt.GetPacketNo());
    
    if (src_user)
        src_user->AddPacket(di_pkt, *chan);

    if(std::cmp_equal(di_pkt.GetDestUserID() , m_myuserid))
    {
        int const userid = src_user->GetUserID();
        if(!TimerExists(USER_TIMER_DESKTOPINPUT_ACK_ID, userid))
            StartUserTimer(USER_TIMER_DESKTOPINPUT_ACK_ID, userid, 0,
                            CLIENT_DESKTOPINPUT_ACK_DELAY,
                            CLIENT_DESKTOPINPUT_ACK_DELAY);
    }
}

void ClientNode::ReceivedDesktopInputAckPacket(const DesktopInputAckPacket& ack_pkt)
{
    ASSERT_REACTOR_THREAD(*GetEventLoop());

    int const userid = ack_pkt.GetSrcUserID();
    clientuser_t const user = GetUser(userid);
    if (!user)
        return;

    if(std::cmp_not_equal(ack_pkt.GetDestUserID() , m_myuserid))
        return;

    uint8_t session_id = 0;
    uint8_t packetno = 0;
    if(!ack_pkt.GetSessionInfo(&session_id, &packetno))
        return;

    MYTRACE(ACE_TEXT("Received desktop input ACK for user #%d, session %d, pkt %u. Remain: %u\n"),
            ack_pkt.GetSrcUserID(), (int)session_id, (ACE_UINT32)packetno,
            (ACE_UINT32)user->GetDesktopInputRtxQueue().size());

    while((!user->GetDesktopInputRtxQueue().empty()) &&
          user->GetDesktopInputRtxQueue().front()->GetSessionID() == session_id &&
          W8_LEQ(user->GetDesktopInputRtxQueue().front()->GetPacketNo(), packetno))
          user->GetDesktopInputRtxQueue().pop_front();

    //queue more for transmission (and place in rtx queue)
    while(user->GetDesktopInputRtxQueue().size() < DESKTOPINPUT_MAX_RTX_PACKETS &&
          (!user->GetDesktopInputTxQueue().empty()))
    {
        desktopinput_pkt_t const tx_pkt(user->GetDesktopInputTxQueue().front());
        DesktopInputPacket* packet = nullptr;
        ACE_NEW_NORETURN(packet,
                         DesktopInputPacket(*tx_pkt));
        if(!QueuePacket(packet))
            delete packet;
        else
        {
            user->GetDesktopInputRtxQueue().push_back(tx_pkt);
            user->GetDesktopInputTxQueue().pop_front();
        }
    }

    //MYTRACE(ACE_TEXT("Desktop input Tx queues, TX: %u, RTX: %u. Timer active: %d\n"),
    //        user->GetDesktopInputTxQueue().size(),
    //        user->GetDesktopInputRtxQueue().size(), 
    //        (int)TimerExists(USER_TIMER_DESKTOPINPUT_RTX_ID, userid));

    if(user->GetDesktopInputRtxQueue().empty())
    {
        TTASSERT(user->GetDesktopInputTxQueue().empty());
        if(TimerExists(USER_TIMER_DESKTOPINPUT_RTX_ID, userid))
        {
            StopUserTimer(USER_TIMER_DESKTOPINPUT_RTX_ID, userid);
        }
    }
    else
    {
        //restart retransmit, RTX, timer
        if(TimerExists(USER_TIMER_DESKTOPINPUT_RTX_ID, userid))
            StopUserTimer(USER_TIMER_DESKTOPINPUT_RTX_ID, userid);
        StartUserTimer(USER_TIMER_DESKTOPINPUT_RTX_ID, userid, 0,
                       CLIENT_DESKTOPINPUT_RTX_TIMEOUT,
                       CLIENT_DESKTOPINPUT_RTX_TIMEOUT);
    }
}

void ClientNode::SendPackets()
{
    ASSERT_REACTOR_THREAD(*GetEventLoop());

    GUARD_REACTOR(this);

    packet_ptr_t p;
    int ret = 0;
    while( (p = m_tx_queue.GetNextPacket()) )
    {
        // encoders in separate threads calls ACE_Reactor::notify() to
        // trigger send but they don't check if connection has been
        // dropped.
        MYTRACE_COND(!m_def_stream && !m_crypt_stream,
                     ACE_TEXT("Cannot send packet type %d on closed connection.\n"), int(p->GetKind()));

        if ((m_def_stream == nullptr) && (m_crypt_stream == nullptr))
            continue;
        
        switch(p->GetKind())
        {
        case PACKET_KIND_VOICE :
        {
            auto* audpkt = dynamic_cast<VoicePacket*>(p.get());
            TTASSERT(audpkt);
            TTASSERT(!audpkt->HasFragments());
            TTASSERT(!audpkt->Finalized());
            //a packet which isn't finalized will be transmitting to 'm_mychannel'
            if(!audpkt->Finalized())
            {
                //only transmit if we're in a channel, transmitting,
                //allowed to transmit
                if (!m_mychannel)
                    break;
                if(!m_mychannel->CanTransmit(m_myuserid, STREAMTYPE_VOICE))
                    break;
                
                if(((m_mychannel->GetChannelType() & CHANNEL_NO_VOICEACTIVATION) != 0u) &&
                   ((GetFlags() & CLIENT_SNDINPUT_VOICEACTIVATED) != 0u) &&
                   ((GetFlags() & CLIENT_SNDINPUT_VOICEACTIVE) != 0u))
                    break;

                audpkt->SetChannel(m_mychannel->GetChannelID());
            }

            bool const no_record = ((m_mychannel->GetChannelType() & CHANNEL_NO_RECORDING) != 0u) &&
                (GetMyUserAccount().userrights & USERRIGHT_RECORD_VOICE) == USERRIGHT_NONE;
            //store in voicelog
            if((!m_local_voicelog->GetAudioFolder().empty()) && !no_record)
                GetVoiceLogger().AddVoicePacket(*m_local_voicelog, *m_mychannel, *audpkt);

            //if packet is too big we turn it into fragments (packet protocol 2)
            audiopackets_t fragments = BuildAudioFragments(*audpkt, 
                                                           m_mtu_data_size);
            if(!fragments.empty())
            {
                for(const auto & fragment : fragments)
                    SendVoicePacket(*fragment);
            }
            else
                SendVoicePacket(*audpkt);
        }
        break;
        case PACKET_KIND_MEDIAFILE_AUDIO :
        {
            auto* audpkt = dynamic_cast<AudioFilePacket*>(p.get());
            TTASSERT(audpkt);
            TTASSERT(!audpkt->HasFragments());

            //only transmit if we're in a channel, transmitting,
            //allowed to transmit and there's active users
            if (!m_mychannel || 
               !m_mychannel->CanTransmit(m_myuserid, STREAMTYPE_MEDIAFILE))
                break;

            TTASSERT(audpkt->GetStreamID() == m_mediafile_stream_id);
            audpkt->SetChannel(m_mychannel->GetChannelID());

            //if packet is too big we turn it into fragments (packet protocol 2)
            audiopackets_t fragments = BuildAudioFragments(*audpkt, 
                                                           m_mtu_data_size);
            if(!fragments.empty())
            {
                for(const auto & fragment : fragments)
                    SendAudioFilePacket(*fragment);
            }
            else
                SendAudioFilePacket(*audpkt);
        }
        break;
        case PACKET_KIND_VIDEO :
        {
            auto* vidpkt = dynamic_cast<VideoCapturePacket*>(p.get());
            TTASSERT(vidpkt);

            if(!vidpkt->Finalized())
            {
                //only transmit if we're in a channel, transmitting and
                //there's active users
                if (!m_mychannel || 
                   (m_flags & CLIENT_TX_VIDEOCAPTURE) == 0 ||
                   !m_mychannel->CanTransmit(m_myuserid, STREAMTYPE_VIDEOCAPTURE))
                    break;

                TTASSERT(vidpkt->GetStreamID() == m_vidcap_stream_id);
                vidpkt->SetChannel(m_mychannel->GetChannelID());
            }

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream != nullptr)
            {
                clientchannel_t const chan = GetChannel(vidpkt->GetChannel());
                if (!chan)
                    break;
                CryptVideoCapturePacket const crypt_pkt(*vidpkt, chan->GetEncryptKey());
                if((m_myuseraccount.userrights & USERRIGHT_TRANSMIT_VIDEOCAPTURE) != 0u)
                    ret = SendPacket(crypt_pkt, m_serverinfo.udpaddr);
                TTASSERT(crypt_pkt.ValidatePacket());
            }
            else
#endif
            {
                TTASSERT(m_def_stream);
                if((m_myuseraccount.userrights & USERRIGHT_TRANSMIT_VIDEOCAPTURE) != 0u)
                    ret = SendPacket(*vidpkt, m_serverinfo.udpaddr);
                TTASSERT(vidpkt->ValidatePacket());
            }
        }
        break;
        case PACKET_KIND_MEDIAFILE_VIDEO :
        {
            auto* vidpkt = dynamic_cast<VideoFilePacket*>(p.get());
            TTASSERT(vidpkt);

            if(!vidpkt->Finalized())
            {
                //only transmit if we're in a channel, transmitting and
                //there's active users
                if (!m_mychannel || 
                   (m_flags & CLIENT_STREAM_VIDEOFILE) == 0 ||
                   !m_mychannel->CanTransmit(m_myuserid, STREAMTYPE_MEDIAFILE))
                    break;

                TTASSERT(vidpkt->GetStreamID());
                vidpkt->SetChannel(m_mychannel->GetChannelID());
            }

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream != nullptr)
            {
                clientchannel_t const chan = GetChannel(vidpkt->GetChannel());
                if (!chan)
                    break;
                CryptVideoFilePacket const crypt_pkt(*vidpkt, chan->GetEncryptKey());
                if((m_myuseraccount.userrights & USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO) != 0u)
                    ret = SendPacket(crypt_pkt, m_serverinfo.udpaddr);
                TTASSERT(crypt_pkt.ValidatePacket());
            }
            else
#endif
            {
                TTASSERT(m_def_stream);
                if((m_myuseraccount.userrights & USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO) != 0u)
                {
                    ret = SendPacket(*vidpkt, m_serverinfo.udpaddr);
                }
                TTASSERT(vidpkt->ValidatePacket());
            }
        }
        break;
        case PACKET_KIND_DESKTOP :
        {
            auto* desktoppkt = dynamic_cast<DesktopPacket*>(p.get());
            TTASSERT(desktoppkt);
            TTASSERT(!desktoppkt->Finalized());

            if (!m_mychannel ||
               !m_mychannel->CanTransmit(m_myuserid, STREAMTYPE_DESKTOP))
                break;

            //desktop update hasn't completed (non-null)
            if (!m_desktop_tx)
                break;

            //packet has been ack'ed since it was readded to the tx queue
            if(m_desktop_tx->IsDesktopPacketAcked(desktoppkt->GetPacketIndex()))
            {
                MYTRACE(ACE_TEXT("Skipped desktop packet %d\n"), 
                        desktoppkt->GetPacketIndex());
                break;
            }
            desktoppkt->SetChannel(m_mychannel->GetChannelID());

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream != nullptr)
            {
                CryptDesktopPacket const crypt_pkt(*desktoppkt, m_mychannel->GetEncryptKey());
                ret = 0;
                if((m_myuseraccount.userrights & USERRIGHT_TRANSMIT_DESKTOP) != 0u)
                    ret = SendPacket(crypt_pkt, m_serverinfo.udpaddr);
                TTASSERT(crypt_pkt.ValidatePacket());
            }
            else
#endif
            {
                TTASSERT(m_def_stream);
                ret = 0;
                if((m_myuseraccount.userrights & USERRIGHT_TRANSMIT_DESKTOP) != 0u)
                    ret = SendPacket(*desktoppkt, m_serverinfo.udpaddr);
                TTASSERT(desktoppkt->ValidatePacket());
            }
            //MYTRACE(ACE_TEXT("Sent desktop packet %d size %d, %u\n"), 
            //    desktoppkt->GetPacketIndex(), ret, desktoppkt->GetTime());
        }
        break;
        case PACKET_KIND_DESKTOP_ACK :
        {
            auto* ack_packet = dynamic_cast<DesktopAckPacket*>(p.get());
            TTASSERT(ack_packet);
            TTASSERT(ack_packet->Finalized());

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream != nullptr)
            {
                clientchannel_t const chan = GetChannel(ack_packet->GetChannel());
                if (!chan)
                    break;
                CryptDesktopAckPacket const crypt_pkt(*ack_packet, chan->GetEncryptKey());
                ret = SendPacket(crypt_pkt, m_serverinfo.udpaddr);
                TTASSERT(crypt_pkt.ValidatePacket());
            }
            else
#endif
            {
                TTASSERT(m_def_stream);
                ret = SendPacket(*ack_packet, m_serverinfo.udpaddr);
                TTASSERT(ack_packet->ValidatePacket());
            }
        }
        break;
        case PACKET_KIND_DESKTOP_NAK :
        {
            auto* nak_packet = dynamic_cast<DesktopNakPacket*>(p.get());
            TTASSERT(nak_packet);
            TTASSERT(nak_packet->Finalized());

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream != nullptr)
            {
                clientchannel_t const chan = GetChannel(nak_packet->GetChannel());
                if (!chan)
                    break;
                CryptDesktopNakPacket const crypt_pkt(*nak_packet, chan->GetEncryptKey());
                ret = SendPacket(crypt_pkt, m_serverinfo.udpaddr);
                TTASSERT(crypt_pkt.ValidatePacket());
            }
            else
#endif
            {
                TTASSERT(m_def_stream);
                ret = SendPacket(*nak_packet, m_serverinfo.udpaddr);
                TTASSERT(nak_packet->ValidatePacket());
            }
        }
        break;
        case PACKET_KIND_DESKTOPCURSOR :
        {
            auto* cursor_pkt = dynamic_cast<DesktopCursorPacket*>(p.get());
            TTASSERT(cursor_pkt);
            TTASSERT(cursor_pkt->Finalized());

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream != nullptr)
            {
                clientchannel_t const chan = GetChannel(cursor_pkt->GetChannel());
                if (!chan)
                    break;
                CryptDesktopCursorPacket const crypt_pkt(*cursor_pkt, chan->GetEncryptKey());
                ret = SendPacket(crypt_pkt, m_serverinfo.udpaddr);
                TTASSERT(crypt_pkt.ValidatePacket());
            }
            else
#endif
            {
                ret = SendPacket(*cursor_pkt, m_serverinfo.udpaddr);
                TTASSERT(cursor_pkt->ValidatePacket());
            }
        }
        break;
        case PACKET_KIND_DESKTOPINPUT :
        {
            auto* input_pkt = dynamic_cast<DesktopInputPacket*>(p.get());
            TTASSERT(input_pkt);
            TTASSERT(input_pkt->Finalized());

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream != nullptr)
            {
                clientchannel_t const chan = GetChannel(input_pkt->GetChannel());
                if (!chan)
                    break;
                CryptDesktopInputPacket const crypt_pkt(*input_pkt, chan->GetEncryptKey());
                ret = SendPacket(crypt_pkt, m_serverinfo.udpaddr);
                TTASSERT(crypt_pkt.ValidatePacket());
            }
            else
#endif
            {
                ret = SendPacket(*input_pkt, m_serverinfo.udpaddr);
                TTASSERT(input_pkt->ValidatePacket());
            }

            //MYTRACE(ACE_TEXT("Sent desktop input packet, %d:%u pkt no: %d\n"),
            //        cursor_pkt->GetSessionID(), cursor_pkt->GetTime(),
            //        cursor_pkt->GetPacketNo());
        }
        break;
        case PACKET_KIND_DESKTOPINPUT_ACK :
        {
            auto* ack_pkt = dynamic_cast<DesktopInputAckPacket*>(p.get());
            TTASSERT(ack_pkt);
            TTASSERT(ack_pkt->Finalized());

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream != nullptr)
            {
                clientchannel_t const chan = GetChannel(ack_pkt->GetChannel());
                if (!chan)
                    break;
                CryptDesktopInputAckPacket const crypt_pkt(*ack_pkt, chan->GetEncryptKey());
                ret = SendPacket(crypt_pkt, m_serverinfo.udpaddr);
                TTASSERT(crypt_pkt.ValidatePacket());
            }
            else
#endif
            {
                ret = SendPacket(*ack_pkt, m_serverinfo.udpaddr);
                TTASSERT(ack_pkt->ValidatePacket());
            }
        }
        break;
        default :
            TTASSERT(0);
            ret = SendPacket(*p, m_serverinfo.udpaddr);
        }
    }
    ACE_UNUSED_ARG(ret);
}

bool ClientNode::QueuePacket(FieldPacket* packet)
{
    bool const b = m_tx_queue.QueuePacket(packet) >= 0;

    MYTRACE_COND(!b, ACE_TEXT("Failed to queue packet kind %d to channel %d\n"),
                 (int)packet->GetKind(), (int)packet->GetChannel());

    ACE_Time_Value tv;
    auto *reactor = GetEventLoop();
    int const ret = reactor->notify(&m_packethandler, ACE_Event_Handler::WRITE_MASK, &tv);
    TTASSERT(ret>=0);
    return b;
}

//Send packet to address
int ClientNode::SendPacket(const FieldPacket& packet, const ACE_INET_Addr& addr)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    assert(packet.GetPacketSize() <= MAX_PACKET_SIZE);

#ifdef _DEBUG
    TTASSERT(packet.ValidatePacket());
#endif

#if SIMULATE_TX_PACKETLOSS
    static int dropped = 0, transmitted = 0;
    transmitted++;
    if((ACE_OS::rand() % SIMULATE_TX_PACKETLOSS) == 0)
    {
        dropped++;
        MYTRACE(ACE_TEXT("Dropped TX packet kind %d, dropped %d/%d\n"), 
                (int)packet.GetKind(), dropped, transmitted);
        return packet.GetPacketSize();
    }
#endif

    SocketOptGuard const sog(m_packethandler.Socket(), IPPROTO_IP, IP_TOS,
                       ToIPTOSValue(packet));
        
    //normal send without encryption
    int buffers = 0;
    const iovec* vv = packet.GetPacket(buffers);
    ssize_t const ret = m_packethandler.Socket().send(vv, buffers, addr);
    if(ret>0)
    {
        switch(packet.GetKind())
        {
        case PACKET_KIND_HELLO :
            MYTRACE(ACE_TEXT("Sent hello to %s\n"), 
                    InetAddrToString(addr).c_str());
            break;
        case PACKET_KIND_KEEPALIVE :
            MYTRACE(ACE_TEXT("Sent keepalive to %s\n"), 
                    InetAddrToString(addr).c_str());
            break;
        case PACKET_KIND_VOICE :
#ifdef ENABLE_ENCRYPTION
            TTASSERT(m_def_stream); //sending unencrypted
#endif
        case PACKET_KIND_VOICE_CRYPT :
            m_clientstats.voicebytes_sent += ret; break;
        case PACKET_KIND_VIDEO :
#ifdef ENABLE_ENCRYPTION
            TTASSERT(m_def_stream); //sending unencrypted
#endif
        case PACKET_KIND_VIDEO_CRYPT :
            m_clientstats.vidcapbytes_sent += ret; break;
        case PACKET_KIND_MEDIAFILE_AUDIO :
#ifdef ENABLE_ENCRYPTION
            TTASSERT(m_def_stream); //sending unencrypted
#endif
        case PACKET_KIND_MEDIAFILE_AUDIO_CRYPT :
            m_clientstats.mediafile_audio_bytes_sent += ret; break;
        case PACKET_KIND_MEDIAFILE_VIDEO :
#ifdef ENABLE_ENCRYPTION
            TTASSERT(m_def_stream); //sending unencrypted
#endif
        case PACKET_KIND_MEDIAFILE_VIDEO_CRYPT :
            m_clientstats.mediafile_video_bytes_sent += ret; break;
        case PACKET_KIND_DESKTOP :
        case PACKET_KIND_DESKTOP_ACK :
        case PACKET_KIND_DESKTOP_NAK :
#ifdef ENABLE_ENCRYPTION
            TTASSERT(m_def_stream); //sending unencrypted
#endif
        case PACKET_KIND_DESKTOP_CRYPT :
        case PACKET_KIND_DESKTOP_ACK_CRYPT :
        case PACKET_KIND_DESKTOP_NAK_CRYPT :
        case PACKET_KIND_DESKTOPCURSOR :
        case PACKET_KIND_DESKTOPCURSOR_CRYPT :
        case PACKET_KIND_DESKTOPINPUT :
        case PACKET_KIND_DESKTOPINPUT_CRYPT :
        case PACKET_KIND_DESKTOPINPUT_ACK :
        case PACKET_KIND_DESKTOPINPUT_ACK_CRYPT :
            m_clientstats.desktopbytes_sent += ret; break;
        default :
            MYTRACE(ACE_TEXT("Sending unknown packet type %d\n"), packet.GetKind());
            break;
        }
        m_clientstats.udpbytes_sent += ret;
    }
    else
    {
        MYTRACE(ACE_TEXT("Failed to UDP packet kind %d of size %d\n"),
                (int)packet.GetKind(), (int)packet.GetPacketSize());
    }

    return (int)ret;
}

bool ClientNode::InitSoundInputDevice(int inputdevice)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if((m_flags & CLIENT_SNDINPUT_READY) != 0u)
        return false;

    if(!m_soundsystem->CheckInputDevice(inputdevice))
        return false;

    rguard_t g_snd(LockSndprop());
    TTASSERT(m_soundprop.inputdeviceid == SOUNDDEVICE_IGNORE_ID);
    m_soundprop.inputdeviceid = inputdevice;
    g_snd.release();

    if (m_mychannel)
    {
        //launch recorders
        if(inputdevice != SOUNDDEVICE_IGNORE_ID)
            OpenAudioCapture(m_mychannel->GetAudioCodec());
    }
    m_flags |= CLIENT_SNDINPUT_READY;

    return true;
}

bool ClientNode::InitSoundOutputDevice(int outputdevice)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if((m_flags & CLIENT_SNDOUTPUT_READY) != 0u)
        return false;
    if(!m_soundsystem->CheckOutputDevice(outputdevice))
        return false;

    rguard_t g_snd(LockSndprop());

    TTASSERT(m_soundprop.outputdeviceid == SOUNDDEVICE_IGNORE_ID);
    m_soundprop.outputdeviceid = outputdevice;
    g_snd.release();

    m_flags |= CLIENT_SNDOUTPUT_READY;

    ResetAudioPlayers();

    return true;
}

bool ClientNode::InitSoundDuplexDevices(int inputdeviceid, 
                                        int outputdeviceid)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if(((m_flags & CLIENT_SNDINPUT_READY) != 0u) ||
       ((m_flags & CLIENT_SNDOUTPUT_READY) != 0u))
        return false; //already enabled
    if(!m_soundsystem->CheckInputDevice(inputdeviceid))
        return false;
    if(!m_soundsystem->CheckOutputDevice(outputdeviceid))
        return false;

    rguard_t g_snd(LockSndprop());

    TTASSERT(m_soundprop.inputdeviceid == SOUNDDEVICE_IGNORE_ID);
    TTASSERT(m_soundprop.outputdeviceid == SOUNDDEVICE_IGNORE_ID);
    m_soundprop.inputdeviceid = inputdeviceid;
    m_soundprop.outputdeviceid = outputdeviceid;
    g_snd.release();

    m_flags |= CLIENT_SNDINPUT_READY;
    m_flags |= CLIENT_SNDOUTPUT_READY;
    m_flags |= CLIENT_SNDINOUTPUT_DUPLEX;

    //restart audio capture in duplex mode
    if (m_mychannel)
        OpenAudioCapture(m_mychannel->GetAudioCodec());

    return true;
}

bool ClientNode::CloseSoundInputDevice()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if((m_flags & CLIENT_SNDINOUTPUT_DUPLEX) != 0u)
        return false;

    CloseAudioCapture();

    rguard_t g_snd(LockSndprop());
    m_soundprop.inputdeviceid = SOUNDDEVICE_IGNORE_ID;
    g_snd.release();

    m_flags &= ~CLIENT_SNDINPUT_READY;
    return true;
}

bool ClientNode::CloseSoundOutputDevice()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if((m_flags & CLIENT_SNDINOUTPUT_DUPLEX) != 0u)
        return false;

    rguard_t g_snd(LockSndprop());
    m_soundprop.outputdeviceid = SOUNDDEVICE_IGNORE_ID;
    g_snd.release();

    m_flags &= ~CLIENT_SNDOUTPUT_READY;

    ResetAudioPlayers();

    return true;
}

bool ClientNode::CloseSoundDuplexDevices()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if((m_flags & CLIENT_SNDINOUTPUT_DUPLEX) == 0)
        return false; //already enabled

    ResetAudioPlayers();

    //shutdown cature
    CloseAudioCapture();

    rguard_t g_snd(LockSndprop());
    m_soundprop.inputdeviceid = SOUNDDEVICE_IGNORE_ID;
    m_soundprop.outputdeviceid = SOUNDDEVICE_IGNORE_ID;
    g_snd.release();

    m_flags &= ~CLIENT_SNDINPUT_READY;
    m_flags &= ~CLIENT_SNDOUTPUT_READY;
    m_flags &= ~CLIENT_SNDINOUTPUT_DUPLEX;

    return true;
}

bool ClientNode::SoundDuplexMode()
{
    return (GetFlags() & CLIENT_SNDINOUTPUT_DUPLEX) != CLIENT_CLOSED;
}

bool ClientNode::SetSoundDeviceEffects(const SoundDeviceEffects& effects)
{
    {
        rguard_t const g_snd(LockSndprop());
        m_soundprop.effects = effects;
    }

    if ((m_flags & CLIENT_SNDINOUTPUT_DUPLEX) != 0u)
    {
        if (!m_soundsystem->IsStreamStopped(static_cast<StreamDuplex*>(this)))
            return m_soundsystem->UpdateStreamDuplexFeatures(this);
    }
    else if ((m_flags & CLIENT_SNDINPUT_READY) != 0u)
    {
        if (!m_soundsystem->IsStreamStopped(static_cast<StreamCapture*>(this)))
            return m_soundsystem->UpdateStreamCaptureFeatures(this);
    }
    return true;
}

SoundDeviceEffects ClientNode::GetSoundDeviceEffects()
{
    rguard_t const g_snd(LockSndprop());

    return m_soundprop.effects;
}

bool ClientNode::SetSoundOutputVolume(int volume)
{
    rguard_t const g_snd(LockSndprop());
    return m_soundsystem->SetMasterVolume(m_soundprop.soundgroupid, volume);
}

int ClientNode::GetSoundOutputVolume()
{
    rguard_t const g_snd(LockSndprop());
    return m_soundsystem->GetMasterVolume(m_soundprop.soundgroupid);
}

bool ClientNode::EnableVoiceTransmission(bool enable)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if (enable)
    {
        // don't allow voice transmission during audio input
        if (m_audioinput_voice)
            return false;
        
        m_flags |= CLIENT_TX_VOICE;

        //don't increment stream id if voice activated and voice active
        if ((m_flags & CLIENT_SNDINPUT_VOICEACTIVATED) == CLIENT_CLOSED ||
            (((m_flags & CLIENT_SNDINPUT_VOICEACTIVATED) != 0u) &&
             ((m_flags & CLIENT_SNDINPUT_VOICEACTIVE) != 0u)))
        {
            GEN_NEXT_ID(m_voice_stream_id);
        }
    }
    else
    {
        m_voice_tx_closed = ((m_flags & CLIENT_TX_VOICE) != 0u);
        m_flags &= ~CLIENT_TX_VOICE;
    }
    
    return true;
}

int ClientNode::GetCurrentVoiceLevel()
{
    return m_voice_thread.GetCurrentVoiceLevel();
}

bool ClientNode::EnableVoiceActivation(bool enable)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if (enable)
    {
        // don't allow voice activation during audio input
        if (m_audioinput_voice)
            return false;
    
        m_flags |= CLIENT_SNDINPUT_VOICEACTIVATED;
    }
    else
    {
        m_flags &= ~CLIENT_SNDINPUT_VOICEACTIVATED;
        m_flags &= ~CLIENT_SNDINPUT_VOICEACTIVE;
    }

    return true;
}
void ClientNode::SetVoiceActivationLevel(int voicelevel)
{
    m_voice_thread.m_voiceactlevel = voicelevel;
}
int ClientNode::GetVoiceActivationLevel() const
{
    return m_voice_thread.m_voiceactlevel;
}
void ClientNode::SetVoiceActivationStoppedDelay(int msec)
{
    m_voice_thread.m_voiceact_delay = ACE_Time_Value(msec / 1000, 
                                                     (msec % 1000) * 1000);
}
int ClientNode::GetVoiceActivationStoppedDelay() const
{
    return (int)m_voice_thread.m_voiceact_delay.msec();
}

bool ClientNode::EnableAutoPositioning(bool enable)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if((m_flags & CLIENT_SNDINOUTPUT_DUPLEX) != 0u)
        return false;

    if(enable)
        m_flags |= CLIENT_SNDOUTPUT_AUTO3DPOSITION;
    else
        m_flags &= ~CLIENT_SNDOUTPUT_AUTO3DPOSITION;
    return m_soundsystem->SetAutoPositioning(m_soundprop.soundgroupid, enable);
}

bool ClientNode::AutoPositionUsers()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if((m_flags & CLIENT_SNDINOUTPUT_DUPLEX) != 0u)
        return false;
    return m_soundsystem->AutoPositionPlayers(m_soundprop.soundgroupid, true);
}

bool ClientNode::EnableAudioBlockCallback(int userid, StreamTypes sts,
                                          const media::AudioFormat& outfmt,
                                          bool enable)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if (enable)
    {
        if (userid == LOCAL_USERID && (sts == STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO))
        {
            for (int i=1;i<=LOCAL_MEDIAPLAYBACK_SESSIONID_MAX;++i)
                m_audiocontainer.AddAudioSource(i, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, outfmt);
        }
        else
        {
            m_audiocontainer.AddAudioSource(userid, sts, outfmt);
        }
    }
    else
    {
        if (userid == LOCAL_USERID && (sts == STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO))
        {
            for (int i=1;i<=LOCAL_MEDIAPLAYBACK_SESSIONID_MAX;++i)
                m_audiocontainer.RemoveAudioSource(i, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO);
        }
        else
        {
            m_audiocontainer.RemoveAudioSource(userid, sts);
        }
    }

    if (userid == MUX_USERID)
    {
        if (enable)
        {
            audiomuxer_t const newmuxer(new AudioMuxer(sts));
            media::AudioInputFormat infmt;

            if (outfmt.IsValid())
                infmt = media::AudioInputFormat(outfmt, PCM16_DURATION_SAMPLES(20, outfmt.samplerate));
            else if (m_mychannel)
            {
                auto codec = m_mychannel->GetAudioCodec();
                infmt = GetAudioCodecAudioInputFormat(codec);
            }

            if (infmt.IsValid())
            {
                bool const startmux = newmuxer->RegisterMuxCallback(infmt,[this](auto && PH1, auto && PH2) { AudioMuxCallback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
                MYTRACE_COND(!startmux, ACE_TEXT("Failed to start audio muxer\n"));
                if (!startmux)
                    return false;
            }
            else
            {
                return false;
            }

            m_audiomuxer_stream = newmuxer;
        }
        else
        {
            m_audiomuxer_stream.reset();
        }
    }

    return true;
}

bool ClientNode::QueueAudioInput(const media::AudioFrame& frm, int streamid)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if ((GetFlags() & (CLIENT_TX_VOICE | CLIENT_SNDINPUT_VOICEACTIVATED)) != 0u)
        return false;

    if (!m_mychannel)
        return false;

    if (TimerExists(TIMER_STOP_AUDIOINPUT))
    {
        StopTimer(TIMER_STOP_AUDIOINPUT);
        m_audioinput_voice.reset();
    }

    if (frm.input_samples == 0)
    {
        if (m_audioinput_voice)
            m_audioinput_voice->Flush();

        return true;
    }

    if (m_audioinput_voice && streamid != m_audioinput_voice->GetStreamID())
        m_audioinput_voice.reset();

    if (!m_audioinput_voice)
    {
        media::AudioFormat const infmt = GetAudioCodecAudioFormat(m_mychannel->GetAudioCodec());
        int const cbsamples = GetAudioCodecCbSamples(m_mychannel->GetAudioCodec());

        m_audioinput_voice = std::make_shared<AudioInputStreamer>(streamid, MediaStreamOutput(infmt, cbsamples));

        m_audioinput_voice->RegisterAudioCallback([this](auto && PH1, auto && PH2) { return AudioInputCallback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }, true);
        m_audioinput_voice->RegisterAudioInputStatusCallback([this](auto && PH1) { AudioInputStatusCallback(std::forward<decltype(PH1)>(PH1)); }, true);

        if (!m_audioinput_voice->Open() || !m_audioinput_voice->StartStream())
        {
            m_audioinput_voice.reset();
            return false;
        }

        GEN_NEXT_ID(m_voice_stream_id);
    }

    return m_audioinput_voice->InsertAudio(frm);
}

bool ClientNode::MuteAll(bool muteall)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if(muteall)
        m_flags |= CLIENT_SNDOUTPUT_MUTE;
    else
        m_flags &= ~CLIENT_SNDOUTPUT_MUTE;

    rguard_t const g_snd(LockSndprop());
    return m_soundsystem->MuteAll(m_soundprop.soundgroupid, muteall);
}

bool ClientNode::SetVoiceGainLevel(int gainlevel)
{
    rguard_t const g_snd(LockSndprop());

    switch (m_soundprop.preprocessor.preprocessor)
    {
    case AUDIOPREPROCESSOR_TEAMTALK :
        m_soundprop.preprocessor.ttpreprocessor.gainlevel = gainlevel; //cache vvalue
        return SetSoundPreprocess(m_soundprop.preprocessor);
    case AUDIOPREPROCESSOR_NONE :
    case AUDIOPREPROCESSOR_WEBRTC_OBSOLETE_R4332 :
    case AUDIOPREPROCESSOR_WEBRTC :
    case AUDIOPREPROCESSOR_SPEEXDSP : // maybe only denoising is used
                                      // in SpeexDSP but gain should
                                      // still be allowed.
        m_voice_thread.m_gainlevel = gainlevel;
        return true;
    }
    return false;
}

int ClientNode::GetVoiceGainLevel()
{
    rguard_t const g_snd(LockSndprop());

    if (m_soundprop.preprocessor.preprocessor == AUDIOPREPROCESSOR_TEAMTALK)
        return m_soundprop.preprocessor.ttpreprocessor.gainlevel;

    return m_voice_thread.m_gainlevel;
}

bool ClientNode::SetSoundPreprocess(const AudioPreprocessor& preprocessor)
{
    rguard_t const g_snd(LockSndprop());

    m_soundprop.preprocessor = preprocessor;

    return m_voice_thread.UpdatePreprocessor(preprocessor);
}

void ClientNode::SetSoundInputTone(StreamTypes streams, int frequency)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if((streams & STREAMTYPE_MEDIAFILE_AUDIO) != 0u)
        m_audiofile_thread.EnableTone(frequency);
    if((streams & STREAMTYPE_VOICE) != 0u)
        m_voice_thread.EnableTone(frequency);
}

bool ClientNode::StartRecordingMuxedAudioFile(const AudioCodec& codec, 
                                              StreamTypes sts,
                                              const ACE_TString& filename,
                                              AudioFileFormat aff)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if (m_channelrecord.SaveFile(FIXED_AUDIOCODEC_CHANNELID,
                                 codec, sts, filename, aff))
    {
        m_flags |= CLIENT_MUX_AUDIOFILE;
        return true;
    }
    return false;
}

bool ClientNode::StartRecordingMuxedAudioFile(int channelid,
                                              StreamTypes sts,
                                              const ACE_TString& filename,
                                              AudioFileFormat aff)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    auto chan = GetChannel(channelid);
    if (!chan)
        return false;

    return m_channelrecord.SaveFile(channelid, chan->GetAudioCodec(),
                                    sts, filename, aff);
}

void ClientNode::StopRecordingMuxedAudioFile()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    StopRecordingMuxedAudioFile(FIXED_AUDIOCODEC_CHANNELID);
    
    m_flags &= ~CLIENT_MUX_AUDIOFILE;
}

void ClientNode::StopRecordingMuxedAudioFile(int channelid)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    m_channelrecord.CloseFile(channelid);
}

bool ClientNode::StartMTUQuery()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if(TimerExists(TIMER_QUERY_MTU_ID))
        return false;

    m_mtu_packets.clear();

    return StartTimer(TIMER_QUERY_MTU_ID, 0, ACE_Time_Value::zero,
                      CLIENT_QUERY_MTU_INTERVAL)>=0;
}

bool ClientNode::StartStreamingMediaFile(const ACE_TString& filename,
                                         uint32_t offset, bool paused,
                                         const AudioPreprocessor& preprocessor,
                                         const VideoCodec& vid_codec)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    // Media streaming is stopped by a timer to avoid deadlock.
    // First stop media stream if it is completed.
    if (m_mediafile_streamer && m_mediafile_streamer->Completed())
        StopStreamingMediaFile();

    //don't allow video streaming if not in channel or already streaming
    if (!m_mychannel)
        return false;

    if (((m_flags & CLIENT_STREAM_VIDEOFILE) != 0u) || ((m_flags & CLIENT_STREAM_AUDIOFILE) != 0u))
        return false;

    MediaStreamOutput media_out(GetAudioCodecAudioFormat(m_mychannel->GetAudioCodec()),
                                GetAudioCodecCbSamples(m_mychannel->GetAudioCodec()));
    bool const videooutput = vid_codec.codec != CODEC_NO_CODEC;
    if (videooutput)
    {
        media_out.video.fourcc = media::FOURCC_I420; // TODO: this is not enforced. FFmpeg will output RGB32
        // notice we don't set video output format (width/height)
        // since it's determined by the video decoder's format information
    }

    m_mediafile_streamer = MakeMediaFileStreamer(filename, media_out);
    if (!m_mediafile_streamer)
        return false;

    m_mediafile_streamer->RegisterVideoCallback([this](auto && PH1, auto && PH2) { return MediaStreamVideoCallback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }, true);
    m_mediafile_streamer->RegisterAudioCallback([this](auto && PH1, auto && PH2) { return MediaStreamAudioCallback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }, true);
    m_mediafile_streamer->RegisterStatusCallback([this](auto && PH1, auto && PH2) { MediaStreamStatusCallback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }, true);

    if (!m_mediafile_streamer->Open())
    {
        StopStreamingMediaFile();
        return false;
    }

    MediaFileProp const file_in = m_mediafile_streamer->GetMediaFile();

    //initiate audio part of media file
    if(file_in.audio.IsValid() && media_out.audio.IsValid())
    {
        auto cbfunc = [this](auto && PH1, auto && PH2, auto && PH3, auto && PH4, auto && PH5) { EncodedAudioFileFrame(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5)); };
        if (!m_audiofile_thread.StartEncoder(cbfunc, m_mychannel->GetAudioCodec(), true))
        {
            StopStreamingMediaFile();
            return false;
        }
        m_flags |= CLIENT_STREAM_AUDIOFILE;
    }

    TTASSERT(!m_videofile_thread);
    //initiate video part of media file
    if (file_in.video.IsValid() && videooutput)
    {
        m_flags |= CLIENT_STREAM_VIDEOFILE;

        m_videofile_thread = std::make_shared<VideoThread>();
        auto cbfunc = [this](auto && PH1, auto && PH2, auto && PH3, auto && PH4, auto && PH5) { return EncodedVideoFileFrame(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5)); };
        if (!m_videofile_thread->StartEncoder(cbfunc, m_mediafile_streamer->GetMediaOutput().video,
                                              vid_codec, VIDEOFILE_ENCODER_FRAMES_MAX))
        {
            StopStreamingMediaFile();
            return false;
        }
    }
    
    // give up if input file has no video or audio
    if(!file_in.audio.IsValid() && !file_in.video.IsValid())
    {
        StopStreamingMediaFile();
        return false;
    }

    //both audio and video part of mediafile use same stream id
    GEN_NEXT_ID(m_mediafile_stream_id);

    // initial playback should ignore offset being 0 (which is the logical value to use)
    if (!UpdateStreamingMediaFile((offset != 0 ? offset : MEDIASTREAMER_OFFSET_IGNORE),
                                  paused, preprocessor, vid_codec))
    {
        StopStreamingMediaFile();
        return false;
    }

    if (!TimerExists(TIMER_STOP_STREAM_MEDIAFILE_ID))
    {
        long const ret = StartTimer(TIMER_STOP_STREAM_MEDIAFILE_ID,  0, ACE_Time_Value::zero, ToTimeValue(10));
        TTASSERT(ret >= 0);
    }

    return true;
}

bool ClientNode::UpdateStreamingMediaFile(uint32_t offset, bool paused,
                                          const AudioPreprocessor& preprocessor,
                                          const VideoCodec& vid_codec)
{
    if (!m_mediafile_streamer || m_mediafile_streamer->Completed())
        return false;

    if (m_videofile_thread)
    {
        if (!m_videofile_thread->UpdateEncoder(vid_codec))
        {
            MYTRACE(ACE_TEXT("Failed to update video encoder on %s\n"),
                    m_mediafile_streamer->GetMediaFile().filename.c_str());
            return false;
        }
    }

    if (!m_audiofile_thread.UpdatePreprocessor(preprocessor))
        return false;

    m_mediafile_streamer->SetOffset(offset);

    if (paused)
        return m_mediafile_streamer->Pause();
    
            return m_mediafile_streamer->StartStream();
   
}

void ClientNode::StopStreamingMediaFile(bool killtimer/* = true*/)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    bool clear_video = false;
    bool clear_audio = false;

    if (m_mediafile_streamer)
    {
        clear_video = m_mediafile_streamer->GetMediaOutput().HasVideo();
        clear_audio = m_mediafile_streamer->GetMediaOutput().HasAudio();

        AudioUserCallback(LOCAL_USERID, STREAMTYPE_MEDIAFILE_AUDIO, media::AudioFrame());

        m_mediafile_streamer.reset();

        if (killtimer && TimerExists(TIMER_STOP_STREAM_MEDIAFILE_ID))
            StopTimer(TIMER_STOP_STREAM_MEDIAFILE_ID);
    }

    if(clear_video)
    {
        if (m_videofile_thread)
            m_videofile_thread->StopEncoder();
        m_videofile_thread.reset();
        m_flags &= ~CLIENT_STREAM_VIDEOFILE;
    }

    if(clear_audio)
    {
        m_audiofile_thread.StopEncoder();
        m_flags &= ~CLIENT_STREAM_AUDIOFILE;
    }
}

int ClientNode::InitMediaPlayback(const ACE_TString& filename, uint32_t offset, bool paused, 
                                  const AudioPreprocessor& preprocessor)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    auto first = m_mediaplayback_counter;
    GEN_NEXT_ID_MAX(m_mediaplayback_counter, LOCAL_MEDIAPLAYBACK_SESSIONID_MAX);
    while (m_mediaplayback_streams.contains(m_mediaplayback_counter))
    {
        if (m_mediaplayback_counter == first)
            return 0;
        GEN_NEXT_ID_MAX(m_mediaplayback_counter, LOCAL_MEDIAPLAYBACK_SESSIONID_MAX);
    }

    mediaplayback_t playback;
    playback = std::make_shared<MediaPlayback>(m_mediaplayback_counter, m_soundsystem,
                                     [this](auto && PH1, auto && PH2, auto && PH3) { MediaPlaybackStatus(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); },
                                     [this](auto && PH1, auto && PH2) { MediaPlaybackAudio(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
    
    if (!playback)
        return 0;

    if(!playback->OpenFile(filename))
        return 0;

    if ((m_soundprop.soundgroupid != 0) && m_soundprop.outputdeviceid != SOUNDDEVICE_IGNORE_ID)
    {
        if (!playback->OpenSoundSystem(m_soundprop.soundgroupid,
                                       m_soundprop.outputdeviceid,
                                       preprocessor.preprocessor == AUDIOPREPROCESSOR_SPEEXDSP))
            return 0;
    }

    m_mediaplayback_streams[m_mediaplayback_counter] = playback;

    // initial playback should ignore offset being 0 (which is the logical value to use)
    if (!UpdateMediaPlayback(m_mediaplayback_counter, (offset != 0 ? offset : MEDIASTREAMER_OFFSET_IGNORE),
                             paused, preprocessor))
    {
        m_mediaplayback_streams.erase(m_mediaplayback_counter);
        return 0;
    }

    if (!TimerExists(TIMER_REMOVE_LOCALPLAYBACK))
    {
        long const ret = StartTimer(TIMER_REMOVE_LOCALPLAYBACK,  0, ACE_Time_Value::zero, ToTimeValue(PB_FRAMEDURATION_MSEC));
        TTASSERT(ret >= 0);
    }

    MYTRACE(ACE_TEXT("Opened local media playback #%d for file %s. Active %d \n"),
            m_mediaplayback_counter, filename.c_str(), int(m_mediaplayback_streams.size()));

    return m_mediaplayback_counter;
}

bool ClientNode::UpdateMediaPlayback(int id, uint32_t offset, bool paused, 
                                     const AudioPreprocessor& preprocessor, bool /*initial*/)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    auto iplayback = m_mediaplayback_streams.find(id);
    if (iplayback == m_mediaplayback_streams.end())
        return false;

    auto playback = iplayback->second;

    switch(preprocessor.preprocessor)
    {
    case AUDIOPREPROCESSOR_NONE:
        playback->MuteSound(false, false);
        playback->SetGainLevel();
        break;
    case AUDIOPREPROCESSOR_TEAMTALK:
        playback->MuteSound(preprocessor.ttpreprocessor.muteleft, preprocessor.ttpreprocessor.muteright);
        playback->SetGainLevel(preprocessor.ttpreprocessor.gainlevel);
        break;
    case AUDIOPREPROCESSOR_SPEEXDSP:
    {
#if defined(ENABLE_SPEEXDSP)
        SpeexAGC const agc(float(preprocessor.speexdsp.agc_gainlevel), preprocessor.speexdsp.agc_maxincdbsec,
                     preprocessor.speexdsp.agc_maxdecdbsec, preprocessor.speexdsp.agc_maxgaindb);

        if(!playback->SetupSpeexPreprocess(preprocessor.speexdsp.enable_agc, agc,
                                           preprocessor.speexdsp.enable_denoise,
                                           preprocessor.speexdsp.maxnoisesuppressdb))
            return false;
#endif
    }
    case AUDIOPREPROCESSOR_WEBRTC_OBSOLETE_R4332 :
        return false;
    case AUDIOPREPROCESSOR_WEBRTC :
#if defined(ENABLE_WEBRTC)
        if (!playback->SetupWebRTCPreprocess(preprocessor.webrtc))
            return false;
#endif
        break;
    }

    if (!playback->Seek(offset))
        return false;

    if (paused)
        return playback->Pause();
            return playback->PlayMedia();
}

bool ClientNode::StopMediaPlayback(int id)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    auto iplayback = m_mediaplayback_streams.find(id);
    if(iplayback == m_mediaplayback_streams.end())
        return false;

    m_mediaplayback_streams.erase(iplayback);

    AudioUserCallback(id, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, media::AudioFrame());

    return true;
}

void ClientNode::MediaPlaybackStatus(int id, const MediaFileProp& mfp, MediaStreamStatus status)
{
    // don't hold lock since it can cause a deadlock
    switch (status)
    {
    case MEDIASTREAM_STARTED :
        m_listener->OnLocalMediaFilePlayback(id, mfp, MFS_STARTED);
        break;
    case MEDIASTREAM_PLAYING :
        m_listener->OnLocalMediaFilePlayback(id, mfp, MFS_PLAYING);
        break;
    case MEDIASTREAM_ERROR :
        AudioUserCallback(id, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, media::AudioFrame());

        // TIMER_REMOVE_LOCALPLAYBACK will destroy media playback
        m_listener->OnLocalMediaFilePlayback(id, mfp, MFS_ERROR);
        break;
    case MEDIASTREAM_PAUSED :
        // appear as stream finished, otherwise AudioMuxer will be delayed
        AudioUserCallback(id, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, media::AudioFrame());

        m_listener->OnLocalMediaFilePlayback(id, mfp, MFS_PAUSED);
        break;
    case MEDIASTREAM_FINISHED :
        AudioUserCallback(id, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, media::AudioFrame());

        // TIMER_REMOVE_LOCALPLAYBACK will destroy media playback
        m_listener->OnLocalMediaFilePlayback(id, mfp, MFS_FINISHED);
        break;
    case MEDIASTREAM_NONE :
        assert(status != MEDIASTREAM_NONE);
        break;
    }
}

void ClientNode::MediaPlaybackAudio(int id, const media::AudioFrame& frm)
{
    assert(frm.streamid);
    auto cpyframe = frm;
    cpyframe.userdata = STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO;
    AudioUserCallback(id, STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO, cpyframe);
}

bool ClientNode::InitVideoCapture(const ACE_TString& src_id,
                                  const media::VideoFormat& cap_format)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if((m_flags & CLIENT_VIDEOCAPTURE_READY) != 0u)
        return false;

    assert(!m_vidcap);
    if (m_vidcap)
        return false;

    videocapture_t session = vidcap::VideoCapture::Create();

    if (!session->InitVideoCapture(src_id, cap_format))
        return false;
        
    if (session->RegisterVideoFormat([this](auto && PH1, auto && PH2) { return VideoCaptureEncodeCallback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }, media::FOURCC_I420))
    {
        MYTRACE(ACE_TEXT("Video capture sends I420 directly to encoder\n"));
        if (session->RegisterVideoFormat([this](auto && PH1, auto && PH2) { return VideoCaptureRGB32Callback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }, media::FOURCC_RGB32))
        {
            MYTRACE(ACE_TEXT("Video capture sends RGB32 directly to 'm_local_vidcapframes'\n"));
        }
        else
        {
            MYTRACE(ACE_TEXT("Video capture cannot send RGB32', i.e. 'm_local_vidcapframes' is ignored\n"));
        }
    }
    else if (session->RegisterVideoFormat([this](auto && PH1, auto && PH2) { return VideoCaptureDualCallback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }, media::FOURCC_RGB32))
    {
        MYTRACE(ACE_TEXT("Video capture sends RGB32 directly to encoder. Encoder forwards to 'm_local_vidcapframes'\n"));
    }
    else
    {
        return false;
    }

    m_vidcap.swap(session);

    //set max buffers for local video frames
    int bytes = sizeof(media::VideoFrame) + RGB32_BYTES(cap_format.width, cap_format.height);
    bytes *= VIDEOCAPTURE_LOCAL_FRAMES_MAX;
    m_local_vidcapframes.high_water_mark(bytes);//room for one frame
    m_local_vidcapframes.low_water_mark(bytes);
    m_local_vidcapframes.activate();

    if (!m_vidcap->StartVideoCapture())
    {
        CloseVideoCapture();
        return false;
    }

    m_flags |= CLIENT_VIDEOCAPTURE_READY;

    return true;
}

void ClientNode::CloseVideoCapture()
{
    ASSERT_CLIENTNODE_LOCKED(this);
    m_vidcap.reset();

    CloseVideoCaptureSession();

    m_flags &= ~CLIENT_VIDEOCAPTURE_READY;
}

bool ClientNode::OpenVideoCaptureSession(const VideoCodec& codec)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if((m_flags & CLIENT_TX_VIDEOCAPTURE) != 0u)
        return false;

    if (!m_vidcap)
        return false;

    VideoFormat const cap_format = m_vidcap->GetVideoCaptureFormat();
    if(!cap_format.IsValid())
        return false;

    m_vidcap_thread.StopEncoder();

    if(!m_vidcap_thread.StartEncoder([this](auto && PH1, auto && PH2, auto && PH3, auto && PH4, auto && PH5) { return EncodedVideoCaptureFrame(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5)); },
                                     cap_format, codec, VIDEOCAPTURE_ENCODER_FRAMES_MAX))
    {
        CloseVideoCaptureSession();
        return false;
    }
    GEN_NEXT_ID(m_vidcap_stream_id);

    m_flags |= CLIENT_TX_VIDEOCAPTURE;

    return true;
}

void ClientNode::CloseVideoCaptureSession()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    m_vidcap_thread.StopEncoder();
    m_local_vidcapframes.close();

    m_flags &= ~CLIENT_TX_VIDEOCAPTURE;
}

ACE_Message_Block* ClientNode::AcquireVideoCaptureFrame()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    ACE_Message_Block* mb = nullptr;
    ACE_Time_Value tm_zero;
    m_local_vidcapframes.dequeue(mb, &tm_zero);
    return mb;
}

bool ClientNode::EncodedVideoCaptureFrame(ACE_Message_Block* org_frame,
                                          const char* enc_data, int enc_len,
                                          ACE_UINT32 packet_no,
                                          ACE_UINT32 timestamp)
{
    if((enc_data != nullptr) && ((m_flags & CLIENT_AUTHORIZED) != 0u) &&
       ((m_flags & CLIENT_TX_VIDEOCAPTURE) != 0u))
    {
        auto w = (uint16_t)m_vidcap_thread.GetVideoFormat().width;
        auto h = (uint16_t)m_vidcap_thread.GetVideoFormat().height;
        //max supported is uint16 * MAX_PAYLOAD_SIZE
        videopackets_t packets = BuildVideoPackets(PACKET_KIND_VIDEO,
                                                   m_myuserid, timestamp,
                                                   m_mtu_data_size,
                                                   m_vidcap_stream_id, 
                                                   packet_no, &w, &h,
                                                   enc_data, enc_len);

        bool failed = false;
        for(auto & packet : packets)
        {
            if(failed || !QueuePacket(packet))
            {
                delete packet;
                failed = true;
            }
        }
    }
    //MYTRACE(ACE_TEXT("Local video frame queue: %d\n"), m_local_vidcapframes.message_count());

    ACE_Time_Value const tm_zero;
    if(org_frame != nullptr)
    {
        VideoFrame vid_frm(org_frame);
        vid_frm.stream_id = m_vidcap_stream_id;

        if (vid_frm.fourcc == media::FOURCC_RGB32)
            return VideoCaptureRGB32Callback(vid_frm, org_frame);
    }

    return false; //ignored 'org_frame'
}

bool ClientNode::EncodedVideoFileFrame(ACE_Message_Block* /*org_frame*/,
                                       const char* enc_data, int enc_len,
                                       ACE_UINT32 packet_no,
                                       ACE_UINT32 timestamp)
{
    //max supported is uint16 * MAX_PAYLOAD_SIZE
    auto w = (uint16_t)m_videofile_thread->GetVideoFormat().width;
    auto h = (uint16_t)m_videofile_thread->GetVideoFormat().height;
    videopackets_t packets = BuildVideoPackets(PACKET_KIND_MEDIAFILE_VIDEO,
                                               m_myuserid, timestamp,
                                               m_mtu_data_size,
                                               m_mediafile_stream_id, 
                                               packet_no,
                                               &w, &h,
                                               enc_data, enc_len);

    // MYTRACE(ACE_TEXT("Video packet %d, fragments %d, size %d, csum 0x%x\n"),
    //         packet_no, (int)packets.size(), enc_len, 
    //         ACE::crc32(enc_data, enc_len));

    bool failed = false;
    for(auto & packet : packets)
    {
        if(failed || !QueuePacket(packet))
        {
            delete packet;
            failed = true;
        }
    }

    return false; //ignored 'org_frame'
}

int ClientNode::SendDesktopWindow(int width, int height, RGBMode rgb,
                                  DesktopProtocol protocol, 
                                  const char* bmp_buf, int bmp_buf_len)
{
    ACE_UNUSED_ARG(protocol);
    ASSERT_CLIENTNODE_LOCKED(this);

    if (!m_mychannel ||
       (m_myuseraccount.userrights & USERRIGHT_TRANSMIT_DESKTOP) == 0)
        return -1;

    if(!m_mychannel->CanTransmit(m_myuserid, STREAMTYPE_DESKTOP))
        return -1;

    //start new session or update existing
    if (!m_desktop || m_desktop->GetWidth() != width ||
        m_desktop->GetHeight() != height || m_desktop->GetRGBMode() != rgb)
    {
        CloseDesktopSession(true);

        GEN_NEXT_ID(m_desktop_session_id);

        DesktopWindow const new_wnd(m_desktop_session_id, width, height, rgb, 
                              DESKTOPPROTOCOL_ZLIB_1);

        DesktopInitiator* desktop = nullptr;
        ACE_NEW_RETURN(desktop, DesktopInitiator(GetUserID(), new_wnd,
                                                 m_mtu_data_size,
                                                 m_mtu_max_payload_size),
                                                 false);
        m_desktop = desktop_initiator_t(desktop);

        m_flags |= CLIENT_DESKTOP_ACTIVE;
    }

    //ensure all data has already been sent before starting a new transmission
    if (m_desktop_tx && !m_desktop_tx->Done() &&
        m_desktop_tx->GetSessionID() == m_desktop->GetSessionID())
    {
        MYTRACE(ACE_TEXT("Ignored desktop update. Transmission already ongoing\n"));
        return -1;
    }

    if((m_desktop->thr_count() != 0u) || m_desktop->HasDesktopPackets())
    {
        MYTRACE(ACE_TEXT("Ignored desktop updated. Thread active: %d, Desktop packets: %d\n"),
                (int)m_desktop->thr_count(), (int)m_desktop->HasDesktopPackets());
        return -1;
    }

    //ensure we don't get the same timestamp for next desktop update
    if (m_desktop_tx)
        DUP_TIMESTAMP_DELAY(m_desktop_tx->GetUpdateID());

    uint32_t const tm = GETTIMESTAMP();

    //process new bitmap
    int const ret = m_desktop->NewBitmap(bmp_buf, bmp_buf_len, tm);

    if(ret > 0)
    {
        DesktopTransmitter* dtx = nullptr;
        ACE_NEW_NORETURN(dtx, DesktopTransmitter(m_desktop_session_id, tm));
        if(dtx == nullptr)
        {
            CloseDesktopSession(false);
            return -1;
        }
        m_desktop_tx = desktop_transmitter_t(dtx);

        //TODO: find a better way to query for 'complete' instead of busy wait
        ACE_Time_Value const tm(0, 1000);
        if(StartTimer(TIMER_BUILD_DESKTOPPACKETS_ID, 0, tm, tm)<0)
        {
            CloseDesktopSession(false);
            return -1;
        }
        m_flags |= CLIENT_TX_DESKTOP;
    }

    return ret;
}

bool ClientNode::CloseDesktopWindow()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    uint8_t session_id = 0;
    
    if (m_desktop_tx)
        session_id = m_desktop_tx->GetSessionID();
    else if (m_desktop)
        session_id = m_desktop->GetSessionID();

    if(session_id != 0u)
    {
        DesktopNakTransmitter* nak_tx = nullptr;
        ACE_NEW_RETURN(nak_tx, DesktopNakTransmitter(session_id,
                                                     GETTIMESTAMP()),
                       false);

        m_desktop_nak_tx = desktop_nak_tx_t(nak_tx);

        //start timer to close session
        StartTimer(TIMER_DESKTOPNAKPACKET_TIMEOUT_ID, 0, 
                   ACE_Time_Value::zero, CLIENT_DESKTOPNAK_TIMEOUT);
    }
    CloseDesktopSession(false);
    return true;
}

void ClientNode::CloseDesktopSession(bool stop_nak_timer)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if(TimerExists(TIMER_BUILD_DESKTOPPACKETS_ID))
        StopTimer(TIMER_BUILD_DESKTOPPACKETS_ID);
    if(TimerExists(TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID))
        StopTimer(TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID);

    if(stop_nak_timer && TimerExists(TIMER_DESKTOPNAKPACKET_TIMEOUT_ID))
    {
        StopTimer(TIMER_DESKTOPNAKPACKET_TIMEOUT_ID);
        m_desktop_nak_tx.reset();
    }

    if (m_desktop)
    {
        m_desktop->Abort();
        m_desktop->wait();
        m_desktop.reset();
    }
    m_desktop_tx.reset();

    //clear all desktop input for his session
    auto ii = m_users.begin();
    while(ii != m_users.end())
    {
        ii->second->ResetDesktopInputRx();
        ii++;
    }

    m_flags &= ~CLIENT_TX_DESKTOP;
    m_flags &= ~CLIENT_DESKTOP_ACTIVE;
}

void ClientNode::ResetAudioPlayers()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    while(!m_mediaplayback_streams.empty())
        StopMediaPlayback(m_mediaplayback_streams.begin()->first);

    if (m_rootchannel)
    {
        ClientChannel::users_t users;
        m_rootchannel->GetUsers(users, true);
        for(const auto & user : users)
            user->ResetAudioPlayers(true);
    }
}

bool ClientNode::SendDesktopCursor(int x, int y)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    clientchannel_t chan;
    DesktopCursorPacket* pkt = nullptr;

    chan = GetMyChannel();
    if (!chan)
        return false;
    if (!m_desktop)
        return false;

    ACE_NEW_RETURN(pkt, DesktopCursorPacket(GetUserID(),
                                            GETTIMESTAMP(),
                                            m_desktop->GetSessionID(),
                                            x, y), false);
    pkt->SetChannel(chan->GetChannelID());

    if(!QueuePacket(pkt))
    {
        delete pkt;
        return false;
    }
    return true;
}

bool ClientNode::SendDesktopInput(int userid,
                                  const std::vector<DesktopInput>& inputs)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    clientchannel_t chan;
    clientuser_t user;

    user = GetUser(userid);
    if (!user)
        return false;

    int const n_tx_queue = int(user->GetDesktopInputTxQueue().size() + user->GetDesktopInputRtxQueue().size());
    TTASSERT(n_tx_queue <= DESKTOPINPUT_QUEUE_MAX_SIZE);

    TTASSERT(n_tx_queue == 0 ||
            TimerExists(USER_TIMER_DESKTOPINPUT_RTX_ID, userid));

    if(n_tx_queue == DESKTOPINPUT_QUEUE_MAX_SIZE ||
       (m_myuseraccount.userrights & USERRIGHT_TRANSMIT_DESKTOPINPUT) == USERRIGHT_NONE)
       return false;

    chan = user->GetChannel();
    if (!chan)
        return false;

    desktop_viewer_t const viewer = user->GetDesktopSession();
    if (!viewer)
        return false;

    DesktopInputPacket* pkt = nullptr;
    ACE_NEW_RETURN(pkt, DesktopInputPacket(GetUserID(),
                                           GETTIMESTAMP(),
                                           viewer->GetSessionID(),
                                           user->NextDesktopInputTxPacket(),
                                           inputs), false);
    pkt->SetDestUser(userid);
    pkt->SetChannel(chan->GetChannelID());

    //store packet for retransmission
    DesktopInputPacket* rtx_pkt = nullptr;
    ACE_NEW_NORETURN(rtx_pkt,
                     DesktopInputPacket(*pkt));
    if(rtx_pkt == nullptr)
    {
        delete pkt;
        return false;
    }

    MYTRACE(ACE_TEXT("Queueing packet no %d with %u keys\n"),
            (int)pkt->GetPacketNo(), (unsigned)inputs.size());
    //store for tx
    desktopinput_pkt_t const tx_pkt(rtx_pkt);
    user->GetDesktopInputTxQueue().push_back(tx_pkt);

    //start RTX timer?
    if(user->GetDesktopInputTxQueue().size() == 1 &&
       user->GetDesktopInputRtxQueue().empty())
    {
        if(!QueuePacket(pkt))
            delete pkt;
        else
        {
            //move from Tx queue to Rtx queue
            user->GetDesktopInputRtxQueue().push_back(tx_pkt);
            user->GetDesktopInputTxQueue().pop_front();
        }

        if(TimerExists(USER_TIMER_DESKTOPINPUT_RTX_ID, userid))
            StopUserTimer(USER_TIMER_DESKTOPINPUT_RTX_ID, userid);

        StartUserTimer(USER_TIMER_DESKTOPINPUT_RTX_ID, userid, 0,
                        CLIENT_DESKTOPINPUT_RTX_TIMEOUT,
                        CLIENT_DESKTOPINPUT_RTX_TIMEOUT);
    }
    else
        delete pkt;

    return true;
}


bool ClientNode::Connect(bool encrypted, const ACE_TString& hostaddr,
                         u_short tcpport, u_short udpport, 
                         const ACE_TString& sysid,
                         const ACE_TString& localaddr,
                         u_short local_tcpport/* = 0*/, 
                         u_short local_udpport/* = 0*/)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    // can't double connect
    if (((m_flags & CLIENT_CONNECTION) != 0u) || (!m_serverinfo.hostaddrs.empty()))
        return false;

    if ((!localaddr.empty()) || (local_tcpport != 0u) || (local_udpport != 0u))
    {
        m_localTcpAddr = ACE_INET_Addr(local_tcpport, localaddr.c_str());
        m_localUdpAddr = ACE_INET_Addr(local_udpport, localaddr.c_str());
    }
    
    //welcome message to look for
    m_serverinfo.systemid = sysid;
    
    m_serverinfo.hostaddrs = DetermineHostAddress(hostaddr, tcpport);
    if (!m_serverinfo.hostaddrs.empty())
    {
        m_serverinfo.udpaddr = m_serverinfo.hostaddrs[0];
        m_serverinfo.udpaddr.set_port_number(udpport);
    }
    MYTRACE(ACE_TEXT("Resolved %d IP-addresses for \"%s\"\n"), int(m_serverinfo.hostaddrs.size()), hostaddr.c_str());
    
    if ((!m_serverinfo.hostaddrs.empty()) &&
        Connect(encrypted, m_serverinfo.hostaddrs[0], m_localTcpAddr != ACE_INET_Addr() ? &m_localTcpAddr : nullptr))
    {
        StartTimer(TIMER_ONE_SECOND_ID, 0, ACE_Time_Value(1), ACE_Time_Value(1));

        m_flags |= CLIENT_CONNECTING;
        return true;
    }

    Disconnect(); //clean up

    return false;
}

bool ClientNode::Connect(bool encrypted, const ACE_INET_Addr& hosttcpaddr,
                         const ACE_INET_Addr* localtcpaddr)
{
    int ret = 0;

#if !defined(UNICODE)
    MYTRACE(ACE_TEXT("Trying remote TCP: %s:%d, Local TCP: %s:%d, UDP: %s:%d\n"),
            hosttcpaddr.get_host_addr(), hosttcpaddr.get_port_number(),
            m_localTcpAddr.get_host_addr(), m_localTcpAddr.get_port_number(),
            m_localUdpAddr.get_host_addr(), m_localUdpAddr.get_port_number());
#endif            
    
#if defined(ENABLE_ENCRYPTION)
    if(encrypted)
    {
        ACE_NEW_RETURN(m_crypt_stream, CryptStreamHandler(GetEventLoop()), false);
        m_crypt_stream->SetListener(this);
        ACE_Synch_Options const options(ACE_Synch_Options::USE_REACTOR, ACE_Time_Value(0, 0));
        if (localtcpaddr != nullptr)
            ret = m_crypt_connector.connect(m_crypt_stream, hosttcpaddr, 
                                            options, *localtcpaddr);
        else
            ret = m_crypt_connector.connect(m_crypt_stream, hosttcpaddr, 
                                            options);
    }
    else
#endif
    {
        ACE_NEW_RETURN(m_def_stream, DefaultStreamHandler(GetEventLoop()), false);
        m_def_stream->SetListener(this);
        ACE_Synch_Options const options(ACE_Synch_Options::USE_REACTOR, ACE_Time_Value(0,0));
        if (localtcpaddr != nullptr)
            ret = m_connector.connect(m_def_stream, hosttcpaddr, options, *localtcpaddr);
        else
            ret = m_connector.connect(m_def_stream, hosttcpaddr, options);
    }

    int const err = ACE_OS::last_error();
    MYTRACE( ACE_TEXT("Last error: %d. Would block is %d\n"), err, EWOULDBLOCK);
    return ret == 0 || (ret == -1 && err == EWOULDBLOCK);
}

void ClientNode::Disconnect()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    MYTRACE(ACE_TEXT("Disconnecting #%d.\n"), GetUserID());

    ResetTimers();
    
    ACE_HANDLE h = ACE_INVALID_HANDLE;
#if defined(ENABLE_ENCRYPTION)
    if(m_crypt_stream != nullptr)
    {
        //NOTE: same in FileNode::Disconnect()

        m_crypt_stream->SetListener(nullptr);
        h = m_crypt_stream->get_handle();

        m_crypt_connector.cancel(m_crypt_stream);
        m_crypt_stream->close();

        m_crypt_stream = nullptr;
    }
#endif
    if(m_def_stream != nullptr)
    {
        //NOTE: same in FileNode::Disconnect()

        m_def_stream->SetListener(nullptr);
        h = m_def_stream->get_handle();
        //if we're in the process of connecting, then cancel() which
        //doesn't destroy the handler. We destroy it by calling close 
        //manually
        m_connector.cancel(m_def_stream);
        m_def_stream->close();

        m_def_stream = nullptr;
    }

    TTASSERT(GetEventLoop()->find_handler(h) == nullptr);

    m_recvbuffer.clear();
    m_sendbuffer.clear();

    m_packethandler.RemoveListener(this);

    //clear pending packets
    m_tx_queue.Reset();

    m_packethandler.Close();

    //clean up file transfers
    m_waitingTransfers.clear();
    m_filetransfers.clear();

    //clear channels and login status
    LoggedOut();

    // remove all queued audio so a new connection will not be
    // retrieving audio from a previous session
    m_audiocontainer.ReleaseAllAudio();

    m_flags &= ~(CLIENT_CONNECTING | CLIENT_CONNECTED);

    m_serverinfo = ServerInfo();
    m_clientstats = ClientStats();
    m_localTcpAddr = m_localUdpAddr = ACE_INET_Addr();

    MYTRACE(ACE_TEXT("Disconnected #%d.\n"), GetUserID());
}

void ClientNode::JoinChannel(clientchannel_t& chan)
{
    ASSERT_REACTOR_THREAD(*GetEventLoop());
    ASSERT_CLIENTNODE_LOCKED(this);

    if (m_mychannel)
        LeftChannel(*m_mychannel);

    m_mychannel = chan;

    AudioCodec const codec = chan->GetAudioCodec();

    /* Sanity check */
    if (!ValidAudioCodec(codec))
    {
        m_listener->OnInternalError(TT_INTERR_AUDIOCODEC_INIT_FAILED,
                                    GetErrorDescription(TT_INTERR_AUDIOCODEC_INIT_FAILED));
        return;
    }

    //set audio encoder properties for voice stream
    auto cbenc = [this](auto && PH1, auto && PH2, auto && PH3, auto && PH4, auto && PH5) { EncodedAudioVoiceFrame(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5)); };
    if(m_voice_thread.StartEncoder(cbenc, codec, true))
    {
        if (!SetSoundPreprocess(m_soundprop.preprocessor)) //set AGC, denoise, etc.
        {
            m_listener->OnInternalError(TT_INTERR_AUDIOPREPROCESSOR_INIT_FAILED,
                                        GetErrorDescription(TT_INTERR_AUDIOPREPROCESSOR_INIT_FAILED));
        }
    }
    else
    {
        m_listener->OnInternalError(TT_INTERR_AUDIOCODEC_INIT_FAILED,
                                    GetErrorDescription(TT_INTERR_AUDIOCODEC_INIT_FAILED));
    }
        
    // add "self" from muxed recording
    m_channelrecord.AddUser(LOCAL_TX_USERID, STREAMTYPE_VOICE, chan->GetChannelID());

    //start recorder for voice stream
    OpenAudioCapture(codec);
}

void ClientNode::LeftChannel(ClientChannel& chan)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    TTASSERT(m_mychannel.get());

    if (m_mychannel && chan.Compare(m_mychannel))
        m_mychannel.reset();

    //clear files if not admin
    if((m_myuseraccount.usertype & USERTYPE_DEFAULT) != 0u)
        chan.ClearFiles();

    //stop streaming media file
    if((m_flags & (CLIENT_STREAM_AUDIOFILE | CLIENT_STREAM_VIDEOFILE)) != 0u)
    {
        bool const notify = m_mediafile_streamer.get() != nullptr;
        MediaFileProp mfp;
        if (m_mediafile_streamer)
            mfp = m_mediafile_streamer->GetMediaFile();
        
        StopStreamingMediaFile();
        
        if(notify)
            m_listener->OnChannelStreamMediaFile(mfp, MFS_ABORTED);
    }

    ClientChannel::users_t users = chan.GetUsers();
    for(const auto & user : users)
        user->ResetAllStreams();

    //shutdown audio capture
    if(chan.GetAudioCodec().codec != CODEC_NO_CODEC)
        CloseAudioCapture();

    m_voice_thread.StopEncoder();

    // remove "self" from muxed recording
    m_channelrecord.RemoveUser(LOCAL_TX_USERID, STREAMTYPE_VOICE);

    CloseDesktopSession(true);

    //make sure we don't send obsolete packets to channel
    m_tx_queue.RemoveChannelPackets();
}

void ClientNode::LoggedOut()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    //shutdown users' players
    clientchannel_t const chan = GetMyChannel();
    if (chan)
        LeftChannel(*chan);

    //shutdown all users (might be active admin streams)
    intset_t users;
    GetUsers(users);
    auto ite = users.begin();
    while(ite != users.end())
    {
        clientuser_t const user = GetUser(*ite);
        if (user)
        {
            user->ResetAllStreams();
            // remove all users from muxed recording
            m_channelrecord.RemoveUser(*ite, STREAMTYPE_VOICE);
        }
        ite++;
    }

    // Close all channel recordings
    if (m_rootchannel)
    {
        std::queue<clientchannel_t> channels;
        channels.push(m_rootchannel);

        while(!channels.empty())
        {
            m_channelrecord.CloseFile(channels.front()->GetChannelID());
            auto subs = channels.front()->GetSubChannels();
            for (auto& c : subs)
                channels.push(c);

            channels.pop();
        }
    }

    CloseDesktopSession(true);

    //delete my channel
    m_mychannel.reset();
    //delete root channel
    m_rootchannel.reset();
    //clear users
    m_users.clear();
    m_myuseraccount = UserAccount();

    m_flags &= ~CLIENT_AUTHORIZED;
}

int ClientNode::DoLogin(const ACE_TString& nickname,
                        const ACE_TString& username,
                        const ACE_TString& password,
                        const ACE_TString& clientname)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());
    
    //now do login
    ACE_TString command = CLIENT_LOGIN;
    AppendProperty(TT_NICKNAME, nickname, command);
    AppendProperty(TT_USERNAME, username, command);
    AppendProperty(TT_PASSWORD, password, command);
    AppendProperty(TT_CLIENTNAME, clientname, command);
    AppendProperty(TT_PROTOCOL, ACE_TString( TEAMTALK_PROTOCOL_VERSION ), command);
    AppendProperty(TT_VERSION, m_version, command);

    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoLogout()
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_LOGOUT;
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoChangeNickname(const ACE_TString& newnick)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_CHANGENICK;
    AppendProperty(TT_NICKNAME, newnick, command);

    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoChangeStatus(int statusmode, const ACE_TString& statusmsg)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_CHANGESTATUS;
    AppendProperty(TT_STATUSMODE, statusmode, command);
    AppendProperty(TT_STATUSMESSAGE, statusmsg, command);

    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoTextMessage(const TextMessage& msg)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_MESSAGE;
    AppendProperty(TT_MSGTYPE, (int)msg.msgType, command);
    AppendProperty(TT_MSGCONTENT, msg.content, command);
    switch(msg.msgType)
    {
    case TTUserMsg :
    case TTCustomMsg :
        AppendProperty(TT_DESTUSERID, msg.to_userid, command);
        break;
    case TTChannelMsg :
        AppendProperty(TT_CHANNELID, msg.channelid, command);
        break;
    case TTBroadcastMsg:
        break;
    case TTNoneMsg :
        TTASSERT(0);
        break;
    }
    AppendProperty(TT_TEXTMSG_MORE, static_cast<ACE_INT64>(msg.more), command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

//Command handlers (Client -> Server)
int ClientNode::DoPing(bool issue_cmdid)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    ACE_TString command = CLIENT_KEEPALIVE;
    if(issue_cmdid)
        AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    
    m_clientstats.ping_issue_time = ACE_OS::gettimeofday();

    if(!issue_cmdid)
        return TransmitCommand(command, 0);
            return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoJoinChannel(const ChannelProp& chanprop, bool forceexisting)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_JOINCHANNEL;
    if (!GetChannel(chanprop.channelid) && !forceexisting) //new channel
    {
        AppendProperty(TT_CHANNAME, chanprop.name, command);
        AppendProperty(TT_PARENTID, chanprop.parentid, command);
        AppendProperty(TT_TOPIC, chanprop.topic, command);
        AppendProperty(TT_OPPASSWORD, chanprop.oppasswd, command);
        // Deprecated TeamTalk v6
        if (!AudioCodecConvertBug(m_serverinfo.protocol, chanprop.audiocodec))
            AppendProperty(TT_AUDIOCODEC, chanprop.audiocodec, command);
        AppendProperty(TT_AUDIOCFG, chanprop.audiocfg, command);
        AppendProperty(TT_CHANNELTYPE, chanprop.chantype, command);
        AppendProperty(TT_USERDATA, chanprop.userdata, command);
        AppendProperty(TT_VOICEUSERS, chanprop.GetTransmitUsers(STREAMTYPE_VOICE), command);
        AppendProperty(TT_VIDEOUSERS, chanprop.GetTransmitUsers(STREAMTYPE_VIDEOCAPTURE), command);
        AppendProperty(TT_DESKTOPUSERS, chanprop.GetTransmitUsers(STREAMTYPE_DESKTOP), command);
        AppendProperty(TT_MEDIAFILEUSERS, chanprop.GetTransmitUsers(STREAMTYPE_MEDIAFILE), command);
        AppendProperty(TT_CHANMSGUSERS, chanprop.GetTransmitUsers(STREAMTYPE_CHANNELMSG), command);
        if (((chanprop.chantype & CHANNEL_SOLO_TRANSMIT) != 0u) && chanprop.transmitswitchdelay > 0)
            AppendProperty(TT_TRANSMITSWITCHDELAY, chanprop.transmitswitchdelay, command);
        AppendProperty(TT_TOTVOICE, chanprop.totvoice, command);
        AppendProperty(TT_TOTMEDIAFILE, chanprop.totmediafile, command);
    }
    else //already exists
    {
        AppendProperty(TT_CHANNELID, chanprop.channelid, command);
    }

    AppendProperty(TT_PASSWORD, chanprop.passwd, command);

    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoLeaveChannel()
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_LEAVECHANNEL;
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoKickUser(int userid, int channelid)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_KICK;
    if(channelid != 0)
        AppendProperty(TT_CHANNELID, channelid, command);

    AppendProperty(TT_USERID, userid, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoChannelOperator(int userid, int channelid, 
                                  const ACE_TString& oppasswd, bool op)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_CHANNELOP;
    AppendProperty(TT_CHANNELID, channelid, command);
    AppendProperty(TT_USERID, userid, command);
    AppendProperty(TT_OPERATORSTATUS, static_cast<ACE_INT64>(op), command);
    if(!oppasswd.empty())
        AppendProperty(TT_OPPASSWORD, oppasswd, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoFileSend(int channelid, const ACE_TString& localfilepath)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_INT64 const filesize = ACE_OS::filesize(localfilepath.c_str());
    ACE_TString filename = localfilepath;
    size_t const index = localfilepath.rfind(ACE_DIRECTORY_SEPARATOR_CHAR);
    if(index != ACE_TString::npos)
        filename = filename.substr(index+1, localfilepath.length()-index+1);

    FileTransfer transfer;
    transfer.channelid = channelid;
    transfer.localfile = localfilepath;
    transfer.filename = filename;
    transfer.filesize = filesize;
    transfer.inbound = false;
    transfer.transferid = 0;
    transfer.userid = GetUserID();
#if defined(ENABLE_ENCRYPTION)
    uint8_t transferkey[TRANSFERKEY_SIZE];
    RAND_bytes(transferkey, sizeof(transferkey));
    transfer.transferkey = KeyToHexString(transferkey, TRANSFERKEY_SIZE);
#endif
    TTASSERT(GetUserID()>0);

    //first register file transfer before sending
    ACE_TString command = CLIENT_REGSENDFILE;
    AppendProperty(TT_FILENAME, transfer.filename, command);
    AppendProperty(TT_FILESIZE, transfer.filesize, command);
    AppendProperty(TT_CHANNELID, transfer.channelid, command);
    AppendProperty(TT_TRANSFERKEY, transfer.transferkey, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    int const cmdid = TransmitCommand(command, m_cmdid_counter);
    if(cmdid>0)
        m_waitingTransfers[cmdid] = transfer;
    return cmdid;
}

int ClientNode::DoFileRecv(int channelid, 
                           const ACE_TString& localfilepath, 
                           const ACE_TString& remotefilename)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    FileTransfer transfer;
    transfer.channelid = channelid;
    transfer.localfile = localfilepath;
    transfer.filename = remotefilename;
    transfer.filesize = 0;
    transfer.inbound = true;
    transfer.transferid = 0;
    transfer.userid = GetUserID();
#if defined(ENABLE_ENCRYPTION)
    uint8_t transferkey[TRANSFERKEY_SIZE];
    RAND_bytes(transferkey, sizeof(transferkey));
    transfer.transferkey = KeyToHexString(transferkey, TRANSFERKEY_SIZE);
#endif
    TTASSERT(GetUserID()>0);

    //first register file transfer before sending
    ACE_TString command = CLIENT_REGRECVFILE;
    AppendProperty(TT_FILENAME, transfer.filename, command);
    AppendProperty(TT_CHANNELID, transfer.channelid, command);
    AppendProperty(TT_TRANSFERKEY, transfer.transferkey, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    int const cmdid = TransmitCommand(command, m_cmdid_counter);
    if(cmdid>0)
        m_waitingTransfers[cmdid] = transfer;
    return cmdid;
}

int ClientNode::DoFileDelete(int channelid, const ACE_TString& filename)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_DELETEFILE;
    AppendProperty(TT_CHANNELID, channelid, command);
    AppendProperty(TT_FILENAME, filename, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoSubscribe(int userid, Subscriptions subscript)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_SUBSCRIBE;
    AppendProperty(TT_USERID, userid, command);
    AppendProperty(TT_LOCALSUBSCRIPTIONS, subscript, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoUnsubscribe(int userid, Subscriptions subscript)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_UNSUBSCRIBE;
    AppendProperty(TT_USERID, userid, command);
    AppendProperty(TT_LOCALSUBSCRIPTIONS, subscript, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoQueryServerStats()
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_QUERYSTATS;
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoQuit()
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_QUIT;
    command += EOL;
    return TransmitCommand(command, 0);
}

int ClientNode::DoMakeChannel(const ChannelProp& chanprop)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_MAKECHANNEL;
    AppendProperty(TT_PARENTID, chanprop.parentid, command);
    AppendProperty(TT_CHANNAME, chanprop.name, command);
    AppendProperty(TT_PASSWORD, chanprop.passwd, command);
    AppendProperty(TT_TOPIC, chanprop.topic, command);
    AppendProperty(TT_DISKQUOTA, chanprop.diskquota, command);
    AppendProperty(TT_OPPASSWORD, chanprop.oppasswd, command);
    AppendProperty(TT_MAXUSERS, chanprop.maxusers, command);
    // Deprecated TeamTalk v6
    if (!AudioCodecConvertBug(m_serverinfo.protocol, chanprop.audiocodec))
        AppendProperty(TT_AUDIOCODEC, chanprop.audiocodec, command);
    AppendProperty(TT_AUDIOCFG, chanprop.audiocfg, command);
    AppendProperty(TT_CHANNELTYPE, chanprop.chantype, command);
    AppendProperty(TT_USERDATA, chanprop.userdata, command);
    AppendProperty(TT_VOICEUSERS, chanprop.GetTransmitUsers(STREAMTYPE_VOICE), command);
    AppendProperty(TT_VIDEOUSERS, chanprop.GetTransmitUsers(STREAMTYPE_VIDEOCAPTURE), command);
    AppendProperty(TT_DESKTOPUSERS, chanprop.GetTransmitUsers(STREAMTYPE_DESKTOP), command);
    AppendProperty(TT_MEDIAFILEUSERS, chanprop.GetTransmitUsers(STREAMTYPE_MEDIAFILE), command);
    AppendProperty(TT_CHANMSGUSERS, chanprop.GetTransmitUsers(STREAMTYPE_CHANNELMSG), command);
    if (((chanprop.chantype & CHANNEL_SOLO_TRANSMIT) != 0u) && chanprop.transmitswitchdelay > 0)
        AppendProperty(TT_TRANSMITSWITCHDELAY, chanprop.transmitswitchdelay, command);
    AppendProperty(TT_TOTVOICE, chanprop.totvoice, command);
    AppendProperty(TT_TOTMEDIAFILE, chanprop.totmediafile, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoUpdateChannel(const ChannelProp& chanprop)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_UPDATECHANNEL;
    AppendProperty(TT_CHANNELID, chanprop.channelid, command);
    AppendProperty(TT_CHANNAME, chanprop.name, command);
    AppendProperty(TT_PASSWORD, chanprop.passwd, command);
    AppendProperty(TT_TOPIC, chanprop.topic, command);
    AppendProperty(TT_DISKQUOTA, chanprop.diskquota, command);
    AppendProperty(TT_OPPASSWORD, chanprop.oppasswd, command);
    AppendProperty(TT_MAXUSERS, chanprop.maxusers, command);
    // Deprecated TeamTalk v6
    if (!AudioCodecConvertBug(m_serverinfo.protocol, chanprop.audiocodec))
        AppendProperty(TT_AUDIOCODEC, chanprop.audiocodec, command);
    AppendProperty(TT_AUDIOCFG, chanprop.audiocfg, command);
    AppendProperty(TT_CHANNELTYPE, chanprop.chantype, command);
    AppendProperty(TT_USERDATA, chanprop.userdata, command);
    AppendProperty(TT_VOICEUSERS, chanprop.GetTransmitUsers(STREAMTYPE_VOICE), command);
    AppendProperty(TT_VIDEOUSERS, chanprop.GetTransmitUsers(STREAMTYPE_VIDEOCAPTURE), command);
    AppendProperty(TT_DESKTOPUSERS, chanprop.GetTransmitUsers(STREAMTYPE_DESKTOP), command);
    AppendProperty(TT_MEDIAFILEUSERS, chanprop.GetTransmitUsers(STREAMTYPE_MEDIAFILE), command);
    AppendProperty(TT_CHANMSGUSERS, chanprop.GetTransmitUsers(STREAMTYPE_CHANNELMSG), command);
    if ((chanprop.chantype & CHANNEL_SOLO_TRANSMIT) != 0u)
        AppendProperty(TT_TRANSMITSWITCHDELAY, chanprop.transmitswitchdelay, command);
    AppendProperty(TT_TOTVOICE, chanprop.totvoice, command);
    AppendProperty(TT_TOTMEDIAFILE, chanprop.totmediafile, command);

    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoRemoveChannel(int channelid)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_REMOVECHANNEL;
    AppendProperty(TT_CHANNELID, channelid, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoMoveUser(int userid, int channelid)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_MOVEUSER;
    AppendProperty(TT_USERID, userid, command);
    AppendProperty(TT_CHANNELID, channelid, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoUpdateServer(const ServerInfo& serverprop)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_UPDATESERVER;
    AppendProperty(TT_SERVERNAME, serverprop.servername, command);
    AppendProperty(TT_MOTDRAW, serverprop.motd_raw, command);
    AppendProperty(TT_MAXUSERS, serverprop.maxusers, command);
    AppendProperty(TT_MAXLOGINATTEMPTS, serverprop.maxloginattempts, command);
    AppendProperty(TT_MAXLOGINSPERIP, serverprop.max_logins_per_ipaddr, command);
    AppendProperty(TT_LOGINDELAY, serverprop.logindelay, command);
    AppendProperty(TT_AUTOSAVE, static_cast<ACE_INT64>(serverprop.autosave), command);
    if (!serverprop.hostaddrs.empty())
    {
        AppendProperty(TT_TCPPORT, serverprop.hostaddrs[0].get_port_number(), command);
        AppendProperty(TT_UDPPORT, serverprop.udpaddr.get_port_number(), command);
    }
    AppendProperty(TT_USERTIMEOUT, serverprop.usertimeout, command);
    AppendProperty(TT_VOICETXLIMIT, serverprop.voicetxlimit, command);
    AppendProperty(TT_VIDEOTXLIMIT, serverprop.videotxlimit, command);
    AppendProperty(TT_MEDIAFILETXLIMIT, serverprop.mediafiletxlimit, command);
    AppendProperty(TT_DESKTOPTXLIMIT, serverprop.desktoptxlimit, command);
    AppendProperty(TT_TOTALTXLIMIT, serverprop.totaltxlimit, command);
    AppendProperty(TT_LOGEVENTS, serverprop.logevents, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoBanUser(int userid, const BannedUser& ban)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_BAN;
    if(userid>0)
        AppendProperty(TT_USERID, userid, command);
    if(!ban.ipaddr.empty())
        AppendProperty(TT_IPADDR, ban.ipaddr, command);
    AppendProperty(TT_BANTYPE, ban.bantype, command);
    if(!ban.username.empty())
        AppendProperty(TT_USERNAME, ban.username, command);
    if(!ban.chanpath.empty())
        AppendProperty(TT_CHANNEL, ban.chanpath, command);
    if (!ban.nickname.empty())
        AppendProperty(TT_NICKNAME, ban.nickname, command);

    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoUnBanUser(const BannedUser& ban)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_UNBAN;
    AppendProperty(TT_IPADDR, ban.ipaddr, command);
    AppendProperty(TT_BANTYPE, ban.bantype, command);
    AppendProperty(TT_USERNAME, ban.username, command);
    AppendProperty(TT_CHANNEL, ban.chanpath, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoListBans(int chanid, int index, int count)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_LISTBANS;
    AppendProperty(TT_INDEX, index, command);
    AppendProperty(TT_COUNT, count, command);
    if (chanid > 0)
        AppendProperty(TT_CHANNELID, chanid, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoListUserAccounts(int index, int count)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_LISTUSERACCOUNTS;
    AppendProperty(TT_INDEX, index, command);
    AppendProperty(TT_COUNT, count, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoNewUserAccount(const UserAccount& user)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_NEWUSERACCOUNT;
    AppendProperty(TT_USERNAME, user.username, command);
    AppendProperty(TT_PASSWORD, user.passwd, command);
    AppendProperty(TT_USERTYPE, user.usertype, command);
    AppendProperty(TT_USERRIGHTS, user.userrights, command);
    AppendProperty(TT_USERDATA, user.userdata, command);
    AppendProperty(TT_NOTEFIELD, user.note, command);
    AppendProperty(TT_INITCHANNEL, user.init_channel, command);
    AppendProperty(TT_AUTOOPCHANNELS, user.auto_op_channels, command);
    AppendProperty(TT_AUDIOBPSLIMIT, user.audiobpslimit, command);
    AppendProperty(TT_CMDFLOOD, user.abuse.ToParam(), command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoDeleteUserAccount(const ACE_TString& username)
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_DELUSERACCOUNT;
    AppendProperty(TT_USERNAME, username, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoSaveConfig()
{
    ASSERT_CLIENTNODE_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(*GetEventLoop());

    ACE_TString command = CLIENT_SAVECONFIG;
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::TransmitCommand(const ACE_TString& command, int cmdid)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if((m_flags & CLIENT_CONNECTED) == 0)
        return -1;

    MYTRACE(ACE_TEXT("CLIENT #%d: %s"), m_myuserid, command.c_str());

#if defined(UNICODE)
    ACE_CString data = UnicodeToUtf8(command.c_str());
#else
    const ACE_CString& data = command;
#endif /* UNICODE */

    bool const empty = m_sendbuffer.empty();
    m_sendbuffer += data; /* OnSend() mutex */
#if defined(ENABLE_ENCRYPTION)
    if ((m_crypt_stream != nullptr) && empty)
    {
        auto *reactor = m_crypt_stream->reactor();
        int const ret = reactor->register_handler(m_crypt_stream, ACE_Event_Handler::WRITE_MASK);
        TTASSERT(ret >= 0);
    }
#endif
    if ((m_def_stream != nullptr) && empty)
    {
        auto *reactor = m_def_stream->reactor();
        int const ret = reactor->register_handler(m_def_stream, ACE_Event_Handler::WRITE_MASK);
        TTASSERT(ret >= 0);
    }

    return (cmdid != 0)?cmdid:1;
}

ACE_INET_Addr ClientNode::GetLocalAddr()
{
    ACE_INET_Addr localaddr;
    if (m_localUdpAddr == ACE_INET_Addr())
    {
#if defined(ENABLE_ENCRYPTION)
        if (m_crypt_stream != nullptr)
        {
            int const ret = m_crypt_stream->peer().peer().get_local_addr(localaddr);
            TTASSERT(ret == 0);
            localaddr.set_port_number(0);
        }
#endif
        if (m_def_stream != nullptr)
        {
            int const ret = m_def_stream->peer().get_local_addr(localaddr);
            TTASSERT(ret == 0);
            localaddr.set_port_number(0);
        }
    }
    else
    {
        localaddr = m_localUdpAddr;
    }
    return localaddr;
}

#if defined(ENABLE_ENCRYPTION)
ACE_SSL_Context* ClientNode::SetupEncryptionContext()
{
    ASSERT_CLIENTNODE_LOCKED(this);

    if ((m_flags & CLIENT_CONNECTION) != 0u)
        return nullptr;
    
    CryptStreamHandler::RemoveSSLContext(GetEventLoop());

    return CryptStreamHandler::AddSSLContext(GetEventLoop());
}
#endif


void ClientNode::OnOpened()
{
    MYTRACE( ACE_TEXT("Connected successfully on TCP\n") );

    ACE_INET_Addr const localaddr = GetLocalAddr();
    
    MYTRACE(ACE_TEXT("UDP bind: %s. %d\n"), InetAddrToString(localaddr).c_str(), localaddr.get_type());

    if (m_packethandler.Open(localaddr, UDP_SOCKET_RECV_BUF_SIZE,
                             UDP_SOCKET_SEND_BUF_SIZE))
    {
        m_packethandler.AddListener(this);
    }
    else
    {
#if defined(ENABLE_ENCRYPTION)
        if (m_crypt_stream != nullptr)
        {
            m_crypt_stream->close();
        }
#endif
        if (m_def_stream != nullptr)
        {
            m_def_stream->close();
        }
    }
}

#if defined(ENABLE_ENCRYPTION)
void ClientNode::OnOpened(CryptStreamHandler::StreamHandler_t& /*handler*/)
{
    OnOpened();
}
#endif

void ClientNode::OnOpened(DefaultStreamHandler::StreamHandler_t& /*handler*/)
{
    OnOpened();
}

void ClientNode::OnClosed()
{
    GUARD_REACTOR(this);

    bool const encrypted = m_def_stream == nullptr;

#if defined(ENABLE_ENCRYPTION)
    m_crypt_stream = nullptr;
#endif
    m_def_stream = nullptr;
    
    if((m_flags & CLIENT_CONNECTED) != 0u)
    {
        m_flags &= ~CLIENT_CONNECTED;
        //Disconnect and clean up clientnode
        if(m_listener != nullptr)
            m_listener->OnConnectionLost();
    }
    else if((m_flags & CLIENT_CONNECTING) != 0u)
    {
        // try next resolved host before giving up
        if (m_serverinfo.hostaddrs.size() > 1)
        {
            m_serverinfo.hostaddrs.erase(m_serverinfo.hostaddrs.begin());
            u_short const udpport = m_serverinfo.udpaddr.get_port_number();
            m_serverinfo.udpaddr = m_serverinfo.hostaddrs[0];
            m_serverinfo.udpaddr.set_port_number(udpport);
            if (Connect(encrypted, m_serverinfo.hostaddrs[0],
                        m_localTcpAddr != ACE_INET_Addr() ? &m_localTcpAddr : nullptr))
                return;
        }

        m_flags &= ~CLIENT_CONNECTING;
        // Disconnect and clean up clientnode
        if (m_listener != nullptr)
        {
            /* ACE_SSL_Context::report_error() calls
             *   unsigned long const err = ::ERR_get_error ();
             *  ACE_OS::last_error (err); */
            auto ace_lasterr = ACE_OS::last_error();
            if (encrypted && (ace_lasterr != 0))
            {
                char sslerr_str[MAX_STRING_LENGTH] = "";
                ::ERR_error_string_n(ace_lasterr, sslerr_str, sizeof(sslerr_str)-1);
                MYTRACE(ACE_TEXT("SSL errno: 0x%x, %s\n"), ace_lasterr, sslerr_str);
                m_listener->OnEncryptionFailed(ace_lasterr, LocalToUnicode(sslerr_str));
            }
            m_listener->OnConnectFailed();
        }
    }
}

#if defined(ENABLE_ENCRYPTION)
void ClientNode::OnClosed(CryptStreamHandler::StreamHandler_t& /*handler*/)
{
    OnClosed();
}
#endif

void ClientNode::OnClosed(DefaultStreamHandler::StreamHandler_t& /*handler*/){
    OnClosed();
}

bool ClientNode::OnReceive(const char* buff, int len)
{
    TTASSERT(len>0);
    if(len>0)
    {
        m_recvbuffer.append(buff, len);
        ACE_CString cmd;
        ACE_CString remain;
        while(GetCmdLine(m_recvbuffer, cmd, remain))
        {
            if(ProcessCommand(cmd))
                m_recvbuffer = remain;
        }
    }
    return true;
}

#if defined(ENABLE_ENCRYPTION)
bool ClientNode::OnReceive(CryptStreamHandler::StreamHandler_t& /*handler*/, const char* buff, int len)
{
    return OnReceive(buff, len);
}
#endif

bool ClientNode::OnReceive(DefaultStreamHandler::StreamHandler_t& /*handler*/, const char* buff, int len)
{
    return OnReceive(buff, len);
}

bool ClientNode::OnSend(ACE_Message_Queue_Base& msgqueue)
{
    GUARD_REACTOR(this);
    
    if ((m_flags & (CLIENT_CONNECTING | CLIENT_CONNECTED)) != 0u)
    {
        ACE_Time_Value tm = ACE_Time_Value::zero;
        if((!m_sendbuffer.empty()) &&
           QueueStreamData(msgqueue, m_sendbuffer.c_str(), 
                           (int)m_sendbuffer.length(), &tm)<0)
        {
            return false;
        }
        m_sendbuffer.clear();
        return true;
    }
    return false;
}

#if defined(ENABLE_ENCRYPTION)
bool ClientNode::OnSend(CryptStreamHandler::StreamHandler_t& handler)
{
    return OnSend(*handler.msg_queue());
}
#endif


bool ClientNode::OnSend(DefaultStreamHandler::StreamHandler_t& handler)
{
    return OnSend(*handler.msg_queue());
}

bool ClientNode::ProcessCommand(const ACE_CString& cmdline)
{
    GUARD_REACTOR(this);

    ACE_CString tmp_cmd;
    if(!GetCmd(cmdline, tmp_cmd))
    {
        return true;
    }

#if defined(UNICODE)
    ACE_TString cmd = Utf8ToUnicode(tmp_cmd.c_str(), (int)tmp_cmd.length());
    ACE_TString command = Utf8ToUnicode(cmdline.c_str(), (int)cmdline.length());
#else
    ACE_TString const cmd = tmp_cmd;
    const ACE_CString& command = cmdline;
#endif

    MYTRACE(ACE_TEXT("SERVER #%d: %s"), m_myuserid, command.c_str());

    mstrings_t properties;
    if (ExtractProperties(command, properties) < 0)
    {
        MYTRACE(ACE_TEXT("Failed to extract properties from server command: %s\n"), command.c_str());
        return true;
    }
    
    //determine which commands will be executed
    if(cmd == SERVER_BEGINCMD) HandleBeginCmd(properties);
    else if(cmd == SERVER_ENDCMD) HandleEndCmd(properties);
    else if(cmd == m_serverinfo.systemid) HandleWelcome(properties);
    else if(cmd == SERVER_LOGINACCEPTED) HandleAccepted(properties);
    else if(cmd == SERVER_LOGGEDIN) HandleLoggedIn(properties);
    else if(cmd == SERVER_LOGGEDOUT) HandleLoggedOut(properties);
    else if(cmd == SERVER_ADDUSER) HandleAddUser(properties);
    else if(cmd == SERVER_UPDATEUSER) HandleUpdateUser(properties);
    else if(cmd == SERVER_REMOVEUSER) HandleRemoveUser(properties);
    else if(cmd == SERVER_ADDCHANNEL) HandleAddChannel(properties);
    else if(cmd == SERVER_UPDATECHANNEL) HandleUpdateChannel(properties);
    else if(cmd == SERVER_REMOVECHANNEL) HandleRemoveChannel(properties);
    else if(cmd == SERVER_JOINED) HandleJoinedChannel(properties);
    else if(cmd == SERVER_LEFTCHANNEL) HandleLeftChannel(properties);
    else if(cmd == SERVER_ADDFILE) HandleAddFile(properties);
    else if(cmd == SERVER_REMOVEFILE) HandleRemoveFile(properties);
    else if(cmd == SERVER_KEEPALIVE) HandleKeepAlive(properties);
    else if(cmd == SERVER_MESSAGE_DELIVER) HandleTextMessage(properties);
    else if(cmd == SERVER_KICKED) HandleKicked(properties);
    else if(cmd == SERVER_SERVERUPDATE) HandleServerUpdate(properties);
    else if(cmd == SERVER_ERROR) HandleCmdError(properties);
    else if(cmd == SERVER_COMMAND_OK) HandleOk(properties);
    else if(cmd == SERVER_BANNED) HandleBannedUser(properties);
    else if(cmd == SERVER_USERACCOUNT) HandleUserAccount(properties);
    else if(cmd == SERVER_ADDUSERACCOUNT) HandleAddUserAccount(properties);
    else if(cmd == SERVER_REMOVEUSERACCOUNT) HandleRemoveUserAccount(properties);
    else if(cmd == SERVER_FILE_ACCEPTED) HandleFileAccepted(properties);
    else if(cmd == SERVER_STATS) HandleServerStats(properties);
    else
    {
        m_listener->OnCommandError(m_current_cmdid,
                                   TT_CMDERR_INCOMPATIBLE_PROTOCOLS,
                                   GetErrorDescription(TT_CMDERR_INCOMPATIBLE_PROTOCOLS));
    }

    return true;
}

//Command handlers (Server -> Client)
void ClientNode::HandleWelcome(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    TTASSERT(!GetRootChannel());//root channel will be created by add channel command

    if (GetRootChannel())
        return;

    int userid = 0;
    if( GetProperty(properties, TT_PROTOCOL, m_serverinfo.protocol) &&
        GetProperty(properties, TT_USERID, userid) )
    {
        m_myuserid = userid;//set my User ID
        GetProperty(properties, TT_SERVERNAME, m_serverinfo.servername);
        GetProperty(properties, TT_MAXUSERS, m_serverinfo.maxusers);
        GetProperty(properties, TT_MAXLOGINSPERIP, m_serverinfo.max_logins_per_ipaddr);
        GetProperty(properties, TT_USERTIMEOUT, m_serverinfo.usertimeout);
        GetProperty(properties, TT_ACCESSTOKEN, m_serverinfo.accesstoken);
        GetProperty(properties, TT_LOGEVENTS, m_serverinfo.logevents);

        //start keepalive timer for TCP (if not set, then set it to half the user timeout)
        UpdateKeepAlive(GetKeepAlive());
        StartTimer(TIMER_TCPKEEPALIVE_ID, 0, m_keepalive.tcp_keepalive_interval,
                   m_keepalive.tcp_keepalive_interval);

        //start connecting on UDP to server. DoLogin() may not be called before UDP connect succeeds
        m_clientstats.udp_silence_sec = 0; // reset UDP timeout, since we're restarting a connection
        StartTimer(TIMER_UDPCONNECT_ID, 0, ACE_Time_Value::zero, m_keepalive.udp_connect_interval);

        //now wait for UDP timer to complete connection
    }
    else if(m_listener != nullptr)
        m_listener->OnCommandError(m_current_cmdid,
                                   TT_CMDERR_INCOMPATIBLE_PROTOCOLS,
                                   GetErrorDescription(TT_CMDERR_INCOMPATIBLE_PROTOCOLS));
}

void ClientNode::HandleAccepted(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    TTASSERT(m_flags & CLIENT_CONNECTED);
    int userid = 0;
    ACE_TString ipaddr;
    ACE_INET_Addr const udpaddr;

    if (GetProperty(properties, TT_USERID, userid) &&
        GetProperty(properties, TT_USERTYPE, m_myuseraccount.usertype))
    {
        GetProperty(properties, TT_IPADDR, ipaddr);
        GetProperties(properties, m_myuseraccount);

        TTASSERT(GetUserID() == userid);
        m_flags |= CLIENT_AUTHORIZED;

        m_local_voicelog->SetUsername(m_myuseraccount.username);

        //notify parent application
        if(m_listener != nullptr)
            m_listener->OnAccepted(GetUserID(), m_myuseraccount);
    }
}

void ClientNode::HandleServerUpdate(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    GetProperty(properties, TT_SERVERNAME, m_serverinfo.servername);
    GetProperty(properties, TT_MAXUSERS, m_serverinfo.maxusers);
    GetProperty(properties, TT_MAXLOGINATTEMPTS, m_serverinfo.maxloginattempts);
    GetProperty(properties, TT_MAXLOGINSPERIP, m_serverinfo.max_logins_per_ipaddr);
    GetProperty(properties, TT_LOGINDELAY, m_serverinfo.logindelay);
    GetProperty(properties, TT_USERTIMEOUT, m_serverinfo.usertimeout);
    GetProperty(properties, TT_AUTOSAVE, m_serverinfo.autosave);
    GetProperty(properties, TT_VOICETXLIMIT, m_serverinfo.voicetxlimit);
    GetProperty(properties, TT_VIDEOTXLIMIT, m_serverinfo.videotxlimit);
    GetProperty(properties, TT_MEDIAFILETXLIMIT, m_serverinfo.mediafiletxlimit);
    GetProperty(properties, TT_DESKTOPTXLIMIT, m_serverinfo.desktoptxlimit);
    GetProperty(properties, TT_TOTALTXLIMIT, m_serverinfo.totaltxlimit);
    GetProperty(properties, TT_ACCESSTOKEN, m_serverinfo.accesstoken);
    GetProperty(properties, TT_LOGEVENTS, m_serverinfo.logevents);

    if(!m_serverinfo.hostaddrs.empty())
    {
        int newtcpport = 0;
        int tcpport = m_serverinfo.hostaddrs[0].get_port_number();
        int newudpport = 0;
        int udpport = m_serverinfo.udpaddr.get_port_number();
        
        GetProperty(properties, TT_TCPPORT, newtcpport);
        MYTRACE_COND(newtcpport && newtcpport != tcpport,
                     ACE_TEXT("TCP port is different. Indicates server is behind NAT.\n"));
        // don't change m_serverinfo.udpaddr. This will not work for
        // servers behind NAT
        GetProperty(properties, TT_UDPPORT, newudpport);
        MYTRACE_COND(newudpport && newudpport != udpport,
                     ACE_TEXT("UDP port is different. Indicates server is behind NAT.\n"));
    }
    GetProperty(properties, TT_MOTD, m_serverinfo.motd);
    GetProperty(properties, TT_MOTDRAW, m_serverinfo.motd_raw);
    GetProperty(properties, TT_VERSION, m_serverinfo.version);

    UpdateKeepAlive(GetKeepAlive());

    //notify parent application
    m_listener->OnServerUpdate(m_serverinfo);
}

void ClientNode::HandleKeepAlive(const mstrings_t& /*properties*/)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    m_clientstats.tcp_silence_sec = 0;
    m_clientstats.tcpping_time = (ACE_OS::gettimeofday() - m_clientstats.ping_issue_time).msec();
    m_clientstats.tcp_ping_dirty = false;
}

void ClientNode::HandleLoggedIn(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    int userid = 0;
    ACE_TString nick;
    ACE_TString username;
    ACE_TString ipaddr;
    ACE_TString clientname;
    ACE_INET_Addr const udpaddr;
    int status = 0;
    ACE_TString statusmsg;
    ACE_TString version;
    UserTypes type = USERTYPE_NONE;
    Subscriptions localsubscript = SUBSCRIBE_NONE;
    Subscriptions peersubscript = SUBSCRIBE_NONE;
    int userdata = 0;

    if(!GetProperty(properties, TT_USERID, userid))
        return;

    TTASSERT(m_users.find(userid) == m_users.end());
    clientuser_t const user (new ClientUser(userid, this, m_listener, m_soundsystem));

    if(GetProperty(properties, TT_NICKNAME, nick))
        user->SetNickname(nick);
    if(GetProperty(properties, TT_USERNAME, username))
        user->SetUsername(username);
    if(GetProperty(properties, TT_IPADDR, ipaddr))
        user->SetIpAddress(ipaddr);
    if(GetProperty(properties, TT_STATUSMODE, status))
        user->SetStatusMode(status);
    if(GetProperty(properties, TT_STATUSMESSAGE, statusmsg))
        user->SetStatusMessage(statusmsg);
    if(GetProperty(properties, TT_VERSION, version))
        user->SetClientVersion(version);
    if(GetProperty(properties, TT_USERTYPE, type))
        user->SetUserType(type);
    if(GetProperty(properties, TT_LOCALSUBSCRIPTIONS, localsubscript))
        user->SetLocalSubscriptions(localsubscript);
    if(GetProperty(properties, TT_PEERSUBSCRIPTIONS, peersubscript))
        user->SetPeerSubscriptions(peersubscript);
    if(GetProperty(properties, TT_USERDATA, userdata))
        user->SetUserData(userdata);
    if(GetProperty(properties, TT_CLIENTNAME, clientname))
        user->SetClientName(clientname);

    m_users[userid] = user;

    m_listener->OnUserLoggedIn(*user);
}

void ClientNode::HandleLoggedOut(const mstrings_t& properties)
{    
    ASSERT_CLIENTNODE_LOCKED(this);

    int userid = 0;
    GetProperty(properties, TT_USERID, userid);

    if(userid == GetUserID() || userid == 0)
    {
        LoggedOut();

        if(m_listener != nullptr)
            m_listener->OnLoggedOut();
    }
    else
    {
        MYTRACE_COND(m_users.find(userid) == m_users.end(),
                     ACE_TEXT("Unknown user: #%d\n"), userid);

        TTASSERT(m_users.find(userid) != m_users.end());
        //if user is admin he might have file stream coming in
        clientuser_t const user = GetUser(userid);
        if (user)
            user->ResetAllStreams();
        m_users.erase(userid);

        if (user)
            m_listener->OnUserLoggedOut(*user);
    }
}

void ClientNode::HandleAddUser(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    int userid = 0;
    int chanid = 0;
    ACE_TString nickname;
    ACE_TString username;
    ACE_TString ipaddr;
    ACE_TString clientname;
    int statusmode = 0;
    ACE_TString statusmsg;
    ACE_TString version;
    int packet_protocol = 0;
    int userdata = 0;
    UserTypes type = USERTYPE_NONE;
    Subscriptions localsubscript = SUBSCRIBE_NONE;
    Subscriptions peersubscript = SUBSCRIBE_NONE;

    if(!GetProperty(properties, TT_USERID, userid))
        return;

    GetProperty(properties, TT_CHANNELID, chanid);
    clientchannel_t chan = GetChannel(chanid);
    if (!chan)
        return;

    clientuser_t user = GetUser(userid);
    if (!user) //view all users disabled scenario
        user = std::make_shared<ClientUser>(userid, this, m_listener, m_soundsystem);

    if(GetProperty(properties, TT_NICKNAME, nickname))
        user->SetNickname(nickname);
    if(GetProperty(properties, TT_USERNAME, username))
        user->SetUsername(username);
    if(GetProperty(properties, TT_STATUSMESSAGE, statusmsg))
        user->SetStatusMessage(statusmsg);
    if(GetProperty(properties, TT_STATUSMODE, statusmode))
        user->SetStatusMode(statusmode);
    if(GetProperty(properties, TT_PACKETPROTOCOL, packet_protocol))
        user->SetPacketProtocol(packet_protocol);
    if(GetProperty(properties, TT_VERSION, version))
        user->SetClientVersion(version);
    if(GetProperty(properties, TT_IPADDR, ipaddr))
        user->SetIpAddress(ipaddr);
    if(GetProperty(properties, TT_USERTYPE, type))
        user->SetUserType(type);
    if(GetProperty(properties, TT_LOCALSUBSCRIPTIONS, localsubscript))
        user->SetLocalSubscriptions(localsubscript);
    if(GetProperty(properties, TT_PEERSUBSCRIPTIONS, peersubscript))
        user->SetPeerSubscriptions(peersubscript);
    if(GetProperty(properties, TT_USERDATA, userdata))
        user->SetUserData(userdata);
    if(GetProperty(properties, TT_CLIENTNAME, clientname))
        user->SetClientName(clientname);

    TTASSERT(!user->GetChannel());
    user->SetChannel(chan);

    chan->AddUser(user->GetUserID(), user);

    // notify audio file muxer that user is in channel
    m_channelrecord.AddUser(userid, STREAMTYPE_VOICE, chanid);

    //special case for 'local user'
    if(userid == m_myuserid)
    {
        m_local_voicelog->SetNickname(nickname);
    }

    //notify parent application
    m_listener->OnUserJoinChannel(*user, *chan);
}

void ClientNode::HandleUpdateUser(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    int userid = 0;
    ACE_TString nickname;
    ACE_TString ipaddr;
    ACE_TString clientname;
    int statusmode = 0;
    ACE_TString statusmsg;
    Subscriptions localsubscript = SUBSCRIBE_NONE;
    Subscriptions peersubscript = SUBSCRIBE_NONE;

    if(!GetProperty(properties, TT_USERID, userid))
        return;

    clientuser_t const user = GetUser(userid);
    TTASSERT(user);
    if (!user)
        return;

    if(GetProperty(properties, TT_NICKNAME, nickname))
        user->SetNickname(nickname);
    if(GetProperty(properties, TT_IPADDR, ipaddr))
        user->SetIpAddress(ipaddr);
    if(GetProperty(properties, TT_STATUSMODE, statusmode))
        user->SetStatusMode(statusmode);
    if(GetProperty(properties, TT_STATUSMESSAGE, statusmsg))
        user->SetStatusMessage(statusmsg);
    if(GetProperty(properties, TT_LOCALSUBSCRIPTIONS, localsubscript))
        user->SetLocalSubscriptions(localsubscript);
    if(GetProperty(properties, TT_PEERSUBSCRIPTIONS, peersubscript))
        user->SetPeerSubscriptions(peersubscript);
    if(GetProperty(properties, TT_CLIENTNAME, clientname))
        user->SetClientName(clientname);

    user->ResetInactiveStreams();

    //special case for 'local user'
    if(userid == m_myuserid)
    {
        m_local_voicelog->SetNickname(nickname);
    }

    //notify parent application
    m_listener->OnUserUpdate(*user);
}

void ClientNode::HandleRemoveUser(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    int userid = 0;
    int channelid = 0;
    GetProperty(properties, TT_CHANNELID, channelid);
    GetProperty(properties, TT_USERID, userid);

    clientuser_t const user = GetUser(userid);
    MYTRACE_COND(!user, ACE_TEXT("Unknown user: #%d\n"), userid);
    TTASSERT(user.get());
    clientchannel_t const chan = GetChannel(channelid);
    TTASSERT(chan.get());
    if(!user || !chan)
        return;

    // notify audio file muxer that user left channel
    m_channelrecord.RemoveUser(userid, STREAMTYPE_VOICE);

    if(m_mychannel == chan && user->GetUserID() == m_myuserid)
        LeftChannel(*GetMyChannel());
    chan->RemoveUser(user->GetUserID());
    clientchannel_t nullc;
    user->SetChannel(nullc);

    //reset players if user is no longer active
    user->ResetInactiveStreams();

    //notify parent application
    m_listener->OnUserLeftChannel(*user, *chan);
}

void ClientNode::HandleAddChannel(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    ChannelProp chanprop;
    GetProperty(properties, TT_PARENTID, chanprop.parentid);
    GetProperty(properties, TT_CHANNELID, chanprop.channelid);

    clientchannel_t parent = GetChannel(chanprop.parentid);
    clientchannel_t newchan;
    if (parent)
    {
        TTASSERT(m_rootchannel);
        GetProperty(properties, TT_CHANNAME, chanprop.name);
        newchan = std::make_shared<ClientChannel>(parent, chanprop.channelid, chanprop.name);
        parent->AddSubChannel(newchan);
    }
    else
    {
        TTASSERT(!m_rootchannel);
        newchan = std::make_shared<ClientChannel>(chanprop.channelid);
        m_rootchannel = newchan;
    }

    if(GetProperty(properties, TT_PASSWORD, chanprop.passwd))
        newchan->SetPassword(chanprop.passwd);
    if(GetProperty(properties, TT_TOPIC, chanprop.topic))
        newchan->SetTopic(chanprop.topic);
    if(GetProperty(properties, TT_REQPASSWORD, chanprop.bProtected))
        newchan->SetPasswordProtected(chanprop.bProtected);
    if(GetProperty(properties, TT_OPERATORS, chanprop.setops))
        newchan->SetOperators(chanprop.setops);
    if(GetProperty(properties, TT_OPPASSWORD, chanprop.oppasswd))
        newchan->SetOpPassword(chanprop.oppasswd);
    if(GetProperty(properties, TT_DISKQUOTA, chanprop.diskquota))
        newchan->SetMaxDiskUsage(chanprop.diskquota);
    if(GetProperty(properties, TT_MAXUSERS, chanprop.maxusers))
        newchan->SetMaxUsers(chanprop.maxusers);
    if(GetProperty(properties, TT_AUDIOCODEC, chanprop.audiocodec))
        newchan->SetAudioCodec(chanprop.audiocodec);
    if(GetProperty(properties, TT_AUDIOCFG, chanprop.audiocfg))
        newchan->SetAudioConfig(chanprop.audiocfg);
    if(GetProperty(properties, TT_CHANNELTYPE, chanprop.chantype))
        newchan->SetChannelType(chanprop.chantype);
    if(GetProperty(properties, TT_USERDATA, chanprop.userdata))
        newchan->SetUserData(chanprop.userdata);

    GetProperty(properties, TT_VOICEUSERS, chanprop.transmitusers[STREAMTYPE_VOICE]);
    newchan->SetVoiceUsers(chanprop.transmitusers[STREAMTYPE_VOICE]);
    GetProperty(properties, TT_VIDEOUSERS, chanprop.transmitusers[STREAMTYPE_VIDEOCAPTURE]);
    newchan->SetVideoUsers(chanprop.transmitusers[STREAMTYPE_VIDEOCAPTURE]);
    GetProperty(properties, TT_DESKTOPUSERS, chanprop.transmitusers[STREAMTYPE_DESKTOP]);
    newchan->SetDesktopUsers(chanprop.transmitusers[STREAMTYPE_DESKTOP]);
    GetProperty(properties, TT_MEDIAFILEUSERS, chanprop.transmitusers[STREAMTYPE_MEDIAFILE]);
    newchan->SetMediaFileUsers(chanprop.transmitusers[STREAMTYPE_MEDIAFILE]);
    GetProperty(properties, TT_CHANMSGUSERS, chanprop.transmitusers[STREAMTYPE_CHANNELMSG]);
    newchan->SetChannelTextMsgUsers(chanprop.transmitusers[STREAMTYPE_CHANNELMSG]);
    GetProperty(properties, TT_TRANSMITSWITCHDELAY, chanprop.transmitswitchdelay);
    newchan->SetTransmitSwitchDelay(ToTimeValue(chanprop.transmitswitchdelay));
    GetProperty(properties, TT_TOTVOICE, chanprop.totvoice);
    newchan->SetTimeOutTimerVoice(ToTimeValue(chanprop.totvoice));
    GetProperty(properties, TT_TOTMEDIAFILE, chanprop.totmediafile);
    newchan->SetTimeOutTimerMediaFile(ToTimeValue(chanprop.totmediafile));

#if defined(ENABLE_ENCRYPTION)
    ACE_TString crypt_key;
    if(GetProperty(properties, TT_CRYPTKEY, crypt_key))
    {
        //check that key sizes match
        byte_t encrypt[CRYPTKEY_SIZE];
        memset(encrypt, 0, sizeof(encrypt));
        TTASSERT(sizeof(encrypt) == crypt_key.length()/2);
        if(sizeof(encrypt) == crypt_key.length()/2)
            HexStringToKey(crypt_key, encrypt);
        newchan->SetEncryptKey(encrypt);
    }
#endif

    m_listener->OnAddChannel(*newchan);
}

void ClientNode::HandleUpdateChannel(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    TTASSERT(GetRootChannel());
    ChannelProp chanprop;
    GetProperty(properties, TT_CHANNELID, chanprop.channelid);
    clientchannel_t const chan = GetChannel(chanprop.channelid);
    TTASSERT(chan.get());
    if (!chan)
        return;

    if(GetProperty(properties, TT_CHANNAME, chanprop.name))
        chan->SetName(chanprop.name);
    if(GetProperty(properties, TT_TOPIC, chanprop.topic))
        chan->SetTopic(chanprop.topic);
    if(GetProperty(properties, TT_REQPASSWORD, chanprop.bProtected))
        chan->SetPasswordProtected(chanprop.bProtected);
    if(GetProperty(properties, TT_OPERATORS, chanprop.setops))
        chan->SetOperators(chanprop.setops);
    if(GetProperty(properties, TT_DISKQUOTA, chanprop.diskquota))
        chan->SetMaxDiskUsage(chanprop.diskquota);
    if(GetProperty(properties, TT_MAXUSERS, chanprop.maxusers))
        chan->SetMaxUsers(chanprop.maxusers);
    if(GetProperty(properties, TT_PASSWORD, chanprop.passwd))
        chan->SetPassword(chanprop.passwd);
    if(GetProperty(properties, TT_OPPASSWORD, chanprop.oppasswd))
        chan->SetOpPassword(chanprop.oppasswd);
    if(GetProperty(properties, TT_CHANNELTYPE, chanprop.chantype))
        chan->SetChannelType(chanprop.chantype);
    if(GetProperty(properties, TT_USERDATA, chanprop.userdata))
        chan->SetUserData(chanprop.userdata);
    if(GetProperty(properties, TT_AUDIOCODEC, chanprop.audiocodec))
        chan->SetAudioCodec(chanprop.audiocodec);
    if(GetProperty(properties, TT_AUDIOCFG, chanprop.audiocfg))
        chan->SetAudioConfig(chanprop.audiocfg);
    if(GetProperty(properties, TT_TRANSMITQUEUE, chanprop.transmitqueue))
        chan->SetTransmitQueue(chanprop.transmitqueue);

    // as of protocol v5.4 the server only forwards transmit users if
    // it's not empty
    GetProperty(properties, TT_VOICEUSERS, chanprop.transmitusers[STREAMTYPE_VOICE]);
    chan->SetVoiceUsers(chanprop.transmitusers[STREAMTYPE_VOICE]);
    GetProperty(properties, TT_VIDEOUSERS, chanprop.transmitusers[STREAMTYPE_VIDEOCAPTURE]);
    chan->SetVideoUsers(chanprop.transmitusers[STREAMTYPE_VIDEOCAPTURE]);
    GetProperty(properties, TT_DESKTOPUSERS, chanprop.transmitusers[STREAMTYPE_DESKTOP]);
    chan->SetDesktopUsers(chanprop.transmitusers[STREAMTYPE_DESKTOP]);
    GetProperty(properties, TT_MEDIAFILEUSERS, chanprop.transmitusers[STREAMTYPE_MEDIAFILE]);
    chan->SetMediaFileUsers(chanprop.transmitusers[STREAMTYPE_MEDIAFILE]);
    GetProperty(properties, TT_CHANMSGUSERS, chanprop.transmitusers[STREAMTYPE_CHANNELMSG]);
    chan->SetChannelTextMsgUsers(chanprop.transmitusers[STREAMTYPE_CHANNELMSG]);
    GetProperty(properties, TT_TRANSMITSWITCHDELAY, chanprop.transmitswitchdelay);
    chan->SetTransmitSwitchDelay(ToTimeValue(chanprop.transmitswitchdelay));
    GetProperty(properties, TT_TOTVOICE, chanprop.totvoice);
    chan->SetTimeOutTimerVoice(ToTimeValue(chanprop.totvoice));
    GetProperty(properties, TT_TOTMEDIAFILE, chanprop.totmediafile);
    chan->SetTimeOutTimerMediaFile(ToTimeValue(chanprop.totmediafile));

#if defined(ENABLE_ENCRYPTION)
    ACE_TString crypt_key;
    if(GetProperty(properties, TT_CRYPTKEY, crypt_key))
    {
        //check that key sizes match
        byte_t encrypt[CRYPTKEY_SIZE];
        memset(encrypt, 0, sizeof(encrypt));
        TTASSERT(sizeof(encrypt) == crypt_key.length()/2);
        if(sizeof(encrypt) == crypt_key.length()/2)
            HexStringToKey(crypt_key, encrypt);
        chan->SetEncryptKey(encrypt);
    }
#endif

    m_listener->OnUpdateChannel(*chan);

    //close desktop session if no longer allowed to transmit
    if (m_desktop && chan == m_mychannel &&
        !m_mychannel->CanTransmit(m_myuserid, STREAMTYPE_DESKTOP))
    {
        CloseDesktopSession(false);
        m_listener->OnDesktopTransferUpdate(0, 0);
    }
}

void ClientNode::HandleRemoveChannel(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    int chanid = 0;
    GetProperty(properties, TT_CHANNELID, chanid);

    clientchannel_t const chan = GetChannel(chanid);
    TTASSERT(chan.get());
    if (chan)
    {
        clientchannel_t const parent = chan->GetParentChannel();
        //make sure it's not the channel we're in
        TTASSERT(GetMyChannel() != chan);

        TTASSERT(parent);
        if (parent)
            parent->RemoveSubChannel(chan->GetName());

        //notify parent application
        m_listener->OnRemoveChannel(*chan);
    }
}

void ClientNode::HandleJoinedChannel(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    int channelid = 0;
    GetProperty(properties, TT_CHANNELID, channelid);

    clientchannel_t chan = GetChannel(channelid);
    TTASSERT(chan);
    if (chan)
    {
#if defined(ENABLE_ENCRYPTION)
        ACE_TString crypt_key;
        if(GetProperty(properties, TT_CRYPTKEY, crypt_key))
        {
            //check that key sizes match
            byte_t encrypt[CRYPTKEY_SIZE];
            memset(encrypt, 0, sizeof(encrypt));
            TTASSERT(sizeof(encrypt) == crypt_key.length()/2);
            if(sizeof(encrypt) == crypt_key.length()/2)
                HexStringToKey(crypt_key, encrypt);
            chan->SetEncryptKey(encrypt);
        }
#endif
        JoinChannel(chan);

        if(m_listener != nullptr)
            m_listener->OnJoinedChannel(chan->GetChannelID());
    }
}

void ClientNode::HandleLeftChannel(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    int channelid = 0;
    GetProperty(properties, TT_CHANNELID, channelid);
    clientchannel_t const chan = GetChannel(channelid);
    if (chan)
    {
        LeftChannel(*chan);

        if(m_listener != nullptr)
            m_listener->OnLeftChannel(channelid);
    }
}

void ClientNode::HandleTextMessage(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    int msgtype = 0;
    TextMessage msg;
    GetProperty(properties, TT_MSGTYPE, msgtype);
    msg.msgType = (MsgType)msgtype;
    GetProperty(properties, TT_USERID, msg.to_userid);
    GetProperty(properties, TT_SRCUSERID, msg.from_userid);
    GetProperty(properties, TT_CHANNELID, msg.channelid);
    GetProperty(properties, TT_MSGCONTENT, msg.content);
    GetProperty(properties, TT_TEXTMSG_MORE, msg.more);

    clientuser_t const user = GetUser(msg.from_userid);
    if(user.get() != nullptr)
        msg.from_username = user->GetUsername();

    switch(msg.msgType)
    {
    case TTChannelMsg :
        TTASSERT(GetChannel(msg.channelid).get());
    case TTUserMsg :
    case TTBroadcastMsg :
    case TTCustomMsg :
        break;
    default :
        return;
    }

    //notify parent application
    m_listener->OnTextMessage(msg);
}

void ClientNode::HandleKicked(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    int kicker = 0;
    int channelid = 0;
    GetProperty(properties, TT_KICKERID, kicker);
    GetProperty(properties, TT_CHANNELID, channelid);

    clientuser_t const user = GetUser(kicker);
    //notify parent application
    m_listener->OnKicked(user, channelid);
}

void ClientNode::HandleCmdError(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    int errnum = 0;
    ACE_TString msg;
    GetProperty(properties, TT_ERRORNUM, errnum);
    GetProperty(properties, TT_ERRORMSG, msg);

    if(m_current_cmdid != 0u)
        m_waitingTransfers.erase(m_current_cmdid);

    //notify parent application
    if(m_listener != nullptr)
        m_listener->OnCommandError(m_current_cmdid, errnum, msg);
}

void ClientNode::HandleOk(const mstrings_t& /*properties*/)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    //notify parent application
    if(m_listener != nullptr)
        m_listener->OnCommandSuccess(m_current_cmdid);
}

void ClientNode::HandleBannedUser(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    BannedUser ban;

    GetProperty(properties, TT_BANTYPE, ban.bantype);
    GetProperty(properties, TT_IPADDR, ban.ipaddr);
    GetProperty(properties, TT_CHANNEL, ban.chanpath);
    GetProperty(properties, TT_NICKNAME, ban.nickname);
    GetProperty(properties, TT_USERNAME, ban.username);
    GetProperty(properties, TT_BANTIME, ban.bantime);
    GetProperty(properties, TT_BANOWNER, ban.owner);

    m_listener->OnBannedUser(ban);
}

void ClientNode::HandleUserAccount(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    UserAccount useraccount;
    GetProperties(properties, useraccount);

    m_listener->OnUserAccount(useraccount);
}

void ClientNode::HandleAddUserAccount(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    UserAccount useraccount;
    GetProperties(properties, useraccount);

    m_listener->OnAddUserAccount(useraccount);
}

void ClientNode::HandleRemoveUserAccount(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    ACE_TString username;
    GetProperty(properties, TT_USERNAME, username);

    m_listener->OnRemoveUserAccount(username);
}

void ClientNode::HandleFileAccepted(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    TTASSERT(!m_waitingTransfers.empty());

    int transferid = 0;
    GetProperty(properties, TT_TRANSFERID, transferid);

    auto const ite = m_waitingTransfers.find(m_current_cmdid);
    if(ite != m_waitingTransfers.end())
    {
        FileTransfer transfer = ite->second;
        transfer.transferid = transferid;

        m_waitingTransfers.erase(ite);

        filenode_t ptr;
        
#if defined(ENABLE_ENCRYPTION)
        if((m_crypt_stream != nullptr) && (!m_serverinfo.hostaddrs.empty()))
        {
            FileNode* f_ptr = nullptr;
            auto *reactor = m_crypt_stream->reactor();
            ACE_NEW(f_ptr, FileNode(*reactor, true, m_serverinfo.hostaddrs[0],
                                    m_serverinfo, transfer, this));
            ptr = filenode_t(f_ptr);
        }
        else
#endif
        {
            if ((m_def_stream != nullptr) && (!m_serverinfo.hostaddrs.empty()))
            {
                FileNode* f_ptr = nullptr;
                auto *reactor = m_def_stream->reactor();
                ACE_NEW(f_ptr, FileNode(*reactor, false, m_serverinfo.hostaddrs[0],
                                        m_serverinfo, transfer, this));
                ptr = filenode_t(f_ptr);
            }
        }

        if (ptr)
        {
            m_filetransfers[transferid] = ptr;
            ptr->BeginTransfer();
        }
    }
}

void ClientNode::HandleServerStats(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    ServerStats serverstats;

    ACE_INT64 uptime = 0;
    GetProperty(properties, TT_TOTALTX, serverstats.total_bytessent);
    GetProperty(properties, TT_TOTALRX, serverstats.total_bytesreceived);
    GetProperty(properties, TT_VOICETX, serverstats.voice_bytessent);
    GetProperty(properties, TT_VOICERX, serverstats.voice_bytesreceived);
    GetProperty(properties, TT_VIDEOCAPTX, serverstats.vidcap_bytessent);
    GetProperty(properties, TT_VIDEOCAPRX, serverstats.vidcap_bytesreceived);
    GetProperty(properties, TT_MEDIAFILETX, serverstats.mediafile_bytessent);
    GetProperty(properties, TT_MEDIAFILERX, serverstats.mediafile_bytesreceived);
    GetProperty(properties, TT_DESKTOPTX, serverstats.desktop_bytessent);
    GetProperty(properties, TT_DESKTOPRX, serverstats.desktop_bytesreceived);
    GetProperty(properties, TT_USERSSERVED, serverstats.usersservered);
    GetProperty(properties, TT_USERSPEAK, serverstats.userspeak);
    GetProperty(properties, TT_FILESTX, serverstats.files_bytessent);
    GetProperty(properties, TT_FILESRX, serverstats.files_bytesreceived);

    GetProperty(properties, TT_UPTIME, uptime);
    //not really stored as "start time"
    serverstats.starttime = ACE_Time_Value((time_t)uptime / 1000, 
                                             ((suseconds_t)uptime % 1000) * 1000);

    m_listener->OnServerStatistics(serverstats);
}

void ClientNode::HandleBeginCmd(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    int cmdid = 0;
    GetProperty(properties, TT_CMDID, cmdid);
    m_current_cmdid = cmdid;
    if((cmdid != 0) && (m_listener != nullptr))
        m_listener->OnCommandProcessing(cmdid, false);
}

void ClientNode::HandleEndCmd(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    int cmdid = 0;
    GetProperty(properties, TT_CMDID, cmdid);
    m_current_cmdid = 0;
    if((cmdid != 0) && (m_listener != nullptr))
        m_listener->OnCommandProcessing(cmdid, true);
}

void ClientNode::HandleAddFile(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    RemoteFile remotefile;
    GetProperty(properties, TT_FILEID, remotefile.fileid);
    GetProperty(properties, TT_FILEOWNER, remotefile.username);
    GetProperty(properties, TT_CHANNELID, remotefile.channelid);
    GetProperty(properties, TT_FILENAME, remotefile.filename);
    GetProperty(properties, TT_FILESIZE, remotefile.filesize);
    GetProperty(properties, TT_UPLOADTIME, remotefile.uploadtime);

    clientchannel_t const chan = GetChannel(remotefile.channelid);
    TTASSERT(chan);
    if (chan)
    {
        chan->AddFile(remotefile);

        if(m_listener != nullptr)
            m_listener->OnAddFile(*chan, remotefile);
    }
    else TTASSERT(0);
}

void ClientNode::HandleRemoveFile(const mstrings_t& properties)
{
    ASSERT_CLIENTNODE_LOCKED(this);

    int channelid = 0;
    ACE_TString filename;
    GetProperty(properties, TT_CHANNELID, channelid);
    GetProperty(properties, TT_FILENAME, filename);

    RemoteFile remotefile;
    clientchannel_t const chan = GetChannel(channelid);
    TTASSERT(chan);
    if (chan && chan->GetFile(filename, remotefile))
    {
        chan->RemoveFile(filename);

        if(m_listener != nullptr)
            m_listener->OnRemoveFile(*chan, remotefile);
    }
    else TTASSERT(0);
}

