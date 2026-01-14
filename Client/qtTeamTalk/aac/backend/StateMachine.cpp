#include "StateMachine.h"

StateMachine::StateMachine(QObject* parent)
    : QObject(parent)
{
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

    m_backend->requestChannelList();
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

//
// Backend → StateMachine wiring
//

void StateMachine::attachBackend(BackendAdapter* backend)
{
    m_backend = backend;

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
    m_state.connectionState = state;
    emit connectionStateChanged(state);

    if (state == ConnectionState::Connected && m_backend)
        m_backend->requestChannelList();
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
