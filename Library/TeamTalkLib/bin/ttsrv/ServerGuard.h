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

#if !defined(SERVERGUARD_H)
#define SERVERGUARD_H

#include <teamtalk/server/ServerNode.h>
#include "ServerXML.h"

#if defined(ENABLE_HTTP_AUTH)
#include <thread>
#endif

#include <mutex>

namespace teamtalk {

    class ServerGuard : public ServerNodeListener
    {
    public:
        ServerGuard(ServerXML& settings);

        /* begin logging functions */
        void OnUserConnected(const ServerUser& user) override;
        void OnUserLogin(const ServerUser& user) override;
        void OnUserAuthFailed(const ServerUser& user, const ACE_TString& username) override;
        void OnUserLoginBanned(const ServerUser& user) override;
        void OnUserLoggedOut(const ServerUser& user) override;
        void OnUserDisconnected(const ServerUser& user) override;
        void OnUserCryptError(const ServerUser& user, int sslerr, const ACE_TString& msg) override;
        void OnUserDropped(const ServerUser& user) override;
        void OnUserKicked(const ServerUser& kickee, const ServerUser* kicker, const ServerChannel* channel) override;
        void OnUserBanned(const ServerUser& banee, const ServerUser& banner) override;
        void OnUserBanned(const ACE_TString& ipaddr, const ServerUser& banner) override;
        void OnUserBanned(const ServerUser& banner, const BannedUser& ban) override;
        void OnUserUnbanned(const ServerUser& user, const BannedUser& ban) override;
        void OnUserUpdated(const ServerUser& user) override;
        void OnUserSubscribe(const ServerUser& user, const ServerUser& subscriptuser) override;
        void OnUserJoinChannel(const ServerUser& user, const ServerChannel& channel) override;
        void OnUserLeaveChannel(const ServerUser& user, const ServerChannel& channel) override;
        void OnUserMoved(const ServerUser& mover, const ServerUser& movee) override;
        void OnUserMessage(const ServerUser& from, const ServerUser& to, const teamtalk::TextMessage& msg) override;
        void OnChannelMessage(const ServerUser& from, const ServerChannel& channel, const teamtalk::TextMessage& msg) override;
        void OnBroadcastMessage(const ServerUser& from, const teamtalk::TextMessage& msg) override;
        void OnCustomMessage(const ServerUser& from, const ServerUser& to, const teamtalk::TextMessage& msg) override;

        void OnUserUpdateStream(const ServerUser& user, const ServerChannel& channel, StreamType stream, int streamid) override;

        void OnChannelCreated(const ServerChannel& channel, const ServerUser* user = NULL) override;
        void OnChannelUpdated(const ServerChannel& channel, const ServerUser* user = NULL) override;
        void OnChannelRemoved(const ServerChannel& channel, const ServerUser* user = NULL) override;

        void OnFileUploaded(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file) override;
        void OnFileDownloaded(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file) override;
        void OnFileDeleted(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file) override;

        void OnServerUpdated(const ServerUser* user, const ServerSettings& srvprop) override;
        void OnSaveConfiguration(const ServerUser* user) override;

        void OnShutdown(const ServerStats& stats) override;
        /* end logging functions */

        ErrorMsg AuthenticateUser(ServerNode* servernode, ServerUser& user, UserAccount& useraccount) override;
        ErrorMsg JoinChannel(const ServerUser& user, const ServerChannel& chan) override;
        ErrorMsg RemoveChannel(const ServerChannel& chan, const ServerUser* user = nullptr) override;

        ErrorMsg GetUserAccount(const ServerUser& user, UserAccount& useraccount) override;
        ErrorMsg GetRegUsers(const ServerUser& user, useraccounts_t& users) override;
        ErrorMsg AddRegUser(const ServerUser& user, const UserAccount& useraccount) override;
        ErrorMsg DeleteRegUser(const ServerUser& user, const ACE_TString& username) override;

        ErrorMsg AddUserBan(const ServerUser& user, const ServerUser& banee, BanTypes bantype) override;
        ErrorMsg AddUserBan(const ServerUser& user, const BannedUser& ban) override;
        ErrorMsg RemoveUserBan(const ServerUser& user, const BannedUser& ban) override;
        ErrorMsg GetUserBans(const ServerUser& user, std::vector<BannedUser>& bans) override;

        ErrorMsg ChangeNickname(const ServerUser& user, const ACE_TString& newnick) override;
        ErrorMsg ChangeStatus(const ServerUser& user, int mode, const ACE_TString& status) override;

        ErrorMsg SaveConfiguration(const ServerUser& user, ServerNode& servernode) override;

    private:
#if defined(ENABLE_TEAMTALKPRO)
        void WebLoginBearWare(ServerNode* servernode, ACE_UINT32 userid, UserAccount useraccount);
        ErrorMsg WebLoginPostAuthenticate(UserAccount& useraccount);
        void WebLoginComplete(ServerNode* servernode, ACE_UINT32 userid, const UserAccount& useraccount, const ErrorMsg& err);
        std::map<int, UserAccount> m_pendinglogin;
#endif
        teamtalk::ServerXML& m_settings;
    };
}
#endif
