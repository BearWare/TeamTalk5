#include "StateMachine.h"

StateMachine::StateMachine(QObject* parent)
    : QObject(parent)
{
}

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
void StateMachine::attachBackend(BackendAdapter* backend)
{
    m_backend = backend;

    //
    // Connection state
    //
    connect(backend, &BackendAdapter::connectionStateChanged,
            this, &StateMachine::onConnectionStateChanged);

    //
    // Channel join/leave
    //
    connect(backend, &BackendAdapter::channelEvent,
            this, &StateMachine::onChannelEvent);

    //
    // Errors
    //
    connect(backend, &BackendAdapter::errorOccurred,
            this, &StateMachine::onErrorOccurred);

    //
    // AAC‑relevant additions
    //

    // Self voice indicator (Talking / Silent)
    connect(backend, &BackendAdapter::selfVoiceEvent,
            this, &StateMachine::onSelfVoiceEvent);

    // Audio device events (Added / Removed / Failed)
    connect(backend, &BackendAdapter::audioDeviceEvent,
            this, &StateMachine::onAudioDeviceEvent);

    // Incoming text messages (optional TTS)
    connect(backend, &BackendAdapter::textMessageEvent,
            this, &StateMachine::onTextMessageEvent);
}

//
// Connection state
//
void StateMachine::onConnectionStateChanged(ConnectionState state)
{
    m_state.connectionState = state;
    emit connectionStateChanged(state);

    if (state == ConnectionState::Connected) {
        // Automatically refresh channels when connected
        if (m_backend)
            m_backend->requestChannelList();
    }
}

//
// Channel join/leave
//
void StateMachine::onChannelEvent(const ChannelEvent& event)
{
    if (event.type == ChannelEventType::Joined) {
        m_state.currentChannelId = event.channelId;
        emit channelChanged(event.channelId);
    }
    else if (event.type == ChannelEventType::Left) {
        m_state.currentChannelId = -1;
        emit channelChanged(-1);
    }
}

//
// Errors
//
void StateMachine::onErrorOccurred(const ErrorEvent& error)
{
    emit errorOccurred(error.message);
}

//
// Self voice state (Talking / Silent)
//
void StateMachine::onSelfVoiceEvent(const SelfVoiceEvent& event)
{
    emit selfVoiceStateChanged(event.state);
}

//
// Audio device events (Added / Removed / Failed)
//
void StateMachine::onAudioDeviceEvent(const AudioDeviceEvent& event)
{
    emit audioDeviceStateChanged(event.type);
}

//
// Incoming text messages (optional TTS)
//
void StateMachine::onTextMessageEvent(const TextMessageEvent& event)
{
    emit incomingTextMessage(event.fromUserId, event.message);
}
