#pragma once

#include <QMainWindow>

#include "backend/BackendAdapter.h"
#include "backend/StateMachine.h"

#include "ui/DisconnectedScreen.h"
#include "ui/ConnectingScreen.h"
#include "ui/ConnectedScreen.h"
#include "ui/InChannelScreen.h"
#include "ui/AudioBanner.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    // Backend → UI
    void onConnectionStateChanged(ConnectionState state);
    void onChannelChanged(int channelId);
    void onErrorOccurred(const QString& message);

    void onSelfVoiceStateChanged(SelfVoiceState state);
    void onAudioDeviceStateChanged(AudioDeviceEventType type);
    void onIncomingTextMessage(int fromUserId, const QString& message);

private:
    BackendAdapter* m_backend = nullptr;
    StateMachine* m_sm = nullptr;

    QWidget* m_currentScreen = nullptr;

    DisconnectedScreen* m_disconnected = nullptr;
    ConnectingScreen* m_connecting = nullptr;
    ConnectedScreen* m_connected = nullptr;
    InChannelScreen* m_inChannel = nullptr;

    AudioBanner* m_audioBanner = nullptr;

    void switchScreen(QWidget* screen);
    void layoutOverlays();
};
