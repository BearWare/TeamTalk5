#include "MainWindow.h"

#include "aac/backend/BackendAdapter.h"
#include "aac/state/StateMachine.h"

#include "aac/ui/AACScreen.h"
#include "aac/ui/ConnectScreen.h"
#include "aac/ui/ConnectingScreen.h"
#include "aac/ui/ChannelListScreen.h"
#include "aac/ui/InChannelScreen.h"

#include <QTimer>

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
    // Register AAC screens for Large‑Target Mode propagation
    //
    m_aacScreens << m_connectScreen
                 << m_connectingScreen
                 << m_channelListScreen
                 << m_inChannelScreen;

    //
    // Large‑Target Mode → UI
    //
    connect(m_stateMachine, &StateMachine::largeTargetModeChanged,
            this, &MainWindow::onLargeTargetModeChanged);

    //
    // Apply initial mode immediately
    //
    onLargeTargetModeChanged(m_stateMachine->largeTargetModeEnabled());

    //
    // Backend → StateMachine
    //
    connect(m_backend, &BackendAdapter::connectionStateChanged,
            m_stateMachine, &StateMachine::onConnectionStateChanged);

    connect(m_backend, &BackendAdapter::channelsEnumerated,
            m_stateMachine, &StateMachine::onChannelsEnumerated);

    connect(m_backend, &BackendAdapter::channelEvent,
            m_stateMachine, &StateMachine::onChannelEvent);

    connect(m_backend, &BackendAdapter::backendError,
            m_stateMachine, &StateMachine::onBackendError);

    connect(m_backend, &BackendAdapter::selfVoiceEvent,
            m_stateMachine, &StateMachine::onSelfVoiceEvent);

    connect(m_backend, &BackendAdapter::otherUserVoiceEvent,
            m_stateMachine, &StateMachine::onOtherUserVoiceEvent);

    //
    // StateMachine → Backend (actions)
    //
    connect(m_stateMachine, &StateMachine::requestConnect,
            m_backend, &BackendAdapter::connectToServer);

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

    connect(m_stateMachine, &StateMachine::uiShouldShowInChannelScreen,
            this, &MainWindow::showInChannelScreen);

    connect(m_stateMachine, &StateMachine::uiShouldShowDisconnected,
            this, &MainWindow::showConnectScreen);

    connect(m_stateMachine, &StateMachine::uiShouldShowError,
            this, [this](const QString& msg) {
                m_connectScreen->setError(msg);
                showScreen(m_connectScreen);
            });

    //
    // StateMachine → UI (data + in‑channel context)
    //
    connect(m_stateMachine, &StateMachine::channelListChanged,
            m_channelListScreen, &ChannelListScreen::setChannels);

    connect(m_stateMachine, &StateMachine::selfVoiceStateChanged,
            m_inChannelScreen, &InChannelScreen::updateSelfVoiceState);

    connect(m_stateMachine, &StateMachine::otherUserVoiceStateChanged,
            m_inChannelScreen, &InChannelScreen::updateOtherUserVoiceState);

    connect(m_stateMachine, &StateMachine::currentChannelNameChanged,
            m_inChannelScreen, &InChannelScreen::setChannelName);

    connect(m_stateMachine, &StateMachine::inChannelEventMessage,
            m_inChannelScreen, &InChannelScreen::setEventMessage);

    connect(m_stateMachine, &StateMachine::clearInChannelScreen,
            m_inChannelScreen, &InChannelScreen::clearParticipants);

    connect(m_stateMachine, &StateMachine::inChannelError,
            m_inChannelScreen, &InChannelScreen::setEventMessage);

    //
    // UI → StateMachine (user actions)
    //
    connect(m_connectScreen, &ConnectScreen::connectRequested,
            m_stateMachine, &StateMachine::connectRequested);

    connect(m_channelListScreen, &ChannelListScreen::joinChannelRequested,
            m_stateMachine, &StateMachine::onJoinChannelRequested);

    connect(m_inChannelScreen, &InChannelScreen::leaveChannelRequested,
            m_stateMachine, &StateMachine::onLeaveChannelRequested);

    connect(m_inChannelScreen, &InChannelScreen::transmitToggled,
            m_stateMachine, &StateMachine::onTransmitToggled);

    //
    // Event pump timer
    //
    QTimer* pollTimer = new QTimer(this);
    connect(pollTimer, &QTimer::timeout,
            m_backend, &BackendAdapter::processEvents);
    pollTimer->start(10);

    //
    // Start in disconnected state
    //
    showConnectScreen();
}

MainWindow::~MainWindow() = default;

//
// Screen switching helpers
//

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

//
// Large‑Target Mode propagation
//

void MainWindow::onLargeTargetModeChanged(bool enabled)
{
    for (AACScreen* screen : m_aacScreens) {
        if (screen)
            screen->applyLargeTargetMode(enabled);
    }
}
