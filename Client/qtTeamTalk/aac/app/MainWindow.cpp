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

    // Create screens (must happen BEFORE showScreen)
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

    connect(m_stateMachine, &StateMachine::selfVoiceStateChanged,
            m_inChannelScreen, &InChannelScreen::setSelfVoiceState);

    connect(m_stateMachine, &StateMachine::reconnecting,
            this, &MainWindow::onReconnecting);

    connect(m_stateMachine, &StateMachine::reconnectStopped,
            this, &MainWindow::onReconnectStopped);

    connect(m_stateMachine, &StateMachine::notifyUser,
            this, &MainWindow::onNotifyUser);

    // Countdown timer for reconnect (NEW)
    m_reconnectCountdownTimer = new QTimer(this);
    m_reconnectCountdownTimer->setInterval(1000);
    connect(m_reconnectCountdownTimer, &QTimer::timeout,
            this, &MainWindow::updateReconnectCountdown);

// Start on the Connect screen
    showScreen(m_connectScreen);
}

MainWindow::~MainWindow() = default;

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

void MainWindow::onReconnecting(int attempt, int delayMs)
{
    m_reconnectSecondsRemaining = delayMs / 1000;
    m_reconnectCountdownTimer->start();
}

void MainWindow::onReconnectStopped()
{
    m_reconnectCountdownTimer->stop();
}

void MainWindow::onNotifyUser(const QString& message)
{
    // Optional: show message in UI
}

void MainWindow::updateReconnectCountdown()
{
    if (m_reconnectSecondsRemaining > 0) {
        m_reconnectSecondsRemaining--;
    } else {
        m_reconnectCountdownTimer->stop();
    }
}

void MainWindow::onReconnectNowClicked()
{
    m_reconnectCountdownTimer->stop();
    m_stateMachine->stopAutoReconnect();

    m_stateMachine->onConnectRequested(
        m_stateMachine->lastHost(),
        m_stateMachine->lastPort()
    );
}

void MainWindow::updateScreenForChannelState(int channelId)
{
    if (channelId == -1) {
        showScreen(m_channelListScreen);
    } else {
        showScreen(m_inChannelScreen);
    }
