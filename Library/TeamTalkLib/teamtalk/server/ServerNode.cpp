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

#include "ServerNode.h"

#include "TeamTalkDefs.h"
#include "DesktopCache.h"

#include "mystd/MyStd.h"
#include "myace/MyINet.h"
#include "teamtalk/CodecCommon.h"

#include <ace/FILE_IO.h>
#include <ace/FILE_Addr.h>
#include <ace/OS.h>
#include <ace/FILE_Connector.h>
#include <ace/Dirent_Selector.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <stack>
#include <utility>

using namespace teamtalk;

constexpr auto UDP_SOCKET_RECV_BUF_SIZE = (1024*1024);
constexpr auto UDP_SOCKET_SEND_BUF_SIZE = (1024*1024);

ServerNode::ServerNode(const ACE_TString& version,
                       ACE_Reactor* timerReactor,
                       ACE_Reactor* tcpReactor, 
                       ACE_Reactor* udpReactor, 
                       ServerNodeListener* listener /*= NULL*/)
                       : m_timer_reactor(timerReactor)
                       , m_tcp_reactor(tcpReactor)
                       , m_udp_reactor(udpReactor)
                       , m_srvguard(listener)
{
    m_properties.version = version;

    int ret = 0;
    ACE_thread_t tid = ACE_Thread::self();
    ret = timerReactor->owner(&tid);
    TTASSERT(ret >= 0);
    m_reactors[tid] = timerReactor;
    ret = tcpReactor->owner(&tid);
    TTASSERT(ret >= 0);
    m_reactors[tid] = tcpReactor;
    ret = udpReactor->owner(&tid);
    TTASSERT(ret >= 0);
    m_reactors[tid] = udpReactor;
}

ServerNode::~ServerNode()
{
    StopServer(true);
    MYTRACE(ACE_TEXT("~ServerNode()\n"));

#if defined(ENABLE_ENCRYPTION)
    CryptStreamHandler::RemoveSSLContext(m_tcp_reactor);
#endif
}

ACE_Lock& ServerNode::Lock()
{
    return m_timer_reactor->lock();
}

void ServerNode::SetServerProperties(const ServerSettings& srvprop)
{
    m_properties = srvprop;
}

const ServerSettings& ServerNode::GetServerProperties() const
{
    //TODO: should also be guarded with lock (read from ServerUser)
    return m_properties;
}

const ServerStats& ServerNode::GetServerStats() const
{
    //TODO: should also be guarded with lock (read from ServerUser)
    return m_stats;
}

ACE_TString ServerNode::GetMessageOfTheDay(int ignore_userid/* = 0*/)
{
    GUARD_OBJ(this, Lock());

    //%users% %uptime% %voicetx% %voicerx% %admins% %lastuser%
    size_t const users = GetAuthorizedUsers(true).size();
    size_t const admins = GetAdministrators().size();
    ACE_TString const uptime = UptimeHours(GetUptime());

    ACE_TString lastuser;
    ACE_Time_Value const duration = ACE_Time_Value::max_time;
    for (const auto& u : GetAuthorizedUsers(false))
    {
        if (u->GetDuration() < duration &&
            u->GetUserID() != ignore_userid)
            lastuser = u->GetNickname();
    }
    ACE_TString motd = m_properties.motd;
    ReplaceAll(motd, ACE_TEXT("%users%"), I2String((int)users));
    ReplaceAll(motd, ACE_TEXT("%admins%"), I2String((int)admins));
    ReplaceAll(motd, ACE_TEXT("%uptime%"), uptime);
    ReplaceAll(motd, ACE_TEXT("%voicetx%"), I2String((ACE_INT64)(m_stats.total_bytessent / 1024)));
    ReplaceAll(motd, ACE_TEXT("%voicerx%"), I2String((ACE_INT64)(m_stats.total_bytesreceived / 1024)));
    ReplaceAll(motd, ACE_TEXT("%lastuser%"), lastuser);

    return motd;
}

bool ServerNode::IsAutoSaving()
{
    ASSERT_SERVERNODE_LOCKED(this);
    return m_properties.autosave;
}

void ServerNode::SetAutoSaving(bool autosave)
{
    ASSERT_SERVERNODE_LOCKED(this);
    m_properties.autosave = autosave;
}

ACE_Time_Value ServerNode::GetUptime() const
{
    ASSERT_SERVERNODE_LOCKED(this);
    return ACE_OS::gettimeofday() - m_stats.starttime;
}

serverchannel_t& ServerNode::GetRootChannel()
{
    ASSERT_SERVERNODE_LOCKED(this);
    return m_rootchannel;
}

const ServerChannel::users_t& ServerNode::GetAdministrators()
{
    ASSERT_SERVERNODE_LOCKED(this);

#if defined(_DEBUG)
    ServerChannel::users_t users;
    for(auto i=m_mUsers.begin(); i != m_mUsers.end(); i++)
    {
        if(((*i).second->GetUserType() & USERTYPE_ADMIN) != 0u)
            users.push_back((*i).second);
    }
    TTASSERT(m_admins.size() == users.size());
#endif

    return m_admins;
}

ServerChannel::users_t ServerNode::GetAdministrators(const ServerChannel& excludeChannel)
{
    ASSERT_SERVERNODE_LOCKED(this);

    ServerChannel::users_t users;
    for (const auto& au : GetAdministrators())
    {
        if (au->GetChannel().get() != &excludeChannel)
            users.push_back(au);
    }

    return users;
}

ServerChannel::users_t ServerNode::GetAuthorizedUsers(bool excludeAdmins/* = false*/)
{
    ASSERT_SERVERNODE_LOCKED(this);

    ServerChannel::users_t users;
    for(auto i=m_mUsers.begin(); i != m_mUsers.end(); i++)
    {
        if( (*i).second->IsAuthorized())
        {
            if(excludeAdmins && (((*i).second->GetUserType() & USERTYPE_ADMIN) != 0u))
                continue;
                            users.push_back((*i).second);
        }
    }
    return users;
}

ServerChannel::users_t ServerNode::GetNotificationUsers(UserRights urights, const serverchannel_t& chan)
{
    ServerChannel::users_t notifyusers;
    if (chan)
        notifyusers = chan->GetUsers();

    ServerChannel::users_t const users = GetAuthorizedUsers(false);
    for (const auto& u : users)
    {
        if ((u->GetUserRights() & urights) == urights && (!chan || chan != u->GetChannel()))
            notifyusers.push_back(u);
    }
    return notifyusers;
}

bool ServerNode::SetFileSharing(const ACE_TString& rootdir)
{
    ASSERT_SERVERNODE_LOCKED(this);

    ACE_TString const fixedpath = FixFilePath(rootdir);
    ACE_Dirent_Selector dir;
    if(dir.open(fixedpath.c_str()) < 0)
        return false;

    m_properties.filesroot = fixedpath;

    return true;    
}

ACE_INT64 ServerNode::GetDiskUsage()
{
    ASSERT_SERVERNODE_LOCKED(this);

    ACE_INT64 diskusage = 0;

    std::stack<serverchannel_t> sweeper;
    sweeper.push(GetRootChannel());

    while(!sweeper.empty())
    {
        serverchannel_t const chan = sweeper.top();
        sweeper.pop();

        diskusage += chan->GetDiskUsage();

        ServerChannel::channels_t subs = chan->GetSubChannels();
        for(const auto & sub : subs)
            sweeper.push(sub);
    }
    return diskusage;
}

int ServerNode::GetActiveFileTransfers(int& uploads, int& downloads)
{
    ASSERT_SERVERNODE_LOCKED(this);

    int tmpDownloads = 0;
    int tmpUploads = 0;
    mapusers_t::const_iterator ite;
    for(ite=m_mUsers.begin();ite!=m_mUsers.end();ite++)
    {
        if(ite->second->GetFileTransferID() != 0)
        {
            int const transferid = ite->second->GetFileTransferID();
            auto const fite = m_filetransfers.find(transferid);
            if(fite == m_filetransfers.end())
                continue;
            if(fite->second.inbound)
                tmpUploads++;
            else
                tmpDownloads++;
        }
    }

    uploads = tmpUploads;
    downloads = tmpDownloads;

    return tmpUploads + tmpDownloads;
}

bool ServerNode::IsEncrypted() const
{
    return m_def_acceptors.empty();
}

int ServerNode::GetChannelID(const ACE_TString& chanpath)
{
    GUARD_OBJ(this, Lock());

    serverchannel_t const chan = ChangeChannel(GetRootChannel(), chanpath);
    if(chan)
        return chan->GetChannelID();
    return 0;
}

bool ServerNode::GetChannelProp(int channelid, ChannelProp& prop)
{
    GUARD_OBJ(this, Lock());

    serverchannel_t const chan = GetChannel(channelid);
    if(!chan)
        return false;

    prop = chan->GetChannelProp();
    return true;
}

serverchannel_t ServerNode::GetChannel(int channelid) const
{
    ASSERT_SERVERNODE_LOCKED(this);

    if(!m_rootchannel)
        return {};

    if(m_rootchannel->GetChannelID() == channelid)
        return m_rootchannel;
            return m_rootchannel->GetSubChannel(channelid, true); //SLOW
}

ErrorMsg ServerNode::UserBeginFileTransfer(FileTransfer& transfer,
                                           MyFile& file)
{
    GUARD_OBJ(this, Lock());

    if(m_properties.filesroot.empty())
        return ErrorMsg(TT_CMDERR_FILESHARING_DISABLED);

    auto const ite = m_filetransfers.find(transfer.transferid);
    if(ite == m_filetransfers.end())
        return ErrorMsg(TT_CMDERR_FILETRANSFER_NOT_FOUND);
    if (transfer.inbound != ite->second.inbound)
        return ErrorMsg(TT_CMDERR_FILETRANSFER_NOT_FOUND);
    if (transfer.transferkey != ite->second.transferkey)
        return ErrorMsg(TT_CMDERR_FILETRANSFER_NOT_FOUND);

    transfer = ite->second;
    TTASSERT(!transfer.filename.empty());
    TTASSERT(!transfer.localfile.empty());

    serverchannel_t const chan = GetChannel(transfer.channelid);
    if(!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    if(transfer.inbound && (chan->FileExists(transfer.filename)))
        return ErrorMsg(TT_CMDERR_FILE_ALREADY_EXISTS);

    // don't hold lock while creating file. It could be slow operation
    g.release();

    if (transfer.inbound)
    {
        if (!file.NewFile(transfer.localfile))
            return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);
    }
    else if (!file.Open(transfer.localfile))
    {
        return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);
    }

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserEndFileTransfer(int transferid)
{
    GUARD_OBJ(this, Lock());

    //file transfer is ok. Now remove it from the list.
    FileTransfer transfer;
    {
        auto const ite = m_filetransfers.find(transferid);
        if(ite == m_filetransfers.end())
            return ErrorMsg(TT_CMDERR_FILETRANSFER_NOT_FOUND);
        transfer = ite->second;
        m_filetransfers.erase(ite);
    }

    serverchannel_t const chan = GetChannel(transfer.channelid);
    if(!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    serveruser_t const user = GetUser(transfer.userid, nullptr);
    if(!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    return transfer.inbound ? FileInboundCompleted(*user, *chan, transfer) : FileOutboundCompleted(*user, *chan, transfer);
}

ErrorMsg ServerNode::FileInboundCompleted(const ServerUser& user, const ServerChannel& chan,
                                          const FileTransfer& transfer)
{
    RemoteFile remotefile;
    remotefile.fileid = std::max(1, m_file_id_counter + 1);
    while(GetRootChannel()->FileExists(remotefile.fileid, true) &&
           remotefile.fileid != m_file_id_counter)
    {
        remotefile.fileid = std::max(1, remotefile.fileid+1);
    }

    if(remotefile.fileid == m_file_id_counter) //no more IDs
        return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);

    m_file_id_counter = remotefile.fileid;

    ACE_TString internalpath = m_properties.filesroot + ACE_DIRECTORY_SEPARATOR_STR;

    ACE_TCHAR newfilename[MAX_STRING_LENGTH+1] = {};
    ACE_UINT32 dat_id = m_file_id_counter;
    ACE_TString local_filename;
    do
    {
        ACE_OS::snprintf(newfilename, MAX_STRING_LENGTH, ACE_TEXT("data_%x") CHANNELFILEEXTENSION,
                         (unsigned int)dat_id++);
        local_filename = internalpath + newfilename;
    }
    while(ACE_OS::filesize(local_filename.c_str())>=0 && std::cmp_not_equal(dat_id , m_file_id_counter));

    internalpath += newfilename;
    if(ACE_OS::rename(transfer.localfile.c_str(), internalpath.c_str()) != 0)
        return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);

    remotefile.username = user.GetUsername();
    remotefile.filename = transfer.filename;
    remotefile.internalname = newfilename;
    remotefile.filesize = transfer.filesize;
    remotefile.channelid = chan.GetChannelID();

    ErrorMsg err = AddFileToChannel(remotefile);
    if (!err.Success())
        return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);

    m_stats.files_bytesreceived += remotefile.filesize;
    if ((m_properties.logevents & SERVERLOGEVENT_FILE_UPLOADED) != 0u)
    {
        m_srvguard->OnFileUploaded(user, chan, remotefile);
    }

    if (IsAutoSaving() && ((chan.GetChannelType() & CHANNEL_PERMANENT) != 0u))
    {
        err = m_srvguard->SaveConfiguration(user, *this);
        if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u))
            m_srvguard->OnSaveConfiguration(&user);
    }
    return err;
}

ErrorMsg ServerNode::FileOutboundCompleted(const ServerUser& user,
                                           const ServerChannel& chan,
                                           const FileTransfer& transfer)
{
    RemoteFile remotefile;
    if (!chan.GetFile(transfer.filename, remotefile))
        return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);

    m_stats.files_bytessent += remotefile.filesize;
    if ((m_properties.logevents & SERVERLOGEVENT_FILE_DOWNLOADED) != 0u)
    {
        m_srvguard->OnFileDownloaded(user, chan, remotefile);
    }
    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserDeleteFile(int userid, int channelid, 
                                    const ACE_TString& filename)
{
    GUARD_OBJ(this, Lock());

    serverchannel_t const chan = GetChannel(channelid);
    serveruser_t const user = GetUser(userid, nullptr);
    if(!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);
    if(!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);
    if(!chan->FileExists(filename))
        return ErrorMsg(TT_CMDERR_FILE_NOT_FOUND);

    RemoteFile remotefile;
    bool const b = chan->GetFile(filename, remotefile);
    TTASSERT(b);

    if( ((user->GetUserType() & USERTYPE_ADMIN) != 0u) || 
        chan->IsOperator(userid) ||
        (remotefile.username == user->GetUsername()))
    {
        ErrorMsg err = RemoveFileFromChannel(filename, chan->GetChannelID());
        if(err.errorno != TT_CMDERR_SUCCESS)
            return err;
    }
    else
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

    if ((m_properties.logevents & SERVERLOGEVENT_FILE_DELETED) != 0u)
    {
        m_srvguard->OnFileDeleted(*user, *chan, remotefile);
    }

    if (IsAutoSaving() && ((chan->GetChannelType() & CHANNEL_PERMANENT) != 0u))
    {
        auto err = m_srvguard->SaveConfiguration(*user, *this);
        if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u))
            m_srvguard->OnSaveConfiguration(user.get());
    }

    ACE_TString const filepath = m_properties.filesroot + ACE_DIRECTORY_SEPARATOR_STR + remotefile.internalname;
    g.release();

    ACE_FILE_Connector con;
    ACE_FILE_IO file;
    if(con.connect(file, ACE_FILE_Addr(filepath.c_str())) >= 0)
    {
        file.remove();
        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
            return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);
}

int ServerNode::StartTimer(ServerTimer timer, timer_userdata userdata, 
                           const ACE_Time_Value& delay, 
                           const ACE_Time_Value& interval)
{
    TimerHandler* th = nullptr;
    ACE_NEW_RETURN(th, TimerHandler(*this, timer,
                                    userdata), -1);

    return m_timer_reactor->schedule_timer(th, delay, interval);
}

int ServerNode::TimerEvent(ACE_UINT32 timer_event_id, long userdata)
{
    GUARD_OBJ(this, Lock());

    switch(timer_event_id)
    {
    case TIMERSRV_ONE_SECOND_ID :
    {
        CheckKeepAlive();

        //update users who change changed UDP port
        serveruser_t user;
        while(!m_updUserIPs.empty())
        {
            user = GetUser(*m_updUserIPs.begin(), nullptr);
            if(user && user->IsAuthorized())
                UserUpdate(user->GetUserID());
            m_updUserIPs.erase(m_updUserIPs.begin());
        }

        //update throughput
        m_stats.avg_bytesreceived = m_stats.total_bytesreceived - m_stats.last_bytesreceived;
        m_stats.avg_bytessent = m_stats.total_bytessent - m_stats.last_bytessent;
        m_stats.last_bytesreceived = m_stats.total_bytesreceived;
        m_stats.last_bytessent = m_stats.total_bytessent;
        m_stats.last_voice_bytessent = m_stats.voice_bytessent;
        m_stats.last_vidcap_bytessent = m_stats.vidcap_bytessent;
        m_stats.last_mediafile_bytessent = m_stats.mediafile_bytessent;
        m_stats.last_desktop_bytessent = m_stats.desktop_bytessent;

        UpdateSoloTransmitChannels();

        // erase login delays
        ACE_Time_Value const now = ACE_OS::gettimeofday();
        for (auto it = m_logindelay.begin();it != m_logindelay.end();)
        {
            if (now > it->second + ToTimeValue(m_properties.logindelay * 2))
                m_logindelay.erase(it++);
            else
                ++it;
        }
        break;
    }
    case TIMERSRV_DESKTOPACKPACKET_ID :
        SendDesktopAckPacket(userdata);
        return -1;
    case TIMERSRV_DESKTOPPACKET_RTX_TIMEOUT_ID :
    {
        timer_userdata tm_data;
        tm_data.userdata = userdata;
        if(!RetransmitDesktopPackets(tm_data.src_userid, 
                                     tm_data.dest_userid))
        {
            m_desktop_rtx_timers.erase(tm_data.userdata);
            MYTRACE(ACE_TEXT("Cancelled RTO for #%d -> #%d\n"), tm_data.src_userid, 
                    tm_data.dest_userid);
            return -1;
        }
//         MYTRACE(ACE_TEXT("RTO for #%d -> #%d\n"), tm_data.src_userid, 
//                 tm_data.dest_userid);
        return 0;
    }
    case TIMERSRV_START_DESKTOPTX_ID :
    {
        timer_userdata tm_data;
        tm_data.userdata = userdata;
        serveruser_t const src_user = GetUser(tm_data.src_userid, nullptr);
        serveruser_t const dest_user = GetUser(tm_data.dest_userid, src_user.get());
        if(src_user && dest_user)
        {
            serverchannel_t const chan = src_user->GetChannel();
            if(chan && chan == dest_user->GetChannel())
                StartDesktopTransmitter(*src_user, *dest_user, *chan);
        }
        return -1;
    }
    case TIMERSRV_CLOSE_DESKTOPSESSION_ID :
    {
        //continously notify the user that the desktop session has
        //ended
        timer_userdata tm_data;
        tm_data.userdata = userdata;
        serveruser_t const src_user = GetUser(tm_data.src_userid, nullptr);
        serveruser_t const dest_user = GetUser(tm_data.dest_userid, src_user.get());
        if(src_user && dest_user)
        {
            ClosedDesktopSession session;
            if(!dest_user->GetClosedDesktopSession(src_user->GetUserID(), 
                                                   session))
                return -1;

            serverchannel_t const chan = src_user->GetChannel();
            if(!chan)
                return -1;

            DesktopNakPacket nak_pkt(src_user->GetUserID(),
                                     session.update_id,
                                     session.session_id);
            nak_pkt.SetChannel(chan->GetChannelID());
#if defined(ENABLE_ENCRYPTION)
            if(!m_crypt_acceptors.empty())
            {
                CryptDesktopNakPacket const crypt_pkt(nak_pkt, chan->GetEncryptKey());
                SendPacket(crypt_pkt, *dest_user);
            }
            else
#endif
            {
                SendPacket(nak_pkt, *dest_user);
            }
            return 0;
        }
        return -1;
    }
    case TIMERSRV_COMMAND_RESUME_ID :
    {
        timer_userdata tm_data;
        tm_data.userdata = userdata;
        serveruser_t const src_user = GetUser(tm_data.src_userid, nullptr, false);
        if(src_user)
            src_user->ProcessCommandQueue(true);

        return -1;
    }
    default:
        TTASSERT(0);
    }
    return 0;
}

bool ServerNode::SendDesktopAckPacket(int userid)
{
    ASSERT_SERVERNODE_LOCKED(this);

    TTASSERT(m_desktop_ack_timers.find(userid) != m_desktop_ack_timers.end());

    m_desktop_ack_timers.erase(userid);

    serveruser_t const tmp_user = GetUser(userid, nullptr);
    if(!tmp_user)
        return false;

    ServerUser const& user = *tmp_user;
    serverchannel_t const tmp_chan = user.GetChannel();
    if (!tmp_chan)
        return false;

    ServerChannel const& chan = *tmp_chan;

    std::set<uint16_t> recv_packets;
    uint32_t time_ack = 0;
    uint8_t session_id = 0;

    if (!user.GetDesktopSession())
    {
        //check current packet queue and ack
        const desktoppackets_t& session_q = user.GetDesktopSessionQueue();
        auto const ii = session_q.begin();
        
        if(ii != session_q.end())
        {
            time_ack = (*ii)->GetTime();
            session_id = (*ii)->GetSessionID();
            
            if(!GetAckedDesktopPackets(session_id, time_ack, session_q, 
                                       recv_packets))
                return false;
        }
        else return false;
    }
    else
    {
        DesktopCache const& desktop = *user.GetDesktopSession();
        session_id = desktop.GetSessionID();
        time_ack = desktop.GetPendingUpdateTime();

        desktop.GetReceivedPackets(time_ack, recv_packets);
    }

    packet_range_t recv_range;
    std::set<uint16_t> recv_single;
    GetPacketRanges(recv_packets, recv_range, recv_single);

    DesktopAckPacket ack_pkt(0, GETTIMESTAMP(), user.GetUserID(), 
                             session_id, time_ack, recv_single, recv_range);
    ack_pkt.SetChannel(chan.GetChannelID());
#if defined(ENABLE_ENCRYPTION)
    if(!m_crypt_acceptors.empty())
    {
        CryptDesktopAckPacket const crypt_pkt(ack_pkt, chan.GetEncryptKey());
        SendPacket(crypt_pkt, user);
    }
    else
#endif
    {
        SendPacket(ack_pkt, user);
    }
    //set<uint16_t>::const_iterator ii=recv_packets.begin();
    //MYTRACE(ACE_TEXT("Received packets in upd %u: "), time_ack);
    //while(ii != recv_packets.end())
    //{
    //    MYTRACE(ACE_TEXT("%d,"), *ii);
    //    ii++;
    //}
    //MYTRACE(ACE_TEXT("\n"));

    uint16_t max_packet = 0;
    if(!recv_single.empty())
        max_packet = *(--recv_single.end());
    if(!recv_range.empty())
    {
        auto ii=recv_range.end();
        ii--;
        max_packet = std::max(max_packet, ii->second);
    }

//     MYTRACE(ACE_TEXT("Ack sent %u, mac packet index %d\n"), GETTIMESTAMP(), 
//             max_packet);


    return true;
}

bool ServerNode::RetransmitDesktopPackets(int src_userid, int dest_userid)
{
    ASSERT_SERVERNODE_LOCKED(this);

    serveruser_t const src_user = GetUser(src_userid, nullptr);
    if(!src_user)
        return false;

    serveruser_t const dest_user = GetUser(dest_userid, src_user.get());
    if(!dest_user)
        return false;

    desktop_transmitter_t const desktop_tx = dest_user->GetDesktopTransmitter(src_userid);
    if (!desktop_tx)
        return false;

    serverchannel_t const chan = src_user->GetChannel();
    if(!chan)
        return false;

    TTASSERT(chan == dest_user->GetChannel());

    desktoppackets_t rtx_packets;
    desktop_tx->GetLostDesktopPackets(DESKTOP_DEFAULT_RTX_TIMEOUT, rtx_packets, 1);
    auto dpi = rtx_packets.begin();
//     MYTRACE_COND(dpi == rtx_packets.end(), ACE_TEXT("No packets for RTO\n"));
    for(;dpi != rtx_packets.end();dpi++)
    {
        TTASSERT(chan->GetChannelID() == (*dpi)->GetChannel());
#if defined(ENABLE_ENCRYPTION)
        if(!m_crypt_acceptors.empty())
        {
            CryptDesktopPacket const crypt_pkt(*(*dpi), chan->GetEncryptKey());
            if(SendPacket(crypt_pkt, *dest_user) <= 0)
                break;
        }
        else
#endif
        {
            if(SendPacket(*(*dpi), *dest_user) <= 0)
                break;
        }
    }
    return true;
}

bool ServerNode::StartDesktopTransmitter(const ServerUser& src_user,
                                         ServerUser& dest_user,
                                         const ServerChannel& chan)
{
    ASSERT_SERVERNODE_LOCKED(this);

    if (!src_user.GetDesktopSession())
        return false;
    if(((chan.GetChannelType() & CHANNEL_OPERATOR_RECVONLY) != 0u) &&
       !chan.IsOperator(src_user.GetUserID()) &&
       (src_user.GetUserType() & USERTYPE_ADMIN) == 0 &&
       !chan.IsOperator(dest_user.GetUserID()) &&
       (dest_user.GetUserType() & USERTYPE_ADMIN) == 0)
       return false;

    DesktopCache const& desktop = *src_user.GetDesktopSession();
    if(!desktop.IsReady())
        return false;
    uint8_t const session_id = desktop.GetSessionID();
    uint32_t const update_time = desktop.GetCurrentDesktopTime();
    
    //ensure existing desktop transmitter has finished and we're not trying to
    //transfer the same desktop update again
    desktop_transmitter_t dtx = dest_user.GetDesktopTransmitter(src_user.GetUserID());
    if (dtx)
    {
        //don't start new until current has finished
        if(!dtx->Done())
            return false;
        //don't start new if it's the same as the user already has
        if(dtx->GetSessionID() == session_id && dtx->GetUpdateID() == update_time)
            return false;
    }

    //resume desktop transmission (if already exists and done)
    if (dtx && dtx->GetSessionID() == session_id)
        dtx = dest_user.ResumeDesktopTransmitter(src_user, chan, desktop);

    if (!dtx)
    {
        dest_user.CloseDesktopTransmitter(src_user.GetUserID(), false);
        //start new transmitter
        dtx = dest_user.StartDesktopTransmitter(src_user, chan, desktop);
        if (!dtx)
            return false;
    }
    
    //send desktop update
    desktoppackets_t tx_packets;
    dtx->GetNextDesktopPackets(tx_packets);
    auto dpi = tx_packets.begin();
    while(dpi != tx_packets.end())
    {
#if defined(ENABLE_ENCRYPTION)
        if(!m_crypt_acceptors.empty())
        {
            CryptDesktopPacket const crypt_pkt(*(*dpi), chan.GetEncryptKey());
            if(SendPacket(crypt_pkt, dest_user) <= 0)
                break;
        }
        else
#endif
        {
            if(SendPacket(*(*dpi), dest_user) <= 0)
                break;
        }
        dpi++;
    }

    timer_userdata tm_data;
    tm_data.dest_userid = dest_user.GetUserID();
    tm_data.src_userid = src_user.GetUserID();

    //check for existing RTX timer
    if(m_desktop_rtx_timers.contains(tm_data.userdata))
        return true;

    //start RTX timer
    TimerHandler* th = nullptr;
    ACE_NEW_NORETURN(th, TimerHandler(*this,
                                      TIMERSRV_DESKTOPPACKET_RTX_TIMEOUT_ID,
                                      tm_data.userdata));
    if(th != nullptr)
    {
        long const timerid = m_timer_reactor->schedule_timer(th, nullptr, 
                                                    DESKTOP_DEFAULT_RTX_TIMEOUT, 
                                                    DESKTOP_DEFAULT_RTX_TIMEOUT);
        if(timerid>=0)
            m_desktop_rtx_timers[tm_data.userdata] = timerid;
    }
    return true;
}

void ServerNode::StopDesktopTransmitter(const ServerUser& src_user,
                                        ServerUser& dest_user,
                                        bool start_nak_timer)
{
    ASSERT_SERVERNODE_LOCKED(this);

    dest_user.CloseDesktopTransmitter(src_user.GetUserID(), start_nak_timer);

    timer_userdata tm_data;
    tm_data.dest_userid = dest_user.GetUserID();
    tm_data.src_userid = src_user.GetUserID();

    auto const uti = m_desktop_rtx_timers.find(tm_data.userdata);
    if(uti != m_desktop_rtx_timers.end())
    {
        m_timer_reactor->cancel_timer(uti->second, nullptr, 0);
        m_desktop_rtx_timers.erase(uti);
    }

    if(start_nak_timer)
    {
        TimerHandler* th = nullptr;
        timer_userdata tm_data;
        tm_data.src_userid = src_user.GetUserID();
        tm_data.dest_userid = dest_user.GetUserID();
        ACE_NEW_NORETURN(th, TimerHandler(*this, TIMERSRV_CLOSE_DESKTOPSESSION_ID,
                                          tm_data.userdata));
        long const timerid = m_timer_reactor->schedule_timer(th, nullptr, ACE_Time_Value(1));
        TTASSERT(timerid>=0);
    }
}

bool ServerNode::StartServer(bool encrypted, const ACE_TString& sysid)
{
    GUARD_OBJ(this, Lock());

    //don't allow server to start if there's no root channel specified
    if(!m_rootchannel)
        return false;

    bool tcpport = m_properties.tcpaddrs.size() > 0;
    bool udpport = m_properties.udpaddrs.size() > 0;
    for (const auto& a : m_properties.tcpaddrs)
    {
#if defined(ENABLE_ENCRYPTION)
        if (encrypted)
        {
            cryptacceptor_t const ca(new CryptAcceptor(a, m_tcp_reactor, ACE_NONBLOCK, this));
            tcpport &= ca->acceptor().get_handle() != ACE_INVALID_HANDLE;
            m_crypt_acceptors.push_back(ca);
        }
        else
#endif
        {
            defaultacceptor_t const da(new DefaultAcceptor(a, m_tcp_reactor, ACE_NONBLOCK, this));
            tcpport &= da->acceptor().get_handle() != ACE_INVALID_HANDLE;
            m_def_acceptors.push_back(da);
        }
    }

    for (const auto& a : m_properties.udpaddrs)
    {
        packethandler_t const ph(new PacketHandler(m_udp_reactor));
        udpport &= ph->Open(a, UDP_SOCKET_RECV_BUF_SIZE, UDP_SOCKET_SEND_BUF_SIZE);
        if (udpport)
            ph->AddListener(this);
        m_packethandlers.push_back(ph);
    }

    if(tcpport && udpport)
    {
        //reset stats
        m_stats = ServerStats();

        //start keepalive timer
        ACE_Time_Value const interval(SERVER_KEEPALIVE_DELAY);
        auto* th = new TimerHandler(*this, TIMERSRV_ONE_SECOND_ID);
        m_onesec_timerid = m_timer_reactor->schedule_timer(th, nullptr, interval, interval);
        TTASSERT(m_onesec_timerid>=0);

        //uptime
        m_stats.starttime = ACE_OS::gettimeofday();
        //init random numbers
        ACE_OS::srand(ACE_OS::gettimeofday().msec());

        // system id in welcome message
        if(!sysid.empty())
            m_properties.systemid = sysid;
        else
            m_properties.systemid = SERVER_WELCOME;
    }
    else
    {
        StopServer(false);
    }

    return tcpport && udpport;
}

void ServerNode::StopServer(bool docallback)
{
    GUARD_OBJ(this, Lock());

    m_timer_reactor->cancel_timer(m_onesec_timerid, nullptr, 0);
    m_onesec_timerid = -1;
    //close sockets
    //disconnect users
    while(!m_mUsers.empty())
    {
        ACE_HANDLE const h = m_mUsers.begin()->second->ResetStreamHandle();
        TTASSERT(h != ACE_INVALID_HANDLE);
        ACE_Event_Handler* handler = m_tcp_reactor->find_handler(h);
        TTASSERT(handler);
        handler->reactor()->remove_handler(handler, ACE_Event_Handler::ALL_EVENTS_MASK);
    }

    TTASSERT(m_admins.empty());
    m_failedlogins.clear();
    m_logindelay.clear();
    m_filetransfers.clear();
    m_updUserIPs.clear();

    m_packethandlers.clear();

#if defined(ENABLE_ENCRYPTION)
    m_crypt_acceptors.clear();
#endif
    m_def_acceptors.clear();


    if (docallback)
        m_srvguard->OnShutdown(m_stats);
}

serveruser_t ServerNode::GetUser(int userid, const ServerUser* caller, bool authenticated/* = true*/)
{
    ASSERT_SERVERNODE_LOCKED(this);

    //find user with userid
    auto const ite = m_mUsers.find(userid);
    serveruser_t user;
    if (ite != m_mUsers.end())
    {
        user = (*ite).second;
        if (authenticated && !user->IsAuthorized())
            return {};
    }

    if ((caller != nullptr) && user && caller != user.get())
    {
        if ((caller->GetUserRights() & USERRIGHT_VIEW_ALL_USERS) == USERRIGHT_NONE &&
            caller->GetChannel() != user->GetChannel())
            return {};
    }

    return user;
}

int ServerNode::GetNewUserID()
{
    ASSERT_SERVERNODE_LOCKED(this);

    bool found = false;
    bool overflowed = false;

    while(!found)
    {
        auto const ite = m_mUsers.find(++m_userid_counter);
        if(ite == m_mUsers.end() && m_userid_counter < TT_MAX_ID)
            found = true;//found a free User ID
        else if(m_userid_counter >= TT_MAX_ID && !overflowed)
        {
            m_userid_counter = 1;
            overflowed = true;
        }
        else if(m_userid_counter >= TT_MAX_ID && overflowed)
        {
            //no userid available
            m_userid_counter = 0;
            found = true;
        }
    }

    return m_userid_counter;
}

ACE_Event_Handler* ServerNode::RegisterStreamCallback(ACE_HANDLE h)
{
    TTASSERT(h != ACE_INVALID_HANDLE);
    TTASSERT(m_streamhandles.find(h) != m_streamhandles.end());
    ACE_Event_Handler* handler = m_tcp_reactor->find_handler(h);
    TTASSERT(handler);
    if(handler != nullptr)
    {
        int const ret = m_tcp_reactor->register_handler(handler, ACE_Event_Handler::WRITE_MASK);
        TTASSERT(ret >= 0);
    }
    return handler;
}

void ServerNode::OnOpened(ACE_HANDLE h, serveruser_t& user)
{
    ASSERT_SERVERNODE_LOCKED(this);

    m_mUsers[user->GetUserID()] = user;
    user->SetLastKeepAlive(0);
    m_streamhandles[h] = user;

    user->DoWelcome(m_properties);
    if ((m_properties.logevents & SERVERLOGEVENT_USER_CONNECTED) != 0u)
    {
        m_srvguard->OnUserConnected(*user);
    }
}

#if defined(ENABLE_ENCRYPTION)
ACE_SSL_Context* ServerNode::SetupEncryptionContext()
{
    CryptStreamHandler::RemoveSSLContext(m_tcp_reactor);

    return CryptStreamHandler::AddSSLContext(m_tcp_reactor);
}

void ServerNode::OnOpened(CryptStreamHandler::StreamHandler_t& handler)
{
    GUARD_OBJ(this, Lock());

    int const userid = GetNewUserID();
    if(userid != 0)
    {
        ServerUser* ptr = nullptr;
        ACE_NEW(ptr, ServerUser(userid, *this, handler.get_handle()));
        serveruser_t user(ptr);

        ACE_INET_Addr addr;
#if defined(ENABLE_ENCRYPTION)
        handler.peer().peer().get_remote_addr(addr);
#endif
        ACE_TString str;
#if defined(UNICODE)
        str = ACE_Ascii_To_Wide(addr.get_host_addr()).wchar_rep();
#else
        str = addr.get_host_addr();
#endif
        user->SetIpAddress(str);

        int val = 1;
        int const ret = ACE_OS::setsockopt(handler.peer().get_handle(), SOL_SOCKET, 
                                     SO_KEEPALIVE, (char*)&val, sizeof(val));
        TTASSERT(ret != -1);

        OnOpened(handler.get_handle(), user);
    }
}
#endif

void ServerNode::OnOpened(DefaultStreamHandler::StreamHandler_t& handler)
{
    GUARD_OBJ(this, Lock());

    int const userid = GetNewUserID();
    if(userid != 0)
    {
        ServerUser* ptr = nullptr;
        ACE_NEW(ptr, ServerUser(userid, *this, handler.get_handle()));
        serveruser_t user(ptr);

        ACE_INET_Addr addr;
        handler.peer().get_remote_addr(addr);
        ACE_TString str;
#if defined(UNICODE)
        str = ACE_Ascii_To_Wide(addr.get_host_addr()).wchar_rep();
#else
        str = addr.get_host_addr();
#endif
        user->SetIpAddress(str);

        int val = 1;
        int const ret = ACE_OS::setsockopt(handler.peer().get_handle(), SOL_SOCKET, 
                                     SO_KEEPALIVE, (char*)&val, sizeof(val));
        TTASSERT(ret != -1);

        OnOpened(handler.get_handle(), user);
    }
}

void ServerNode::OnClosed(ACE_HANDLE h)
{
    TTASSERT(m_streamhandles.find(h) != m_streamhandles.end());
    serveruser_t const user = m_streamhandles[h];
    TTASSERT(user.get());
    if(user.get() != nullptr)
    {
        // ensure data is not queued up for this user
        user->ResetStreamHandle();
        UserDisconnected(user->GetUserID());
    }
    m_streamhandles.erase(h);
}

#if defined(ENABLE_ENCRYPTION)
void ServerNode::OnClosed(CryptStreamHandler::StreamHandler_t& handler)
{
    auto sslerr = ACE_OS::last_error();
    if ((sslerr != 0) && ((m_properties.logevents & SERVERLOGEVENT_USER_CRYPTERROR) != 0u))
    {
        char sslerr_str[MAX_STRING_LENGTH] = "";
        ::ERR_error_string_n(sslerr, sslerr_str, sizeof(sslerr_str)-1);

        TTASSERT(m_streamhandles.find(handler.get_handle()) != m_streamhandles.end());
        serveruser_t const user = m_streamhandles[handler.get_handle()];
        TTASSERT(user.get());
        if (user.get() != nullptr)
        {
            m_srvguard->OnUserCryptError(*user, sslerr, LocalToUnicode(sslerr_str));
        }
    }
    OnClosed(handler.get_handle());
}
#endif

void ServerNode::OnClosed(DefaultStreamHandler::StreamHandler_t& handler)
{
    OnClosed(handler.get_handle());
}

bool ServerNode::OnReceive(ACE_HANDLE h, const char* buff, int len)
{
    TTASSERT(m_streamhandles.find(h) != m_streamhandles.end());

    serveruser_t const user = m_streamhandles[h];
    if(user)
        return user->ReceiveData(buff, len);
    return false;
}

#if defined(ENABLE_ENCRYPTION)
bool ServerNode::OnReceive(CryptStreamHandler::StreamHandler_t& handler, const char* buff, int len)
{
    return OnReceive(handler.get_handle(), buff, len);
}
#endif

bool ServerNode::OnReceive(DefaultStreamHandler::StreamHandler_t& handler, const char* buff, int len)
{
    return OnReceive(handler.get_handle(), buff, len);
}

#if defined(ENABLE_ENCRYPTION)
bool ServerNode::OnSend(CryptStreamHandler::StreamHandler_t& handler)
{
    TTASSERT(m_streamhandles.find(handler.get_handle()) != m_streamhandles.end());
    serveruser_t const user = m_streamhandles[handler.get_handle()];
    if(user)
        return user->SendData(*handler.msg_queue());
    return false;
}

#endif

bool ServerNode::OnSend(DefaultStreamHandler::StreamHandler_t& handler)
{
    TTASSERT(m_streamhandles.find(handler.get_handle()) != m_streamhandles.end());
    serveruser_t const user = m_streamhandles[handler.get_handle()];
    if(user)
        return user->SendData(*handler.msg_queue());
    return false;
}

void ServerNode::IncLoginAttempt(const ServerUser& user)
{
    ASSERT_SERVERNODE_LOCKED(this);

    //ban user's IP if logged in to many times
    m_failedlogins[user.GetIpAddress()].push_back(ACE_OS::gettimeofday());

    if (m_properties.maxloginattempts > 0 && 
        m_failedlogins[user.GetIpAddress()].size() >= (size_t)m_properties.maxloginattempts)
    {
        BannedUser ban;
        ban.bantype = BANTYPE_IPADDR;
        ban.ipaddr = user.GetIpAddress();
        m_srvguard->AddUserBan(user, ban);
        if ((m_properties.logevents & SERVERLOGEVENT_USER_BANNED) != 0u)
        {
            m_srvguard->OnUserBanned(user.GetIpAddress(), user);
        }

        if (IsAutoSaving())
        {
            auto err = m_srvguard->SaveConfiguration(user, *this);
            if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u))
                m_srvguard->OnSaveConfiguration(&user);
        }
    }
}

bool ServerNode::LoginsExceeded(const ServerUser& user)
{
    ASSERT_SERVERNODE_LOCKED(this);

    if (m_properties.logindelay == 0)
        return false;

    ACE_Time_Value const now = ACE_OS::gettimeofday();
    if (!m_logindelay.contains(user.GetIpAddress()))
    {
        m_logindelay[user.GetIpAddress()] = now;
        return false;
    }

    ACE_Time_Value const delay = ToTimeValue(m_properties.logindelay);
    if (m_logindelay[user.GetIpAddress()] + delay > now)
    {
        m_logindelay[user.GetIpAddress()] = now;
        return true;
    }
    m_logindelay[user.GetIpAddress()] = now;
    
    return false;
}

int ServerNode::SendPacket(const FieldPacket& packet,
                           const ACE_INET_Addr& remoteaddr,
                           const ACE_INET_Addr& localaddr)
{
    int buffers;
    int ret = -1;
    const iovec* vv = packet.GetPacket(buffers);
    TTASSERT(packet.Finalized() || packet.GetKind() == PACKET_KIND_HELLO || packet.GetKind() == PACKET_KIND_KEEPALIVE);
    for (auto& ph : m_packethandlers)
    {
        if (ph->GetLocalAddr() == localaddr)
        {
            SocketOptGuard const sog(ph->Socket(), IPPROTO_IP, IP_TOS,
                               ToIPTOSValue(packet));
            
            if ((m_properties.txloss != 0) && ((m_stats.packets_sent % m_properties.txloss) == 0))
            {
                ret = packet.GetPacketSize();
                MYTRACE(ACE_TEXT("Simulated TX dropped packet. Kind %d\n"), int(packet.GetKind()));
            }
            else
            {
                ret = int(ph->Socket().send(vv, buffers, remoteaddr));
            }
            m_stats.packets_sent++;
        }
    }
    TTASSERT(ret);
    return ret;
}

int ServerNode::SendPacket(const FieldPacket& packet, const ServerUser& user)
{
    return SendPacket(packet, user.GetUdpAddress(), user.GetLocalUdpAddress());
}

int ServerNode::SendPackets(const FieldPacket& packet,
                            const ServerChannel::users_t& users)
{
    wguard_t const g(m_sendmutex);

#ifdef _DEBUG
    //ensure the same destination doesn't appear twice
    for(size_t i=0;i<users.size();i++)
    {
        for(size_t j=0;j<users.size();j++)
        {
            if(i!=j)
                TTASSERT(users[i] != users[j]);
        }
    }
#endif

    ssize_t sent = 0;
    for(const auto & user : users)
    {
        //check that bandwidth limits are not exceeded
        switch(packet.GetKind())
        {
        case PACKET_KIND_VOICE :
        case PACKET_KIND_VOICE_CRYPT :
            if((m_properties.voicetxlimit != 0) && 
               m_stats.voice_bytessent + packet.GetPacketSize() >
               m_stats.last_voice_bytessent + m_properties.voicetxlimit)
               continue;
            break;
        case PACKET_KIND_VIDEO :
        case PACKET_KIND_VIDEO_CRYPT :
            if((m_properties.videotxlimit != 0) && 
               m_stats.vidcap_bytessent + packet.GetPacketSize() >
               m_stats.last_vidcap_bytessent + m_properties.videotxlimit)
               continue;
            break;
        case PACKET_KIND_MEDIAFILE_AUDIO :
        case PACKET_KIND_MEDIAFILE_AUDIO_CRYPT :
        case PACKET_KIND_MEDIAFILE_VIDEO :
        case PACKET_KIND_MEDIAFILE_VIDEO_CRYPT :
            if((m_properties.mediafiletxlimit != 0) && 
               m_stats.mediafile_bytessent + packet.GetPacketSize() >
               m_stats.last_mediafile_bytessent + m_properties.mediafiletxlimit)
               continue;
            break;
        case PACKET_KIND_DESKTOP :
        case PACKET_KIND_DESKTOP_CRYPT :
            if((m_properties.desktoptxlimit != 0) &&
               m_stats.desktop_bytessent + packet.GetPacketSize() >
               m_stats.last_desktop_bytessent + m_properties.desktoptxlimit)
                continue;
        }

        if((m_properties.totaltxlimit != 0) && 
           m_stats.total_bytessent + packet.GetPacketSize() >
           m_stats.last_bytessent + m_properties.totaltxlimit)
            continue;

        //ok to send packet
        int const ret = SendPacket(packet, *user);
        if(ret <= 0)
            continue;

        sent += ret;

        //update stats
        m_stats.total_bytessent += ret;
        switch(packet.GetKind())
        {
        case PACKET_KIND_HELLO :
        case PACKET_KIND_KEEPALIVE :
            break;
        case PACKET_KIND_VOICE :
            m_stats.voice_bytessent += ret;
            TTASSERT(!m_def_acceptors.empty());
            break;
        case PACKET_KIND_VOICE_CRYPT :
            m_stats.voice_bytessent += ret;
#if defined(ENABLE_ENCRYPTION)
            TTASSERT(!m_crypt_acceptors.empty());
#endif
            break;
        case PACKET_KIND_VIDEO :
            m_stats.vidcap_bytessent += ret;
            TTASSERT(!m_def_acceptors.empty());
            break;
        case PACKET_KIND_VIDEO_CRYPT :
            m_stats.vidcap_bytessent += ret;
#if defined(ENABLE_ENCRYPTION)
            TTASSERT(!m_crypt_acceptors.empty());
#endif
            break;
        case PACKET_KIND_MEDIAFILE_AUDIO :
        case PACKET_KIND_MEDIAFILE_VIDEO :
            m_stats.mediafile_bytessent += ret;
            TTASSERT(!m_def_acceptors.empty());
            break;
        case PACKET_KIND_MEDIAFILE_AUDIO_CRYPT :
        case PACKET_KIND_MEDIAFILE_VIDEO_CRYPT :
            m_stats.mediafile_bytessent += ret;
#if defined(ENABLE_ENCRYPTION)
            TTASSERT(!m_crypt_acceptors.empty());
#endif
            break;
        case PACKET_KIND_DESKTOP :
        case PACKET_KIND_DESKTOP_ACK :
        case PACKET_KIND_DESKTOP_NAK :
        case PACKET_KIND_DESKTOPCURSOR :
        case PACKET_KIND_DESKTOPINPUT :
        case PACKET_KIND_DESKTOPINPUT_ACK :
            m_stats.desktop_bytessent += ret;
            TTASSERT(!m_def_acceptors.empty());
            break;
        case PACKET_KIND_DESKTOP_CRYPT :
        case PACKET_KIND_DESKTOP_ACK_CRYPT :
        case PACKET_KIND_DESKTOP_NAK_CRYPT :
        case PACKET_KIND_DESKTOPCURSOR_CRYPT :
        case PACKET_KIND_DESKTOPINPUT_CRYPT :
        case PACKET_KIND_DESKTOPINPUT_ACK_CRYPT :
            m_stats.desktop_bytessent += ret;
#if defined(ENABLE_ENCRYPTION)
            TTASSERT(!m_crypt_acceptors.empty());
#endif
            break;
        default:
            MYTRACE(ACE_TEXT("Unknown packet sent %d\n"), packet.GetKind());
            break;
        }
    }

    return (int)sent;
}

void ServerNode::ReceivedPacket(PacketHandler* ph, const char* packet_data,
                                int packet_size, const ACE_INET_Addr& remoteaddr)
{
    GUARD_OBJ(this, Lock());

    m_stats.packets_received++;
    
    FieldPacket const packet(packet_data, packet_size);

    if ((m_properties.rxloss != 0) && ((m_stats.packets_received % m_properties.rxloss) == 0))
    {
        MYTRACE(ACE_TEXT("Simulated RX dropped packet. Kind %d\n"), int(packet.GetKind()));
        return;
    }
    
    m_stats.total_bytesreceived += packet_size;
//     MYTRACE(ACE_TEXT("Packet %d size %d\n"), packet.GetKind(), packet_size);
    if(!packet.ValidatePacket())
    {
        MYTRACE(ACE_TEXT("Received invalid packet from %s\n"), 
                InetAddrToString(remoteaddr).c_str());
        return;
    }

    auto packetkind = packet.GetKind();

    serveruser_t const user = GetUser(packet.GetSrcUserID(), nullptr, packetkind != PACKET_KIND_HELLO && packetkind != PACKET_KIND_KEEPALIVE);
    if (!user)
        return;

    //only allow packet to pass through if it's from the initial IP-address
    if(!remoteaddr.is_ip_equal(user->GetUdpAddress()) && 
       !user->GetUdpAddress().is_any())
    {
        MYTRACE(ACE_TEXT("User #%d sent UDP packet from invalid IP-address %s. Should be %s\n"),
                user->GetUserID(), InetAddrToString(remoteaddr).c_str(), 
                InetAddrToString(user->GetUdpAddress()).c_str());
        return;
    }

    ACE_INET_Addr const localaddr = ph->GetLocalAddr();
    switch (packetkind)
    {
    case PACKET_KIND_HELLO :
        ReceivedHelloPacket(*user, HelloPacket(packet_data, packet_size),
                            remoteaddr, localaddr);
        break;
    case PACKET_KIND_KEEPALIVE :
        ReceivedKeepAlivePacket(*user, KeepAlivePacket(packet_data, packet_size),
                                remoteaddr, localaddr);
        break;
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_VOICE_CRYPT :
        if((user->GetUserRights() & USERRIGHT_TRANSMIT_VOICE) != 0u)
            ReceivedVoicePacket(*user, CryptVoicePacket(packet_data, packet_size), 
                                remoteaddr, localaddr);
        m_stats.voice_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_VOICE :
        if((user->GetUserRights() & USERRIGHT_TRANSMIT_VOICE) != 0u)
            ReceivedVoicePacket(*user, VoicePacket(packet_data, packet_size), 
                                remoteaddr, localaddr);
        m_stats.voice_bytesreceived += packet_size;
        break;
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_MEDIAFILE_AUDIO_CRYPT :
        if((user->GetUserRights() & USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO) != 0u)
            ReceivedAudioFilePacket(*user, CryptAudioFilePacket(packet_data, packet_size), 
                                    remoteaddr, localaddr);
        m_stats.mediafile_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_MEDIAFILE_AUDIO :
        if((user->GetUserRights() & USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO) != 0u)
            ReceivedAudioFilePacket(*user, AudioFilePacket(packet_data, packet_size), 
                                    remoteaddr, localaddr);
        m_stats.mediafile_bytesreceived += packet_size;
        break;
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_VIDEO_CRYPT :
        if((user->GetUserRights() & USERRIGHT_TRANSMIT_VIDEOCAPTURE) != 0u)
            ReceivedVideoCapturePacket(*user, CryptVideoCapturePacket(packet_data, packet_size), 
                                       remoteaddr, localaddr);
        m_stats.vidcap_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_VIDEO :
        if((user->GetUserRights() & USERRIGHT_TRANSMIT_VIDEOCAPTURE) != 0u)
            ReceivedVideoCapturePacket(*user, VideoCapturePacket(packet_data, packet_size), 
                                       remoteaddr, localaddr);
        m_stats.vidcap_bytesreceived += packet_size;
        break;
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_MEDIAFILE_VIDEO_CRYPT :
        if((user->GetUserRights() & USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO) != 0u)
            ReceivedVideoFilePacket(*user, CryptVideoFilePacket(packet_data, packet_size), 
                                    remoteaddr, localaddr);
        m_stats.mediafile_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_MEDIAFILE_VIDEO :
        if((user->GetUserRights() & USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO) != 0u)
            ReceivedVideoFilePacket(*user, VideoFilePacket(packet_data, packet_size), 
                                    remoteaddr, localaddr);
        m_stats.mediafile_bytesreceived += packet_size;
        break;
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_DESKTOP_CRYPT :
        if((user->GetUserRights() & USERRIGHT_TRANSMIT_DESKTOP) != 0u)
            ReceivedDesktopPacket(*user,
                                  CryptDesktopPacket(packet_data, packet_size), 
                                  remoteaddr, localaddr);
        m_stats.desktop_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_DESKTOP :
        if((user->GetUserRights() & USERRIGHT_TRANSMIT_DESKTOP) != 0u)
            ReceivedDesktopPacket(*user,
                                  DesktopPacket(packet_data, packet_size), 
                                  remoteaddr, localaddr);
        m_stats.desktop_bytesreceived += packet_size;
        break;
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_DESKTOP_ACK_CRYPT :
        ReceivedDesktopAckPacket(*user,
                                 CryptDesktopAckPacket(packet_data, packet_size), 
                                 remoteaddr, localaddr);
        m_stats.desktop_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_DESKTOP_ACK :
        ReceivedDesktopAckPacket(*user,
                                 DesktopAckPacket(packet_data, packet_size), 
                                 remoteaddr, localaddr);
        m_stats.desktop_bytesreceived += packet_size;
        break;
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_DESKTOP_NAK_CRYPT :
        ReceivedDesktopNakPacket(*user,
                                 CryptDesktopNakPacket(packet_data, packet_size), 
                                 remoteaddr, localaddr);
        m_stats.desktop_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_DESKTOP_NAK :
        ReceivedDesktopNakPacket(*user,
                                 DesktopNakPacket(packet_data, packet_size), 
                                 remoteaddr, localaddr);
        m_stats.desktop_bytesreceived += packet_size;
        break;
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_DESKTOPCURSOR_CRYPT :
        ReceivedDesktopCursorPacket(*user,
                                    CryptDesktopCursorPacket(packet_data, packet_size),
                                    remoteaddr, localaddr);
        m_stats.desktop_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_DESKTOPCURSOR :
        ReceivedDesktopCursorPacket(*user,
                                    DesktopCursorPacket(packet_data, packet_size),
                                    remoteaddr, localaddr);
        m_stats.desktop_bytesreceived += packet_size;
        break;
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_DESKTOPINPUT_CRYPT :
        if((user->GetUserRights() & USERRIGHT_TRANSMIT_DESKTOPINPUT) != 0u)
            ReceivedDesktopInputPacket(*user,
                                       CryptDesktopInputPacket(packet_data, packet_size),
                                       remoteaddr, localaddr);
        m_stats.desktop_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_DESKTOPINPUT :
        if((user->GetUserRights() & USERRIGHT_TRANSMIT_DESKTOPINPUT) != 0u)
            ReceivedDesktopInputPacket(*user,
                                       DesktopInputPacket(packet_data, packet_size),
                                       remoteaddr, localaddr);
        m_stats.desktop_bytesreceived += packet_size;
        break;
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_DESKTOPINPUT_ACK_CRYPT :
        ReceivedDesktopInputAckPacket(*user,
                                      CryptDesktopInputAckPacket(packet_data, packet_size),
                                      remoteaddr, localaddr);
        m_stats.desktop_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_DESKTOPINPUT_ACK :
        ReceivedDesktopInputAckPacket(*user,
                                      DesktopInputAckPacket(packet_data, packet_size),
                                      remoteaddr, localaddr);
        m_stats.desktop_bytesreceived += packet_size;
        break;
    default :
        MYTRACE(ACE_TEXT("Received an unknown packet %d from #%d\n"),
                (int)packet.GetKind(), packet.GetSrcUserID());
        break;
    }
}

void ServerNode::ReceivedHelloPacket(ServerUser& user, 
                                     const HelloPacket& packet, 
                                     const ACE_INET_Addr& remoteaddr,
                                     const ACE_INET_Addr& localaddr)
{
    ASSERT_SERVERNODE_LOCKED(this);

    int const userid = packet.GetSrcUserID();
    int const version = packet.GetProtocol();

    //set client properties
    if(user.GetUdpAddress() != remoteaddr && user.GetUdpAddress() != ACE_INET_Addr())
        m_updUserIPs.insert(user.GetUserID());

    user.SetUdpAddress(remoteaddr, localaddr);
    user.SetPacketProtocol(version);

    //send acknowledge packet
    HelloPacket const ackpacket((uint16_t)0, packet.GetTime());
    SendPacket(ackpacket, user);
}

void ServerNode::ReceivedKeepAlivePacket(ServerUser& user, 
                                         const KeepAlivePacket& packet, 
                                         const ACE_INET_Addr& remoteaddr,
                                         const ACE_INET_Addr& localaddr)
{
    ASSERT_SERVERNODE_LOCKED(this);

    int const userid = packet.GetSrcUserID();

    //check if it's a MTU query keep alive packet
    int buffers = 0;
    const iovec* v = packet.GetPacket(buffers);
    KeepAlivePacket const ka_pkt(v, buffers);
    uint16_t const payload_data_size = ka_pkt.GetPayloadSize();
    bool is_set = false;
    if(payload_data_size > 0 && 
       (W32_GEQ(packet.GetTime(), user.GetLastTimeStamp(&is_set)) || !is_set))
    {
        user.SetMaxDataChunkSize(payload_data_size);
        user.SetMaxPayloadSize(payload_data_size + FIELDHEADER_PAYLOAD);
        MYTRACE(ACE_TEXT("Updated #%d %s to payload size: %d\n"), 
                user.GetUserID(), user.GetNickname().c_str(), 
                payload_data_size);
    }

    KeepAlivePacket const reply((uint16_t)0, packet.GetTime());
    if(remoteaddr != user.GetUdpAddress())
    {
        user.SetUdpAddress(remoteaddr, localaddr);
        m_updUserIPs.insert(user.GetUserID());
    }
    SendPacket(reply, user);
    //reset keep alive
    user.SetLastKeepAlive(0);
}

serverchannel_t ServerNode::GetPacketChannel(ServerUser& user, 
                                             const FieldPacket& packet,
                                             const ACE_INET_Addr& remoteaddr,
                                             const ACE_INET_Addr& localaddr)
{
    ASSERT_SERVERNODE_LOCKED(this);

    uint16_t const chanid = packet.GetChannel();
    if(chanid == 0u)
        return {};

    serverchannel_t chan = user.GetChannel();
    if(!chan || chan->GetChannelID() != chanid)
    {
        //only admins can stream outside their channels
        if((user.GetUserType() & USERTYPE_ADMIN) == 0)
            return {};

        //this branch will only happen if a user is streaming outside his channel
        TTASSERT(m_rootchannel.get());
        chan = GetChannel(chanid);
        if(!chan)
            return {};
    }

    //update IP if changed
    if(remoteaddr != user.GetUdpAddress())
    {
        user.SetUdpAddress(remoteaddr, localaddr);
        m_updUserIPs.insert(user.GetUserID());
    }

    //update user's timestamp
    user.UpdateLastTimeStamp((PacketKind)packet.GetKind(), packet.GetTime());

    return chan;
}

ServerChannel::users_t ServerNode::GetPacketDestinations(const ServerUser& user,
                                                         const ServerChannel& channel,
                                                         const FieldPacket& packet,
                                                         Subscriptions subscrip_check,
                                                         Subscriptions intercept_check)
{
    ASSERT_SERVERNODE_LOCKED(this);

    ACE_UINT8 const pp_min = TEAMTALK_DEFAULT_PACKET_PROTOCOL;
    switch(subscrip_check)
    {
    case SUBSCRIBE_VOICE :
        break;
    case SUBSCRIBE_VIDEOCAPTURE :
        break;
    case SUBSCRIBE_DESKTOP :
        break;
    case SUBSCRIBE_DESKTOPINPUT :
        break;
    case SUBSCRIBE_MEDIAFILE :
        break;
    default:
        TTASSERT(0); //unknown min packet protocol
    }

    ServerChannel::users_t result;
    uint16_t const dest_userid = packet.GetDestUserID();
    int const fromuserid = user.GetUserID();

    if (dest_userid != 0u) //the packet is only for certain users
    {
        for (const auto &u : channel.GetUsers())
        {
            if (u->GetUserID() == dest_userid &&
                ((u->GetSubscriptions(user) & subscrip_check) != 0u) &&
                u->GetPacketProtocol() >= pp_min)
            {
                result.push_back(u);
            }
        }

        //admins can also subscribe outside their channels
        for (const auto& au : GetAdministrators())
        {
            if (((au->GetSubscriptions(user) & intercept_check) != 0u) &&
                !channel.UserExists(au->GetUserID()) &&
                au->GetPacketProtocol() >= pp_min)
            {
                result.push_back(au);
            }
        }
    }
    else if (packet.GetChannel() != 0u)
    {
        if (((channel.GetChannelType() & CHANNEL_OPERATOR_RECVONLY) != 0u) &&
            !channel.IsOperator(fromuserid) && 
            (user.GetUserType() & USERTYPE_ADMIN) == 0)
        {
            //only operators and admins will receive from default users
            //in channel type CHANNEL_OPERATOR_RECVONLY
            for (const auto& u : channel.GetUsers())
            {
                if ((channel.IsOperator(u->GetUserID()) ||
                     ((u->GetUserType() & USERTYPE_ADMIN) != 0u)) &&
                    ((u->GetSubscriptions(user) & subscrip_check) != 0u) &&
                    u->GetPacketProtocol() >= pp_min)
                {
                    result.push_back(u);
                }
            }
        }
        else
        {
            //forward to all users in same channel
            for (const auto &u : channel.GetUsers())
            {
                if (((u->GetSubscriptions(user) & subscrip_check) != 0u) &&
                    u->GetPacketProtocol() >= pp_min)
                {
                    result.push_back(u);
                }
            }
        }

        //admins can also subscribe outside their channels
        for (const auto &au : GetAdministrators())
        {
            if (((au->GetSubscriptions(user) & intercept_check) != 0u) &&
                !channel.UserExists(au->GetUserID()) &&
                au->GetPacketProtocol() >= pp_min)
            {
                result.push_back(au);
            }
        }
    }
    return result;
}

void ServerNode::ReceivedVoicePacket(ServerUser& user, 
                                     const FieldPacket& packet, 
                                     const ACE_INET_Addr& remoteaddr,
                                     const ACE_INET_Addr& localaddr)
{
    ASSERT_SERVERNODE_LOCKED(this);

    serverchannel_t const tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;
    uint8_t streamid = 0;
    switch (packet.GetKind())
    {
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_VOICE_CRYPT :
    {
        auto p = CryptVoicePacket(packet).Decrypt(chan.GetEncryptKey());
        if (p)
            streamid = p->GetStreamID();
        break;
    }
#endif
    case PACKET_KIND_VOICE :
        streamid = VoicePacket(packet).GetStreamID();
        break;
    default :
        assert(packet.GetKind() == PACKET_KIND_VOICE);
        break;
    }

    bool modified = false;
    bool const tx_ok = chan.CanTransmit(user.GetUserID(), STREAMTYPE_VOICE, streamid, &modified);

    if (((chan.GetChannelType() & CHANNEL_SOLO_TRANSMIT) != 0u) && modified)
    {
        UpdateChannel(chan, chan.GetUsers(), &user);
    }

    if(!tx_ok)
        return;

    if (((m_properties.logevents & SERVERLOGEVENT_USER_NEW_STREAM) != 0u) &&
        user.UpdateActiveStream(STREAMTYPE_VOICE, streamid) != streamid)
    {
        m_srvguard->OnUserUpdateStream(user, chan, STREAMTYPE_VOICE, streamid);
    }

    ServerChannel::users_t const users = GetPacketDestinations(user, chan, packet,
                                                         SUBSCRIBE_VOICE,
                                                         SUBSCRIBE_INTERCEPT_VOICE);

    SendPackets(packet, users);
}

void ServerNode::ReceivedAudioFilePacket(ServerUser& user, 
                                         const FieldPacket& packet, 
                                         const ACE_INET_Addr& remoteaddr,
                                         const ACE_INET_Addr& localaddr)
{
    ASSERT_SERVERNODE_LOCKED(this);

    serverchannel_t const tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;
    uint8_t streamid = 0;
    
    switch (packet.GetKind())
    {
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_MEDIAFILE_AUDIO_CRYPT :
    {
        auto p = CryptAudioFilePacket(packet).Decrypt(chan.GetEncryptKey());
        if (p)
            streamid = p->GetStreamID();
        break;
    }
#endif
    case PACKET_KIND_MEDIAFILE_AUDIO :
        streamid = AudioFilePacket(packet).GetStreamID();
        break;
    default :
        assert(packet.GetKind() == PACKET_KIND_MEDIAFILE_AUDIO);
        break;
    }

    bool modified = false;
    bool const tx_ok = chan.CanTransmit(user.GetUserID(), STREAMTYPE_MEDIAFILE, streamid, &modified);

    if (((chan.GetChannelType() & CHANNEL_SOLO_TRANSMIT) != 0u) && modified)
    {
        UpdateChannel(chan, chan.GetUsers(), &user);
    }

    if(!tx_ok)
        return;

    if (((m_properties.logevents & SERVERLOGEVENT_USER_NEW_STREAM) != 0u) &&
        user.UpdateActiveStream(STREAMTYPE_MEDIAFILE_AUDIO, streamid) != streamid)
    {
        m_srvguard->OnUserUpdateStream(user, chan, STREAMTYPE_MEDIAFILE_AUDIO, streamid);
    }

    ServerChannel::users_t const users = GetPacketDestinations(user, chan, packet, SUBSCRIBE_MEDIAFILE,
                                                         SUBSCRIBE_INTERCEPT_MEDIAFILE);

    SendPackets(packet, users);
}

void ServerNode::ReceivedVideoCapturePacket(ServerUser& user, 
                                            const FieldPacket& packet, 
                                            const ACE_INET_Addr& remoteaddr,
                                            const ACE_INET_Addr& localaddr)
{
    ASSERT_SERVERNODE_LOCKED(this);

    serverchannel_t const tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;
    uint8_t streamid = 0;
    
    switch (packet.GetKind())
    {
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_VIDEO_CRYPT :
    {
        auto p = CryptVideoCapturePacket(packet).Decrypt(chan.GetEncryptKey());
        if (p)
            streamid = p->GetStreamID();
        break;
    }
#endif
    case PACKET_KIND_VIDEO :
        streamid = VideoCapturePacket(packet).GetStreamID();
        break;
    default :
        assert(packet.GetKind() == PACKET_KIND_VIDEO_CRYPT);
    }

    if(!chan.CanTransmit(user.GetUserID(), STREAMTYPE_VIDEOCAPTURE, streamid, nullptr))
        return;

    if (((m_properties.logevents & SERVERLOGEVENT_USER_NEW_STREAM) != 0u) &&
        user.UpdateActiveStream(STREAMTYPE_VIDEOCAPTURE, streamid) != streamid)
    {
        m_srvguard->OnUserUpdateStream(user, chan, STREAMTYPE_VIDEOCAPTURE, streamid);
    }


    ServerChannel::users_t const users = GetPacketDestinations(user, chan, packet, SUBSCRIBE_VIDEOCAPTURE,
                                                         SUBSCRIBE_INTERCEPT_VIDEOCAPTURE);
    
    SendPackets(packet, users);
}


void ServerNode::ReceivedVideoFilePacket(ServerUser& user, 
                                         const FieldPacket& packet, 
                                         const ACE_INET_Addr& remoteaddr,
                                         const ACE_INET_Addr& localaddr)
{
    ASSERT_SERVERNODE_LOCKED(this);

    serverchannel_t const tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    // MYTRACE("Received video packet %d fragment %d/%d from #%d\n",
    //         packet.GetPacketNo(), packet.GetFragmentNo(), packet.GetFragmentCount(), user.GetUserID());

    ServerChannel& chan = *tmp_chan;
    uint8_t streamid = 0;

    switch (packet.GetKind())
    {
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_MEDIAFILE_VIDEO_CRYPT :
    {
        auto p = CryptVideoFilePacket(packet).Decrypt(chan.GetEncryptKey());
        if (p)
            streamid = p->GetStreamID();
        break;
    }
#endif
    case PACKET_KIND_MEDIAFILE_VIDEO :
        streamid = VideoFilePacket(packet).GetStreamID();
        break;
    default :
        assert(packet.GetKind() == PACKET_KIND_MEDIAFILE_VIDEO);
        break;
    }
    
    bool modified = false;
    bool const tx_ok = chan.CanTransmit(user.GetUserID(), STREAMTYPE_MEDIAFILE, streamid, &modified);
    
    if (((chan.GetChannelType() & CHANNEL_SOLO_TRANSMIT) != 0u) && modified)
    {
        UpdateChannel(chan, chan.GetUsers(), &user);
    }

    if(!tx_ok)
        return;

    if (((m_properties.logevents & SERVERLOGEVENT_USER_NEW_STREAM) != 0u) &&
        user.UpdateActiveStream(STREAMTYPE_MEDIAFILE_VIDEO, streamid) != streamid)
    {
        m_srvguard->OnUserUpdateStream(user, chan, STREAMTYPE_MEDIAFILE_VIDEO, streamid);
    }

    ServerChannel::users_t const users = GetPacketDestinations(user, chan, packet, SUBSCRIBE_MEDIAFILE,
                                                         SUBSCRIBE_INTERCEPT_MEDIAFILE);

    SendPackets(packet, users);
}

#if defined(ENABLE_ENCRYPTION)
void ServerNode::ReceivedDesktopPacket(ServerUser& user, 
                                       const CryptDesktopPacket& crypt_pkt, 
                                       const ACE_INET_Addr& remoteaddr,
                                       const ACE_INET_Addr& localaddr)
{
    serverchannel_t const tmp_chan = GetPacketChannel(user, crypt_pkt, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel const& chan = *tmp_chan;

    auto decrypt_pkt = crypt_pkt.Decrypt(chan.GetEncryptKey());
    if(!decrypt_pkt)
        return;
    
    ReceivedDesktopPacket(user, *decrypt_pkt, remoteaddr, localaddr);
}
#endif

void ServerNode::ReceivedDesktopPacket(ServerUser& user, 
                                       const DesktopPacket& packet, 
                                       const ACE_INET_Addr& remoteaddr,
                                       const ACE_INET_Addr& localaddr)
{
    ASSERT_SERVERNODE_LOCKED(this);

    serverchannel_t const tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;

    if (!chan.CanTransmit(user.GetUserID(), STREAMTYPE_DESKTOP, packet.GetSessionID(), nullptr))
       return;

    if (((m_properties.logevents & SERVERLOGEVENT_USER_NEW_STREAM) != 0u) &&
        user.UpdateActiveStream(STREAMTYPE_DESKTOP, packet.GetSessionID()) != packet.GetSessionID())
    {
        m_srvguard->OnUserUpdateStream(user, chan, STREAMTYPE_DESKTOP, packet.GetSessionID());
    }

    uint8_t prev_session_id = 0;
    uint32_t prev_update_id = 0;
    bool prev_session_ready = false;
    if (user.GetDesktopSession())
    {
        prev_session_id = user.GetDesktopSession()->GetSessionID();
        prev_update_id = user.GetDesktopSession()->GetCurrentDesktopTime();
        prev_session_ready = user.GetDesktopSession()->IsReady();
    }

    bool const add_pkt = user.AddDesktopPacket(packet);
    MYTRACE_COND(!add_pkt, ACE_TEXT("Failed to DesktopPacket %d to session %d:%u\n"),
                 packet.GetPacketIndex(), packet.GetSessionID(), packet.GetTime());

    //start delayed ack
    int const userid = user.GetUserID();
    if(!m_desktop_ack_timers.contains(userid))
    {
        ACE_Time_Value const interval(0, 2000); //delayed ack time
        TimerHandler* th = nullptr;
        ACE_NEW(th, TimerHandler(*this, TIMERSRV_DESKTOPACKPACKET_ID, userid));
        long const timerid = m_timer_reactor->schedule_timer(th, nullptr, interval, interval);
        if(timerid >= 0)
            m_desktop_ack_timers[userid] = timerid;
    }

    //don't forward the desktop packet if no session is ready
    if (!user.GetDesktopSession())
        return;

    DesktopCache const& desktop = *user.GetDesktopSession();
    uint8_t const session_id = desktop.GetSessionID();
    uint32_t const update_time = desktop.GetCurrentDesktopTime();
    bool const session_ready = desktop.IsReady();

    //don't forward a new session or update until it's complete
    if((prev_session_id == session_id && prev_update_id == update_time &&
        prev_session_ready == session_ready) || !session_ready)
       return;
    //if(desktop.GetMissingPacketsCount(packet.GetTime()) != 0)
    //    return; //don't continue unless this packet completed the update

    //if this doesn't evaluate to true it means we're initiating new
    //desktop transmitters from an old desktop packet
    TTASSERT(!prev_session_id ||
             SESSIONID_GEQ(session_id, prev_session_id) &&
             W32_GEQ(update_time, prev_update_id));

    MYTRACE(ACE_TEXT("Desktop update %d:%u completed for #%d\n"), 
            session_id, update_time, userid);

    //don't process packet if it's not for the users current channel.
    //If it should be possible to send a desktop to a user outside
    //'user' current channel then this restriction must be removed.
    serverchannel_t const user_chan = user.GetChannel();
    if(!user_chan || user_chan->GetChannelID() != chan.GetChannelID())
    {
        MYTRACE(ACE_TEXT("Ignored desktop packet from #%d to %s\n"),
                user.GetUserID(), chan.GetChannelPath().c_str());
        return;
    }

    std::vector<ACE_INET_Addr> const addrs;
    ServerChannel::users_t users = GetPacketDestinations(user, chan, packet, SUBSCRIBE_DESKTOP,
                                                         SUBSCRIBE_INTERCEPT_DESKTOP);

    for(auto & ui : users)
    {
        desktop_transmitter_t const dtx = ui->GetDesktopTransmitter(user.GetUserID());
        if (dtx)
        {
            timer_userdata tm_data;
            tm_data.dest_userid = ui->GetUserID();
            tm_data.src_userid = user.GetUserID();

            //check for existing RTX timer
            TTASSERT(dtx->Done() || m_desktop_rtx_timers.find(tm_data.userdata) != m_desktop_rtx_timers.end());

            //close existing transmitters if new session is started
            if(dtx->GetSessionID() != desktop.GetSessionID())
                StopDesktopTransmitter(user, *ui, false);
            else if(!dtx->Done())
                continue; //skip new transmitter if one is already active
        }
        MYTRACE(ACE_TEXT("Starting new DTX for #%d\n"), ui->GetUserID());
        //start or resume transmitter
        StartDesktopTransmitter(user, *ui, chan);
    }
}

#if defined(ENABLE_ENCRYPTION)
void ServerNode::ReceivedDesktopAckPacket(ServerUser& user, 
                                          const CryptDesktopAckPacket& crypt_pkt, 
                                          const ACE_INET_Addr& remoteaddr,
                                          const ACE_INET_Addr& localaddr)
{
    serverchannel_t const tmp_chan = GetPacketChannel(user, crypt_pkt, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel const& chan = *tmp_chan;

    auto decrypt_pkt = crypt_pkt.Decrypt(chan.GetEncryptKey());
    if(!decrypt_pkt)
        return;

    ReceivedDesktopAckPacket(user, *decrypt_pkt, remoteaddr, localaddr);
}
#endif

void ServerNode::ReceivedDesktopAckPacket(ServerUser& user, 
                                          const DesktopAckPacket& packet, 
                                          const ACE_INET_Addr& remoteaddr,
                                          const ACE_INET_Addr& localaddr)
{
    ASSERT_SERVERNODE_LOCKED(this);

    serverchannel_t const tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel const& chan = *tmp_chan;

    uint16_t owner_userid = 0;
    uint8_t session_id = 0; 
    uint32_t upd_time = 0;

    if(!packet.GetSessionInfo(&owner_userid, &session_id, &upd_time))
        return;

    desktop_transmitter_t const dtx = user.GetDesktopTransmitter(owner_userid);
    if (!dtx)
    {
        //Check for ACK to a NAK (closed desktop session with timer active)
        ClosedDesktopSession old_session;
        if(user.GetClosedDesktopSession(owner_userid, old_session) && 
           old_session.session_id == session_id &&
           old_session.update_id == upd_time)
        {
            user.ClosePendingDesktopTerminate(owner_userid);
            return;
        }
    }

    if (!dtx || dtx->GetSessionID() != session_id)
    {
        DesktopNakPacket nak_pkt(owner_userid, upd_time, session_id);
        nak_pkt.SetChannel(chan.GetChannelID());
#if defined(ENABLE_ENCRYPTION)
        if(!m_crypt_acceptors.empty())
        {
            CryptDesktopNakPacket const crypt_pkt(nak_pkt, chan.GetEncryptKey());
            SendPacket(crypt_pkt, remoteaddr, localaddr);
        }
        else
        {
            SendPacket(nak_pkt, remoteaddr, localaddr);
        }
#else
        SendPacket(nak_pkt, remoteaddr, localaddr);
#endif
        MYTRACE(ACE_TEXT("Sending NAK to #%d for user #%d's session %d\n"),
                user.GetUserID(), owner_userid, session_id);
        return;
    }

    //if it's an ACK to an old update (DTX) just ignore it
    if(dtx->GetUpdateID() != upd_time)
        return;

    if(!dtx->ProcessDesktopAckPacket(packet))
        return;

    desktoppackets_t tx_packets;
    dtx->GetDupAckLostDesktopPackets(tx_packets);
    dtx->GetNextDesktopPackets(tx_packets);

    auto dpi = tx_packets.begin();
    while(dpi != tx_packets.end())
    {
#if defined(ENABLE_ENCRYPTION)
        if(!m_crypt_acceptors.empty())
        {
            CryptDesktopPacket const crypt_pkt(*(*dpi), chan.GetEncryptKey());
            if(SendPacket(crypt_pkt, user) <= 0)
                break;
        }
        else
#endif
        {
            if(SendPacket(*(*dpi), user) <= 0)
                break;
        }

        dpi++;
    }

    if(dtx->Done())
    {
        timer_userdata tm_data;
        tm_data.dest_userid = user.GetUserID();
        tm_data.src_userid = owner_userid;

        auto const ii = m_desktop_rtx_timers.find(tm_data.userdata);
        if(ii != m_desktop_rtx_timers.end())
        {
            m_timer_reactor->cancel_timer(ii->second, nullptr, 0);
            m_desktop_rtx_timers.erase(ii);
        }
        MYTRACE(ACE_TEXT("Desktop TX update %d:%u completed for #%d\n"),
            dtx->GetSessionID(), dtx->GetUpdateID(), user.GetUserID());

        //check if a new update is ready
        serveruser_t const src_user = GetUser(owner_userid, &user);
        if(src_user)
        {
            desktop_cache_t const desktop = src_user->GetDesktopSession();
            if (desktop)
                StartDesktopTransmitter(*src_user, user, chan);
        }
    }
}

#if defined(ENABLE_ENCRYPTION)
void ServerNode::ReceivedDesktopNakPacket(ServerUser& user, 
                                          const CryptDesktopNakPacket& crypt_pkt, 
                                          const ACE_INET_Addr& remoteaddr,
                                          const ACE_INET_Addr& localaddr)
{
    serverchannel_t const tmp_chan = GetPacketChannel(user, crypt_pkt, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel const& chan = *tmp_chan;

    auto decrypt_pkt = crypt_pkt.Decrypt(chan.GetEncryptKey());
    if(!decrypt_pkt)
        return;

    ReceivedDesktopNakPacket(user, *decrypt_pkt, remoteaddr, localaddr);
}
#endif

void ServerNode::ReceivedDesktopNakPacket(ServerUser& user, 
                                          const DesktopNakPacket& packet, 
                                          const ACE_INET_Addr& remoteaddr,
                                          const ACE_INET_Addr& localaddr)
{
    serverchannel_t const tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel const& chan = *tmp_chan;

    desktop_cache_t const desktop = user.GetDesktopSession();
    if (desktop && desktop->GetSessionID() == packet.GetSessionID())
    {
        user.CloseDesktopSession();
        MYTRACE(ACE_TEXT("Close desktop session %d for user #%d\n"),
                packet.GetSessionID(), user.GetUserID());
    }
    std::vector<ACE_INET_Addr> const addrs;
    ServerChannel::users_t users = GetPacketDestinations(user, chan, packet, SUBSCRIBE_DESKTOP,
                                                         SUBSCRIBE_INTERCEPT_DESKTOP);

    for(auto & ui : users)
        StopDesktopTransmitter(user, *ui, true);

    DesktopAckPacket ack_pkt(0, GETTIMESTAMP(), user.GetUserID(), 
                             packet.GetSessionID(), packet.GetTime(),
                             std::set<uint16_t>(), packet_range_t());
    ack_pkt.SetChannel(chan.GetChannelID());
#if defined(ENABLE_ENCRYPTION)
    if(!m_crypt_acceptors.empty())
    {
        CryptDesktopAckPacket const crypt_ackpkt(ack_pkt, chan.GetEncryptKey());
        SendPacket(crypt_ackpkt, user);
    }
    else
#endif
    {
        SendPacket(ack_pkt, user);
    }
}

#if defined(ENABLE_ENCRYPTION)
void ServerNode::ReceivedDesktopCursorPacket(ServerUser& user, 
                                             const CryptDesktopCursorPacket& crypt_pkt, 
                                             const ACE_INET_Addr& remoteaddr,
                                             const ACE_INET_Addr& localaddr)
{
    serverchannel_t const tmp_chan = GetPacketChannel(user, crypt_pkt, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel const& chan = *tmp_chan;

    auto decrypt_pkt = crypt_pkt.Decrypt(chan.GetEncryptKey());
    if(!decrypt_pkt)
        return;

    ReceivedDesktopCursorPacket(user, *decrypt_pkt, remoteaddr, localaddr);
}
#endif

void ServerNode::ReceivedDesktopCursorPacket(ServerUser& user, 
                                             const DesktopCursorPacket& packet, 
                                             const ACE_INET_Addr& remoteaddr,
                                             const ACE_INET_Addr& localaddr)
{
    serverchannel_t const tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;

    if(!chan.CanTransmit(user.GetUserID(), STREAMTYPE_DESKTOP, packet.GetSessionID(), nullptr))
       return;
    
#ifdef _DEBUG
    bool ok_tm = false;
    user.GetLastTimeStamp(&ok_tm);
    TTASSERT(ok_tm);
#endif

    //ignore cursor if it's not the current desktop session
    uint8_t session_id = 0;
    uint16_t dest_userid = 0;
    int16_t x;
    int16_t y;
    if(!packet.GetSessionCursor(&dest_userid, &session_id, &x, &y))
        return;

    desktop_cache_t const session = user.GetDesktopSession();
    if (!session || session->GetSessionID() != session_id)
        return;

    //throw away packet if a newer one has already arrived
    bool is_set = false;
    if(!W32_GEQ(packet.GetTime(),
                user.GetLastTimeStamp(packet, &is_set)) && is_set)
        return;

    if (((m_properties.logevents & SERVERLOGEVENT_USER_NEW_STREAM) != 0u) &&
        user.UpdateActiveStream(STREAMTYPE_DESKTOPINPUT, packet.GetSessionID()) != packet.GetSessionID())
    {
        m_srvguard->OnUserUpdateStream(user, chan, STREAMTYPE_DESKTOPINPUT, packet.GetSessionID());
    }

    ServerChannel::users_t const users = GetPacketDestinations(user, chan, packet, SUBSCRIBE_DESKTOP,
                                                         SUBSCRIBE_INTERCEPT_DESKTOP);

#if defined(ENABLE_ENCRYPTION)
    if(!m_crypt_acceptors.empty())
    {
        // copy crypt sections using copy constructor
        CryptDesktopCursorPacket const crypt_pkt(DesktopCursorPacket(packet), chan.GetEncryptKey());
        SendPackets(crypt_pkt, users);
    }
    else
#endif
    {
        SendPackets(packet, users);
    }
}

#if defined(ENABLE_ENCRYPTION)
void ServerNode::ReceivedDesktopInputPacket(ServerUser& user, 
                                            const CryptDesktopInputPacket& crypt_pkt, 
                                            const ACE_INET_Addr& remoteaddr,
                                            const ACE_INET_Addr& localaddr)
{
    serverchannel_t const tmp_chan = GetPacketChannel(user, crypt_pkt, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel const& chan = *tmp_chan;

    auto decrypt_pkt = crypt_pkt.Decrypt(chan.GetEncryptKey());
    if(!decrypt_pkt)
        return;

    ReceivedDesktopInputPacket(user, *decrypt_pkt, remoteaddr, localaddr);
}
#endif

void ServerNode::ReceivedDesktopInputPacket(ServerUser& user, 
                                            const DesktopInputPacket& packet, 
                                            const ACE_INET_Addr& remoteaddr,
                                            const ACE_INET_Addr& localaddr)
{
    serverchannel_t const tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel const& chan = *tmp_chan;

    serveruser_t destuser;

    uint16_t const dest_userid = packet.GetDestUserID();
    destuser = GetUser(dest_userid, &user);
    if(!destuser)
        return;
    
    //throw away desktop input if it's not the current session
    desktop_cache_t const session = destuser->GetDesktopSession();
    if (!session || session->GetSessionID() != packet.GetSessionID())
        return;

    if (((m_properties.logevents & SERVERLOGEVENT_USER_NEW_STREAM) != 0u) &&
        user.UpdateActiveStream(STREAMTYPE_DESKTOPINPUT, packet.GetSessionID()) != packet.GetSessionID())
    {
        m_srvguard->OnUserUpdateStream(user, chan, STREAMTYPE_DESKTOPINPUT, packet.GetSessionID());
    }

    ServerChannel::users_t const users = GetPacketDestinations(user, chan, packet,
                                                         SUBSCRIBE_DESKTOPINPUT,
                                                         SUBSCRIBE_NONE);

#if defined(ENABLE_ENCRYPTION)
    if(!m_crypt_acceptors.empty())
    {
        // create new input packet with encrypted sections
        CryptDesktopInputPacket const crypt_pkt(DesktopInputPacket(packet), chan.GetEncryptKey());
        SendPackets(crypt_pkt, users);
    }
    else
#endif
    {
        SendPackets(packet, users);
    }
}

#if defined(ENABLE_ENCRYPTION)
void ServerNode::ReceivedDesktopInputAckPacket(ServerUser& user, 
                                               const CryptDesktopInputAckPacket& crypt_pkt, 
                                               const ACE_INET_Addr& remoteaddr,
                                               const ACE_INET_Addr& localaddr)
{
    serverchannel_t const tmp_chan = GetPacketChannel(user, crypt_pkt, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel const& chan = *tmp_chan;

    auto decrypt_pkt = crypt_pkt.Decrypt(chan.GetEncryptKey());
    if(!decrypt_pkt)
        return;

    ReceivedDesktopInputAckPacket(user, *decrypt_pkt, remoteaddr, localaddr);
}
#endif

void ServerNode::ReceivedDesktopInputAckPacket(ServerUser& user, 
                                               const DesktopInputAckPacket& packet, 
                                               const ACE_INET_Addr& remoteaddr,
                                               const ACE_INET_Addr& localaddr)
{
    serverchannel_t const tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel const& chan = *tmp_chan;

    uint16_t const dest_userid = packet.GetDestUserID();
    serveruser_t const dest_user = GetUser(dest_userid, &user);
    if(dest_user)
    {
#if defined(ENABLE_ENCRYPTION)
        if(!m_crypt_acceptors.empty())
        {
            CryptDesktopInputAckPacket const crypt_pkt(DesktopInputAckPacket(packet), chan.GetEncryptKey());
            SendPacket(crypt_pkt, *dest_user);
        }
        else
#endif
        {
            SendPacket(packet, *dest_user);
        }
    }
}

void ServerNode::CheckKeepAlive()
{
    ASSERT_SERVERNODE_LOCKED(this);

    std::vector<serveruser_t> theDead;
    for(auto & m_mUser : m_mUsers)
    {
        if(m_mUser.second->GetLastKeepAlive() >= m_properties.usertimeout)
            theDead.push_back(m_mUser.second);
        else if(m_mUser.second->GetFileTransferID() == 0)
            m_mUser.second->SetLastKeepAlive( m_mUser.second->GetLastKeepAlive() + SERVER_KEEPALIVE_DELAY );
    }

    //disconnect the dead
    for(const auto & j : theDead)
    {
        //notify of dropped users (due to keepalive)
        if ((m_properties.logevents & SERVERLOGEVENT_USER_TIMEDOUT) != 0u)
        {
            m_srvguard->OnUserDropped(*j);
        }
        // SSL handler could be hanging in CryptStreamHandler::process_ssl()
        // therefore we have to forcefully delete the handler
        ACE_Event_Handler* h = RegisterStreamCallback(j->ResetStreamHandle());
        delete h;
    }
}

ErrorMsg ServerNode::UserLogin(int userid, const ACE_TString& username,
                               const ACE_TString& passwd)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr, false);
    TTASSERT(user);
    if(!user)
        return TT_CMDERR_USER_NOT_FOUND;

    ErrorMsg err;

    UserAccount useraccount;
    useraccount.username = username;
    useraccount.passwd = passwd;
    
    //don't hold lock during callback since it might be slow
    err = m_srvguard->AuthenticateUser(this, *user, useraccount);

    switch(err.errorno)
    {
    case TT_CMDERR_SUCCESS :
    {
        if (LoginsExceeded(*user))
            return TT_CMDERR_COMMAND_FLOOD;
        break;
    }
    case TT_CMDERR_SERVER_BANNED :
        if ((m_properties.logevents & SERVERLOGEVENT_USER_BANNED) != 0u)
        {
            m_srvguard->OnUserLoginBanned(*user);
        }
        return err; //banned from server
    case TT_SRVERR_COMMAND_SUSPEND :
        return err;
    default :
        if ((m_properties.logevents & SERVERLOGEVENT_USER_LOGINFAILED) != 0u)
        {
            m_srvguard->OnUserAuthFailed(*user, username);
        }

        IncLoginAttempt(*user);
        return err;
    }

    // usertype must be set
    if(useraccount.usertype == USERTYPE_NONE)
    {
        return ErrorMsg(TT_CMDERR_INVALID_ACCOUNT);
    }

    int const user_count = int(GetAuthorizedUsers(false).size());
    if(user_count+1 > m_properties.maxusers && 
       (useraccount.usertype & USERTYPE_ADMIN) == 0)
        return ErrorMsg(TT_CMDERR_MAX_SERVER_USERS_EXCEEDED); //user limit

    //check for double login
    if((useraccount.userrights & USERRIGHT_MULTI_LOGIN) == 0)
    {
        for (const auto &u : GetAuthorizedUsers())
        {
            TTASSERT(u != user);
            if (u->GetUsername() == username)
                UserKick(0, u->GetUserID(), 0, true);
        }
    }

    //check for max logins per ip-address (ignore admin users)
    if((m_properties.max_logins_per_ipaddr != 0) &&
       (useraccount.usertype & USERTYPE_ADMIN) == 0)
    {
        int logins = 1; //include self
        for (const auto &u : GetAuthorizedUsers())
        {
            if (u->GetIpAddress() == user->GetIpAddress())
                logins++;
        }
        if(logins > m_properties.max_logins_per_ipaddr)
            return ErrorMsg(TT_CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED);
    }

    //set user-account now meaning the user is not authorized
    user->SetUserAccount(useraccount);

    //store in admin cache
    if((user->GetUserType() & USERTYPE_ADMIN) != 0u)
        m_admins.push_back(user);

    //clear any wrong logins
    m_failedlogins.erase(user->GetIpAddress());

    //do connect accepted
    user->DoAccepted(useraccount);
    user->DoServerUpdate(m_properties);

    //forward all channels
    user->ForwardChannels(GetRootChannel(), IsEncrypted());

    //send all files to user if admin
    if((user->GetUserType() & USERTYPE_ADMIN) != 0u)
        user->ForwardFiles(GetRootChannel(), true);

    //notify other users of new user
    for (const auto& u : GetNotificationUsers(USERRIGHT_VIEW_ALL_USERS))
    {
        if(u->GetUserID() != userid)
            u->DoLoggedIn(*user);
    }

    //forward users if USERRIGHT_VIEW_ALL_USERS enabled
    if((user->GetUserRights() & USERRIGHT_VIEW_ALL_USERS) != 0u)
    {
        for (const auto &u : GetAuthorizedUsers())
            user->DoLoggedIn(*u);

        user->ForwardUsers(GetRootChannel(), true);
    }
    //register peak and users servered
    m_stats.userspeak = std::max(m_stats.userspeak, user_count+1);
    m_stats.usersservered++;

    //notify listener if any
    if ((m_properties.logevents & SERVERLOGEVENT_USER_LOGGEDIN) != 0u)
    {
        m_srvguard->OnUserLogin(*user);
    }

    // update last login
    if (IsAutoSaving())
    {
        auto err = m_srvguard->SaveConfiguration(*user, *this);
        if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u))
            m_srvguard->OnSaveConfiguration(user.get());
    }

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserLogout(int userid)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    if(!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    TTASSERT(user->IsAuthorized());
    if(!user->IsAuthorized())
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    // leave channel
    serverchannel_t const chan = user->GetChannel();
    if(chan)
    {
        ErrorMsg err = UserLeaveChannel(userid, chan->GetChannelID());
        if(err.errorno != TT_CMDERR_SUCCESS)
            return err;
    }

    // clear operator status in other channels
    std::set<int> chanids = m_rootchannel->RemoveOperator(userid, true);
    std::set<int>::iterator i;
    for(i=chanids.begin();i!=chanids.end();i++)
    {
        serverchannel_t const chan = GetChannel(*i);
        TTASSERT(chan);
        if(chan)
            UpdateChannel(chan, user.get());
    }

    // clear file transfers owned by user
    for (auto ift = m_filetransfers.begin();ift != m_filetransfers.end();)
    {
        if (ift->second.userid == userid)
            ift = m_filetransfers.erase(ift);
        else ++ift;
    }

    user->DoLoggedOut();
    user->SetUserAccount(UserAccount());

    //remove from admin cache
    for(size_t i=0;i<m_admins.size();i++)
    {
        if(m_admins[i]->GetUserID() == userid)
        {
            m_admins.erase(m_admins.begin()+i);
            break;
        }
    }

    //notify users of logout
    for (const auto& u : GetNotificationUsers(USERRIGHT_VIEW_ALL_USERS))
        u->DoLoggedOut(*user);

    if ((m_properties.logevents & SERVERLOGEVENT_USER_LOGGEDOUT) != 0u)
    {
        m_srvguard->OnUserLoggedOut(*user);
    }

    //reset important user info.
    user->SetUserAccount(UserAccount());

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserChangeNickname(int userid, const ACE_TString& newnick)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    if(!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    ErrorMsg err = m_srvguard->ChangeNickname(*user, newnick);
    if(err.Success())
    {
        user->SetNickname(newnick);
        return UserUpdate(userid);
    }
    return err;
}

ErrorMsg ServerNode::UserChangeStatus(int userid, int mode, const ACE_TString& status)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    if(!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    ErrorMsg err = m_srvguard->ChangeStatus(*user, mode, status);
    if(err.Success())
    {
        user->SetStatusMode(mode);
        user->SetStatusMessage(status);

        return UserUpdate(userid);
    }
    return err;
}

ErrorMsg ServerNode::UserUpdate(int userid)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    if(!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    ServerChannel::users_t const notifyusers = GetNotificationUsers(USERRIGHT_VIEW_ALL_USERS,
                                                              user->GetChannel());
    for (const auto& u : notifyusers)
        u->DoUpdateUser(*user);

    if ((m_properties.logevents & SERVERLOGEVENT_USER_UPDATED) != 0u)
    {
        m_srvguard->OnUserUpdated(*user);
    }

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserJoinChannel(int userid, const ChannelProp& chanprop)
{
    GUARD_OBJ(this, Lock());

    TTASSERT(GetRootChannel());

    serveruser_t user = GetUser(userid, nullptr);
    TTASSERT(user);

    bool makeop = false;

    if(!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t newchan = GetChannel(chanprop.channelid);
    if (!newchan)
    {
        // handle case where channel already exists but ID is unknown
        // to use, e.g. CHANNEL_HIDDEN.
        newchan = GetChannel(chanprop.parentid);
        if (newchan && (!chanprop.name.empty()))
            newchan = newchan->GetSubChannel(chanprop.name);
        else
            newchan.reset();
    }

    if (!newchan)//user is trying to create a new channel
    {
        // ensure user is allowed to create channels
        if((user->GetUserRights() & USERRIGHT_CREATE_TEMPORARY_CHANNEL) == 0 &&
           (user->GetUserRights() & USERRIGHT_MODIFY_CHANNELS) == 0)
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        // CHANNEL_PERMANENT and CHANNEL_HIDDEN require USERRIGHT_MODIFY_CHANNELS
        if ((user->GetUserRights() & USERRIGHT_MODIFY_CHANNELS) == 0 &&
            ((chanprop.chantype & (CHANNEL_PERMANENT | CHANNEL_HIDDEN)) != 0u))
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        // ensure user cannot create channel which is not direct subchannel of current
        // unless user has USERRIGHT_MODIFY_CHANNELS
        serverchannel_t const parent = GetChannel(chanprop.parentid);
        if (parent != m_rootchannel && user->GetChannel() != parent &&
            (user->GetUserRights() & USERRIGHT_MODIFY_CHANNELS) == 0)
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
     
        ErrorMsg err = MakeChannel(chanprop, user.get());
        if(err.errorno != TT_CMDERR_SUCCESS)
            return err;

        newchan = parent->GetSubChannel(chanprop.name);
        TTASSERT(newchan);
        makeop = true;
    }
    else
    {
        ErrorMsg err = m_srvguard->JoinChannel(*user, *newchan);
        if(!err.Success())
            return err;

        if (newchan->IsOwner(*user))
            makeop = true;
    }

    if(!newchan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    //check whether it's user initial channel or has specified correct password
    if(!user->GetInitialChannel().empty() &&
       newchan == ChangeChannel(GetRootChannel(),
                                user->GetInitialChannel()))
    {
    }
    else if(chanprop.passwd != newchan->GetPassword())
        return ErrorMsg(TT_CMDERR_INCORRECT_CHANNEL_PASSWORD);

    serverchannel_t const oldchan = user->GetChannel();

    if(oldchan && newchan->Compare(oldchan))
        return ErrorMsg(TT_CMDERR_ALREADY_IN_CHANNEL);

    if(newchan->GetUsersCount()+1 > newchan->GetMaxUsers())
        return ErrorMsg(TT_CMDERR_MAX_CHANNEL_USERS_EXCEEDED);

    //leave the current channel if applicable
    if(oldchan)
    {
        //HACK: protect new channel so it doesn't get deleted if it's dynamic 
        //and parent of 'oldchan'
        int const chantype = newchan->GetChannelType();
        newchan->SetChannelType(chantype | CHANNEL_PERMANENT);
        ErrorMsg const err = UserLeaveChannel(user->GetUserID(), oldchan->GetChannelID());
        TTASSERT(err.errorno == TT_CMDERR_SUCCESS);
        newchan->SetChannelType(chantype);
    }

    //add user to channel
    newchan->AddUser(user->GetUserID(), user);

    //set new channel
    user->SetChannel(newchan);

    // forward hidden channel to user if user cannot see it
    if (((newchan->GetChannelType() & CHANNEL_HIDDEN) != 0u) &&
        (user->GetUserRights() & USERRIGHT_VIEW_HIDDEN_CHANNELS) == USERRIGHT_NONE)
    {
        user->DoAddChannel(*newchan, IsEncrypted());
    }
    
    //notify user of new channel
    user->DoJoinedChannel(*newchan, IsEncrypted());

    //check if user should automatically become operator of channel
    UserAccount useraccount = user->GetUserAccount();
    if(useraccount.auto_op_channels.contains(newchan->GetChannelID()))
        makeop = true;

    //notify users that new user has joined
    ServerChannel::users_t notifyusers;
    if ((newchan->GetChannelType() & CHANNEL_HIDDEN) != 0u)
        notifyusers = GetNotificationUsers(USERRIGHT_VIEW_HIDDEN_CHANNELS | USERRIGHT_VIEW_ALL_USERS, newchan);
    else
        notifyusers = GetNotificationUsers(USERRIGHT_VIEW_ALL_USERS, newchan);

    for (const auto& u : notifyusers)
    {
        u->DoAddUser(*user, *newchan);
    }

    // notify new user of other users in same channel if not visible
    if ((user->GetUserRights() & USERRIGHT_VIEW_ALL_USERS) == USERRIGHT_NONE ||
        (((newchan->GetChannelType() & CHANNEL_HIDDEN) != 0u) && (user->GetUserRights() & USERRIGHT_VIEW_HIDDEN_CHANNELS) == USERRIGHT_NONE))
    {
        for (const auto &cu : newchan->GetUsers())
        {
            if (cu != user)
                user->DoAddUser(*cu, *newchan);
        }
    }

    if(makeop)
    {
        newchan->AddOperator(user->GetUserID());
        UpdateChannel(newchan, user.get()); //notify users of new operator
    }

    //send channel's file list
    if((user->GetUserType() & USERTYPE_DEFAULT) != 0u)
        user->ForwardFiles(newchan, false);

    //start active desktop transmissions
    for (const auto &cu : newchan->GetUsers())
    {
        if (cu->GetDesktopSession() && 
           ((user->GetSubscriptions(*cu) & SUBSCRIBE_DESKTOP) != 0u))
        {
            //Start delayed timers for desktop transmission, so the new 
            //user will have received the channel's channel-key.
            TimerHandler* th = nullptr;
            timer_userdata tm_data;
            tm_data.src_userid = cu->GetUserID();
            tm_data.dest_userid = user->GetUserID();
            ACE_NEW_NORETURN(th, TimerHandler(*this, TIMERSRV_START_DESKTOPTX_ID,
                                              tm_data.userdata));
            long const timerid = m_timer_reactor->schedule_timer(th, nullptr, ACE_Time_Value(1));
            TTASSERT(timerid>=0);
        }
        //TODO: user could actually reuse the desktop session (but restarts at the moment)
//         if (user->GetDesktopSession() && 
//            cu->GetUserID() != user->GetUserID())
//             StartDesktopTransmitter(*user, cu, *newchan);
    }

    //notify listener
    TTASSERT(newchan == user->GetChannel());
    if ((m_properties.logevents & SERVERLOGEVENT_USER_JOINEDCHANNEL) != 0u)
    {
        m_srvguard->OnUserJoinChannel(*user, *newchan);
    }

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserLeaveChannel(int userid, int channelid)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    serverchannel_t chan;
    //'channelid' can only be "current channel"
    if(channelid>0)
        chan = GetChannel(channelid);
    else if(user)
        chan = user->GetChannel();

    if(!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);
    if(!user || !chan->UserExists(userid))
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    //send command to user that he left the channel
    user->DoLeftChannel(*chan);

    //notify admins and "show-all" users
    ServerChannel::users_t notifyusers;
    if ((chan->GetChannelType() & CHANNEL_HIDDEN) != 0u)
        notifyusers = GetNotificationUsers(USERRIGHT_VIEW_HIDDEN_CHANNELS | USERRIGHT_VIEW_ALL_USERS, chan);
    else
        notifyusers = GetNotificationUsers(USERRIGHT_VIEW_ALL_USERS, chan);
    
    for (const auto& u : notifyusers)
        u->DoRemoveUser(*user, *chan);

    //close active desktop transmissions
    for (const auto &u : chan->GetUsers())
    {
        StopDesktopTransmitter(*u, *user, false);
        StopDesktopTransmitter(*user, *u, false);
    }

    // remove users in channel so they are not visible to user after exiting
    if ((user->GetUserRights() & USERRIGHT_VIEW_ALL_USERS) == USERRIGHT_NONE)
    {
        for (const auto &u : chan->GetUsers())
        {
            if(user->GetUserID() != u->GetUserID())
                user->DoRemoveUser(*u, *chan);
        }
    }

    //check if in allowed to transmit
    bool modified = false;
    chan->RemoveUser(user->GetUserID(), &modified);

    if (modified)
        UpdateChannel(*chan, chan->GetUsers(), user.get());

    // forward hidden channel to user
    if (((chan->GetChannelType() & CHANNEL_HIDDEN) != 0u) &&
        (user->GetUserRights() & USERRIGHT_VIEW_HIDDEN_CHANNELS) == USERRIGHT_NONE)
    {
        user->DoRemoveChannel(*chan);
    }

    serverchannel_t nullc;
    user->SetChannel(nullc);

    if ((m_properties.logevents & SERVERLOGEVENT_USER_LEFTCHANNEL) != 0u)
    {
        m_srvguard->OnUserLeaveChannel(*user, *chan);
    }

    CleanChannels(chan);

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

void ServerNode::UserDisconnected(int userid)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr, false);
    TTASSERT(user);
    if(user)
    {
        //logout user
        if(user->IsAuthorized())
            UserLogout(userid);

        //if users have modified any subscriptions to this user, clear it
        for (const auto& u : GetAuthorizedUsers())
            u->ClearUserSubscription(*user);

        //notify listener (if any)
        if ((m_properties.logevents & SERVERLOGEVENT_USER_DISCONNECTED) != 0u)
        {
            m_srvguard->OnUserDisconnected(*user);
        }

        //if it's a file transfer. Clean it up.
        if(user->GetFileTransferID() != 0)
            m_filetransfers.erase(user->GetFileTransferID());

        m_updUserIPs.erase(userid);
        m_mUsers.erase(userid);
        TTASSERT(!m_rootchannel || m_rootchannel->GetUser(userid) == nullptr);
    }
}

ErrorMsg ServerNode::UserOpDeOp(int userid, int channelid, 
                                const ACE_TString& oppasswd, int op_userid, bool op)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const opper = GetUser(userid, nullptr);
    serveruser_t const op_user = GetUser(op_userid, opper.get());
    if(!op_user || !opper)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t const chan = GetChannel(channelid);
    if(!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    if(((opper->GetUserRights() & USERRIGHT_OPERATOR_ENABLE) != 0u) ||
        ((!oppasswd.empty()) && oppasswd == chan->GetOpPassword()))
    {
        if(op)
            chan->AddOperator(op_userid);
        else
            chan->RemoveOperator(op_userid);

        UpdateChannel(chan, opper.get());

        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    if(oppasswd.length())
        return ErrorMsg(TT_CMDERR_INCORRECT_OP_PASSWORD);
    return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
}

ErrorMsg ServerNode::UserKick(int userid, int kick_userid, int chanid,
                              bool force_kick)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const kicker = GetUser(userid, nullptr);
    serveruser_t const kickee = GetUser(kick_userid, kicker.get());
    if (!kickee || (!kicker && !force_kick))
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    if(chanid != 0) //kick from channel
    {
        serverchannel_t const chan = GetChannel(chanid);
        if(!chan)
            return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

        if(!chan->UserExists(kick_userid))
            return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

        if (force_kick ||
            ((kicker->GetUserRights() & USERRIGHT_KICK_USERS) != 0u) ||
            chan->IsOperator(userid) ||
            chan->IsAutoOperator(*kicker))
        {
            kickee->DoKicked(userid, true);

            if ((m_properties.logevents & SERVERLOGEVENT_USER_KICKED) != 0u)
            {
                m_srvguard->OnUserKicked(*kickee, kicker.get(), chan.get());
            }

            return UserLeaveChannel(kick_userid, chanid);
        }
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
    }

    if(force_kick || ((kicker->GetUserRights() & USERRIGHT_KICK_USERS) != 0u))//kick from server
    {
        kickee->DoKicked(userid, false);

        if ((m_properties.logevents & SERVERLOGEVENT_USER_KICKED) != 0u)
        {
            m_srvguard->OnUserKicked(*kickee, kicker.get(), kickee->GetChannel().get());
        }

        if(kickee->IsAuthorized())
            return UserLogout(kick_userid);
                    return ErrorMsg(TT_CMDERR_USER_NOT_FOUND); //already logged out
    }
    return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
}

ErrorMsg ServerNode::UserBan(int userid, BannedUser ban)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const banner = GetUser(userid, nullptr);
    if (!banner)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t banchan;
    ErrorMsg err(TT_CMDERR_SUCCESS);

    if ((ban.bantype & BANTYPE_CHANNEL) != 0u)
    {
        if(ban.chanpath.is_empty())
            return TT_CMDERR_CHANNEL_NOT_FOUND;

        banchan = ChangeChannel(GetRootChannel(), ban.chanpath);
        if (!banchan)
            return TT_CMDERR_CHANNEL_NOT_FOUND;
        ban.chanpath = banchan->GetChannelPath();
    }

    ban.owner = banner->GetUsername();
    if ((banner->GetUserRights() & USERRIGHT_BAN_USERS) == 0)
    {
        if (banchan && (banchan->IsOperator(userid) || banchan->IsAutoOperator(*banner)))
            err = m_srvguard->AddUserBan(*banner, ban);
        else
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
    }
    else
    {
        err = m_srvguard->AddUserBan(*banner, ban);
    }

    if (banchan && err.Success())
        AddBannedUserToChannel(ban);

    if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_USER_BANNED) != 0u))
    {
        m_srvguard->OnUserBanned(*banner, ban);
    }

    if (err.Success() && IsAutoSaving())
    {
        err = m_srvguard->SaveConfiguration(*banner, *this);
        if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u))
        {
            m_srvguard->OnSaveConfiguration(banner.get());
        }
    }
    return err;
}

ErrorMsg ServerNode::UserBan(int userid, int ban_userid, BannedUser ban)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const banner = GetUser(userid, nullptr);
    if (!banner)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t banchan;
    ErrorMsg err(TT_CMDERR_SUCCESS);

    serveruser_t const ban_user = GetUser(ban_userid, banner.get());
    if (!ban_user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    if ((ban.bantype & BANTYPE_CHANNEL) != 0u)
    {
        if (!ban.chanpath.empty())
        {
            banchan = ChangeChannel(GetRootChannel(), ban.chanpath);
            if (!banchan)
                return TT_CMDERR_CHANNEL_NOT_FOUND;
        }
        else
        {
            banchan = ban_user->GetChannel();
            if (!banchan)
                return TT_CMDERR_CHANNEL_NOT_FOUND;
            ban.chanpath = banchan->GetChannelPath();
        }
        ban = ban_user->GenerateBan(ban.bantype, ban.chanpath);
    }

    if ((banner->GetUserRights() & USERRIGHT_BAN_USERS) == 0)
    {
        if (!banchan || (!banchan->IsOperator(userid) && !banchan->IsAutoOperator(*banner)))
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
    }

    ban.owner = banner->GetUsername();
    err = m_srvguard->AddUserBan(*banner, *ban_user, ban.bantype);
    if (banchan && err.Success())
        AddBannedUserToChannel(ban);

    if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_USER_BANNED) != 0u))
    {
        m_srvguard->OnUserBanned(*ban_user, *banner);
    }

    if (err.Success() && IsAutoSaving())
    {
        err = m_srvguard->SaveConfiguration(*banner, *this);
        if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u))
        {
            m_srvguard->OnSaveConfiguration(banner.get());
        }
    }
    return err;
}

ErrorMsg ServerNode::UserUnBan(int userid, const BannedUser& ban)
{
    GUARD_OBJ(this, Lock());
    ErrorMsg err;

    serveruser_t const user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t banchan;
    if((ban.bantype & BANTYPE_CHANNEL) != 0u)
    {
        banchan = ChangeChannel(GetRootChannel(), ban.chanpath);
        if (!banchan)
            return TT_CMDERR_CHANNEL_NOT_FOUND;
    }

    if((user->GetUserRights() & USERRIGHT_BAN_USERS) == 0)
    {
        if (banchan && (banchan->IsOperator(userid) || banchan->IsAutoOperator(*user)))
            err = m_srvguard->RemoveUserBan(*user, ban);
        else
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
    }
    else
    {
        err = m_srvguard->RemoveUserBan(*user, ban);
    }

    if (err.Success() && banchan)
        banchan->RemoveUserBan(ban);

    if(err.Success())
    {
        if ((m_properties.logevents & SERVERLOGEVENT_USER_BANNED) != 0u)
        {
            m_srvguard->OnUserUnbanned(*user, ban);
        }

        if(IsAutoSaving())
        {
            err = m_srvguard->SaveConfiguration(*user, *this);
            if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u))
            {
                m_srvguard->OnSaveConfiguration(user.get());
            }
        }
    }
    return err;
}

ErrorMsg ServerNode::UserListServerBans(int userid, int chanid, int index, int count)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t const banchan = GetChannel(chanid);
    if (chanid > 0 && !banchan)
        return TT_CMDERR_CHANNEL_NOT_FOUND;

    if((user->GetUserRights() & USERRIGHT_BAN_USERS) == 0)
    {
        if (!banchan || (!banchan->IsOperator(userid) && !banchan->IsAutoOperator(*user)))
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
    }

    std::vector<BannedUser> bans;
    if (banchan)
    {
        bans = banchan->GetBans();
    }
    else
    {
        m_srvguard->GetUserBans(*user, bans);
    }

    for(;index<std::min(count, int(bans.size()));++index)
    {
        user->DoShowBan(bans[index]);
    }
    
    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserListUserAccounts(int userid, int index, int count)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    useraccounts_t users;
    ErrorMsg ret = m_srvguard->GetRegUsers(*user, users);
    if(ret.errorno == TT_CMDERR_SUCCESS)
    {
        for(size_t i=index;i<users.size() && ((count--) != 0);i++)
            user->DoShowUserAccount(users[i]);
    }
    return ret;
}

ErrorMsg ServerNode::UserNewUserAccount(int userid, const UserAccount& regusr)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    // allow anonymous account
    // if(regusr.username.empty())
    //     return ErrorMsg(TT_CMDERR_INVALID_USERNAME);

    auto is = regusr.auto_op_channels.begin();
    for(;is != regusr.auto_op_channels.end();is++)
    {
        serverchannel_t const chan = GetChannel(*is);
        if (!chan)
            return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);
    }
    
    ErrorMsg err = m_srvguard->AddRegUser(*user, regusr);
    if (err.Success())
    {
        if (IsAutoSaving())
        {
            err = m_srvguard->SaveConfiguration(*user, *this);
            if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u))
            {
                m_srvguard->OnSaveConfiguration(user.get());
            }
        }
        for (const auto& au : GetAdministrators())
        {
            if (VersionSameOrLater(au->GetStreamProtocol(), ACE_TEXT("5.11")))
                au->DoAddUserAccount(regusr);
        }
    }
    return err;
}

ErrorMsg ServerNode::UserDeleteUserAccount(int userid, const ACE_TString& username)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    ErrorMsg err = m_srvguard->DeleteRegUser(*user, username);
    if (err.Success())
    {    
        if (IsAutoSaving())
        {
            err = m_srvguard->SaveConfiguration(*user, *this);
            if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u))
                m_srvguard->OnSaveConfiguration(user.get());
        }
        for (const auto& au : GetAdministrators())
        {
            if (VersionSameOrLater(au->GetStreamProtocol(), ACE_TEXT("5.11")))
                au->DoRemoveUserAccount(username);
        }
    }
    return err;
}

ErrorMsg ServerNode::UserUpdateChannel(int userid, const ChannelProp& chanprop)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t const chan = GetChannel(chanprop.channelid);
    if (!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    // don't allow codec change in channel with users
    if ((chan->GetUsersCount() != 0) && chanprop.audiocodec != chan->GetAudioCodec())
        return ErrorMsg(TT_CMDERR_CHANNEL_HAS_USERS);

    //don't allow change of hidden channel property
    if ((chanprop.chantype & CHANNEL_HIDDEN) != (chan->GetChannelType() & CHANNEL_HIDDEN))
    {
        if ((user->GetUserRights() & USERRIGHT_MODIFY_CHANNELS) != 0u)
            return ErrorMsg(TT_CMDERR_CHANNEL_CANNOT_BE_HIDDEN);
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
    }

    //user can update channel if either admin or operator of channel
    if ((user->GetUserRights() & USERRIGHT_MODIFY_CHANNELS) != 0u)
        return UpdateChannel(chanprop, user.get());
    if (chan->IsOperator(userid) || chan->IsAutoOperator(*user))
    {
        //don't allow operator to change static channel property
        if ((chanprop.chantype & CHANNEL_PERMANENT) != (chan->GetChannelType() & CHANNEL_PERMANENT))
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        //don't allow operator to change name of static channel
        if ((chan->GetChannelType() & CHANNEL_PERMANENT) && chanprop.name != chan->GetName())
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        return UpdateChannel(chanprop, user.get());
    }
    else
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
}

ErrorMsg ServerNode::UserUpdateServer(int userid, const ServerSettings& properties)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    if((user->GetUserRights() & USERRIGHT_UPDATE_SERVERPROPERTIES) == 0)
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

    return UpdateServer(properties, user.get());
}

ErrorMsg ServerNode::UserSaveServerConfig(int userid)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    auto errmsg = m_srvguard->SaveConfiguration(*user, *this);
    if (errmsg.Success() && ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u))
    {
        m_srvguard->OnSaveConfiguration(user.get());
    }
    return errmsg;
}

ErrorMsg ServerNode::UpdateServer(const ServerSettings& properties,
                                  const ServerUser* user /*= nullptr*/)
{
    // log server save event if it is or was enabled
    bool const logevent = ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u) ||
                    ((properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u);
    SetServerProperties(properties);

    if (logevent)
    {
        m_srvguard->OnServerUpdated(user, properties);
    }

    for (const auto& u : GetAuthorizedUsers())
        u->DoServerUpdate(m_properties);

    ErrorMsg err(TT_CMDERR_SUCCESS);
    if (IsAutoSaving() && (user != nullptr))
    {
        err = m_srvguard->SaveConfiguration(*user, *this);
        if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u))
        {
            m_srvguard->OnSaveConfiguration(user);
        }
    }

    return err;
}

ErrorMsg ServerNode::MakeChannel(const ChannelProp& chanprop, 
                                 const ServerUser* user/* = NULL*/)
{
    GUARD_OBJ(this, Lock());

    //initial server configuration creates the root channel, so initially it's null
    if (m_rootchannel && m_rootchannel->GetSubChannelCount(true) + 1 > MAX_CHANNELS)
        return ErrorMsg(TT_CMDERR_MAX_CHANNELS_EXCEEDED);

    //check bandwidth restriction
    if((user != nullptr) && (user->GetUserAccount().audiobpslimit != 0) &&
        GetAudioCodecBitRate(chanprop.audiocodec) > user->GetUserAccount().audiobpslimit)
        return ErrorMsg(TT_CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED);

    //ensure channel doesn't already exist by name or id
    serverchannel_t parent;
    if (m_rootchannel)
    {
        parent = GetChannel(chanprop.parentid);
        if (!parent)
            return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

        if (parent->GetSubChannel(chanprop.name))
            return ErrorMsg(TT_CMDERR_CHANNEL_ALREADY_EXISTS);

        if (chanprop.name.empty())
            return ErrorMsg(TT_CMDERR_CHANNEL_ALREADY_EXISTS);

        if(parent->GetChannelPath().length() + chanprop.name.length() + 
            ACE_TString(CHANNEL_SEPARATOR).length() >= MAX_STRING_LENGTH)
            return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

        if ((parent->GetChannelType() & CHANNEL_HIDDEN) != 0u)
            return ErrorMsg(TT_CMDERR_CHANNEL_CANNOT_BE_HIDDEN);
    }

    if(chanprop.passwd.length() >= MAX_STRING_LENGTH)
        return ErrorMsg(TT_CMDERR_INCORRECT_CHANNEL_PASSWORD);

    if(GetChannel(chanprop.channelid).get() != nullptr)
        return ErrorMsg(TT_CMDERR_CHANNEL_ALREADY_EXISTS);

    //generate next channel id.
    int chanid = chanprop.channelid;
    while((chanid == 0) || (chanid < MAX_CHANNELS && (GetChannel(chanid).get() != nullptr)))
    {
        if(++chanid > MAX_CHANNELS)
            chanid = 1;
    }

    serverchannel_t chan;
    if (!parent)
    {
        TTASSERT(!GetRootChannel());

        if ((chanprop.chantype & CHANNEL_HIDDEN) != 0u)
            return ErrorMsg(TT_CMDERR_CHANNEL_CANNOT_BE_HIDDEN);
        
        chan = std::make_shared<ServerChannel>(chanid);
        m_rootchannel = chan;
    }
    else
    {
        chan = std::make_shared<ServerChannel>(parent, chanid, chanprop.name);
        parent->AddSubChannel(chan);
    }
    chan->SetPassword(chanprop.passwd);
    chan->SetTopic(chanprop.topic);
    chan->SetMaxDiskUsage(chanprop.diskquota);
    chan->SetOpPassword(chanprop.oppasswd);
    chan->SetMaxUsers(chanprop.maxusers);
    chan->SetAudioCodec(chanprop.audiocodec);
    chan->SetAudioConfig(chanprop.audiocfg);
    chan->SetChannelType(chanprop.chantype);
    chan->SetUserData(chanprop.userdata);
    chan->SetVoiceUsers(chanprop.GetTransmitUsers(STREAMTYPE_VOICE));
    chan->SetVideoUsers(chanprop.GetTransmitUsers(STREAMTYPE_VIDEOCAPTURE));
    chan->SetDesktopUsers(chanprop.GetTransmitUsers(STREAMTYPE_DESKTOP));
    chan->SetMediaFileUsers(chanprop.GetTransmitUsers(STREAMTYPE_MEDIAFILE));
    chan->SetChannelTextMsgUsers(chanprop.GetTransmitUsers(STREAMTYPE_CHANNELMSG));
    chan->SetTransmitQueue(chanprop.transmitqueue);
    if (chanprop.transmitswitchdelay > 0) // inherit default from ServerChannel
        chan->SetTransmitSwitchDelay(ToTimeValue(chanprop.transmitswitchdelay));
    chan->SetTimeOutTimerVoice(ToTimeValue(chanprop.totvoice));
    chan->SetTimeOutTimerMediaFile(ToTimeValue(chanprop.totmediafile));
    if (user != nullptr)
        chan->SetOwner(*user);

    //forward new channel to users
    ServerChannel::users_t users;
    if ((chan->GetChannelType() & CHANNEL_HIDDEN) != 0u)
        users = GetNotificationUsers(USERRIGHT_VIEW_HIDDEN_CHANNELS, chan);
    else
        users = GetAuthorizedUsers();

    for (const auto& u : users)
    {
        u->DoAddChannel(*chan, IsEncrypted());
    }

    //notify listener if any
    if ((m_properties.logevents & SERVERLOGEVENT_CHANNEL_CREATED) != 0u)
    {
        m_srvguard->OnChannelCreated(*chan, user);
    }

    ErrorMsg err(TT_CMDERR_SUCCESS);
    if (IsAutoSaving() && ((chanprop.chantype & CHANNEL_PERMANENT) != 0u) && (user != nullptr))
    {
        err = m_srvguard->SaveConfiguration(*user, *this);
        if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u))
        {
            m_srvguard->OnSaveConfiguration(user);
        }
    }

    return err;
}

ErrorMsg ServerNode::UpdateChannel(const ChannelProp& chanprop, 
                                   const ServerUser* user/* = NULL*/)
{
    GUARD_OBJ(this, Lock());

    TTASSERT(GetRootChannel());
    serverchannel_t const chan = GetChannel(chanprop.channelid);
    if (!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    //ensure channel with same name doesn't already exist
    serverchannel_t const parent = GetChannel(chanprop.parentid);
    if (parent)
    {
        serverchannel_t const subchan = parent->GetSubChannel(chanprop.name);
        if((subchan.get() != nullptr) && subchan->GetChannelID() != chanprop.channelid)
            return ErrorMsg(TT_CMDERR_CHANNEL_ALREADY_EXISTS);
    }

    if(m_rootchannel != chan)
    {
        if(chanprop.name.empty())
            return ErrorMsg(TT_CMDERR_CHANNEL_ALREADY_EXISTS);

        bool const newname = chanprop.name != chan->GetName();
        chan->SetName(chanprop.name);
        if (newname)
            chan->UpdateChannelBans();
    }
    chan->SetTopic(chanprop.topic);
    chan->SetMaxDiskUsage(chanprop.diskquota);
    chan->SetOpPassword(chanprop.oppasswd);
    chan->SetMaxUsers(chanprop.maxusers);
    chan->SetChannelType(chanprop.chantype);
    chan->SetUserData(chanprop.userdata);
    //don't change codec if the channel has users
    if(chan->GetUsersCount() == 0)
        chan->SetAudioCodec(chanprop.audiocodec);
    chan->SetAudioConfig(chanprop.audiocfg);
    chan->SetPassword(chanprop.passwd);
    chan->SetVoiceUsers(chanprop.GetTransmitUsers(STREAMTYPE_VOICE));
    chan->SetVideoUsers(chanprop.GetTransmitUsers(STREAMTYPE_VIDEOCAPTURE));
    //close active desktop sessions
    if(chan->GetDesktopUsers() != chanprop.GetTransmitUsers(STREAMTYPE_DESKTOP))
    {
        auto ii = chan->GetDesktopUsers().begin();
        for(;ii!=chan->GetDesktopUsers().end();ii++)
        {
            if(chanprop.GetTransmitUsers(STREAMTYPE_DESKTOP).contains(*ii) != 0u)
            {
                serveruser_t const src_user = GetUser(*ii, nullptr);
                //TTASSERT(src_user); userid can be TRANSMITUSERS_FREEFORALL (0xFFF)
                if (!src_user || !src_user->GetDesktopSession())
                    continue;
                //TODO: this doesn't handle users who're intercepting packets
                for (const auto &u : chan->GetUsers())
                    StopDesktopTransmitter(*src_user, *u, true);
            }
        }
    }
    chan->SetDesktopUsers(chanprop.GetTransmitUsers(STREAMTYPE_DESKTOP));
    chan->SetMediaFileUsers(chanprop.GetTransmitUsers(STREAMTYPE_MEDIAFILE));
    chan->SetChannelTextMsgUsers(chanprop.GetTransmitUsers(STREAMTYPE_CHANNELMSG));
    chan->SetTransmitQueue(chanprop.transmitqueue);
    chan->SetTransmitSwitchDelay(ToTimeValue(chanprop.transmitswitchdelay));
    chan->SetTimeOutTimerVoice(ToTimeValue(chanprop.totvoice));
    chan->SetTimeOutTimerMediaFile(ToTimeValue(chanprop.totmediafile));

    UpdateChannel(chan, user);

    if (IsAutoSaving() && ((chan->GetChannelType() & CHANNEL_PERMANENT) != 0u) && (user != nullptr))
    {
        auto err = m_srvguard->SaveConfiguration(*user, *this);
        if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u))
        {
            m_srvguard->OnSaveConfiguration(user);
        }
    }

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

void ServerNode::CleanChannels(serverchannel_t& channel)
{
    ASSERT_SERVERNODE_LOCKED(this);

    //remove channel if empty
    while (channel && channel != GetRootChannel())
    {
        if( channel->GetUsersCount()==0 && 
            channel->GetSubChannelCount()==0 &&
            (channel->GetChannelType() & CHANNEL_PERMANENT) == 0)
        {
            serverchannel_t const parent = channel->GetParentChannel();
            RemoveChannel(channel->GetChannelID());
            channel = parent;
        }
        else
            break;
    }
}

void ServerNode::UpdateSoloTransmitChannels()
{
     //update solo transmisson
     std::stack<serverchannel_t> sweeper;
     sweeper.push(GetRootChannel());

     while(!sweeper.empty())
     {
         serverchannel_t const chan = sweeper.top();
         sweeper.pop();

         std::vector<int> const txq = chan->GetTransmitQueue();
         chan->CanTransmit(SERVER_USERID, STREAMTYPE_VOICE, 0, nullptr);
         chan->ClearFromTransmitQueue(SERVER_USERID);
         if (txq != chan->GetTransmitQueue())
         {
             UpdateChannel(*chan, chan->GetUsers(), nullptr);
         }

         ServerChannel::channels_t subs = chan->GetSubChannels();
         for(const auto & sub : subs)
             sweeper.push(sub);
     }
}

ErrorMsg ServerNode::RemoveChannel(int channelid, const ServerUser* user/* = NULL*/)
{
    GUARD_OBJ(this, Lock());

    TTASSERT(GetRootChannel());
    bool bStatic = false;

    serverchannel_t const chan = GetChannel(channelid);
    if (!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    ErrorMsg err = m_srvguard->RemoveChannel(*chan, user);
    if (!err.Success())
        return err;

    bStatic = ((chan->GetChannelType() & CHANNEL_PERMANENT) != 0u);

    //recursive remove
    auto subchannels = chan->GetSubChannels();
    for (const auto& c : subchannels)
    {
        TTASSERT(c);
        auto err = RemoveChannel(c->GetChannelID(), user);
        if (!err.Success())
            return err;
    }

    ServerChannel::users_t const users = chan->GetUsers(); //copy because mutates during UserKick()
    for (const auto &u : users)
    {
        ErrorMsg const err = UserKick(0, u->GetUserID(), chan->GetChannelID(), true);
        TTASSERT(err.Success());
    }
    TTASSERT(chan->GetUsers().empty());
    TTASSERT(chan->GetSubChannels().empty());

    //check if channel still exists (kick may have removed it)
    if (!GetChannel(chan->GetChannelID()))
        return ErrorMsg(TT_CMDERR_SUCCESS);

    //remove files from channel
    files_t files;
    chan->GetFiles(files, false);
    for (const auto& file : files)
    {
        ErrorMsg const err = RemoveFileFromChannel(file.filename, chan->GetChannelID());
        TTASSERT(err.Success());
    }

    //remove as subchannel (unless it's the root)
    serverchannel_t const parent = chan->GetParentChannel();
    if (parent)
    {
        //notify users
        ServerChannel::users_t notifyusers;
        if ((chan->GetChannelType() & CHANNEL_HIDDEN) != 0u)
            notifyusers = GetNotificationUsers(USERRIGHT_VIEW_HIDDEN_CHANNELS);
        else
            notifyusers = GetNotificationUsers(USERRIGHT_VIEW_ALL_USERS);

        for (const auto& u : notifyusers)
            u->DoRemoveChannel(*chan);

        parent->RemoveSubChannel(chan->GetName());
        //notify listener if any
        if ((m_properties.logevents & SERVERLOGEVENT_CHANNEL_REMOVED) != 0u)
        {
            m_srvguard->OnChannelRemoved(*chan, user);
        }
    }

    if (IsAutoSaving() && bStatic && (user != nullptr))
    {
        err = m_srvguard->SaveConfiguration(*user, *this);
        if (err.Success() && ((m_properties.logevents & SERVERLOGEVENT_SERVER_SAVECONFIG) != 0u))
        {
            m_srvguard->OnSaveConfiguration(user);
        }
    }

    return err;
}

void ServerNode::UpdateChannel(const serverchannel_t& chan, const ServerUser* user)
{
    ASSERT_SERVERNODE_LOCKED(this);
    TTASSERT(chan);

    if ((chan->GetChannelType() & CHANNEL_HIDDEN) != 0u)
        UpdateChannel(*chan, GetNotificationUsers(USERRIGHT_VIEW_HIDDEN_CHANNELS, chan), user);
    else
        UpdateChannel(*chan, GetAuthorizedUsers(), user);
}

void ServerNode::UpdateChannel(const ServerChannel& chan, const ServerChannel::users_t& users,
                               const ServerUser* user)
{
    ASSERT_SERVERNODE_LOCKED(this);

    for (const auto& u : users)
        u->DoUpdateChannel(chan, IsEncrypted());

    if ((m_properties.logevents & SERVERLOGEVENT_CHANNEL_UPDATED) != 0u)
        m_srvguard->OnChannelUpdated(chan, user);
}

ErrorMsg ServerNode::UserMove(int userid, int moveuserid, int channelid)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    if((user->GetUserRights() & USERRIGHT_MOVE_USERS) == 0)
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

    serveruser_t const moveuser = GetUser(moveuserid, user.get());
    if (!moveuser)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t const chan = GetChannel(channelid);
    if (!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    ErrorMsg err = UserJoinChannel(moveuserid, chan->GetChannelProp());

    if(err.errorno == TT_CMDERR_SUCCESS)
    {
        if ((m_properties.logevents & SERVERLOGEVENT_USER_MOVED) != 0u)
        {
            m_srvguard->OnUserMoved(*user, *moveuser);
        }
    }
    return err;
}

ErrorMsg ServerNode::UserTextMessage(const TextMessage& msg)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const from = GetUser(msg.from_userid, nullptr);
    if (!from)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    switch(msg.msgType)
    {
    case TTUserMsg :
    {
        if ((from->GetUserRights() & USERRIGHT_TEXTMESSAGE_USER) == USERRIGHT_NONE)
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        serveruser_t const to_user = GetUser(msg.to_userid, from.get());
        if (!to_user)
            return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

        //just ignore text message if the user doesn't subscribe
        if( (to_user->GetSubscriptions(*from) & SUBSCRIBE_USER_MSG) == 0)
            return ErrorMsg(TT_CMDERR_SUCCESS);

        to_user->DoTextMessage(*from, msg);

        //notify administrators for user2user message
        for (const auto& au : GetAdministrators())
        {
            if (((au->GetSubscriptions(*from) & SUBSCRIBE_INTERCEPT_USER_MSG) != 0u) &&
                au->GetUserID() != msg.to_userid && 
                au->GetUserID() != msg.from_userid)
            {
                au->DoTextMessage(*from, msg);
            }
        }

        //log text message
        if ((m_properties.logevents & SERVERLOGEVENT_USER_TEXTMESSAGE_PRIVATE) != 0u)
        {
            m_srvguard->OnUserMessage(*from, *to_user, msg);
        }

        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    case TTCustomMsg :
    {
        serveruser_t const to_user = GetUser(msg.to_userid, from.get());
        if (!to_user)
            return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

        //just ignore text message if the user doesn't subscribe
        if( (to_user->GetSubscriptions(*from) & SUBSCRIBE_CUSTOM_MSG) == 0)
            return ErrorMsg(TT_CMDERR_SUCCESS);

        to_user->DoTextMessage(*from, msg);

        //notify administrators for user2user message
        for (const auto& au : GetAdministrators())
        {
            if (((au->GetSubscriptions(*from) & SUBSCRIBE_INTERCEPT_CUSTOM_MSG) != 0u) &&
                au->GetUserID() != msg.to_userid && 
                au->GetUserID() != msg.from_userid)
            {
                au->DoTextMessage(*from, msg);
            }
        }

        //log text message
        if ((m_properties.logevents & SERVERLOGEVENT_USER_TEXTMESSAGE_CUSTOM) != 0u)
        {
            m_srvguard->OnCustomMessage(*from, *to_user, msg);
        }

        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    case TTChannelMsg :
    {
        if ((from->GetUserRights() & USERRIGHT_TEXTMESSAGE_CHANNEL) == USERRIGHT_NONE)
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        serverchannel_t const chan = GetChannel(msg.channelid);
        if (!chan)
            return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

        if(from->GetChannel() != chan &&
           (from->GetUserType() & USERTYPE_ADMIN) == 0)
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        if (!chan->CanTransmit(from->GetUserID(), STREAMTYPE_CHANNELMSG, 0, nullptr))
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        //forward message to all users of that channel
        intset_t already_recv;
        for (const auto& cu : chan->GetUsers())
        {
            already_recv.insert(cu->GetUserID());
            if ((cu->GetSubscriptions(*from) & SUBSCRIBE_CHANNEL_MSG) != 0u)
                cu->DoTextMessage(*from, msg);
        }

        //notify administrators of user2channel message                
        for (const auto& au : GetAdministrators(*chan))
        {
            if (already_recv.contains(au->GetUserID()))
                continue;
            if ((au->GetSubscriptions(*from) & SUBSCRIBE_INTERCEPT_CHANNEL_MSG) != 0u)
                au->DoTextMessage(*from, msg);
        }

        //log text message
        if ((m_properties.logevents & SERVERLOGEVENT_USER_TEXTMESSAGE_CHANNEL) != 0u)
        {
            m_srvguard->OnChannelMessage(*from, *chan, msg);
        }

        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    case TTBroadcastMsg :
    {
        if ((from->GetUserRights() & USERRIGHT_TEXTMESSAGE_BROADCAST) == USERRIGHT_NONE)
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        for (const auto& u : GetAuthorizedUsers())
        {
            if ((u->GetSubscriptions(*from) & SUBSCRIBE_BROADCAST_MSG) != 0u)
                u->DoTextMessage(*from, msg);
        }

        //log text message
        if ((m_properties.logevents & SERVERLOGEVENT_USER_TEXTMESSAGE_BROADCAST) != 0u)
        {
            m_srvguard->OnBroadcastMessage(*from, msg);
        }

        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    default : 
        //unknown message type
        return ErrorMsg(TT_CMDERR_INCOMPATIBLE_PROTOCOLS);
    }
}

ErrorMsg ServerNode::UserRegFileTransfer(FileTransfer& transfer)
{
    GUARD_OBJ(this, Lock());

    if(m_properties.filesroot.empty())
        return ErrorMsg(TT_CMDERR_FILESHARING_DISABLED);

    serveruser_t const user = GetUser(transfer.userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t const chan = GetChannel(transfer.channelid);
    if (!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    if (!chan->UserExists(user->GetUserID()) && (user->GetUserType() & USERTYPE_ADMIN) == 0)
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

    if (CountFileTransfers(user->GetUserID()) >= m_properties.maxfiletransfers)
        return ErrorMsg(TT_CMDERR_MAX_FILETRANSFERS_EXCEEDED);

    if(transfer.inbound)
    {
        if((user->GetUserRights() & USERRIGHT_UPLOAD_FILES) == 0)
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        //upload only if is admin or the filesize is less than the channel max usage
        //and also less than the total disk usage allowed.
        if(((chan->GetDiskUsage() + transfer.filesize) > chan->GetMaxDiskUsage() && 
            (user->GetUserType() & USERTYPE_ADMIN) == 0)
            || (GetDiskUsage() + transfer.filesize) > m_properties.maxdiskusage)
            return ErrorMsg(TT_CMDERR_MAX_DISKUSAGE_EXCEEDED);

        //set temporary filename for transfer
        int const id = std::max(1, m_filetx_id_counter++);
        if(m_filetransfers.contains(id)) //no IDs left
            return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);

        ACE_TString const tmpfilename = ACE_TEXT("tmp_") + I2String(id) + CHANNELFILEEXTENSION;
        ACE_TString const filepath = m_properties.filesroot + ACE_DIRECTORY_SEPARATOR_STR + tmpfilename;
        if(chan->FileExists(transfer.filename))
            return ErrorMsg(TT_CMDERR_FILE_ALREADY_EXISTS);
                    transfer.localfile = filepath;
        transfer.transferid = id;
    }
    else
    {
        if((user->GetUserRights() & USERRIGHT_DOWNLOAD_FILES) == 0)
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        //a user download
        RemoteFile remotefile;
        if(!chan->GetFile(transfer.filename, remotefile))
            return ErrorMsg(TT_CMDERR_FILE_NOT_FOUND);

        ACE_TString const internalpath = m_properties.filesroot + ACE_DIRECTORY_SEPARATOR_STR + remotefile.internalname;
        if(ACE_OS::filesize(internalpath.c_str())<0)
            return ErrorMsg(TT_CMDERR_FILE_NOT_FOUND);

        transfer.localfile = internalpath;
        transfer.filesize = remotefile.filesize;

        transfer.transferid = std::max(1, m_filetx_id_counter++);
        if(m_filetransfers.contains(transfer.transferid)) //no IDs left
            return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);
    }

    m_filetransfers[transfer.transferid] = transfer;

    TTASSERT(transfer.transferid>0);
    user->DoFileAccepted(transfer);
    return ErrorMsg(TT_CMDERR_SUCCESS);
}

int ServerNode::CountFileTransfers(int userid)
{
    ASSERT_SERVERNODE_LOCKED(this);
    auto ite = m_filetransfers.begin();
    int c = 0;
    while (ite != m_filetransfers.end())
    {
        ite = std::find_if(ite, m_filetransfers.end(),
                           [userid] (const std::pair<int, FileTransfer>& transfer) { return transfer.second.userid == userid; });
        if (ite != m_filetransfers.end())
        {
            ++c; ++ite;
        }
    }
    return c;
}

ErrorMsg ServerNode::UserSubscribe(int userid, int subuserid, 
                                   Subscriptions subscrip)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    serveruser_t const subscriptuser = GetUser(subuserid, user.get());
    if (!user || !subscriptuser)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);
    
    //only admins can intercept
    if((user->GetUserType() & USERTYPE_ADMIN) == 0)
    {
        if((subscrip & SUBSCRIBE_INTERCEPT_ALL) != 0u)
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
    }

    user->AddSubscriptions(*subscriptuser, subscrip);

    //update user's subscription mask, if viewing all users or
    //in same channel
    if (((subscriptuser->GetUserRights() & USERRIGHT_VIEW_ALL_USERS) != 0u) ||
        user->GetChannel() == subscriptuser->GetChannel())
    {
        subscriptuser->DoUpdateUser(*user);
    }

    if(subscriptuser != user)
        user->DoUpdateUser(*subscriptuser);

    //if active desktop then start it
    if (subscriptuser->GetDesktopSession() &&
        ((subscrip & (SUBSCRIBE_DESKTOP | SUBSCRIBE_INTERCEPT_DESKTOP)) != 0u))
    {
        serverchannel_t const chan = subscriptuser->GetChannel();
        if (chan)
            StartDesktopTransmitter(*subscriptuser, *user, *chan);
    }

    m_srvguard->OnUserSubscribe(*user, *subscriptuser);

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserUnsubscribe(int userid, int subuserid, 
                                     Subscriptions subscrip)
{
    GUARD_OBJ(this, Lock());

    serveruser_t const user = GetUser(userid, nullptr);
    serveruser_t const subscriptuser = GetUser(subuserid, user.get());
    if (user && subscriptuser)
    {
        user->ClearSubscriptions(*subscriptuser, subscrip);
        //update user's subscription mask, if viewing all users or
        //in same channel
        if (((subscriptuser->GetUserRights() & USERRIGHT_VIEW_ALL_USERS) != 0u) ||
            user->GetChannel() == subscriptuser->GetChannel())
        {
            subscriptuser->DoUpdateUser(*user);
        }
        
        if(subscriptuser != user)
            user->DoUpdateUser(*subscriptuser);

        //if active desktop then stop it
        if((subscrip & (SUBSCRIBE_DESKTOP | SUBSCRIBE_INTERCEPT_DESKTOP)) != 0u)
            StopDesktopTransmitter(*subscriptuser, *user, true);

        m_srvguard->OnUserSubscribe(*user, *subscriptuser);

        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);
}

ErrorMsg ServerNode::AddFileToChannel(const RemoteFile& remotefile)
{
    ASSERT_SERVERNODE_LOCKED(this);

    TTASSERT(!GetRootChannel()->FileExists(remotefile.fileid));
    serverchannel_t const channel = GetChannel(remotefile.channelid);
    if (!channel)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    channel->AddFile(remotefile);

    for (const auto& u : channel->GetUsers())
        u->DoAddFile(remotefile);
    for (const auto& au : GetAdministrators(*channel))
        au->DoAddFile(remotefile);

    return {};
}

ErrorMsg ServerNode::RemoveFileFromChannel(const ACE_TString& filename, int channelid)
{
    ASSERT_SERVERNODE_LOCKED(this);

    serverchannel_t const channel = GetChannel(channelid);
    if (!channel)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    RemoteFile remotefile;
    if(channel->GetFile(filename, remotefile))
    {
        channel->RemoveFile(filename);

        ACE_FILE_Connector con;
        ACE_FILE_IO file;
        ACE_TString const internalpath = m_properties.filesroot + ACE_DIRECTORY_SEPARATOR_STR + remotefile.internalname;
        if(con.connect(file, ACE_FILE_Addr(internalpath.c_str())) >= 0)
            file.remove();

        for (const auto& u : channel->GetUsers())
            u->DoRemoveFile(filename, *channel);

        for (const auto& au : GetAdministrators(*channel))
            au->DoRemoveFile(filename, *channel);
    }
    return {};
}

ErrorMsg ServerNode::AddBannedUserToChannel(const BannedUser& ban)
{
    TTASSERT(ban.bantype & BANTYPE_CHANNEL);
    serverchannel_t const chan = ChangeChannel(GetRootChannel(), ban.chanpath);
    if (!chan)
        return TT_CMDERR_CHANNEL_NOT_FOUND;
    chan->AddUserBan(ban);
    return TT_CMDERR_SUCCESS;
}

ErrorMsg ServerNode::SendTextMessage(const TextMessage& msg)
{
    switch(msg.msgType)
    {
    case TTUserMsg :
    case TTCustomMsg :
    {
        serveruser_t const to_user = GetUser(msg.to_userid, nullptr);
        if (!to_user)
            return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);
        to_user->DoTextMessage(msg);
        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    case TTChannelMsg :
    {
        serverchannel_t const chan = GetChannel(msg.channelid);
        if (!chan)
            return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

        //forward message to all users of that channel
        for (const auto& u : chan->GetUsers())
        {
            u->DoTextMessage(msg);
        }
        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    case TTBroadcastMsg :
    {
        for (const auto& u : GetAuthorizedUsers())
        {
            u->DoTextMessage(msg);
        }
        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    case TTNoneMsg :
        break;
    }
    return ErrorMsg(TT_CMDERR_INCOMPATIBLE_PROTOCOLS);
}

