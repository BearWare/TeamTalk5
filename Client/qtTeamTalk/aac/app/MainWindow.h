#pragma once

#include <QMainWindow>

class AACAccessibilityManager;
class StateMachine;
class BackendAdapter;

class ConnectScreen;
class ChannelListScreen;
class InChannelScreen;
class ConnectingScreen;
class AppSettingsScreen;
class AACSettingsScreen;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // Navigation
    void showConnectScreen();
    void showChannelListScreen();
    void showInChannelScreen(int channelId, const QString& channelName);
    void showConnectingScreen();
    void showAppSettingsScreen();
    void showAACSettingsScreen();

    // ConnectScreen
    void onConnectRequested(const QString& host, int port, const QString& username);

    // ChannelListScreen
    void onRefreshRequested();
    void onJoinChannelRequested(int channelId);

    // InChannelScreen
    void onLeaveChannelRequested();
    void onTransmitToggled(bool enabled);

    // ConnectingScreen
    void onCancelConnectRequested();

    // Backend events
    void onConnected();
    void onConnectionFailed(const QString& reason);
    void onDisconnected();
    void onChannelsUpdated(const QList<ChannelInfo>& channels);
    void onJoinedChannel(int channelId, const QString& channelName);
    void onLeftChannel();
    void onSelfVoiceState(SelfVoiceState state);
    void onOtherUserVoiceState(const OtherUserVoiceEvent& event);
    void onEventMessage(const QString& message);

private:
    void wireScreens();
    void applyAACDefaults();

    AACAccessibilityManager* m_aac = nullptr;
    StateMachine* m_state = nullptr;
    BackendAdapter* m_backend = nullptr;

    ConnectScreen* m_connectScreen = nullptr;
    ChannelListScreen* m_channelListScreen = nullptr;
    InChannelScreen* m_inChannelScreen = nullptr;
    ConnectingScreen* m_connectingScreen = nullptr;
    AppSettingsScreen* m_appSettingsScreen = nullptr;
    AACSettingsScreen* m_aacSettingsScreen = nullptr;
};
