#pragma once

#include <QMainWindow>
#include "backend/BackendEvents.h"

class BackendAdapter;
class StateMachine;

// Forward declare your screen classes
class ConnectScreen;
class ConnectingScreen;
class ChannelListScreen;
class InChannelScreen;

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

private slots:
    void onBackendError(const ErrorEvent& error);
};
