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

#if !defined(SERVERMONITOR_H)
#define SERVERMONITOR_H

#include <teamtalk/server/ServerNode.h>

#include "TeamTalkSrv.h"

class ServerMonitor : public teamtalk::ServerNodeListener
{
public:
    ServerMonitor();
    virtual ~ServerMonitor();

    void OnUserConnected(const teamtalk::ServerUser& user);
    void OnUserLogin(const teamtalk::ServerUser& user);
    void OnUserAuthFailed(const teamtalk::ServerUser& user,
                          const ACE_TString& username);
    void OnUserLoginBanned(const teamtalk::ServerUser& user);
    void OnUserLoggedOut(const teamtalk::ServerUser& user);
    void OnUserDisconnected(const teamtalk::ServerUser& user);
    void OnUserDropped(const teamtalk::ServerUser& user);
    void OnUserKicked(const teamtalk::ServerUser& kickee,
                      const teamtalk::ServerUser* kicker,
                      const teamtalk::ServerChannel* channel);
    void OnUserBanned(const teamtalk::ServerUser& banee,
                      const teamtalk::ServerUser& banner);
    void OnUserBanned(const ACE_TString& ipaddr,
                      const teamtalk::ServerUser& banner);
    void OnUserBanned(const teamtalk::ServerUser& banner, const teamtalk::BannedUser& ban);
    void OnUserUnbanned(const teamtalk::ServerUser& user,
                        const teamtalk::BannedUser& ban);
    void OnUserUpdated(const teamtalk::ServerUser& user);
    void OnUserJoinChannel(const teamtalk::ServerUser& user,
                           const teamtalk::ServerChannel& channel);
    void OnUserLeaveChannel(const teamtalk::ServerUser& user,
                            const teamtalk::ServerChannel& channel);
    void OnUserMoved(const teamtalk::ServerUser& mover,
                     const teamtalk::ServerUser& movee);
    void OnUserMessage(const teamtalk::ServerUser& from,
                       const teamtalk::ServerUser& to,
                       const teamtalk::TextMessage& msg);
    void OnChannelMessage(const teamtalk::ServerUser& from,
                          const teamtalk::ServerChannel& channel,
                          const teamtalk::TextMessage& msg);
    void OnBroadcastMessage(const teamtalk::ServerUser& from,
                            const teamtalk::TextMessage& msg);
    void OnCustomMessage(const teamtalk::ServerUser& from,
                         const teamtalk::ServerUser& to,
                         const teamtalk::TextMessage& msg);

    void OnChannelCreated(const teamtalk::ServerChannel& channel,
                          const teamtalk::ServerUser* user = NULL);
    void OnChannelUpdated(const teamtalk::ServerChannel& channel,
                          const teamtalk::ServerUser* user = NULL);
    void OnChannelRemoved(const teamtalk::ServerChannel& channel,
                          const teamtalk::ServerUser* user = NULL);

    void OnFileUploaded(const teamtalk::ServerUser& user,
                        const teamtalk::ServerChannel& chan,
                        const teamtalk::RemoteFile& file);
    void OnFileDownloaded(const teamtalk::ServerUser& user,
                          const teamtalk::ServerChannel& chan,
                          const teamtalk::RemoteFile& file);
    void OnFileDeleted(const teamtalk::ServerUser& user,
                       const teamtalk::ServerChannel& chan,
                       const teamtalk::RemoteFile& file);

    void OnServerUpdated(const teamtalk::ServerUser& user,
                         const teamtalk::ServerSettings& srvprop);
    void OnSaveConfiguration(teamtalk::ServerNode& servernode,
                             const teamtalk::ServerUser* user = NULL);

    void OnShutdown(const teamtalk::ServerStats& stats);

    typedef std::map<VOID*, UserConnectedCallback*> userconnected_t;
    userconnected_t m_userconnected;

    typedef std::map<VOID*, UserLoggedInCallback*> userloggedin_t;
    userloggedin_t m_userloggedin;

    typedef std::map<VOID*, UserLoggedOutCallback*> userloggedout_t;
    userloggedout_t m_userloggedout;

    typedef std::map<VOID*, UserDisconnectedCallback*> userdisconnected_t;
    userdisconnected_t m_userdisconnected;

    typedef std::map<VOID*, UserTimedoutCallback*> usertimedout_t;
    usertimedout_t m_usertimedout;

    typedef std::map<VOID*, UserKickedCallback*> userkicked_t;
    userkicked_t m_userkicked;

    typedef std::map<VOID*, UserBannedCallback*> userbanned_t;
    userbanned_t m_userbanned;

    typedef std::map<VOID*, UserUnbannedCallback*> userunbanned_t;
    userunbanned_t m_userunbanned;

    typedef std::map<VOID*, UserUpdatedCallback*> userupdate_t;
    userupdate_t m_userupdate;

    typedef std::map<VOID*, UserJoinedChannelCallback*> userjoinedchannel_t;
    userjoinedchannel_t m_userjoined;

    typedef std::map<VOID*, UserLeftChannelCallback*> userleftchannel_t;
    userleftchannel_t m_userleft;

    typedef std::map<VOID*, UserMovedCallback*> usermoved_t;
    usermoved_t m_usermoved;

    typedef std::map<VOID*, UserTextMessageCallback*> usertextmsg_t;
    usertextmsg_t m_usertextmsg;

    typedef std::map<VOID*, ChannelCreatedCallback*> chancreated_t;
    chancreated_t m_chancreated;

    typedef std::map<VOID*, ChannelUpdatedCallback*> chanupdated_t;
    chanupdated_t m_chanupdated;

    typedef std::map<VOID*, ChannelRemovedCallback*> chanremoved_t;
    chanremoved_t m_chanremoved;

    typedef std::map<VOID*, FileUploadedCallback*> fileupload_t;
    fileupload_t m_fileupload;

    typedef std::map<VOID*, FileDownloadedCallback*> filedownload_t;
    filedownload_t m_filedownload;

    typedef std::map<VOID*, FileDeletedCallback*> filedelete_t;
    filedelete_t m_filedelete;

    typedef std::map<VOID*, ServerUpdatedCallback*> serverupdated_t;
    serverupdated_t m_serverupdated;

    typedef std::map<VOID*, SaveServerConfigCallback*> saveservercfg_t;
    saveservercfg_t m_saveservercfg;

    teamtalk::ErrorMsg AuthenticateUser(teamtalk::ServerNode* servernode, 
                                        teamtalk::ServerUser& user,
                                        teamtalk::UserAccount& useraccount); //user-type is set, therefore not const
    teamtalk::ErrorMsg JoinChannel(const teamtalk::ServerUser& user, const teamtalk::ServerChannel& chan);
    teamtalk::ErrorMsg GetUserAccount(const teamtalk::ServerUser& user,
                                      teamtalk::UserAccount& useraccount);
    teamtalk::ErrorMsg GetRegUsers(const teamtalk::ServerUser& user,
                                   teamtalk::useraccounts_t& users);
    teamtalk::ErrorMsg AddRegUser(const teamtalk::ServerUser& user,
                                  const teamtalk::UserAccount& useraccount);
    teamtalk::ErrorMsg DeleteRegUser(const teamtalk::ServerUser& user,
                                     const ACE_TString& username);

    teamtalk::ErrorMsg AddUserBan(const teamtalk::ServerUser& banner, const teamtalk::ServerUser& banee, teamtalk::BanTypes bantype);
    teamtalk::ErrorMsg AddUserBan(const teamtalk::ServerUser& banner, const teamtalk::BannedUser& ban);
    teamtalk::ErrorMsg RemoveUserBan(const teamtalk::ServerUser& user, const teamtalk::BannedUser& ban);
    teamtalk::ErrorMsg GetUserBans(const teamtalk::ServerUser& user, std::vector<teamtalk::BannedUser>& bans);

    teamtalk::ErrorMsg ChangeNickname(const teamtalk::ServerUser& user, const ACE_TString& newnick);
    teamtalk::ErrorMsg ChangeStatus(const teamtalk::ServerUser& user, int mode, const ACE_TString& status);

    TTSInstance* m_ttInst;

    typedef std::map<VOID*, UserLoginCallback*> login_t;
    login_t m_login_callbacks;

    typedef std::map<VOID*, UserCreateUserAccountCallback*> createuseraccount_t;
    createuseraccount_t m_createuseraccount_callback;

    typedef std::map<VOID*, UserDeleteUserAccountCallback*> deleteuseraccount_t;
    deleteuseraccount_t m_deleteuseraccount_callback;

    typedef std::map<VOID*, UserAddServerBanCallback*> addserverban_t;
    addserverban_t m_addserverban_callback;

    typedef std::map<VOID*, UserAddServerBanIPAddressCallback*> addserverbanip_t;
    addserverbanip_t m_addserverbanip_callback;

    typedef std::map<VOID*, UserDeleteServerBanCallback*> deleteserverban_t;
    deleteserverban_t m_deleteserverban_callback;

    typedef std::map<VOID*, UserChangeNicknameCallback*> changenickname_t;
    changenickname_t m_changenickname_callback;

    typedef std::map<VOID*, UserChangeStatusCallback*> changestatus_t;
    changestatus_t m_changestatus_callback;

};

#endif /* SERVERMONITOR_H */
