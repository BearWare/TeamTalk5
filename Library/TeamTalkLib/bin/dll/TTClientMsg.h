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
    void RegisterEventSuspender(teamtalk::EventSuspender* suspender);

    void OnConnectSuccess();
    void OnConnectFailed();
    void OnConnectionLost();

    void OnAccepted(int myuserid, const teamtalk::UserAccount& account);
    void OnLoggedOut();

    void OnUserLoggedIn(const teamtalk::ClientUser& user);
    void OnUserLoggedOut(const teamtalk::ClientUser& user);
    void OnUserUpdate(const teamtalk::ClientUser& user);

    void OnUserJoinChannel(const teamtalk::ClientUser& user,
                           const teamtalk::ClientChannel& chan);
    void OnUserLeftChannel(const teamtalk::ClientUser& user,
                           const teamtalk::ClientChannel& chan);

    void OnAddChannel(const teamtalk::ClientChannel& chan);
    void OnUpdateChannel(const teamtalk::ClientChannel& chan);
    void OnRemoveChannel(const teamtalk::ClientChannel& chan);

    void OnJoinedChannel(int channelid);
    void OnLeftChannel(int channelid);

    void OnAddFile(const teamtalk::ClientChannel& chan,
                   const teamtalk::RemoteFile& file);
    void OnRemoveFile(const teamtalk::ClientChannel& chan,
                      const teamtalk::RemoteFile& file);

    void OnUserAccount(const teamtalk::UserAccount& account);
    void OnBannedUser(const teamtalk::BannedUser& banuser);

    void OnTextMessage(const teamtalk::TextMessage& textmsg);

    void OnFileTransferStatus(const teamtalk::FileTransfer& transfer);

    void OnKicked(const teamtalk::clientuser_t& user, int channelid);
    void OnServerUpdate(const teamtalk::ServerInfo& serverinfo);

    void OnServerStatistics(const teamtalk::ServerStats& serverstats);

    void OnCommandError(int cmdid, int err_num, const ACE_TString& msg);
    void OnCommandSuccess(int cmdid);
    void OnCommandProcessing(int cmdid, bool begin_end);

    void OnInternalError(int errorno, const ACE_TString& msg);

    void OnVoiceActivated(bool enabled);

    void OnUserStateChange(const teamtalk::ClientUser& user);
    void OnUserVideoCaptureFrame(int userid, int stream_id);
    void OnUserMediaFileVideoFrame(int userid, int stream_id);

    void OnDesktopTransferUpdate(int session_id, int remain_bytes);

    void OnUserDesktopWindow(int userid, int session_id);
    void OnUserDesktopCursor(int src_userid, const teamtalk::DesktopInput& input);
    void OnUserDesktopInput(int src_userid, const teamtalk::DesktopInput& input);

    void OnChannelStreamMediaFile(const MediaFileProp& mfp,
                                  teamtalk::MediaFileStatus active);

    void OnLocalMediaFilePlayback(int sessionid, const MediaFileProp& mfp,
                                  teamtalk::MediaFileStatus status);

    void OnAudioInputStatus(int voicestreamid, const AudioInputStatus& ais);

    void OnUserAudioBlock(int userid, teamtalk::StreamType stream_type);

    void OnMTUQueryComplete(int payload_size);

    //VoiceLogListener
    void OnMediaFileStatus(int userid, teamtalk::MediaFileStatus status, 
                           const teamtalk::VoiceLogFile& vlog);

    /* HotKeyListener events */
#if defined(WIN32)
    void SetKeyHWND(HWND hKeyWnd, UINT eventMsg)
    { m_hKeyWnd = hKeyWnd; m_EventHKeyWndMsg = eventMsg; }

    void OnHotKeyActive(int hotkeyid);
    void OnHotKeyInactive(int hotkeyid);

    void OnKeyDown(UINT nVK);
    void OnKeyUp(UINT nVK);
#endif
};

#endif
