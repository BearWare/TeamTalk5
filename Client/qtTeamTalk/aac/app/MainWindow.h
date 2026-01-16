#pragma once

#include <QMainWindow>

class BackendAdapter;
class StateMachine;

class AACAccessibilityManager;
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
    // AAC framework
    AACAccessibilityManager* m_aac = nullptr;

    // Backend + StateMachine
    BackendAdapter* m_backend = nullptr;
    StateMachine* m_stateMachine = nullptr;

    // Screens
    ConnectScreen* m_connectScreen = nullptr;
    ConnectingScreen* m_connectingScreen = nullptr;
    ChannelListScreen* m_channelListScreen = nullptr;
    InChannelScreen* m_inChannelScreen = nullptr;

    void showScreen(QWidget* screen);

private slots:
    void showConnectScreen();
    void showConnectingScreen();
    void showChannelListScreen();
    void showInChannelScreen();

protected:
    void keyPressEvent(QKeyEvent* e) override;
};
