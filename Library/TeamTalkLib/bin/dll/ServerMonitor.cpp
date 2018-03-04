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

#include "ServerMonitor.h"
#include "Convert.h"

using teamtalk::ErrorMsg;

ServerMonitor::ServerMonitor()
: m_ttInst(NULL)
{
}

ServerMonitor::~ServerMonitor()
{
}

void ServerMonitor::OnUserConnected(const teamtalk::ServerUser& user)
{
    User suser;
    Convert(user, suser);

    userconnected_t::iterator i=m_userconnected.begin();
    for(;i!=m_userconnected.end();i++)
        i->second(m_ttInst, i->first, &suser);
}

void ServerMonitor::OnUserLogin(const teamtalk::ServerUser& user)
{
    User suser;
    Convert(user, suser);

    userloggedin_t::iterator i=m_userloggedin.begin();
    for(;i!=m_userloggedin.end();i++)
        i->second(m_ttInst, i->first, &suser);
}

void ServerMonitor::OnUserAuthFailed(const teamtalk::ServerUser& user,
                                     const ACE_TString& username)
{
}

void ServerMonitor::OnUserLoginBanned(const teamtalk::ServerUser& user)
{
}

void ServerMonitor::OnUserLoggedOut(const teamtalk::ServerUser& user)
{
    User suser;
    Convert(user, suser);

    userloggedout_t::iterator i=m_userloggedout.begin();
    for(;i!=m_userloggedout.end();i++)
        i->second(m_ttInst, i->first, &suser);
}

void ServerMonitor::OnUserDisconnected(const teamtalk::ServerUser& user)
{
    User suser;
    Convert(user, suser);

    userdisconnected_t::iterator i=m_userdisconnected.begin();
    for(;i!=m_userdisconnected.end();i++)
        i->second(m_ttInst, i->first, &suser);
}

void ServerMonitor::OnUserDropped(const teamtalk::ServerUser& user)
{
    User suser;
    Convert(user, suser);

    usertimedout_t::iterator i=m_usertimedout.begin();
    for(;i!=m_usertimedout.end();i++)
        i->second(m_ttInst, i->first, &suser);
}

void ServerMonitor::OnUserKicked(const teamtalk::ServerUser& kickee,
                                 const teamtalk::ServerUser* kicker,
                                 const teamtalk::ServerChannel* channel)
{
    User skicker, skickee;
    if(kicker)
        Convert(*kicker, skicker);
    Convert(kickee, skickee);
    Channel schan;
    if(channel)
        Convert(channel->GetChannelProp(), schan);

    userkicked_t::iterator i=m_userkicked.begin();
    for(;i!=m_userkicked.end();i++)
        i->second(m_ttInst, i->first, kicker?&skicker:NULL, &skickee, NULL);
}

void ServerMonitor::OnUserBanned(const teamtalk::ServerUser& banee,
                                 const teamtalk::ServerUser& banner)
{
    User sbanner, sbanee;
    Convert(banner, sbanner);
    Convert(banee, sbanee);

    userbanned_t::iterator i=m_userbanned.begin();
    for(;i!=m_userbanned.end();i++)
        i->second(m_ttInst, i->first, &sbanner, &sbanee, NULL);
}

void ServerMonitor::OnUserBanned(const ACE_TString& ipaddr,
                                 const teamtalk::ServerUser& banner)
{
    User sbanner, sbanee = {0};
    Convert(banner, sbanner);
    ACE_OS::strsncpy(sbanee.szIPAddress, ipaddr.c_str(), TT_STRLEN);

    userbanned_t::iterator i=m_userbanned.begin();
    for(;i!=m_userbanned.end();i++)
        i->second(m_ttInst, i->first, &sbanner, &sbanee, NULL);
}

void ServerMonitor::OnUserUnbanned(const teamtalk::ServerUser& user,
                                   const teamtalk::BannedUser& ban)
{
    User suser;
    Convert(user, suser);

    userunbanned_t::iterator i=m_userunbanned.begin();
    for(;i!=m_userunbanned.end();i++)
        i->second(m_ttInst, i->first, &suser, ban.ipaddr.c_str());
}

void ServerMonitor::OnUserUpdated(const teamtalk::ServerUser& user)
{
    User suser;
    Convert(user, suser);

    userupdate_t::iterator i=m_userupdate.begin();
    for(;i!=m_userupdate.end();i++)
        i->second(m_ttInst, i->first, &suser);
}

void ServerMonitor::OnUserJoinChannel(const teamtalk::ServerUser& user,
                                      const teamtalk::ServerChannel& channel)
{
    User suser;
    Convert(user, suser);
    Channel schan;
    Convert(channel.GetChannelProp(), schan);

    userleftchannel_t::iterator i=m_userjoined.begin();
    for(;i!=m_userjoined.end();i++)
        i->second(m_ttInst, i->first, &suser, &schan);
}

void ServerMonitor::OnUserLeaveChannel(const teamtalk::ServerUser& user,
                                       const teamtalk::ServerChannel& channel)
{
    User suser;
    Convert(user, suser);
    Channel schan;
    Convert(channel.GetChannelProp(), schan);

    userleftchannel_t::iterator i=m_userleft.begin();
    for(;i!=m_userleft.end();i++)
        i->second(m_ttInst, i->first, &suser, &schan);
}

void ServerMonitor::OnUserMoved(const teamtalk::ServerUser& mover,
                                const teamtalk::ServerUser& movee)
{
    User smover, smovee;
    Convert(mover, smover);
    Convert(movee, smovee);

    usermoved_t::iterator i=m_usermoved.begin();
    for(;i!=m_usermoved.end();i++)
        i->second(m_ttInst, i->first, &smover, &smovee);
}

void ServerMonitor::OnUserMessage(const teamtalk::ServerUser& from,
                                  const teamtalk::ServerUser& to,
                                  const teamtalk::TextMessage& msg)
{
    User sfrom;
    Convert(from, sfrom);
    TextMessage smsg;
    Convert(msg, smsg);

    usertextmsg_t::iterator i=m_usertextmsg.begin();
    for(;i!=m_usertextmsg.end();i++)
        i->second(m_ttInst, i->first, &sfrom, &smsg);
}

void ServerMonitor::OnChannelMessage(const teamtalk::ServerUser& from,
                                     const teamtalk::ServerChannel& channel,
                                     const teamtalk::TextMessage& msg)
{
    User sfrom;
    Convert(from, sfrom);
    TextMessage smsg;
    Convert(msg, smsg);

    usertextmsg_t::iterator i=m_usertextmsg.begin();
    for(;i!=m_usertextmsg.end();i++)
        i->second(m_ttInst, i->first, &sfrom, &smsg);
}

void ServerMonitor::OnBroadcastMessage(const teamtalk::ServerUser& from,
                                       const teamtalk::TextMessage& msg)
{
    User sfrom;
    Convert(from, sfrom);
    TextMessage smsg;
    Convert(msg, smsg);

    usertextmsg_t::iterator i=m_usertextmsg.begin();
    for(;i!=m_usertextmsg.end();i++)
        i->second(m_ttInst, i->first, &sfrom, &smsg);
}

void ServerMonitor::OnCustomMessage(const teamtalk::ServerUser& from,
                                    const teamtalk::ServerUser& to,
                                    const teamtalk::TextMessage& msg)
{
    User sfrom;
    Convert(from, sfrom);
    TextMessage smsg;
    Convert(msg, smsg);

    usertextmsg_t::iterator i=m_usertextmsg.begin();
    for(;i!=m_usertextmsg.end();i++)
        i->second(m_ttInst, i->first, &sfrom, &smsg);
}

void ServerMonitor::OnChannelCreated(const teamtalk::ServerChannel& channel,
                                     const teamtalk::ServerUser* user /*= NULL*/)
{
    User suser;
    if(user)
        Convert(*user, suser);
    Channel schan;
    Convert(channel.GetChannelProp(), schan);

    chancreated_t::iterator i=m_chancreated.begin();
    for(;i!=m_chancreated.end();i++)
        i->second(m_ttInst, i->first, &schan, user?&suser:NULL);
}

void ServerMonitor::OnChannelUpdated(const teamtalk::ServerChannel& channel,
                                     const teamtalk::ServerUser* user /*= NULL*/)
{
    User suser;
    if(user)
        Convert(*user, suser);
    Channel schan;
    Convert(channel.GetChannelProp(), schan);

    chanupdated_t::iterator i=m_chanupdated.begin();
    for(;i!=m_chanupdated.end();i++)
        i->second(m_ttInst, i->first, &schan, user?&suser:NULL);
}

void ServerMonitor::OnChannelRemoved(const teamtalk::ServerChannel& channel,
                                     const teamtalk::ServerUser* user /*= NULL*/)
{
    User suser;
    if(user)
        Convert(*user, suser);
    Channel schan;
    Convert(channel.GetChannelProp(), schan);

    chanremoved_t::iterator i=m_chanremoved.begin();
    for(;i!=m_chanremoved.end();i++)
        i->second(m_ttInst, i->first, &schan, user?&suser:NULL);
}

void ServerMonitor::OnFileUploaded(const teamtalk::ServerUser& user,
                                   const teamtalk::ServerChannel& chan,
                                   const teamtalk::RemoteFile& file)
{
    User suser;
    Convert(user, suser);
    RemoteFile sfile;
    Convert(file, sfile);

    fileupload_t::iterator i=m_fileupload.begin();
    for(;i!=m_fileupload.end();i++)
        i->second(m_ttInst, i->first, &sfile, &suser);
}

void ServerMonitor::OnFileDownloaded(const teamtalk::ServerUser& user,
                                     const teamtalk::ServerChannel& chan,
                                     const teamtalk::RemoteFile& file)
{
    User suser;
    Convert(user, suser);
    RemoteFile sfile;
    Convert(file, sfile);

    filedownload_t::iterator i=m_filedownload.begin();
    for(;i!=m_filedownload.end();i++)
        i->second(m_ttInst, i->first, &sfile, &suser);
}

void ServerMonitor::OnFileDeleted(const teamtalk::ServerUser& user,
                                  const teamtalk::ServerChannel& chan,
                                  const teamtalk::RemoteFile& file)
{
    User suser;
    Convert(user, suser);
    RemoteFile sfile;
    Convert(file, sfile);

    filedelete_t::iterator i=m_filedelete.begin();
    for(;i!=m_filedelete.end();i++)
        i->second(m_ttInst, i->first, &sfile, &suser);
}

void ServerMonitor::OnServerUpdated(const teamtalk::ServerUser& user,
                                    const teamtalk::ServerProperties& srvprop)
{
    User suser;
    ServerProperties sprop;
    Convert(user, suser);
    Convert(srvprop, sprop);
    
    serverupdated_t::iterator i=m_serverupdated.begin();
    for(;i!=m_serverupdated.end();i++)
        i->second(m_ttInst, i->first, &sprop, &suser);
}

void ServerMonitor::OnSaveConfiguration(teamtalk::ServerNode& servernode,
                                        const teamtalk::ServerUser* user /*= NULL*/)
{
    User suser;
    if(user)
        Convert(*user, suser);

    saveservercfg_t::iterator i=m_saveservercfg.begin();
    for(;i!=m_saveservercfg.end();i++)
    {
        if(user)
            return i->second(m_ttInst, i->first, &suser);
        else
            return i->second(m_ttInst, i->first, NULL);
    }
}

void ServerMonitor::OnShutdown(const teamtalk::ServerStats& stats)
{
}

teamtalk::ErrorMsg ServerMonitor::AuthenticateUser(teamtalk::ServerNode* servernode,
                                                   teamtalk::ServerUser& user,
                                                   teamtalk::UserAccount& useraccount)
{
    ClientErrorMsg err;
    ZERO_STRUCT(err);
    User suser;
    UserAccount auser;
    Convert(user, suser);
    Convert(useraccount, auser);
    login_t::iterator i=m_login_callbacks.begin();
    for(;i!=m_login_callbacks.end();i++)
    {
        i->second(m_ttInst, i->first, &err, &suser, &auser);
        Convert(auser, useraccount);
        break;
    }

    ErrorMsg ret;
    return Convert(err, ret);
}

teamtalk::ErrorMsg ServerMonitor::JoinChannel(const teamtalk::ServerUser& user, const teamtalk::ServerChannel& chan)
{
    return teamtalk::TT_CMDERR_SUCCESS;
}

teamtalk::ErrorMsg ServerMonitor::GetUserAccount(const teamtalk::ServerUser& user,
                                                 teamtalk::UserAccount& useraccount)
{

    return ErrorMsg(teamtalk::TT_CMDERR_SUCCESS);
}

teamtalk::ErrorMsg ServerMonitor::GetRegUsers(const teamtalk::ServerUser& user,
                                              teamtalk::useraccounts_t& users)
{
    return ErrorMsg(teamtalk::TT_CMDERR_SUCCESS);
}

teamtalk::ErrorMsg ServerMonitor::AddRegUser(const teamtalk::ServerUser& user,
                                             const teamtalk::UserAccount& useraccount)
{
    ClientErrorMsg err;
    ZERO_STRUCT(err);

    User suser;
    UserAccount auser;
    Convert(user, suser);
    Convert(useraccount, auser);
    createuseraccount_t::iterator i=m_createuseraccount_callback.begin();
    for(;i!=m_createuseraccount_callback.end();i++)
        i->second(m_ttInst, i->first, &err, &suser, &auser);

    ErrorMsg ret;
    return Convert(err, ret);
}

teamtalk::ErrorMsg ServerMonitor::DeleteRegUser(const teamtalk::ServerUser& user,
                                                const ACE_TString& username)
{
    ClientErrorMsg err;
    ZERO_STRUCT(err);

    User suser;
    Convert(user, suser);
    deleteuseraccount_t::iterator i=m_deleteuseraccount_callback.begin();
    for(;i!=m_deleteuseraccount_callback.end();i++)
        i->second(m_ttInst, i->first, &err, &suser, username.c_str());

    ErrorMsg ret;
    return Convert(err, ret);
}

teamtalk::ErrorMsg ServerMonitor::AddUserBan(const teamtalk::ServerUser& banner, 
                                               const teamtalk::ServerUser& banee,
                                               teamtalk::BanTypes bantype)
{
    ClientErrorMsg err;
    ZERO_STRUCT(err);

    User sbanner, sbanee;
    Convert(banner, sbanner);
    Convert(banee, sbanee);
    addserverban_t::iterator i=m_addserverban_callback.begin();
    for(;i!=m_addserverban_callback.end();i++)
        i->second(m_ttInst, i->first, &err, &sbanner, &sbanee);

    ErrorMsg ret;
    return Convert(err, ret);
}

teamtalk::ErrorMsg ServerMonitor::AddUserBan(const teamtalk::ServerUser& banner, 
                                               const teamtalk::BannedUser& ban)
{
    ClientErrorMsg err;
    ZERO_STRUCT(err);

    User sbanner;
    Convert(banner, sbanner);
    addserverbanip_t::iterator i=m_addserverbanip_callback.begin();
    for(;i!=m_addserverbanip_callback.end();i++)
        i->second(m_ttInst, i->first, &err, &sbanner, ban.ipaddr.c_str());

    ErrorMsg ret;
    return Convert(err, ret);
}

teamtalk::ErrorMsg ServerMonitor::RemoveUserBan(const teamtalk::ServerUser& user, 
                                                  const teamtalk::BannedUser& ban)
{
    ClientErrorMsg err;
    ZERO_STRUCT(err);

    User unbanner;
    Convert(user, unbanner);
    deleteserverban_t::iterator i=m_deleteserverban_callback.begin();
    for(;i!=m_deleteserverban_callback.end();i++)
        i->second(m_ttInst, i->first, &err, &unbanner, ban.ipaddr.c_str());

    ErrorMsg ret;
    return Convert(err, ret);
}

teamtalk::ErrorMsg ServerMonitor::GetUserBans(const teamtalk::ServerUser& user, 
                                                std::vector<teamtalk::BannedUser>& bans)
{
    return ErrorMsg(teamtalk::TT_CMDERR_SUCCESS);
}

teamtalk::ErrorMsg ServerMonitor::ChangeNickname(const teamtalk::ServerUser& user, 
                                                 const ACE_TString& newnick)
{
    ClientErrorMsg err;
    ZERO_STRUCT(err);

    User changer;
    Convert(user, changer);

    for(auto i=m_changenickname_callback.begin();i!=m_changenickname_callback.end();++i)
        i->second(m_ttInst, i->first, &err, &changer, newnick.c_str());

    ErrorMsg ret;
    return Convert(err, ret);
}

teamtalk::ErrorMsg ServerMonitor::ChangeStatus(const teamtalk::ServerUser& user, 
                                               int mode, const ACE_TString& status)
{
    ClientErrorMsg err;
    ZERO_STRUCT(err);

    User changer;
    Convert(user, changer);

    for(auto i=m_changestatus_callback.begin();i!=m_changestatus_callback.end();++i)
        i->second(m_ttInst, i->first, &err, &changer, mode, status.c_str());

    ErrorMsg ret;
    return Convert(err, ret);
}
