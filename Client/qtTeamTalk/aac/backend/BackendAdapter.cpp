#include "BackendAdapter.h"

BackendAdapter::BackendAdapter(QObject* parent)
    : QObject(parent)
{
    // Initialize TeamTalk instance in polling mode
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
    m_pollTimer->start(20); // 20 ms polling interval; adjust if needed
}

void BackendAdapter::connectToServer(const QString& host, int port)
{
    if (!m_tt)
        return;

    // Optional: update your BackendState here
    // m_state.connectionState = ConnectionState::Connecting;
    // emit connectionStateChanged(ConnectionState::Connecting);

    // Minimal connect: adjust parameters if you later add username/auth/etc.
    const QByteArray hostUtf8 = host.toUtf8();

    INT32 cmdid = TT_Connect(m_tt,
                             hostUtf8.constData(),
                             port,
                             0,  // udpport
                             0,  // encrypted
                             0,  // timeout
                             0); // bind IP (0 = default)

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

    // Empty password for now; you can extend BackendAdapter to accept one later.
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

    // Voice stream only for now; you can extend this if you need more.
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

        case CLIENTEVENT_CON_SUCCESS: {
            // Map to your ConnectionState enum
            connectionStateChanged(ConnectionState::Connected);
            break;
        }

        case CLIENTEVENT_CON_FAILED: {
            ErrorEvent err;
            err.message = QStringLiteral("Connection failed");
            emit errorOccurred(err);
            // If you have a Disconnected state, you may want to emit it here.
            break;
        }

        case CLIENTEVENT_CON_LOST: {
            connectionStateChanged(ConnectionState::Disconnected);
            break;
        }

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

        case CLIENTEVENT_CMD_USER_JOINED: {
            ChannelEvent ev;
            // These fields depend on how you defined ChannelEvent in BackendEvents.h.
            // Example mapping:
            // ev.type = ChannelEventType::UserJoined;
            // ev.userId = msg.user.nUserID;
            // ev.channelId = msg.user.nChannelID;
            emit channelEvent(ev);
            break;
        }

        case CLIENTEVENT_CMD_USER_LEFT: {
            ChannelEvent ev;
            // Example mapping:
            // ev.type = ChannelEventType::UserLeft;
            // ev.userId = msg.user.nUserID;
            // ev.channelId = msg.user.nChannelID;
            emit channelEvent(ev);
            break;
        }

        default:
            // You can expand this switch with more events over time.
            break;
        }
    }
}
