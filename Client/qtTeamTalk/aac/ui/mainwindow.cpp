#include "MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QResizeEvent>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    //
    // Load high‑contrast AAC stylesheet
    //
    QFile f(":/aac/ui/aac-highcontrast.qss");
    if (f.open(QFile::ReadOnly)) {
        const QString css = QString::fromUtf8(f.readAll());
        qApp->setStyleSheet(css);
    }

    //
    // Backend + state machine
    //
    m_backend = new BackendAdapter(this);
    m_sm = new StateMachine(this);
    m_sm->attachBackend(m_backend);

    //
    // Screens
    //
    m_disconnected = new DisconnectedScreen(this);
    m_connecting   = new ConnectingScreen(this);
    m_connected    = new ConnectedScreen(this);
    m_inChannel    = new InChannelScreen(this);

    //
    // Overlays
    //
    m_audioBanner = new AudioBanner(this);
    m_audioBanner->setObjectName("AudioBanner");
    m_audioBanner->hide();

    //
    // Initial screen
    //
    setCentralWidget(nullptr); // we manage children manually
    switchScreen(m_disconnected);

    //
    // Backend → UI wiring
    //
    connect(m_sm, &StateMachine::connectionStateChanged,
            this, &MainWindow::onConnectionStateChanged);

    connect(m_sm, &StateMachine::channelChanged,
            this, &MainWindow::onChannelChanged);

    connect(m_sm, &StateMachine::errorOccurred,
            this, &MainWindow::onErrorOccurred);

    connect(m_sm, &StateMachine::selfVoiceStateChanged,
            this, &MainWindow::onSelfVoiceStateChanged);

    connect(m_sm, &StateMachine::audioDeviceStateChanged,
            this, &MainWindow::onAudioDeviceStateChanged);

    connect(m_sm, &StateMachine::incomingTextMessage,
            this, &MainWindow::onIncomingTextMessage);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    layoutOverlays();
}

void MainWindow::switchScreen(QWidget* screen)
{
    if (!screen)
        return;

    if (m_currentScreen && m_currentScreen != screen)
        m_currentScreen->hide();

    m_currentScreen = screen;
    m_currentScreen->setParent(this);
    m_currentScreen->setGeometry(this->rect());
    m_currentScreen->show();

    layoutOverlays();
}

void MainWindow::layoutOverlays()
{
    if (!m_audioBanner)
        return;

    const int bannerHeight = m_audioBanner->height();
    m_audioBanner->setGeometry(0, 0, width(), bannerHeight);

    if (m_currentScreen) {
        QRect r = this->rect();
        r.setTop(bannerHeight);
        m_currentScreen->setGeometry(r);
    }
}

//
// Backend → UI slots
//

void MainWindow::onConnectionStateChanged(ConnectionState state)
{
    switch (state) {
    case ConnectionState::Disconnected:
        switchScreen(m_disconnected);
        break;
    case ConnectionState::Connecting:
        switchScreen(m_connecting);
        break;
    case ConnectionState::Connected:
        switchScreen(m_connected);
        break;
    case ConnectionState::Error:
        // stay on current screen; error banner will show
        break;
    }
}

void MainWindow::onChannelChanged(int channelId)
{
    if (channelId == -1)
        switchScreen(m_connected);
    else
        switchScreen(m_inChannel);
}

void MainWindow::onErrorOccurred(const QString& message)
{
    if (m_audioBanner)
        m_audioBanner->showMessage(QStringLiteral("Error: ") + message);
}

void MainWindow::onSelfVoiceStateChanged(SelfVoiceState state)
{
    if (!m_inChannel)
        return;

    const bool talking = (state == SelfVoiceState::Talking);
    m_inChannel->setTransmitState(talking);
}

void MainWindow::onAudioDeviceStateChanged(AudioDeviceEventType type)
{
    if (!m_audioBanner)
        return;

    QString msg;
    switch (type) {
    case AudioDeviceEventType::Added:
        msg = QStringLiteral("Microphone connected");
        break;
    case AudioDeviceEventType::Removed:
        msg = QStringLiteral("Microphone disconnected");
        break;
    case AudioDeviceEventType::Failed:
        msg = QStringLiteral("Microphone error");
        break;
    }

    if (!msg.isEmpty())
        m_audioBanner->showMessage(msg);
}

void MainWindow::onIncomingTextMessage(int fromUserId, const QString& message)
{
    if (!m_inChannel)
        return;

    m_inChannel->showMessageBubble(fromUserId, message);
}
