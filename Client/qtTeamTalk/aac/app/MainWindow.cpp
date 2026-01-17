#include "MainWindow.h"

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QDebug>

#include "aac/AACAccessibilityManager.h"
#include "aac/AACLayoutEngine.h"

#include "backend/BackendAdapter.h"
#include "backend/StateMachine.h"

#include "screens/ConnectScreen.h"
#include "screens/ChannelListScreen.h"
#include "screens/InChannelScreen.h"
#include "screens/ConnectingScreen.h"
#include "screens/AppSettingsScreen.h"
#include "screens/AACSettingsScreen.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    m_aac = new AACAccessibilityManager(this);
    m_backend = new BackendAdapter(this);
    m_state = new StateMachine(this);

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    auto* stack = new QStackedWidget(central);
    layout->addWidget(stack);
    setCentralWidget(central);

    m_connectScreen = new ConnectScreen(m_aac, this);
    m_channelListScreen = new ChannelListScreen(m_aac, this);
    m_inChannelScreen = new InChannelScreen(m_aac, this);
    m_connectingScreen = new ConnectingScreen(m_aac, this);
    m_appSettingsScreen = new AppSettingsScreen(m_aac, this);
    m_aacSettingsScreen = new AACSettingsScreen(m_aac, this);

    stack->addWidget(m_connectScreen);
    stack->addWidget(m_channelListScreen);
    stack->addWidget(m_inChannelScreen);
    stack->addWidget(m_connectingScreen);
    stack->addWidget(m_appSettingsScreen);
    stack->addWidget(m_aacSettingsScreen);

    wireScreens();
    applyAACDefaults();

    showConnectScreen();
}

MainWindow::~MainWindow() {}

void MainWindow::wireScreens()
{
    connect(m_connectScreen, &ConnectScreen::connectRequested,
            this, &MainWindow::onConnectRequested);

    connect(m_connectScreen, &ConnectScreen::settingsRequested,
            this, &MainWindow::showAppSettingsScreen);

    connect(m_connectScreen, &ConnectScreen::aacSettingsRequested,
            this, &MainWindow::showAACSettingsScreen);

    connect(m_channelListScreen, &ChannelListScreen::refreshRequested,
            this, &MainWindow::onRefreshRequested);

    connect(m_channelListScreen, &ChannelListScreen::joinChannelRequested,
            this, &MainWindow::onJoinChannelRequested);

    connect(m_channelListScreen, &ChannelListScreen::settingsRequested,
            this, &MainWindow::showAppSettingsScreen);

    connect(m_channelListScreen, &ChannelListScreen::aacSettingsRequested,
            this, &MainWindow::showAACSettingsScreen);

    connect(m_inChannelScreen, &InChannelScreen::leaveChannelRequested,
            this, &MainWindow::onLeaveChannelRequested);

    connect(m_inChannelScreen, &InChannelScreen::transmitToggled,
            this, &MainWindow::onTransmitToggled);

    connect(m_inChannelScreen, &InChannelScreen::settingsRequested,
            this, &MainWindow::showAACSettingsScreen);

    connect(m_connectingScreen, &ConnectingScreen::cancelRequested,
            this, &MainWindow::onCancelConnectRequested);

    connect(m_backend, &BackendAdapter::connected,
            this, &MainWindow::onConnected);

    connect(m_backend, &BackendAdapter::connectionFailed,
            this, &MainWindow::onConnectionFailed);

    connect(m_backend, &BackendAdapter::disconnected,
            this, &MainWindow::onDisconnected);

    connect(m_backend, &BackendAdapter::channelsUpdated,
            this, &MainWindow::onChannelsUpdated);

    connect(m_backend, &BackendAdapter::joinedChannel,
            this, &MainWindow::onJoinedChannel);

    connect(m_backend, &BackendAdapter::leftChannel,
            this, &MainWindow::onLeftChannel);

    connect(m_backend, &BackendAdapter::selfVoiceState,
            this, &MainWindow::onSelfVoiceState);

    connect(m_backend, &BackendAdapter::otherUserVoiceState,
            this, &MainWindow::onOtherUserVoiceState);

    connect(m_backend, &BackendAdapter::eventMessage,
            this, &MainWindow::onEventMessage);
}

void MainWindow::applyAACDefaults()
{
    AACModeFlags modes;
    modes.largeTargets = true;
    modes.auditoryFeedback = true;
    modes.predictiveStrip = true;

    m_aac->setModes(modes);

    AACLayoutConfig cfg;
    cfg.oneHandRightSide = true;
    m_aac->setLayoutConfig(cfg);
}

void MainWindow::showConnectScreen()
{
    centralWidget()->findChild<QStackedWidget*>()->setCurrentWidget(m_connectScreen);
}

void MainWindow::showChannelListScreen()
{
    centralWidget()->findChild<QStackedWidget*>()->setCurrentWidget(m_channelListScreen);
}

void MainWindow::showInChannelScreen(int channelId, const QString& channelName)
{
    m_inChannelScreen->setChannelName(channelName);
    centralWidget()->findChild<QStackedWidget*>()->setCurrentWidget(m_inChannelScreen);
}

void MainWindow::showConnectingScreen()
{
    centralWidget()->findChild<QStackedWidget*>()->setCurrentWidget(m_connectingScreen);
}

void MainWindow::showAppSettingsScreen()
{
    centralWidget()->findChild<QStackedWidget*>()->setCurrentWidget(m_appSettingsScreen);
}

void MainWindow::showAACSettingsScreen()
{
    centralWidget()->findChild<QStackedWidget*>()->setCurrentWidget(m_aacSettingsScreen);
}

void MainWindow::onConnectRequested(const QString& host, int port, const QString& username)
{
    showConnectingScreen();
    m_backend->connectToServer(host, port, username);
}

void MainWindow::onRefreshRequested()
{
    m_backend->requestChannels();
}

void MainWindow::onJoinChannelRequested(int channelId)
{
    m_backend->joinChannel(channelId);
}

void MainWindow::onLeaveChannelRequested()
{
    m_backend->leaveChannel();
}

void MainWindow::onTransmitToggled(bool enabled)
{
    m_backend->setTransmitEnabled(enabled);
}

void MainWindow::onCancelConnectRequested()
{
    m_backend->disconnectFromServer();
    showConnectScreen();
}

void MainWindow::onConnected()
{
    showChannelListScreen();
    m_backend->requestChannels();
}

void MainWindow::onConnectionFailed(const QString& reason)
{
    showConnectScreen();
    m_connectScreen->setError(reason);
}

void MainWindow::onDisconnected()
{
    showConnectScreen();
}

void MainWindow::onChannelsUpdated(const QList<ChannelInfo>& channels)
{
    m_channelListScreen->setChannels(channels);
}

void MainWindow::onJoinedChannel(int channelId, const QString& channelName)
{
    showInChannelScreen(channelId, channelName);
}

void MainWindow::onLeftChannel()
{
    showChannelListScreen();
}

void MainWindow::onSelfVoiceState(SelfVoiceState state)
{
    m_inChannelScreen->updateSelfVoiceState(state);
}

void MainWindow::onOtherUserVoiceState(const OtherUserVoiceEvent& event)
{
    m_inChannelScreen->updateOtherUserVoiceState(event);
}

void MainWindow::onEventMessage(const QString& message)
{
    m_inChannelScreen->setEventMessage(message);
}
