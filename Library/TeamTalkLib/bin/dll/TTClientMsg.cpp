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

#include "Convert.h"
#include "TeamTalkDefs.h"
#include "mystd/MyStd.h"
#include "teamtalk/TTAssert.h"

#include <cstddef>
#include <cstring>
#include <memory>
#include <mutex>

constexpr size_t INTMSG_MAX_SIZE = 0x7F000000;
/*
 * Message queue must be able to handle initial login where there's max number
 * of channels, max number of users and all users are in channels, as well as
 * connect and login events */
constexpr size_t INTMSG_SUSPEND_SIZE = ((sizeof(TTMessage) * MAX_CHANNELS) + (sizeof(TTMessage) * MAX_USERS) + (sizeof(TTMessage) * MAX_USERS) + (sizeof(TTMessage) * 100));

constexpr auto DEBUG_TTMSGQUEUE = 0;

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
        SoundDevice* sounddevice;
    };
};

static std::unique_ptr<MsgBlock> MakeMsgBlock(ClientEvent event,
                                              UINT32 nSource,
                                              TTType ttType,
                                              IntTTMessage** out)
{
    int const size = TT_DBG_SIZEOF(ttType);
    auto mb = std::make_unique<MsgBlock>(sizeof(IntTTMessage) + size);
    std::memset(mb->RdPtr(), 0, sizeof(IntTTMessage) + size);
    auto* ret = reinterpret_cast<IntTTMessage*>(mb->RdPtr());
    ret->event = event;
    ret->nSource = nSource;
    ret->ttType = ttType;
    if(size != 0)
        ret->any = mb->RdPtr() + sizeof(IntTTMessage);
    else
        ret->any = nullptr;
    if (out != nullptr)
        *out = ret;
    return mb;
}

TTMsgQueue::TTMsgQueue()
    : m_suspender(nullptr)
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
= default;

void TTMsgQueue::InitMsgQueue()
{
    m_event_queue.SetWaterMark(INTMSG_MAX_SIZE);
    static_assert(INTMSG_MAX_SIZE >= INTMSG_SUSPEND_SIZE, "Message queue size invalid");
}

void TTMsgQueue::EnqueueMsg(std::unique_ptr<MsgBlock> mb)
{
    bool suspend = false;
    {
        std::unique_lock<std::mutex> const g(m_mutex);

        size_t const old_size = m_event_queue.BytesQueued();
        if (old_size <= INTMSG_SUSPEND_SIZE &&
            m_event_queue.BytesQueued() + mb->Capacity() > INTMSG_SUSPEND_SIZE)
        {
            // submit message along with overflow message

            m_event_queue.Put(std::move(mb));

            IntTTMessage* msg = nullptr;
            auto mb2 = MakeMsgBlock(CLIENTEVENT_INTERNAL_ERROR, 0, __CLIENTERRORMSG, &msg);
            Convert(teamtalk::ErrorMsg(INTERR_TTMESSAGE_QUEUE_OVERFLOW,
                                       TTEXT("The internal message queue has overflowed")),
                    *msg->clienterrmsg);

            m_event_queue.Put(std::move(mb2));
            assert(!m_suspended);
            if (m_suspender != nullptr)
                m_suspended = suspend = m_suspender->CanSuspend();
            else
                m_suspended = suspend = true;

            MYTRACE(TTEXT("TTMsgQueue message queue has overflowed. Suspend: %d\n"), static_cast<int>(suspend));
        }
        else if (m_event_queue.BytesQueued() + mb->Capacity() <= INTMSG_SUSPEND_SIZE)
        {
            m_event_queue.Put(std::move(mb));
        }
        else
        {
            if (!m_suspended && (m_suspender != nullptr) && m_suspender->CanSuspend())
                m_suspended = suspend = true;

            auto* intmsg = reinterpret_cast<IntTTMessage*>(mb->RdPtr());
            MYTRACE(TTEXT("TTMsgQueue message queue has overflowed. Dropped ClientEvent %u. Suspend: %d\n"), intmsg->event, static_cast<int>(suspend));
        }
        MYTRACE_COND(DEBUG_TTMSGQUEUE, TTEXT("Enqueue %p: Old size: %u, Cur size: %u. Max size: %u\n"),
                     this, old_size, m_event_queue.BytesQueued(), m_event_queue.GetWaterMark());
    }

    if (suspend && (m_suspender != nullptr))
    {
        m_suspender->SuspendEventHandling(false);
    }

#if defined(WIN32)
    if(m_hWnd)
        ::PostMessage(m_hWnd, m_EventHWndMsg, 0, 0);
#endif
}

TTBOOL TTMsgQueue::GetMessage(TTMessage& msg, std::chrono::milliseconds tv)
{
    bool resume = false;
    std::unique_ptr<MsgBlock> mb;
    {
        // Blocking get with timeout outside m_mutex since
        // EnqueueMsg() acquires m_mutex before calling Put()
        if (m_event_queue.Get(mb, tv))
        {
            std::unique_lock<std::mutex> const g(m_mutex);

            auto* intmsg = reinterpret_cast<IntTTMessage*>(mb->RdPtr());
            msg.nClientEvent = intmsg->event;
            msg.nSource = intmsg->nSource;
            msg.ttType = intmsg->ttType;
            int const size = TT_DBG_SIZEOF(intmsg->ttType);
            TTASSERT(!size || intmsg->any);
            if(size>0 && (intmsg->any != nullptr))
                std::memcpy(msg.data, intmsg->any, size);
            MYTRACE_COND(DEBUG_TTMSGQUEUE, TTEXT("Dequeue %p: Cur size: %u\n"),
                         this, m_event_queue.BytesQueued());

            if (m_suspended && m_event_queue.BytesQueued() <= INTMSG_SUSPEND_SIZE)
            {
                resume = true;
                m_suspended = false;

                MYTRACE_COND(DEBUG_TTMSGQUEUE, TTEXT("TTMsgQueue message queue is resuming.\n"));
            }
        }
    }

    if (resume)
        m_suspender->ResumeEventHandling();

    return static_cast<TTBOOL>(mb != nullptr);
}

void TTMsgQueue::RegisterEventSuspender(teamtalk::EventSuspender* suspender)
{
    m_suspender = suspender;
}

void TTMsgQueue::OnConnectSuccess()
{
    auto mb = MakeMsgBlock(CLIENTEVENT_CON_SUCCESS, 0, __NONE, nullptr);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnConnectFailed()
{
    auto mb = MakeMsgBlock(CLIENTEVENT_CON_FAILED, 0, __NONE, nullptr);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnConnectionLost()
{
    auto mb = MakeMsgBlock(CLIENTEVENT_CON_LOST, 0, __NONE, nullptr);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnEncryptionFailed(int sslerr, const tstring& errmsg)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CON_CRYPT_ERROR,
                                     sslerr, __CLIENTERRORMSG, &msg);
    Convert(teamtalk::ErrorMsg(sslerr, errmsg), *msg->clienterrmsg);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnAccepted(int myuserid, const teamtalk::UserAccount& account)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_MYSELF_LOGGEDIN,
                                     myuserid, __USERACCOUNT, &msg);
    Convert(account, *msg->useraccount);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnLoggedOut()
{
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_MYSELF_LOGGEDOUT, 0, __NONE, nullptr);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUserLoggedIn(const teamtalk::ClientUser& user)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_USER_LOGGEDIN,
                                     0, __USER, &msg);
    Convert(user, *msg->user);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUserLoggedOut(const teamtalk::ClientUser& user)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_USER_LOGGEDOUT,
                                     0, __USER, &msg);
    Convert(user, *msg->user);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUserUpdate(const teamtalk::ClientUser& user)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_USER_UPDATE,
                                     0, __USER, &msg);
    Convert(user, *msg->user);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUserJoinChannel(const teamtalk::ClientUser& user,
                                   const teamtalk::ClientChannel& /*chan*/)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_USER_JOINED,
                                     0, __USER, &msg);
    Convert(user, *msg->user);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUserLeftChannel(const teamtalk::ClientUser& user,
                                   const teamtalk::ClientChannel& chan)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_USER_LEFT,
                                     chan.GetChannelID(),
                                     __USER, &msg);
    Convert(user, *msg->user);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnAddChannel(const teamtalk::ClientChannel& chan)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_CHANNEL_NEW,
                                     0,
                                     __CHANNEL, &msg);
    Convert(chan.GetChannelProp(), *msg->chan);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUpdateChannel(const teamtalk::ClientChannel& chan)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_CHANNEL_UPDATE,
                                     0,
                                     __CHANNEL, &msg);
    Convert(chan.GetChannelProp(), *msg->chan);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnRemoveChannel(const teamtalk::ClientChannel& chan)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_CHANNEL_REMOVE,
                                     0,
                                     __CHANNEL, &msg);
    Convert(chan.GetChannelProp(), *msg->chan);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnAddFile(const teamtalk::ClientChannel& /*chan*/,
                           const teamtalk::RemoteFile& file)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_FILE_NEW,
                                     0,
                                     __REMOTEFILE, &msg);
    Convert(file, *msg->remotefile);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnRemoveFile(const teamtalk::ClientChannel& /*chan*/,
                              const teamtalk::RemoteFile& file)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_FILE_REMOVE,
                                     0,
                                     __REMOTEFILE, &msg);
    Convert(file, *msg->remotefile);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUserAccount(const teamtalk::UserAccount& account)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_USERACCOUNT,
                                     0,
                                     __USERACCOUNT, &msg);
    Convert(account, *msg->useraccount);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnAddUserAccount(const teamtalk::UserAccount& account)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_USERACCOUNT_NEW,
                                     0,
                                     __USERACCOUNT, &msg);
    Convert(account, *msg->useraccount);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnRemoveUserAccount(const tstring& username)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_USERACCOUNT_REMOVE,
                                     0,
                                     __USERACCOUNT, &msg);
    teamtalk::UserAccount account;
    account.username = username;
    Convert(account, *msg->useraccount);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnBannedUser(const teamtalk::BannedUser& banuser)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_BANNEDUSER,
                                     0,
                                     __BANNEDUSER, &msg);
    Convert(banuser, *msg->banneduser);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnTextMessage(const teamtalk::TextMessage& textmsg)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_USER_TEXTMSG,
                                     0, __TEXTMESSAGE, &msg);
    Convert(textmsg, *msg->textmessage);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnJoinedChannel(int /*channelid*/)
{
}

void TTMsgQueue::OnLeftChannel(int /*channelid*/)
{
}

void TTMsgQueue::OnKicked(const teamtalk::clientuser_t& user, int channelid)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_MYSELF_KICKED,
                                     channelid, user? __USER : __NONE, &msg);
    if(user)
        Convert(*user, *msg->user);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnServerUpdate(const teamtalk::ServerInfo& serverinfo)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_SERVER_UPDATE,
                                     0, __SERVERPROPERTIES, &msg);
    Convert(serverinfo, *msg->serverproperties);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnServerStatistics(const teamtalk::ServerStats& serverstats)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_SERVERSTATISTICS,
                                     0, __SERVERSTATISTICS, &msg);
    Convert(serverstats, *msg->serverstatistics);
    EnqueueMsg(std::move(mb));
}


void TTMsgQueue::OnFileTransferStatus(const teamtalk::FileTransfer& transfer)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_FILETRANSFER,
                                     0, __FILETRANSFER, &msg);
    Convert(transfer, *msg->filetransfer);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnCommandProcessing(int cmdid, bool begin_end)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_PROCESSING,
                                     cmdid, __TTBOOL, &msg);
    *msg->active = static_cast<TTBOOL>(!begin_end);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnCommandError(int cmdid, int err_num, const tstring& msg_)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_ERROR,
                                     cmdid, __CLIENTERRORMSG, &msg);
    Convert(teamtalk::ErrorMsg(err_num, msg_), *msg->clienterrmsg);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnCommandSuccess(int cmdid)
{
    auto mb = MakeMsgBlock(CLIENTEVENT_CMD_SUCCESS, cmdid, __NONE, nullptr);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnInternalError(int errorno, const tstring& msg_)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_INTERNAL_ERROR,
                                     0, __CLIENTERRORMSG, &msg);
    Convert(teamtalk::ErrorMsg(errorno, msg_), *msg->clienterrmsg);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnVoiceActivated(bool enabled)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_VOICE_ACTIVATION,
                                     0, __TTBOOL, &msg);
    *msg->active = static_cast<TTBOOL>(enabled);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUserFirstStreamVoicePacket(const teamtalk::ClientUser& user, int streamid)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_USER_FIRSTVOICESTREAMPACKET,
                                     streamid, __USER, &msg);
    Convert(user, *msg->user);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUserStateChange(const teamtalk::ClientUser& user)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_USER_STATECHANGE,
                                     0, __USER, &msg);
    Convert(user, *msg->user);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUserVideoCaptureFrame(int userid, int stream_id)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_USER_VIDEOCAPTURE,
                                     userid, __INT32, &msg);
    *msg->stream_id = stream_id;
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUserMediaFileVideoFrame(int userid, int stream_id)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_USER_MEDIAFILE_VIDEO,
                                     userid, __INT32, &msg);
    *msg->stream_id = stream_id;
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnDesktopTransferUpdate(int session_id, int remain_bytes)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_DESKTOPWINDOW_TRANSFER,
                                     session_id, __INT32, &msg);
    *msg->bytesremain = remain_bytes;
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUserDesktopWindow(int userid, int session_id)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_USER_DESKTOPWINDOW,
                                     userid, __INT32, &msg);
    *msg->stream_id = session_id;
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUserDesktopCursor(int src_userid, const teamtalk::DesktopInput& input)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_USER_DESKTOPCURSOR,
                                     src_userid, __DESKTOPINPUT, &msg);
    Convert(input, *msg->desktopinput);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUserDesktopInput(int src_userid, const teamtalk::DesktopInput& input)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_USER_DESKTOPINPUT,
                                     src_userid, __DESKTOPINPUT, &msg);
    Convert(input, *msg->desktopinput);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnChannelStreamMediaFile(const MediaFileProp& mfp,
                                          teamtalk::MediaFileStatus status)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_STREAM_MEDIAFILE,
                                     0, __MEDIAFILEINFO, &msg);
    Convert(mfp, *msg->mediafileinfo);
    msg->mediafileinfo->nStatus = static_cast<MediaFileStatus>(status);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnLocalMediaFilePlayback(int sessionid, const MediaFileProp& mfp,
                                          teamtalk::MediaFileStatus status)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_LOCAL_MEDIAFILE,
                                     sessionid, __MEDIAFILEINFO, &msg);
    Convert(mfp, *msg->mediafileinfo);
    msg->mediafileinfo->nStatus = static_cast<MediaFileStatus>(status);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnAudioInputStatus(int voicestreamid, const AudioInputStatus& ais)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_AUDIOINPUT,
                                     voicestreamid, __AUDIOINPUTPROGRESS, &msg);
    Convert(ais, *msg->audioinputprogress);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnUserAudioBlock(int userid, teamtalk::StreamTypes sts)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_USER_AUDIOBLOCK,
                                     userid, __STREAMTYPE, &msg);
    *msg->streamtype = static_cast<StreamType>(sts);
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnMTUQueryComplete(int payload_size)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED,
                                     0, __INT32, &msg);
    *msg->payload_size = payload_size;
    EnqueueMsg(std::move(mb));
}

//VoiceLogListener
void TTMsgQueue::OnMediaFileStatus(int userid,
                                   teamtalk::MediaFileStatus status,
                                   const teamtalk::VoiceLogFile& vlog)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_USER_RECORD_MEDIAFILE,
                                     userid, __MEDIAFILEINFO, &msg);
    Convert(status, vlog, *msg->mediafileinfo);
    EnqueueMsg(std::move(mb));
}

/* HotKeyListener events */
#if defined(WIN32)
void TTMsgQueue::OnHotKeyActive(int hotkeyid)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_HOTKEY,
                                     hotkeyid, __TTBOOL, &msg);
    *msg->active = TRUE;
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnHotKeyInactive(int hotkeyid)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_HOTKEY,
                                     hotkeyid, __TTBOOL, &msg);
    *msg->active = FALSE;
    EnqueueMsg(std::move(mb));
}

void TTMsgQueue::OnKeyDown(UINT nVK)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_HOTKEY_TEST,
                                    nVK, __TTBOOL, &msg);
    *msg->active = TRUE;
    EnqueueMsg(std::move(mb));

    if(m_hKeyWnd)
    {
        BOOL b = ::PostMessage(m_hKeyWnd, m_EventHKeyWndMsg, nVK, TRUE);
    }
}

void TTMsgQueue::OnKeyUp(UINT nVK)
{
    IntTTMessage* msg = nullptr;
    auto mb = MakeMsgBlock(CLIENTEVENT_HOTKEY_TEST,
                                    nVK, __TTBOOL, &msg);
    *msg->active = FALSE;
    EnqueueMsg(std::move(mb));

    if(m_hKeyWnd)
    {
        BOOL b = ::PostMessage(m_hKeyWnd, m_EventHKeyWndMsg, nVK, FALSE);
    }
}

void TTMsgQueue::AudioDeviceChange(AudioDevEvent event, const LPCWSTR& name, const LPCWSTR& id)
{
    SoundDevice snd = {};
    snd.nSoundSystem = SOUNDSYSTEM_WASAPI;
    snd.nDeviceID = -1;
    tstrsncpy(snd.szDeviceName, name, TT_STRLEN);
    tstrsncpy(snd.szDeviceID, id, TT_STRLEN);

    IntTTMessage* msg = nullptr;
    std::unique_ptr<MsgBlock> mb;
    switch (event)
    {
    case AUDIODEVICE_ADD :
        mb = MakeMsgBlock(CLIENTEVENT_SOUNDDEVICE_ADDED, 0, __SOUNDDEVICE, &msg);
        break;
    case AUDIODEVICE_REMOVE :
        mb = MakeMsgBlock(CLIENTEVENT_SOUNDDEVICE_REMOVED, 0, __SOUNDDEVICE, &msg);
        break;
    case AUDIODEVICE_UNPLUGGED:
        mb = MakeMsgBlock(CLIENTEVENT_SOUNDDEVICE_UNPLUGGED, 0, __SOUNDDEVICE, &msg);
        break;
    case AUDIODEVICE_NEW_DEFAULT_INPUT :
        mb = MakeMsgBlock(CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_INPUT, 0, __SOUNDDEVICE, &msg);
        break;
    case AUDIODEVICE_NEW_DEFAULT_OUTPUT :
        mb = MakeMsgBlock(CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_OUTPUT, 0, __SOUNDDEVICE, &msg);
        break;
    case AUDIODEVICE_NEW_DEFAULT_INPUT_COMDEVICE:
        mb = MakeMsgBlock(CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_INPUT_COMDEVICE, 0, __SOUNDDEVICE, &msg);
        break;
    case AUDIODEVICE_NEW_DEFAULT_OUTPUT_COMDEVICE:
        mb = MakeMsgBlock(CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_OUTPUT_COMDEVICE, 0, __SOUNDDEVICE, &msg);
        break;
    }

    *msg->sounddevice = snd;
    EnqueueMsg(std::move(mb));
}

#endif /* WIN32 */
