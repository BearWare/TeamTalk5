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

#if !defined(TEAMTALK_MESSAGES)
#define TEAMTALK_MESSAGES

#include <teamtalk/client/ClientNode.h>
#include <TeamTalk.h>

#if defined(WIN32)
#include <win32/HotKey.h>
#endif

class TTMsgQueue
    : public teamtalk::ClientListener
#if defined(WIN32)
    , public HotKeyListener
#endif
{
    msg_queue_t m_event_queue;
    teamtalk::EventSuspender* m_suspender;
    bool m_suspended = false;
    std::mutex m_mutex;
#if defined(WIN32)
    HWND m_hKeyWnd;
    HWND m_hWnd;
    UINT m_EventHWndMsg;
    UINT m_EventHKeyWndMsg;
#endif
    void InitMsgQueue();
    void EnqueueMsg(ACE_Message_Block* mb);

public:
    TTMsgQueue();
#if defined(WIN32)
    TTMsgQueue(HWND m_hWnd, UINT eventMsg);

    void SetHWND(HWND hWnd) { m_hWnd = hWnd; }
#endif
    virtual ~TTMsgQueue();

    TTBOOL GetMessage(TTMessage& msg, ACE_Time_Value* tv);

    //ClientListener
    void RegisterEventSuspender(teamtalk::EventSuspender* suspender) override;

    void OnConnectSuccess() override;
    void OnConnectFailed() override;
    void OnConnectionLost() override;

    void OnAccepted(int myuserid, const teamtalk::UserAccount& account) override;
    void OnLoggedOut() override;

    void OnUserLoggedIn(const teamtalk::ClientUser& user) override;
    void OnUserLoggedOut(const teamtalk::ClientUser& user) override;
    void OnUserUpdate(const teamtalk::ClientUser& user) override;

    void OnUserJoinChannel(const teamtalk::ClientUser& user,
                           const teamtalk::ClientChannel& chan) override;
    void OnUserLeftChannel(const teamtalk::ClientUser& user,
                           const teamtalk::ClientChannel& chan) override;

    void OnAddChannel(const teamtalk::ClientChannel& chan) override;
    void OnUpdateChannel(const teamtalk::ClientChannel& chan) override;
    void OnRemoveChannel(const teamtalk::ClientChannel& chan) override;

    void OnJoinedChannel(int channelid) override;
    void OnLeftChannel(int channelid) override;

    void OnAddFile(const teamtalk::ClientChannel& chan,
                   const teamtalk::RemoteFile& file) override;
    void OnRemoveFile(const teamtalk::ClientChannel& chan,
                      const teamtalk::RemoteFile& file) override;

    void OnUserAccount(const teamtalk::UserAccount& account) override;
    void OnBannedUser(const teamtalk::BannedUser& banuser) override;

    void OnTextMessage(const teamtalk::TextMessage& textmsg) override;

    void OnFileTransferStatus(const teamtalk::FileTransfer& transfer) override;

    void OnKicked(const teamtalk::clientuser_t& user, int channelid) override;
    void OnServerUpdate(const teamtalk::ServerInfo& serverinfo) override;

    void OnServerStatistics(const teamtalk::ServerStats& serverstats) override;

    void OnCommandError(int cmdid, int err_num, const ACE_TString& msg) override;
    void OnCommandSuccess(int cmdid) override;
    void OnCommandProcessing(int cmdid, bool begin_end) override;

    void OnInternalError(int errorno, const ACE_TString& msg) override;

    void OnVoiceActivated(bool enabled) override;

    void OnUserFirstStreamVoicePacket(const teamtalk::ClientUser& user, int streamid) override;
    void OnUserStateChange(const teamtalk::ClientUser& user) override;
    void OnUserVideoCaptureFrame(int userid, int stream_id) override;
    void OnUserMediaFileVideoFrame(int userid, int stream_id) override;

    void OnDesktopTransferUpdate(int session_id, int remain_bytes) override;

    void OnUserDesktopWindow(int userid, int session_id) override;
    void OnUserDesktopCursor(int src_userid, const teamtalk::DesktopInput& input) override;
    void OnUserDesktopInput(int src_userid, const teamtalk::DesktopInput& input) override;

    void OnChannelStreamMediaFile(const MediaFileProp& mfp,
                                  teamtalk::MediaFileStatus active) override;

    void OnLocalMediaFilePlayback(int sessionid, const MediaFileProp& mfp,
                                  teamtalk::MediaFileStatus status) override;

    void OnAudioInputStatus(int voicestreamid, const AudioInputStatus& ais) override;

    void OnUserAudioBlock(int userid, teamtalk::StreamTypes sts) override;

    void OnMTUQueryComplete(int payload_size) override;

    //VoiceLogListener
    void OnMediaFileStatus(int userid, teamtalk::MediaFileStatus status, 
                           const teamtalk::VoiceLogFile& vlog) override;

    /* HotKeyListener events */
#if defined(WIN32)
    void SetKeyHWND(HWND hKeyWnd, UINT eventMsg)
    { m_hKeyWnd = hKeyWnd; m_EventHKeyWndMsg = eventMsg; }

    void OnHotKeyActive(int hotkeyid) override;
    void OnHotKeyInactive(int hotkeyid) override;

    void OnKeyDown(UINT nVK) override;
    void OnKeyUp(UINT nVK) override;
#endif
};

#endif
