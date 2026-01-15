#include "MainWindow.h"

#include "aac/backend/BackendAdapter.h"
#include "aac/state/StateMachine.h"

#include "aac/ui/ConnectScreen.h"
#include "aac/ui/ConnectingScreen.h"
#include "aac/ui/ChannelListScreen.h"
#include "aac/ui/InChannelScreen.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    //
    // Create backend + state machine
    //
    m_backend = new BackendAdapter(this);
    m_stateMachine = new StateMachine(this);
    m_stateMachine->attachBackend(m_backend);

    //
    // Create screens
    //
    m_connectScreen      = new ConnectScreen(this);
    m_connectingScreen   = new ConnectingScreen(this);
    m_channelListScreen  = new ChannelListScreen(this);
    m_inChannelScreen    = new InChannelScreen(this);

    //
    // Backend → StateMachine
    //
    connect(m_backend, &BackendAdapter::connectionStateChanged,
            m_stateMachine, &StateMachine::onConnectionStateChanged);

    connect(m_backend, &BackendAdapter::channelsEnumerated,
            m_stateMachine, &StateMachine::onChannelsEnumerated);

    connect(m_backend, &BackendAdapter::channelEvent,
            m_stateMachine, &StateMachine::onChannelEvent);

    connect(m_backend, &BackendAdapter::errorOccurred,
            m_stateMachine, &StateMachine::onBackendError);

    connect(m_backend, &BackendAdapter::selfVoiceEvent,
            m_stateMachine, &StateMachine::onSelfVoiceEvent);

    //
    // StateMachine → Backend (actions)
    //
    connect(m_stateMachine, &StateMachine::connectRequested,
            m_backend, [this]() {
                // TODO: pull host/port from ConnectScreen
                m_backend->connectToServer("localhost", 10333);
            });

    connect(m_stateMachine, &StateMachine::disconnectRequested,
            m_backend, &BackendAdapter::disconnectFromServer);

    connect(m_stateMachine, &StateMachine::onRefreshChannelsRequested,
            m_backend, &BackendAdapter::refreshChannels);

    connect(m_stateMachine, &StateMachine::onJoinChannelRequested,
            m_backend, &BackendAdapter::joinChannel);

    connect(m_stateMachine, &StateMachine::onLeaveChannelRequested,
            m_backend, &BackendAdapter::leaveChannel);

    connect(m_stateMachine, &StateMachine::onTransmitToggled,
            m_backend, &BackendAdapter::setTransmitEnabled);

    //
    // StateMachine → UI (screen switching)
    //
    connect(m_stateMachine, &StateMachine::uiShouldShowConnecting,
            this, &MainWindow::showConnectingScreen);

    connect(m_stateMachine, &StateMachine::uiShouldShowConnected,
            this, &MainWindow::showChannelListScreen);

    connect(m_stateMachine, &StateMachine::uiShouldShowDisconnected,
            this, &MainWindow::showConnectScreen);

    connect(m_stateMachine, &StateMachine::uiShouldShowError,
            this, [this](const QString& msg) {
                m_connectScreen->setError(msg);
                showScreen(m_connectScreen);
            });

    connect(m_stateMachine, &StateMachine::channelListChanged,
            m_channelListScreen, &ChannelListScreen::setChannels);

    connect(m_stateMachine, &StateMachine::selfVoiceStateChanged,
            m_inChannelScreen, &InChannelScreen::updateSelfVoiceState);

    //
    // UI → StateMachine (user actions)
    //
    connect(m_connectScreen, &ConnectScreen::connectRequested,
            m_stateMachine, &StateMachine::connectRequested);

    connect(m_channelListScreen, &ChannelListScreen::joinChannelRequested,
            m_stateMachine, &StateMachine::onJoinChannelRequested);

    connect(m_inChannelScreen, &InChannelScreen::leaveChannelRequested,
            m_stateMachine, &StateMachine::onLeaveChannelRequested);

    //
    // Start in disconnected state
    //
    showConnectScreen();
}

MainWindow::~MainWindow() = default;

void MainWindow::showScreen(QWidget* screen)
{
    if (!screen)
        return;

    setCentralWidget(screen);
}

void MainWindow::showConnectScreen()
{
    showScreen(m_connectScreen);
}

void MainWindow::showConnectingScreen()
{
    showScreen(m_connectingScreen);
}

void MainWindow::showChannelListScreen()
{
    showScreen(m_channelListScreen);
}

void MainWindow::showInChannelScreen()
{
    showScreen(m_inChannelScreen);
}
