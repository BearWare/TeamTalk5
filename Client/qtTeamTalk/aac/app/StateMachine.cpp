#include "StateMachine.h"
#include "backend/BackendAdapter.h"

StateMachine::StateMachine(QObject* parent)
    : QObject(parent)
{
}

void StateMachine::attachBackend(BackendAdapter* backend)
{
    m_backend = backend;

    connect(backend, &BackendAdapter::channelsEnumerated,
            this, &StateMachine::onChannelsEnumerated);

    connect(backend, &BackendAdapter::connectionStateChanged,
            this, &StateMachine::onConnectionStateChanged);

    connect(backend, &BackendAdapter::channelEvent,
            this, &StateMachine::onChannelEvent);

    connect(backend, &BackendAdapter::backendError,
            this, &StateMachine::onBackendError);

    connect(backend, &BackendAdapter::selfVoiceEvent,
            this, &StateMachine::onSelfVoiceEvent);

    connect(backend, &BackendAdapter::otherUserVoiceEvent,
            this, &StateMachine::onOtherUserVoiceEvent);
}

//
// User → StateMachine
//

void StateMachine::connectRequested(const QString& host, int port, const QString& username)
{
    m_state = UiConnectionState::Connecting;
    emit uiShouldShowConnecting();
    emit requestConnect(host, port, username);
}

void StateMachine::disconnectRequested()
{
    m_state = UiConnectionState::Idle;
    m_currentChannelId = -1;
    emit uiShouldShowDisconnected();

    if (m_backend)
        m_backend->disconnectFromServer();
}

void StateMachine::onRefreshChannelsRequested()
{
    if (m_backend)
        m_backend->refreshChannels();
}

void StateMachine::onJoinChannelRequested(int channelId)
{
    if (m_backend)
        m_backend->joinChannel(channelId);
}

void StateMachine::onLeaveChannelRequested()
{
    if (m_backend)
        m_backend->leaveChannel();
}

void StateMachine::onTransmitToggled(bool enabled)
{
    if (m_backend)
        m_backend->setTransmitEnabled(enabled);
}

//
// Backend → StateMachine
//

void StateMachine::onConnectionStateChanged(ConnectionState state)
{
    switch (state) {
    case ConnectionState::Connecting:
        m_state = UiConnectionState::Connecting;
        emit uiShouldShowConnecting();
        break;

    case ConnectionState::Connected:
        m_state = UiConnectionState::Connected;
        emit uiShouldShowConnected();
        break;

    case ConnectionState::Disconnected:
        m_state = UiConnectionState::Idle;
        m_currentChannelId = -1;
        emit uiShouldShowDisconnected();
        break;
    }
}

void StateMachine::onChannelsEnumerated(const QList<ChannelInfo>& channels)
{
    m_channels = channels;
    emit channelListChanged(channels);
}

void StateMachine::onChannelEvent(const ChannelEvent& event)
{
    switch (event.type) {
    case ChannelEventType::Joined: {
        m_currentChannelId = event.channelId;

        QString channelName = QObject::tr("Channel");
        for (const auto& ch : m_channels) {
            if (ch.id == event.channelId) {
                channelName = ch.name;
                break;
            }
        }

        emit currentChannelNameChanged(channelName);
        emit clearInChannelScreen();
        emit inChannelEventMessage(QObject::tr("You joined: %1").arg(channelName));
        emit uiShouldShowInChannelScreen();
        break;
    }

    case ChannelEventType::Left:
        m_currentChannelId = -1;
        emit inChannelEventMessage(QObject::tr("You left the channel"));
        emit clearInChannelScreen();
        emit uiShouldShowConnected();
        break;
    }
}

void StateMachine::onBackendError(const ErrorEvent& error)
{
    if (m_state == UiConnectionState::Connected && m_currentChannelId != -1) {
        emit inChannelError(error.message);
    } else {
        emit uiShouldShowError(error.message);
    }
}

void StateMachine::onSelfVoiceEvent(const SelfVoiceEvent& event)
{
    emit selfVoiceStateChanged(event.state);
}

void StateMachine::onOtherUserVoiceEvent(const OtherUserVoiceEvent& event)
{
    emit otherUserVoiceStateChanged(event);
}
