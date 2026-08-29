#include "StateMachine.h"

StateMachine::StateMachine(QObject* parent)
    : QObject(parent)
{
}

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

void StateMachine::onConnectionStateChanged(ConnectionState state)
{
    m_state.connectionState = state;
    emit connectionStateChanged(state);
}

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
