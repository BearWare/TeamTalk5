#pragma once

#include <QMainWindow>

class BackendAdapter;
class StateMachine;

class ConnectScreen;
class ConnectingScreen;
class ChannelListScreen;
class InChannelScreen;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    // Backend + StateMachine
    BackendAdapter* m_backend = nullptr;
    StateMachine* m_stateMachine = nullptr;

    // Screens
    ConnectScreen* m_connectScreen = nullptr;
    ConnectingScreen* m_connectingScreen = nullptr;
    ChannelListScreen* m_channelListScreen = nullptr;
    InChannelScreen* m_inChannelScreen = nullptr;

    // Helpers
    void showScreen(QWidget* screen);

private slots:
    // StateMachine → UI
    void showConnectScreen();
    void showConnectingScreen();
    void showChannelListScreen();
    void showInChannelScreen();
};
