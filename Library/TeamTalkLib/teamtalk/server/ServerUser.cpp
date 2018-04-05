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

#include "ServerUser.h"
#include "ServerNode.h"

#include <myace/MyACE.h>

#include <teamtalk/Commands.h>
#include <queue>

#if defined(ENABLE_ENCRYPTION)
#include <openssl/rand.h>
#endif

using namespace std;
using namespace teamtalk;

#define GET_PROP_OR_RETURN(properties, name, value)                     \
    do {                                                                \
        if(!GetProperty(properties, name, value))                       \
            return ErrorMsg(TT_CMDERR_MISSING_PARAMETER, name);         \
    }while(0)


ServerUser::ServerUser(int userid, 
                       ServerNode& servernode,
                       ACE_HANDLE h)
                       : User(userid)
                       , m_servernode(servernode) //init parent class
                       , m_stream_handle(h)
                       , m_cmdsuspended(false)
{
    //MYTRACE("StreamHandler for userid %d is %d\n", GetUserID(), handler.get_handle());
    //TTASSERT(handler.get_handle() != ACE_INVALID_HANDLE);
    m_nLastKeepAlive = 0;
}

ServerUser::~ServerUser()
{
    CloseTransfer();
    MYTRACE( ACE_TEXT("~ServerUser #%d\r\n"), GetUserID());
}

bool ServerUser::ReceiveData(const char* data, int len)
{
    TTASSERT(len>0);

    if(m_filetransfer.get() && m_filetransfer->active && m_filetransfer->inbound)
    {
        bool bContinue = true;
        HandleBinaryFileWrite(data, len, bContinue);
        return bContinue;
    }

    //make sure client doesn't use too much memory
    if(m_recvbuf.length() > MAX_COMMAND_LENGTH)
        return false;

    m_recvbuf.append(data, len);

    return ProcessCommandQueue(false);
}

bool ServerUser::SendData(ACE_Message_Queue_Base& msg_queue)
{
    if(m_stream_handle == ACE_INVALID_HANDLE)
    {
        return false;
    }

    //fill with more commands?
    if(m_filetransfer.get() && m_filetransfer->active && m_filetransfer->inbound == false)
    {
        if(m_filetransfer->file.tell() < m_filetransfer->filesize)
        {
            SendFile(msg_queue);
            return true;
        }

        CloseTransfer();
    }
    else if(m_sendbuf.length())
    {
        ACE_Time_Value tm = ACE_Time_Value::zero;

        if(QueueStreamData(msg_queue, m_sendbuf.c_str(), (int)m_sendbuf.length(), &tm) < 0)
        {
            MYTRACE(ACE_TEXT("Forcing disconnect of #%d %s. Buffer full\n"),
                    GetUserID(), GetNickname().c_str());
            return false;
        }
#if defined(UNICODE)
        MYTRACE(ACE_TEXT("SERVERUSER > #%d: %s"), GetUserID(), Utf8ToUnicode(m_sendbuf.c_str()).c_str());
#else
        MYTRACE(ACE_TEXT("SERVERUSER > #%d: %s"), GetUserID(), m_sendbuf.c_str());
#endif
        m_sendbuf.clear();
    }
    return true;
}

ACE_Time_Value ServerUser::GetDuration() const 
{
    return ACE_OS::gettimeofday() - m_LogonTime;
}

void ServerUser::ForwardChannels(const serverchannel_t& root, bool encrypted)
{
    TTASSERT(IsAuthorized());

    TTASSERT(root.get());
    std::queue<serverchannel_t> chanqueue;
    chanqueue.push(root);
    while(chanqueue.size())
    {
        serverchannel_t ch = chanqueue.front();
        chanqueue.pop();
        const ServerChannel::channels_t& subs = ch->GetSubChannels();
        for(size_t i=0;i<subs.size();i++)
            chanqueue.push(subs[i]);
        DoAddChannel(*ch.get(), encrypted);
    }
}

void ServerUser::ForwardUsers(const serverchannel_t& channel, bool recursive)
{
    TTASSERT(IsAuthorized());

    TTASSERT(channel.get());
    std::queue<serverchannel_t> chanqueue;
    chanqueue.push(channel);
    while(chanqueue.size())
    {
        serverchannel_t ch = chanqueue.front();
        chanqueue.pop();
        vector<serverchannel_t> subs = ch->GetSubChannels();
        for(size_t i=0;i<subs.size() && recursive;i++)
            chanqueue.push(subs[i]);
        ServerChannel::users_t users = ch->GetUsers();
        for(size_t i=0;i<users.size();i++)
            DoAddUser(*users[i].get(), *ch.get());
    }
}

void ServerUser::ForwardFiles(const serverchannel_t& root, bool recursive)
{
    TTASSERT(IsAuthorized());

    TTASSERT(root.get());
    std::queue<serverchannel_t> chanqueue;
    chanqueue.push(root);
    while(chanqueue.size())
    {
        serverchannel_t ch = chanqueue.front();
        chanqueue.pop();
        vector<serverchannel_t> subs = ch->GetSubChannels();
        for(size_t i=0;i<subs.size() && recursive;i++)
            chanqueue.push(subs[i]);
        files_t files;
        ch.get()->GetFiles(files, false);
        for(size_t i=0;i<files.size();i++)
            DoAddFile(files[i]);
    }
}

bool ServerUser::ProcessCommandQueue(bool clearsuspended)
{
    if(clearsuspended)
        m_cmdsuspended = false;
    
    ACE_CString cmd;
    ACE_CString remain;
    while(!m_cmdsuspended && GetCmdLine(m_recvbuf, cmd, remain))
    {
        switch(ProcessCommand(cmd, clearsuspended))
        {
        case CMD_ABORT : 
            return false;
        case CMD_DONE : 
            m_recvbuf = remain;
            break;
        case CMD_SUSPENDED :
            m_cmdsuspended = true;
            break;
        }
        clearsuspended = false;
    }
    return true;
 }

ServerUser::CmdProcessing ServerUser::ProcessCommand(const ACE_CString& cmdline, bool was_suspended)
{
    MYTRACE(ACE_TEXT("SERVERUSER < #%d: %s"), GetUserID(),
#if defined(UNICODE)
        Utf8ToUnicode(cmdline.c_str()).c_str()
#else
        cmdline.c_str()
#endif
    );

    ACE_CString tmp_cmd;
    if(!GetCmd(cmdline, tmp_cmd))
    {
        DoError(TT_CMDERR_UNKNOWN_COMMAND);
        return CMD_DONE;
    }

    if(!ValidUtf8(cmdline))
    {
        DoError(TT_CMDERR_SYNTAX_ERROR);
        return CMD_DONE;
    }

#if defined(UNICODE)
    ACE_TString cmd = Utf8ToUnicode(tmp_cmd.c_str(), int(tmp_cmd.length()));
    ACE_TString command = Utf8ToUnicode(cmdline.c_str(), int(cmdline.length()));
#else
    const ACE_TString& cmd = tmp_cmd;
    const ACE_CString& command = cmdline;
#endif

    mstrings_t properties;
    if(ExtractProperties(command, properties)<0)
    {
        DoError(TT_CMDERR_SYNTAX_ERROR);
        return CMD_DONE;
    }

    if(cmd == ACE_TString(CLIENT_QUIT))
    {
        return CMD_ABORT;
    }

    int cmdid = 0;
    GetProperty(properties, TT_CMDID, cmdid);

    if(cmdid && !was_suspended)
        DoBeginCmd(cmdid);

    // command flood protection
    if(!was_suspended && GetUserAccount().abuse.n_cmds && GetUserAccount().abuse.cmd_msec)
    {
        m_floodcmd.insert(GETTIMESTAMP());
    
        auto i = m_floodcmd.lower_bound(GETTIMESTAMP() - GetUserAccount().abuse.cmd_msec);
        if(i != m_floodcmd.begin())
        {
            m_floodcmd.erase(m_floodcmd.begin(), i);
        }
        
        if(int(m_floodcmd.size()) > GetUserAccount().abuse.n_cmds)
        {
            DoError(TT_CMDERR_COMMAND_FLOOD);

            if(cmdid)
                DoEndCmd(cmdid);

            return CMD_DONE;
        }
    }

    ErrorMsg err = HandleCommand(cmd, properties);
    
    if (err.errorno == TT_SRVERR_COMMAND_SUSPEND)
        return CMD_SUSPENDED;
    else
        DoError(err);

    if(cmdid)
        DoEndCmd(cmdid);

    return CMD_DONE;
}

ErrorMsg ServerUser::HandleCommand(const ACE_TString& cmd, const mstrings_t& properties)
{
    //determine which commands will be executed and perform
    //state check
    if(cmd == CLIENT_LOGIN)
    {
        if(IsAuthorized())
            return TT_CMDERR_ALREADY_LOGGEDIN;
        else
            return HandleLogin(properties);
    }
    else if(cmd == CLIENT_LOGOUT)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleLogout(properties);
    }
    else if(cmd == CLIENT_CHANGENICK) 
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleChangeNick(properties);
    }
    else if(cmd == CLIENT_KEEPALIVE)
    {
        return HandleKeepAlive(properties);
    }
    else if(cmd == CLIENT_JOINCHANNEL)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleJoinChannel(properties);
    }
    else if(cmd == CLIENT_LEAVECHANNEL)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleLeaveChannel(properties);
    }
    else if(cmd == CLIENT_CHANGESTATUS)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleChangeStatus(properties);
    }
    else if(cmd == CLIENT_MESSAGE)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleMessage(properties);
    }
    else if(cmd == CLIENT_KICK)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleKick(properties);
    }
    else if(cmd == CLIENT_MAKECHANNEL)
    { 
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else 
            return HandleMakeChannel(properties); 
    }
    else if(cmd == CLIENT_UPDATECHANNEL)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleUpdateChannel(properties); 
    }
    else if(cmd == CLIENT_REMOVECHANNEL)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleRemoveChannel(properties); 
    }
    else if(cmd == CLIENT_MOVEUSER)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleMoveUser(properties);
    }
    else if(cmd == CLIENT_UPDATESERVER)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleUpdateServer(properties);
    }
    else if(cmd == CLIENT_SAVECONFIG)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleSaveConfig(properties);
    }
    else if(cmd == CLIENT_CHANNELOP)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleChannelOp(properties);
    }
    else if(cmd == CLIENT_BAN)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleUserBan(properties);
    }
    else if(cmd == CLIENT_UNBAN)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleUserUnban(properties);
    }
    else if(cmd == CLIENT_LISTBANS)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleListServerBans(properties);
    }
    else if(cmd == CLIENT_REGSENDFILE)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleRegSendFile(properties);
    }
    else if(cmd == CLIENT_REGRECVFILE)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleRegRecvFile(properties);
    }
    else if(cmd == CLIENT_SENDFILE)
    {
        if(IsAuthorized())
            return TT_CMDERR_ALREADY_LOGGEDIN;
        else
            return HandleSendFile(properties);
    }
    else if(cmd == CLIENT_RECVFILE)
    {
        if(IsAuthorized())
            return TT_CMDERR_ALREADY_LOGGEDIN;
        else
            return HandleRecvFile(properties);
    }
    else if(cmd == CLIENT_DELIVERFILE)
    {
        return HandleFileDeliver(properties);
    }
    else if(cmd == CLIENT_DELETEFILE)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleDeleteFile(properties);
    }
    else if(cmd == CLIENT_SUBSCRIBE)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleSubscribe(properties);
    }
    else if(cmd == CLIENT_UNSUBSCRIBE)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleUnsubscribe(properties);
    }
    else if(cmd == CLIENT_LISTUSERACCOUNTS)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleListUserAccounts(properties);
    }
    else if(cmd == CLIENT_NEWUSERACCOUNT)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleNewUserAccount(properties);
    }
    else if(cmd == CLIENT_DELUSERACCOUNT)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleDeleteUserAccount(properties);
    }
    else if(cmd == CLIENT_QUERYSTATS)
    {
        if(!IsAuthorized())
            return TT_CMDERR_NOT_LOGGEDIN;
        else
            return HandleQueryStats(properties);
    }

    return TT_CMDERR_UNKNOWN_COMMAND;
}

//////////////////////////////
//    CLIENT TO SERVER COMMANDS
//////////////////////////////
ErrorMsg ServerUser::HandleLogin(const mstrings_t& properties)
{
    ACE_TString username, passwd;
    //get protocol
    GetProperty(properties, TT_PROTOCOL, m_stream_protocol);
    GetProperty(properties, TT_VERSION, m_version);
    
    GetProperty(properties, TT_NICKNAME, m_nickname);
    GetProperty(properties, TT_USERNAME, username);
    GetProperty(properties, TT_PASSWORD, passwd);
    GetProperty(properties, TT_CLIENTNAME, m_clientname);

    ErrorMsg err = m_servernode.UserLogin(GetUserID(), username, passwd);
    if(!err.success())
    {
        m_nickname.clear();
        m_stream_protocol.clear();
    }
    else
    {
        m_LogonTime = ACE_OS::gettimeofday();
    }
    return err;
}

ErrorMsg ServerUser::HandleLogout(const mstrings_t& properties)
{
    return m_servernode.UserLogout(GetUserID());
}

ErrorMsg ServerUser::HandleChangeNick(const mstrings_t& properties)
{
    ACE_TString nick;
    GET_PROP_OR_RETURN(properties, TT_NICKNAME, nick);

    return m_servernode.UserChangeNickname(GetUserID(), nick);
}

ErrorMsg ServerUser::HandleChangeStatus(const mstrings_t& properties)
{
    int mode;
    ACE_TString msg;
    GET_PROP_OR_RETURN(properties, TT_STATUSMODE, mode);

    GetProperty(properties, TT_STATUSMESSAGE, msg);

    return m_servernode.UserChangeStatus(GetUserID(), mode, msg);
}

ErrorMsg ServerUser::HandleMessage(const mstrings_t& properties)
{
    TextMessage txtmsg;
    int m = 0;
    GET_PROP_OR_RETURN(properties, TT_MSGTYPE, m);
    GET_PROP_OR_RETURN(properties, TT_MSGCONTENT, txtmsg.content);

    switch(m)
    {
    case TTUserMsg :
    case TTCustomMsg :
        GET_PROP_OR_RETURN(properties, TT_DESTUSERID, txtmsg.to_userid);
        break;
    case TTChannelMsg :
        GET_PROP_OR_RETURN(properties, TT_CHANNELID, txtmsg.channelid);
        break;
    case TTBroadcastMsg :
        break;
    }
    txtmsg.msgType = (MsgType)m;
    txtmsg.from_userid = GetUserID();

    return m_servernode.UserTextMessage(txtmsg);
}

ErrorMsg ServerUser::HandleKeepAlive(const mstrings_t& properties)
{
    SetLastKeepAlive(0);
    DoPingReply();

    return TT_CMDERR_IGNORE;
}

ErrorMsg ServerUser::HandleJoinChannel(const mstrings_t& properties)
{
    const ServerProperties& srvprop = m_servernode.GetServerProperties();
    
    ChannelProp chanprop;
    GetProperty(properties, TT_CHANNELID, chanprop.channelid);
    GetProperty(properties, TT_CHANNAME, chanprop.name);
    GetProperty(properties, TT_PARENTID, chanprop.parentid);
    GetProperty(properties, TT_TOPIC, chanprop.topic);
    GetProperty(properties, TT_OPPASSWORD, chanprop.oppasswd);
    if(HasProperty(properties, TT_AUDIOCODEC) &&
       !GetProperty(properties, TT_AUDIOCODEC, chanprop.audiocodec))
    {
        return TT_CMDERR_UNKNOWN_AUDIOCODEC;
    }
    GetProperty(properties, TT_AUDIOCFG, chanprop.audiocfg);
    GetProperty(properties, TT_CHANNELTYPE, chanprop.chantype);
    GetProperty(properties, TT_USERDATA, chanprop.userdata);
    GetProperty(properties, TT_VOICEUSERS, chanprop.transmitusers[STREAMTYPE_VOICE]);
    GetProperty(properties, TT_VIDEOUSERS, chanprop.transmitusers[STREAMTYPE_VIDEOCAPTURE]);
    GetProperty(properties, TT_DESKTOPUSERS, chanprop.transmitusers[STREAMTYPE_DESKTOP]);
    GetProperty(properties, TT_MEDIAFILEUSERS, chanprop.transmitusers[STREAMTYPE_MEDIAFILE]);
    GetProperty(properties, TT_PASSWORD, chanprop.passwd);

    if(chanprop.name.find(CHANNEL_SEPARATOR) != ACE_TString::npos)
    {
        return TT_CMDERR_SYNTAX_ERROR;
    }

    //set default settings
    chanprop.diskquota = srvprop.diskquota;

    return m_servernode.UserJoinChannel(GetUserID(), chanprop);
}

ErrorMsg ServerUser::HandleLeaveChannel(const mstrings_t& properties)
{
    serverchannel_t chan = GetChannel();
    if(chan.null())
    {
        return TT_CMDERR_NOT_IN_CHANNEL;
    }

    return m_servernode.UserLeaveChannel(GetUserID(), chan->GetChannelID());
}

ErrorMsg ServerUser::HandleKick(const mstrings_t& properties)
{
    int kick_userid = 0, channelid = 0;
    GET_PROP_OR_RETURN(properties, TT_USERID, kick_userid);

    GetProperty(properties, TT_CHANNELID, channelid);

    return m_servernode.UserKick(GetUserID(), kick_userid, 
                                 channelid, false);
}

ErrorMsg ServerUser::HandleChannelOp(const mstrings_t& properties)
{
    int userid = 0, channelid = 0;
    ACE_TString oppasswd;
    bool op = false;
    GET_PROP_OR_RETURN(properties, TT_USERID, userid);
    GET_PROP_OR_RETURN(properties, TT_CHANNELID, channelid);
    GET_PROP_OR_RETURN(properties, TT_OPERATORSTATUS, op);
    GetProperty(properties, TT_OPPASSWORD, oppasswd);

    return m_servernode.UserOpDeOp(GetUserID(), channelid, oppasswd, 
                                   userid, op);
}

ErrorMsg ServerUser::HandleMakeChannel(const mstrings_t& properties)
{
    if( (GetUserRights() & USERRIGHT_MODIFY_CHANNELS) == 0)
    {
        return TT_CMDERR_NOT_AUTHORIZED;
    }

    ChannelProp chanprop;
    GET_PROP_OR_RETURN(properties, TT_PARENTID, chanprop.parentid);
    GET_PROP_OR_RETURN(properties, TT_CHANNAME, chanprop.name);
    GetProperty(properties, TT_PASSWORD, chanprop.passwd);
    GetProperty(properties, TT_TOPIC, chanprop.topic);
    GetProperty(properties, TT_DISKQUOTA, chanprop.diskquota);
    GetProperty(properties, TT_OPPASSWORD, chanprop.oppasswd);
    GetProperty(properties, TT_MAXUSERS, chanprop.maxusers);
    if(!GetProperty(properties, TT_AUDIOCODEC, chanprop.audiocodec))
    {
        return TT_CMDERR_UNKNOWN_AUDIOCODEC;
    }
    GetProperty(properties, TT_AUDIOCFG, chanprop.audiocfg);
    GetProperty(properties, TT_CHANNELTYPE, chanprop.chantype);
    GetProperty(properties, TT_USERDATA, chanprop.userdata);
    GetProperty(properties, TT_VOICEUSERS, chanprop.transmitusers[STREAMTYPE_VOICE]);
    GetProperty(properties, TT_VIDEOUSERS, chanprop.transmitusers[STREAMTYPE_VIDEOCAPTURE]);
    GetProperty(properties, TT_DESKTOPUSERS, chanprop.transmitusers[STREAMTYPE_DESKTOP]);
    GetProperty(properties, TT_MEDIAFILEUSERS, chanprop.transmitusers[STREAMTYPE_MEDIAFILE]);

    if(chanprop.name.find(CHANNEL_SEPARATOR) != ACE_TString::npos)
    {
        return TT_CMDERR_SYNTAX_ERROR;
    }

    return m_servernode.MakeChannel(chanprop, this);
}

ErrorMsg ServerUser::HandleUpdateChannel(const mstrings_t& properties)
{
    ChannelProp chanprop;

    GET_PROP_OR_RETURN(properties, TT_CHANNELID, chanprop.channelid);

    //TODO: Guard in m_servernode.GetChannelProp()
    if(!m_servernode.GetChannelProp(chanprop.channelid, chanprop))
    {
        return TT_CMDERR_CHANNEL_NOT_FOUND;
    }

    GetProperty(properties, TT_CHANNAME, chanprop.name);
    GetProperty(properties, TT_PASSWORD, chanprop.passwd);
    GetProperty(properties, TT_TOPIC, chanprop.topic);
    GetProperty(properties, TT_OPPASSWORD, chanprop.oppasswd);
    GetProperty(properties, TT_CHANNELTYPE, chanprop.chantype);
    GetProperty(properties, TT_USERDATA, chanprop.userdata);
    if(HasProperty(properties, TT_VOICEUSERS))
        chanprop.transmitusers[STREAMTYPE_VOICE].clear();
    GetProperty(properties, TT_VOICEUSERS, chanprop.transmitusers[STREAMTYPE_VOICE]);
    if(HasProperty(properties, TT_VIDEOUSERS))
        chanprop.transmitusers[STREAMTYPE_VIDEOCAPTURE].clear();
    GetProperty(properties, TT_VIDEOUSERS, chanprop.transmitusers[STREAMTYPE_VIDEOCAPTURE]);
    if(HasProperty(properties, TT_DESKTOPUSERS))
        chanprop.transmitusers[STREAMTYPE_DESKTOP].clear();
    GetProperty(properties, TT_DESKTOPUSERS, chanprop.transmitusers[STREAMTYPE_DESKTOP]);
    if(HasProperty(properties, TT_MEDIAFILEUSERS))
        chanprop.transmitusers[STREAMTYPE_MEDIAFILE].clear();
    GetProperty(properties, TT_MEDIAFILEUSERS, chanprop.transmitusers[STREAMTYPE_MEDIAFILE]);

    if(GetUserRights() & USERRIGHT_MODIFY_CHANNELS)
    {
        //only admin can change these properties
        GetProperty(properties, TT_DISKQUOTA, chanprop.diskquota);
        GetProperty(properties, TT_MAXUSERS, chanprop.maxusers);
        //users with only temp-channels cannot change codec (since
        //they're in the channel)
        if(HasProperty(properties, TT_AUDIOCODEC) &&
           !GetProperty(properties, TT_AUDIOCODEC, chanprop.audiocodec))
        {
            return TT_CMDERR_UNKNOWN_AUDIOCODEC;
        }
    }
    GetProperty(properties, TT_AUDIOCFG, chanprop.audiocfg);

    return m_servernode.UserUpdateChannel(GetUserID(), chanprop);
}

ErrorMsg ServerUser::HandleRemoveChannel(const mstrings_t& properties)
{
    int channelid = 0;
    GET_PROP_OR_RETURN(properties, TT_CHANNELID, channelid);

    if( (GetUserRights() & USERRIGHT_MODIFY_CHANNELS) == 0)
    {
        return TT_CMDERR_NOT_AUTHORIZED;
    }

    return m_servernode.RemoveChannel(channelid, this);
}

ErrorMsg ServerUser::HandleMoveUser(const mstrings_t& properties)
{
    int userid = 0, channelid = 0;
    GET_PROP_OR_RETURN(properties, TT_CHANNELID, channelid);
    GET_PROP_OR_RETURN(properties, TT_USERID, userid);

    return m_servernode.UserMove(GetUserID(), userid, channelid);
}

ErrorMsg ServerUser::HandleUpdateServer(const mstrings_t& properties)
{
    //extract properties
    ServerProperties srvprop = m_servernode.GetServerProperties();

    GetProperty(properties, TT_AUTOSAVE, srvprop.autosave);
    GetProperty(properties, TT_MOTDRAW, srvprop.motd);
    GetProperty(properties, TT_SERVERNAME, srvprop.servername);
    GetProperty(properties, TT_MAXUSERS, srvprop.maxusers);
    GetProperty(properties, TT_MAXLOGINATTEMPTS, srvprop.maxloginattempts);
    GetProperty(properties, TT_MAXLOGINSPERIP, srvprop.max_logins_per_ipaddr);
    GetProperty(properties, TT_VOICETXLIMIT, srvprop.voicetxlimit);
    GetProperty(properties, TT_VIDEOTXLIMIT, srvprop.videotxlimit);
    GetProperty(properties, TT_MEDIAFILETXLIMIT, srvprop.mediafiletxlimit);
    GetProperty(properties, TT_DESKTOPTXLIMIT, srvprop.desktoptxlimit);
    GetProperty(properties, TT_TOTALTXLIMIT, srvprop.totaltxlimit);

    int tcpport = srvprop.tcpaddr.get_port_number();
    int udpport = srvprop.udpaddr.get_port_number();
    GetProperty(properties, TT_TCPPORT, tcpport);
    GetProperty(properties, TT_UDPPORT, udpport);
    srvprop.tcpaddr.set_port_number(tcpport);
    srvprop.udpaddr.set_port_number(udpport);

    GetProperty(properties, TT_USERTIMEOUT, srvprop.usertimeout);

    return m_servernode.UserUpdateServer(GetUserID(), srvprop);
}

ErrorMsg ServerUser::HandleSaveConfig(const mstrings_t& properties)
{
    if((GetUserType() & USERTYPE_ADMIN) == 0)
    {
        return TT_CMDERR_NOT_AUTHORIZED;
    }

    return m_servernode.UserSaveServerConfig(GetUserID());
}

ErrorMsg ServerUser::HandleListUserAccounts(const mstrings_t& properties)
{
    if( (GetUserType() & USERTYPE_ADMIN) == 0)
    {
        return TT_CMDERR_NOT_AUTHORIZED;
    }

    int index = 0, count = 1000000;
    
    GetProperty(properties, TT_INDEX, index);
    GetProperty(properties, TT_COUNT, count);

    return m_servernode.UserListUserAccounts(GetUserID(), 
                                             index, count);
}

ErrorMsg ServerUser::HandleNewUserAccount(const mstrings_t& properties)
{
    UserAccount account;
    GET_PROP_OR_RETURN(properties, TT_USERNAME, account.username);
    GET_PROP_OR_RETURN(properties, TT_PASSWORD, account.passwd);
    GET_PROP_OR_RETURN(properties, TT_USERTYPE, account.usertype);
    GetProperty(properties, TT_USERRIGHTS, account.userrights);
    GetProperty(properties, TT_USERDATA, account.userdata);
    GetProperty(properties, TT_NOTEFIELD, account.note);
    GetProperty(properties, TT_INITCHANNEL, account.init_channel);
    GetProperty(properties, TT_AUTOOPCHANNELS, account.auto_op_channels);
    GetProperty(properties, TT_AUDIOBPSLIMIT, account.audiobpslimit);
    vector<int> flood;
    if(GetProperty(properties, TT_CMDFLOOD, flood))
        account.abuse.fromParam(flood);

    return m_servernode.UserNewUserAccount(GetUserID(), account);
}

ErrorMsg ServerUser::HandleDeleteUserAccount(const mstrings_t& properties)
{
    ACE_TString username;
    GET_PROP_OR_RETURN(properties, TT_USERNAME, username);

    return m_servernode.UserDeleteUserAccount(GetUserID(), username);
}

ErrorMsg ServerUser::HandleUserBan(const mstrings_t& properties)
{
    int ban_userid = 0, chanid = 0;
    BannedUser ban;
    ban.bantype = BANTYPE_DEFAULT; //default pre-TT-protocol 5.3

    GetProperty(properties, TT_USERID, ban_userid);
    GetProperty(properties, TT_IPADDR, ban.ipaddr);
    GetProperty(properties, TT_BANTYPE, ban.bantype);
    GetProperty(properties, TT_USERNAME, ban.username);
    GetProperty(properties, TT_CHANNEL, ban.chanpath);
    GetProperty(properties, TT_CHANNELID, chanid);

    if (chanid)
        ban.bantype |= BANTYPE_CHANNEL;

    //ban if admin
    return m_servernode.UserBan(GetUserID(), ban_userid, ban);
}

ErrorMsg ServerUser::HandleUserUnban(const mstrings_t& properties)
{
    BannedUser ban;
    ban.bantype = BANTYPE_DEFAULT; //default pre-TT-protocol 5.3

    GetProperty(properties, TT_IPADDR, ban.ipaddr);
    GetProperty(properties, TT_BANTYPE, ban.bantype);
    GetProperty(properties, TT_USERNAME, ban.username);
    GetProperty(properties, TT_CHANNEL, ban.chanpath);

    //ban if admin
    return m_servernode.UserUnBan(GetUserID(), ban);
}

ErrorMsg ServerUser::HandleListServerBans(const mstrings_t& properties)
{
    int index = 0, count = 1000000, chanid = 0;
    
    GetProperty(properties, TT_CHANNELID, chanid);
    GetProperty(properties, TT_INDEX, index);
    GetProperty(properties, TT_COUNT, count);

    //ban if admin
    return m_servernode.UserListServerBans(GetUserID(), chanid, index, count);
}

ErrorMsg ServerUser::HandleRegSendFile(const mstrings_t& properties)
{
    FileTransfer transfer;
    transfer.transferid = 0;
    transfer.inbound = true;
    transfer.userid = GetUserID();

    GET_PROP_OR_RETURN(properties, TT_CHANNELID, transfer.channelid);
    GET_PROP_OR_RETURN(properties, TT_FILENAME, transfer.filename);
    GET_PROP_OR_RETURN(properties, TT_FILESIZE, transfer.filesize);

    return m_servernode.UserRegFileTransfer(transfer);
}

ErrorMsg ServerUser::HandleRegRecvFile(const mstrings_t& properties)
{
    FileTransfer transfer;
    transfer.transferid = 0;
    transfer.inbound = false;
    transfer.userid = GetUserID();

    GET_PROP_OR_RETURN(properties, TT_CHANNELID, transfer.channelid);
    GET_PROP_OR_RETURN(properties, TT_FILENAME, transfer.filename);

    return m_servernode.UserRegFileTransfer(transfer);
}

ErrorMsg ServerUser::HandleSendFile(const mstrings_t& properties)
{
    int transferid = 0;
    GET_PROP_OR_RETURN(properties, TT_TRANSFERID, transferid);

    m_filetransfer.reset(new LocalFileTransfer());

    FileTransfer transfer;
    ErrorMsg err = m_servernode.UserBeginFileTransfer(transferid, transfer, m_filetransfer->file);
    if(err.errorno != TT_CMDERR_SUCCESS)
    {
        DoError(err);
        m_filetransfer.reset();
    }
    else
    {
        m_filetransfer->filesize = ACE_OFF_T(transfer.filesize);
        m_filetransfer->inbound = true;
        m_filetransfer->transferid = transfer.transferid;

        int size = FILEBUFFERSIZE, optlen = sizeof(size);
        DoFileDeliver(transfer);
        m_filetransfer->active = true;
    }
    return TT_CMDERR_IGNORE;
}

ErrorMsg ServerUser::HandleRecvFile(const mstrings_t& properties)
{
    int transferid = 0;
    GET_PROP_OR_RETURN(properties, TT_TRANSFERID, transferid);

    m_filetransfer.reset(new LocalFileTransfer());

    FileTransfer transfer;
    ErrorMsg err = m_servernode.UserBeginFileTransfer(transferid, transfer, m_filetransfer->file);
    if(err.errorno != TT_CMDERR_SUCCESS)
    {
        m_filetransfer.reset();
        DoError(err);
    }
    else
    {
        m_filetransfer->filesize = ACE_OFF_T(transfer.filesize);
        m_filetransfer->transferid = transfer.transferid;
        m_filetransfer->inbound = false;
        DoFileReady();
    }
    return TT_CMDERR_IGNORE;
}

ErrorMsg ServerUser::HandleFileDeliver(const mstrings_t& properties)
{
    TTASSERT(m_filetransfer.get());
    if(!m_filetransfer.get())
        return TT_CMDERR_FILETRANSFER_NOT_FOUND;
    m_filetransfer->active = true;

    if(m_stream_handle != ACE_INVALID_HANDLE)
        m_servernode.RegisterStreamCallback(m_stream_handle);
    return TT_CMDERR_IGNORE;
}

ErrorMsg ServerUser::HandleDeleteFile(const mstrings_t& properties)
{
    int channelid = 0;
    ACE_TString filename;
    GET_PROP_OR_RETURN(properties, TT_FILENAME, filename);
    GET_PROP_OR_RETURN(properties, TT_CHANNELID, channelid);

    return m_servernode.UserDeleteFile(GetUserID(), channelid, filename);
}

ErrorMsg ServerUser::HandleSubscribe(const mstrings_t& properties)
{
    int userid = 0;
    Subscriptions subscript = SUBSCRIBE_NONE;
    GET_PROP_OR_RETURN(properties, TT_USERID, userid);
    GET_PROP_OR_RETURN(properties, TT_LOCALSUBSCRIPTIONS, subscript);

    return m_servernode.UserSubscribe(GetUserID(), userid, subscript);
}

ErrorMsg ServerUser::HandleUnsubscribe(const mstrings_t& properties)
{
    int userid = 0;
    Subscriptions subscript = SUBSCRIBE_NONE;
    GET_PROP_OR_RETURN(properties, TT_USERID, userid);
    GET_PROP_OR_RETURN(properties, TT_LOCALSUBSCRIPTIONS, subscript);

    return m_servernode.UserUnsubscribe(GetUserID(), userid, subscript);
}

ErrorMsg ServerUser::HandleQueryStats(const mstrings_t& properties)
{
    if((GetUserType() & USERTYPE_ADMIN) == 0)
    {
        return TT_CMDERR_NOT_AUTHORIZED;
    }
    DoServerStats();
    return TT_CMDERR_SUCCESS;
}

BannedUser ServerUser::GetBan(BanTypes bantype, const ACE_TString& chanpath) const
{
    BannedUser ban;
    ban.bantype = bantype;
    ban.nickname = GetNickname();
    ban.username = GetUsername();
    if ((bantype & BANTYPE_CHANNEL) && chanpath.length())
        ban.chanpath = chanpath;
    else if (!GetChannel().null())
        ban.chanpath = GetChannel()->GetChannelPath();
    ban.ipaddr = GetIpAddress();
    return ban;
}

void ServerUser::AddSubscriptions(const ServerUser& user, Subscriptions subscribe)
{
#ifdef _DEBUG
    MYTRACE(ACE_TEXT("Added subscription: %d -> %d, "), GetUserID(), user.GetUserID());
    MYTRACE(ACE_TEXT("audio=%d "), (int)(subscribe & SUBSCRIBE_VOICE) == SUBSCRIBE_VOICE);
    MYTRACE(ACE_TEXT("video=%d "), (int)(subscribe & SUBSCRIBE_VIDEOCAPTURE) == SUBSCRIBE_VIDEOCAPTURE);
    MYTRACE(ACE_TEXT("media=%d "), (int)(subscribe & SUBSCRIBE_MEDIAFILE) == SUBSCRIBE_MEDIAFILE);
    MYTRACE(ACE_TEXT("usermsg=%d "), (int)(subscribe & SUBSCRIBE_USER_MSG) == SUBSCRIBE_USER_MSG);
    MYTRACE(ACE_TEXT("chanmsg=%d "), (int)(subscribe & SUBSCRIBE_CHANNEL_MSG) == SUBSCRIBE_CHANNEL_MSG);
    MYTRACE(ACE_TEXT("bcast=%d "), (int)(subscribe & SUBSCRIBE_BROADCAST_MSG) == SUBSCRIBE_BROADCAST_MSG);
    MYTRACE(ACE_TEXT("desktop=%d "), (int)(subscribe & SUBSCRIBE_DESKTOP) == SUBSCRIBE_DESKTOP);
    MYTRACE(ACE_TEXT("desktopinput=%d\n"), (int)(subscribe & SUBSCRIBE_DESKTOPINPUT) == SUBSCRIBE_DESKTOPINPUT);
#endif

    Subscriptions cur_subscriptions = GetSubscriptions(user);
    m_usersubscriptions[user.GetUserID()] = (cur_subscriptions | subscribe);

    if(user.GetUserID() == GetUserID() &&
       GetSubscriptions(user) == SUBSCRIBE_LOCAL_DEFAULT)
        m_usersubscriptions.erase(user.GetUserID());
    else if(user.GetUserID() != GetUserID() &&
            GetSubscriptions(user) == SUBSCRIBE_PEER_DEFAULT)
        m_usersubscriptions.erase(user.GetUserID());

    TTASSERT((GetSubscriptions(user) & subscribe) == subscribe);
}

void ServerUser::ClearSubscriptions(const ServerUser& user, Subscriptions subscribe)
{
    Subscriptions cur_subscriptions = GetSubscriptions(user);
    m_usersubscriptions[user.GetUserID()] = (cur_subscriptions & ~subscribe);

    if(user.GetUserID() == GetUserID() &&
       GetSubscriptions(user) == SUBSCRIBE_LOCAL_DEFAULT)
        m_usersubscriptions.erase(user.GetUserID());
    else if(user.GetUserID() != GetUserID() &&
            GetSubscriptions(user) == SUBSCRIBE_PEER_DEFAULT)
        m_usersubscriptions.erase(user.GetUserID());

    TTASSERT((GetSubscriptions(user) & subscribe) == SUBSCRIBE_NONE);

#ifdef _DEBUG
    MYTRACE(ACE_TEXT("Cleared subscription: %d -> %d, "), GetUserID(), user.GetUserID());
    MYTRACE(ACE_TEXT("audio=%d "), (int)(subscribe & SUBSCRIBE_VOICE) == SUBSCRIBE_VOICE);
    MYTRACE(ACE_TEXT("video=%d "), (int)(subscribe & SUBSCRIBE_VIDEOCAPTURE) == SUBSCRIBE_VIDEOCAPTURE);
    MYTRACE(ACE_TEXT("media=%d "), (int)(subscribe & SUBSCRIBE_MEDIAFILE) == SUBSCRIBE_MEDIAFILE);
    MYTRACE(ACE_TEXT("usermsg=%d "), (int)(subscribe & SUBSCRIBE_USER_MSG) == SUBSCRIBE_USER_MSG);
    MYTRACE(ACE_TEXT("chanmsg=%d "), (int)(subscribe & SUBSCRIBE_CHANNEL_MSG) == SUBSCRIBE_CHANNEL_MSG);
    MYTRACE(ACE_TEXT("bcast=%d "), (int)(subscribe & SUBSCRIBE_BROADCAST_MSG) == SUBSCRIBE_BROADCAST_MSG);
    MYTRACE(ACE_TEXT("desktop=%d "), (int)(subscribe & SUBSCRIBE_DESKTOP) == SUBSCRIBE_DESKTOP);
    MYTRACE(ACE_TEXT("desktopinput=%d\n"), (int)(subscribe & SUBSCRIBE_DESKTOPINPUT) == SUBSCRIBE_DESKTOPINPUT);
#endif
}

Subscriptions ServerUser::GetSubscriptions(const ServerUser& user) const
{
    Subscriptions result;
    if(user.GetUserID() == GetUserID())
        result = SUBSCRIBE_LOCAL_DEFAULT;
    else
        result = SUBSCRIBE_PEER_DEFAULT;

    if(m_usersubscriptions.size())
    {
        usersubscriptions_t::const_iterator ite;
        ite = m_usersubscriptions.find(user.GetUserID());
        if(ite != m_usersubscriptions.end())
            return ite->second;
    }

    return result;
}

void ServerUser::ClearUserSubscription(const ServerUser& user)
{
    m_usersubscriptions.erase(user.GetUserID());
}

void ServerUser::HandleBinaryFileWrite(const char* buff, int len, bool& bContinue)
{
    TTASSERT(m_filetransfer.get());
    if(!m_filetransfer.get())
        return;

    m_filetransfer->file.send_n(buff, len);

    if(m_filetransfer->file.tell() == m_filetransfer->filesize)
    {
        m_filetransfer->active = false;

        DoFileCompleted();

        TTASSERT(m_filetransfer->transferid);
        ErrorMsg err = m_servernode.UserEndFileTransfer(m_filetransfer->transferid);
        MYTRACE_COND(err.errorno != TT_CMDERR_SUCCESS, 
                     ACE_TEXT("File transfer %d failed\n"), m_filetransfer->transferid);
        CloseTransfer();
    }
    else if(m_filetransfer->file.tell() > m_filetransfer->filesize)
    {
        bContinue = false;
    }
}

///////////////////////////////
//    SERVER TO CLIENT COMMANDS
///////////////////////////////

void ServerUser::DoError(ErrorMsg cmderr)
{
    if(cmderr.success())
        DoOk();
    else if (cmderr.errorno == TT_CMDERR_IGNORE)
    {
        //special case for 'ping' and file transfers
    }
    else
    {
        ACE_TString command = ACE_TString( SERVER_ERROR );
        AppendProperty(TT_ERRORNUM, cmderr.errorno, command);
        AppendProperty(TT_ERRORMSG, cmderr.errmsg, command);
        if(cmderr.errorno == TT_CMDERR_MISSING_PARAMETER)
            AppendProperty(TT_PARAMNAME, cmderr.paramname, command);
        command += ACE_TString(EOL);

        TransmitCommand(command);
    }
}

void ServerUser::DoWelcome(const ServerProperties& properties)
{
    ACE_TString command = properties.systemid;
    AppendProperty(TT_USERID, GetUserID(), command);
    AppendProperty(TT_SERVERNAME, properties.servername, command);
    AppendProperty(TT_MAXUSERS, properties.maxusers, command);
    AppendProperty(TT_MAXLOGINSPERIP, properties.max_logins_per_ipaddr, command);
    AppendProperty(TT_USERTIMEOUT, properties.usertimeout, command);
    AppendProperty(TT_PROTOCOL, ACE_TString(TEAMTALK_PROTOCOL_VERSION), command);

    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoServerUpdate(const ServerProperties& properties)
{
    TTASSERT(IsAuthorized());

    ACE_TString command;
    command = ACE_TString(SERVER_SERVERUPDATE);
    AppendProperty(TT_SERVERNAME, properties.servername, command);
    AppendProperty(TT_MAXUSERS, properties.maxusers, command);
    AppendProperty(TT_MAXLOGINSPERIP, properties.max_logins_per_ipaddr, command);
    AppendProperty(TT_USERTIMEOUT, properties.usertimeout, command);
    AppendProperty(TT_MOTD, m_servernode.GetMessageOfTheDay(), command);
    if(GetUserRights() & USERRIGHT_UPDATE_SERVERPROPERTIES)
    {
        AppendProperty(TT_MOTDRAW, properties.motd, command);
        AppendProperty(TT_MAXLOGINATTEMPTS, properties.maxloginattempts, command);
        AppendProperty(TT_AUTOSAVE, properties.autosave, command);
        AppendProperty(TT_TCPPORT, properties.tcpaddr.get_port_number(), command);
        AppendProperty(TT_UDPPORT, properties.udpaddr.get_port_number(), command);
    }
    AppendProperty(TT_VOICETXLIMIT, properties.voicetxlimit, command);
    AppendProperty(TT_VIDEOTXLIMIT, properties.videotxlimit, command);
    AppendProperty(TT_MEDIAFILETXLIMIT, properties.mediafiletxlimit, command);
    AppendProperty(TT_DESKTOPTXLIMIT, properties.desktoptxlimit, command);
    AppendProperty(TT_TOTALTXLIMIT, properties.totaltxlimit, command);

    AppendProperty(TT_VERSION, properties.version, command);

    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoAccepted(const UserAccount& useraccount)
{
    ACE_TString command;
    command = ACE_TString(SERVER_LOGINACCEPTED);
    AppendProperty(TT_USERID, GetUserID(), command);

    AppendProperty(TT_NICKNAME, GetNickname(), command);

    AppendProperty(TT_IPADDR, GetIpAddress(), command);
    //user account information
    AppendProperty(TT_USERNAME, useraccount.username, command);
    AppendProperty(TT_USERTYPE, useraccount.usertype, command);
    AppendProperty(TT_USERDATA, useraccount.userdata, command);
    AppendProperty(TT_USERRIGHTS, useraccount.userrights, command);
    AppendProperty(TT_NOTEFIELD, useraccount.note, command);
    AppendProperty(TT_INITCHANNEL, useraccount.init_channel, command);
    AppendProperty(TT_AUTOOPCHANNELS, useraccount.auto_op_channels, command);
    AppendProperty(TT_AUDIOBPSLIMIT, useraccount.audiobpslimit, command);
    AppendProperty(TT_CMDFLOOD, useraccount.abuse.toParam(), command);

    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoLoggedOut()
{
    TTASSERT(IsAuthorized());
    ACE_TString command;
    command = ACE_TString(SERVER_LOGGEDOUT);
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoLoggedIn(const ServerUser& user)
{
    TTASSERT(IsAuthorized());
    ACE_TString command;
    command = ACE_TString(SERVER_LOGGEDIN);
    AppendProperty(TT_USERID, user.GetUserID(), command);
    AppendProperty(TT_NICKNAME, user.GetNickname(), command);
    AppendProperty(TT_USERNAME, user.GetUsername(), command);
    if((GetUserRights() & USERRIGHT_BAN_USERS) ||
       user.GetUserID() == GetUserID())
    {
        AppendProperty(TT_IPADDR, user.GetIpAddress(), command);
    }
    AppendProperty(TT_STATUSMODE, user.GetStatusMode(), command);
    AppendProperty(TT_STATUSMESSAGE, user.GetStatusMessage(), command);
    AppendProperty(TT_VERSION, user.GetClientVersion(), command);
    AppendProperty(TT_PACKETPROTOCOL, user.GetPacketProtocol(), command);
    AppendProperty(TT_USERTYPE, user.GetUserType(), command);
    AppendProperty(TT_LOCALSUBSCRIPTIONS, GetSubscriptions(user), command);
    AppendProperty(TT_PEERSUBSCRIPTIONS, user.GetSubscriptions(*this), command);
    AppendProperty(TT_USERDATA, user.GetUserData(), command);
    AppendProperty(TT_CLIENTNAME, user.GetClientName(), command);
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoLoggedOut(const ServerUser& user)
{
    TTASSERT(IsAuthorized());
    ACE_TString command;
    command = ACE_TString(SERVER_LOGGEDOUT);
    AppendProperty(TT_USERID, user.GetUserID(), command);
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoAddUser(const ServerUser& user, const ServerChannel& channel)
{
    TTASSERT(IsAuthorized());

    ACE_TString command;
    command = ACE_TString(SERVER_ADDUSER);
    AppendProperty(TT_USERID, user.GetUserID(), command);
    AppendProperty(TT_NICKNAME, user.GetNickname(), command);
    AppendProperty(TT_USERNAME, user.GetUsername(), command);
    if((GetUserRights() & USERRIGHT_BAN_USERS) ||
       user.GetUserID() == GetUserID())
    {
        AppendProperty(TT_IPADDR, user.GetIpAddress(), command);
    }
    AppendProperty(TT_CHANNELID, channel.GetChannelID(), command);
    AppendProperty(TT_STATUSMODE, user.GetStatusMode(), command);
    AppendProperty(TT_STATUSMESSAGE, user.GetStatusMessage(), command);
    AppendProperty(TT_VERSION, user.GetClientVersion(), command);
    AppendProperty(TT_PACKETPROTOCOL, user.GetPacketProtocol(), command);
    AppendProperty(TT_USERTYPE, user.GetUserType(), command);
    AppendProperty(TT_LOCALSUBSCRIPTIONS, GetSubscriptions(user), command);
    AppendProperty(TT_PEERSUBSCRIPTIONS, user.GetSubscriptions(*this), command);
    AppendProperty(TT_USERDATA, user.GetUserData(), command);
    AppendProperty(TT_CLIENTNAME, user.GetClientName(), command);
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoUpdateUser(const ServerUser& user)
{
    TTASSERT(IsAuthorized());

    ACE_TString command;
    command = ACE_TString(SERVER_UPDATEUSER);
    AppendProperty(TT_USERID, user.GetUserID(),command);
    AppendProperty(TT_NICKNAME, user.GetNickname(),command);
    AppendProperty(TT_STATUSMODE, user.GetStatusMode(), command);
    AppendProperty(TT_STATUSMESSAGE, user.GetStatusMessage(), command);
    AppendProperty(TT_LOCALSUBSCRIPTIONS, GetSubscriptions(user), command);
    AppendProperty(TT_PEERSUBSCRIPTIONS, user.GetSubscriptions(*this), command);
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoRemoveUser(const ServerUser& user, const ServerChannel& channel)
{
    TTASSERT(IsAuthorized());

    ACE_TString command;
    command = ACE_TString(SERVER_REMOVEUSER);
    AppendProperty(TT_USERID, user.GetUserID(), command);
    AppendProperty(TT_CHANNELID, channel.GetChannelID(), command);
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoAddChannel(const ServerChannel& channel, bool encrypted)
{
    TTASSERT(IsAuthorized());

    const std::set<int>& setOps = channel.GetOperators();
    ACE_TString command;
    command = ACE_TString(SERVER_ADDCHANNEL);

    AppendProperty(TT_CHANNEL, channel.GetChannelPath(), command);
    AppendProperty(TT_CHANNELID, channel.GetChannelID(), command);
    if(channel.IsRootChannel())
        AppendProperty(TT_PARENTID, 0, command);
    else
    {
        AppendProperty(TT_PARENTID, channel.GetParentChannel()->GetChannelID(), command);
        AppendProperty(TT_CHANNAME, channel.GetName(), command);
    }
    if((GetUserRights() & USERRIGHT_MODIFY_CHANNELS) ||
       channel.IsOperator(GetUserID()))
    {
        AppendProperty(TT_PASSWORD, channel.GetPassword(), command);
        AppendProperty(TT_OPPASSWORD, channel.GetOpPassword(), command);
    }

    if(GetUserType() & USERTYPE_ADMIN)  //admins should see password
    {
#if defined(ENABLE_ENCRYPTION)
        if(encrypted)
            AppendProperty(TT_CRYPTKEY, KeyToHexString(channel.GetEncryptKey(),
                                                       CRYPTKEY_SIZE), command);
#endif
    }
    AppendProperty(TT_REQPASSWORD, channel.IsPasswordProtected(), command);

    AppendProperty(TT_TOPIC, channel.GetTopic(), command);
    AppendProperty(TT_OPERATORS, setOps, command);
    AppendProperty(TT_DISKQUOTA, channel.GetMaxDiskUsage(), command);
    AppendProperty(TT_MAXUSERS, channel.GetMaxUsers(), command);
    AppendProperty(TT_CHANNELTYPE, channel.GetChannelType(), command);
    AppendProperty(TT_USERDATA, channel.GetUserData(), command);    
    AppendProperty(TT_AUDIOCODEC, channel.GetAudioCodec(), command);
    AppendProperty(TT_AUDIOCFG, channel.GetAudioConfig(), command);
    if (channel.GetVoiceUsers().size())
        AppendProperty(TT_VOICEUSERS, channel.GetVoiceUsers(), command);
    if (channel.GetVideoUsers().size())
        AppendProperty(TT_VIDEOUSERS, channel.GetVideoUsers(), command);
    if (channel.GetDesktopUsers().size())
        AppendProperty(TT_DESKTOPUSERS, channel.GetDesktopUsers(), command);
    if (channel.GetMediaFileUsers().size())
        AppendProperty(TT_MEDIAFILEUSERS, channel.GetMediaFileUsers(), command);
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoUpdateChannel(const ServerChannel& channel, bool encrypted)
{
    TTASSERT(IsAuthorized());

    const std::set<int>& setOps = channel.GetOperators();

    ACE_TString command;
    command = ACE_TString(SERVER_UPDATECHANNEL);
    AppendProperty(TT_CHANNELID, channel.GetChannelID(), command);
    AppendProperty(TT_CHANNAME, channel.GetName(), command);

    if((GetUserRights() & USERRIGHT_MODIFY_CHANNELS) ||
       channel.IsOperator(GetUserID()))
    {
        AppendProperty(TT_PASSWORD, channel.GetPassword(), command);
        AppendProperty(TT_OPPASSWORD, channel.GetOpPassword(), command);
    }

    if(GetUserType() & USERTYPE_ADMIN)
    {
#if defined(ENABLE_ENCRYPTION)
        if(encrypted)
            AppendProperty(TT_CRYPTKEY, KeyToHexString(channel.GetEncryptKey(),
                                                       CRYPTKEY_SIZE), command);
#endif
    }
    AppendProperty(TT_REQPASSWORD, channel.IsPasswordProtected(), command);
    AppendProperty(TT_TOPIC, channel.GetTopic(), command);
    AppendProperty(TT_OPERATORS, setOps, command);
    AppendProperty(TT_DISKQUOTA, channel.GetMaxDiskUsage(), command);
    AppendProperty(TT_MAXUSERS, channel.GetMaxUsers(), command);
    AppendProperty(TT_CHANNELTYPE, channel.GetChannelType(), command);
    AppendProperty(TT_USERDATA, channel.GetUserData(), command);
    AppendProperty(TT_AUDIOCODEC, channel.GetAudioCodec(), command);
    AppendProperty(TT_AUDIOCFG, channel.GetAudioConfig(), command);
    if (channel.GetVoiceUsers().size())
        AppendProperty(TT_VOICEUSERS, channel.GetVoiceUsers(), command);
    if (channel.GetVideoUsers().size())
        AppendProperty(TT_VIDEOUSERS, channel.GetVideoUsers(), command);
    if (channel.GetDesktopUsers().size())
        AppendProperty(TT_DESKTOPUSERS, channel.GetDesktopUsers(), command);
    if (channel.GetMediaFileUsers().size())
        AppendProperty(TT_MEDIAFILEUSERS, channel.GetMediaFileUsers(), command);

    if(channel.GetChannelType() & CHANNEL_SOLO_TRANSMIT)
    {
        AppendProperty(TT_TRANSMITQUEUE, channel.GetTransmitQueue(), command);
    }
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoRemoveChannel(const ServerChannel& channel)
{
    TTASSERT(IsAuthorized());

    ACE_TString command;
    command = ACE_TString(SERVER_REMOVECHANNEL);
    AppendProperty(TT_CHANNELID, channel.GetChannelID(), command);
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoPingReply()
{
    ACE_TString command;
    command = ACE_TString(SERVER_KEEPALIVE) + ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoBeginCmd(int cmdID)
{
    ACE_TString command;
    command = ACE_TString(SERVER_BEGINCMD);
    AppendProperty(TT_CMDID, cmdID, command);
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoEndCmd(int cmdID)
{
    ACE_TString command;
    command = ACE_TString(SERVER_ENDCMD);
    AppendProperty(TT_CMDID, cmdID, command);
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoTextMessage(const ServerUser& fromuser, const TextMessage& msg)
{
    TTASSERT(IsAuthorized());
    //check whether user is subscribing to events
    ACE_TString command;
    command = ACE_TString(SERVER_MESSAGE_DELIVER);
    AppendProperty(TT_MSGTYPE, msg.msgType, command);
    AppendProperty(TT_SRCUSERID, msg.from_userid, command);
    AppendProperty(TT_MSGCONTENT, msg.content, command);

    switch(msg.msgType)
    {
    case TTChannelMsg :
        AppendProperty(TT_CHANNELID, msg.channelid, command);
        break;
    case TTUserMsg :
    case TTCustomMsg :
        AppendProperty(TT_DESTUSERID, msg.to_userid, command);
        break;
    }
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoTextMessage(const TextMessage& msg)
{
    TTASSERT(IsAuthorized());
    //check whether user is subscribing to events
    ACE_TString command;
    command = ACE_TString(SERVER_MESSAGE_DELIVER);
    AppendProperty(TT_MSGTYPE, msg.msgType, command);
    AppendProperty(TT_SRCUSERID, msg.from_userid, command);
    AppendProperty(TT_MSGCONTENT, msg.content, command);

    switch(msg.msgType)
    {
    case TTChannelMsg :
        AppendProperty(TT_CHANNELID, msg.channelid, command);
        break;
    case TTUserMsg :
    case TTCustomMsg :
        AppendProperty(TT_DESTUSERID, msg.to_userid, command);
        break;
    }
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoKicked(int kicker_userid, bool channel_kick)
{
    ACE_TString command;
    command = ACE_TString(SERVER_KICKED);
    AppendProperty(TT_KICKERID, kicker_userid, command);
    if(channel_kick && !GetChannel().null())
        AppendProperty(TT_CHANNELID, GetChannel()->GetChannelID(), command);

    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoJoinedChannel(const ServerChannel& channel, bool encrypted)
{
    TTASSERT(IsAuthorized());
    ACE_TString command;
    command = ACE_TString(SERVER_JOINED);
    AppendProperty(TT_CHANNELID, channel.GetChannelID(), command);
#if defined(ENABLE_ENCRYPTION)
    if(encrypted)
        AppendProperty(TT_CRYPTKEY, KeyToHexString(channel.GetEncryptKey(),
                                                   CRYPTKEY_SIZE), command);
#endif
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoLeftChannel(const ServerChannel& channel)
{
    TTASSERT(IsAuthorized());
    ACE_TString command;
    command = ACE_TString(SERVER_LEFTCHANNEL);
    AppendProperty(TT_CHANNELID, channel.GetChannelID(), command);
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoShowBan(const BannedUser& ban)
{
    TTASSERT(IsAuthorized());

    ACE_TString command;
    command = ACE_TString(SERVER_BANNED);

    AppendProperty(TT_BANTYPE, ban.bantype, command);
    AppendProperty(TT_IPADDR, ban.ipaddr, command);
    AppendProperty(TT_CHANNEL, ban.chanpath, command);
    AppendProperty(TT_NICKNAME, ban.nickname, command);
    AppendProperty(TT_USERNAME, ban.username, command);
    AppendProperty(TT_BANTIME, (ACE_INT64)ban.bantime.sec(), command);
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoShowUserAccount(const UserAccount& user)
{
    TTASSERT(IsAuthorized());

    ACE_TString command;
    command = ACE_TString(SERVER_USERACCOUNT);

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

    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoFileDeliver(const FileTransfer& transfer)
{
    TTASSERT(m_filetransfer.get());
    if(!m_filetransfer.get())
        return;

    ACE_TString command = SERVER_FILE_DELIVER;

    TTASSERT(transfer.filesize == m_filetransfer->filesize);
    AppendProperty(TT_FILESIZE, transfer.filesize, command);
    AppendProperty(TT_FILENAME, transfer.filename, command);
    command += EOL;

    TransmitCommand(command);
}

void ServerUser::DoFileCompleted()
{
    ACE_TString command = SERVER_FILE_COMPLETED;
    command += EOL;

    TransmitCommand(command);
}

void ServerUser::DoFileReady()
{
    TTASSERT(m_filetransfer.get());
    if(!m_filetransfer.get())
        return;

    ACE_TString command = SERVER_FILE_READY;
    AppendProperty(TT_TRANSFERID, m_filetransfer->transferid, command);
    AppendProperty(TT_FILESIZE, ACE_INT64(m_filetransfer->filesize), command);
    command += EOL;

    TransmitCommand(command);
}

void ServerUser::DoFileAccepted(const FileTransfer& transfer)
{
    TTASSERT(IsAuthorized());

    ACE_TString command;
    command = SERVER_FILE_ACCEPTED;
    AppendProperty(TT_TRANSFERID, transfer.transferid, command);
    command += EOL;

    TransmitCommand(command);
}

void ServerUser::DoAddFile(const RemoteFile& file)
{
    TTASSERT(IsAuthorized());

    ACE_TString command;
    command = SERVER_ADDFILE;
    AppendProperty(TT_FILENAME, file.filename, command);
    AppendProperty(TT_FILESIZE, file.filesize, command);
    AppendProperty(TT_FILEID, file.fileid, command);
    AppendProperty(TT_FILEOWNER, file.username, command);
    AppendProperty(TT_CHANNELID, file.channelid, command);
    command += EOL;

    TransmitCommand(command);
}

void ServerUser::DoRemoveFile(const ACE_TString& filename, const ServerChannel& channel)
{
    TTASSERT(IsAuthorized());

    ACE_TString command;
    command = SERVER_REMOVEFILE;
    AppendProperty(TT_FILENAME, filename, command);
    AppendProperty(TT_CHANNELID, channel.GetChannelID(), command);
    command += EOL;

    TransmitCommand(command);
}

void ServerUser::DoServerStats()
{
    const ServerStats& stats = m_servernode.GetServerStats();
    ACE_Time_Value tm = ACE_OS::gettimeofday() - stats.starttime;
    ACE_UINT64 msec = 0;
    tm.msec(msec);

    ACE_TString command;
    command = SERVER_STATS;
    AppendProperty(TT_TOTALTX, stats.total_bytessent, command);
    AppendProperty(TT_TOTALRX, stats.total_bytesreceived, command);
    AppendProperty(TT_VOICETX, stats.voice_bytessent, command);
    AppendProperty(TT_VOICERX, stats.voice_bytesreceived, command);
    AppendProperty(TT_VIDEOCAPTX, stats.vidcap_bytessent, command);
    AppendProperty(TT_VIDEOCAPRX, stats.vidcap_bytesreceived, command);
    AppendProperty(TT_MEDIAFILETX, stats.mediafile_bytessent, command);
    AppendProperty(TT_MEDIAFILERX, stats.mediafile_bytesreceived, command);
    AppendProperty(TT_DESKTOPTX, stats.desktop_bytessent, command);
    AppendProperty(TT_DESKTOPRX, stats.desktop_bytesreceived, command);
    AppendProperty(TT_USERSSERVED, stats.usersservered, command);
    AppendProperty(TT_USERSPEAK, stats.userspeak, command);
    AppendProperty(TT_FILESTX, stats.files_bytessent, command);
    AppendProperty(TT_FILESRX, stats.files_bytesreceived, command);
    AppendProperty(TT_UPTIME, (ACE_INT64)msec, command);
    command += EOL;

    TransmitCommand(command);
}

void ServerUser::DoOk()
{
    ACE_TString command;
    command = ACE_TString(SERVER_COMMAND_OK);
    command += ACE_TString(EOL);

    TransmitCommand(command);
}

void ServerUser::DoQuit()
{
    ACE_TString command;
    command = ACE_TString(SERVER_QUIT) + ACE_TString(EOL);

    TransmitCommand(command);

    //TRACE("SERVER: Sent quit to #%d nickname: %s channel: \"%s\"\n", GetUserID(), GetNickname().c_str(), (GetChannel()==NULL)? "" : GetChannel()->GetChannelPath().c_str());
}

void ServerUser::TransmitCommand(const ACE_TString& cmdline)
{
    TTASSERT(!m_filetransfer.get() || !m_filetransfer->active);

    if(m_stream_handle != ACE_INVALID_HANDLE)
    {
#if defined(UNICODE)
        m_sendbuf += UnicodeToUtf8(cmdline.c_str());
#else
        m_sendbuf += cmdline;
#endif
        m_servernode.RegisterStreamCallback(m_stream_handle);
    }
}

bool ServerUser::AddDesktopPacket(const DesktopPacket& packet)
{
    if(!m_desktop_cache.null() && 
        m_desktop_cache->GetSessionID() != packet.GetSessionID() &&
        W32_GEQ(packet.GetTime(), m_desktop_cache->GetCurrentDesktopTime()))
    {
        //delete current session and start new session
        CloseDesktopSession();
    }

    if(m_desktop_cache.null())
    {
        uint8_t session_id;
        uint16_t width, height, pkt_index, pkt_count;
        uint8_t bmp_mode;
        if(!packet.GetSessionProperties(&session_id, &width, &height, &bmp_mode,
            &pkt_index, &pkt_count))
        {
            //if there's any obsolete packets, delete them
            RemoveObsoleteDesktopPackets(packet, m_desktop_queue);

            //queue packet until we get its session properties
            DesktopPacket* p;
            ACE_NEW_RETURN(p, DesktopPacket(packet), false);
            m_desktop_queue.push_back(desktoppacket_t(p));
            MYTRACE(ACE_TEXT("Queued packet #%d due to missing header - size %d\n"), 
                    p->GetPacketIndex(), (int)m_desktop_queue.size());
            return true;
        }

        //ok, we can start new session
        DesktopWindow wnd(session_id, width, height, (RGBMode)bmp_mode,
                          DESKTOPPROTOCOL_ZLIB_1);
        DesktopCache* dcache;
        ACE_NEW_RETURN(dcache, DesktopCache(GetUserID(), wnd, 
                                            packet.GetTime()), 
                       false);

        m_desktop_cache = desktop_cache_t(dcache);
        TTASSERT(m_desktop_cache->GetBlocksCount());
        if(m_desktop_cache->GetBlocksCount() == 0)
        {
            CloseDesktopSession();
            return false;
        }
        if(!m_desktop_cache->AddDesktopPacket(packet))
            return false;

        //flush out desktop queue
        desktoppackets_t::iterator ii = m_desktop_queue.begin();
        while(ii != m_desktop_queue.end())
        {
            TTASSERT((*ii)->GetSessionID() == packet.GetSessionID());
            if((*ii)->GetSessionID() == packet.GetSessionID())
                m_desktop_cache->AddDesktopPacket(*(*ii));
            ii++;
        }
        m_desktop_queue.clear();
        return true;
    }
    else
        return m_desktop_cache->AddDesktopPacket(packet);
}

void ServerUser::CloseDesktopSession()
{
    m_desktop_cache.reset();
    m_desktop_queue.clear();
}

desktop_transmitter_t ServerUser::StartDesktopTransmitter(
                                         const ServerUser& src_user,
                                         const ServerChannel& channel,
                                         const DesktopCache& desktop)
{
    user_desktoptx_t::iterator ii = m_user_desktop_tx.find(src_user.GetUserID());
    TTASSERT(ii == m_user_desktop_tx.end());
    if(ii != m_user_desktop_tx.end())
        return desktop_transmitter_t();

    DesktopTransmitter* desktop_tx;
    ACE_NEW_RETURN(desktop_tx, DesktopTransmitter(desktop.GetSessionID(), 
                                                  desktop.GetCurrentDesktopTime()),
                   desktop_transmitter_t());

    desktop_transmitter_t dtx(desktop_tx);

    //place initial update in transmission queue
    desktoppackets_t packets;
    desktop.GetDesktopPackets(desktop.GetCurrentDesktopTime(), 
                              src_user.GetMaxDataChunkSize(),
                              src_user.GetMaxPayloadSize(), packets);
    TTASSERT(packets.size());
    if(packets.empty())
        return desktop_transmitter_t();

    desktoppackets_t::iterator dpi = packets.begin();
    for(;dpi != packets.end();dpi++)
    {
        (*dpi)->SetChannel(channel.GetChannelID());
        desktop_tx->AddDesktopPacketToQueue(*dpi);
    }

    m_user_desktop_tx[src_user.GetUserID()] = dtx;

    MYTRACE(ACE_TEXT("Started Desktop TX for #%d -> #%d update %d:%u\n"),
        src_user.GetUserID(), GetUserID(), dtx->GetSessionID(), dtx->GetUpdateID());

    return dtx;
}

desktop_transmitter_t ServerUser::ResumeDesktopTransmitter(
                                          const ServerUser& src_user,
                                          const ServerChannel& channel,
                                          const DesktopCache& desktop)
{
    user_desktoptx_t::iterator ii = m_user_desktop_tx.find(src_user.GetUserID());
    TTASSERT(ii != m_user_desktop_tx.end());
    if(ii == m_user_desktop_tx.end())
        return desktop_transmitter_t();

    desktop_transmitter_t& dtx = ii->second;
    TTASSERT(dtx->Done());
    if(!dtx->Done())
        return desktop_transmitter_t();

    uint32_t last_update_time = dtx->GetUpdateID();

    //place updated packets in transmission queue
    desktoppackets_t packets;
    if(!desktop.GetDesktopPackets(last_update_time, 
                                  src_user.GetMaxDataChunkSize(),
                                  src_user.GetMaxPayloadSize(), packets))
        return desktop_transmitter_t();
    
    DesktopTransmitter* desktop_tx;
    ACE_NEW_RETURN(desktop_tx, DesktopTransmitter(desktop.GetSessionID(),
                                                  desktop.GetCurrentDesktopTime()),
                   desktop_transmitter_t());
    dtx = desktop_transmitter_t(desktop_tx);

    desktoppackets_t::iterator dpi = packets.begin();
    for(;dpi != packets.end();dpi++)
    {
        (*dpi)->SetChannel(channel.GetChannelID());
        desktop_tx->AddDesktopPacketToQueue(*dpi);
    }

    m_user_desktop_tx[src_user.GetUserID()] = dtx;

    MYTRACE(ACE_TEXT("Resumed Desktop TX for #%d -> #%d update %d:%u\n"),
        src_user.GetUserID(), GetUserID(), dtx->GetSessionID(), dtx->GetUpdateID());

    return dtx;
}

desktop_transmitter_t ServerUser::GetDesktopTransmitter(int src_userid) const
{
    user_desktoptx_t::const_iterator ii = m_user_desktop_tx.find(src_userid);
    if(ii != m_user_desktop_tx.end())
        return ii->second;

    return desktop_transmitter_t();
}

void ServerUser::CloseDesktopTransmitter(int src_userid, bool store_closed_session)
{
    if(store_closed_session)
    {
        user_desktoptx_t::const_iterator ii = m_user_desktop_tx.find(src_userid);
        if(ii != m_user_desktop_tx.end())
        {
            MYTRACE(ACE_TEXT("Closed Desktop TX for #%d -> #%d\n"),
                    src_userid, GetUserID());

            ClosedDesktopSession session;
            session.session_id = ii->second->GetSessionID();
            session.update_id = ii->second->GetUpdateID();
            m_closed_desktops[src_userid] = session;
        }
    }

    m_user_desktop_tx.erase(src_userid);
}

bool ServerUser::GetClosedDesktopSession(int src_userid, 
                                         ClosedDesktopSession& session) const
{
    closed_desktops_t::const_iterator ii = m_closed_desktops.find(src_userid);
    if(ii != m_closed_desktops.end())
    {
        session = ii->second;
        return true;
    }
    return false;
 }

bool ServerUser::ClosePendingDesktopTerminate(int src_userid)
{
    if(m_closed_desktops.find(src_userid) != m_closed_desktops.end())
    {
        m_closed_desktops.erase(src_userid);
        return true;
    }
    return false;
}

void ServerUser::SendFile(ACE_Message_Queue_Base& msg_queue)
{
    ssize_t ret = 0;
    ACE_OFF_T bytes = 0;

    TTASSERT(m_filetransfer.get());
    if(!m_filetransfer.get())
        return;

    TTASSERT(m_filetransfer->file.get_handle() != ACE_INVALID_HANDLE);
    TTASSERT(m_filetransfer->readbuffer.size());
    TTASSERT(m_filetransfer->inbound == false);
    while(true)
    {
        bytes = m_filetransfer->file.recv(&m_filetransfer->readbuffer[0], 
                                          m_filetransfer->readbuffer.size());
        TTASSERT(ret>=0);

        if(bytes>0)
        {
            ACE_Time_Value tm = ACE_Time_Value::zero;
            ret = QueueStreamData(msg_queue, &m_filetransfer->readbuffer[0], (int)bytes, &tm);
            if(ret<0)
            {
                m_filetransfer->file.seek(m_filetransfer->file.tell() - bytes, SEEK_SET);    //rewind since we didn't send
                break;
            }
            else if(m_filetransfer->file.tell() >= m_filetransfer->filesize)
                break;
        }
        else
            break;
    }
}

void ServerUser::CloseTransfer()
{
    if(!m_filetransfer.get())
        return;

    if(m_filetransfer->file.get_handle() != ACE_INVALID_HANDLE)
    {
        if(m_filetransfer->inbound && m_filetransfer->file.tell() < m_filetransfer->filesize)
            m_filetransfer->file.remove();
    }
    m_filetransfer.reset();
}
