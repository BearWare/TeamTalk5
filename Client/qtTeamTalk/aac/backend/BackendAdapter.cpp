#include "BackendAdapter.h"
#include <QDebug>

BackendAdapter::BackendAdapter(QObject* parent)
    : QObject(parent)
{
    // Initialize TeamTalk in polling mode
    m_tt = TT_InitTeamTalkPoll();
    if (!m_tt) {
        ErrorEvent err;
        err.message = QStringLiteral("Failed to initialize TeamTalk backend");
        emit errorOccurred(err);
        return;
    }

    // Start periodic polling of TeamTalk messages
    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout,
            this, &BackendAdapter::pollTeamTalk);
    m_pollTimer->start(20); // 20 ms polling interval
}

void BackendAdapter::connectToServer(const QString& host, int port)
{
    if (!m_tt)
        return;

    const QByteArray hostUtf8 = host.toUtf8();

    INT32 cmdid = TT_Connect(m_tt,
                             hostUtf8.constData(),
                             port,
                             0,  // udpport
                             0,  // encrypted
                             0,  // timeout
                             0); // local bind IP

    if (cmdid <= 0) {
        ErrorEvent err;
        err.message = QStringLiteral("Failed to initiate connection");
        emit errorOccurred(err);
    }
}

void BackendAdapter::disconnectFromServer()
{
    if (!m_tt)
        return;

    TT_Disconnect(m_tt);
}

void BackendAdapter::joinChannel(int channelId)
{
    if (!m_tt)
        return;

    INT32 cmdid = TT_DoJoinChannelByID(m_tt, channelId, "");
    if (cmdid <= 0) {
        ErrorEvent err;
        err.message = QStringLiteral("Failed to send join channel command");
        emit errorOccurred(err);
    }
}

void BackendAdapter::leaveChannel()
{
    if (!m_tt)
        return;

    INT32 cmdid = TT_DoLeaveChannel(m_tt);
    if (cmdid <= 0) {
        ErrorEvent err;
        err.message = QStringLiteral("Failed to send leave channel command");
        emit errorOccurred(err);
    }
}

void BackendAdapter::setTransmitEnabled(bool enabled)
{
    if (!m_tt)
        return;

    INT32 myUserId = TT_GetMyUserID(m_tt);
    if (myUserId <= 0)
        return;

    bool ok = TT_SetUserTransmitStream(m_tt,
                                       myUserId,
                                       STREAMTYPE_VOICE,
                                       enabled ? TRANSMIT_ENABLE : TRANSMIT_DISABLE);

    if (!ok) {
        ErrorEvent err;
        err.message = QStringLiteral("Failed to change transmit state");
        emit errorOccurred(err);
    }
}

void BackendAdapter::pollTeamTalk()
{
    if (!m_tt)
        return;

    TTMessage msg;
    while (TT_GetMessage(m_tt, &msg, 0)) {

        switch (msg.nClientEvent) {

        // ---------------------------------------------------------------------
        // CONNECTION EVENTS
        // ---------------------------------------------------------------------

        case CLIENTEVENT_CON_SUCCESS: {
            emit connectionStateChanged(ConnectionState::Connected);
            break;
        }

        case CLIENTEVENT_CON_FAILED: {
            ErrorEvent err;
            err.message = QStringLiteral("Connection failed");
            emit errorOccurred(err);
            break;
        }

        case CLIENTEVENT_CON_LOST: {
            emit connectionStateChanged(ConnectionState::Disconnected);
            break;
        }

        // ---------------------------------------------------------------------
        // COMMAND ERRORS
        // ---------------------------------------------------------------------

        case CLIENTEVENT_CMD_ERROR: {
            ErrorEvent err;
            if (msg.clienterrormsg.szErrorMsg[0]) {
                err.message = QString::fromUtf8(msg.clienterrormsg.szErrorMsg);
            } else {
                err.message = QStringLiteral("Backend reported an unknown error");
            }
            emit errorOccurred(err);
            break;
        }

        // ---------------------------------------------------------------------
        // CHANNEL EVENTS
        // ---------------------------------------------------------------------

        case CLIENTEVENT_CMD_ADD_CHANNEL: {
            ChannelEvent ev;
            // ev.type = ChannelEventType::ChannelAdded;
            // ev.channelId = msg.channel.nChannelID;
            // ev.parentId = msg.channel.nParentID;
            // ev.name = QString::fromUtf8(msg.channel.szName);
            emit channelEvent(ev);
            break;
        }

        case CLIENTEVENT_CMD_UPDATE_CHANNEL: {
            ChannelEvent ev;
            // ev.type = ChannelEventType::ChannelUpdated;
            // ev.channelId = msg.channel.nChannelID;
            // ev.parentId = msg.channel.nParentID;
            // ev.name = QString::fromUtf8(msg.channel.szName);
            emit channelEvent(ev);
            break;
        }

        case CLIENTEVENT_CMD_REMOVE_CHANNEL: {
            ChannelEvent ev;
            // ev.type = ChannelEventType::ChannelRemoved;
            // ev.channelId = msg.channel.nChannelID;
            emit channelEvent(ev);
            break;
        }

        // ---------------------------------------------------------------------
        // USER EVENTS (JOIN/LEAVE/UPDATE/TALKING)
        // ---------------------------------------------------------------------

        case CLIENTEVENT_CMD_USER_JOINED: {
            ChannelEvent ev;
            // ev.type = ChannelEventType::UserJoined;
            // ev.userId = msg.user.nUserID;
            // ev.channelId = msg.user.nChannelID;
            // ev.userName = QString::fromUtf8(msg.user.szNickname);
            emit channelEvent(ev);
            break;
        }

        case CLIENTEVENT_CMD_USER_LEFT: {
            ChannelEvent ev;
            // ev.type = ChannelEventType::UserLeft;
            // ev.userId = msg.user.nUserID;
            // ev.channelId = msg.user.nChannelID;
            emit channelEvent(ev);
            break;
        }

        case CLIENTEVENT_CMD_USER_UPDATE: {
            ChannelEvent ev;
            // bool talking = (msg.user.uUserState & USERSTATE_VOICE) != 0;
            // ev.type = ChannelEventType::UserUpdated;
            // ev.userId = msg.user.nUserID;
            // ev.channelId = msg.user.nChannelID;
            // ev.isTalking = talking;
            emit channelEvent(ev);
            break;
        }

        case CLIENTEVENT_USER_STATECHANGE: {
            ChannelEvent ev;
            // bool talking = (msg.user.uUserState & USERSTATE_VOICE) != 0;
            // ev.type = ChannelEventType::UserUpdated;
            // ev.userId = msg.user.nUserID;
            // ev.channelId = msg.user.nChannelID;
            // ev.isTalking = talking;
            emit channelEvent(ev);
            break;
        }

        // ---------------------------------------------------------------------
        // AAC‑RELEVANT: AUDIO DEVICE CHANGES
        // ---------------------------------------------------------------------

        case CLIENTEVENT_SOUNDDEVICE_ADD: {
            ErrorEvent err;
            err.message = QStringLiteral("A new sound device was added");
            emit errorOccurred(err);
            break;
        }

        case CLIENTEVENT_SOUNDDEVICE_REMOVE: {
            ErrorEvent err;
            err.message = QStringLiteral("A sound device was removed");
            emit errorOccurred(err);
            break;
        }

        case CLIENTEVENT_SOUNDDEVICE_FAILURE: {
            ErrorEvent err;
            err.message = QStringLiteral("A sound device failed");
            emit errorOccurred(err);
            break;
        }

        // ---------------------------------------------------------------------
        // AAC‑RELEVANT: SERVER STATISTICS
        // ---------------------------------------------------------------------

        case CLIENTEVENT_CMD_SERVERSTATISTICS: {
            // msg.serverstatistics
            break;
        }

        // ---------------------------------------------------------------------
        // OPTIONAL: TEXT MESSAGES
        // ---------------------------------------------------------------------

        case CLIENTEVENT_USER_TEXTMESSAGE: {
            break;
        }

        case CLIENTEVENT_CMD_TEXTMESSAGE: {
            break;
        }

        // ---------------------------------------------------------------------
        // OPTIONAL: CHANNEL OPERATORS
        // ---------------------------------------------------------------------

        case CLIENTEVENT_CMD_CHANNEL_OPERATOR: {
            break;
        }

        // ---------------------------------------------------------------------
        // OPTIONAL: USER ACCOUNT EVENTS
        // ---------------------------------------------------------------------

        case CLIENTEVENT_CMD_USERACCOUNT: {
            break;
        }

        // ---------------------------------------------------------------------
        // OPTIONAL: FILE TRANSFERS
        // ---------------------------------------------------------------------

        case CLIENTEVENT_FILETRANSFER: {
            break;
        }

        case CLIENTEVENT_FILETRANSFER_BEGIN: {
            break;
        }

        case CLIENTEVENT_FILETRANSFER_END: {
            break;
        }

        // ---------------------------------------------------------------------
        // OPTIONAL: MEDIA PLAYBACK
        // ---------------------------------------------------------------------

        case CLIENTEVENT_USER_MEDIAFILE_STATUS: {
            break;
        }

        // ---------------------------------------------------------------------
        // OPTIONAL: VOICE ACTIVATION (VAD)
        // ---------------------------------------------------------------------

        case CLIENTEVENT_VOICEACTIVATION: {
            break;
        }

        // ---------------------------------------------------------------------
        // DEFAULT: IGNORE EVERYTHING ELSE
        // ---------------------------------------------------------------------

        default:
            break;
        }
    }
}
