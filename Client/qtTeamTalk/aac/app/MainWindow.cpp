#include "MainWindow.h"

#include "aac/backend/BackendAdapter.h"
#include "aac/state/StateMachine.h"

#include "aac/aac/AACFramework.h"

#include "aac/ui/ConnectScreen.h"
#include "aac/ui/ConnectingScreen.h"
#include "aac/ui/ChannelListScreen.h"
#include "aac/ui/InChannelScreen.h"

#include <QTimer>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    //
    // AAC framework
    //
    m_aac = new AACAccessibilityManager(this);

    AACModeFlags modes;
    modes.largeTargets = true;        // keep your Large-Target baseline
    modes.dwell = true;               // enable dwell
    modes.auditoryFeedback = true;    // ready for sounds
    // other modes can be toggled later
    m_aac->setModes(modes);

    //
    // Backend + state machine
    //
    m_backend = new BackendAdapter(this);
    m_stateMachine = new StateMachine(this);
    m_stateMachine->attachBackend(m_backend);

    //
    // Screens (now AAC-aware)
    //
    m_connectScreen      = new ConnectScreen(m_aac, this);
    m_connectingScreen   = new ConnectingScreen(m_aac, this);
    m_channelListScreen  = new ChannelListScreen(m_aac, this);
    m_inChannelScreen    = new InChannelScreen(m_aac, this);

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
    // StateMachine → Backend
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
    // StateMachine → UI
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
    // UI → StateMachine
    //
    connect(m_connectScreen, &ConnectScreen::connectRequested,
            m_stateMachine, &StateMachine::connectRequested);

    connect(m_channelListScreen, &ChannelListScreen::joinChannelRequested,
            m_stateMachine, &StateMachine::onJoinChannelRequested);

    connect(m_channelListScreen, &ChannelListScreen::refreshRequested,
            m_stateMachine, &StateMachine::onRefreshChannelsRequested);

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

void MainWindow::keyPressEvent(QKeyEvent* e)
{
    if (!m_aac) {
        QMainWindow::keyPressEvent(e);
        return;
    }

    if (e->key() == Qt::Key_Space) {
        m_aac->inputController()->onSwitchActivate();
        e->accept();
        return;
    }

    if (e->key() == Qt::Key_Tab) {
        m_aac->inputController()->onSwitchNext();
        e->accept();
        return;
    }

    QMainWindow::keyPressEvent(e);
}
