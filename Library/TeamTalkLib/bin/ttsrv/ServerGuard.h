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
        void OnUserConnected(const ServerUser& user);
        void OnUserLogin(const ServerUser& user);
        void OnUserAuthFailed(const ServerUser& user, const ACE_TString& username);
        void OnUserLoginBanned(const ServerUser& user);
        void OnUserLoggedOut(const ServerUser& user);
        void OnUserDisconnected(const ServerUser& user);
        void OnUserDropped(const ServerUser& user);
        void OnUserKicked(const ServerUser& kickee, const ServerUser* kicker, const ServerChannel* channel);
        void OnUserBanned(const ServerUser& banee, const ServerUser& banner);
        void OnUserBanned(const ACE_TString& ipaddr, const ServerUser& banner);
        void OnUserBanned(const ServerUser& banner, const BannedUser& ban);
        void OnUserUnbanned(const ServerUser& user, const BannedUser& ban);
        void OnUserUpdated(const ServerUser& user);
        void OnUserJoinChannel(const ServerUser& user, const ServerChannel& channel);
        void OnUserLeaveChannel(const ServerUser& user, const ServerChannel& channel);
        void OnUserMoved(const ServerUser& mover, const ServerUser& movee);
        void OnUserMessage(const ServerUser& from, const ServerUser& to, const teamtalk::TextMessage& msg);
        void OnChannelMessage(const ServerUser& from, const ServerChannel& channel, const teamtalk::TextMessage& msg);
        void OnBroadcastMessage(const ServerUser& from, const teamtalk::TextMessage& msg);
        void OnCustomMessage(const ServerUser& from, const ServerUser& to, const teamtalk::TextMessage& msg);

        void OnChannelCreated(const ServerChannel& channel, const ServerUser* user = NULL);
        void OnChannelUpdated(const ServerChannel& channel, const ServerUser* user = NULL);
        void OnChannelRemoved(const ServerChannel& channel, const ServerUser* user = NULL);

        void OnFileUploaded(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file);
        void OnFileDownloaded(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file);
        void OnFileDeleted(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file);

        void OnServerUpdated(const ServerUser& user, const ServerSettings& srvprop);
        void OnSaveConfiguration(ServerNode& servernode, const ServerUser* user);

        void OnShutdown(const ServerStats& stats);
        /* end logging functions */

        ErrorMsg AuthenticateUser(ServerNode* servernode, ServerUser& user, UserAccount& useraccount);
        ErrorMsg JoinChannel(const ServerUser& user, const ServerChannel& chan);

        ErrorMsg GetUserAccount(const ServerUser& user, UserAccount& useraccount);
        ErrorMsg GetRegUsers(const ServerUser& user, useraccounts_t& users);
        ErrorMsg AddRegUser(const ServerUser& user, const UserAccount& useraccount);
        ErrorMsg DeleteRegUser(const ServerUser& user, const ACE_TString& username);

        ErrorMsg AddUserBan(const ServerUser& user, const ServerUser& banee, BanTypes bantype);
        ErrorMsg AddUserBan(const ServerUser& user, const BannedUser& ban);
        ErrorMsg RemoveUserBan(const ServerUser& user, const BannedUser& ban);
        ErrorMsg GetUserBans(const ServerUser& user, std::vector<BannedUser>& bans);

        ErrorMsg ChangeNickname(const ServerUser& user, const ACE_TString& newnick);
        ErrorMsg ChangeStatus(const ServerUser& user, int mode, const ACE_TString& status);

    private:
#if defined(ENABLE_HTTP_AUTH)
        void WebLoginFacebook(ServerNode* servernode, ACE_UINT32 userid, UserAccount useraccount);
        void WebLoginBearWare(ServerNode* servernode, ACE_UINT32 userid, UserAccount useraccount);
        ErrorMsg WebLoginPostAuthenticate(UserAccount& useraccount);
        void WebLoginComplete(ServerNode* servernode, ACE_UINT32 userid, const UserAccount& useraccount, const ErrorMsg& err);
        std::map<int, UserAccount> m_pendinglogin;
#endif
        teamtalk::ServerXML& m_settings;
    };

    bool LoadConfig(ServerXML& xmlSettings, const ACE_TString& cfgfile);

    bool ReadServerProperties(ServerXML& xmlSettings, ServerSettings& properties,
                              statchannels_t& channels);

#if defined(ENABLE_TEAMTALKPRO)
    bool SetupEncryption(ServerNode& servernode, ServerXML& xmlSettings);
#endif

    bool ConfigureServer(ServerNode& servernode, const ServerSettings& properties,
                         const statchannels_t& channels);

    void ConvertChannels(const serverchannel_t& root, teamtalk::statchannels_t& channels, bool onlystatic = false);

    void MakeStaticChannels(ServerNode& servernode, const statchannels_t& channels);

    void RotateLogfile(const ACE_TString& cwd, const ACE_TString& logname, std::ofstream& logfile);
}
#endif
