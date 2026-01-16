#pragma once

#include <QMainWindow>
#include <QList>

class BackendAdapter;
class StateMachine;

class AACScreen;
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

    // All AAC screens for Large‑Target Mode propagation
    QList<AACScreen*> m_aacScreens;

    // Helpers
    void showScreen(QWidget* screen);

private slots:
    // StateMachine → UI
    void showConnectScreen();
    void showConnectingScreen();
    void showChannelListScreen();
    void showInChannelScreen();

    // Large‑Target Mode → UI
    void onLargeTargetModeChanged(bool enabled);
};
