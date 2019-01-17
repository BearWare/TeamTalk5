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

#include <codec/BmpFile.h>
#include <teamtalk/CodecCommon.h>
#include <teamtalk/ttassert.h>
#include <teamtalk/Commands.h>
#include <teamtalk/PacketLayout.h>
#include <teamtalk/PacketHelper.h>
#include <myace/MyACE.h>

#include <ace/Reactor.h>
#include <ace/INET_Addr.h>
#include <ace/Event_Handler.h>
#include <ace/Synch_Options.h>

#include "AudioContainer.h"

using namespace teamtalk;
using namespace std;
using namespace media;
using namespace soundsystem;
using namespace vidcap;
using namespace std::placeholders;

#define GEN_NEXT_ID(id) (++id==0?++id:id)

#define VIDEOFILE_ENCODER_FRAMES_MAX 3
#define VIDEOCAPTURE_ENCODER_FRAMES_MAX 3
#define VIDEOCAPTURE_LOCAL_FRAMES_MAX 10

#define UDP_SOCKET_RECV_BUF_SIZE 0x20000
#define UDP_SOCKET_SEND_BUF_SIZE 0x20000

#define LOCAL_USERID 0

ClientNode::ClientNode(const ACE_TString& version, ClientListener* listener)
                       : m_reactor(new ACE_Select_Reactor(), true) //Ensure we don't use ACE_WFMO_Reactor!!!
#ifdef _DEBUG
                       , m_reactor_thr_id(0)
                       , m_active_timerid(0)
                       , m_reactor_wait(0)
#endif
                       , m_flags(CLIENT_CLOSED)
                       , m_connector(&m_reactor, ACE_NONBLOCK)
                       , m_def_stream(NULL)
#if defined(ENABLE_ENCRYPTION)
                       , m_crypt_connector(&m_reactor)
                       , m_crypt_stream(NULL)
#endif
                       , m_packethandler(&m_reactor)
                       , m_listener(listener)
                       , m_myuserid(0)
                       , m_tcpkeepalive_interval(0)
                       , m_udpkeepalive_interval(CLIENT_UDPKEEPALIVE_INTERVAL)
                       , m_server_timeout(CLIENT_CONNECTIONLOST_DELAY)
                       , m_voice_stream_id(0)
                       , m_voice_pkt_counter(0)
                       , m_vidcap_stream_id(0)
                       , m_mediafile_stream_id(0)
                       , m_audiofile_pkt_counter(0)
                       , m_desktop_session_id(0)
                       , m_cmdid_counter(0)
                       , m_current_cmdid(0)
                       , m_mtu_data_size(MAX_PAYLOAD_DATA_SIZE)
                       , m_mtu_max_payload_size(MAX_PACKET_PAYLOAD_SIZE)
                       , m_version(version)
{

    this->reactor(&m_reactor);

    m_listener->RegisterEventSuspender(this);

    m_local_voicelog = clientuser_t(new ClientUser(LOCAL_USERID, this, m_listener));

    this->activate();
#if defined(_DEBUG)
    m_reactor_wait.acquire();
#endif

    m_soundprop.soundgroupid = SOUNDSYSTEM->OpenSoundGroup();
}

ClientNode::~ClientNode()
{
    //close reactor so no one can register new handlers
    int ret = m_reactor.end_reactor_event_loop();
    TTASSERT(ret>=0);
    this->wait();

    {
        //guard needed for disconnect since Logout and LeaveChannel are called
        GUARD_REACTOR(this);
        Disconnect();
        StopStreamingMediaFile();
        CloseVideoCapture();
        CloseSoundInputDevice();
        CloseSoundOutputDevice();
        CloseSoundDuplexDevices();
    }

    audiomuxer().StopThread();

    AUDIOCONTAINER::instance()->ReleaseAllAudio(m_soundprop.soundgroupid);
    SOUNDSYSTEM->RemoveSoundGroup(m_soundprop.soundgroupid);

    MYTRACE( (ACE_TEXT("~ClientNode\n")) );
}

int ClientNode::svc(void)
{
    int ret = m_reactor.owner (ACE_OS::thr_self ());
    assert(ret >= 0);

#if defined(_DEBUG)
    m_reactor_wait.release();
#endif

    m_reactor.run_reactor_event_loop ();
    MYTRACE( (ACE_TEXT("ClientNode reactor thread exited.\n")) );
    return 0;
}

void ClientNode::SuspendEventHandling()
{
    if(this->thr_count())
    {
        m_reactor.end_reactor_event_loop();
        MYTRACE( (ACE_TEXT("ClientNode reactor thread suspended.\n")) );
    }
}

void ClientNode::ResumeEventHandling()
{
    if(this->thr_count() == 0)
    {
        ACE_thread_t thr_id = 0;
        m_reactor.owner(&thr_id);
        TTASSERT(thr_id != ACE_OS::thr_self());

        MYTRACE( (ACE_TEXT("ClientNode reactor thread waiting.\n")) );
        if(thr_id != ACE_OS::thr_self())
            this->wait();
        MYTRACE( (ACE_TEXT("ClientNode reactor thread activating.\n")) );
        m_reactor.reset_reactor_event_loop();
        int ret = this->activate();
        assert(ret >= 0);
        MYTRACE( (ACE_TEXT("ClientNode reactor thread activated.\n")) );
    }
}

ACE_Lock& ClientNode::reactor_lock()
{
    // char name[100] = "";
    // if(ACE_OS::thr_id(name, sizeof(name))>0)
    // {
    //     MYTRACE("Reactor lock obtained by %s\n", name);
    // }
    // else
    // {
    //     MYTRACE("Reactor lock obtained by %p\n", ACE_OS::thr_self());
    // }
    return m_reactor.lock();
}

VoiceLogger& ClientNode::voicelogger()
{
    ASSERT_REACTOR_LOCKED(this);

    if(m_voicelogger.null())
    {
        VoiceLogger* vlog = new VoiceLogger(m_listener);
        m_voicelogger = voicelogger_t(vlog);
    }

    return *m_voicelogger;
}

AudioMuxer& ClientNode::audiomuxer()
{
    if(m_audiomuxer.null())
    {
        AudioMuxer* audmuxer = new AudioMuxer();
        m_audiomuxer = audiomuxer_t(audmuxer);
    }

    return *m_audiomuxer;
}

bool ClientNode::GetServerInfo(ServerInfo& info)
{
    ASSERT_REACTOR_LOCKED(this);

    if(m_flags & CLIENT_CONNECTED)
    {
        info = m_serverinfo;
        return true;
    }
    return false;
}

bool ClientNode::GetClientStatistics(ClientStats& stats)
{
    ASSERT_REACTOR_LOCKED(this);

    if(m_flags & CLIENT_CONNECTED)
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
    ASSERT_REACTOR_LOCKED(this);

    return m_rootchannel;
}

clientchannel_t ClientNode::GetMyChannel()
{
    ASSERT_REACTOR_LOCKED(this);

    return m_mychannel;
}

int ClientNode::GetChannelID()
{
    ASSERT_REACTOR_LOCKED(this);

    if(!m_mychannel.null())
        return m_mychannel->GetChannelID();
    return 0;
}

clientchannel_t ClientNode::GetChannel(int channelid)
{
    ASSERT_REACTOR_LOCKED(this);

    clientchannel_t c = GetRootChannel();
    if(!c.null())
    {
        if(m_mychannel.get() && m_mychannel->GetChannelID() == channelid)
            return m_mychannel; //most likely scenario
        else if(c->GetChannelID() == channelid)
            return c;
        else
            return c->GetSubChannel(channelid, true);//recursive (SLOW)
    }
    return clientchannel_t();
}

ACE_TString ClientNode::GetChannelPath(int channelid)
{
    ASSERT_REACTOR_LOCKED(this);

    ACE_TString chanpath;
    clientchannel_t chan = GetChannel(channelid);
    if(!chan.null())
        chanpath = chan->GetChannelPath();
    return chanpath;
}

bool ClientNode::GetChannelProp(int channelid, ChannelProp& prop)
{
    ASSERT_REACTOR_LOCKED(this);

    clientchannel_t chan = GetChannel(channelid);
    if(!chan.null())
    {
        prop = chan->GetChannelProp();
        return true;
    }
    return false;
}

clientuser_t ClientNode::GetUser(int userid, bool include_local/* = false*/)
{
    ASSERT_REACTOR_LOCKED(this);

    //fast search
    musers_t::iterator ite = m_users.find(userid);
    if(ite != m_users.end())
        return ite->second;
    //most likely search
    clientuser_t suser;
    if(m_mychannel.get())
        suser = m_mychannel->GetUser(userid, false);
    //slow search
    if(suser.null() && m_rootchannel.get())
        suser = m_rootchannel->GetUser(userid, true);
    //special case for local user properties
    if(userid == LOCAL_USERID && include_local)
        return m_local_voicelog;

    return suser;
}

clientuser_t ClientNode::GetUserByUsername(const ACE_TString& username)
{
    ASSERT_REACTOR_LOCKED(this);

    //fast search
    musers_t::iterator ite = m_users.begin();
    while(ite != m_users.end())
    {
        if(ite->second->GetUsername() == username)
            return ite->second;
        ite++;
    }

    //with view-all-users disabled we need to check channels
    if(m_rootchannel.get() && 
        (m_myuseraccount.userrights & USERRIGHT_VIEW_ALL_USERS) == 0)
    {
        ClientChannel::users_t users;
        m_rootchannel->GetUsers(users, true);
        for(size_t i=0;i<users.size();i++)
        {
            if(users[i]->GetUsername() == username)
                return users[i];
        }
    }
    return clientuser_t();
}

void ClientNode::GetUsers(std::set<int>& userids)
{
    ASSERT_REACTOR_LOCKED(this);

    //fast search
    musers_t::iterator ite = m_users.begin();
    while(ite != m_users.end())
    {
        userids.insert(ite->first);
        ite++;
    }
    //with view-all-users disabled we need to check channels
    if(m_rootchannel.get() && 
       (m_myuseraccount.userrights & USERRIGHT_VIEW_ALL_USERS) == 0)
    {
        ClientChannel::users_t users;
        m_rootchannel->GetUsers(users, true);
        for(size_t i=0;i<users.size();i++)
            userids.insert(users[i]->GetUserID());
    }
}

void ClientNode::SetKeepAliveInterval(int tcp_seconds, int udp_seconds)
{
    ASSERT_REACTOR_LOCKED(this);

    if(TimerExists(TIMER_TCPKEEPALIVE_ID))
        StopTimer(TIMER_TCPKEEPALIVE_ID);

    //reset keep alive counters (otherwise we might disconnect by mistake)
    m_clientstats.tcp_silence_sec = 0;
    m_clientstats.udp_silence_sec = 0;

    if(udp_seconds <= 0)
        m_udpkeepalive_interval = CLIENT_UDPKEEPALIVE_INTERVAL;
    else
        m_udpkeepalive_interval = udp_seconds;

    m_tcpkeepalive_interval = tcp_seconds;

    //only start keep alive timer if HandleWelcome has been called
    TTASSERT((m_serverinfo.usertimeout &&
              (m_flags & (CLIENT_CONNECTING | CLIENT_CONNECTED))) || 
             m_serverinfo.usertimeout == 0);
    if(m_serverinfo.usertimeout)
    {
        if(tcp_seconds <= 0)
            m_tcpkeepalive_interval = m_serverinfo.usertimeout>1?m_serverinfo.usertimeout/2:1;

        ACE_Time_Value tcptime(m_tcpkeepalive_interval);
        StartTimer(TIMER_TCPKEEPALIVE_ID, 0, tcptime, tcptime);
    }
}

void ClientNode::GetKeepAliveInterval(int &tcp, int& udp) const
{ 
    ASSERT_REACTOR_LOCKED(this);

    tcp = m_tcpkeepalive_interval; udp = m_udpkeepalive_interval;
}

void ClientNode::SetServerTimeout(int seconds)
{
    ASSERT_REACTOR_LOCKED(this);

    m_clientstats.tcp_silence_sec = m_clientstats.udp_silence_sec = 0;
    
    m_server_timeout = std::max(1, seconds);
}

long ClientNode::StartTimer(ACE_UINT32 timer_id, long userdata, 
                            const ACE_Time_Value& delay, 
                            const ACE_Time_Value& interval)
{
    TimerHandler* th;
    ACE_NEW_RETURN(th, TimerHandler(*this, timer_id, userdata), -1);

    //ensure we don't have duplicate timers
    bool stoptimer = StopTimer(timer_id);
    MYTRACE_COND(stoptimer, ACE_TEXT("Starting timer which was already active: %u\n"), timer_id);

    //make sure we don't hold reactor lock when scheduling timer
    {
        //lock timer set
        wguard_t g(lock_timers());
        TTASSERT(m_timers.find(timer_id) == m_timers.end());
        m_timers[timer_id] = th; //put in before schedule because timeout might be 0
    }

    long reactor_timerid = m_reactor.schedule_timer(th, 0, delay, interval);
    TTASSERT(reactor_timerid>=0);
    if(reactor_timerid<0)
    {
        //lock timer set
        wguard_t g(lock_timers());
        m_timers.erase(timer_id);
        delete th;
    }

    return reactor_timerid;
}

bool ClientNode::StopTimer(ACE_UINT32 timer_id)
{
    wguard_t g(lock_timers());
    
#ifdef _DEBUG
    //ensure timer isn't delete from inside a callback
    TTASSERT(timer_id != m_active_timerid);
#endif

    timer_handlers_t::iterator ii = m_timers.find(timer_id);
    if(ii != m_timers.end())
    {
        TimerHandler* th = ii->second;
        m_timers.erase(ii);
        g.release(); //don't hold reactor lock when cancelling

        if(m_reactor.cancel_timer(th, 0) != -1)
            return true;
    }
    return false;
}

void ClientNode::ClearTimer(ACE_UINT32 timer_id)
{
    //lock timer set
    wguard_t g(lock_timers());

    m_timers.erase(timer_id);
}

//Start/stop timers handled outside ClientNode
long ClientNode::StartUserTimer(ACE_UINT32 timer_id, int userid, 
                                long userdata, const ACE_Time_Value& delay, 
                                const ACE_Time_Value& interval/* = ACE_Time_Value::zero*/)
{
    TTASSERT(timer_id & USER_TIMER_MASK);

    return StartTimer(USER_TIMERID(timer_id, userid), userdata, delay, interval);
}

bool ClientNode::StopUserTimer(ACE_UINT32 timer_id, int userid)
{
    return StopTimer(USER_TIMERID(timer_id, userid));
}

bool ClientNode::TimerExists(ACE_UINT32 timer_id)
{
    TTASSERT((timer_id & USER_TIMER_MASK) == 0);

    //lock timer set
    rguard_t g(lock_timers());
    
    return m_timers.find(timer_id) != m_timers.end();
}

bool ClientNode::TimerExists(ACE_UINT32 timer_id, int userid)
{
    ACE_UINT32 tm_id = USER_TIMERID(timer_id, userid);
    //lock timer set
    rguard_t g(lock_timers());
    
    return m_timers.find(tm_id) != m_timers.end();
}

//TimerListener
int ClientNode::TimerEvent(ACE_UINT32 timer_event_id, long userdata)
{
    GUARD_REACTOR(this);

#ifdef _DEBUG
    //ensure timer isn't delete from inside a callback
    m_active_timerid = timer_event_id;
#endif

    int ret = -1;
    int userid = TIMER_USERID(timer_event_id);

    TTASSERT(TimerExists(timer_event_id, userid));

    switch(timer_event_id & TIMERID_MASK)
    {
    case TIMER_UDPCONNECT_ID :
    {
        SendPacket(HelloPacket(m_myuserid, GETTIMESTAMP()),
                   m_serverinfo.udpaddr);
        ret = 0;
    }
    break;
    case TIMER_UDPCONNECT_TIMEOUT_ID :
    {
        if(m_flags & CLIENT_CONNECTING)
            OnClosed();
        
        ret = -1;
    }
    break;
    case TIMER_ONE_SECOND_ID :
        ret = Timer_OneSecond();
        break;
    case TIMER_TCPKEEPALIVE_ID :
    {
        //send tcp keepalive
        DoPing(false);
        ret = 0;
    }
    break;
    case TIMER_UDPKEEPALIVE_ID :
        ret = Timer_UdpKeepAlive();
        break;
    case TIMER_UDPLIVENESS_ID :
        if(m_clientstats.udp_silence_sec >= 
           m_udpkeepalive_interval + CLIENT_UDP_CONNECTIONLOST_DELAY)
        {
            MYTRACE(ACE_TEXT("Recreating UDP socket due to connectivity problems\n"));
            RecreateUdpSocket();
            ret = 0;
        }
        else
        {
            ret = -1; //unregister
        }
        break;
    case TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID :
        ret = Timer_DesktopPacketRTX();
        break;
    case TIMER_DESKTOPNAKPACKET_TIMEOUT_ID :
        ret = Timer_DesktopNAKPacket();
        break;
    case TIMER_BUILD_DESKTOPPACKETS_ID :
        ret = Timer_BuildDesktopPackets();
        break;
    case TIMER_QUERY_MTU_ID :
        ret = Timer_QueryMTU(userdata);
        break;
    case USER_TIMER_VOICE_PLAYBACK_ID :
    {
        clientuser_t user = GetUser(userid);
        if(user.get())
            ret = user->TimerMonitorVoicePlayback();
        else
            ret = -1;
    }
    break;
    case USER_TIMER_MEDIAFILE_AUDIO_PLAYBACK_ID :
    {
        clientuser_t user = GetUser(userid);
        if(user.get())
            ret = user->TimerMonitorAudioFilePlayback();
        else
            ret = -1;
    }
    break;
    case USER_TIMER_MEDIAFILE_VIDEO_PLAYBACK_ID :
    {
        clientuser_t user = GetUser(userid);
        if(user.get())
            ret = user->TimerMonitorVideoFilePlayback();
        else
            ret = -1;
    }
    break;
    case USER_TIMER_DESKTOPACKPACKET_ID :
    {
        clientuser_t user = GetUser(userid);
        if(user.get())
            ret = user->TimerDesktopDelayedAck();
        else
            ret = -1;
    }
    break;
    case USER_TIMER_STOP_STREAM_MEDIAFILE_ID :
        StopStreamingMediaFile();
        ret = -1;
        break;
    case USER_TIMER_DESKTOPINPUT_RTX_ID :
    {
        clientuser_t user = GetUser(userid);
        if(user.null())
        {
            ret = -1;
            break;
        }

        //if user closed desktop session then just return
        desktop_viewer_t desktop = user->GetDesktopSession();
        if(desktop.null())
        {
            ret = -1;
            break;
        }
        //process desktop input RTX queue
        while(user->GetDesktopInputRtxQueue().size())
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

            //queue desktop input packet for RTX
            DesktopInputPacket* packet;
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

        clientuser_t user = GetUser(userid);
        if(user.null() || m_desktop.null() ||
           m_mychannel.null())
            break;

        DesktopInputAckPacket* ack_pkt;
        ACE_NEW_NORETURN(ack_pkt,
                         DesktopInputAckPacket(m_myuserid,
                                               GETTIMESTAMP(),
                                               m_desktop->GetSessionID(),
                                               user->GetNextDesktopInputRxPacketNo()-1));
        if(!ack_pkt)
            break;
        ack_pkt->SetChannel(m_mychannel->GetChannelID());
        ack_pkt->SetDestUser(userid);
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
        clientuser_t user = GetUser(userdata);
        if(user.get())
            m_listener->OnUserStateChange(*user);
        ret = -1;
    }
    break;
    default:
        TTASSERT(0);
        ret = -1;
    }
    //ensure timer is still there otherwise there will be a double delete
    TTASSERT(m_timers.find(timer_event_id) != m_timers.end());

#ifdef _DEBUG
    //ensure timer isn't delete from inside a callback
    m_active_timerid = 0;
#endif

    if(ret < 0)
    {
        ClearTimer(timer_event_id);
        
        //set of timers which should be restarted ugly....
        switch(timer_event_id & TIMERID_MASK)
        {
        case TIMER_UDPKEEPALIVE_ID :
        {
            //create new UDP timer
            ACE_Time_Value tm(CLIENT_UDPKEEPALIVE_CHECK_INTERVAL);
            ACE_Time_Value tm_rtx(CLIENT_UDPKEEPALIVE_RTX_INTERVAL);
            long timer_id = StartTimer(TIMER_UDPKEEPALIVE_ID, 0, tm, tm_rtx);
            TTASSERT(timer_id >= 0);
            break;
        }
        default :
            break;
        }
    }

    return ret;
}

int ClientNode::Timer_OneSecond()
{
    ASSERT_REACTOR_LOCKED(this);

    //Check server has replied to TCP keep alives and disconnect if inactive
    m_clientstats.tcp_silence_sec += 1;
    m_clientstats.udp_silence_sec += 1;

    //if no UDP packet has been received within the UDP keepalive time
    //then recreate the UDP socket
    if(m_clientstats.udp_silence_sec >= 
       m_udpkeepalive_interval + CLIENT_UDP_CONNECTIONLOST_DELAY && 
       !TimerExists(TIMER_UDPLIVENESS_ID))
    {
        StartTimer(TIMER_UDPLIVENESS_ID, 0, ACE_Time_Value::zero,
                   CLIENT_UDPLIVENESS_INTERVAL);
    }

    //check whether server has replied within the timeout limit on TCP and within
    //TIMER_UDPKEEPALIVE_INTERVAL_MS * 3 on UDP
    if(m_clientstats.tcp_silence_sec >= (ACE_UINT32)m_server_timeout ||
       (m_clientstats.udp_silence_sec >= (ACE_UINT32)m_server_timeout &&
        m_serverinfo.udpaddr != ACE_INET_Addr()))
    {

        m_packethandler.close();

#if defined(ENABLE_ENCRYPTION)
        if(m_crypt_stream)
        {
            m_crypt_stream->close(); // automatically calls OnClosed()
        }
#endif
        if(m_def_stream)
        {
            m_def_stream->close();
        }
    }

    return 0;
}

int ClientNode::Timer_UdpKeepAlive()
{
    ASSERT_REACTOR_LOCKED(this);

    //check if we still have a 'live' UDP connection to server
    bool call_again = false;

    KeepAlivePacket p(m_myuserid, GETTIMESTAMP());
    if(m_clientstats.udp_silence_sec >= m_udpkeepalive_interval)
    {
        SendPacket(p, m_serverinfo.udpaddr);
        call_again = true;
    }

    if(call_again)
        return 0;
    else
        return -1;
}

int ClientNode::Timer_BuildDesktopPackets()
{
    ASSERT_REACTOR_LOCKED(this);

    TTASSERT(!m_desktop.null());
    if(m_desktop.null())
        return -1;

    if(m_desktop->thr_count())
        return 0;

    TTASSERT(!m_desktop_tx.null());
    if(m_desktop_tx.null())
        return -1;

    desktoppackets_t packets;
    m_desktop->GetDesktopPackets(packets);
    if(packets.empty())
        return -1;

    //calculate bytes to send
    int total_size = 0, pk_size = 0;
    desktoppackets_t::const_iterator ii = packets.begin();
    while(ii != packets.end())
    {
        desktoppacket_t dp(*ii);
        m_desktop_tx->AddDesktopPacketToQueue(dp);

        total_size += (*ii)->GetPacketSize();
        if(pk_size < (*ii)->GetPacketSize())
            pk_size = (*ii)->GetPacketSize();
        ii++;
    }
    //MYTRACE(ACE_TEXT("Packets ready: %u, Total size: %d, Biggest packet: %d\n"), 
    //        packets.size(), total_size, pk_size);

    int session_id = m_desktop->GetSessionID();

    //get packets to send
    desktoppackets_t tx_packets;
    m_desktop_tx->GetNextDesktopPackets(tx_packets);

    desktoppackets_t::iterator dpi = tx_packets.begin();
    while(dpi != tx_packets.end())
    {
        DesktopPacket* dp;
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
        ACE_Time_Value rtx_interval(DESKTOP_RTX_TIMER_INTERVAL);
        
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

int ClientNode::Timer_DesktopPacketRTX()
{
    ASSERT_REACTOR_LOCKED(this);

    //retransmit lost desktop packets
    TTASSERT(!m_desktop_tx.null());
    if(!m_desktop_tx.null())
    {
        ACE_Time_Value rtx_timeout(DESKTOP_DEFAULT_RTX_TIMEOUT);
            /*GetDesktopPacketRTxTimeout(m_clientstats.udpping_time);*/
        desktoppackets_t rtx_packets;
        m_desktop_tx->GetLostDesktopPackets(rtx_timeout, rtx_packets, 1);
        desktoppackets_t::iterator dpi = rtx_packets.begin();
        while(dpi != rtx_packets.end())
        {
            DesktopPacket* dp;
            ACE_NEW_RETURN(dp, DesktopPacket(*(*dpi)), 0);
            if(!QueuePacket(dp))
                delete dp;

            dpi++;
        }
    }
    return 0;
}

int ClientNode::Timer_DesktopNAKPacket()
{
    ASSERT_REACTOR_LOCKED(this);

    if(!m_desktop_nak_tx.null() && !m_mychannel.null())
    {
        DesktopNakPacket* nak_pkt;
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

int ClientNode::Timer_QueryMTU(int mtu_index)
{
    ASSERT_REACTOR_LOCKED(this);

    TTASSERT(mtu_index < (int)MTU_QUERY_SIZES_COUNT);

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

    KeepAlivePacket* ka_pkt;

    uint32_t tm = GETTIMESTAMP();
    ACE_NEW_RETURN(ka_pkt, KeepAlivePacket(GetUserID(), tm, 
                                              MTU_QUERY_SIZES[mtu_index]), 0);

    ka_mtu_packet_t pkt(ka_pkt);

    if(SendPacket(*pkt, m_serverinfo.udpaddr)>0)
        m_mtu_packets[tm] = pkt;

    return 0;
}

void ClientNode::RecreateUdpSocket()
{
    ASSERT_REACTOR_LOCKED(this);

    //recreate the UDP socket which has the server connection
    m_packethandler.close();
    m_packethandler.open(m_localUdpAddr, UDP_SOCKET_RECV_BUF_SIZE, 
                         UDP_SOCKET_SEND_BUF_SIZE);

    if(TimerExists(TIMER_UDPCONNECT_ID))
        StopTimer(TIMER_UDPCONNECT_ID);

    StartTimer(TIMER_UDPCONNECT_ID, 0, ACE_Time_Value(),
               CLIENT_UDPCONNECT_INTERVAL);
}

void ClientNode::OpenAudioCapture(const AudioCodec& codec)
{
    int codec_samplerate = GetAudioCodecSampleRate(codec);
    int codec_samples = GetAudioCodecCbSamples(codec);
    int codec_channels = GetAudioCodecChannels(codec);
    int output_channels = GetAudioCodecSimulateStereo(codec)?2:codec_channels;

    rguard_t g_snd(lock_sndprop());

    if(codec_samples <= 0 || codec_samplerate <= 0 || codec_channels == 0 ||
       m_soundprop.inputdeviceid == SOUNDDEVICE_IGNORE_ID)
        return;

    int input_samplerate = 0, input_channels = 0, input_samples = 0;
    if(!SOUNDSYSTEM->SupportsInputFormat(m_soundprop.inputdeviceid,
                                       codec_channels, codec_samplerate))
    {
        DeviceInfo dev;
        if(!SOUNDSYSTEM->GetDevice(m_soundprop.inputdeviceid, dev) ||
           dev.default_samplerate == 0)
            return;
        
        //choose highest sample rate supported by device
        input_samplerate = dev.default_samplerate;
        //choose channels supported by device
        input_channels = dev.GetSupportedInputChannels(codec_channels);
        //get callback size for new samplerate
        input_samples = CalcSamples(codec_samplerate, codec_samples,
                                    input_samplerate);

        m_capture_resampler = MakeAudioResampler(input_channels,
                                                 input_samplerate,
                                                 codec_channels,
                                                 codec_samplerate);

        if(m_capture_resampler.null())
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

    bool b;
    if(m_flags & CLIENT_SNDINOUTPUT_DUPLEX)
    {
        DeviceInfo dev;
        SOUNDSYSTEM->GetDevice(m_soundprop.outputdeviceid, dev);
        assert(dev.SupportsOutputFormat(output_channels, input_samplerate));
        if(!dev.SupportsOutputFormat(output_channels, input_samplerate))
        {
            m_playback_resampler = MakeAudioResampler(output_channels,
                                                      input_samplerate, //sample rate shared in dpx mode
                                                      codec_channels,
                                                      codec_samplerate);

            if(m_playback_resampler.null())
            {
                m_playback_resampler.reset();
                m_listener->OnInternalError(TT_INTERR_SNDOUTPUT_FAILURE,
                                            ACE_TEXT("Cannot create resampler for sound output device"));
                return;
            }

            m_playback_buffer.resize(codec_samples * codec_channels);
        }

        b = SOUNDSYSTEM->OpenDuplexStream(this, m_soundprop.inputdeviceid,
                                        m_soundprop.outputdeviceid,
                                        m_soundprop.soundgroupid, 
                                        input_samplerate, input_channels,
                                        output_channels, input_samples);
    }
    else
        b = SOUNDSYSTEM->OpenInputStream(this, m_soundprop.inputdeviceid, 
                                       m_soundprop.soundgroupid,
                                       input_samplerate, input_channels, 
                                       input_samples);
    if(!b)
    {
        if(m_listener)
            m_listener->OnInternalError(TT_INTERR_SNDINPUT_FAILURE,
                                        ACE_TEXT("Failed to open sound input device"));
    }
}

void ClientNode::CloseAudioCapture()
{
    ASSERT_REACTOR_LOCKED(this);

    if(m_flags & CLIENT_SNDINOUTPUT_DUPLEX)
        SOUNDSYSTEM->CloseDuplexStream(this);
    else
        SOUNDSYSTEM->CloseInputStream(this);

    audiomuxer().QueueUserAudio(MUX_MYSELF_USERID, NULL, 
                                m_soundprop.samples_transmitted, true,
                                0, 0);
    m_soundprop.samples_transmitted = 0;
    m_soundprop.samples_recorded = 0;

    //clear capture resampler if initiated (in duplex mode)
    m_capture_resampler.reset();
    m_capture_buffer.clear();
    //clear playback resampler if initiated (in duplex mode)
    m_playback_resampler.reset();
    m_playback_buffer.clear();
}

bool ClientNode::UpdateSoundInputPreprocess()
{
    ASSERT_REACTOR_LOCKED(this);

    rguard_t g_snd(lock_sndprop());

    //if audio thread isn't running, then Speex preprocess is not set up
    if(m_voice_thread.codec().codec == CODEC_NO_CODEC)
        return true;

    int channels = GetAudioCodecChannels(m_voice_thread.codec());

#if defined(ENABLE_SPEEXDSP)
    //set AGC
    bool ret = true;
    wguard_t gp(m_voice_thread.m_preprocess_lock);

    SpeexAGC agc;
    agc.gain_level = (float)m_soundprop.speexdsp.agc_gainlevel;
    agc.max_increment = m_soundprop.speexdsp.agc_maxincdbsec;
    agc.max_decrement = m_soundprop.speexdsp.agc_maxdecdbsec;
    agc.max_gain = m_soundprop.speexdsp.agc_maxgaindb;

    //AGC
    ret &= m_voice_thread.m_preprocess_left.EnableAGC(m_soundprop.speexdsp.enable_agc);
    ret &= (channels == 1 || m_voice_thread.m_preprocess_right.EnableAGC(m_soundprop.speexdsp.enable_agc));
    ret &= m_voice_thread.m_preprocess_left.SetAGCSettings(agc);
    ret &= (channels == 1 || m_voice_thread.m_preprocess_right.SetAGCSettings(agc));

    //denoise
    ret &= m_voice_thread.m_preprocess_left.EnableDenoise(m_soundprop.speexdsp.enable_denoise);
    ret &= (channels == 1 || m_voice_thread.m_preprocess_right.EnableDenoise(m_soundprop.speexdsp.enable_denoise));
    ret &= m_voice_thread.m_preprocess_left.SetDenoiseLevel(m_soundprop.speexdsp.maxnoisesuppressdb);
    ret &= (channels == 1 || m_voice_thread.m_preprocess_right.SetDenoiseLevel(m_soundprop.speexdsp.maxnoisesuppressdb));

    //set AEC
    ret &= m_voice_thread.m_preprocess_left.EnableEchoCancel(m_soundprop.speexdsp.enable_aec);
    ret &= (channels == 1 || m_voice_thread.m_preprocess_right.EnableEchoCancel(m_soundprop.speexdsp.enable_aec));

    ret &= m_voice_thread.m_preprocess_left.SetEchoSuppressLevel(m_soundprop.speexdsp.aec_suppress_level);
    ret &= (channels == 1 || m_voice_thread.m_preprocess_right.SetEchoSuppressLevel(m_soundprop.speexdsp.aec_suppress_level));
    ret &= m_voice_thread.m_preprocess_left.SetEchoSuppressActive(m_soundprop.speexdsp.aec_suppress_active);
    ret &= (channels == 1 || m_voice_thread.m_preprocess_right.SetEchoSuppressActive(m_soundprop.speexdsp.aec_suppress_active));

    //set dereverb
    m_voice_thread.m_preprocess_left.EnableDereverb(m_soundprop.dereverb);
    if(channels == 2)
        m_voice_thread.m_preprocess_right.EnableDereverb(m_soundprop.dereverb);

    MYTRACE_COND(!ret, ACE_TEXT("Failed to set AGC settings\n"));

    if((m_soundprop.speexdsp.enable_agc || m_soundprop.speexdsp.enable_denoise || 
        m_soundprop.speexdsp.enable_aec) && !ret)
        return false;

    MYTRACE(ACE_TEXT("Set audio cfg. AGC: %d, %d, %d, %d, %d. Denoise: %d, %d. AEC: %d, %d, %d.\n"),
            m_soundprop.speexdsp.enable_agc, (int)m_soundprop.speexdsp.agc_gainlevel,
            m_soundprop.speexdsp.agc_maxincdbsec,m_soundprop.speexdsp.agc_maxdecdbsec,
            m_soundprop.speexdsp.agc_maxgaindb, m_soundprop.speexdsp.enable_denoise,
            m_soundprop.speexdsp.maxnoisesuppressdb, m_soundprop.speexdsp.enable_aec,
            m_soundprop.speexdsp.aec_suppress_level, m_soundprop.speexdsp.aec_suppress_active);

    return true;
#else
    return false;
#endif
}

void ClientNode::QueueAudioFrame(const media::AudioFrame& audframe)
{
    TTASSERT(audframe.userdata == STREAMTYPE_VOICE);

    m_voice_thread.QueueAudio(audframe);

    if((m_flags & CLIENT_TX_VOICE) ||
       ((m_flags & CLIENT_SNDINPUT_VOICEACTIVATED) && 
        (m_flags & CLIENT_SNDINPUT_VOICEACTIVE)))
    {
        audiomuxer().QueueUserAudio(MUX_MYSELF_USERID, audframe.input_buffer, 
                                    m_soundprop.samples_transmitted, 
                                    false, audframe.input_samples,
                                    audframe.inputfmt.channels);

        m_soundprop.samples_transmitted += audframe.input_samples;
    }
    else if(m_flags & CLIENT_MUX_AUDIOFILE)
        audiomuxer().QueueUserAudio(MUX_MYSELF_USERID, NULL, 
                                    m_soundprop.samples_transmitted, true,
                                    0, 0);

    if(AUDIOCONTAINER::instance()->AddAudio(m_soundprop.soundgroupid,
                                            0, STREAMTYPE_VOICE,
                                            m_voice_stream_id, 
                                            audframe.inputfmt.samplerate,
                                            audframe.inputfmt.channels,
                                            audframe.input_buffer,
                                            audframe.input_samples,
                                            m_soundprop.samples_recorded))
    {
        m_listener->OnUserAudioBlock(0, STREAMTYPE_VOICE);
    }

    m_soundprop.samples_recorded += audframe.input_samples;
}

void ClientNode::SendVoicePacket(const VoicePacket& packet)
{
    ASSERT_REACTOR_LOCKED(this);

    TTASSERT(packet.Finalized());

#ifdef ENABLE_ENCRYPTION
    if(m_crypt_stream)
    {
        clientchannel_t chan = GetChannel(packet.GetChannel());
        if(chan.null())
            return;

        CryptVoicePacket crypt_pkt(packet, chan->GetEncryptKey());
        if(m_myuseraccount.userrights & USERRIGHT_TRANSMIT_VOICE)
            SendPacket(crypt_pkt, m_serverinfo.udpaddr);
        TTASSERT(crypt_pkt.ValidatePacket());
    }
    else
#endif
    {
        TTASSERT(m_def_stream);
        if(m_myuseraccount.userrights & USERRIGHT_TRANSMIT_VOICE)
            SendPacket(packet, m_serverinfo.udpaddr);
        TTASSERT(packet.ValidatePacket());
    }

    //int ss = tx_pkt->GetPacketSize();
    //MYTRACE(ACE_TEXT("Packet no %d became %d bytes\n"),
    //    packet.GetPacketNumber(), packet.GetPacketSize());
}

void ClientNode::SendAudioFilePacket(const AudioFilePacket& packet)
{
    ASSERT_REACTOR_LOCKED(this);

    TTASSERT(packet.Finalized());
    TTASSERT(packet.GetKind() == PACKET_KIND_MEDIAFILE_AUDIO);

#ifdef ENABLE_ENCRYPTION
    if(m_crypt_stream)
    {
        clientchannel_t chan = GetChannel(packet.GetChannel());
        if(chan.null())
            return;

        CryptAudioFilePacket crypt_pkt(packet, chan->GetEncryptKey());
        if(m_myuseraccount.userrights & USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO)
            SendPacket(crypt_pkt, m_serverinfo.udpaddr);
        TTASSERT(crypt_pkt.ValidatePacket());
    }
    else
#endif
    {
        MYTRACE_COND(!m_def_stream, ACE_TEXT("Sending UDP data on closed connected\n"));
        if(m_myuseraccount.userrights & USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO)
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
        if((m_flags & CLIENT_SNDINPUT_VOICEACTIVE) &&
           (m_flags & CLIENT_SNDINPUT_VOICEACTIVATED))
        {
            m_flags &= ~CLIENT_SNDINPUT_VOICEACTIVE;
            m_listener->OnVoiceActivated(false);
        }
        return;
    }
    else if((m_flags & CLIENT_SNDINPUT_VOICEACTIVE) == 0 &&
            (m_flags & CLIENT_SNDINPUT_VOICEACTIVATED))
    {
        m_flags |= CLIENT_SNDINPUT_VOICEACTIVE;
        m_listener->OnVoiceActivated(true);

        if((m_flags & CLIENT_TX_VOICE) == 0)
            GEN_NEXT_ID(m_voice_stream_id);
    }
    //MYTRACE(ACE_TEXT("Queue voice packet #%d at TS: %u, pkt time: %u\n"),
    //        m_voice_pkt_counter, GETTIMESTAMP(), org_frame.timestamp);

    VoicePacket* newpacket;
    if (GetAudioCodecFramesPerPacket(codec)>1 && GetAudioCodecVBRMode(codec))
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
    
    AudioFilePacket* newpacket;
    if (GetAudioCodecFramesPerPacket(codec)>1 && GetAudioCodecVBRMode(codec))
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


void ClientNode::StreamCaptureCb(const soundsystem::InputStreamer& streamer,
                                 const short* buffer, int n_samples)
{
    rguard_t g_snd(lock_sndprop());

    int codec_samplerate = GetAudioCodecSampleRate(m_voice_thread.codec());
    int codec_samples = GetAudioCodecCbSamples(m_voice_thread.codec());
    int codec_channels = GetAudioCodecChannels(m_voice_thread.codec());

    const short* capture_buffer = NULL;
    if(!m_capture_resampler.null())
    {
        assert((int)m_capture_buffer.size() == codec_samples * codec_channels);
        int ret = m_capture_resampler->Resample(buffer, n_samples, 
                                                &m_capture_buffer[0],
                                                codec_samples);
        assert(ret > 0);
        assert(ret <= codec_samples);
        MYTRACE_COND(ret != codec_samples,
                     ACE_TEXT("Resampler output incorrect no. samples, expect %d, got %d\n"),
                     codec_samples, ret);
        capture_buffer = &m_capture_buffer[0];
    }
    else
        capture_buffer = buffer;

    AudioFrame audframe;
    audframe.force_enc = (m_flags & CLIENT_TX_VOICE);
    audframe.voiceact_enc = (m_flags & CLIENT_SNDINPUT_VOICEACTIVATED);
    audframe.soundgrpid = m_soundprop.soundgroupid;
    audframe.inputfmt.channels = codec_channels;
    audframe.input_buffer = const_cast<short*>(capture_buffer);
    audframe.input_samples = codec_samples;
    audframe.inputfmt.samplerate = codec_samplerate;
    audframe.userdata = STREAMTYPE_VOICE;
    
    QueueAudioFrame(audframe);
}

void ClientNode::StreamDuplexEchoCb(const soundsystem::DuplexStreamer& streamer,
                                    const short* input_buffer, 
                                    const short* prev_output_buffer, int n_samples)
{
    rguard_t g_snd(lock_sndprop());

    int codec_samplerate = GetAudioCodecSampleRate(m_voice_thread.codec());
    int codec_samples = GetAudioCodecCbSamples(m_voice_thread.codec());
    int codec_channels = GetAudioCodecChannels(m_voice_thread.codec());

    const short* capture_buffer = NULL;
    if(!m_capture_resampler.null())
    {
        assert((int)m_capture_buffer.size() == codec_samples * codec_channels);
        int ret = m_capture_resampler->Resample(input_buffer, n_samples, 
                                                &m_capture_buffer[0],
                                                codec_samples);
        assert(ret > 0);
        assert(ret <= codec_samples);
        MYTRACE_COND(ret != codec_samples,
                     ACE_TEXT("Resampler output incorrect no. samples, expect %d, got %d\n"),
                     codec_samples, ret);

        capture_buffer = &m_capture_buffer[0];
    }
    else
        capture_buffer = input_buffer;

    const short* playback_buffer = NULL;
    if(!m_playback_resampler.null())
    {
        assert((int)m_playback_buffer.size() == codec_samples * codec_channels);
        int ret = m_playback_resampler->Resample(prev_output_buffer, n_samples, 
                                                 &m_playback_buffer[0],
                                                 codec_samples);
        assert(ret>0);
        assert(ret <= codec_samples);
        MYTRACE_COND(ret != codec_samples,
                     ACE_TEXT("Resampler output incorrect no. samples, expect %d, got %d\n"),
                     codec_samples, ret);

        playback_buffer = &m_playback_buffer[0];
    }
    else
        playback_buffer = prev_output_buffer;

    AudioFrame audframe;
    audframe.force_enc = (m_flags & CLIENT_TX_VOICE);
    audframe.voiceact_enc = (m_flags & CLIENT_SNDINPUT_VOICEACTIVATED);
    audframe.soundgrpid = m_soundprop.soundgroupid;
    audframe.inputfmt.channels = codec_channels;
    audframe.input_buffer = const_cast<short*>(capture_buffer);
    audframe.input_samples = codec_samples;
    audframe.inputfmt.samplerate = codec_samplerate;
    audframe.outputfmt.channels = codec_channels;
    audframe.output_buffer = playback_buffer;
    audframe.output_samples = codec_samples;
    audframe.outputfmt.samplerate = codec_samplerate;
    audframe.userdata = STREAMTYPE_VOICE;

    QueueAudioFrame(audframe);
}

bool ClientNode::VideoCaptureRGB32Callback(media::VideoFrame& video_frame,
                                           ACE_Message_Block* mb_video)
{
    ACE_Time_Value tm_zero;
    if (m_local_vidcapframes.enqueue(mb_video, &tm_zero) < 0)
    {
        //make room by deleting oldest video frame
        ACE_Message_Block* mb;
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

    if(mb_video)
    {
        m_vidcap_thread.QueueFrame(mb_video);
        return true;
    }
    else
    {
        m_vidcap_thread.QueueFrame(video_frame);
    }
    return false;
}

bool ClientNode::VideoCaptureDualCallback(media::VideoFrame& video_frame,
                                          ACE_Message_Block* mb_video)
{
    if ((m_flags & CLIENT_TX_VIDEOCAPTURE))
    {
        return VideoCaptureEncodeCallback(video_frame, mb_video);
    }
    else
    {
        return VideoCaptureRGB32Callback(video_frame, mb_video);
    }
}

bool ClientNode::MediaStreamVideoCallback(MediaStreamer* streamer,
                                          media::VideoFrame& video_frame,
                                          ACE_Message_Block* mb_video)
{
    TTASSERT(m_flags & CLIENT_STREAM_VIDEOFILE);
    TTASSERT(!m_videofile_thread.null());
    if(m_videofile_thread.null())
        return false;

    VideoFormat cap_format = m_videofile_thread->GetVideoFormat();
    assert(RGB32_BYTES(cap_format.width, cap_format.height) == video_frame.frame_length);

//     static int x = 1;
//     if((x++ % 200) == 0)
//         WriteBitmap(i2string(x) + ACE_TString(".bmp"), video_frame.width, video_frame.height, 4,
//                     video_frame.frame, video_frame.frame_length);
    m_videofile_thread->QueueFrame(mb_video);

    return true; //m_video_thread always takes ownership
}

bool ClientNode::MediaStreamAudioCallback(MediaStreamer* streamer,
                                          AudioFrame& audio_frame,
                                          ACE_Message_Block* mb_audio)
{
    TTASSERT(m_flags & CLIENT_STREAM_AUDIOFILE);
/*
    static int x = 0;
    static ACE_UINT32 start = GETTIMESTAMP();
    ACE_UINT32 now = GETTIMESTAMP();
    MYTRACE(ACE_TEXT("Sent %d msec at %u. Duration: %u. Frame time: %u\n"), 
        x += GetAudioCodecCbMillis(m_audiofile_thread.codec()), 
        GETTIMESTAMP(), now - start, audio_frame.timestamp);
    assert(audio_frame.timestamp < now + 2000);
*/
    AudioCodec codec = m_audiofile_thread.codec();

    TTASSERT(audio_frame.input_samples == GetAudioCodecCbSamples(codec));
    TTASSERT(audio_frame.inputfmt.channels);
    TTASSERT(audio_frame.input_buffer);
    TTASSERT(audio_frame.inputfmt.samplerate);
    TTASSERT(audio_frame.input_samples);

    audio_frame.force_enc = true;
    audio_frame.userdata = STREAMTYPE_MEDIAFILE_AUDIO;
    m_audiofile_thread.QueueAudio(mb_audio);

    return true;  //m_video_thread always takes ownership
}

void ClientNode::MediaStreamStatusCallback(MediaStreamer* streamer,
                                           const MediaFileProp& mfp,
                                           MediaStreamStatus status)
{
    TTASSERT(m_flags & (CLIENT_STREAM_AUDIOFILE | CLIENT_STREAM_VIDEOFILE));

    MediaFileStatus mfs = MFS_CLOSED;

    switch(status)
    {
    case MEDIASTREAM_STARTED :
        mfs = MFS_STARTED;
        break;
    case MEDIASTREAM_ERROR :
        mfs = MFS_ERROR;
        StartUserTimer(USER_TIMER_STOP_STREAM_MEDIAFILE_ID, 0, 0, 
                       ACE_Time_Value::zero);
        break;
    case MEDIASTREAM_FINISHED :
        mfs = MFS_FINISHED;
        StartUserTimer(USER_TIMER_STOP_STREAM_MEDIAFILE_ID, 0, 0, 
                       ACE_Time_Value::zero);
        break;
    }

    m_listener->OnChannelStreamMediaFile(mfp, mfs);
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
    ASSERT_REACTOR_LOCKED(this);
    
    filenodes_t::iterator ite = m_filetransfers.find(transferid);
    if(ite != m_filetransfers.end())
        transfer = ite->second->GetFileTransferInfo();
    return ite != m_filetransfers.end();
}

bool ClientNode::CancelFileTransfer(int transferid)
{
    ASSERT_REACTOR_LOCKED(this);
    
    filenodes_t::iterator ite = m_filetransfers.find(transferid);
    if(ite != m_filetransfers.end())
    {
        m_filetransfers.erase(ite);
        return true;
    }
    return false;
}

void ClientNode::ReceivedPacket(PacketHandler* ph,
                                const char* packet_data, int packet_size, 
                                const ACE_INET_Addr& addr)
{
    ASSERT_REACTOR_THREAD(m_reactor);

    GUARD_REACTOR(this);

    m_clientstats.udpbytes_recv += packet_size;

    MYTRACE_COND(m_serverinfo.udpaddr != addr, 
                 ACE_TEXT("Received packet not from server\n"));

    if(m_serverinfo.udpaddr != addr)
        return;

    //used for validating that the UDP connections is still open
    FieldPacket packet(packet_data, packet_size);

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

    clientuser_t user = GetUser(packet.GetSrcUserID());

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
    FieldPacket chanpacket(packet_data, packet_size);

    clientchannel_t chan = GetChannel(chanpacket.GetChannel());
    if(chan.null())
    {
        MYTRACE(ACE_TEXT("Received FieldPacket without a specified channel\n"));
        return;
    }
    
    switch(chanpacket.GetKind())
    {
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_VOICE_CRYPT :
    {
        CryptVoicePacket crypt_pkt(packet_data, packet_size);
        VoicePacket* decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        MYTRACE_COND(!decrypt_pkt, ACE_TEXT("Failed to decrypted voice packet from #%d\n"),
                     crypt_pkt.GetSrcUserID());
        if(!decrypt_pkt)
            return;
        packet_ptr_t ptr(decrypt_pkt);
        MYTRACE_COND(user.null(),
                     ACE_TEXT("Received crypt voice packet from unknown user #%d\n"),
                     packet.GetSrcUserID());
        m_clientstats.voicebytes_recv += packet_size;
        bool no_record = (chan->GetChannelType() & CHANNEL_NO_RECORDING) &&
            (GetMyUserAccount().userrights & USERRIGHT_RECORD_VOICE) == USERRIGHT_NONE;
        if(!user.null())
            user->AddVoicePacket(*decrypt_pkt, m_soundprop, voicelogger(), !no_record);
    }
    break;
#endif
    case PACKET_KIND_VOICE :
    {
        VoicePacket audio_pkt(packet_data, packet_size);
        MYTRACE_COND(user.null(),
                     ACE_TEXT("Received voice packet from unknown user #%d\n"),
                     packet.GetSrcUserID());
        m_clientstats.voicebytes_recv += packet_size;
        bool no_record = (chan->GetChannelType() & CHANNEL_NO_RECORDING) &&
            (GetMyUserAccount().userrights & USERRIGHT_RECORD_VOICE) == USERRIGHT_NONE;
        if(!user.null())
            user->AddVoicePacket(audio_pkt, m_soundprop, voicelogger(), !no_record);
        break;
    }
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_MEDIAFILE_AUDIO_CRYPT :
    {
        CryptAudioFilePacket crypt_pkt(packet_data, packet_size);
        AudioFilePacket* decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        MYTRACE_COND(!decrypt_pkt, ACE_TEXT("Failed to decrypted audio packet from #%d\n"),
                     crypt_pkt.GetSrcUserID());
        if(!decrypt_pkt)
            return;
        packet_ptr_t ptr(decrypt_pkt);
        MYTRACE_COND(user.null(),
                     ACE_TEXT("Received CryptAudioFilePacket from unknown user #%d"),
                     packet.GetSrcUserID());
        m_clientstats.mediafile_audio_bytes_recv += packet_size;
        if(!user.null())
            user->AddAudioFilePacket(*decrypt_pkt, m_soundprop);
    }
    break;
#endif
    case PACKET_KIND_MEDIAFILE_AUDIO :
    {
        AudioFilePacket audio_pkt(packet_data, packet_size);
        MYTRACE_COND(user.null(),
                     ACE_TEXT("Received AudioFilePacket from unknown user #%d"),
                     packet.GetSrcUserID());
        m_clientstats.mediafile_audio_bytes_recv += packet_size;
        if(!user.null())
            user->AddAudioFilePacket(audio_pkt, m_soundprop);
        break;
    }
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_VIDEO_CRYPT :
    {
        CryptVideoCapturePacket crypt_pkt(packet_data, packet_size);
        VideoPacket* decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        packet_ptr_t ptr(decrypt_pkt);
        MYTRACE_COND(user.null(),
                     ACE_TEXT("Received CryptVideoCapturePacket from unknown user #%d"),
                     packet.GetSrcUserID());
        m_clientstats.vidcapbytes_recv += packet_size;
        if(!user.null())
            user->AddVideoCapturePacket(*decrypt_pkt, *chan);
    }
    break;
#endif
    case PACKET_KIND_VIDEO :
    {
        VideoCapturePacket video_pkt(packet_data, packet_size);
        MYTRACE_COND(user.null(),
                     ACE_TEXT("Received VideoCapturePacket from unknown user #%d\n"),
                     packet.GetSrcUserID());
        m_clientstats.vidcapbytes_recv += packet_size;
        if(!user.null())
            user->AddVideoCapturePacket(video_pkt, *chan);
    }
    break;
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_MEDIAFILE_VIDEO_CRYPT :
    {
        CryptVideoFilePacket crypt_pkt(packet_data, packet_size);
        VideoFilePacket* decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        packet_ptr_t ptr(decrypt_pkt);
        MYTRACE_COND(user.null(),
                     ACE_TEXT("Received CryptVideoCapturePacket from unknown user #%d"),
                     packet.GetSrcUserID());
        m_clientstats.mediafile_video_bytes_recv += packet_size;
        if(!user.null())
            user->AddVideoFilePacket(*decrypt_pkt, *chan);
    }
    break;
#endif
    case PACKET_KIND_MEDIAFILE_VIDEO :
    {
        VideoFilePacket video_pkt(packet_data, packet_size);
        MYTRACE_COND(user.null(),
                     ACE_TEXT("Received VideoFilePacket from unknown user #%d\n"),
                     packet.GetSrcUserID());
        m_clientstats.mediafile_video_bytes_recv += packet_size;
        if(!user.null())
            user->AddVideoFilePacket(video_pkt, *chan);
    }
    break;
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_DESKTOP_CRYPT :
    {
        CryptDesktopPacket crypt_pkt(packet_data, packet_size);
        DesktopPacket* decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        packet_ptr_t ptr(decrypt_pkt);
        MYTRACE_COND(user.null(),
                     ACE_TEXT("Received CryptDesktopPacket from unknown user #%d"),
                     packet.GetSrcUserID());

        m_clientstats.desktopbytes_recv += packet_size;
        if(!user.null())
            ReceivedDesktopPacket(*user, *chan, *decrypt_pkt);
    }
    break;
#endif
    case PACKET_KIND_DESKTOP :
    {
        DesktopPacket desktop_pkt(packet_data, packet_size);
        MYTRACE_COND(user.null(),
                     ACE_TEXT("Received DesktopPacket from unknown user #%d"),
                     packet.GetSrcUserID());

        m_clientstats.desktopbytes_recv += packet_size;
        if(!user.null())
            ReceivedDesktopPacket(*user, *chan, desktop_pkt);
    }
    break;
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_DESKTOP_ACK_CRYPT :
    {
        TTASSERT(chan.get());
        CryptDesktopAckPacket crypt_pkt(packet_data, packet_size);
        DesktopAckPacket* decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        packet_ptr_t ptr(decrypt_pkt);
        ReceivedDesktopAckPacket(*decrypt_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
#endif
    case PACKET_KIND_DESKTOP_ACK :
    {
        DesktopAckPacket ack_pkt(packet_data, packet_size);
        ReceivedDesktopAckPacket(ack_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_DESKTOP_NAK_CRYPT :
    {
        TTASSERT(chan.get());
        CryptDesktopNakPacket crypt_pkt(packet_data, packet_size);
        DesktopNakPacket* decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        packet_ptr_t ptr(decrypt_pkt);
        ReceivedDesktopNakPacket(*decrypt_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
#endif
    case PACKET_KIND_DESKTOP_NAK :
    {
        DesktopNakPacket nak_pkt(packet_data, packet_size);
        ReceivedDesktopNakPacket(nak_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_DESKTOPCURSOR_CRYPT :
    {
        TTASSERT(chan.get());
        CryptDesktopCursorPacket crypt_pkt(packet_data, packet_size);
        DesktopCursorPacket* decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        packet_ptr_t ptr(decrypt_pkt);
        ReceivedDesktopCursorPacket(*decrypt_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
#endif
    case PACKET_KIND_DESKTOPCURSOR :
    {
        DesktopCursorPacket csr_pkt(packet_data, packet_size);
        ReceivedDesktopCursorPacket(csr_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
        break;
    }
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_DESKTOPINPUT_CRYPT :
    {
        TTASSERT(chan.get());
        CryptDesktopInputPacket crypt_pkt(packet_data, packet_size);
        DesktopInputPacket* decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        packet_ptr_t ptr(decrypt_pkt);
        ReceivedDesktopInputPacket(*decrypt_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
#endif
    case PACKET_KIND_DESKTOPINPUT :
    {
        DesktopInputPacket csr_pkt(packet_data, packet_size);
        ReceivedDesktopInputPacket(csr_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
        break;
    }
#ifdef ENABLE_ENCRYPTION
    case PACKET_KIND_DESKTOPINPUT_ACK_CRYPT :
    {
        CryptDesktopInputAckPacket crypt_pkt(packet_data, packet_size);
        DesktopInputAckPacket* decrypt_pkt = crypt_pkt.Decrypt(chan->GetEncryptKey());
        if(!decrypt_pkt)
            return;
        packet_ptr_t ptr(decrypt_pkt);
        ReceivedDesktopInputAckPacket(*decrypt_pkt);
        m_clientstats.desktopbytes_recv += packet_size;
    }
    break;
#endif
    case PACKET_KIND_DESKTOPINPUT_ACK :
    {
        DesktopInputAckPacket ack_pkt(packet_data, packet_size);
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
                                        const ACE_INET_Addr& addr)
{
    ASSERT_REACTOR_THREAD(m_reactor);
    ASSERT_REACTOR_LOCKED(this);

    int userid = packet.GetSrcUserID();

    if( (m_flags & CLIENT_CONNECTING))//server ACK
    {
        m_flags &= ~CLIENT_CONNECTING;
        m_flags |= CLIENT_CONNECTED;

        m_clientstats.udpping_time = GETTIMESTAMP() - packet.GetTime();
        m_clientstats.udp_ping_dirty = false;

        m_serverinfo.packetprotocol = packet.GetProtocol();

        if(TimerExists(TIMER_UDPCONNECT_ID))
            StopTimer(TIMER_UDPCONNECT_ID);
        if(TimerExists(TIMER_UDPCONNECT_TIMEOUT_ID))
            StopTimer(TIMER_UDPCONNECT_TIMEOUT_ID);

        ACE_Time_Value tm(CLIENT_UDPKEEPALIVE_CHECK_INTERVAL);
        ACE_Time_Value tm_rtx(CLIENT_UDPKEEPALIVE_RTX_INTERVAL);

        StartTimer(TIMER_UDPKEEPALIVE_ID, 0, tm, tm_rtx);
        
        //notify parent application
        if(m_listener)
            m_listener->OnConnectSuccess();
    }
    else if( (m_flags & CLIENT_CONNECTED))//server ACK
    {
        if(TimerExists(TIMER_UDPCONNECT_ID))
            StopTimer(TIMER_UDPCONNECT_ID);
    }
}

void ClientNode::ReceivedKeepAliveReplyPacket(const KeepAlivePacket& packet,
                                              const ACE_INET_Addr& addr)
{
    ASSERT_REACTOR_THREAD(m_reactor);
    ASSERT_REACTOR_LOCKED(this);

    int userid = packet.GetSrcUserID();
    MYTRACE( ACE_TEXT("Received UDP keepalive reply from #%d %s\n"), 
        userid, InetAddrToString(addr).c_str());

    //reset server keep aliver timeout
    m_clientstats.udp_silence_sec = 0;
    m_clientstats.udpping_time = GETTIMESTAMP() - packet.GetTime();
    m_clientstats.udp_ping_dirty = false;

    //this is a reply to a MTU query
    mtu_packets_t::iterator ii = m_mtu_packets.find(packet.GetTime());
    if(ii != m_mtu_packets.end())
    {
        TTASSERT(ii->second->GetPayloadSize()>0);

        uint16_t payload_size = ii->second->GetPayloadSize();
        MYTRACE(ACE_TEXT("MTU query %u reported reply to %d payload\n"),
                ii->second->GetTime(), payload_size);
        StopTimer(TIMER_QUERY_MTU_ID);

        m_mtu_packets.clear();

        m_mtu_data_size = payload_size;
        m_mtu_max_payload_size = m_mtu_data_size + FIELDHEADER_PAYLOAD;

        if(payload_size < MTU_QUERY_SIZES[MTU_QUERY_SIZES_COUNT-1])
        {
            size_t i;
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
    uint8_t session_id;
    uint32_t time_ack;
    if(!ack_pkt.GetSessionInfo(0, &session_id, &time_ack))
        return;

    if(!m_desktop_tx.null() &&
        m_desktop_tx->GetSessionID() == session_id &&
        m_desktop_tx->GetUpdateID() == time_ack)
    {
        uint16_t packets_remain = m_desktop_tx->GetRemainingPacketsCount();
        m_desktop_tx->ProcessDesktopAckPacket(ack_pkt);

        //transmit new packets
        desktoppackets_t tx_packets;
        m_desktop_tx->GetDupAckLostDesktopPackets(tx_packets);
        m_desktop_tx->GetNextDesktopPackets(tx_packets);

        desktoppackets_t::iterator dpi = tx_packets.begin();
        while(dpi != tx_packets.end())
        {
            DesktopPacket* dp;
            ACE_NEW(dp, DesktopPacket(*(*dpi)));
            if(!QueuePacket(dp))
            {
                delete dp;
                break;
            }

            dpi++;
        }

        int new_remain_packets = m_desktop_tx->GetRemainingPacketsCount();
        int new_remain_bytes = m_desktop_tx->GetRemainingBytes();

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
        if(packets_remain != new_remain_packets)
            m_listener->OnDesktopTransferUpdate(session_id, new_remain_bytes);
    }
    else if(!m_desktop_nak_tx.null())
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
    ASSERT_REACTOR_THREAD(m_reactor);

    clientuser_t user = GetUser(nak_pkt.GetSrcUserID());
    MYTRACE_COND(user.null(), ACE_TEXT("Asked to delete desktop session #%d ")
                 ACE_TEXT("from user who doesn't exist\n"), nak_pkt.GetSessionID());
    if(user.null())
        return;

    desktop_viewer_t viewer = user->GetDesktopSession();
    MYTRACE_COND(viewer.null(), ACE_TEXT("Asked to delete desktop session #%d ")
                 ACE_TEXT("which doesn't exist from user #%d\n"), 
                 nak_pkt.GetSessionID(), nak_pkt.GetSrcUserID());

    if(viewer.null())
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
    clientuser_t src_user = GetUser(csr_pkt.GetSrcUserID());
    clientchannel_t chan = GetChannel(csr_pkt.GetChannel());
    uint16_t dest_userid;
    uint8_t session_id;
    if(!chan.null() && csr_pkt.GetSessionCursor(&dest_userid, &session_id, 0, 0))
    {
        if(dest_userid == 0)
        {
            if(!src_user.null())
                src_user->AddPacket(csr_pkt, *chan);
        }
        else
        {
            clientuser_t dest_user = GetUser(dest_userid);
            if(!dest_user.null())
                dest_user->AddPacket(csr_pkt, *chan);
        }
    }
}

void ClientNode::ReceivedDesktopInputPacket(const DesktopInputPacket& di_pkt)
{
    ASSERT_REACTOR_THREAD(m_reactor);

    clientuser_t src_user = GetUser(di_pkt.GetSrcUserID());
    clientchannel_t chan = GetChannel(di_pkt.GetChannel());
    if(chan.null())
        return;

    if(m_desktop.null() || m_desktop->GetSessionID() != di_pkt.GetSessionID())
        return;

    if(!src_user.null())
        src_user->AddPacket(di_pkt, *chan);

    if(di_pkt.GetDestUserID() == m_myuserid)
    {
        MYTRACE(ACE_TEXT("Received desktop input from #%d session: %d, pktno: %u\n"),
                di_pkt.GetSrcUserID(), di_pkt.GetSessionID(),
                (ACE_UINT32)di_pkt.GetPacketNo());
            
        int userid = src_user->GetUserID();
        if(!TimerExists(USER_TIMER_DESKTOPINPUT_ACK_ID, userid))
            StartUserTimer(USER_TIMER_DESKTOPINPUT_ACK_ID, userid, 0,
                            CLIENT_DESKTOPINPUT_ACK_DELAY,
                            CLIENT_DESKTOPINPUT_ACK_DELAY);
    }
}

void ClientNode::ReceivedDesktopInputAckPacket(const DesktopInputAckPacket& ack_pkt)
{
    ASSERT_REACTOR_THREAD(m_reactor);

    int userid = ack_pkt.GetSrcUserID();
    clientuser_t user = GetUser(userid);
    if(user.null())
        return;

    if(ack_pkt.GetDestUserID() != m_myuserid)
        return;

    uint8_t session_id;
    uint8_t packetno;
    if(!ack_pkt.GetSessionInfo(&session_id, &packetno))
        return;

    MYTRACE(ACE_TEXT("Received desktop input ACK for user #%d, session %d, pkt %u. Remain: %u\n"),
            ack_pkt.GetSrcUserID(), (int)session_id, (ACE_UINT32)packetno,
            (ACE_UINT32)user->GetDesktopInputRtxQueue().size());

    while(user->GetDesktopInputRtxQueue().size() &&
          user->GetDesktopInputRtxQueue().front()->GetSessionID() == session_id &&
          W8_LEQ(user->GetDesktopInputRtxQueue().front()->GetPacketNo(), packetno))
          user->GetDesktopInputRtxQueue().pop_front();

    //queue more for transmission (and place in rtx queue)
    while(user->GetDesktopInputRtxQueue().size() < DESKTOPINPUT_MAX_RTX_PACKETS &&
          user->GetDesktopInputTxQueue().size())
    {
        desktopinput_pkt_t tx_pkt(user->GetDesktopInputTxQueue().front());
        DesktopInputPacket* packet;
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
    ASSERT_REACTOR_THREAD(m_reactor);

    GUARD_REACTOR(this);

    int ret;
    FieldPacket* p;
    while( (p = m_tx_queue.GetNextPacket()) )
    {
        packet_ptr_t p_ptr(p); //auto delete

#if SIMULATE_TX_PACKETLOSS
        static int dropped = 0, transmitted = 0;
        transmitted++;
        if((ACE_OS::rand() % SIMULATE_TX_PACKETLOSS) == 0)
        {
            dropped++;
            MYTRACE(ACE_TEXT("Dropped TX packet kind %d, dropped %d/%d\n"), 
                (int)p->GetKind(), dropped, transmitted);
            continue;
        }
#endif

        switch(p->GetKind())
        {
        case PACKET_KIND_VOICE :
        {
            VoicePacket* audpkt = dynamic_cast<VoicePacket*>(p);
            TTASSERT(audpkt);
            TTASSERT(!audpkt->HasFragments());
            TTASSERT(!audpkt->Finalized());
            //a packet which isn't finalized will be transmitting to 'm_mychannel'
            if(!audpkt->Finalized())
            {
                //only transmit if we're in a channel, transmitting,
                //allowed to transmit
                if(m_mychannel.null())
                    break;
                if(!m_mychannel->CanTransmit(m_myuserid, STREAMTYPE_VOICE))
                    break;
                
                if((m_mychannel->GetChannelType() & CHANNEL_NO_VOICEACTIVATION) &&
                   (GetFlags() & CLIENT_SNDINPUT_VOICEACTIVATED) &&
                   (GetFlags() & CLIENT_SNDINPUT_VOICEACTIVE))
                    break;

                audpkt->SetChannel(m_mychannel->GetChannelID());
            }

            bool no_record = (m_mychannel->GetChannelType() & CHANNEL_NO_RECORDING) &&
                (GetMyUserAccount().userrights & USERRIGHT_RECORD_VOICE) == USERRIGHT_NONE;
            //store in voicelog
            if(m_local_voicelog->GetAudioFolder().length() && !no_record)
                voicelogger().AddVoicePacket(*m_local_voicelog, *m_mychannel, *audpkt);

            //if packet is too big we turn it into fragments (packet protocol 2)
            audiopackets_t fragments = BuildAudioFragments(*audpkt, 
                                                           m_mtu_data_size);
            if(fragments.size())
            {
                for(size_t i=0;i<fragments.size();i++)
                    SendVoicePacket(*fragments[i]);
            }
            else
                SendVoicePacket(*audpkt);
        }
        break;
        case PACKET_KIND_MEDIAFILE_AUDIO :
        {
            AudioFilePacket* audpkt = dynamic_cast<AudioFilePacket*>(p);
            TTASSERT(audpkt);
            TTASSERT(!audpkt->HasFragments());

            //only transmit if we're in a channel, transmitting,
            //allowed to transmit and there's active users
            if(m_mychannel.null() || 
               !m_mychannel->CanTransmit(m_myuserid, STREAMTYPE_MEDIAFILE))
                break;

            TTASSERT(audpkt->GetStreamID() == m_mediafile_stream_id);
            audpkt->SetChannel(m_mychannel->GetChannelID());

            //if packet is too big we turn it into fragments (packet protocol 2)
            audiopackets_t fragments = BuildAudioFragments(*audpkt, 
                                                           m_mtu_data_size);
            if(fragments.size())
            {
                for(size_t i=0;i<fragments.size();i++)
                    SendAudioFilePacket(*fragments[i]);
            }
            else
                SendAudioFilePacket(*audpkt);
        }
        break;
        case PACKET_KIND_VIDEO :
        {
            VideoCapturePacket* vidpkt = dynamic_cast<VideoCapturePacket*>(p);
            TTASSERT(vidpkt);

            if(!vidpkt->Finalized())
            {
                //only transmit if we're in a channel, transmitting and
                //there's active users
                if(m_mychannel.null() || 
                   (m_flags & CLIENT_TX_VIDEOCAPTURE) == 0 ||
                   !m_mychannel->CanTransmit(m_myuserid, STREAMTYPE_VIDEOCAPTURE))
                    break;

                TTASSERT(vidpkt->GetStreamID() == m_vidcap_stream_id);
                vidpkt->SetChannel(m_mychannel->GetChannelID());
            }

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream)
            {
                clientchannel_t chan = GetChannel(vidpkt->GetChannel());
                if(chan.null())
                    break;
                CryptVideoCapturePacket crypt_pkt(*vidpkt, chan->GetEncryptKey());
                if(m_myuseraccount.userrights & USERRIGHT_TRANSMIT_VIDEOCAPTURE)
                    ret = SendPacket(crypt_pkt, m_serverinfo.udpaddr);
                TTASSERT(crypt_pkt.ValidatePacket());
            }
            else
#endif
            {
                TTASSERT(m_def_stream);
                if(m_myuseraccount.userrights & USERRIGHT_TRANSMIT_VIDEOCAPTURE)
                    ret = SendPacket(*vidpkt, m_serverinfo.udpaddr);
                TTASSERT(vidpkt->ValidatePacket());
            }
        }
        break;
        case PACKET_KIND_MEDIAFILE_VIDEO :
        {
            VideoFilePacket* vidpkt = dynamic_cast<VideoFilePacket*>(p);
            TTASSERT(vidpkt);

            if(!vidpkt->Finalized())
            {
                //only transmit if we're in a channel, transmitting and
                //there's active users
                if(m_mychannel.null() || 
                   (m_flags & CLIENT_STREAM_VIDEOFILE) == 0 ||
                   !m_mychannel->CanTransmit(m_myuserid, STREAMTYPE_MEDIAFILE))
                    break;

                TTASSERT(vidpkt->GetStreamID());
                vidpkt->SetChannel(m_mychannel->GetChannelID());
            }

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream)
            {
                clientchannel_t chan = GetChannel(vidpkt->GetChannel());
                if(chan.null())
                    break;
                CryptVideoFilePacket crypt_pkt(*vidpkt, chan->GetEncryptKey());
                if(m_myuseraccount.userrights & USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO)
                    ret = SendPacket(crypt_pkt, m_serverinfo.udpaddr);
                TTASSERT(crypt_pkt.ValidatePacket());
            }
            else
#endif
            {
                TTASSERT(m_def_stream);
                if(m_myuseraccount.userrights & USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO)
                {
                    ret = SendPacket(*vidpkt, m_serverinfo.udpaddr);
                }
                TTASSERT(vidpkt->ValidatePacket());
            }
        }
        break;
        case PACKET_KIND_DESKTOP :
        {
            DesktopPacket* desktoppkt = dynamic_cast<DesktopPacket*>(p);
            TTASSERT(desktoppkt);
            TTASSERT(!desktoppkt->Finalized());

            if(m_mychannel.null() ||
               !m_mychannel->CanTransmit(m_myuserid, STREAMTYPE_DESKTOP))
                break;

            //desktop update hasn't completed (non-null)
            if(m_desktop_tx.null())
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
            if(m_crypt_stream)
            {
                CryptDesktopPacket crypt_pkt(*desktoppkt, m_mychannel->GetEncryptKey());
                ret = 0;
                if(m_myuseraccount.userrights & USERRIGHT_TRANSMIT_DESKTOP)
                    ret = SendPacket(crypt_pkt, m_serverinfo.udpaddr);
                TTASSERT(crypt_pkt.ValidatePacket());
            }
            else
#endif
            {
                TTASSERT(m_def_stream);
                ret = 0;
                if(m_myuseraccount.userrights & USERRIGHT_TRANSMIT_DESKTOP)
                    ret = SendPacket(*desktoppkt, m_serverinfo.udpaddr);
                TTASSERT(desktoppkt->ValidatePacket());
            }
            //MYTRACE(ACE_TEXT("Sent desktop packet %d size %d, %u\n"), 
            //    desktoppkt->GetPacketIndex(), ret, desktoppkt->GetTime());
        }
        break;
        case PACKET_KIND_DESKTOP_ACK :
        {
            DesktopAckPacket* ack_packet = dynamic_cast<DesktopAckPacket*>(p);
            TTASSERT(ack_packet);
            TTASSERT(ack_packet->Finalized());

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream)
            {
                clientchannel_t chan = GetChannel(ack_packet->GetChannel());
                if(chan.null())
                    break;
                CryptDesktopAckPacket crypt_pkt(*ack_packet, chan->GetEncryptKey());
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
            DesktopNakPacket* nak_packet = dynamic_cast<DesktopNakPacket*>(p);
            TTASSERT(nak_packet);
            TTASSERT(nak_packet->Finalized());

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream)
            {
                clientchannel_t chan = GetChannel(nak_packet->GetChannel());
                if(chan.null())
                    break;
                CryptDesktopNakPacket crypt_pkt(*nak_packet, chan->GetEncryptKey());
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
            DesktopCursorPacket* cursor_pkt = dynamic_cast<DesktopCursorPacket*>(p);
            TTASSERT(cursor_pkt);
            TTASSERT(cursor_pkt->Finalized());

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream)
            {
                clientchannel_t chan = GetChannel(cursor_pkt->GetChannel());
                if(chan.null())
                    break;
                CryptDesktopCursorPacket crypt_pkt(*cursor_pkt, chan->GetEncryptKey());
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
            DesktopInputPacket* cursor_pkt = dynamic_cast<DesktopInputPacket*>(p);
            TTASSERT(cursor_pkt);
            TTASSERT(cursor_pkt->Finalized());

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream)
            {
                clientchannel_t chan = GetChannel(cursor_pkt->GetChannel());
                if(chan.null())
                    break;
                CryptDesktopInputPacket crypt_pkt(*cursor_pkt, chan->GetEncryptKey());
                ret = SendPacket(crypt_pkt, m_serverinfo.udpaddr);
                TTASSERT(crypt_pkt.ValidatePacket());
            }
            else
#endif
            {
                ret = SendPacket(*cursor_pkt, m_serverinfo.udpaddr);
                TTASSERT(cursor_pkt->ValidatePacket());
            }

            //MYTRACE(ACE_TEXT("Sent desktop input packet, %d:%u pkt no: %d\n"),
            //        cursor_pkt->GetSessionID(), cursor_pkt->GetTime(),
            //        cursor_pkt->GetPacketNo());
        }
        break;
        case PACKET_KIND_DESKTOPINPUT_ACK :
        {
            DesktopInputAckPacket* ack_pkt = dynamic_cast<DesktopInputAckPacket*>(p);
            TTASSERT(ack_pkt);
            TTASSERT(ack_pkt->Finalized());

#ifdef ENABLE_ENCRYPTION
            if(m_crypt_stream)
            {
                clientchannel_t chan = GetChannel(ack_pkt->GetChannel());
                if(chan.null())
                    break;
                CryptDesktopInputAckPacket crypt_pkt(*ack_pkt, chan->GetEncryptKey());
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
    bool b = m_tx_queue.QueuePacket(packet) >= 0;

    MYTRACE_COND(!b, ACE_TEXT("Failed to queue packet kind %d to channel %d\n"),
                 (int)packet->GetKind(), (int)packet->GetChannel());

    ACE_Time_Value tv;
    int ret = m_reactor.notify(&m_packethandler, 
                               ACE_Event_Handler::WRITE_MASK, &tv);
    TTASSERT(ret>=0);
    return b;
}

//Send packet to address
int ClientNode::SendPacket(const FieldPacket& packet, const ACE_INET_Addr& addr)
{
    ASSERT_REACTOR_LOCKED(this);

    assert(packet.GetPacketSize() <= MAX_PACKET_SIZE);

#ifdef _DEBUG
    TTASSERT(packet.ValidatePacket());
#endif

    //normal send without encryption
    int buffers;
    const iovec* vv = packet.GetPacket(buffers);
    ssize_t ret = m_packethandler.sock_i().send(vv, buffers, addr);
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
    ASSERT_REACTOR_LOCKED(this);

    if(m_flags & CLIENT_SNDINPUT_READY)
        return false;

    if(!SOUNDSYSTEM->CheckInputDevice(inputdevice))
        return false;

    rguard_t g_snd(lock_sndprop());
    TTASSERT(m_soundprop.inputdeviceid == SOUNDDEVICE_IGNORE_ID);
    m_soundprop.inputdeviceid = inputdevice;
    g_snd.release();

    if(!m_mychannel.null())
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
    ASSERT_REACTOR_LOCKED(this);

    if(m_flags & CLIENT_SNDOUTPUT_READY)
        return false;
    if(!SOUNDSYSTEM->CheckOutputDevice(outputdevice))
        return false;

    rguard_t g_snd(lock_sndprop());

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
    ASSERT_REACTOR_LOCKED(this);

    if((m_flags & CLIENT_SNDINPUT_READY) ||
       (m_flags & CLIENT_SNDOUTPUT_READY))
        return false; //already enabled
    if(!SOUNDSYSTEM->CheckInputDevice(inputdeviceid))
        return false;
    if(!SOUNDSYSTEM->CheckOutputDevice(outputdeviceid))
        return false;

    rguard_t g_snd(lock_sndprop());

    TTASSERT(m_soundprop.inputdeviceid == SOUNDDEVICE_IGNORE_ID);
    TTASSERT(m_soundprop.outputdeviceid == SOUNDDEVICE_IGNORE_ID);
    m_soundprop.inputdeviceid = inputdeviceid;
    m_soundprop.outputdeviceid = outputdeviceid;
    g_snd.release();

    m_flags |= CLIENT_SNDINPUT_READY;
    m_flags |= CLIENT_SNDOUTPUT_READY;
    m_flags |= CLIENT_SNDINOUTPUT_DUPLEX;

    //restart audio capture in duplex mode
    if(!m_mychannel.null())
        OpenAudioCapture(m_mychannel->GetAudioCodec());

    return true;
}

bool ClientNode::CloseSoundInputDevice()
{
    ASSERT_REACTOR_LOCKED(this);

    if(m_flags & CLIENT_SNDINOUTPUT_DUPLEX)
        return false;

    CloseAudioCapture();

    rguard_t g_snd(lock_sndprop());
    m_soundprop.inputdeviceid = SOUNDDEVICE_IGNORE_ID;
    g_snd.release();

    m_flags &= ~CLIENT_SNDINPUT_READY;
    return true;
}

bool ClientNode::CloseSoundOutputDevice()
{
    ASSERT_REACTOR_LOCKED(this);

    if(m_flags & CLIENT_SNDINOUTPUT_DUPLEX)
        return false;

    rguard_t g_snd(lock_sndprop());
    m_soundprop.outputdeviceid = SOUNDDEVICE_IGNORE_ID;
    g_snd.release();

    m_flags &= ~CLIENT_SNDOUTPUT_READY;

    ResetAudioPlayers();

    return true;
}

bool ClientNode::CloseSoundDuplexDevices()
{
    ASSERT_REACTOR_LOCKED(this);

    if((m_flags & CLIENT_SNDINOUTPUT_DUPLEX) == 0)
        return false; //already enabled

    ResetAudioPlayers();

    //shutdown cature
    CloseAudioCapture();

    rguard_t g_snd(lock_sndprop());
    m_soundprop.inputdeviceid = SOUNDDEVICE_IGNORE_ID;
    m_soundprop.outputdeviceid = SOUNDDEVICE_IGNORE_ID;
    g_snd.release();

    m_flags &= ~CLIENT_SNDINPUT_READY;
    m_flags &= ~CLIENT_SNDOUTPUT_READY;
    m_flags &= ~CLIENT_SNDINOUTPUT_DUPLEX;

    return true;
}

bool ClientNode::SetSoundOutputVolume(int volume)
{
    rguard_t g_snd(lock_sndprop());
    return SOUNDSYSTEM->SetMasterVolume(m_soundprop.soundgroupid, volume);
}
int ClientNode::GetSoundOutputVolume()
{
    rguard_t g_snd(lock_sndprop());
    return SOUNDSYSTEM->GetMasterVolume(m_soundprop.soundgroupid);
}

void ClientNode::EnableVoiceTransmission(bool enable)
{
    ASSERT_REACTOR_LOCKED(this);

    if(enable)
    {
        m_flags |= CLIENT_TX_VOICE;

        //don't increment stream id if voice activated and voice active
        if((m_flags & CLIENT_SNDINPUT_VOICEACTIVATED) == CLIENT_CLOSED ||
           ((m_flags & CLIENT_SNDINPUT_VOICEACTIVATED) &&
            (m_flags & CLIENT_SNDINPUT_VOICEACTIVE)))
            GEN_NEXT_ID(m_voice_stream_id);
    }
    else
        m_flags &= ~CLIENT_TX_VOICE;
}

int ClientNode::GetCurrentVoiceLevel()
{
    return m_voice_thread.m_voicelevel;
}
void ClientNode::EnableVoiceActivation(bool enable)
{
    ASSERT_REACTOR_LOCKED(this);

    if(enable)
        m_flags |= CLIENT_SNDINPUT_VOICEACTIVATED;
    else
    {
        m_flags &= ~CLIENT_SNDINPUT_VOICEACTIVATED;
        m_flags &= ~CLIENT_SNDINPUT_VOICEACTIVE;
    }
}
void ClientNode::SetVoiceActivationLevel(int voicelevel)
{
    m_voice_thread.m_voiceactlevel = voicelevel;
}
int ClientNode::GetVoiceActivationLevel()
{
    return m_voice_thread.m_voiceactlevel;
}
void ClientNode::SetVoiceActivationStoppedDelay(int msec)
{
    m_voice_thread.m_voiceact_delay = ACE_Time_Value(msec / 1000, 
                                                     (msec % 1000) * 1000);
}
int ClientNode::GetVoiceActivationStoppedDelay()
{
    return (int)m_voice_thread.m_voiceact_delay.msec();
}

bool ClientNode::EnableAutoPositioning(bool enable)
{
    ASSERT_REACTOR_LOCKED(this);

    if(m_flags & CLIENT_SNDINOUTPUT_DUPLEX)
        return false;

    if(enable)
        m_flags |= CLIENT_SNDOUTPUT_AUTO3DPOSITION;
    else
        m_flags &= ~CLIENT_SNDOUTPUT_AUTO3DPOSITION;
    return SOUNDSYSTEM->SetAutoPositioning(m_soundprop.soundgroupid, enable);
}
bool ClientNode::AutoPositionUsers()
{
    ASSERT_REACTOR_LOCKED(this);

    if(m_flags & CLIENT_SNDINOUTPUT_DUPLEX)
        return false;
    return SOUNDSYSTEM->AutoPositionPlayers(m_soundprop.soundgroupid, true);
}
void ClientNode::EnableAudioBlockCallback(int userid, StreamType stream_type,
                                          bool enable)
{
    ASSERT_REACTOR_LOCKED(this);

    if(enable)
        AUDIOCONTAINER::instance()->AddSoundSource(m_soundprop.soundgroupid,
                                                   userid, stream_type);
    else
        AUDIOCONTAINER::instance()->RemoveSoundSource(m_soundprop.soundgroupid,
                                                      userid, stream_type);
}
bool ClientNode::MuteAll(bool muteall)
{
    ASSERT_REACTOR_LOCKED(this);

    if(muteall)
        m_flags |= CLIENT_SNDOUTPUT_MUTE;
    else
        m_flags &= ~CLIENT_SNDOUTPUT_MUTE;

    rguard_t g_snd(lock_sndprop());
    return SOUNDSYSTEM->MuteAll(m_soundprop.soundgroupid, muteall);
}

void ClientNode::SetVoiceGainLevel(int gainlevel)
{
    rguard_t g_snd(lock_sndprop());

    m_soundprop.gainlevel = gainlevel; //cache value
    m_voice_thread.m_gainlevel = gainlevel;
}

int ClientNode::GetVoiceGainLevel()
{
    rguard_t g_snd(lock_sndprop());

    return m_soundprop.gainlevel;
}

void ClientNode::SetSoundPreprocess(const SpeexDSP& speexdsp)
{
    ASSERT_REACTOR_LOCKED(this);
    rguard_t g_snd(lock_sndprop());

    m_soundprop.speexdsp = speexdsp;

    UpdateSoundInputPreprocess();
}

void ClientNode::SetSoundInputTone(StreamTypes streams, int frequency)
{
    if(streams & STREAMTYPE_MEDIAFILE_AUDIO)
        m_audiofile_thread.EnableTone(frequency);
    if(streams & STREAMTYPE_VOICE)
        m_voice_thread.EnableTone(frequency);
}

bool ClientNode::StartRecordingMuxedAudioFile(const AudioCodec& codec, 
                                              const ACE_TString& filename,
                                              AudioFileFormat aff)
{
    ASSERT_REACTOR_LOCKED(this);

    if(audiomuxer().StartThread(filename, aff, codec))
    {
        m_flags |= CLIENT_MUX_AUDIOFILE;
        return true;
    }
    return false;
}

void ClientNode::StopRecordingMuxedAudioFile()
{
    ASSERT_REACTOR_LOCKED(this);

    audiomuxer().StopThread();
    m_flags &= ~CLIENT_MUX_AUDIOFILE;
}

bool ClientNode::StartMTUQuery()
{
    ASSERT_REACTOR_LOCKED(this);

    if(TimerExists(TIMER_QUERY_MTU_ID))
        return false;

    m_mtu_packets.clear();

    return StartTimer(TIMER_QUERY_MTU_ID, 0, ACE_Time_Value::zero,
                      CLIENT_QUERY_MTU_INTERVAL)>=0;
}

bool ClientNode::StartStreamingMediaFile(const ACE_TString& filename,
                                         const VideoCodec& vid_codec)
{
    ASSERT_REACTOR_LOCKED(this);

    //don't allow video streaming if not in channel or already streaming
    if(m_mychannel.null() ||
       (m_flags & CLIENT_STREAM_VIDEOFILE) ||
       (m_flags & CLIENT_STREAM_AUDIOFILE))
        return false;

    MediaStreamOutput media_out;
#if defined(ENABLE_DSHOW)
    media_out.video.fourcc = media::FOURCC_RGB32;
#else
    media_out.video.fourcc = media::FOURCC_I420;
#endif
    media_out.audio.channels = GetAudioCodecChannels(m_mychannel->GetAudioCodec());
    media_out.audio.samplerate = GetAudioCodecSampleRate(m_mychannel->GetAudioCodec());
    media_out.audio_samples = GetAudioCodecCbSamples(m_mychannel->GetAudioCodec());

    TTASSERT(m_media_streamer.null());
    if(m_media_streamer.null())
        m_media_streamer = MakeMediaStreamer(this);

    MediaFileProp file_in(filename);
    if(m_media_streamer.null() ||
       !m_media_streamer->OpenFile(file_in, media_out))
    {
        StopStreamingMediaFile();
        return false;
    }

    file_in = m_media_streamer->GetMediaInput();

    //initiate audio part of media file
    if(file_in.audio.IsValid())
    {
        if(!m_audiofile_thread.StartEncoder(std::bind(&ClientNode::EncodedAudioFileFrame, this,
                                                      _1, _2, _3, _4, _5),
                                            m_mychannel->GetAudioCodec(), true))
        {
            StopStreamingMediaFile();
            return false;
        }
        m_flags |= CLIENT_STREAM_AUDIOFILE;
    }

    TTASSERT(m_videofile_thread.null());
    //initiate video part of media file
    if(file_in.video.IsValid() && m_videofile_thread.null())
    {
        m_flags |= CLIENT_STREAM_VIDEOFILE;

        VideoThread* vid_thread;
        ACE_NEW_NORETURN(vid_thread, VideoThread());
        m_videofile_thread = video_thread_t(vid_thread);
    
        if(!vid_thread ||
           !vid_thread->StartEncoder(std::bind(&ClientNode::EncodedVideoFileFrame, this, _1, _2, _3, _4, _5),
                                     m_media_streamer->GetMediaOutput().video, vid_codec, VIDEOFILE_ENCODER_FRAMES_MAX))
        {
            StopStreamingMediaFile();
            return false;
        }

    }
    
    // give up if input file has no video or audio
    if(file_in.audio.IsValid() == false && file_in.video.IsValid() == false)
    {
        StopStreamingMediaFile();
        return false;
    }

    //both audio and video part of mediafile use same stream id
    GEN_NEXT_ID(m_mediafile_stream_id);

    bool b = m_media_streamer->StartStream();
    if(!b)
    {
        StopStreamingMediaFile();
        return false;
    }

    return true;
}

void ClientNode::StopStreamingMediaFile()
{
    ASSERT_REACTOR_LOCKED(this);

    bool clear_video = false, clear_audio = false;

    if(!m_media_streamer.null())
    {
        clear_video = m_media_streamer->GetMediaOutput().HasVideo();
        clear_audio = m_media_streamer->GetMediaOutput().HasAudio();
        m_media_streamer->Close();
        m_media_streamer.reset();
    }

    if(clear_video)
    {
        if(!m_videofile_thread.null())
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

bool ClientNode::InitVideoCapture(const ACE_TString& src_id,
                                  const media::VideoFormat& cap_format)
{
    ASSERT_REACTOR_LOCKED(this);

    if(m_flags & CLIENT_VIDEOCAPTURE_READY)
        return false;

    assert(!m_vidcap);
    if (m_vidcap)
        return false;

    videocapture_t session = vidcap::VideoCapture::Create();

    if (!session->InitVideoCapture(src_id, cap_format))
        return false;
        
    if (session->RegisterVideoFormat(std::bind(&ClientNode::VideoCaptureEncodeCallback, this, _1, _2), media::FOURCC_I420))
    {
        MYTRACE(ACE_TEXT("Video capture sends I420 directly to encoder\n"));
        if (session->RegisterVideoFormat(std::bind(&ClientNode::VideoCaptureRGB32Callback, this, _1, _2), media::FOURCC_RGB32))
        {
            MYTRACE(ACE_TEXT("Video capture sends RGB32 directly to 'm_local_vidcapframes'\n"));
        }
        else
        {
            MYTRACE(ACE_TEXT("Video capture cannot send RGB32', i.e. 'm_local_vidcapframes' is ignored\n"));
        }
    }
    else if (session->RegisterVideoFormat(std::bind(&ClientNode::VideoCaptureDualCallback, this, _1, _2), media::FOURCC_RGB32))
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
    ASSERT_REACTOR_LOCKED(this);
    m_vidcap.reset();

    CloseVideoCaptureSession();

    m_flags &= ~CLIENT_VIDEOCAPTURE_READY;
}

bool ClientNode::OpenVideoCaptureSession(const VideoCodec& codec)
{
    ASSERT_REACTOR_LOCKED(this);

    if(m_flags & CLIENT_TX_VIDEOCAPTURE)
        return false;

    if (!m_vidcap)
        return false;

    VideoFormat cap_format = m_vidcap->GetVideoCaptureFormat();
    if(!cap_format.IsValid())
        return false;

    m_vidcap_thread.StopEncoder();

    if(!m_vidcap_thread.StartEncoder(std::bind(&ClientNode::EncodedVideoCaptureFrame, this, _1, _2, _3, _4, _5),
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
    ASSERT_REACTOR_LOCKED(this);

    m_vidcap_thread.StopEncoder();
    m_local_vidcapframes.close();

    m_flags &= ~CLIENT_TX_VIDEOCAPTURE;
}

ACE_Message_Block* ClientNode::AcquireVideoCaptureFrame()
{
    ASSERT_REACTOR_LOCKED(this);

    ACE_Message_Block* mb = NULL;
    ACE_Time_Value tm_zero;
    m_local_vidcapframes.dequeue(mb, &tm_zero);
    return mb;
}

bool ClientNode::EncodedVideoCaptureFrame(ACE_Message_Block* org_frame,
                                          const char* enc_data, int enc_len,
                                          ACE_UINT32 packet_no,
                                          ACE_UINT32 timestamp)
{
    if(enc_data && (m_flags & CLIENT_AUTHORIZED) &&
       (m_flags & CLIENT_TX_VIDEOCAPTURE))
    {
        uint16_t w = (uint16_t)m_vidcap_thread.GetVideoFormat().width;
        uint16_t h = (uint16_t)m_vidcap_thread.GetVideoFormat().height;
        //max supported is uint16 * MAX_PAYLOAD_SIZE
        videopackets_t packets = BuildVideoPackets(PACKET_KIND_VIDEO,
                                                   m_myuserid, timestamp,
                                                   m_mtu_data_size,
                                                   m_vidcap_stream_id, 
                                                   packet_no, &w, &h,
                                                   enc_data, enc_len);

        bool failed = false;
        for(size_t i=0;i<packets.size();i++)
        {
            if(failed || !QueuePacket(packets[i]))
            {
                delete packets[i];
                failed = true;
            }
        }
    }
    //MYTRACE(ACE_TEXT("Local video frame queue: %d\n"), m_local_vidcapframes.message_count());

    ACE_Time_Value tm_zero;
    if(org_frame)
    {
        VideoFrame vid_frm(org_frame);
        vid_frm.stream_id = m_vidcap_stream_id;

        if (vid_frm.fourcc == media::FOURCC_RGB32)
            return VideoCaptureRGB32Callback(vid_frm, org_frame);
    }

    return false; //ignored 'org_frame'
}

bool ClientNode::EncodedVideoFileFrame(ACE_Message_Block* org_frame,
                                       const char* enc_data, int enc_len,
                                       ACE_UINT32 packet_no,
                                       ACE_UINT32 timestamp)
{
    //max supported is uint16 * MAX_PAYLOAD_SIZE
    uint16_t w = (uint16_t)m_videofile_thread->GetVideoFormat().width;
    uint16_t h = (uint16_t)m_videofile_thread->GetVideoFormat().height;
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
    for(size_t i=0;i<packets.size();i++)
    {
        if(failed || !QueuePacket(packets[i]))
        {
            delete packets[i];
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
    ASSERT_REACTOR_LOCKED(this);

    if(m_mychannel.null() ||
       (m_myuseraccount.userrights & USERRIGHT_TRANSMIT_DESKTOP) == 0)
        return -1;

    if(!m_mychannel->CanTransmit(m_myuserid, STREAMTYPE_DESKTOP))
        return -1;

    //start new session or update existing
    if(m_desktop.null() || m_desktop->GetWidth() != width ||
       m_desktop->GetHeight() != height || m_desktop->GetRGBMode() != rgb)
    {
        CloseDesktopSession(true);

        GEN_NEXT_ID(m_desktop_session_id);

        DesktopWindow new_wnd(m_desktop_session_id, width, height, rgb, 
                              DESKTOPPROTOCOL_ZLIB_1);

        DesktopInitiator* desktop;
        ACE_NEW_RETURN(desktop, DesktopInitiator(GetUserID(), new_wnd,
                                                 m_mtu_data_size,
                                                 m_mtu_max_payload_size),
                                                 false);
        m_desktop = desktop_initiator_t(desktop);

        m_flags |= CLIENT_DESKTOP_ACTIVE;
    }

    //ensure all data has already been sent before starting a new transmission
    if(!m_desktop_tx.null() && !m_desktop_tx->Done() &&
        m_desktop_tx->GetSessionID() == m_desktop->GetSessionID())
    {
        MYTRACE(ACE_TEXT("Ignored desktop update. Transmission already ongoing\n"));
        return -1;
    }

    if(m_desktop->thr_count() || m_desktop->HasDesktopPackets())
    {
        MYTRACE(ACE_TEXT("Ignored desktop updated. Thread active: %d, Desktop packets: %d\n"),
                (int)m_desktop->thr_count(), (int)m_desktop->HasDesktopPackets());
        return -1;
    }

    //ensure we don't get the same timestamp for next desktop update
    if(!m_desktop_tx.null())
        DUP_TIMESTAMP_DELAY(m_desktop_tx->GetUpdateID());

    uint32_t tm = GETTIMESTAMP();

    //process new bitmap
    int ret = m_desktop->NewBitmap(bmp_buf, bmp_buf_len, tm);

    if(ret > 0)
    {
        DesktopTransmitter* dtx;
        ACE_NEW_NORETURN(dtx, DesktopTransmitter(m_desktop_session_id, tm));
        if(!dtx)
        {
            CloseDesktopSession(false);
            return -1;
        }
        m_desktop_tx = desktop_transmitter_t(dtx);

        //TODO: find a better way to query for 'complete' instead of busy wait
        ACE_Time_Value tm(0, 1000);
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
    ASSERT_REACTOR_LOCKED(this);

    uint8_t session_id = 0;
    
    if(!m_desktop_tx.null())
        session_id = m_desktop_tx->GetSessionID();
    else if(!m_desktop.null())
        session_id = m_desktop->GetSessionID();

    if(session_id)
    {
        DesktopNakTransmitter* nak_tx;
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
    ASSERT_REACTOR_LOCKED(this);

    if(TimerExists(TIMER_BUILD_DESKTOPPACKETS_ID))
        StopTimer(TIMER_BUILD_DESKTOPPACKETS_ID);
    if(TimerExists(TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID))
        StopTimer(TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID);

    if(stop_nak_timer && TimerExists(TIMER_DESKTOPNAKPACKET_TIMEOUT_ID))
    {
        StopTimer(TIMER_DESKTOPNAKPACKET_TIMEOUT_ID);
        m_desktop_nak_tx.reset();
    }

    if(!m_desktop.null())
    {
        m_desktop->Abort();
        m_desktop->wait();
        m_desktop.reset();
    }
    m_desktop_tx.reset();

    //clear all desktop input for his session
    musers_t::iterator ii = m_users.begin();
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
    ASSERT_REACTOR_LOCKED(this);

    if(!m_rootchannel.null())
    {
        ClientChannel::users_t users;
        m_rootchannel->GetUsers(users, true);
        for(size_t i=0;i<users.size();i++)
            users[i]->ResetAudioPlayers(true);
    }
}

bool ClientNode::SendDesktopCursor(int x, int y)
{
    ASSERT_REACTOR_LOCKED(this);

    clientchannel_t chan;
    DesktopCursorPacket* pkt;

    chan = GetMyChannel();
    if(chan.null())
        return false;
    if(m_desktop.null())
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
    ASSERT_REACTOR_LOCKED(this);
    clientchannel_t chan;
    clientuser_t user;

    user = GetUser(userid);
    if(user.null())
        return false;

    int n_tx_queue = int(user->GetDesktopInputTxQueue().size() + user->GetDesktopInputRtxQueue().size());
    TTASSERT(n_tx_queue <= DESKTOPINPUT_QUEUE_MAX_SIZE);

    TTASSERT(n_tx_queue == 0 ||
            TimerExists(USER_TIMER_DESKTOPINPUT_RTX_ID, userid));

    if(n_tx_queue == DESKTOPINPUT_QUEUE_MAX_SIZE ||
       (m_myuseraccount.userrights & USERRIGHT_TRANSMIT_DESKTOPINPUT) == USERRIGHT_NONE)
       return false;

    chan = user->GetChannel();
    if(chan.null())
        return false;

    desktop_viewer_t viewer = user->GetDesktopSession();
    if(viewer.null())
        return false;

    DesktopInputPacket* pkt;
    ACE_NEW_RETURN(pkt, DesktopInputPacket(GetUserID(),
                                           GETTIMESTAMP(),
                                           viewer->GetSessionID(),
                                           user->NextDesktopInputTxPacket(),
                                           inputs), false);
    pkt->SetDestUser(userid);
    pkt->SetChannel(chan->GetChannelID());

    //store packet for retransmission
    DesktopInputPacket* rtx_pkt;
    ACE_NEW_NORETURN(rtx_pkt,
                     DesktopInputPacket(*pkt));
    if(!rtx_pkt)
    {
        delete pkt;
        return false;
    }

    //MYTRACE(ACE_TEXT("Queueing packet no %d with %u keys\n"),
    //    pkt->GetPacketNo(), inputs.size());
    //store for tx
    desktopinput_pkt_t tx_pkt(rtx_pkt);
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
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    // can't double connect
    if(m_flags & CLIENT_CONNECTION)
        return false;

    if (localaddr.length() || local_tcpport || local_udpport)
    {
        m_localTcpAddr = ACE_INET_Addr(local_tcpport, localaddr.c_str());
        m_localUdpAddr = ACE_INET_Addr(local_udpport, localaddr.c_str());
    }
    
    //welcome message to look for
    m_serverinfo.systemid = sysid;
    
    m_serverinfo.hostaddrs = DetermineHostAddress(hostaddr, tcpport);
    if (m_serverinfo.hostaddrs.size())
    {
        m_serverinfo.udpaddr = m_serverinfo.hostaddrs[0];
        m_serverinfo.udpaddr.set_port_number(udpport);
    }
    MYTRACE(ACE_TEXT("Resolved %d IP-addresses\n"), int(m_serverinfo.hostaddrs.size()));
    
    if (m_serverinfo.hostaddrs.size() &&
        Connect(encrypted, m_serverinfo.hostaddrs[0], m_localTcpAddr != ACE_INET_Addr() ? &m_localTcpAddr : NULL))
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
    int ret;

#if !defined(UNICODE)
    MYTRACE(ACE_TEXT("Trying remote TCP: %s:%d, Local TCP: %s:%d, UDP: %s:%d\n"),
            hosttcpaddr.get_host_addr(), hosttcpaddr.get_port_number(),
            m_localTcpAddr.get_host_addr(), m_localTcpAddr.get_port_number(),
            m_localUdpAddr.get_host_addr(), m_localUdpAddr.get_port_number());
#endif            
    
#if defined(ENABLE_ENCRYPTION)
    if(encrypted)
    {
        ACE_NEW_RETURN(m_crypt_stream, CryptStreamHandler(&m_reactor), false);
        m_crypt_stream->SetListener(this);
        //ACE_Synch_Options options = ACE_Synch_Options::defaults;
        //ACE only supports OpenSSL on blocking sockets
        ACE_Synch_Options options(ACE_Synch_Options::USE_TIMEOUT, ACE_Time_Value(10));
        if (localtcpaddr)
            ret = m_crypt_connector.connect(m_crypt_stream, hosttcpaddr, 
                                            options, *localtcpaddr);
        else
            ret = m_crypt_connector.connect(m_crypt_stream, hosttcpaddr, 
                                            options);
    }
    else
#endif
    {
        ACE_NEW_RETURN(m_def_stream, DefaultStreamHandler(&m_reactor), false);
        m_def_stream->SetListener(this);
        ACE_Synch_Options options(ACE_Synch_Options::USE_REACTOR, ACE_Time_Value(0,0));
        if (localtcpaddr)
            ret = m_connector.connect(m_def_stream, hosttcpaddr, options, *localtcpaddr);
        else
            ret = m_connector.connect(m_def_stream, hosttcpaddr, options);
    }

    int err = ACE_OS::last_error();
    MYTRACE( ACE_TEXT("Last error: %d. Would block is %d\n"), err, EWOULDBLOCK);
    return ret == 0 || (ret == -1 && err == EWOULDBLOCK);
}

void ClientNode::Disconnect()
{
    ASSERT_REACTOR_LOCKED(this);

    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    while(m_timers.size())
    {
        m_reactor.cancel_timer(m_timers.begin()->second, 0);
        m_timers.erase(m_timers.begin());
    }

    ACE_HANDLE h = ACE_INVALID_HANDLE;
#if defined(ENABLE_ENCRYPTION)
    if(m_crypt_stream)
    {
        //NOTE: same in FileNode::Disconnect()

        m_crypt_stream->SetListener(NULL);
        h = m_crypt_stream->get_handle();

        m_crypt_connector.cancel(m_crypt_stream);
        m_crypt_stream->close();

        m_crypt_stream = NULL;
    }
#endif
    if(m_def_stream)
    {
        //NOTE: same in FileNode::Disconnect()

        m_def_stream->SetListener(NULL);
        h = m_def_stream->get_handle();
        //if we're in the process of connecting, then cancel() which
        //doesn't destroy the handler. We destroy it by calling close 
        //manually
        m_connector.cancel(m_def_stream);
        m_def_stream->close();

        m_def_stream = NULL;
    }

    TTASSERT(m_reactor.find_handler(h) == NULL);

    m_recvbuffer.clear();

    m_packethandler.RemoveListener(this);

    //clear pending packets
    m_tx_queue.Reset();

    m_packethandler.close();

    //clean up file transfers
    m_waitingTransfers.clear();
    m_filetransfers.clear();

    //clear channels and login status
    LoggedOut();

    m_flags &= ~(CLIENT_CONNECTING | CLIENT_CONNECTED);

    m_serverinfo = ServerInfo();
    m_clientstats = ClientStats();
    m_localTcpAddr = m_localUdpAddr = ACE_INET_Addr();
}

void ClientNode::JoinChannel(clientchannel_t& chan)
{
    ASSERT_REACTOR_THREAD(m_reactor);
    ASSERT_REACTOR_LOCKED(this);

    if(!m_mychannel.null())
        LeftChannel(*m_mychannel);

    m_mychannel = chan;

    AudioCodec codec = chan->GetAudioCodec();
    /* Sanity check */
    if(ValidAudioCodec(codec))
    {
        //set encoder properties
        if(m_voice_thread.StartEncoder(std::bind(&ClientNode::EncodedAudioVoiceFrame, this,
                                                 _1, _2, _3, _4, _5),
                                       codec, true))
        {
            if(!UpdateSoundInputPreprocess()) //set AGC, denoise, etc.
            {
                m_listener->OnInternalError(TT_INTERR_AUDIOCONFIG_INIT_FAILED,
                            GetErrorDescription(TT_INTERR_AUDIOCONFIG_INIT_FAILED));
            }
        }
        else
        {
            m_listener->OnInternalError(TT_INTERR_AUDIOCODEC_INIT_FAILED,
                                        GetErrorDescription(TT_INTERR_AUDIOCODEC_INIT_FAILED));
        }
        //start recorder
        OpenAudioCapture(codec);
    }
    else
    {
        m_listener->OnInternalError(TT_INTERR_AUDIOCODEC_INIT_FAILED,
                                    GetErrorDescription(TT_INTERR_AUDIOCODEC_INIT_FAILED));
    }
}

void ClientNode::LeftChannel(ClientChannel& chan)
{
    ASSERT_REACTOR_LOCKED(this);

    TTASSERT(m_mychannel.get());

    if(!m_mychannel.null() && chan.Compare(m_mychannel))
        m_mychannel.reset();

    //clear files if not admin
    if(m_myuseraccount.usertype & USERTYPE_DEFAULT)
        chan.ClearFiles();

    //stop streaming media file
    if(m_flags & (CLIENT_STREAM_AUDIOFILE | CLIENT_STREAM_VIDEOFILE))
    {
        bool notify = !m_media_streamer.null();
        MediaFileProp mfp;
        if(!m_media_streamer.null())
            mfp = m_media_streamer->GetMediaInput();
        StopStreamingMediaFile();
        if(notify)
            m_listener->OnChannelStreamMediaFile(mfp, MFS_ABORTED);
    }

    ClientChannel::users_t users = chan.GetUsers();
    for(size_t i=0;i<users.size();i++)
        users[i]->ResetAllStreams();

    //shutdown audio capture
    if(chan.GetAudioCodec().codec != CODEC_NO_CODEC)
        CloseAudioCapture();

    m_voice_thread.StopEncoder();

    CloseDesktopSession(true);

    //make sure we don't send obsolete packets to channel
    m_tx_queue.RemoveChannelPackets((uint16_t)chan.GetChannelID());
}

void ClientNode::LoggedOut()
{
    ASSERT_REACTOR_LOCKED(this);

    //shutdown users' players
    clientchannel_t chan = GetMyChannel();
    if(!chan.null())
        LeftChannel(*chan);

    //shutdown all users (might be active admin streams)
    intset_t users;
    GetUsers(users);
    intset_t::const_iterator ite = users.begin();
    while(ite != users.end())
    {
        clientuser_t user = GetUser(*ite);
        if(!user.null())
            user->ResetAllStreams();
        ite++;
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
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);
    
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
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_LOGOUT;
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoChangeNickname(const ACE_TString& newnick)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_CHANGENICK;
    AppendProperty(TT_NICKNAME, newnick, command);

    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoChangeStatus(int statusmode, const ACE_TString& statusmsg)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_CHANGESTATUS;
    AppendProperty(TT_STATUSMODE, statusmode, command);
    AppendProperty(TT_STATUSMESSAGE, statusmsg, command);

    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoTextMessage(const TextMessage& msg)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

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
    case TTNoneMsg :
    case TTBroadcastMsg :
        TTASSERT(0);
        break;
    }
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

//Command handlers (Client -> Server)
int ClientNode::DoPing(bool issue_cmdid)
{
    ASSERT_REACTOR_LOCKED(this);

    ACE_TString command = CLIENT_KEEPALIVE;
    if(issue_cmdid)
        AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    
    m_clientstats.ping_issue_time = ACE_OS::gettimeofday();

    if(!issue_cmdid)
        return TransmitCommand(command, 0);
    else
        return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoJoinChannel(const ChannelProp& chanprop)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_JOINCHANNEL;
    if(GetChannel(chanprop.channelid).null()) //new channel
    {
        AppendProperty(TT_CHANNAME, chanprop.name, command);
        AppendProperty(TT_PARENTID, chanprop.parentid, command);
        AppendProperty(TT_TOPIC, chanprop.topic, command);
        AppendProperty(TT_OPPASSWORD, chanprop.oppasswd, command);
        AppendProperty(TT_AUDIOCODEC, chanprop.audiocodec, command);
        AppendProperty(TT_AUDIOCFG, chanprop.audiocfg, command);
        AppendProperty(TT_CHANNELTYPE, chanprop.chantype, command);
        AppendProperty(TT_USERDATA, chanprop.userdata, command);
        AppendProperty(TT_VOICEUSERS, chanprop.GetTransmitUsers(STREAMTYPE_VOICE), command);
        AppendProperty(TT_VIDEOUSERS, chanprop.GetTransmitUsers(STREAMTYPE_VIDEOCAPTURE), command);
        AppendProperty(TT_DESKTOPUSERS, chanprop.GetTransmitUsers(STREAMTYPE_DESKTOP), command);
        AppendProperty(TT_MEDIAFILEUSERS, chanprop.GetTransmitUsers(STREAMTYPE_MEDIAFILE), command);
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
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_LEAVECHANNEL;
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoKickUser(int userid, int channelid)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_KICK;
    if(channelid)
        AppendProperty(TT_CHANNELID, channelid, command);

    AppendProperty(TT_USERID, userid, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoChannelOperator(int userid, int channelid, 
                                  const ACE_TString& oppasswd, bool op)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_CHANNELOP;
    AppendProperty(TT_CHANNELID, channelid, command);
    AppendProperty(TT_USERID, userid, command);
    AppendProperty(TT_OPERATORSTATUS, op, command);
    if(oppasswd.length())
        AppendProperty(TT_OPPASSWORD, oppasswd, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoFileSend(int channelid, const ACE_TString& localfilepath)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_INT64 filesize = ACE_OS::filesize(localfilepath.c_str());
    ACE_TString filename = localfilepath;
    size_t index = localfilepath.rfind(ACE_DIRECTORY_SEPARATOR_CHAR);
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
    TTASSERT(GetUserID()>0);

    //first register file transfer before sending
    ACE_TString command = CLIENT_REGSENDFILE;
    AppendProperty(TT_FILENAME, transfer.filename, command);
    AppendProperty(TT_FILESIZE, filesize, command);
    AppendProperty(TT_CHANNELID, channelid, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    int cmdid = TransmitCommand(command, m_cmdid_counter);
    if(cmdid>0)
        m_waitingTransfers[cmdid] = transfer;
    return cmdid;
}

int ClientNode::DoFileRecv(int channelid, 
                           const ACE_TString& localfilepath, 
                           const ACE_TString& remotefilename)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    FileTransfer transfer;
    transfer.channelid = channelid;
    transfer.localfile = localfilepath;
    transfer.filename = remotefilename;
    transfer.filesize = 0;
    transfer.inbound = true;
    transfer.transferid = 0;
    transfer.userid = GetUserID();
    TTASSERT(GetUserID()>0);

    //first register file transfer before sending
    ACE_TString command = CLIENT_REGRECVFILE;
    AppendProperty(TT_FILENAME, remotefilename, command);
    AppendProperty(TT_CHANNELID, channelid, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    int cmdid = TransmitCommand(command, m_cmdid_counter);
    if(cmdid>0)
        m_waitingTransfers[cmdid] = transfer;
    return cmdid;
}

int ClientNode::DoFileDelete(int channelid, const ACE_TString& filename)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_DELETEFILE;
    AppendProperty(TT_CHANNELID, channelid, command);
    AppendProperty(TT_FILENAME, filename, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoSubscribe(int userid, Subscriptions subscript)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_SUBSCRIBE;
    AppendProperty(TT_USERID, userid, command);
    AppendProperty(TT_LOCALSUBSCRIPTIONS, subscript, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoUnsubscribe(int userid, Subscriptions subscript)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_UNSUBSCRIBE;
    AppendProperty(TT_USERID, userid, command);
    AppendProperty(TT_LOCALSUBSCRIPTIONS, subscript, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoQueryServerStats()
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_QUERYSTATS;
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;
    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoQuit()
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_QUIT;
    command += EOL;
    return TransmitCommand(command, 0);
}

int ClientNode::DoMakeChannel(const ChannelProp& chanprop)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_MAKECHANNEL;
    AppendProperty(TT_PARENTID, chanprop.parentid, command);
    AppendProperty(TT_CHANNAME, chanprop.name, command);
    AppendProperty(TT_PASSWORD, chanprop.passwd, command);
    AppendProperty(TT_TOPIC, chanprop.topic, command);
    AppendProperty(TT_DISKQUOTA, chanprop.diskquota, command);
    AppendProperty(TT_OPPASSWORD, chanprop.oppasswd, command);
    AppendProperty(TT_MAXUSERS, chanprop.maxusers, command);
    AppendProperty(TT_AUDIOCODEC, chanprop.audiocodec, command);
    AppendProperty(TT_AUDIOCFG, chanprop.audiocfg, command);
    AppendProperty(TT_CHANNELTYPE, chanprop.chantype, command);
    AppendProperty(TT_USERDATA, chanprop.userdata, command);
    AppendProperty(TT_VOICEUSERS, chanprop.GetTransmitUsers(STREAMTYPE_VOICE), command);
    AppendProperty(TT_VIDEOUSERS, chanprop.GetTransmitUsers(STREAMTYPE_VIDEOCAPTURE), command);
    AppendProperty(TT_DESKTOPUSERS, chanprop.GetTransmitUsers(STREAMTYPE_DESKTOP), command);
    AppendProperty(TT_MEDIAFILEUSERS, chanprop.GetTransmitUsers(STREAMTYPE_MEDIAFILE), command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoUpdateChannel(const ChannelProp& chanprop)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_UPDATECHANNEL;
    AppendProperty(TT_CHANNELID, chanprop.channelid, command);
    AppendProperty(TT_CHANNAME, chanprop.name, command);
    AppendProperty(TT_PASSWORD, chanprop.passwd, command);
    AppendProperty(TT_TOPIC, chanprop.topic, command);
    AppendProperty(TT_DISKQUOTA, chanprop.diskquota, command);
    AppendProperty(TT_OPPASSWORD, chanprop.oppasswd, command);
    AppendProperty(TT_MAXUSERS, chanprop.maxusers, command);
    AppendProperty(TT_AUDIOCODEC, chanprop.audiocodec, command);
    AppendProperty(TT_AUDIOCFG, chanprop.audiocfg, command);
    AppendProperty(TT_CHANNELTYPE, chanprop.chantype, command);
    AppendProperty(TT_USERDATA, chanprop.userdata, command);
    AppendProperty(TT_VOICEUSERS, chanprop.GetTransmitUsers(STREAMTYPE_VOICE), command);
    AppendProperty(TT_VIDEOUSERS, chanprop.GetTransmitUsers(STREAMTYPE_VIDEOCAPTURE), command);
    AppendProperty(TT_DESKTOPUSERS, chanprop.GetTransmitUsers(STREAMTYPE_DESKTOP), command);
    AppendProperty(TT_MEDIAFILEUSERS, chanprop.GetTransmitUsers(STREAMTYPE_MEDIAFILE), command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoRemoveChannel(int channelid)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_REMOVECHANNEL;
    AppendProperty(TT_CHANNELID, channelid, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoMoveUser(int userid, int channelid)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_MOVEUSER;
    AppendProperty(TT_USERID, userid, command);
    AppendProperty(TT_CHANNELID, channelid, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoUpdateServer(const ServerInfo& serverprop)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_UPDATESERVER;
    AppendProperty(TT_SERVERNAME, serverprop.servername, command);
    AppendProperty(TT_MOTDRAW, serverprop.motd_raw, command);
    AppendProperty(TT_MAXUSERS, serverprop.maxusers, command);
    AppendProperty(TT_MAXLOGINATTEMPTS, serverprop.maxloginattempts, command);
    AppendProperty(TT_MAXLOGINSPERIP, serverprop.max_logins_per_ipaddr, command);
    AppendProperty(TT_AUTOSAVE, serverprop.autosave, command);
    if (serverprop.hostaddrs.size())
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
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoBanUser(int userid, const BannedUser& ban)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_BAN;
    if(userid>0)
        AppendProperty(TT_USERID, userid, command);
    if(ban.ipaddr.length())
        AppendProperty(TT_IPADDR, ban.ipaddr, command);
    AppendProperty(TT_BANTYPE, ban.bantype, command);
    if(ban.username.length())
        AppendProperty(TT_USERNAME, ban.username, command);
    if(ban.chanpath.length())
        AppendProperty(TT_CHANNEL, ban.chanpath, command);

    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoUnBanUser(const BannedUser& ban)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

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
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

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
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_LISTUSERACCOUNTS;
    AppendProperty(TT_INDEX, index, command);
    AppendProperty(TT_COUNT, count, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoNewUserAccount(const UserAccount& user)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

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
    AppendProperty(TT_CMDFLOOD, user.abuse.toParam(), command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoDeleteUserAccount(const ACE_TString& username)
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_DELUSERACCOUNT;
    AppendProperty(TT_USERNAME, username, command);
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::DoSaveConfig()
{
    ASSERT_REACTOR_LOCKED(this);
    ASSERT_NOT_REACTOR_THREAD(m_reactor);

    ACE_TString command = CLIENT_SAVECONFIG;
    AppendProperty(TT_CMDID, GEN_NEXT_ID(m_cmdid_counter), command);
    command += EOL;

    return TransmitCommand(command, m_cmdid_counter);
}

int ClientNode::TransmitCommand(const ACE_TString& command, int cmdid)
{
    ASSERT_REACTOR_LOCKED(this);

    if((m_flags & CLIENT_CONNECTED) == 0)
        return -1;

    MYTRACE(ACE_TEXT("CLIENT #%d: %s"), m_myuserid, command.c_str());

#if defined(UNICODE)
    ACE_CString data = UnicodeToUtf8(command.c_str());
#else
    const ACE_CString& data = command;
#endif /* UNICODE */

    bool empty = m_sendbuffer.empty();
    m_sendbuffer += data; /* OnSend() mutex */
#if defined(ENABLE_ENCRYPTION)
    if (m_crypt_stream && empty)
    {
        int ret = m_reactor.register_handler(m_crypt_stream, ACE_Event_Handler::WRITE_MASK);
        TTASSERT(ret >= 0);
    }
#endif
    if (m_def_stream && empty)
    {
        int ret = m_reactor.register_handler(m_def_stream, ACE_Event_Handler::WRITE_MASK);
        TTASSERT(ret >= 0);
    }

    return cmdid?cmdid:1;
}

void ClientNode::OnOpened()
{
    MYTRACE( ACE_TEXT("Connected successfully on TCP\n") );

    ACE_INET_Addr localaddr;
    if (m_localUdpAddr == ACE_INET_Addr())
    {
#if defined(ENABLE_ENCRYPTION)
        if (m_crypt_stream)
        {
            int ret = m_crypt_stream->peer().peer().get_local_addr(localaddr);
            TTASSERT(ret == 0);
            localaddr.set_port_number(0);
        }
#endif
        if (m_def_stream)
        {
            int ret = m_def_stream->peer().get_local_addr(localaddr);
            TTASSERT(ret == 0);
            localaddr.set_port_number(0);
        }
    }
    else
        localaddr = m_localUdpAddr;
    
    MYTRACE(ACE_TEXT("UDP bind: %s. %d\n"), InetAddrToString(localaddr).c_str(), localaddr.get_type());

    if (m_packethandler.open(localaddr, UDP_SOCKET_RECV_BUF_SIZE,
                             UDP_SOCKET_SEND_BUF_SIZE))
    {
        m_packethandler.AddListener(this);
    }
    else
    {
#if defined(ENABLE_ENCRYPTION)
        if (m_crypt_stream)
        {
            m_crypt_stream->close();
        }
#endif
        if (m_def_stream)
        {
            m_def_stream->close();
        }
    }
}

#if defined(ENABLE_ENCRYPTION)
void ClientNode::OnOpened(CryptStreamHandler::StreamHandler_t& handler)
{
    OnOpened();
}
#endif

void ClientNode::OnOpened(DefaultStreamHandler::StreamHandler_t& handler)
{
    OnOpened();
}

void ClientNode::OnClosed()
{
    GUARD_REACTOR(this);

    bool encrypted = m_def_stream == NULL;
    
#if defined(ENABLE_ENCRYPTION)
    m_crypt_stream = NULL;
#endif
    m_def_stream = NULL;
    

    if (m_serverinfo.hostaddrs.size() > 1)
    {
        m_serverinfo.hostaddrs.erase(m_serverinfo.hostaddrs.begin());
        u_short udpport = m_serverinfo.udpaddr.get_port_number();
        m_serverinfo.udpaddr = m_serverinfo.hostaddrs[0];
        m_serverinfo.udpaddr.set_port_number(udpport);
        if (Connect(encrypted, m_serverinfo.hostaddrs[0],
                    m_localTcpAddr != ACE_INET_Addr() ? &m_localTcpAddr : NULL))
            return;
    }

    if(m_flags & CLIENT_CONNECTED)
    {
        m_flags &= ~CLIENT_CONNECTED;
        //Disconnect and clean up clientnode
        if(m_listener)
            m_listener->OnConnectionLost();
    }
    else if(m_flags & CLIENT_CONNECTING)
    {
        m_flags &= ~CLIENT_CONNECTING;
        //Disconnect and clean up clientnode
        if(m_listener)
            m_listener->OnConnectFailed();
    }
}

#if defined(ENABLE_ENCRYPTION)
void ClientNode::OnClosed(CryptStreamHandler::StreamHandler_t& handler)
{
    OnClosed();
}
#endif

void ClientNode::OnClosed(DefaultStreamHandler::StreamHandler_t& handler)
{
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
bool ClientNode::OnReceive(CryptStreamHandler::StreamHandler_t& handler, const char* buff, int len)
{
    return OnReceive(buff, len);
}
#endif

bool ClientNode::OnReceive(DefaultStreamHandler::StreamHandler_t& handler, const char* buff, int len)
{
    return OnReceive(buff, len);
}

bool ClientNode::OnSend(ACE_Message_Queue_Base& msgqueue)
{
    GUARD_REACTOR(this);
    
    if (m_flags & (CLIENT_CONNECTING | CLIENT_CONNECTED))
    {
        ACE_Time_Value tm = ACE_Time_Value::zero;
        if(m_sendbuffer.length() &&
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
    ACE_TString cmd = tmp_cmd;
    const ACE_CString& command = cmdline;
#endif

    MYTRACE(ACE_TEXT("SERVER #%d: %s"), m_myuserid, command.c_str());

    mstrings_t properties;
    if(ExtractProperties(command, properties)<0)
        return true;

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
    ASSERT_REACTOR_LOCKED(this);

    TTASSERT(GetRootChannel().null());//root channel will be created by add channel command

    if(!GetRootChannel().null())
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

        //start keepalive timer for TCP (if not set, then set it to half the user timeout)
        if(m_tcpkeepalive_interval>0)
            SetKeepAliveInterval(m_tcpkeepalive_interval, m_udpkeepalive_interval);
        else
        {
            int tcp_keepalive = m_serverinfo.usertimeout>1?m_serverinfo.usertimeout/2:1;
            SetKeepAliveInterval(tcp_keepalive, m_udpkeepalive_interval);
        }

        int max_timeout = std::max(m_tcpkeepalive_interval, m_udpkeepalive_interval) * 2;
        SetServerTimeout(max_timeout);

        //start connecting on UDP to server. DoLogin() may not be called before UDP connect succeeds
        StartTimer(TIMER_UDPCONNECT_ID, 0, ACE_Time_Value(),
                    CLIENT_UDPCONNECT_INTERVAL);

        //when to give up
        StartTimer(TIMER_UDPCONNECT_TIMEOUT_ID, 0,
                    CLIENT_UDPCONNECT_SERVER_TIMEOUT);
        
        //now wait for UDP timer to complete connection
    }
    else if(m_listener)
        m_listener->OnCommandError(m_current_cmdid,
                                   TT_CMDERR_INCOMPATIBLE_PROTOCOLS,
                                   GetErrorDescription(TT_CMDERR_INCOMPATIBLE_PROTOCOLS));
}

void ClientNode::HandleAccepted(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    TTASSERT(m_flags & CLIENT_CONNECTED);
    int userid;
    ACE_TString ipaddr;
    ACE_INET_Addr udpaddr;

    if(GetProperty(properties, TT_USERID, userid) &&
       GetProperty(properties, TT_USERTYPE, m_myuseraccount.usertype))
    {
        GetProperty(properties, TT_IPADDR, ipaddr);
        GetProperty(properties, TT_USERNAME, m_myuseraccount.username);
        GetProperty(properties, TT_USERDATA, m_myuseraccount.userdata);
        GetProperty(properties, TT_USERRIGHTS, m_myuseraccount.userrights);
        GetProperty(properties, TT_NOTEFIELD, m_myuseraccount.note);
        GetProperty(properties, TT_INITCHANNEL, m_myuseraccount.init_channel);
        GetProperty(properties, TT_AUTOOPCHANNELS, m_myuseraccount.auto_op_channels);
        GetProperty(properties, TT_AUDIOBPSLIMIT, m_myuseraccount.audiobpslimit);
        vector<int> flood;
        if(GetProperty(properties, TT_CMDFLOOD, flood))
            m_myuseraccount.abuse.fromParam(flood);

        TTASSERT(GetUserID() == userid);
        m_flags |= CLIENT_AUTHORIZED;

        m_local_voicelog->SetUsername(m_myuseraccount.username);

        //notify parent application
        if(m_listener)
            m_listener->OnAccepted(GetUserID(), m_myuseraccount);
    }
}

void ClientNode::HandleServerUpdate(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    GetProperty(properties, TT_SERVERNAME, m_serverinfo.servername);
    GetProperty(properties, TT_MAXUSERS, m_serverinfo.maxusers);
    GetProperty(properties, TT_MAXLOGINATTEMPTS, m_serverinfo.maxloginattempts);
    GetProperty(properties, TT_MAXLOGINSPERIP, m_serverinfo.max_logins_per_ipaddr);
    GetProperty(properties, TT_USERTIMEOUT, m_serverinfo.usertimeout);
    GetProperty(properties, TT_AUTOSAVE, m_serverinfo.autosave);
    GetProperty(properties, TT_VOICETXLIMIT, m_serverinfo.voicetxlimit);
    GetProperty(properties, TT_VIDEOTXLIMIT, m_serverinfo.videotxlimit);
    GetProperty(properties, TT_MEDIAFILETXLIMIT, m_serverinfo.mediafiletxlimit);
    GetProperty(properties, TT_DESKTOPTXLIMIT, m_serverinfo.desktoptxlimit);
    GetProperty(properties, TT_TOTALTXLIMIT, m_serverinfo.totaltxlimit);

    if(m_serverinfo.hostaddrs.size())
    {
        int tcpport = m_serverinfo.hostaddrs[0].get_port_number();
        int udpport = m_serverinfo.udpaddr.get_port_number();
        if (GetProperty(properties, TT_TCPPORT, tcpport))
        {
            for (auto& a : m_serverinfo.hostaddrs)
                a.set_port_number(tcpport);
        }
        if (GetProperty(properties, TT_UDPPORT, udpport))
            m_serverinfo.udpaddr.set_port_number(udpport);
    }
    GetProperty(properties, TT_MOTD, m_serverinfo.motd);
    GetProperty(properties, TT_MOTDRAW, m_serverinfo.motd_raw);
    GetProperty(properties, TT_VERSION, m_serverinfo.version);

    //notify parent application
    m_listener->OnServerUpdate(m_serverinfo);
}

void ClientNode::HandleKeepAlive(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    m_clientstats.tcp_silence_sec = 0;
    m_clientstats.tcpping_time = (ACE_OS::gettimeofday() - m_clientstats.ping_issue_time).msec();
    m_clientstats.tcp_ping_dirty = false;
}

void ClientNode::HandleLoggedIn(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    int userid;
    ACE_TString nick, username;
    ACE_TString ipaddr, clientname;
    ACE_INET_Addr udpaddr;
    int udpport = 0;
    int status = 0;
    ACE_TString statusmsg;
    ACE_TString version;
    UserTypes type = USERTYPE_NONE;
    Subscriptions localsubscript = SUBSCRIBE_NONE, peersubscript = SUBSCRIBE_NONE;
    int userdata = 0;

    if(!GetProperty(properties, TT_USERID, userid))
        return;

    TTASSERT(m_users.find(userid) == m_users.end());
    clientuser_t user (new ClientUser(userid, this, m_listener));

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
    ASSERT_REACTOR_LOCKED(this);

    int userid = 0;
    GetProperty(properties, TT_USERID, userid);

    if(userid == GetUserID() || userid == 0)
    {
        LoggedOut();

        if(m_listener)
            m_listener->OnLoggedOut();
    }
    else
    {
        MYTRACE_COND(m_users.find(userid) == m_users.end(),
                     ACE_TEXT("Unknown user: #%d\n"), userid);

        TTASSERT(m_users.find(userid) != m_users.end());
        //if user is admin he might have file stream coming in
        clientuser_t user = GetUser(userid);
        if(!user.null())
            user->ResetAllStreams();
        m_users.erase(userid);

        if(!user.null())
            m_listener->OnUserLoggedOut(*user);
    }
}

void ClientNode::HandleAddUser(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    int userid = 0, chanid = 0;
    ACE_TString nickname, username;
    ACE_TString ipaddr, clientname;
    int statusmode = 0;
    ACE_TString statusmsg;
    ACE_TString version;
    int packet_protocol = 0;
    int userdata = 0;
    UserTypes type = USERTYPE_NONE;
    Subscriptions localsubscript = SUBSCRIBE_NONE, peersubscript = SUBSCRIBE_NONE;

    if(!GetProperty(properties, TT_USERID, userid))
        return;

    GetProperty(properties, TT_CHANNELID, chanid);
    clientchannel_t chan = GetChannel(chanid);
    if(chan.null())
        return;

    clientuser_t user = GetUser(userid);
    if(user.null()) //view all users disabled scenario
        user = clientuser_t(new ClientUser(userid, this, m_listener));

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

    TTASSERT(user->GetChannel().null());
    user->SetChannel(chan);

    chan->AddUser(user->GetUserID(), user);

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
    ASSERT_REACTOR_LOCKED(this);

    int userid = 0;
    ACE_TString nickname;
    ACE_TString ipaddr, clientname;
    int statusmode = 0;
    ACE_TString statusmsg;
    UserTypes type = USERTYPE_NONE;
    Subscriptions localsubscript = SUBSCRIBE_NONE, peersubscript = SUBSCRIBE_NONE;

    if(!GetProperty(properties, TT_USERID, userid))
        return;

    clientuser_t user = GetUser(userid);
    TTASSERT(!user.null());
    if(user.null())
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
    ASSERT_REACTOR_LOCKED(this);

    int userid = 0, channelid = 0;
    Subscriptions localsubscript = SUBSCRIBE_NONE, peersubscript = SUBSCRIBE_NONE;

    GetProperty(properties, TT_CHANNELID, channelid);
    GetProperty(properties, TT_USERID, userid);

    clientuser_t user = GetUser(userid);
    MYTRACE_COND(user.null(), ACE_TEXT("Unknown user: #%d\n"), userid);
    TTASSERT(user.get());
    clientchannel_t chan = GetChannel(channelid);
    TTASSERT(chan.get());
    if(user.null() || chan.null())
        return;

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
    ASSERT_REACTOR_LOCKED(this);

    ChannelProp chanprop;
    GetProperty(properties, TT_PARENTID, chanprop.parentid);
    GetProperty(properties, TT_CHANNELID, chanprop.channelid);

    clientchannel_t parent = GetChannel(chanprop.parentid);
    clientchannel_t newchan;
    if(!parent.null())
    {
        TTASSERT(!m_rootchannel.null());
        GetProperty(properties, TT_CHANNAME, chanprop.name);
        newchan = clientchannel_t(new ClientChannel(parent, chanprop.channelid, chanprop.name));
        parent->AddSubChannel(newchan);
    }
    else
    {
        TTASSERT(m_rootchannel.null());
        newchan = clientchannel_t(new ClientChannel(chanprop.channelid));
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
    ASSERT_REACTOR_LOCKED(this);

    TTASSERT(!GetRootChannel().null());
    ChannelProp chanprop;
    GetProperty(properties, TT_CHANNELID, chanprop.channelid);
    clientchannel_t chan = GetChannel(chanprop.channelid);
    TTASSERT(chan.get());
    if(chan.null())
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
    if(!m_desktop.null() && chan == m_mychannel &&
        !m_mychannel->CanTransmit(m_myuserid, STREAMTYPE_DESKTOP))
    {
        CloseDesktopSession(false);
        m_listener->OnDesktopTransferUpdate(0, 0);
    }
}

void ClientNode::HandleRemoveChannel(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    int chanid = 0;
    GetProperty(properties, TT_CHANNELID, chanid);

    clientchannel_t chan = GetChannel(chanid);
    TTASSERT(chan.get());
    if(!chan.null())
    {
        clientchannel_t parent = chan->GetParentChannel();
        //make sure it's not the channel we're in
        TTASSERT(GetMyChannel() != chan);

        TTASSERT(!parent.null());
        if(!parent.null())
            parent->RemoveSubChannel(chan->GetName());

        //notify parent application
        m_listener->OnRemoveChannel(*chan);
    }
}

void ClientNode::HandleJoinedChannel(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    int channelid = 0;
    GetProperty(properties, TT_CHANNELID, channelid);

    clientchannel_t chan = GetChannel(channelid);
    TTASSERT(!chan.null());
    if(!chan.null())
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

        if(m_listener)
            m_listener->OnJoinedChannel(chan->GetChannelID());
    }
}

void ClientNode::HandleLeftChannel(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    int channelid = 0;
    GetProperty(properties, TT_CHANNELID, channelid);
    clientchannel_t chan = GetChannel(channelid);
    if(!chan.null())
    {
        LeftChannel(*chan);

        if(m_listener)
            m_listener->OnLeftChannel(channelid);
    }
}

void ClientNode::HandleTextMessage(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    int msgtype;
    TextMessage msg;
    GetProperty(properties, TT_MSGTYPE, msgtype);
    msg.msgType = (MsgType)msgtype;
    GetProperty(properties, TT_USERID, msg.to_userid);
    GetProperty(properties, TT_SRCUSERID, msg.from_userid);
    GetProperty(properties, TT_CHANNELID, msg.channelid);
    GetProperty(properties, TT_MSGCONTENT, msg.content);

    clientuser_t user = GetUser(msg.from_userid);
    if(user.get())
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
    ASSERT_REACTOR_LOCKED(this);

    int kicker = 0, channelid = 0;
    GetProperty(properties, TT_KICKERID, kicker);
    GetProperty(properties, TT_CHANNELID, channelid);

    clientuser_t user = GetUser(kicker);
    //notify parent application
    m_listener->OnKicked(user, channelid);
}

void ClientNode::HandleCmdError(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    int errnum = 0;
    ACE_TString msg;
    GetProperty(properties, TT_ERRORNUM, errnum);
    GetProperty(properties, TT_ERRORMSG, msg);

    if(m_current_cmdid)
        m_waitingTransfers.erase(m_current_cmdid);

    //notify parent application
    if(m_listener)
        m_listener->OnCommandError(m_current_cmdid, errnum, msg);
}

void ClientNode::HandleOk(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    //notify parent application
    if(m_listener)
        m_listener->OnCommandSuccess(m_current_cmdid);
}

void ClientNode::HandleBannedUser(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    BannedUser ban;
    ACE_INT64 bantime;

    GetProperty(properties, TT_BANTYPE, ban.bantype);
    GetProperty(properties, TT_IPADDR, ban.ipaddr);
    GetProperty(properties, TT_CHANNEL, ban.chanpath);
    GetProperty(properties, TT_NICKNAME, ban.nickname);
    GetProperty(properties, TT_USERNAME, ban.username);
    GetProperty(properties, TT_BANTIME, bantime);
    ban.bantime = ACE_Time_Value((time_t)bantime);

    m_listener->OnBannedUser(ban);
}

void ClientNode::HandleUserAccount(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    UserAccount user;
    GetProperty(properties, TT_USERNAME, user.username);
    GetProperty(properties, TT_PASSWORD, user.passwd);
    GetProperty(properties, TT_USERTYPE, user.usertype);
    GetProperty(properties, TT_USERRIGHTS, user.userrights);
    GetProperty(properties, TT_USERDATA, user.userdata);
    GetProperty(properties, TT_NOTEFIELD, user.note);
    GetProperty(properties, TT_INITCHANNEL, user.init_channel);
    GetProperty(properties, TT_AUTOOPCHANNELS, user.auto_op_channels);
    GetProperty(properties, TT_AUDIOBPSLIMIT, user.audiobpslimit);
    vector<int> flood;
    if(GetProperty(properties, TT_CMDFLOOD, flood))
        user.abuse.fromParam(flood);

    m_listener->OnUserAccount(user);
}

void ClientNode::HandleFileAccepted(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    TTASSERT(m_waitingTransfers.size());

    int transferid = 0;
    GetProperty(properties, TT_TRANSFERID, transferid);

    filetransfer_q_t::iterator ite = m_waitingTransfers.find(m_current_cmdid);
    if(ite != m_waitingTransfers.end())
    {
        FileTransfer transfer = ite->second;
        transfer.transferid = transferid;

        m_waitingTransfers.erase(ite);

        FileNode* f_ptr;
#if defined(ENABLE_ENCRYPTION)
        if(m_crypt_stream && m_serverinfo.hostaddrs.size())
            ACE_NEW(f_ptr, FileNode(m_reactor, true, m_serverinfo.hostaddrs[0],
                                    m_serverinfo, transfer, this));
        else
#endif
            if (m_serverinfo.hostaddrs.size())
                ACE_NEW(f_ptr, FileNode(m_reactor, false, m_serverinfo.hostaddrs[0],
                                        m_serverinfo, transfer, this));

        filenode_t ptr(f_ptr);
        m_filetransfers[transferid] = ptr;
        ptr->BeginTransfer();
    }
}

void ClientNode::HandleServerStats(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

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
    ASSERT_REACTOR_LOCKED(this);

    int cmdid = 0;
    GetProperty(properties, TT_CMDID, cmdid);
    m_current_cmdid = cmdid;
    if(cmdid && m_listener)
        m_listener->OnCommandProcessing(cmdid, false);
}

void ClientNode::HandleEndCmd(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    int cmdid = 0;
    GetProperty(properties, TT_CMDID, cmdid);
    m_current_cmdid = 0;
    if(cmdid && m_listener)
        m_listener->OnCommandProcessing(cmdid, true);
}

void ClientNode::HandleAddFile(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    RemoteFile remotefile;
    GetProperty(properties, TT_FILEID, remotefile.fileid);
    GetProperty(properties, TT_FILEOWNER, remotefile.username);
    GetProperty(properties, TT_CHANNELID, remotefile.channelid);
    GetProperty(properties, TT_FILENAME, remotefile.filename);
    GetProperty(properties, TT_FILESIZE, remotefile.filesize);

    clientchannel_t chan = GetChannel(remotefile.channelid);
    TTASSERT(!chan.null());
    if(!chan.null())
    {
        chan->AddFile(remotefile);

        if(m_listener)
            m_listener->OnAddFile(*chan, remotefile);
    }
    else TTASSERT(0);
}

void ClientNode::HandleRemoveFile(const mstrings_t& properties)
{
    ASSERT_REACTOR_LOCKED(this);

    int channelid = 0;
    ACE_TString filename;
    GetProperty(properties, TT_CHANNELID, channelid);
    GetProperty(properties, TT_FILENAME, filename);

    RemoteFile remotefile;
    clientchannel_t chan = GetChannel(channelid);
    TTASSERT(!chan.null());
    if(!chan.null() && chan->GetFile(filename, remotefile))
    {
        chan->RemoveFile(filename);

        if(m_listener)
            m_listener->OnRemoveFile(*chan, remotefile);
    }
    else TTASSERT(0);
}

