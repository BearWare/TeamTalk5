#include "BackendAdapter.h"
#include <QDebug>

BackendAdapter::BackendAdapter(QObject* parent)
    : QObject(parent)
{
    m_tt = TT_InitTeamTalkPoll();
    if (!m_tt) {
        ErrorEvent err;
        err.message = QStringLiteral("Failed to initialize TeamTalk backend");
        emit errorOccurred(err);
        return;
    }

    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout,
            this, &BackendAdapter::pollTeamTalk);
    m_pollTimer->start(20);
}

void BackendAdapter::connectToServer(const QString& host, int port)
{
    if (!m_tt)
        return;

    const QByteArray hostUtf8 = host.toUtf8();

    INT32 cmdid = TT_Connect(m_tt,
                             hostUtf8.constData(),
                             port,
                             0,
                             0,
                             0,
                             0);

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

        // --------------------------------------------------------------
        // CONNECTION EVENTS
        // --------------------------------------------------------------

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

        // --------------------------------------------------------------
        // COMMAND ERRORS
        // --------------------------------------------------------------

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

        // --------------------------------------------------------------
        // YOU JOINED A CHANNEL
        // --------------------------------------------------------------

        case CLIENTEVENT_CMD_MYSELF_JOINED: {
            ChannelEvent ev;
            ev.type = ChannelEventType::Joined;
            ev.channelId = msg.user.nChannelID;
            emit channelEvent(ev);
            break;
        }

        // --------------------------------------------------------------
        // YOU LEFT A CHANNEL
        // --------------------------------------------------------------

        case CLIENTEVENT_CMD_MYSELF_LEFT: {
            ChannelEvent ev;
            ev.type = ChannelEventType::Left;
            ev.channelId = msg.user.nChannelID;
            emit channelEvent(ev);
            break;
        }

        // --------------------------------------------------------------
        // SELF VOICE STATE (talking indicator)
        // --------------------------------------------------------------

        case CLIENTEVENT_USER_STATECHANGE:
        case CLIENTEVENT_CMD_USER_UPDATE: {
            if (msg.user.nUserID == TT_GetMyUserID(m_tt)) {
                SelfVoiceEvent ev;
                bool talking = (msg.user.uUserState & USERSTATE_VOICE) != 0;
                ev.state = talking ? SelfVoiceState::Talking : SelfVoiceState::Silent;
                emit selfVoiceEvent(ev);
            }
            break;
        }

        // --------------------------------------------------------------
        // AUDIO DEVICE EVENTS
        // --------------------------------------------------------------

        case CLIENTEVENT_SOUNDDEVICE_ADD: {
            AudioDeviceEvent ev;
            ev.type = AudioDeviceEventType::Added;
            emit audioDeviceEvent(ev);
            break;
        }

        case CLIENTEVENT_SOUNDDEVICE_REMOVE: {
            AudioDeviceEvent ev;
            ev.type = AudioDeviceEventType::Removed;
            emit audioDeviceEvent(ev);
            break;
        }

        case CLIENTEVENT_SOUNDDEVICE_FAILURE: {
            AudioDeviceEvent ev;
            ev.type = AudioDeviceEventType::Failed;
            emit audioDeviceEvent(ev);
            break;
        }

        // --------------------------------------------------------------
        // TEXT MESSAGES (optional TTS)
        // --------------------------------------------------------------

        case CLIENTEVENT_USER_TEXTMESSAGE:
        case CLIENTEVENT_CMD_TEXTMESSAGE: {
            TextMessageEvent ev;
            ev.fromUserId = msg.textmessage.nFromUserID;
            ev.message = QString::fromUtf8(msg.textmessage.szMessage);
            emit textMessageEvent(ev);
            break;
        }

        // --------------------------------------------------------------
        // IGNORE EVERYTHING ELSE
        // --------------------------------------------------------------

        default:
            break;
        }
    }
}
void BackendAdapter::refreshChannels()
{
    const QList<ChannelInfo> channels = enumerateChannels();
    emit channelsEnumerated(channels);
}

QList<ChannelInfo> BackendAdapter::enumerateChannels() const
{
    QList<ChannelInfo> out;

    if (!m_tt)
        return out;

    // TeamTalk C API: get all channels on the server
    int chanids[TT_CHANNELS_MAX];
    int count = TT_GetServerChannels(m_tt, chanids, TT_CHANNELS_MAX);

    if (count <= 0)
        return out;

    for (int i = 0; i < count; ++i) {
        TTChannel chan = {};
        if (!TT_GetChannel(m_tt, chanids[i], &chan))
            continue;

        ChannelInfo ci;
        ci.id = chan.nChannelID;
        ci.name = QString::fromUtf8(chan.szName);
        ci.userCount = chan.nUsers;
        ci.isPasswordProtected = chan.bPassword != 0;
        ci.topic = QString::fromUtf8(chan.szTopic);

        out.append(ci);
    }

    return out;
}
