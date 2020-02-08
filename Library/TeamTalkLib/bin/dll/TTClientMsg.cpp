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

#include "TTClientMsg.h"
#include <teamtalk/ttassert.h>
#include "Convert.h"

#define INTMSG_MAX_SIZE (0x7F000000)
#define INTMSG_SUSPEND_SIZE (1024*1024)

struct IntTTMessage
{
    ClientEvent event;
    UINT32 nSource;
    TTType ttType;
    union
    {
        char data[1];
        VOID* any;
        Channel* chan;
        User* user;
        UserAccount* useraccount;
        RemoteFile* remotefile;
        TextMessage* textmessage;
        ServerProperties* serverproperties;
        ServerStatistics* serverstatistics;
        FileTransfer* filetransfer;
        ClientErrorMsg* clienterrmsg;
        DesktopInput* desktopinput;
        BannedUser* banneduser;
        TTBOOL* active;
        INT32* bytesremain;
        INT32* stream_id;
        INT32* payload_size;
        MediaFileInfo* mediafileinfo;
        StreamType* streamtype;
        AudioInputProgress* audioinputprogress;
    };
};

IntTTMessage* MakeMsgBlock(ACE_Message_Block*& mb,
                           ClientEvent event,
                           UINT32 nSource,
                           TTType ttType)
{
    int size = TT_DBG_SIZEOF(ttType);
    ACE_NEW_RETURN(mb, ACE_Message_Block(sizeof(IntTTMessage) + size), NULL);
    IntTTMessage* ret = reinterpret_cast<IntTTMessage*>(mb->rd_ptr());
    ret->event = event;
    ret->nSource = nSource;
    ret->ttType = ttType;
    if(size)
        ret->any = &mb->rd_ptr()[sizeof(IntTTMessage)];
    else
        ret->any = NULL;
    return ret;
}

TTMsgQueue::TTMsgQueue()
    : m_suspender(0)
#if defined(WIN32)
    , m_hKeyWnd(0)
    , m_hWnd(0)
    , m_EventHWndMsg(0)
    , m_EventHKeyWndMsg(0)
#endif
{
    InitMsgQueue();
}

#if defined(WIN32)
TTMsgQueue::TTMsgQueue(HWND m_hWnd, UINT eventMsg)
    : m_suspender(0)
    , m_hKeyWnd(0)
    , m_hWnd(m_hWnd)
    , m_EventHWndMsg(eventMsg)
    , m_EventHKeyWndMsg(0)
{
    InitMsgQueue();
}
#endif

TTMsgQueue::~TTMsgQueue()
{
}

void TTMsgQueue::InitMsgQueue()
{
    m_event_queue.low_water_mark(INTMSG_MAX_SIZE);
    m_event_queue.high_water_mark(INTMSG_MAX_SIZE);
}

void TTMsgQueue::EnqueueMsg(ACE_Message_Block* mb)
{
    size_t old_size = m_event_queue.message_bytes();
    ACE_Time_Value tv;
    int ret = m_event_queue.enqueue(mb, &tv);
    TTASSERT(ret >= 0);
    if(m_suspender &&
       old_size < INTMSG_SUSPEND_SIZE &&
       m_event_queue.message_bytes() >= INTMSG_SUSPEND_SIZE)
    {
        m_suspender->SuspendEventHandling();

        IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_INTERNAL_ERROR,
                                         0, __CLIENTERRORMSG);
        msg->clienterrmsg->nErrorNo = INTERR_TTMESSAGE_QUEUE_OVERFLOW;

        ACE_OS::strsncpy(msg->clienterrmsg->szErrorMsg,
                         ACE_TEXT("The internal message queue has overflowed"),
                         TT_STRLEN);
        ret = m_event_queue.enqueue(mb, &tv);
        assert(ret >= 0);
    }

#if defined(WIN32)
    if(m_hWnd)
        ::PostMessage(m_hWnd, m_EventHWndMsg, 0, 0);
#endif
}

TTBOOL TTMsgQueue::GetMessage(TTMessage& msg, ACE_Time_Value* tv)
{
    size_t old_size = m_event_queue.message_bytes();
    ACE_Message_Block* mb;

    if(m_event_queue.dequeue(mb, tv)>=0)
    {
        IntTTMessage* intmsg = reinterpret_cast<IntTTMessage*>(mb->rd_ptr());
        msg.nClientEvent = intmsg->event;
        msg.nSource = intmsg->nSource;
        msg.ttType = intmsg->ttType;
        int size = TT_DBG_SIZEOF(intmsg->ttType);
        TTASSERT(!size || intmsg->any);
        if(size>0 && intmsg->any)
            ACE_OS::memcpy(msg.data, intmsg->any, size);
        mb->release();

        if(m_suspender &&
            old_size >= INTMSG_SUSPEND_SIZE &&
            m_event_queue.message_bytes()<INTMSG_SUSPEND_SIZE)
            m_suspender->ResumeEventHandling();
        return TRUE;
    }
    return FALSE;
}

void TTMsgQueue::RegisterEventSuspender(teamtalk::EventSuspender* suspender)
{
    m_suspender = suspender;
}

void TTMsgQueue::OnConnectSuccess()
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CON_SUCCESS,
                                     0, __NONE);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnConnectFailed()
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CON_FAILED,
                                     0, __NONE);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnConnectionLost()
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CON_LOST,
                                     0, __NONE);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnAccepted(int myuserid, const teamtalk::UserAccount& account)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_MYSELF_LOGGEDIN,
                                     myuserid, __USERACCOUNT);
    Convert(account, *msg->useraccount);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnLoggedOut()
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_MYSELF_LOGGEDOUT,
                                     0, __NONE);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnUserLoggedIn(const teamtalk::ClientUser& user)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_USER_LOGGEDIN,
                                     0, __USER);
    Convert(user, *msg->user);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnUserLoggedOut(const teamtalk::ClientUser& user)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_USER_LOGGEDOUT,
                                     0, __USER);
    Convert(user, *msg->user);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnUserUpdate(const teamtalk::ClientUser& user)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_USER_UPDATE,
                                     0, __USER);
    Convert(user, *msg->user);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnUserJoinChannel(const teamtalk::ClientUser& user,
                                   const teamtalk::ClientChannel& chan)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_USER_JOINED,
                                     0, __USER);
    Convert(user, *msg->user);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnUserLeftChannel(const teamtalk::ClientUser& user,
                                   const teamtalk::ClientChannel& chan)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_USER_LEFT,
                                     chan.GetChannelID(),
                                     __USER);
    Convert(user, *msg->user);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnAddChannel(const teamtalk::ClientChannel& chan)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_CHANNEL_NEW,
                                     0,
                                     __CHANNEL);
    Convert(chan.GetChannelProp(), *msg->chan);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnUpdateChannel(const teamtalk::ClientChannel& chan)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_CHANNEL_UPDATE,
                                     0,
                                     __CHANNEL);
    Convert(chan.GetChannelProp(), *msg->chan);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnRemoveChannel(const teamtalk::ClientChannel& chan)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_CHANNEL_REMOVE,
                                     0,
                                     __CHANNEL);
    Convert(chan.GetChannelProp(), *msg->chan);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnAddFile(const teamtalk::ClientChannel& chan,
                           const teamtalk::RemoteFile& file)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_FILE_NEW,
                                     0,
                                     __REMOTEFILE);
    Convert(file, *msg->remotefile);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnRemoveFile(const teamtalk::ClientChannel& chan,
                              const teamtalk::RemoteFile& file)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_FILE_REMOVE,
                                     0,
                                     __REMOTEFILE);
    Convert(file, *msg->remotefile);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnUserAccount(const teamtalk::UserAccount& account)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_USERACCOUNT,
                                     0,
                                     __USERACCOUNT);
    Convert(account, *msg->useraccount);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnBannedUser(const teamtalk::BannedUser& banuser)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_BANNEDUSER,
                                     0,
                                     __BANNEDUSER);
    Convert(banuser, *msg->banneduser);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnTextMessage(const teamtalk::TextMessage& textmsg)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_USER_TEXTMSG,
                                     0, __TEXTMESSAGE);
    Convert(textmsg, *msg->textmessage);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnJoinedChannel(int channelid)
{
}

void TTMsgQueue::OnLeftChannel(int channelid)
{
}

void TTMsgQueue::OnKicked(const teamtalk::clientuser_t& user, int channelid)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_MYSELF_KICKED,
                                     channelid, user? __USER : __NONE);
    if(user)
        Convert(*user, *msg->user);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnServerUpdate(const teamtalk::ServerInfo& serverinfo)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_SERVER_UPDATE,
                                     0, __SERVERPROPERTIES);
    Convert(serverinfo, *msg->serverproperties);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnServerStatistics(const teamtalk::ServerStats& serverstats)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_SERVERSTATISTICS,
                                     0, __SERVERSTATISTICS);
    Convert(serverstats, *msg->serverstatistics);
    EnqueueMsg(mb);
}


void TTMsgQueue::OnFileTransferStatus(const teamtalk::FileTransfer& transfer)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_FILETRANSFER,
                                     0, __FILETRANSFER);
    Convert(transfer, *msg->filetransfer);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnCommandProcessing(int cmdid, bool begin_end)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_PROCESSING,
                                     cmdid, __TTBOOL);
    *msg->active = !begin_end;
    EnqueueMsg(mb);
}

void TTMsgQueue::OnCommandError(int cmdid, int err_num, const ACE_TString& msg_)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_ERROR,
                                     cmdid, __CLIENTERRORMSG);
    msg->clienterrmsg->nErrorNo = err_num;
    ACE_OS::strsncpy(msg->clienterrmsg->szErrorMsg, msg_.c_str(), TT_STRLEN);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnCommandSuccess(int cmdid)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CMD_SUCCESS,
                                     cmdid, __NONE);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnInternalError(int errorno, const ACE_TString& msg_)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_INTERNAL_ERROR,
                                     0, __CLIENTERRORMSG);
    msg->clienterrmsg->nErrorNo = errorno;
    ACE_OS::strsncpy(msg->clienterrmsg->szErrorMsg, msg_.c_str(), TT_STRLEN);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnVoiceActivated(bool enabled)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_VOICE_ACTIVATION,
                                     0, __TTBOOL);
    *msg->active = enabled;
    EnqueueMsg(mb);
}

void TTMsgQueue::OnUserStateChange(const teamtalk::ClientUser& user)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_USER_STATECHANGE,
                                     0, __USER);
    Convert(user, *msg->user);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnUserVideoCaptureFrame(int userid, int stream_id)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_USER_VIDEOCAPTURE,
                                     userid, __INT32);
    *msg->stream_id = stream_id;
    EnqueueMsg(mb);
}

void TTMsgQueue::OnUserMediaFileVideoFrame(int userid, int stream_id)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_USER_MEDIAFILE_VIDEO,
                                     userid, __INT32);
    *msg->stream_id = stream_id;
    EnqueueMsg(mb);
}

void TTMsgQueue::OnDesktopTransferUpdate(int session_id, int remain_bytes)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_DESKTOPWINDOW_TRANSFER,
                                     session_id, __INT32);
    *msg->bytesremain = remain_bytes;
    EnqueueMsg(mb);
}

void TTMsgQueue::OnUserDesktopWindow(int userid, int session_id)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_USER_DESKTOPWINDOW,
                                     userid, __INT32);
    *msg->stream_id = session_id;
    EnqueueMsg(mb);
}

void TTMsgQueue::OnUserDesktopCursor(int src_userid, const teamtalk::DesktopInput& input)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_USER_DESKTOPCURSOR,
                                     src_userid, __DESKTOPINPUT);
    Convert(input, *msg->desktopinput);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnUserDesktopInput(int src_userid, const teamtalk::DesktopInput& input)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_USER_DESKTOPINPUT,
                                     src_userid, __DESKTOPINPUT);
    Convert(input, *msg->desktopinput);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnChannelStreamMediaFile(const MediaFileProp& mfp,
                                          teamtalk::MediaFileStatus status)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_STREAM_MEDIAFILE,
                                     0, __MEDIAFILEINFO);
    Convert(mfp, *msg->mediafileinfo);
    msg->mediafileinfo->nStatus = (MediaFileStatus)status;
    EnqueueMsg(mb);
}

void TTMsgQueue::OnLocalMediaFilePlayback(int sessionid, const MediaFileProp& mfp,
                                          teamtalk::MediaFileStatus status)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_LOCAL_MEDIAFILE,
        sessionid, __MEDIAFILEINFO);
    Convert(mfp, *msg->mediafileinfo);
    msg->mediafileinfo->nStatus = (MediaFileStatus)status;
    EnqueueMsg(mb);
}

void TTMsgQueue::OnAudioInputStatus(int voicestreamid, const AudioInputStatus& ais)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_AUDIOINPUT,
                                     voicestreamid, __AUDIOINPUTPROGRESS);
    Convert(ais, *msg->audioinputprogress);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnUserAudioBlock(int userid, teamtalk::StreamType stream_type)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_USER_AUDIOBLOCK,
                                     userid, __STREAMTYPE);
    *msg->streamtype = (StreamType)stream_type;
    assert((StreamType)stream_type == STREAMTYPE_VOICE ||
           (StreamType)stream_type == STREAMTYPE_MEDIAFILE_AUDIO);
    EnqueueMsg(mb);
}

void TTMsgQueue::OnMTUQueryComplete(int payload_size)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED,
                                     0, __INT32);
    *msg->payload_size = payload_size;
    EnqueueMsg(mb);
}

//VoiceLogListener
void TTMsgQueue::OnMediaFileStatus(int userid,
                                   teamtalk::MediaFileStatus status,
                                   const teamtalk::VoiceLogFile& vlog)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_USER_RECORD_MEDIAFILE,
                                     userid, __MEDIAFILEINFO);
    Convert(status, vlog, *msg->mediafileinfo);
    EnqueueMsg(mb);
}

/* HotKeyListener events */
#if defined(WIN32)
void TTMsgQueue::OnHotKeyActive(int hotkeyid)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_HOTKEY,
                                     hotkeyid, __TTBOOL);
    *msg->active = TRUE;
    EnqueueMsg(mb);
}

void TTMsgQueue::OnHotKeyInactive(int hotkeyid)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_HOTKEY,
                                     hotkeyid, __TTBOOL);
    *msg->active = FALSE;
    EnqueueMsg(mb);
}

void TTMsgQueue::OnKeyDown(UINT nVK)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_HOTKEY_TEST,
                                    nVK, __TTBOOL);
    *msg->active = TRUE;
    EnqueueMsg(mb);

    if(m_hKeyWnd)
    {
        BOOL b = ::PostMessage(m_hKeyWnd, m_EventHKeyWndMsg, nVK, TRUE);
    }
}

void TTMsgQueue::OnKeyUp(UINT nVK)
{
    ACE_Message_Block* mb;
    IntTTMessage* msg = MakeMsgBlock(mb, CLIENTEVENT_HOTKEY_TEST,
                                    nVK, __TTBOOL);
    *msg->active = FALSE;
    EnqueueMsg(mb);

    if(m_hKeyWnd)
    {
        BOOL b = ::PostMessage(m_hKeyWnd, m_EventHKeyWndMsg, nVK, FALSE);
    }
}
#endif
