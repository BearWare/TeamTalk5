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

#if !defined(CLIENTNODEEVENT_H)
#define CLIENTNODEEVENT_H

#include "Client.h"
#include "ClientNodeBase.h"
#include "ClientChannel.h"
#include "ClientUser.h"
#include "VoiceLogger.h"

#include <avstream/MediaStreamer.h>
#include <avstream/AudioInputStreamer.h>

namespace teamtalk {

    class ClientListener : public VoiceLogListener //VoiceLogger
    {
    public:
        virtual ~ClientListener() {}

        virtual void RegisterEventSuspender(EventSuspender* suspender) = 0;

        virtual void OnConnectSuccess() = 0;
        virtual void OnConnectFailed() = 0;
        virtual void OnConnectionLost() = 0;

        virtual void OnAccepted(int myuserid, const teamtalk::UserAccount& account) = 0;
        virtual void OnLoggedOut() = 0;

        virtual void OnUserLoggedIn(const teamtalk::ClientUser& user) = 0;
        virtual void OnUserLoggedOut(const teamtalk::ClientUser& user) = 0;
        virtual void OnUserUpdate(const teamtalk::ClientUser& user) = 0;

        virtual void OnUserJoinChannel(const teamtalk::ClientUser& user,
                                       const teamtalk::ClientChannel& chan) = 0;
        virtual void OnUserLeftChannel(const teamtalk::ClientUser& user,
                                       const teamtalk::ClientChannel& chan) = 0;

        virtual void OnAddChannel(const ClientChannel& chan) = 0;
        virtual void OnUpdateChannel(const teamtalk::ClientChannel& chan) = 0;
        virtual void OnRemoveChannel(const teamtalk::ClientChannel& chan) = 0;

        virtual void OnJoinedChannel(int channelid) = 0;
        virtual void OnLeftChannel(int channelid) = 0;

        virtual void OnAddFile(const teamtalk::ClientChannel& chan,
                               const teamtalk::RemoteFile& file) = 0;
        virtual void OnRemoveFile(const teamtalk::ClientChannel& chan,
                                  const teamtalk::RemoteFile& file) = 0;

        virtual void OnUserAccount(const teamtalk::UserAccount& account) = 0;
        virtual void OnBannedUser(const teamtalk::BannedUser& banuser) = 0;

        virtual void OnTextMessage(const teamtalk::TextMessage& textmsg) = 0;

        virtual void OnKicked(const teamtalk::clientuser_t& user, int channelid) = 0;
        virtual void OnServerUpdate(const ServerInfo& serverinfo) = 0;
        virtual void OnServerStatistics(const ServerStats& serverstats) = 0;

        virtual void OnFileTransferStatus(const teamtalk::FileTransfer& transfer) = 0;

        virtual void OnCommandError(int cmdid, int err_num, const ACE_TString& msg) = 0;
        virtual void OnCommandSuccess(int cmdid) = 0;
        virtual void OnCommandProcessing(int cmdid, bool begin_end) = 0;

        virtual void OnInternalError(int err_num, const ACE_TString& msg) = 0;

        virtual void OnVoiceActivated(bool enabled) = 0;

        virtual void OnUserFirstStreamVoicePacket(const teamtalk::ClientUser& user, int streamid) = 0;
        virtual void OnUserStateChange(const teamtalk::ClientUser& user) = 0;
        virtual void OnUserVideoCaptureFrame(int userid, int stream_id) = 0;
        virtual void OnUserMediaFileVideoFrame(int userid, int stream_id) = 0;

        virtual void OnDesktopTransferUpdate(int session_id, int remain_bytes) = 0;

        virtual void OnUserDesktopWindow(int userid, int session_id) = 0;
        virtual void OnUserDesktopCursor(int src_userid, const teamtalk::DesktopInput& input) = 0;
        virtual void OnUserDesktopInput(int src_userid, const teamtalk::DesktopInput& input) = 0;

        virtual void OnChannelStreamMediaFile(const MediaFileProp& mfp,
                                              MediaFileStatus status) = 0;

        virtual void OnLocalMediaFilePlayback(int sessionid, const MediaFileProp& mfp,
                                              MediaFileStatus status) = 0;

        virtual void OnAudioInputStatus(int voicestreamid, const AudioInputStatus& progress) = 0;

        virtual void OnUserAudioBlock(int userid, StreamTypes sts) = 0;

        virtual void OnMTUQueryComplete(int payload_size) = 0;
    };

}

#endif
