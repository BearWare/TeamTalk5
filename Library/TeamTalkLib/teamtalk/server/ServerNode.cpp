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
#include "ServerUser.h"

#include <ace/FILE_Connector.h>
#include <ace/Dirent_Selector.h>
#include <ace/Dirent.h>
#include <ace/OS_NS_sys_stat.h>
#include <ace/OS_NS_sys_socket.h>

#include <stack>
#include <queue>
#include <memory>
#include <vector>
#include <algorithm>

#include <teamtalk/ttassert.h>
#include <teamtalk/PacketLayout.h>
#include <teamtalk/CodecCommon.h>
#include <TeamTalkDefs.h>

using namespace std;
using namespace teamtalk;

#define UDP_SOCKET_RECV_BUF_SIZE (1024*1024)
#define UDP_SOCKET_SEND_BUF_SIZE (1024*1024)

ServerNode::ServerNode(const ACE_TString& version,
                       ACE_Reactor* timerReactor,
                       ACE_Reactor* tcpReactor, 
                       ACE_Reactor* udpReactor, 
                       ServerNodeListener* listener /*= NULL*/)
                       : m_userid_counter(0)
                       , m_timer_reactor(timerReactor)
                       , m_tcp_reactor(tcpReactor)
                       , m_udp_reactor(udpReactor)
                       , m_srvguard(listener)
                       , m_onesec_timerid(-1)
                       , m_filetx_id_counter(0)
                       , m_file_id_counter(0)
{
    m_properties.version = version;

    int ret;
    ACE_thread_t tid;
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
}

ACE_Lock& ServerNode::lock()
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
    GUARD_OBJ(this, lock());

    //%users% %uptime% %voicetx% %voicerx% %admins% %lastuser%
    const ServerChannel::users_t& userslst = GetAuthorizedUsers(true);
    size_t users = GetAuthorizedUsers().size();
    size_t admins = users - userslst.size();
    ACE_TString uptime = UptimeHours(GetUptime());

    ACE_TString lastuser;
    ACE_Time_Value duration = ACE_Time_Value::max_time;
    for(size_t i=0;i<userslst.size();i++)
    {
        if(userslst[i]->GetDuration() < duration &&
           userslst[i]->GetUserID() != ignore_userid)
            lastuser = userslst[i]->GetNickname();
    }
    ACE_TString motd = m_properties.motd;
    replace_all(motd, ACE_TEXT("%users%"), i2string((int)users));
    replace_all(motd, ACE_TEXT("%admins%"), i2string((int)admins));
    replace_all(motd, ACE_TEXT("%uptime%"), uptime);
    replace_all(motd, ACE_TEXT("%voicetx%"), i2string((ACE_INT64)(m_stats.total_bytessent / 1024)));
    replace_all(motd, ACE_TEXT("%voicerx%"), i2string((ACE_INT64)(m_stats.total_bytesreceived / 1024)));
    replace_all(motd, ACE_TEXT("%lastuser%"), lastuser);

    return motd;
}

bool ServerNode::IsAutoSaving()
{
    ASSERT_REACTOR_LOCKED(this);
    return m_properties.autosave;
}

void ServerNode::SetAutoSaving(bool autosave)
{
    ASSERT_REACTOR_LOCKED(this);
    m_properties.autosave = autosave;
}

ACE_Time_Value ServerNode::GetUptime() const
{
    ASSERT_REACTOR_LOCKED(this);
    return ACE_OS::gettimeofday() - m_stats.starttime;
}

serverchannel_t& ServerNode::GetRootChannel()
{
    ASSERT_REACTOR_LOCKED(this);
    return m_rootchannel;
}

const ServerChannel::users_t& ServerNode::GetAdministrators()
{
    ASSERT_REACTOR_LOCKED(this);

#if defined(_DEBUG)
    ServerChannel::users_t users;
    for(mapusers_t::const_iterator i=m_mUsers.begin(); i != m_mUsers.end(); i++)
    {
        if((*i).second->GetUserType() & USERTYPE_ADMIN)
            users.push_back((*i).second);
    }
    TTASSERT(m_admins.size() == users.size());
#endif

    return m_admins;
}

ServerChannel::users_t ServerNode::GetAdministrators(const ServerChannel& excludeChannel)
{
    ASSERT_REACTOR_LOCKED(this);

    ServerChannel::users_t users;
    for(mapusers_t::const_iterator i=m_mUsers.begin(); i != m_mUsers.end(); i++)
    {
        if(((*i).second->GetUserType() & USERTYPE_ADMIN) && 
            (!(*i).second->GetChannel() ||
            (*i).second->GetChannel()->GetChannelID() != excludeChannel.GetChannelID()))
            users.push_back((*i).second);
    }

    return users;
}


ServerChannel::users_t ServerNode::GetAuthorizedUsers(bool excludeAdmins/* = false*/)
{
    ASSERT_REACTOR_LOCKED(this);

    ServerChannel::users_t users;
    for(mapusers_t::const_iterator i=m_mUsers.begin(); i != m_mUsers.end(); i++)
    {
        if( (*i).second->IsAuthorized())
        {
            if(excludeAdmins && ((*i).second->GetUserType() & USERTYPE_ADMIN))
                continue;
            else
                users.push_back((*i).second);
        }
    }
    return users;
}

ServerChannel::users_t ServerNode::GetNotificationUsers(const ServerChannel& excludeChannel)
{
    ASSERT_REACTOR_LOCKED(this);

    //vector with users who'll be notified of changes on server
    ServerChannel::users_t notifyusers = GetAdministrators(excludeChannel);

    const ServerChannel::users_t& users = GetAuthorizedUsers(true); //get all users except admins
    for(size_t i=0;i<users.size();i++)
    {
        if((users[i]->GetUserRights() & USERRIGHT_VIEW_ALL_USERS) &&
            !excludeChannel.UserExists(users[i]->GetUserID()))
            notifyusers.push_back(users[i]);
    }
    return notifyusers;
}

ServerChannel::users_t ServerNode::GetNotificationUsers()
{
    ASSERT_REACTOR_LOCKED(this);

    //vector with users who'll be notified of changes on server
    ServerChannel::users_t notifyusers = GetAdministrators();
    const ServerChannel::users_t& users = GetAuthorizedUsers(true); //get all users except admins
    for(size_t i=0;i<users.size();i++)
    {
        if(users[i]->GetUserRights() & USERRIGHT_VIEW_ALL_USERS)
            notifyusers.push_back(users[i]);
    }
    return notifyusers;
}

bool ServerNode::SetFileSharing(const ACE_TString& rootdir)
{
    ASSERT_REACTOR_LOCKED(this);

    ACE_TString fixedpath = FixFilePath(rootdir);
    ACE_Dirent_Selector dir;
    if(dir.open(fixedpath.c_str()) < 0)
        return false;

    m_properties.filesroot = fixedpath;

    return true;    
}

ACE_INT64 ServerNode::GetDiskUsage()
{
    ASSERT_REACTOR_LOCKED(this);

    ACE_INT64 diskusage = 0;

    std::stack<serverchannel_t> sweeper;
    sweeper.push(GetRootChannel());

    while(sweeper.size())
    {
        serverchannel_t chan = sweeper.top();
        sweeper.pop();

        diskusage += chan->GetDiskUsage();

        ServerChannel::channels_t subs = chan->GetSubChannels();
        for(size_t i=0;i<subs.size();i++)
            sweeper.push(subs[i]);
    }
    return diskusage;
}

int ServerNode::GetActiveFileTransfers(int& uploads, int& downloads)
{
    ASSERT_REACTOR_LOCKED(this);

    int tmpDownloads = 0, tmpUploads = 0;
    mapusers_t::const_iterator ite;
    for(ite=m_mUsers.begin();ite!=m_mUsers.end();ite++)
    {
        if(ite->second->GetFileTransferID())
        {
            int transferid = ite->second->GetFileTransferID();
            filetransfers_t::const_iterator fite = m_filetransfers.find(transferid);
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

int ServerNode::GetAuthUserCount()
{
    ASSERT_REACTOR_LOCKED(this);

    int nUserCount = 0;

    // get number of connected users
    for(mapusers_t::iterator ite = m_mUsers.begin();
        ite != m_mUsers.end();
        ite++)
        if(ite->second->IsAuthorized())
            nUserCount++;

    return nUserCount;
}

int ServerNode::GetChannelID(const ACE_TString& chanpath)
{
    GUARD_OBJ(this, lock());

    serverchannel_t chan = ChangeChannel(GetRootChannel(), chanpath);
    if(chan)
        return chan->GetChannelID();
    return 0;
}

bool ServerNode::GetChannelProp(int channelid, ChannelProp& prop)
{
    GUARD_OBJ(this, lock());

    serverchannel_t chan = GetChannel(channelid);
    if(!chan)
        return false;

    prop = chan->GetChannelProp();
    return true;
}

serverchannel_t ServerNode::GetChannel(int channelid) const
{
    ASSERT_REACTOR_LOCKED(this);

    if(!m_rootchannel)
        return serverchannel_t();

    if(m_rootchannel->GetChannelID() == channelid)
        return m_rootchannel;
    else
        return m_rootchannel->GetSubChannel(channelid, true); //SLOW
}

ErrorMsg ServerNode::UserBeginFileTransfer(int transferid, 
                                           FileTransfer& transfer, 
                                           ACE_FILE_IO& file)
{
    GUARD_OBJ(this, lock());

    if(m_properties.filesroot.length() == 0)
        return ErrorMsg(TT_CMDERR_FILESHARING_DISABLED);

    filetransfers_t::iterator ite = m_filetransfers.find(transferid);
    if(ite == m_filetransfers.end())
        return ErrorMsg(TT_CMDERR_FILETRANSFER_NOT_FOUND);

    transfer = ite->second;
    TTASSERT(transfer.filename.length());
    TTASSERT(transfer.localfile.length());

    serverchannel_t chan = GetChannel(transfer.channelid);
    if(!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    if(transfer.inbound && (chan->FileExists(transfer.filename)))
        return ErrorMsg(TT_CMDERR_FILE_ALREADY_EXISTS);

    // don't hold lock while creating file. It could be slow operation
    g.release();

    int openFlag = 0;
    if(transfer.inbound)
        openFlag = O_CREAT | O_RDWR | O_TRUNC;
    else
        openFlag = O_RDONLY;

    ACE_FILE_Connector con;
    if(con.connect(file, ACE_FILE_Addr(transfer.localfile.c_str()), 0, ACE_Addr::sap_any, 0, openFlag) < 0)
        return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);
    else
        return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserEndFileTransfer(int transferid)
{
    GUARD_OBJ(this, lock());

    //file transfer is ok. Now remove it from the list.
    FileTransfer transfer;
    {
        filetransfers_t::iterator ite = m_filetransfers.find(transferid);
        if(ite == m_filetransfers.end())
            return ErrorMsg(TT_CMDERR_FILETRANSFER_NOT_FOUND);
        transfer = ite->second;
        m_filetransfers.erase(ite);
    }

    serverchannel_t chan = GetChannel(transfer.channelid);
    if(!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    serveruser_t user = GetUser(transfer.userid, nullptr);
    if(!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

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
        ACE_OS::snprintf(newfilename, MAX_STRING_LENGTH, ACE_TEXT("data_%x.dat"),
                         (unsigned int)dat_id++);
        local_filename = internalpath + newfilename;
    }
    while(ACE_OS::filesize(local_filename.c_str())>=0 && dat_id != m_file_id_counter);

    internalpath += newfilename;
    if(ACE_OS::rename(transfer.localfile.c_str(), internalpath.c_str()))
        return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);

    remotefile.username = user->GetUsername();
    remotefile.filename = transfer.filename;
    remotefile.internalname = newfilename;
    remotefile.filesize = transfer.filesize;
    remotefile.channelid = chan->GetChannelID();

    ErrorMsg err = AddFileToChannel(remotefile);
    if(err.errorno != TT_CMDERR_SUCCESS)
        return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);

    if(transfer.inbound)
    {
        m_stats.files_bytesreceived += remotefile.filesize;
        m_srvguard->OnFileUploaded(*user, *chan, remotefile);
        if(IsAutoSaving() && (chan->GetChannelType() & CHANNEL_PERMANENT))
            m_srvguard->OnSaveConfiguration(*this, user.get());
    }
    else
    {
        m_stats.files_bytessent += remotefile.filesize;
        m_srvguard->OnFileDownloaded(*user, *chan, remotefile);
    }

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserDeleteFile(int userid, int channelid, 
                                    const ACE_TString& filename)
{
    GUARD_OBJ(this, lock());

    serverchannel_t chan = GetChannel(channelid);
    serveruser_t user = GetUser(userid, nullptr);
    if(!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);
    if(!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);
    if(!chan->FileExists(filename))
        return ErrorMsg(TT_CMDERR_FILE_NOT_FOUND);

    RemoteFile remotefile;
    bool b = chan->GetFile(filename, remotefile);
    TTASSERT(b);

    if( (user->GetUserType() & USERTYPE_ADMIN) || 
        chan->IsOperator(userid) ||
        (remotefile.username == user->GetUsername()))
    {
        ErrorMsg err = RemoveFileFromChannel(filename, chan->GetChannelID());
        if(err.errorno != TT_CMDERR_SUCCESS)
            return err;
    }
    else
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

    m_srvguard->OnFileDeleted(*user, *chan, remotefile);
    if(IsAutoSaving() && chan->GetChannelType() & CHANNEL_PERMANENT)
        m_srvguard->OnSaveConfiguration(*this);

    ACE_TString filepath = m_properties.filesroot + ACE_DIRECTORY_SEPARATOR_STR + remotefile.internalname;
    g.release();

    ACE_FILE_Connector con;
    ACE_FILE_IO file;
    if(con.connect(file, ACE_FILE_Addr(filepath.c_str())) >= 0)
    {
        file.remove();
        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    else
        return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);
}

int ServerNode::StartTimer(ServerTimer timer, timer_userdata userdata, 
                           const ACE_Time_Value& delay, 
                           const ACE_Time_Value& interval)
{
    TimerHandler* th;
    ACE_NEW_RETURN(th, TimerHandler(*this, timer,
                                    userdata), -1);

    return m_timer_reactor->schedule_timer(th, delay, interval);
}

int ServerNode::TimerEvent(ACE_UINT32 timer_event_id, long userdata)
{
    GUARD_OBJ(this, lock());

    switch(timer_event_id)
    {
    case TIMER_ONE_SECOND_ID :
    {
        CheckKeepAlive();

        //update users who change changed UDP port
        serveruser_t user;
        while(m_updUserIPs.size())
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
        ACE_Time_Value now = ACE_OS::gettimeofday();
        for (auto it = m_logindelay.begin();it != m_logindelay.end();)
        {
            if (now > it->second + ToTimeValue(m_properties.logindelay * 2))
                m_logindelay.erase(it++);
            else
                ++it;
        }
        break;
    }
    case TIMER_DESKTOPACKPACKET_ID :
        SendDesktopAckPacket(userdata);
        return -1;
    case TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID :
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
    case TIMER_START_DESKTOPTX_ID :
    {
        timer_userdata tm_data;
        tm_data.userdata = userdata;
        serveruser_t src_user = GetUser(tm_data.src_userid, nullptr);
        serveruser_t dest_user = GetUser(tm_data.dest_userid, src_user.get());
        if(src_user && dest_user)
        {
            serverchannel_t chan = src_user->GetChannel();
            if(chan && chan == dest_user->GetChannel())
                StartDesktopTransmitter(*src_user, *dest_user, *chan);
        }
        return -1;
    }
    case TIMER_CLOSE_DESKTOPSESSION_ID :
    {
        //continously notify the user that the desktop session has
        //ended
        timer_userdata tm_data;
        tm_data.userdata = userdata;
        serveruser_t src_user = GetUser(tm_data.src_userid, nullptr);
        serveruser_t dest_user = GetUser(tm_data.dest_userid, src_user.get());
        if(src_user && dest_user)
        {
            ClosedDesktopSession session;
            if(!dest_user->GetClosedDesktopSession(src_user->GetUserID(), 
                                                   session))
                return -1;

            serverchannel_t chan = src_user->GetChannel();
            if(!chan)
                return -1;

            DesktopNakPacket nak_pkt(src_user->GetUserID(),
                                     session.update_id,
                                     session.session_id);
            nak_pkt.SetChannel(chan->GetChannelID());
#if defined(ENABLE_ENCRYPTION)
            if(m_crypt_acceptors.size())
            {
                CryptDesktopNakPacket crypt_pkt(nak_pkt, chan->GetEncryptKey());
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
    case TIMER_COMMAND_RESUME :
    {
        timer_userdata tm_data;
        tm_data.userdata = userdata;
        serveruser_t src_user = GetUser(tm_data.src_userid, nullptr);
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
    ASSERT_REACTOR_LOCKED(this);

    TTASSERT(m_desktop_ack_timers.find(userid) != m_desktop_ack_timers.end());

    m_desktop_ack_timers.erase(userid);

    serveruser_t tmp_user = GetUser(userid, nullptr);
    if(!tmp_user)
        return false;

    ServerUser& user = *tmp_user;
    serverchannel_t tmp_chan = user.GetChannel();
    if (!tmp_chan)
        return false;

    ServerChannel& chan = *tmp_chan;

    set<uint16_t> recv_packets;
    uint32_t time_ack;
    uint8_t session_id;

    if (!user.GetDesktopSession())
    {
        //check current packet queue and ack
        const desktoppackets_t& session_q = user.GetDesktopSessionQueue();
        desktoppackets_t::const_iterator ii = session_q.begin();
        
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
        DesktopCache& desktop = *user.GetDesktopSession();
        session_id = desktop.GetSessionID();
        time_ack = desktop.GetPendingUpdateTime();

        desktop.GetReceivedPackets(time_ack, recv_packets);
    }

    packet_range_t recv_range;
    set<uint16_t> recv_single;
    GetPacketRanges(recv_packets, recv_range, recv_single);

    DesktopAckPacket ack_pkt(0, GETTIMESTAMP(), user.GetUserID(), 
                             session_id, time_ack, recv_single, recv_range);
    ack_pkt.SetChannel(chan.GetChannelID());
#if defined(ENABLE_ENCRYPTION)
    if(m_crypt_acceptors.size())
    {
        CryptDesktopAckPacket crypt_pkt(ack_pkt, chan.GetEncryptKey());
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
    if(recv_single.size())
        max_packet = *(--recv_single.end());
    if(recv_range.size())
    {
        packet_range_t::const_iterator ii=recv_range.end();
        ii--;
        max_packet = ACE_MAX(max_packet, ii->second);
    }

//     MYTRACE(ACE_TEXT("Ack sent %u, mac packet index %d\n"), GETTIMESTAMP(), 
//             max_packet);


    return true;
}

bool ServerNode::RetransmitDesktopPackets(int src_userid, int dest_userid)
{
    ASSERT_REACTOR_LOCKED(this);

    serveruser_t src_user = GetUser(src_userid, nullptr);
    if(!src_user)
        return false;

    serveruser_t dest_user = GetUser(dest_userid, src_user.get());
    if(!dest_user)
        return false;

    desktop_transmitter_t desktop_tx = dest_user->GetDesktopTransmitter(src_userid);
    if (!desktop_tx)
        return false;

    serverchannel_t chan = src_user->GetChannel();
    if(!chan)
        return false;

    TTASSERT(chan == dest_user->GetChannel());

    desktoppackets_t rtx_packets;
    desktop_tx->GetLostDesktopPackets(DESKTOP_DEFAULT_RTX_TIMEOUT, rtx_packets, 1);
    desktoppackets_t::iterator dpi = rtx_packets.begin();
//     MYTRACE_COND(dpi == rtx_packets.end(), ACE_TEXT("No packets for RTO\n"));
    for(;dpi != rtx_packets.end();dpi++)
    {
        TTASSERT(chan->GetChannelID() == (*dpi)->GetChannel());
#if defined(ENABLE_ENCRYPTION)
        if(m_crypt_acceptors.size())
        {
            CryptDesktopPacket crypt_pkt(*(*dpi), chan->GetEncryptKey());
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
    ASSERT_REACTOR_LOCKED(this);

    if (!src_user.GetDesktopSession())
        return false;
    if((chan.GetChannelType() & CHANNEL_OPERATOR_RECVONLY) &&
       !chan.IsOperator(src_user.GetUserID()) &&
       (src_user.GetUserType() & USERTYPE_ADMIN) == 0 &&
       !chan.IsOperator(dest_user.GetUserID()) &&
       (dest_user.GetUserType() & USERTYPE_ADMIN) == 0)
       return false;

    DesktopCache& desktop = *src_user.GetDesktopSession();
    if(!desktop.IsReady())
        return false;
    uint8_t session_id = desktop.GetSessionID();
    uint32_t update_time = desktop.GetCurrentDesktopTime();
    
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
    desktoppackets_t::iterator dpi = tx_packets.begin();
    while(dpi != tx_packets.end())
    {
#if defined(ENABLE_ENCRYPTION)
        if(m_crypt_acceptors.size())
        {
            CryptDesktopPacket crypt_pkt(*(*dpi), chan.GetEncryptKey());
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
    if(m_desktop_rtx_timers.find(tm_data.userdata) != m_desktop_rtx_timers.end())
        return true;

    //start RTX timer
    TimerHandler* th;
    ACE_NEW_NORETURN(th, TimerHandler(*this,
                                      TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID,
                                      tm_data.userdata));
    if(th)
    {
        long timerid = m_timer_reactor->schedule_timer(th, 0, 
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
    ASSERT_REACTOR_LOCKED(this);

    dest_user.CloseDesktopTransmitter(src_user.GetUserID(), start_nak_timer);

    timer_userdata tm_data;
    tm_data.dest_userid = dest_user.GetUserID();
    tm_data.src_userid = src_user.GetUserID();

    user_desktoppacket_rtx_t::iterator uti = m_desktop_rtx_timers.find(tm_data.userdata);
    if(uti != m_desktop_rtx_timers.end())
    {
        m_timer_reactor->cancel_timer(uti->second, 0, 0);
        m_desktop_rtx_timers.erase(uti);
    }

    if(start_nak_timer)
    {
        TimerHandler* th;
        timer_userdata tm_data;
        tm_data.src_userid = src_user.GetUserID();
        tm_data.dest_userid = dest_user.GetUserID();
        ACE_NEW_NORETURN(th, TimerHandler(*this, TIMER_CLOSE_DESKTOPSESSION_ID,
                                          tm_data.userdata));
        long timerid = m_timer_reactor->schedule_timer(th, 0, ACE_Time_Value(1));
        TTASSERT(timerid>=0);
    }
}

bool ServerNode::StartServer(bool encrypted, const ACE_TString& sysid)
{
    GUARD_OBJ(this, lock());

    //don't allow server to start if there's no root channel specified
    if(!m_rootchannel)
        return false;

    bool tcpport = m_properties.tcpaddrs.size() > 0, udpport = m_properties.udpaddrs.size() > 0;
    for (auto a : m_properties.tcpaddrs)
    {
#if defined(ENABLE_ENCRYPTION)
        if (encrypted)
        {
            cryptacceptor_t ca(new CryptAcceptor(a, m_tcp_reactor, ACE_NONBLOCK, this));
            tcpport &= ca->acceptor().get_handle() != ACE_INVALID_HANDLE;
            m_crypt_acceptors.push_back(ca);
        }
        else
#endif
        {
            defaultacceptor_t da(new DefaultAcceptor(a, m_tcp_reactor, ACE_NONBLOCK, this));
            tcpport &= da->acceptor().get_handle() != ACE_INVALID_HANDLE;
            m_def_acceptors.push_back(da);
        }
    }

    for (auto a : m_properties.udpaddrs)
    {
        packethandler_t ph(new PacketHandler(m_udp_reactor));
        udpport &= ph->open(a, UDP_SOCKET_RECV_BUF_SIZE, UDP_SOCKET_SEND_BUF_SIZE);
        if (udpport)
            ph->AddListener(this);
        m_packethandlers.push_back(ph);
    }

    if(tcpport && udpport)
    {
        //reset stats
        m_stats = ServerStats();

        //start keepalive timer
        ACE_Time_Value interval(SERVER_KEEPALIVE_DELAY);
        TimerHandler* th = new TimerHandler(*this, TIMER_ONE_SECOND_ID);
        m_onesec_timerid = m_timer_reactor->schedule_timer(th, 0, interval, interval);
        TTASSERT(m_onesec_timerid>=0);

        //uptime
        m_stats.starttime = ACE_OS::gettimeofday();
        //init random numbers
        ACE_OS::srand(ACE_OS::gettimeofday().msec());

        // system id in welcome message
        if(sysid.length())
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
    GUARD_OBJ(this, lock());

    m_timer_reactor->cancel_timer(m_onesec_timerid, 0, 0);
    m_onesec_timerid = -1;
    //close sockets
    //disconnect users
    while(m_mUsers.size())
    {
        ACE_HANDLE h = m_mUsers.begin()->second->ResetStreamHandle();
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

serveruser_t ServerNode::GetUser(int userid, const ServerUser* caller)
{
    ASSERT_REACTOR_LOCKED(this);

    //find user with userid
    mapusers_t::iterator ite = m_mUsers.find(userid);
    serveruser_t user;
    if(ite != m_mUsers.end())
        user = (*ite).second;
    
    if (caller && user && caller != user.get())
    {
        if ((caller->GetUserRights() & USERRIGHT_VIEW_ALL_USERS) == USERRIGHT_NONE &&
            caller->GetChannel() != user->GetChannel())
            return serveruser_t();
    }

    return user;
}

int ServerNode::GetNewUserID()
{
    ASSERT_REACTOR_LOCKED(this);

    bool found = false;
    bool overflowed = false;

    while(!found)
    {
        mapusers_t::iterator ite = m_mUsers.find(++m_userid_counter);
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
    if(handler)
    {
        int ret = m_tcp_reactor->register_handler(handler, ACE_Event_Handler::WRITE_MASK);
        TTASSERT(ret >= 0);
    }
    return handler;
}

void ServerNode::OnOpened(ACE_HANDLE h, serveruser_t& user)
{
    ASSERT_REACTOR_LOCKED(this);

    m_mUsers[user->GetUserID()] = user;
    user->SetLastKeepAlive(0);
    m_streamhandles[h] = user;

    user->DoWelcome(m_properties);
    m_srvguard->OnUserConnected(*user);
}

#if defined(ENABLE_ENCRYPTION)
void ServerNode::OnOpened(CryptStreamHandler::StreamHandler_t& handler)
{
    GUARD_OBJ(this, lock());

    int userid = GetNewUserID();
    if(userid != 0)
    {
        ServerUser* ptr;
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
        int ret = ACE_OS::setsockopt(handler.peer().get_handle(), SOL_SOCKET, 
                                     SO_KEEPALIVE, (char*)&val, sizeof(val));
        TTASSERT(ret != -1);

        OnOpened(handler.get_handle(), user);
    }
}
#endif

void ServerNode::OnOpened(DefaultStreamHandler::StreamHandler_t& handler)
{
    GUARD_OBJ(this, lock());

    int userid = GetNewUserID();
    if(userid != 0)
    {
        ServerUser* ptr;
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
        int ret = ACE_OS::setsockopt(handler.peer().get_handle(), SOL_SOCKET, 
                                     SO_KEEPALIVE, (char*)&val, sizeof(val));
        TTASSERT(ret != -1);

        OnOpened(handler.get_handle(), user);
    }
}

void ServerNode::OnClosed(ACE_HANDLE h)
{
    TTASSERT(m_streamhandles.find(h) != m_streamhandles.end());
    serveruser_t user = m_streamhandles[h];
    TTASSERT(user.get());
    if(user.get())
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

    serveruser_t user = m_streamhandles[h];
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
    serveruser_t user = m_streamhandles[handler.get_handle()];
    if(user)
        return user->SendData(*handler.msg_queue());
    return false;
}

#endif

bool ServerNode::OnSend(DefaultStreamHandler::StreamHandler_t& handler)
{
    TTASSERT(m_streamhandles.find(handler.get_handle()) != m_streamhandles.end());
    serveruser_t user = m_streamhandles[handler.get_handle()];
    if(user)
        return user->SendData(*handler.msg_queue());
    return false;
}

void ServerNode::IncLoginAttempt(const ServerUser& user)
{
    ASSERT_REACTOR_LOCKED(this);

    //ban user's IP if logged in to many times
    m_failedlogins[user.GetIpAddress()].push_back(ACE_OS::gettimeofday());

    if (m_properties.maxloginattempts > 0 && 
        m_failedlogins[user.GetIpAddress()].size() >= (size_t)m_properties.maxloginattempts)
    {
        BannedUser ban;
        ban.bantype = BANTYPE_DEFAULT;
        ban.ipaddr = user.GetIpAddress();
        m_srvguard->AddUserBan(user, ban);
        if(IsAutoSaving())
            m_srvguard->OnSaveConfiguration(*this, &user);
    }
}

bool ServerNode::LoginsExceeded(const ServerUser& user)
{
    ASSERT_REACTOR_LOCKED(this);

    if (m_properties.logindelay == 0)
        return false;

    ACE_Time_Value now = ACE_OS::gettimeofday();
    if (m_logindelay.find(user.GetIpAddress()) == m_logindelay.end())
    {
        m_logindelay[user.GetIpAddress()] = now;
        return false;
    }

    ACE_Time_Value delay = ToTimeValue(m_properties.logindelay);
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
    int buffers, ret = -1;
    const iovec* vv = packet.GetPacket(buffers);
    TTASSERT(packet.Finalized() || packet.GetKind() == PACKET_KIND_HELLO || packet.GetKind() == PACKET_KIND_KEEPALIVE);
    for (auto& ph : m_packethandlers)
    {
        if (ph->GetLocalAddr() == localaddr)
        {
            SocketOptGuard sog(ph->sock_i(), IPPROTO_IP, IP_TOS,
                               ToIPTOSValue(packet));
            
            if (m_properties.txloss && ((m_stats.packets_sent % m_properties.txloss) == 0))
            {
                ret = packet.GetPacketSize();
                MYTRACE(ACE_TEXT("Simulated TX dropped packet. Kind %d\n"), int(packet.GetKind()));
            }
            else
            {
                ret = int(ph->sock_i().send(vv, buffers, remoteaddr));
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
    wguard_t g(m_sendmutex);

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
    for(size_t i=0;i<users.size();i++)
    {
        //check that bandwidth limits are not exceeded
        switch(packet.GetKind())
        {
        case PACKET_KIND_VOICE :
        case PACKET_KIND_VOICE_CRYPT :
            if(m_properties.voicetxlimit && 
               m_stats.voice_bytessent + packet.GetPacketSize() >
               m_stats.last_voice_bytessent + m_properties.voicetxlimit)
               continue;
            break;
        case PACKET_KIND_VIDEO :
        case PACKET_KIND_VIDEO_CRYPT :
            if(m_properties.videotxlimit && 
               m_stats.vidcap_bytessent + packet.GetPacketSize() >
               m_stats.last_vidcap_bytessent + m_properties.videotxlimit)
               continue;
            break;
        case PACKET_KIND_MEDIAFILE_AUDIO :
        case PACKET_KIND_MEDIAFILE_AUDIO_CRYPT :
        case PACKET_KIND_MEDIAFILE_VIDEO :
        case PACKET_KIND_MEDIAFILE_VIDEO_CRYPT :
            if(m_properties.mediafiletxlimit && 
               m_stats.mediafile_bytessent + packet.GetPacketSize() >
               m_stats.last_mediafile_bytessent + m_properties.mediafiletxlimit)
               continue;
            break;
        case PACKET_KIND_DESKTOP :
        case PACKET_KIND_DESKTOP_CRYPT :
            if(m_properties.desktoptxlimit &&
               m_stats.desktop_bytessent + packet.GetPacketSize() >
               m_stats.last_desktop_bytessent + m_properties.desktoptxlimit)
                continue;
        }

        if(m_properties.totaltxlimit && 
           m_stats.total_bytessent + packet.GetPacketSize() >
           m_stats.last_bytessent + m_properties.totaltxlimit)
            continue;

        //ok to send packet
        int ret = SendPacket(packet, *users[i]);
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
            TTASSERT(m_def_acceptors.size());
            break;
        case PACKET_KIND_VOICE_CRYPT :
            m_stats.voice_bytessent += ret;
#if defined(ENABLE_ENCRYPTION)
            TTASSERT(m_crypt_acceptors.size());
#endif
            break;
        case PACKET_KIND_VIDEO :
            m_stats.vidcap_bytessent += ret;
            TTASSERT(m_def_acceptors.size());
            break;
        case PACKET_KIND_VIDEO_CRYPT :
            m_stats.vidcap_bytessent += ret;
#if defined(ENABLE_ENCRYPTION)
            TTASSERT(m_crypt_acceptors.size());
#endif
            break;
        case PACKET_KIND_MEDIAFILE_AUDIO :
        case PACKET_KIND_MEDIAFILE_VIDEO :
            m_stats.mediafile_bytessent += ret;
            TTASSERT(m_def_acceptors.size());
            break;
        case PACKET_KIND_MEDIAFILE_AUDIO_CRYPT :
        case PACKET_KIND_MEDIAFILE_VIDEO_CRYPT :
            m_stats.mediafile_bytessent += ret;
#if defined(ENABLE_ENCRYPTION)
            TTASSERT(m_crypt_acceptors.size());
#endif
            break;
        case PACKET_KIND_DESKTOP :
        case PACKET_KIND_DESKTOP_ACK :
        case PACKET_KIND_DESKTOP_NAK :
        case PACKET_KIND_DESKTOPCURSOR :
        case PACKET_KIND_DESKTOPINPUT :
        case PACKET_KIND_DESKTOPINPUT_ACK :
            m_stats.desktop_bytessent += ret;
            TTASSERT(m_def_acceptors.size());
            break;
        case PACKET_KIND_DESKTOP_CRYPT :
        case PACKET_KIND_DESKTOP_ACK_CRYPT :
        case PACKET_KIND_DESKTOP_NAK_CRYPT :
        case PACKET_KIND_DESKTOPCURSOR_CRYPT :
        case PACKET_KIND_DESKTOPINPUT_CRYPT :
        case PACKET_KIND_DESKTOPINPUT_ACK_CRYPT :
            m_stats.desktop_bytessent += ret;
#if defined(ENABLE_ENCRYPTION)
            TTASSERT(m_crypt_acceptors.size());
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
    GUARD_OBJ(this, lock());

    m_stats.packets_received++;
    
    FieldPacket packet(packet_data, packet_size);

    if (m_properties.rxloss && ((m_stats.packets_received % m_properties.rxloss) == 0))
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

    serveruser_t user = GetUser(packet.GetSrcUserID(), nullptr);
    if(!user)
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

    ACE_INET_Addr localaddr = ph->GetLocalAddr();
    switch(packet.GetKind())
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
        if(user->GetUserRights() & USERRIGHT_TRANSMIT_VOICE)
            ReceivedVoicePacket(*user, CryptVoicePacket(packet_data, packet_size), 
                                remoteaddr, localaddr);
        m_stats.voice_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_VOICE :
        if(user->GetUserRights() & USERRIGHT_TRANSMIT_VOICE)
            ReceivedVoicePacket(*user, VoicePacket(packet_data, packet_size), 
                                remoteaddr, localaddr);
        m_stats.voice_bytesreceived += packet_size;
        break;
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_MEDIAFILE_AUDIO_CRYPT :
        if(user->GetUserRights() & USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO)
            ReceivedAudioFilePacket(*user, CryptAudioFilePacket(packet_data, packet_size), 
                                    remoteaddr, localaddr);
        m_stats.mediafile_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_MEDIAFILE_AUDIO :
        if(user->GetUserRights() & USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO)
            ReceivedAudioFilePacket(*user, AudioFilePacket(packet_data, packet_size), 
                                    remoteaddr, localaddr);
        m_stats.mediafile_bytesreceived += packet_size;
        break;
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_VIDEO_CRYPT :
        if(user->GetUserRights() & USERRIGHT_TRANSMIT_VIDEOCAPTURE)
            ReceivedVideoCapturePacket(*user, CryptVideoCapturePacket(packet_data, packet_size), 
                                       remoteaddr, localaddr);
        m_stats.vidcap_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_VIDEO :
        if(user->GetUserRights() & USERRIGHT_TRANSMIT_VIDEOCAPTURE)
            ReceivedVideoCapturePacket(*user, VideoCapturePacket(packet_data, packet_size), 
                                       remoteaddr, localaddr);
        m_stats.vidcap_bytesreceived += packet_size;
        break;
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_MEDIAFILE_VIDEO_CRYPT :
        if(user->GetUserRights() & USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO)
            ReceivedVideoFilePacket(*user, CryptVideoFilePacket(packet_data, packet_size), 
                                    remoteaddr, localaddr);
        m_stats.mediafile_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_MEDIAFILE_VIDEO :
        if(user->GetUserRights() & USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO)
            ReceivedVideoFilePacket(*user, VideoFilePacket(packet_data, packet_size), 
                                    remoteaddr, localaddr);
        m_stats.mediafile_bytesreceived += packet_size;
        break;
#if defined(ENABLE_ENCRYPTION)
    case PACKET_KIND_DESKTOP_CRYPT :
        if(user->GetUserRights() & USERRIGHT_TRANSMIT_DESKTOP)
            ReceivedDesktopPacket(*user,
                                  CryptDesktopPacket(packet_data, packet_size), 
                                  remoteaddr, localaddr);
        m_stats.desktop_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_DESKTOP :
        if(user->GetUserRights() & USERRIGHT_TRANSMIT_DESKTOP)
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
        if(user->GetUserRights() & USERRIGHT_TRANSMIT_DESKTOPINPUT)
            ReceivedDesktopInputPacket(*user,
                                       CryptDesktopInputPacket(packet_data, packet_size),
                                       remoteaddr, localaddr);
        m_stats.desktop_bytesreceived += packet_size;
        break;
#endif
    case PACKET_KIND_DESKTOPINPUT :
        if(user->GetUserRights() & USERRIGHT_TRANSMIT_DESKTOPINPUT)
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
    ASSERT_REACTOR_LOCKED(this);

    int userid = packet.GetSrcUserID();
    int version = packet.GetProtocol();

    //set client properties
    if(user.GetUdpAddress() != remoteaddr && user.GetUdpAddress() != ACE_INET_Addr())
        m_updUserIPs.insert(user.GetUserID());

    user.SetUdpAddress(remoteaddr, localaddr);
    user.SetPacketProtocol(version);

    //send acknowledge packet
    HelloPacket ackpacket((uint16_t)0, packet.GetTime());
    SendPacket(ackpacket, user);
}

void ServerNode::ReceivedKeepAlivePacket(ServerUser& user, 
                                         const KeepAlivePacket& packet, 
                                         const ACE_INET_Addr& remoteaddr,
                                         const ACE_INET_Addr& localaddr)
{
    ASSERT_REACTOR_LOCKED(this);

    int userid = packet.GetSrcUserID();

    //check if it's a MTU query keep alive packet
    int buffers = 0;
    const iovec* v = packet.GetPacket(buffers);
    KeepAlivePacket ka_pkt(v, buffers);
    uint16_t payload_data_size = ka_pkt.GetPayloadSize();
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

    KeepAlivePacket reply((uint16_t)0, packet.GetTime());
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
    ASSERT_REACTOR_LOCKED(this);

    uint16_t chanid = packet.GetChannel();
    if(!chanid)
        return serverchannel_t();

    serverchannel_t chan = user.GetChannel();
    if(!chan || chan->GetChannelID() != chanid)
    {
        //only admins can stream outside their channels
        if((user.GetUserType() & USERTYPE_ADMIN) == 0)
            return serverchannel_t();

        //this branch will only happen if a user is streaming outside his channel
        TTASSERT(m_rootchannel.get());
        chan = GetChannel(chanid);
        if(!chan)
            return serverchannel_t();
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
    ASSERT_REACTOR_LOCKED(this);

    ACE_UINT8 pp_min = TEAMTALK_DEFAULT_PACKET_PROTOCOL;
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
    uint16_t dest_userid = packet.GetDestUserID();
    int fromuserid = user.GetUserID();
    const ServerChannel::users_t& users = channel.GetUsers();

    if(dest_userid) //the packet is only for certain users
    {
        for(size_t i=0; i < users.size(); i++)
        {
            if(users[i]->GetUserID() == dest_userid &&
                (users[i]->GetSubscriptions(user) & subscrip_check) &&
                users[i]->GetPacketProtocol() >= pp_min)
            {
                result.push_back(users[i]);
            }
        }

        //admins can also subscribe outside their channels
        const ServerChannel::users_t& admins = GetAdministrators();
        for(size_t i=0;i<admins.size();i++)
        {
            if( (admins[i]->GetSubscriptions(user) & intercept_check) &&
                !channel.UserExists(admins[i]->GetUserID()) &&
                admins[i]->GetPacketProtocol() >= pp_min)
            {
                result.push_back(users[i]);
            }
        }
    }
    else if(packet.GetChannel())
    {
        if((channel.GetChannelType() & CHANNEL_OPERATOR_RECVONLY) &&
            !channel.IsOperator(fromuserid) && 
            (user.GetUserType() & USERTYPE_ADMIN) == 0)
        {
            //only operators and admins will receive from default users
            //in channel type CHANNEL_OPERATOR_RECVONLY
            for(size_t i=0;i<users.size();i++)
            {
                if((channel.IsOperator(users[i]->GetUserID()) ||
                    users[i]->GetUserType() & USERTYPE_ADMIN) &&
                    (users[i]->GetSubscriptions(user) & subscrip_check) &&
                    users[i]->GetPacketProtocol() >= pp_min)
                {
                    result.push_back(users[i]);
                }
            }
        }
        else
        {
            //forward to all users in same channel
            for(size_t i=0; i < users.size(); i++)
            {
                if((users[i]->GetSubscriptions(user) & subscrip_check) &&
                   users[i]->GetPacketProtocol() >= pp_min)
                {
                    result.push_back(users[i]);
                }
            }
        }

        //admins can also subscribe outside their channels
        const ServerChannel::users_t& admins = GetAdministrators();
        for(size_t i=0;i<admins.size();i++)
        {
            if( (admins[i]->GetSubscriptions(user) & intercept_check) &&
                !channel.UserExists(admins[i]->GetUserID()) &&
                admins[i]->GetPacketProtocol() >= pp_min)
            {
                result.push_back(admins[i]);
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
    ASSERT_REACTOR_LOCKED(this);

    serverchannel_t tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
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

    std::vector<int> txqueue = chan.GetTransmitQueue();
    bool tx_ok = chan.CanTransmit(user.GetUserID(), STREAMTYPE_VOICE, streamid);

    if((chan.GetChannelType() & CHANNEL_SOLO_TRANSMIT) &&
       txqueue != chan.GetTransmitQueue())
    {
        UpdateChannel(chan, chan.GetUsers());
    }

    if(!tx_ok)
        return;

    ServerChannel::users_t users = GetPacketDestinations(user, chan, packet,
                                                         SUBSCRIBE_VOICE,
                                                         SUBSCRIBE_INTERCEPT_VOICE);

    SendPackets(packet, users);
}

void ServerNode::ReceivedAudioFilePacket(ServerUser& user, 
                                         const FieldPacket& packet, 
                                         const ACE_INET_Addr& remoteaddr,
                                         const ACE_INET_Addr& localaddr)
{
    ASSERT_REACTOR_LOCKED(this);

    serverchannel_t tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
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

    std::vector<int> txqueue = chan.GetTransmitQueue();
    bool tx_ok = chan.CanTransmit(user.GetUserID(), STREAMTYPE_MEDIAFILE, streamid);

    if((chan.GetChannelType() & CHANNEL_SOLO_TRANSMIT) &&
       txqueue != chan.GetTransmitQueue())
    {
        UpdateChannel(chan, chan.GetUsers());
    }

    if(!tx_ok)
        return;

    ServerChannel::users_t users = GetPacketDestinations(user, chan, packet, SUBSCRIBE_MEDIAFILE,
                                                         SUBSCRIBE_INTERCEPT_MEDIAFILE);

    SendPackets(packet, users);
}

void ServerNode::ReceivedVideoCapturePacket(ServerUser& user, 
                                            const FieldPacket& packet, 
                                            const ACE_INET_Addr& remoteaddr,
                                            const ACE_INET_Addr& localaddr)
{
    ASSERT_REACTOR_LOCKED(this);

    serverchannel_t tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
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

    if(!chan.CanTransmit(user.GetUserID(), STREAMTYPE_VIDEOCAPTURE, streamid))
        return;

    ServerChannel::users_t users = GetPacketDestinations(user, chan, packet, SUBSCRIBE_VIDEOCAPTURE,
                                                         SUBSCRIBE_INTERCEPT_VIDEOCAPTURE);
    
    SendPackets(packet, users);
}


void ServerNode::ReceivedVideoFilePacket(ServerUser& user, 
                                         const FieldPacket& packet, 
                                         const ACE_INET_Addr& remoteaddr,
                                         const ACE_INET_Addr& localaddr)
{
    ASSERT_REACTOR_LOCKED(this);

    serverchannel_t tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
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
    
    std::vector<int> txqueue = chan.GetTransmitQueue();
    bool tx_ok = chan.CanTransmit(user.GetUserID(), STREAMTYPE_MEDIAFILE, streamid);
    
    if((chan.GetChannelType() & CHANNEL_SOLO_TRANSMIT) &&
       txqueue != chan.GetTransmitQueue())
    {
        UpdateChannel(chan, chan.GetUsers());
    }

    if(!tx_ok)
        return;

    ServerChannel::users_t users = GetPacketDestinations(user, chan, packet, SUBSCRIBE_MEDIAFILE,
                                                         SUBSCRIBE_INTERCEPT_MEDIAFILE);

    SendPackets(packet, users);
}

#if defined(ENABLE_ENCRYPTION)
void ServerNode::ReceivedDesktopPacket(ServerUser& user, 
                                       const CryptDesktopPacket& crypt_pkt, 
                                       const ACE_INET_Addr& remoteaddr,
                                       const ACE_INET_Addr& localaddr)
{
    serverchannel_t tmp_chan = GetPacketChannel(user, crypt_pkt, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;

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
    ASSERT_REACTOR_LOCKED(this);

    serverchannel_t tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;

    if(!chan.CanTransmit(user.GetUserID(), STREAMTYPE_DESKTOP, packet.GetSessionID()))
       return;

    uint8_t prev_session_id = 0;
    uint32_t prev_update_id = 0;
    bool prev_session_ready = false;
    if (user.GetDesktopSession())
    {
        prev_session_id = user.GetDesktopSession()->GetSessionID();
        prev_update_id = user.GetDesktopSession()->GetCurrentDesktopTime();
        prev_session_ready = user.GetDesktopSession()->IsReady();
    }

    bool add_pkt = user.AddDesktopPacket(packet);
    MYTRACE_COND(!add_pkt, ACE_TEXT("Failed to DesktopPacket %d to session %d:%u\n"),
                 packet.GetPacketIndex(), packet.GetSessionID(), packet.GetTime());

    //start delayed ack
    int userid = user.GetUserID();
    if(m_desktop_ack_timers.find(userid) == m_desktop_ack_timers.end())
    {
        ACE_Time_Value interval(0, 2000); //delayed ack time
        TimerHandler* th;
        ACE_NEW(th, TimerHandler(*this, TIMER_DESKTOPACKPACKET_ID, userid));
        long timerid = m_timer_reactor->schedule_timer(th, 0, interval, interval);
        if(timerid >= 0)
            m_desktop_ack_timers[userid] = timerid;
    }

    //don't forward the desktop packet if no session is ready
    if (!user.GetDesktopSession())
        return;

    DesktopCache& desktop = *user.GetDesktopSession();
    uint8_t session_id = desktop.GetSessionID();
    uint32_t update_time = desktop.GetCurrentDesktopTime();
    bool session_ready = desktop.IsReady();

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
    serverchannel_t user_chan = user.GetChannel();
    if(!user_chan || user_chan->GetChannelID() != chan.GetChannelID())
    {
        MYTRACE(ACE_TEXT("Ignored desktop packet from #%d to %s\n"),
                user.GetUserID(), chan.GetChannelPath().c_str());
        return;
    }

    vector<ACE_INET_Addr> addrs;
    ServerChannel::users_t users = GetPacketDestinations(user, chan, packet, SUBSCRIBE_DESKTOP,
                                                         SUBSCRIBE_INTERCEPT_DESKTOP);

    for(auto ui=users.begin();ui!=users.end();ui++)
    {
        desktop_transmitter_t dtx = (*ui)->GetDesktopTransmitter(user.GetUserID());
        if (dtx)
        {
            timer_userdata tm_data;
            tm_data.dest_userid = (*ui)->GetUserID();
            tm_data.src_userid = user.GetUserID();

            //check for existing RTX timer
            TTASSERT(dtx->Done() || m_desktop_rtx_timers.find(tm_data.userdata) != m_desktop_rtx_timers.end());

            //close existing transmitters if new session is started
            if(dtx->GetSessionID() != desktop.GetSessionID())
                StopDesktopTransmitter(user, *(*ui), false);
            else if(!dtx->Done())
                continue; //skip new transmitter if one is already active
        }
        MYTRACE(ACE_TEXT("Starting new DTX for #%d\n"), (*ui)->GetUserID());
        //start or resume transmitter
        StartDesktopTransmitter(user, *(*ui), chan);
    }
}

#if defined(ENABLE_ENCRYPTION)
void ServerNode::ReceivedDesktopAckPacket(ServerUser& user, 
                                          const CryptDesktopAckPacket& crypt_pkt, 
                                          const ACE_INET_Addr& remoteaddr,
                                          const ACE_INET_Addr& localaddr)
{
    serverchannel_t tmp_chan = GetPacketChannel(user, crypt_pkt, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;

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
    ASSERT_REACTOR_LOCKED(this);

    serverchannel_t tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;

    uint16_t owner_userid;
    uint8_t session_id; 
    uint32_t upd_time;

    if(!packet.GetSessionInfo(&owner_userid, &session_id, &upd_time))
        return;

    desktop_transmitter_t dtx = user.GetDesktopTransmitter(owner_userid);
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
        if(m_crypt_acceptors.size())
        {
            CryptDesktopNakPacket crypt_pkt(nak_pkt, chan.GetEncryptKey());
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

    desktoppackets_t::iterator dpi = tx_packets.begin();
    while(dpi != tx_packets.end())
    {
#if defined(ENABLE_ENCRYPTION)
        if(m_crypt_acceptors.size())
        {
            CryptDesktopPacket crypt_pkt(*(*dpi), chan.GetEncryptKey());
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

        user_desktoppacket_rtx_t::iterator ii = m_desktop_rtx_timers.find(tm_data.userdata);
        if(ii != m_desktop_rtx_timers.end())
        {
            m_timer_reactor->cancel_timer(ii->second, 0, 0);
            m_desktop_rtx_timers.erase(ii);
        }
        MYTRACE(ACE_TEXT("Desktop TX update %d:%u completed for #%d\n"),
            dtx->GetSessionID(), dtx->GetUpdateID(), user.GetUserID());

        //check if a new update is ready
        serveruser_t src_user = GetUser(owner_userid, &user);
        if(src_user)
        {
            desktop_cache_t desktop = src_user->GetDesktopSession();
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
    serverchannel_t tmp_chan = GetPacketChannel(user, crypt_pkt, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;

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
    serverchannel_t tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;

    desktop_cache_t desktop = user.GetDesktopSession();
    if (desktop && desktop->GetSessionID() == packet.GetSessionID())
    {
        user.CloseDesktopSession();
        MYTRACE(ACE_TEXT("Close desktop session %d for user #%d\n"),
                packet.GetSessionID(), user.GetUserID());
    }
    vector<ACE_INET_Addr> addrs;
    ServerChannel::users_t users = GetPacketDestinations(user, chan, packet, SUBSCRIBE_DESKTOP,
                                                         SUBSCRIBE_INTERCEPT_DESKTOP);

    for(auto ui=users.begin();ui!=users.end();ui++)
        StopDesktopTransmitter(user, *(*ui), true);

    DesktopAckPacket ack_pkt(0, GETTIMESTAMP(), user.GetUserID(), 
                             packet.GetSessionID(), packet.GetTime(),
                             set<uint16_t>(), packet_range_t());
    ack_pkt.SetChannel(chan.GetChannelID());
#if defined(ENABLE_ENCRYPTION)
    if(m_crypt_acceptors.size())
    {
        CryptDesktopAckPacket crypt_ackpkt(ack_pkt, chan.GetEncryptKey());
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
    serverchannel_t tmp_chan = GetPacketChannel(user, crypt_pkt, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;

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
    serverchannel_t tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;

    if(!chan.CanTransmit(user.GetUserID(), STREAMTYPE_DESKTOP, packet.GetSessionID()))
       return;
    
#ifdef _DEBUG
    bool ok_tm = false;
    user.GetLastTimeStamp(&ok_tm);
    TTASSERT(ok_tm);
#endif

    //ignore cursor if it's not the current desktop session
    uint8_t session_id;
    uint16_t dest_userid;
    int16_t x, y;
    if(!packet.GetSessionCursor(&dest_userid, &session_id, &x, &y))
        return;

    desktop_cache_t session = user.GetDesktopSession();
    if (!session || session->GetSessionID() != session_id)
        return;

    //throw away packet if a newer one has already arrived
    bool is_set = false;
    if(!W32_GEQ(packet.GetTime(),
                user.GetLastTimeStamp(packet, &is_set)) && is_set)
        return;

    ServerChannel::users_t users = GetPacketDestinations(user, chan, packet, SUBSCRIBE_DESKTOP,
                                                         SUBSCRIBE_INTERCEPT_DESKTOP);

#if defined(ENABLE_ENCRYPTION)
    if(m_crypt_acceptors.size())
    {
        // copy crypt sections using copy constructor
        CryptDesktopCursorPacket crypt_pkt(DesktopCursorPacket(packet), chan.GetEncryptKey());
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
    serverchannel_t tmp_chan = GetPacketChannel(user, crypt_pkt, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;

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
    serverchannel_t tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;

    serveruser_t destuser;

    uint16_t dest_userid = packet.GetDestUserID();
    destuser = GetUser(dest_userid, &user);
    if(!destuser)
        return;
    
    //throw away desktop input if it's not the current session
    desktop_cache_t session = destuser->GetDesktopSession();
    if (!session || session->GetSessionID() != packet.GetSessionID())
        return;

    ServerChannel::users_t users = GetPacketDestinations(user, chan, packet,
                                                         SUBSCRIBE_DESKTOPINPUT,
                                                         SUBSCRIBE_NONE);

#if defined(ENABLE_ENCRYPTION)
    if(m_crypt_acceptors.size())
    {
        // create new input packet with encrypted sections
        CryptDesktopInputPacket crypt_pkt(DesktopInputPacket(packet), chan.GetEncryptKey());
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
    serverchannel_t tmp_chan = GetPacketChannel(user, crypt_pkt, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;

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
    serverchannel_t tmp_chan = GetPacketChannel(user, packet, remoteaddr, localaddr);
    if(!tmp_chan)
        return;

    ServerChannel& chan = *tmp_chan;

    uint16_t dest_userid = packet.GetDestUserID();
    serveruser_t dest_user = GetUser(dest_userid, &user);
    if(dest_user)
    {
#if defined(ENABLE_ENCRYPTION)
        if(m_crypt_acceptors.size())
        {
            CryptDesktopInputAckPacket crypt_pkt(DesktopInputAckPacket(packet), chan.GetEncryptKey());
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
    ASSERT_REACTOR_LOCKED(this);

    std::vector<serveruser_t> theDead;
    for(mapusers_t::iterator i=m_mUsers.begin(); i != m_mUsers.end(); i++)
    {
        if((*i).second->GetLastKeepAlive() >= m_properties.usertimeout)
            theDead.push_back((*i).second);
        else if((*i).second->GetFileTransferID() == 0)
            (*i).second->SetLastKeepAlive( (*i).second->GetLastKeepAlive() + SERVER_KEEPALIVE_DELAY );
    }

    //disconnect the dead
    for(size_t j=0;j<theDead.size();j++)
    {
        //notify of dropped users (due to keepalive)
        m_srvguard->OnUserDropped(*theDead[j]);
#if defined(ENABLE_ENCRYPTION)
        // SSL handler could be hanging in CryptStreamHandler::process_ssl()
        // therefore we have to forcefully delete the handler
        ACE_Event_Handler* h = RegisterStreamCallback(theDead[j]->ResetStreamHandle());
        delete h;
#else
        RegisterStreamCallback(theDead[j]->ResetStreamHandle());
#endif
    }
}

ErrorMsg ServerNode::UserLogin(int userid, const ACE_TString& username,
                               const ACE_TString& passwd)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
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
        m_srvguard->OnUserLoginBanned(*user);
        return err; //banned from server
    case TT_SRVERR_COMMAND_SUSPEND :
        return err;
    default :
        m_srvguard->OnUserAuthFailed(*user, username);

        IncLoginAttempt(*user);
        return err;
    }

    // usertype must be set
    if(useraccount.usertype == USERTYPE_NONE)
    {
        return ErrorMsg(TT_CMDERR_INVALID_ACCOUNT);
    }

    int user_count = GetAuthUserCount();
    if(user_count+1 > m_properties.maxusers && 
       (useraccount.usertype & USERTYPE_ADMIN) == 0)
        return ErrorMsg(TT_CMDERR_MAX_SERVER_USERS_EXCEEDED); //user limit

    //check for double login
    if((useraccount.userrights & USERRIGHT_MULTI_LOGIN) == 0)
    {
        ServerChannel::users_t users = GetAuthorizedUsers();
        for(size_t i=0;i<users.size();i++)
        {
            TTASSERT(users[i] != user);
            if(users[i]->GetUsername() == username)
                UserKick(user->GetUserID(), users[i]->GetUserID(), 0, true);
        }
    }

    //check for max logins per ip-address (ignore admin users)
    if(m_properties.max_logins_per_ipaddr &&
       (useraccount.usertype & USERTYPE_ADMIN) == 0)
    {
        int logins = 1; //include self
        ServerChannel::users_t users = GetAuthorizedUsers();
        for(size_t i=0;i<users.size();i++)
        {
            if(users[i]->GetIpAddress() == user->GetIpAddress())
                logins++;
        }
        if(logins > m_properties.max_logins_per_ipaddr)
            return ErrorMsg(TT_CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED);
    }

    //set user-account now meaning the user is not authorized
    user->SetUserAccount(useraccount);

    //store in admin cache
    if(user->GetUserType() & USERTYPE_ADMIN)
        m_admins.push_back(user);

    //clear any wrong logins
    m_failedlogins.erase(user->GetIpAddress());

    //do connect accepted
    user->DoAccepted(useraccount);
    user->DoServerUpdate(m_properties);

    //forward all channels
    user->ForwardChannels(GetRootChannel(), IsEncrypted());
    //send all files to user if admin
    if(user->GetUserType() & USERTYPE_ADMIN)
        user->ForwardFiles(GetRootChannel(), true);

    //notify other users of new user
    ServerChannel::users_t users = GetNotificationUsers();
    for(size_t i=0;i<users.size();i++)
    {
        if(users[i]->GetUserID() != userid)
            users[i]->DoLoggedIn(*user);
    }

    //forward users if USERRIGHT_VIEW_ALL_USERS enabled
    if(user->GetUserRights() & USERRIGHT_VIEW_ALL_USERS)
    {
        users = GetAuthorizedUsers();
        for(size_t i=0;i<users.size();i++)
            user->DoLoggedIn(*users[i]);

        user->ForwardUsers(GetRootChannel(), true);
    }
    //register peak and users servered
    m_stats.userspeak = max(m_stats.userspeak, user_count+1);
    m_stats.usersservered++;

    //notify listener if any
    m_srvguard->OnUserLogin(*user);

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserLogout(int userid)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    if(!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    TTASSERT(user->IsAuthorized());
    if(!user->IsAuthorized())
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t chan = user->GetChannel();
    if(chan)
    {
        ErrorMsg err = UserLeaveChannel(userid, chan->GetChannelID());
        if(err.errorno != TT_CMDERR_SUCCESS)
            return err;
    }

    std::set<int> chanids = m_rootchannel->RemoveOperator(userid, true);
    std::set<int>::iterator i;
    for(i=chanids.begin();i!=chanids.end();i++)
    {
        serverchannel_t chan = GetChannel(*i);
        TTASSERT(chan);
        if(chan)
            UpdateChannel(*chan);
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
    ServerChannel::users_t users = GetNotificationUsers();
    for(size_t i=0;i<users.size();i++)
        users[i]->DoLoggedOut(*user);

    m_srvguard->OnUserLoggedOut(*user);

    //reset important user info.
    user->SetUserAccount(UserAccount());

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserChangeNickname(int userid, const ACE_TString& newnick)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    if(!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    ErrorMsg err = m_srvguard->ChangeNickname(*user, newnick);
    if(err.success())
    {
        user->SetNickname(newnick);
        return UserUpdate(userid);
    }
    return err;
}

ErrorMsg ServerNode::UserChangeStatus(int userid, int mode, const ACE_TString& status)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    if(!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    ErrorMsg err = m_srvguard->ChangeStatus(*user, mode, status);
    if(err.success())
    {
        user->SetStatusMode(mode);
        user->SetStatusMessage(status);

        return UserUpdate(userid);
    }
    return err;
}

ErrorMsg ServerNode::UserUpdate(int userid)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    if(!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    const serverchannel_t& chan = user->GetChannel();
    if(chan)
    {
        const ServerChannel::users_t& users = chan->GetUsers();
        for(size_t i=0;i<users.size();i++)
            users[i]->DoUpdateUser(*user);

        //notify admins and show-all-users
        ServerChannel::users_t notifyusers = GetNotificationUsers(*chan);
        for(size_t i=0;i<notifyusers.size();i++)
            notifyusers[i]->DoUpdateUser(*user);
    }
    else
    {
        //notify admins and show-all-users
        ServerChannel::users_t notifyusers = GetNotificationUsers();
        for(size_t i=0;i<notifyusers.size();i++)
            notifyusers[i]->DoUpdateUser(*user);
    }

    m_srvguard->OnUserUpdated(*user);

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserJoinChannel(int userid, const ChannelProp& chanprop)
{
    GUARD_OBJ(this, lock());

    TTASSERT(GetRootChannel());

    serveruser_t user = GetUser(userid, nullptr);
    TTASSERT(user);

    bool makeop = false;

    if(!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t newchan = GetChannel(chanprop.channelid);
    if(!newchan)//user is trying to create a new channel
    {
        if((user->GetUserRights() & USERRIGHT_CREATE_TEMPORARY_CHANNEL) == 0 &&
           (user->GetUserRights() & USERRIGHT_MODIFY_CHANNELS) == 0)
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        if((user->GetUserRights() & USERRIGHT_MODIFY_CHANNELS) == 0 &&
            (chanprop.chantype & CHANNEL_PERMANENT) )
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        //ensure users cannot create channels which is not direct subchannel of current
        serverchannel_t parent = GetChannel(chanprop.parentid);
        if(parent != m_rootchannel && user->GetChannel() != parent)
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
        if(!err.success())
            return err;
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

    serverchannel_t oldchan = user->GetChannel();

    if(oldchan && newchan->Compare(oldchan))
        return ErrorMsg(TT_CMDERR_ALREADY_IN_CHANNEL);

    if(newchan->GetUsersCount()+1 > newchan->GetMaxUsers())
        return ErrorMsg(TT_CMDERR_MAX_CHANNEL_USERS_EXCEEDED);

    //leave the current channel if applicable
    if(oldchan)
    {
        //HACK: protect new channel so it doesn't get deleted if it's dynamic 
        //and parent of 'oldchan'
        int chantype = newchan->GetChannelType();
        newchan->SetChannelType(chantype | CHANNEL_PERMANENT);
        ErrorMsg err = UserLeaveChannel(user->GetUserID(), oldchan->GetChannelID());
        TTASSERT(err.errorno == TT_CMDERR_SUCCESS);
        newchan->SetChannelType(chantype);
    }
    //add user to channel
    newchan->AddUser(user->GetUserID(), user);

    //notify user of new channel
    user->DoJoinedChannel(*newchan, IsEncrypted());

    //set new channel
    user->SetChannel(newchan);

    //check if user should automatically become operator of channel
    UserAccount useraccount = user->GetUserAccount();
    if(useraccount.auto_op_channels.find(newchan->GetChannelID()) != 
        useraccount.auto_op_channels.end())
        makeop = true;

    //vector with users who'll be notified of the new user
    ServerChannel::users_t notifyusers = GetNotificationUsers(*newchan);
    for(size_t i=0;i<notifyusers.size();i++)
        notifyusers[i]->DoAddUser(*user, *newchan);

    //notify users in new channel that new user has joined
    const ServerChannel::users_t& users = newchan->GetUsers();
    if(user->GetUserRights() & USERRIGHT_VIEW_ALL_USERS)
    {
        for(size_t i=0;i<users.size();i++)
            users[i]->DoAddUser(*user, *newchan);
    }
    else
    {
        for(size_t i=0;i<users.size();i++)
        {
            users[i]->DoAddUser(*user, *newchan);
            if(user->GetUserID() != users[i]->GetUserID())
                user->DoAddUser(*users[i], *newchan);
        }
    }

    if(makeop)
    {
        newchan->AddOperator(user->GetUserID());
        UpdateChannel(*newchan); //notify users of new operator
    }

    //send channel's file list
    if(user->GetUserType() & USERTYPE_DEFAULT)
        user->ForwardFiles(newchan, false);

    //start active desktop transmissions
    for(size_t i=0;i<users.size();i++)
    {
        if (users[i]->GetDesktopSession() && 
           (user->GetSubscriptions(*users[i]) & SUBSCRIBE_DESKTOP))
        {
            //Start delayed timers for desktop transmission, so the new 
            //user will have received the channel's channel-key.
            TimerHandler* th;
            timer_userdata tm_data;
            tm_data.src_userid = users[i]->GetUserID();
            tm_data.dest_userid = user->GetUserID();
            ACE_NEW_NORETURN(th, TimerHandler(*this, TIMER_START_DESKTOPTX_ID,
                                              tm_data.userdata));
            long timerid = m_timer_reactor->schedule_timer(th, 0, ACE_Time_Value(1));
            TTASSERT(timerid>=0);
        }
        //TODO: user could actually reuse the desktop session (but restarts at the moment)
//         if (user->GetDesktopSession() && 
//            users[i]->GetUserID() != user->GetUserID())
//             StartDesktopTransmitter(*user, *users[i], *newchan);
    }

    //notify listener
    TTASSERT(newchan == user->GetChannel());
    m_srvguard->OnUserJoinChannel(*user, *newchan);

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserLeaveChannel(int userid, int channelid)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
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
    ServerChannel::users_t notifyusers = GetNotificationUsers(*chan);
    for(size_t i=0;i<notifyusers.size();i++)
        notifyusers[i]->DoRemoveUser(*user, *chan);

    //notify channel users
    const ServerChannel::users_t& users = chan->GetUsers();

    //close active desktop transmissions
    for(size_t i=0;i<users.size();i++)
    {
        StopDesktopTransmitter(*users[i], *user, false);
        StopDesktopTransmitter(*user, *users[i], false);
    }

    if(user->GetUserRights() & USERRIGHT_VIEW_ALL_USERS)
    {
        for(size_t i=0;i<users.size();i++)
            users[i]->DoRemoveUser(*user, *chan);
    }
    else
    {
        for(size_t i=0;i<users.size();i++)
        {
            users[i]->DoRemoveUser(*user, *chan);
            if(user->GetUserID() != users[i]->GetUserID())
                user->DoRemoveUser(*users[i], *chan);
        }
    }

    //check if in allowed to transmit
    bool upd_chan = chan->ClearTransmitUser(userid).size();
    chan->RemoveUser(user->GetUserID());

    if(upd_chan)
        UpdateChannel(*chan);

    serverchannel_t nullc;
    user->SetChannel(nullc);

    m_srvguard->OnUserLeaveChannel(*user, *chan);

    CleanChannels(chan);

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

void ServerNode::UserDisconnected(int userid)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    TTASSERT(user);
    if(user)
    {
        //logout user
        if(user->IsAuthorized())
            UserLogout(userid);

        //if users have modified any subscriptions to this user, clear it
        const ServerChannel::users_t& users = GetAuthorizedUsers();
        for(size_t i=0;i<users.size();i++)
            users[i]->ClearUserSubscription(*user);

        //notify listener (if any)
        m_srvguard->OnUserDisconnected(*user);

        //if it's a file transfer. Clean it up.
        if(user->GetFileTransferID())
            m_filetransfers.erase(user->GetFileTransferID());

        m_updUserIPs.erase(userid);
        m_mUsers.erase(userid);
        TTASSERT(!m_rootchannel || m_rootchannel->GetUser(userid) == NULL);
    }
}

ErrorMsg ServerNode::UserOpDeOp(int userid, int channelid, 
                                const ACE_TString& oppasswd, int op_userid, bool op)
{
    GUARD_OBJ(this, lock());

    serveruser_t opper = GetUser(userid, nullptr);
    serveruser_t op_user = GetUser(op_userid, opper.get());
    if(!op_user || !opper)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t chan = GetChannel(channelid);
    if(!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    if((opper->GetUserRights() & USERRIGHT_OPERATOR_ENABLE) ||
        (oppasswd.length() && oppasswd == chan->GetOpPassword()))
    {
        if(op)
            chan->AddOperator(op_userid);
        else
            chan->RemoveOperator(op_userid);
        UpdateChannel(*chan);

        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    else if(oppasswd.length())
        return ErrorMsg(TT_CMDERR_INCORRECT_OP_PASSWORD);
    return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
}

ErrorMsg ServerNode::UserKick(int userid, int kick_userid, int chanid,
                              bool force_kick)
{
    GUARD_OBJ(this, lock());

    serveruser_t kicker = GetUser(userid, nullptr);
    serveruser_t kickee = GetUser(kick_userid, kicker.get());
    if (!kickee || (!kicker && !force_kick))
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    if(chanid) //kick from channel
    {
        serverchannel_t chan = GetChannel(chanid);
        if(!chan)
            return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

        if(!chan->UserExists(kick_userid))
            return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

        if(force_kick || (kicker->GetUserRights() & USERRIGHT_KICK_USERS) ||
           chan->IsOperator(userid))
        {
            kickee->DoKicked(userid, true);

            m_srvguard->OnUserKicked(*kickee, kicker.get(), chan.get());

            return UserLeaveChannel(kick_userid, chanid);
        }
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
    }

    if(force_kick || (kicker->GetUserRights() & USERRIGHT_KICK_USERS))//kick from server
    {
        kickee->DoKicked(userid, false);

        m_srvguard->OnUserKicked(*kickee, kicker.get(), 
                                 kickee->GetChannel().get());

        if(kickee->IsAuthorized())
            return UserLogout(kick_userid);
        else
            return ErrorMsg(TT_CMDERR_USER_NOT_FOUND); //already logged out
    }
    return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
}

ErrorMsg ServerNode::UserBan(int userid, int ban_userid, BannedUser ban)
{
    GUARD_OBJ(this, lock());

    serveruser_t banner = GetUser(userid, nullptr);
    if (!banner)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t banchan;
    ErrorMsg err(TT_CMDERR_SUCCESS);

    if(ban_userid > 0)
    {
        serveruser_t ban_user = GetUser(ban_userid, banner.get());
        if (!ban_user)
            return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

        if (ban.bantype & BANTYPE_CHANNEL)
        {
            if(ban.chanpath.length())
            {
                banchan = ChangeChannel(GetRootChannel(), ban.chanpath);
                if (!banchan)
                    return TT_CMDERR_CHANNEL_NOT_FOUND;
            }
            else
            {
                banchan = ban_user->GetChannel();
                ban.chanpath = banchan->GetChannelPath();
            }
            ban = ban_user->GetBan(ban.bantype, ban.chanpath);
        }

        if((banner->GetUserRights() & USERRIGHT_BAN_USERS) == 0)
        {
            if (!banchan || !banchan->IsOperator(userid))
                return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
        }

        err = m_srvguard->AddUserBan(*banner, *ban_user, ban.bantype);
        if (banchan && err.success())
            AddBannedUserToChannel(ban);
        
        if (err.success())
            m_srvguard->OnUserBanned(*ban_user, *banner);
    }
    else
    {
        if (ban.bantype & BANTYPE_CHANNEL)
        {
            if(ban.chanpath.is_empty())
                return TT_CMDERR_CHANNEL_NOT_FOUND;

            banchan = ChangeChannel(GetRootChannel(), ban.chanpath);
            if (!banchan)
                return TT_CMDERR_CHANNEL_NOT_FOUND;
            ban.chanpath = banchan->GetChannelPath();
        }

        if((banner->GetUserRights() & USERRIGHT_BAN_USERS) == 0)
        {
            if (banchan && banchan->IsOperator(userid))
                err = m_srvguard->AddUserBan(*banner, ban);
            else
                return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
        }
        else
        {
            err = m_srvguard->AddUserBan(*banner, ban);
        }

        if (banchan && err.success())
            AddBannedUserToChannel(ban);

        if (err.success())
            m_srvguard->OnUserBanned(*banner, ban);
    }

    if(err.success() && IsAutoSaving())
        m_srvguard->OnSaveConfiguration(*this);

    return err;
}

ErrorMsg ServerNode::UserUnBan(int userid, const BannedUser& ban)
{
    GUARD_OBJ(this, lock());
    ErrorMsg err;

    serveruser_t user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t banchan;
    if(ban.bantype & BANTYPE_CHANNEL)
    {
        banchan = ChangeChannel(GetRootChannel(), ban.chanpath);
        if (!banchan)
            return TT_CMDERR_CHANNEL_NOT_FOUND;
    }

    if((user->GetUserRights() & USERRIGHT_BAN_USERS) == 0)
    {
        if (banchan && banchan->IsOperator(userid))
            err = m_srvguard->RemoveUserBan(*user, ban);
        else
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
    }
    else
    {
        err = m_srvguard->RemoveUserBan(*user, ban);
    }

    if (err.success() && banchan)
        banchan->RemoveUserBan(ban);

    if(err.success())
    {
        m_srvguard->OnUserUnbanned(*user, ban);
        if(IsAutoSaving())
            m_srvguard->OnSaveConfiguration(*this);
    }
    return err;
}

ErrorMsg ServerNode::UserListServerBans(int userid, int chanid, int index, int count)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t banchan = GetChannel(chanid);
    if (chanid > 0 && !banchan)
        return TT_CMDERR_CHANNEL_NOT_FOUND;

    if((user->GetUserRights() & USERRIGHT_BAN_USERS) == 0)
    {
        if (!banchan || !banchan->IsOperator(userid))
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
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    useraccounts_t users;
    ErrorMsg ret = m_srvguard->GetRegUsers(*user, users);
    if(ret.errorno == TT_CMDERR_SUCCESS)
    {
        for(size_t i=index;i<users.size() && count--;i++)
            user->DoShowUserAccount(users[i]);
    }
    return ret;
}

ErrorMsg ServerNode::UserNewUserAccount(int userid, const UserAccount& regusr)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    // allow anonymous account
    // if(regusr.username.empty())
    //     return ErrorMsg(TT_CMDERR_INVALID_USERNAME);

    intset_t::const_iterator is = regusr.auto_op_channels.begin();
    for(;is != regusr.auto_op_channels.end();is++)
    {
        serverchannel_t chan = GetChannel(*is);
        if (!chan)
            return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);
    }
    
    ErrorMsg err = m_srvguard->AddRegUser(*user, regusr);
    if(err.errorno == TT_CMDERR_SUCCESS)
    {
        if(IsAutoSaving())
            m_srvguard->OnSaveConfiguration(*this, user.get());
    }
    return err;
}

ErrorMsg ServerNode::UserDeleteUserAccount(int userid, const ACE_TString& username)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    ErrorMsg err = m_srvguard->DeleteRegUser(*user, username);
    if(err.errorno == TT_CMDERR_SUCCESS)
    {    
        if(IsAutoSaving())
            m_srvguard->OnSaveConfiguration(*this, user.get());
    }
    return err;
}

ErrorMsg ServerNode::UserUpdateChannel(int userid, const ChannelProp& chanprop)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t chan = GetChannel(chanprop.channelid);
    if (!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    int c = chan->GetUsersCount();
    if(chan->GetUsersCount() && 
       memcmp(&chanprop.audiocodec, &chan->GetAudioCodec(), sizeof(chanprop.audiocodec)) != 0)
        return ErrorMsg(TT_CMDERR_CHANNEL_HAS_USERS);

    //user can update channel if either admin or operator of channel
    if((user->GetUserRights() & USERRIGHT_MODIFY_CHANNELS))
        return UpdateChannel(chanprop, user.get());
    else if(chan->IsOperator(userid))
    {
        //don't allow user to change static channel property
        if( (chanprop.chantype & CHANNEL_PERMANENT) !=
            (chan->GetChannelType() & CHANNEL_PERMANENT))
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
        //don't allow user to change name of static channel
        if( (chan->GetChannelType() & CHANNEL_PERMANENT) &&
            chanprop.name != chan->GetName())
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        return UpdateChannel(chanprop, user.get());
    }
    else
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
}

ErrorMsg ServerNode::UserUpdateServer(int userid, const ServerSettings& properties)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    if((user->GetUserRights() & USERRIGHT_UPDATE_SERVERPROPERTIES) == 0)
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

    ErrorMsg err = UpdateServer(properties);
    if(err.errorno == TT_CMDERR_SUCCESS)
        m_srvguard->OnServerUpdated(*user, properties);
    
    return err;
}

ErrorMsg ServerNode::UserSaveServerConfig(int userid)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    m_srvguard->OnSaveConfiguration(*this, user.get());

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UpdateServer(const ServerSettings& properties)
{
    SetServerProperties(properties);

    ServerChannel::users_t users = GetAuthorizedUsers();
    for(size_t i=0;i<users.size();i++)
        users[i]->DoServerUpdate(m_properties);

    if(IsAutoSaving())
        m_srvguard->OnSaveConfiguration(*this);

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::MakeChannel(const ChannelProp& chanprop, 
                                 const ServerUser* user/* = NULL*/)
{
    GUARD_OBJ(this, lock());

    //initial server configuration creates the root channel, so initially it's null
    if (m_rootchannel && m_rootchannel->GetSubChannelCount(true) + 1 > MAX_CHANNELS)
        return ErrorMsg(TT_CMDERR_MAX_CHANNELS_EXCEEDED);

    //check bandwidth restriction
    if(user && user->GetUserAccount().audiobpslimit &&
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
        if(chanprop.name.empty())
            return ErrorMsg(TT_CMDERR_CHANNEL_ALREADY_EXISTS);

        if(parent->GetChannelPath().length() + chanprop.name.length() + 
            ACE_TString(CHANNEL_SEPARATOR).length() >= MAX_STRING_LENGTH)
            return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);
    }

    if(chanprop.passwd.length() >= MAX_STRING_LENGTH)
        return ErrorMsg(TT_CMDERR_INCORRECT_CHANNEL_PASSWORD);

    if(GetChannel(chanprop.channelid).get())
        return ErrorMsg(TT_CMDERR_CHANNEL_ALREADY_EXISTS);

    //generate next channel id.
    int chanid = chanprop.channelid;
    while(!chanid || (chanid < MAX_CHANNELS && GetChannel(chanid).get()))
    {
        if(++chanid > MAX_CHANNELS)
            chanid = 1;
    }

    serverchannel_t chan;
    if (!parent)
    {
        TTASSERT(!GetRootChannel());
        ServerChannel* newchan = new ServerChannel(chanid);
        chan = serverchannel_t(newchan);
        m_rootchannel = chan;
    }
    else
    {
        ServerChannel* newchan = new ServerChannel(parent, chanid, chanprop.name);
        chan = serverchannel_t(newchan);
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

    //forward new channel to all connected users
    const ServerChannel::users_t& users = GetAuthorizedUsers();
    for(size_t i=0;i<users.size();i++)
    {
        if(users[i]->IsAuthorized())
            users[i]->DoAddChannel(*chan, IsEncrypted());
    }

    //notify listener if any
    m_srvguard->OnChannelCreated(*chan, user);

    if(IsAutoSaving() && (chanprop.chantype & CHANNEL_PERMANENT))
        m_srvguard->OnSaveConfiguration(*this, user);

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UpdateChannel(const ChannelProp& chanprop, 
                                   const ServerUser* user/* = NULL*/)
{
    GUARD_OBJ(this, lock());

    TTASSERT(GetRootChannel());
    serverchannel_t chan = GetChannel(chanprop.channelid);
    if (!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    //ensure channel with same name doesn't already exist
    serverchannel_t parent = GetChannel(chanprop.parentid);
    if (parent)
    {
        serverchannel_t subchan = parent->GetSubChannel(chanprop.name);
        if(subchan.get() && subchan->GetChannelID() != chanprop.channelid)
            return ErrorMsg(TT_CMDERR_CHANNEL_ALREADY_EXISTS);
    }

    if(m_rootchannel != chan)
    {
        if(chanprop.name.empty())
            return ErrorMsg(TT_CMDERR_CHANNEL_ALREADY_EXISTS);

        chan->SetName(chanprop.name);
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
        const ServerChannel::users_t& users = chan->GetUsers();
        set<int>::const_iterator ii = chan->GetDesktopUsers().begin();
        for(;ii!=chan->GetDesktopUsers().end();ii++)
        {
            if(chanprop.GetTransmitUsers(STREAMTYPE_DESKTOP).count(*ii))
            {
                serveruser_t src_user = GetUser(*ii, nullptr);
                //TTASSERT(src_user); userid can be TRANSMITUSERS_FREEFORALL (0xFFF)
                if (!src_user || !src_user->GetDesktopSession())
                    continue;
                //TODO: this doesn't handle users who're intercepting packets
                for(size_t i=0;i<users.size();i++)
                    StopDesktopTransmitter(*src_user, *users[i], true);
            }
        }
    }
    chan->SetDesktopUsers(chanprop.GetTransmitUsers(STREAMTYPE_DESKTOP));
    chan->SetMediaFileUsers(chanprop.GetTransmitUsers(STREAMTYPE_MEDIAFILE));
    chan->SetTransmitQueue(chanprop.transmitqueue);

    UpdateChannel(*chan);

    //notify listener if any
    m_srvguard->OnChannelUpdated(*chan, user);

    if(IsAutoSaving() && (chan->GetChannelType() & CHANNEL_PERMANENT))
        m_srvguard->OnSaveConfiguration(*this, user);

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

void ServerNode::CleanChannels(serverchannel_t& channel)
{
    ASSERT_REACTOR_LOCKED(this);

    //remove channel if empty
    while (channel && channel != GetRootChannel())
    {
        if( channel->GetUsersCount()==0 && 
            channel->GetSubChannelCount()==0 &&
            (channel->GetChannelType() & CHANNEL_PERMANENT) == 0)
        {
            serverchannel_t parent = channel->GetParentChannel();
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

     while(sweeper.size())
     {
         serverchannel_t chan = sweeper.top();
         sweeper.pop();

         size_t txq = chan->GetTransmitQueue().size();
         chan->CanTransmit(0, STREAMTYPE_VOICE, 0);
         chan->ClearFromTransmitQueue(0);
         if(txq != chan->GetTransmitQueue().size())
             UpdateChannel(*chan, chan->GetUsers());

         ServerChannel::channels_t subs = chan->GetSubChannels();
         for(size_t i=0;i<subs.size();i++)
             sweeper.push(subs[i]);
     }

}

ErrorMsg ServerNode::RemoveChannel(int channelid, const ServerUser* user/* = NULL*/)
{
    GUARD_OBJ(this, lock());

    TTASSERT(GetRootChannel());
    bool bStatic = false;

    serverchannel_t chan = GetChannel(channelid);
    if (!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    bStatic = (chan->GetChannelType() & CHANNEL_PERMANENT);
    //recursive remove
    std::stack<serverchannel_t> stackChannels;
    stackChannels.push(chan);

    while(!stackChannels.empty())
    {
        chan = stackChannels.top();
        stackChannels.pop();
        vector<serverchannel_t> vecSubChannels = chan->GetSubChannels();
        if(!vecSubChannels.empty())
        {
            //to support recursive delete
            stackChannels.push(chan);

            for(size_t i=0;i<vecSubChannels.size();i++)
                stackChannels.push(vecSubChannels[i]);
        }
        else
        {
            const ServerChannel::users_t& users = chan->GetUsers();
            for(size_t i=0;i<users.size();i++)
                UserKick(0, users[i]->GetUserID(), chan->GetChannelID(), true);

            //check if channel still exists (kick may have removed it
            if (!GetChannel(chan->GetChannelID()))
                continue;

            //remove files from channel
            files_t files;
            chan->GetFiles(files, false);
            for(size_t i=0;i<files.size();i++)
                RemoveFileFromChannel(files[i].filename, chan->GetChannelID());

            //remove as subchannel (unless it's the root)
            serverchannel_t parent = chan->GetParentChannel();
            if (parent)
            {
                //notify users
                for(mapusers_t::iterator ite=m_mUsers.begin(); 
                    ite != m_mUsers.end(); ite++)
                {
                    if((*ite).second->IsAuthorized())
                        (*ite).second->DoRemoveChannel(*chan);
                }

                parent->RemoveSubChannel(chan->GetName());
                //notify listener if any
                m_srvguard->OnChannelRemoved(*chan, user);
            }                                  
        }
    }

    if(IsAutoSaving() && bStatic)
        m_srvguard->OnSaveConfiguration(*this, user);

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

void ServerNode::UpdateChannel(const ServerChannel& chan)
{
    ASSERT_REACTOR_LOCKED(this);

    //don't show channel updates when show-all-users is disabled.
    for( mapusers_t::iterator ite = m_mUsers.begin(); 
        ite != m_mUsers.end();
        ite++ )
    {
        if( (*ite).second->IsAuthorized() && 
            ((ite->second->GetUserRights() & USERRIGHT_VIEW_ALL_USERS) || 
             &chan == ite->second->GetChannel().get()) )
            (*ite).second->DoUpdateChannel(chan, IsEncrypted());
    }
}

void ServerNode::UpdateChannel(const ServerChannel& chan, const ServerChannel::users_t& users)
{
    ASSERT_REACTOR_LOCKED(this);

    for(auto u=users.begin();u!=users.end();++u)
        (*u)->DoUpdateChannel(chan, IsEncrypted());
}

ErrorMsg ServerNode::UserMove(int userid, int moveuserid, int channelid)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    if((user->GetUserRights() & USERRIGHT_MOVE_USERS) == 0)
        return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

    serveruser_t moveuser = GetUser(moveuserid, user.get());
    if (!moveuser)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t chan = GetChannel(channelid);
    if (!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    ErrorMsg err = UserJoinChannel(moveuserid, chan->GetChannelProp());

    if(err.errorno == TT_CMDERR_SUCCESS)
        m_srvguard->OnUserMoved(*user, *moveuser);
    return err;
}

ErrorMsg ServerNode::UserTextMessage(const TextMessage& msg)
{
    GUARD_OBJ(this, lock());

    serveruser_t from = GetUser(msg.from_userid, nullptr);
    if (!from)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    switch(msg.msgType)
    {
    case TTUserMsg :
    {
        serveruser_t to_user = GetUser(msg.to_userid, from.get());
        if (!to_user)
            return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

        //just ignore text message if the user doesn't subscribe
        if( (to_user->GetSubscriptions(*from) & SUBSCRIBE_USER_MSG) == 0)
            return ErrorMsg(TT_CMDERR_SUCCESS);

        to_user->DoTextMessage(*from, msg);

        //notify administrators for user2user message
        const ServerChannel::users_t& admins = GetAdministrators();
        for(size_t i=0;i<admins.size();i++)
        {
            if((admins[i]->GetSubscriptions(*from) & SUBSCRIBE_INTERCEPT_USER_MSG) &&
                admins[i]->GetUserID() != msg.to_userid && 
                admins[i]->GetUserID() != msg.from_userid )
                admins[i]->DoTextMessage(*from, msg);
        }

        //log text message
        m_srvguard->OnUserMessage(*from, *to_user, msg);

        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    case TTCustomMsg :
    {
        serveruser_t to_user = GetUser(msg.to_userid, from.get());
        if (!to_user)
            return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

        //just ignore text message if the user doesn't subscribe
        if( (to_user->GetSubscriptions(*from) & SUBSCRIBE_CUSTOM_MSG) == 0)
            return ErrorMsg(TT_CMDERR_SUCCESS);

        to_user->DoTextMessage(*from, msg);

        //notify administrators for user2user message
        const ServerChannel::users_t& admins = GetAdministrators();
        for(size_t i=0;i<admins.size();i++)
        {
            if((admins[i]->GetSubscriptions(*from) & SUBSCRIBE_INTERCEPT_CUSTOM_MSG) &&
                admins[i]->GetUserID() != msg.to_userid && 
                admins[i]->GetUserID() != msg.from_userid )
                admins[i]->DoTextMessage(*from, msg);
        }

        //log text message
        m_srvguard->OnCustomMessage(*from, *to_user, msg);

        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    case TTChannelMsg :
    {
        serverchannel_t chan = GetChannel(msg.channelid);
        if (!chan)
            return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

        if(from->GetChannel() != chan &&
           (from->GetUserType() & USERTYPE_ADMIN) == 0)
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        //forward message to all users of that channel
        const ServerChannel::users_t& chanusers = chan->GetUsers();
        intset_t already_recv;
        for(size_t i=0;i<chanusers.size();i++)
        {
            already_recv.insert(chanusers[i]->GetUserID());
            if(chanusers[i]->GetSubscriptions(*from) & SUBSCRIBE_CHANNEL_MSG)
                chanusers[i]->DoTextMessage(*from, msg);
        }
        //notify administrators of user2channel message                
        const ServerChannel::users_t& admins = GetAdministrators(*chan);
        for(size_t i=0;i<admins.size();i++)
        {
            if(already_recv.find(admins[i]->GetUserID()) != already_recv.end())
                continue;
            if(admins[i]->GetSubscriptions(*from) & SUBSCRIBE_INTERCEPT_CHANNEL_MSG)
                admins[i]->DoTextMessage(*from, msg);
        }

        //log text message
        m_srvguard->OnChannelMessage(*from, *chan, msg);

        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    case TTBroadcastMsg :
    {
        if((from->GetUserRights() & USERRIGHT_TEXTMESSAGE_BROADCAST) == 0)
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);

        const ServerChannel::users_t& users = GetAuthorizedUsers();
        for(size_t i=0;i<users.size();i++)
        {
            if( users[i]->GetSubscriptions(*from) & SUBSCRIBE_BROADCAST_MSG )
                users[i]->DoTextMessage(*from, msg);
        }

        //log text message
        m_srvguard->OnBroadcastMessage(*from, msg);

        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    default : 
        //unknown message type
        return ErrorMsg(TT_CMDERR_INCOMPATIBLE_PROTOCOLS);
    }
}

ErrorMsg ServerNode::UserRegFileTransfer(FileTransfer& transfer)
{
    GUARD_OBJ(this, lock());

    if(m_properties.filesroot.length() == 0)
        return ErrorMsg(TT_CMDERR_FILESHARING_DISABLED);

    serveruser_t user = GetUser(transfer.userid, nullptr);
    if (!user)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);

    serverchannel_t chan = GetChannel(transfer.channelid);
    if (!chan)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

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
        int id = std::max(1, m_filetx_id_counter++);
        if(m_filetransfers.find(id) != m_filetransfers.end()) //no IDs left
            return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);

        ACE_TString tmpfilename = ACE_TEXT("tmp_") + i2string(id) + ACE_TEXT(".dat");
        ACE_TString filepath = m_properties.filesroot + ACE_DIRECTORY_SEPARATOR_STR + tmpfilename;
        if(chan->FileExists(transfer.filename))
            return ErrorMsg(TT_CMDERR_FILE_ALREADY_EXISTS);
        else
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

        ACE_TString internalpath = m_properties.filesroot + ACE_DIRECTORY_SEPARATOR_STR + remotefile.internalname;
        if(ACE_OS::filesize(internalpath.c_str())<0)
            return ErrorMsg(TT_CMDERR_FILE_NOT_FOUND);

        transfer.localfile = internalpath;
        transfer.filesize = remotefile.filesize;

        transfer.transferid = std::max(1, m_filetx_id_counter++);
        if(m_filetransfers.find(transfer.transferid) != m_filetransfers.end()) //no IDs left
            return ErrorMsg(TT_CMDERR_OPENFILE_FAILED);
    }

    m_filetransfers[transfer.transferid] = transfer;

    TTASSERT(transfer.transferid>0);
    user->DoFileAccepted(transfer);
    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserSubscribe(int userid, int subuserid, 
                                   Subscriptions subscrip)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    serveruser_t subscriptuser = GetUser(subuserid, user.get());
    if (!user || !subscriptuser)
        return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);
    
    //only admins can intercept
    if((user->GetUserType() & USERTYPE_ADMIN) == 0)
    {
        if(subscrip & SUBSCRIBE_INTERCEPT_ALL)
            return ErrorMsg(TT_CMDERR_NOT_AUTHORIZED);
    }

    user->AddSubscriptions(*subscriptuser, subscrip);

    //update user's subscription mask, if viewing all users or
    //in same channel
    if ((subscriptuser->GetUserRights() & USERRIGHT_VIEW_ALL_USERS) ||
        user->GetChannel() == subscriptuser->GetChannel())
    {
        subscriptuser->DoUpdateUser(*user);
    }

    if(subscriptuser != user)
        user->DoUpdateUser(*subscriptuser);

    //if active desktop then start it
    if (subscriptuser->GetDesktopSession() &&
        subscrip & (SUBSCRIBE_DESKTOP | SUBSCRIBE_INTERCEPT_DESKTOP))
    {
        serverchannel_t chan = subscriptuser->GetChannel();
        if (chan)
            StartDesktopTransmitter(*subscriptuser, *user, *chan);
    }

    return ErrorMsg(TT_CMDERR_SUCCESS);
}

ErrorMsg ServerNode::UserUnsubscribe(int userid, int subuserid, 
                                     Subscriptions subscrip)
{
    GUARD_OBJ(this, lock());

    serveruser_t user = GetUser(userid, nullptr);
    serveruser_t subscriptuser = GetUser(subuserid, user.get());
    if (user && subscriptuser)
    {
        user->ClearSubscriptions(*subscriptuser, subscrip);
        //update user's subscription mask, if viewing all users or
        //in same channel
        if ((subscriptuser->GetUserRights() & USERRIGHT_VIEW_ALL_USERS) ||
            user->GetChannel() == subscriptuser->GetChannel())
        {
            subscriptuser->DoUpdateUser(*user);
        }
        
        if(subscriptuser != user)
            user->DoUpdateUser(*subscriptuser);

        //if active desktop then stop it
        if(subscrip & (SUBSCRIBE_DESKTOP | SUBSCRIBE_INTERCEPT_DESKTOP))
            StopDesktopTransmitter(*subscriptuser, *user, true);

        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);
}

ErrorMsg ServerNode::AddFileToChannel(const RemoteFile& remotefile)
{
    ASSERT_REACTOR_LOCKED(this);

    TTASSERT(!GetRootChannel()->FileExists(remotefile.fileid));
    serverchannel_t channel = GetChannel(remotefile.channelid);
    if (!channel)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    channel->AddFile(remotefile);

    ServerChannel::users_t users = channel->GetUsers(); //do copy
    const ServerChannel::users_t& admins = GetAdministrators(*channel);
    users.insert(users.end(), admins.begin(), admins.end());

    for(size_t i=0;i<users.size();i++)
        users[i]->DoAddFile(remotefile);

    return ErrorMsg();
}

ErrorMsg ServerNode::RemoveFileFromChannel(const ACE_TString& filename, int channelid)
{
    ASSERT_REACTOR_LOCKED(this);

    serverchannel_t channel = GetChannel(channelid);
    if (!channel)
        return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

    RemoteFile remotefile;
    if(channel->GetFile(filename, remotefile))
    {
        channel->RemoveFile(filename);

        ACE_FILE_Connector con;
        ACE_FILE_IO file;
        ACE_TString internalpath = m_properties.filesroot + ACE_DIRECTORY_SEPARATOR_STR + remotefile.internalname;
        if(con.connect(file, ACE_FILE_Addr(internalpath.c_str())) >= 0)
            file.remove();

        ServerChannel::users_t users = channel->GetUsers(); //do copy
        const ServerChannel::users_t& admins = GetAdministrators(*channel);
        users.insert(users.end(), admins.begin(), admins.end());

        for(size_t i=0;i<users.size();i++)
            users[i]->DoRemoveFile(filename, *channel);
    }
    return ErrorMsg();
}

ErrorMsg ServerNode::AddBannedUserToChannel(const BannedUser& ban)
{
    TTASSERT(ban.bantype & BANTYPE_CHANNEL);
    serverchannel_t chan = ChangeChannel(GetRootChannel(), ban.chanpath);
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
        serveruser_t to_user = GetUser(msg.to_userid, nullptr);
        if (!to_user)
            return ErrorMsg(TT_CMDERR_USER_NOT_FOUND);
        to_user->DoTextMessage(msg);
        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    case TTChannelMsg :
    {
        serverchannel_t chan = GetChannel(msg.channelid);
        if (!chan)
            return ErrorMsg(TT_CMDERR_CHANNEL_NOT_FOUND);

        //forward message to all users of that channel
        const ServerChannel::users_t& chanusers = chan->GetUsers();
        for(size_t i=0;i<chanusers.size();i++)
        {
            chanusers[i]->DoTextMessage(msg);
        }
        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    case TTBroadcastMsg :
    {
        const ServerChannel::users_t& users = GetAuthorizedUsers();
        for(size_t i=0;i<users.size();i++)
        {
            users[i]->DoTextMessage(msg);
        }
        return ErrorMsg(TT_CMDERR_SUCCESS);
    }
    case TTNoneMsg :
        break;
    }
    return ErrorMsg(TT_CMDERR_INCOMPATIBLE_PROTOCOLS);
}

