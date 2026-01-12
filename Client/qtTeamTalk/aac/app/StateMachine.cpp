#include "StateMachine.h"
#include "backend/BackendAdapter.h"

StateMachine::StateMachine(QObject* parent)
    : QObject(parent)
{
    m_state = UiConnectionState::Idle;
}

void StateMachine::attachBackend(BackendAdapter* backend)
{
    m_backend = backend;

    if (!m_backend)
        return;

    connect(m_backend, &BackendAdapter::connectionStateChanged,
            this, &StateMachine::onConnectionStateChanged);

    connect(m_backend, &BackendAdapter::channelEvent,
            this, &StateMachine::onChannelEvent);

    connect(m_backend, &BackendAdapter::errorOccurred,
            this, &StateMachine::onBackendError);
}

void StateMachine::connectRequested()
{
    if (!m_backend)
        return;

    if (m_state != UiConnectionState::Idle)
        return;

    m_state = UiConnectionState::Connecting;
    emit uiShouldShowConnecting();

    // TODO: invoke backend connect (e.g. m_backend->connectToServer(...))
}

void StateMachine::disconnectRequested()
{
    if (!m_backend)
        return;

    if (m_state == UiConnectionState::Idle)
        return;

    // TODO: invoke backend disconnect (e.g. m_backend->disconnectFromServer())

    // We optimistically move to Idle; backend events will keep us honest
    m_state = UiConnectionState::Idle;
    emit uiShouldShowDisconnected();
}

void StateMachine::onConnectionStateChanged(ConnectionState state)
{
    switch (state) {
    case ConnectionState::Connected:
        m_state = UiConnectionState::Connected;
        emit uiShouldShowConnected();
        break;

    case ConnectionState::Disconnected:
        m_state = UiConnectionState::Idle;
        emit uiShouldShowDisconnected();
        break;

    case ConnectionState::Connecting:
        m_state = UiConnectionState::Connecting;
        emit uiShouldShowConnecting();
        break;

    default:
        break;
    }
}

void StateMachine::onChannelEvent(const ChannelEvent& event)
{
    Q_UNUSED(event);
    // TODO: handle channel events (mute/transmit state, etc.)
}

void StateMachine::onBackendError(const ErrorEvent& error)
{
    m_state = UiConnectionState::Idle;
    emit uiShouldShowError(error.message);
}

