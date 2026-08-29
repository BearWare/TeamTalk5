#include "StateMachine.h"

StateMachine::StateMachine(QObject* parent)
    : QObject(parent)
{
    // Create reconnect timer
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);

    // When the timer fires, attempt a reconnect
    connect(m_reconnectTimer, &QTimer::timeout,
            this, &StateMachine::attemptReconnect);
}
//
// UI → StateMachine
//

void StateMachine::onConnectRequested(const QString& host, int port)
{
    if (!m_backend)
        return;

    // Update internal state
    m_state.connectionState = ConnectionState::Connecting;
    emit connectionStateChanged(ConnectionState::Connecting);

    // Delegate to backend
    m_backend->connectToServer(host, port);
}

void StateMachine::onRefreshChannelsRequested()
{
    if (!m_backend)
        return;

    m_backend->refreshChannels();
}

void StateMachine::onJoinChannelRequested(int channelId)
{
    if (!m_backend)
        return;

    m_backend->joinChannel(channelId);
}

void StateMachine::onLeaveChannelRequested()
{
    if (!m_backend)
        return;

    m_backend->leaveChannel();
}

void StateMachine::onTransmitToggled(bool on)
{
    if (!m_backend)
        return;

    m_backend->setTransmitEnabled(on);
}

void StateMachine::onChannelsEnumerated(const QList<ChannelInfo>& channels)
{
    // Store in backend state
    m_state.channels = channels;

    // Convert to UI-friendly format
    QList<QPair<int, QString>> uiList;
    for (const auto& ch : channels) {
        uiList.append({ ch.id, ch.name });
    }

    // Notify UI
    emit channelListChanged(uiList);
}

//
// Backend → StateMachine wiring
//

void StateMachine::attachBackend(BackendAdapter* backend)
{
    m_backend = backend;

    connect(backend, &BackendAdapter::channelsEnumerated,
            this, &StateMachine::onChannelsEnumerated);

    connect(backend, &BackendAdapter::connectionStateChanged,
            this, &StateMachine::onConnectionStateChanged);

    connect(backend, &BackendAdapter::channelEvent,
            this, &StateMachine::onChannelEvent);

    connect(backend, &BackendAdapter::errorOccurred,
            this, &StateMachine::onErrorOccurred);

    connect(backend, &BackendAdapter::selfVoiceEvent,
            this, &StateMachine::onSelfVoiceEvent);

    connect(backend, &BackendAdapter::audioDeviceEvent,
            this, &StateMachine::onAudioDeviceEvent);

    connect(backend, &BackendAdapter::textMessageEvent,
            this, &StateMachine::onTextMessageEvent);
}
//
// Backend → UI translation
//

void StateMachine::onConnectionStateChanged(ConnectionState state)
{
    // Update internal state
    m_state.connectionState = state;
    emit connectionStateChanged(state);

    //
    // Connected → stop reconnecting, refresh channels
    //
    if (state == ConnectionState::Connected) {

        // Stop any pending reconnect attempts
        m_reconnectAttempts = 0;
        if (m_reconnectTimer->isActive())
            m_reconnectTimer->stop();

        // Refresh channels
        if (m_backend)
            m_backend->refreshChannels();

        emit notifyUser(QStringLiteral("Connected to server"));
        return;
    }

    //
    // Disconnected → clear state, notify, start reconnect logic
    //
    if (state == ConnectionState::Disconnected) {

        // Clear channel list
        m_state.channels.clear();
        emit channelListChanged({});

        // Reset channel ID
        m_state.currentChannelId = -1;
        emit channelChanged(-1);

        emit notifyUser(QStringLiteral("Disconnected from server"));

        // Only reconnect if we have connection details
        if (m_lastHost.isEmpty() || m_lastPort == 0)
            return;

        // Attempt reconnect with exponential backoff
        if (m_reconnectAttempts < m_maxReconnectAttempts) {

            // Exponential backoff: 3s, 6s, 12s, 24s, capped at 30s
            int delay = qMin(30000, 3000 * (1 << m_reconnectAttempts));

            emit reconnecting(m_reconnectAttempts + 1, delay);

            m_reconnectTimer->start(delay);
            m_reconnectAttempts++;

        } else {
            emit notifyUser(QStringLiteral("Reconnect attempts stopped"));
            emit reconnectStopped();
        }

        return;
    }

    //
    // Connecting → nothing special
    //
}


void StateMachine::onChannelEvent(const ChannelEvent& event)
{
    if (event.type == ChannelEventType::Joined) {
        m_state.currentChannelId = event.channelId;
        emit channelChanged(event.channelId);
    } else if (event.type == ChannelEventType::Left) {
        m_state.currentChannelId = -1;
        emit channelChanged(-1);
    }
}

void StateMachine::onErrorOccurred(const ErrorEvent& error)
{
    emit errorOccurred(error.message);
}

void StateMachine::onSelfVoiceEvent(const SelfVoiceEvent& event)
{
    emit selfVoiceStateChanged(event.state);
}

void StateMachine::onAudioDeviceEvent(const AudioDeviceEvent& event)
{
    emit audioDeviceStateChanged(event.type);
}

void StateMachine::onTextMessageEvent(const TextMessageEvent& event)
{
    emit incomingTextMessage(event.fromUserId, event.message);
}

void StateMachine::attemptReconnect()
{
    if (!m_backend)
        return;

    emit notifyUser(QStringLiteral("Reconnecting…"));
    m_backend->connectToServer(m_lastHost, m_lastPort);
}
void StateMachine::stopAutoReconnect()
{
    if (m_reconnectTimer->isActive())
        m_reconnectTimer->stop();

    m_reconnectAttempts = 0;

    emit notifyUser(QStringLiteral("Reconnect cancelled"));
    emit reconnectStopped();
}
