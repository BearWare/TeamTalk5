#include "StateMachine.h"

#include "backend/BackendAdapter.h"
#include <QDebug>

StateMachine::StateMachine(QObject* parent)
    : QObject(parent)
{
}

void StateMachine::attachBackend(BackendAdapter* backend)
{
    if (m_backend == backend)
        return;

    m_backend = backend;

    if (!m_backend)
        return;

    // Backend → StateMachine
    connect(m_backend, &BackendAdapter::connectionStateChanged,
            this, &StateMachine::onConnectionStateChanged);

    connect(m_backend, &BackendAdapter::channelEvent,
            this, &StateMachine::onChannelEvent);

    connect(m_backend, &BackendAdapter::errorOccurred,
            this, &StateMachine::onBackendError);

    connect(m_backend, &BackendAdapter::channelsEnumerated,
            this, &StateMachine::onChannelsEnumerated);

    connect(m_backend, &BackendAdapter::selfVoiceEvent,
            this, &StateMachine::onSelfVoiceEvent);
}

// ---------------------------------------------------------------------
// User-intent entry points
// ---------------------------------------------------------------------

void StateMachine::connectRequested()
{
    if (!m_backend)
        return;

    if (m_state == UiConnectionState::Connecting ||
        m_state == UiConnectionState::Connected) {
        return;
    }

    m_state = UiConnectionState::Connecting;
    emit uiShouldShowConnecting();

    // The actual host/port selection is handled elsewhere (e.g. MainWindow).
    // This state machine only drives UI state and reacts to backend events.
}

void StateMachine::disconnectRequested()
{
    if (!m_backend)
        return;

    // Ask backend to disconnect; UI will be updated when we see
    // ConnectionState::Disconnected from the backend.
    m_backend->disconnectFromServer();
}

void StateMachine::onRefreshChannelsRequested()
{
    if (!m_backend)
        return;

    if (m_state != UiConnectionState::Connected)
        return;

    m_backend->refreshChannels();
}

void StateMachine::onJoinChannelRequested(int channelId)
{
    if (!m_backend)
        return;

    if (m_state != UiConnectionState::Connected)
        return;

    m_backend->joinChannel(channelId);
}

void StateMachine::onLeaveChannelRequested()
{
    if (!m_backend)
        return;

    m_backend->leaveChannel();
}

void StateMachine::onTransmitToggled(bool enabled)
{
    if (!m_backend)
        return;

    m_backend->setTransmitEnabled(enabled);
}

// ---------------------------------------------------------------------
// Backend → StateMachine
// ---------------------------------------------------------------------

void StateMachine::onChannelsEnumerated(const QList<ChannelInfo>& channels)
{
    m_channels = channels;
    emit channelListChanged(channels);
}

void StateMachine::onConnectionStateChanged(ConnectionState state)
{
    switch (state) {
    case ConnectionState::Connected:
        m_state = UiConnectionState::Connected;
        emit uiShouldShowConnected();
        break;

    case ConnectionState::Disconnected:
    default:
        m_state = UiConnectionState::Idle;
        emit uiShouldShowDisconnected();
        break;
    }
}

void StateMachine::onChannelEvent(const ChannelEvent& event)
{
    switch (event.type) {
    case ChannelEventType::Joined:
        // We’re in a channel: ensure UI is in the “connected” view.
        emit uiShouldShowConnected();
        break;

    case ChannelEventType::Left:
        // Left the channel; still connected to server, but UI may want
        // to show the channel list again.
        emit uiShouldShowConnected();
        break;

    default:
        break;
    }
}

void StateMachine::onBackendError(const ErrorEvent& error)
{
    emit uiShouldShowError(error.message);
}

void StateMachine::onSelfVoiceEvent(const SelfVoiceEvent& event)
{
    emit selfVoiceStateChanged(event.state);
}
