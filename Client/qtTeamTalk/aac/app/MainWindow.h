#pragma once

#include <QMainWindow>
#include "backend/BackendEvents.h"

class BackendAdapter;
class StateMachine;

class ConnectScreen;
class ConnectingScreen;
class ChannelListScreen;
class InChannelScreen;

class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    BackendAdapter* m_backend = nullptr;
    StateMachine* m_stateMachine = nullptr;

    // AAC UI screens
    ConnectScreen* m_connectScreen = nullptr;
    ConnectingScreen* m_connectingScreen = nullptr;
    ChannelListScreen* m_channelListScreen = nullptr;
    InChannelScreen* m_inChannelScreen = nullptr;

    // Reconnect countdown
    QTimer* m_reconnectCountdownTimer = nullptr;
    int m_reconnectSecondsRemaining = 0;

private slots:
    void onBackendError(const ErrorEvent& error);

    void updateScreenForConnectionState(ConnectionState state);
    void updateScreenForChannelState(int channelId);

    void onReconnecting(int attempt, int delayMs);
    void onReconnectStopped();
    void onNotifyUser(const QString& message);

    void updateReconnectCountdown();
    void onReconnectNowClicked();

private:
    void showScreen(QWidget* screen);
};
