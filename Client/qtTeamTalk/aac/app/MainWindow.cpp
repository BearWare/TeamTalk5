#include "MainWindow.h"
#include "backend/BackendAdapter.h"
#include "backend/StateMachine.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Create backend + state machine
    m_backend = new BackendAdapter(this);
    m_stateMachine = new StateMachine(this);
showScreen(m_connectScreen);

m_connectScreen = new ConnectScreen(this);
m_connectingScreen = new ConnectingScreen(this);
m_channelListScreen = new ChannelListScreen(this);
m_inChannelScreen = new InChannelScreen(this);
    // Wire backend → state machine
    m_stateMachine->attachBackend(m_backend);

    // Wire state machine → UI
connect(m_stateMachine, &StateMachine::connectionStateChanged,
        this, &MainWindow::updateScreenForConnectionState);

connect(m_stateMachine, &StateMachine::channelChanged,
        this, &MainWindow::updateScreenForChannelState);

    connect(m_stateMachine, &StateMachine::errorOccurred,
            this, &MainWindow::onBackendError);

// UI → state machine wiring

connect(m_connectScreen, &ConnectScreen::connectRequested,
        m_stateMachine, &StateMachine::onConnectRequested);

connect(m_channelListScreen, &ChannelListScreen::refreshRequested,
        m_stateMachine, &StateMachine::onRefreshChannelsRequested);

connect(m_channelListScreen, &ChannelListScreen::joinChannelRequested,
        m_stateMachine, &StateMachine::onJoinChannelRequested);

connect(m_inChannelScreen, &InChannelScreen::leaveRequested,
        m_stateMachine, &StateMachine::onLeaveChannelRequested);

connect(m_inChannelScreen, &InChannelScreen::transmitToggled,
        m_stateMachine, &StateMachine::onTransmitToggled);
}

MainWindow::~MainWindow() = default;

void MainWindow::onConnectionStateChanged(ConnectionState state)
{
    updateScreenForConnectionState(state);
}

void MainWindow::onChannelEvent(const ChannelEvent& event)
{
    updateScreenForChannelState(event.channelId);
}

void MainWindow::onBackendError(const ErrorEvent& error)
{
    QMessageBox::warning(this, tr("Error"), error.message);
}
void MainWindow::showScreen(QWidget* screen)
{
    setCentralWidget(screen);
    screen->show();
}

void MainWindow::updateScreenForConnectionState(ConnectionState state)
{
    switch (state) {
    case ConnectionState::Disconnected:
        showScreen(m_connectScreen);
        break;

    case ConnectionState::Connecting:
        showScreen(m_connectingScreen);
        break;

    case ConnectionState::Connected:
        showScreen(m_channelListScreen);
        break;
    }
}

void MainWindow::updateScreenForChannelState(int channelId)
{
    if (channelId == -1) {
        // Not in a channel → show channel list
        showScreen(m_channelListScreen);
    } else {
        // In a channel → show in-channel screen
        showScreen(m_inChannelScreen);
    }
}
