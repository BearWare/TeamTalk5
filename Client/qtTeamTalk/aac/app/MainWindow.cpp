#include "MainWindow.h"
#include "backend/BackendAdapter.h"
#include "backend/StateMachine.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Backend + StateMachine
    m_backend = new BackendAdapter(this);
    m_stateMachine = new StateMachine(this);
    m_stateMachine->attachBackend(m_backend);

    // Screens
    m_connectScreen = new ConnectScreen(this);
    m_connectingScreen = new ConnectingScreen(this);
    m_channelListScreen = new ChannelListScreen(this);
    m_inChannelScreen = new InChannelScreen(this);

    // ------------------------------------------------------------
    // CONNECT ARC
    // ------------------------------------------------------------
    connect(m_connectScreen, &ConnectScreen::connectRequested,
            m_stateMachine, &StateMachine::connectRequested);

    connect(m_connectScreen, &ConnectScreen::connectToServer,
            m_backend, &BackendAdapter::connectToServer);

    connect(m_stateMachine, &StateMachine::uiShouldShowConnecting,
            this, &MainWindow::showConnectingScreen);

    connect(m_stateMachine, &StateMachine::uiShouldShowConnected,
            this, &MainWindow::showChannelListScreen);

    connect(m_stateMachine, &StateMachine::uiShouldShowDisconnected,
            this, &MainWindow::showConnectScreen);

    // ------------------------------------------------------------
    // CONNECTING ARC
    // ------------------------------------------------------------
    connect(m_connectingScreen, &ConnectingScreen::cancelRequested,
            m_stateMachine, &StateMachine::disconnectRequested);

    // ------------------------------------------------------------
    // CHANNEL LIST ARC
    // ------------------------------------------------------------
    connect(m_channelListScreen, &ChannelListScreen::refreshRequested,
            m_stateMachine, &StateMachine::onRefreshChannelsRequested);

    connect(m_stateMachine, &StateMachine::channelListChanged,
            m_channelListScreen, &ChannelListScreen::setChannels);

    connect(m_channelListScreen, &ChannelListScreen::joinChannelRequested,
            m_stateMachine, &StateMachine::onJoinChannelRequested);

    // ------------------------------------------------------------
    // IN-CHANNEL ARC
    // ------------------------------------------------------------
    connect(m_inChannelScreen, &InChannelScreen::transmitToggled,
            m_stateMachine, &StateMachine::onTransmitToggled);

    connect(m_inChannelScreen, &InChannelScreen::leaveRequested,
            m_stateMachine, &StateMachine::onLeaveChannelRequested);

    connect(m_stateMachine, &StateMachine::selfVoiceStateChanged,
            m_inChannelScreen, &InChannelScreen::setSelfVoiceState);

    // ------------------------------------------------------------
    // INITIAL SCREEN
    // ------------------------------------------------------------
    showScreen(m_connectScreen);
}
