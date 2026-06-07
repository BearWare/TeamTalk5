#include "BackendAdapter.h"
#include <QTextStream>

BackendAdapter::BackendAdapter(QObject* parent)
    : QObject(parent)
{
    // m_tt = TT_InitTeamTalkPoll();
}

//
// ------------------------------------------------------------
// Connection
// ------------------------------------------------------------
//

void BackendAdapter::connectToServer(const QString& host, int port, const QString& username)
{
    m_state.username = username;

    log(QString("Connecting to %1:%2 as %3")
        .arg(host).arg(port).arg(username));

    TT_Connect(
        m_tt,
        host.toUtf8().constData(),
        port,
        0, 0,
        username.toUtf8().constData(),
        ""
    );
}

void BackendAdapter::disconnectFromServer()
{
    log("Disconnecting from server");
    TT_Disconnect(m_tt);
}

//
// ------------------------------------------------------------
// Channel operations
// ------------------------------------------------------------
//

void BackendAdapter::refreshChannels()
{
    log("Requesting channel list");
    TT_DoChannelList(m_tt);
}

void BackendAdapter::joinChannel(int channelId)
{
    log(QString("Joining channel %1").arg(channelId));
    TT_JoinChannelByID(m_tt, channelId, "");
}

void BackendAdapter::leaveChannel()
{
    log("Leaving channel");
    TT_LeaveChannel(m_tt);
}

//
// ------------------------------------------------------------
// Voice
// ------------------------------------------------------------
//

void BackendAdapter::setTransmitEnabled(bool enabled)
{
    log(QString("Transmit %1").arg(enabled ? "ON" : "OFF"));

    TT_SetTransmissionMode(
        m_tt,
        enabled ? TRANSMIT_AUDIO : TRANSMIT_NONE
    );
}

//
// ------------------------------------------------------------
// Event pump
// ------------------------------------------------------------
//

void BackendAdapter::processEvents()
{
    TTMessage msg;
    while (TT_GetMessage(m_tt, &msg)) {

        switch (msg.nClientEvent) {

        // --------------------------------------------------------
        // Connection state
        // --------------------------------------------------------
        case CLIENTEVENT_CON_CONNECTING:
            emit connectionStateChanged(ConnectionState::Connecting);
            break;

        case CLIENTEVENT_CON_SUCCESS:
            emit connectionStateChanged(ConnectionState::Connected);
            break;

        case CLIENTEVENT_CON_FAILED:
        case CLIENTEVENT_CON_LOST:
            emit connectionStateChanged(ConnectionState::Disconnected);
            break;

        // --------------------------------------------------------
        // Channel list
        // --------------------------------------------------------
        case CLIENTEVENT_CMD_CHANNEL_LIST:
        {
            QList<ChannelInfo> list;

            for (int i = 0; i < msg.channel.nChannels; ++i) {
                const TTChannel* ch = msg.channel.lpChannels[i];

                ChannelInfo info;
                info.id = ch->nChannelID;
                info.parentId = ch->nParentID;
                info.name = QString::fromUtf8(ch->szName);

                list.append(info);
            }

            emit channelsEnumerated(list);
            break;
        }

        // --------------------------------------------------------
        // Join / leave channel
        // --------------------------------------------------------
        case CLIENTEVENT_CMD_MYSELF_JOINED_CHANNEL:
        {
            ChannelEvent ev;
            ev.type = ChannelEventType::Joined;
            ev.channelId = msg.channel.nChannelID;
            emit channelEvent(ev);
            break;
        }

        case CLIENTEVENT_CMD_MYSELF_LEFT_CHANNEL:
        {
            ChannelEvent ev;
            ev.type = ChannelEventType::Left;
            ev.channelId = msg.channel.nChannelID;
            emit channelEvent(ev);
            break;
        }

        // --------------------------------------------------------
        // Errors
        // --------------------------------------------------------
        case CLIENTEVENT_CMD_ERROR:
        {
            ErrorEvent err;
            err.message = mapErrorCode(
                msg.clienterrormsg.nErrorNo,
                QString::fromUtf8(msg.clienterrormsg.szErrorMsg)
            );

            log(QString("Error: %1").arg(err.message));
            emit backendError(err);
            break;
        }

        // --------------------------------------------------------
        // Voice state (self + others)
        // --------------------------------------------------------
        case CLIENTEVENT_USER_STATECHANGE:
        {
            int myId = TT_GetMyUserID(m_tt);

            // Self
            if (msg.user.nUserID == myId) {
                SelfVoiceEvent ev;
                ev.state = (msg.user.uUserState & USERSTATE_VOICE)
                    ? SelfVoiceState::Transmitting
                    : SelfVoiceState::Silent;

                emit selfVoiceEvent(ev);
            }
            // Others
            else {
                OtherUserVoiceEvent ev;
                ev.userId = msg.user.nUserID;
                ev.username = QString::fromUtf8(msg.user.szNickname);
                ev.state = (msg.user.uUserState & USERSTATE_VOICE)
                    ? OtherUserVoiceState::Speaking
                    : OtherUserVoiceState::Silent;

                emit otherUserVoiceEvent(ev);
            }
            break;
        }

        default:
            break;
        }
    }
}

//
// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------
//

QString BackendAdapter::mapErrorCode(int code, const QString& raw)
{
    switch (code) {

    case TT_CMDERR_TIMEOUT:
        return "The server did not respond. Please check your connection.";

    case TT_CMDERR_SERVER_FULL:
        return "The server is full. Try again later.";

    case TT_CMDERR_NOT_LOGGEDIN:
        return "You are not connected to the server.";

    case TT_CMDERR_CHANNEL_NOT_FOUND:
        return "The channel no longer exists.";

    case TT_CMDERR_ALREADY_IN_CHANNEL:
        return "You are already in this channel.";

    case TT_CMDERR_INVALID_USERNAME:
        return "The username is not allowed.";

    default:
        return raw.isEmpty()
            ? "An unknown error occurred."
            : raw;
    }
}

void BackendAdapter::log(const QString& message)
{
    QFile f("backend.log");
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&f);
        out << QDateTime::currentDateTime().toString(Qt::ISODate)
            << " - " << message << "\n";
    }
}
