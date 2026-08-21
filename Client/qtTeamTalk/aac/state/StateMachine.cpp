#include "StateMachine.h"
#include "BackendAdapter.h"
#include "MainWindow.h"

StateMachine::StateMachine(QObject* parent)
    : QObject(parent)
{
}

void StateMachine::attachBackend(BackendAdapter* backend)
{
    m_backend = backend;

    // Backend → StateMachine
    connect(backend, &BackendAdapter::connectionStateChanged,
            this, &StateMachine::onConnectionStateChanged);

    connect(backend, &BackendAdapter::channelEvent,
            this, &StateMachine::onChannelEvent);

    connect(backend, &BackendAdapter::errorOccurred,
            this, &StateMachine::onErrorOccurred);
}

void StateMachine::attachMainWindow(MainWindow* window)
{
    m_window = window;

    // StateMachine → UI
    connect(this, &StateMachine::uiShouldShowConnecting,
            window, &MainWindow::showConnecting);

    connect(this, &StateMachine::uiShouldShowConnected,
            window, &MainWindow::showConnected);

    connect(this, &StateMachine::uiShouldShowDisconnected,
            window, &MainWindow::showDisconnected);

    connect(this, &StateMachine::uiShouldShowError,
            window, &MainWindow::showErrorMessage);
}

void StateMachine::onConnectionStateChanged(ConnectionState state)
{
    m_connectionState = state;
    updateUI();
}

void StateMachine::onChannelEvent(const ChannelEvent& event)
{
    // For PR#9, we only track the current channel ID.
    if (event.type == ChannelEventType::Joined)
        m_currentChannelId = event.channelId;
    else if (event.type == ChannelEventType::Left)
        m_currentChannelId = -1;

    updateUI();
}

void StateMachine::onErrorOccurred(const ErrorEvent& error)
{
    emit uiShouldShowError(error.message);
}

void StateMachine::updateUI()
{
    switch (m_connectionState)
    {
    case ConnectionState::Connecting:
        emit uiShouldShowConnecting();
        break;

    case ConnectionState::Connected:
        emit uiShouldShowConnected();
        break;

    case ConnectionState::Disconnected:
        emit uiShouldShowDisconnected();
        break;

    case ConnectionState::Error:
        emit uiShouldShowError(QStringLiteral("Connection error"));
        break;
    }
}
