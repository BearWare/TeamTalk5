#pragma once

#include <QMainWindow>

class QStackedWidget;

class AACAccessibilityManager;
class BackendAdapter;
class StateMachine;

class ConnectScreen;
class ChannelListScreen;
class InChannelScreen;
class ConnectingScreen;
class AppSettingsScreen;
class AACSettingsScreen;
class AACCategoryScreen;
class AACSymbolGridScreen;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void showConnectScreen();
    void showChannelListScreen();
    void showInChannelScreen(int channelId, const QString& channelName);
    void showConnectingScreen();
    void showAppSettingsScreen();
    void showAACSettingsScreen();

    void onConnectRequested(const QString& host, int port, const QString& username);
    void onRefreshRequested();
    void onJoinChannelRequested(int channelId);
    void onLeaveChannelRequested();
    void onTransmitToggled(bool enabled);
    void onCancelConnectRequested();

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
    AACAccessibilityManager* m_aac = nullptr;
    BackendAdapter* m_backend = nullptr;
    StateMachine* m_state = nullptr;

    QStackedWidget* m_stack = nullptr;

    ConnectScreen* m_connectScreen = nullptr;
    ChannelListScreen* m_channelListScreen = nullptr;
    InChannelScreen* m_inChannelScreen = nullptr;
    ConnectingScreen* m_connectingScreen = nullptr;
    AppSettingsScreen* m_appSettingsScreen = nullptr;
    AACSettingsScreen* m_aacSettingsScreen = nullptr;

    AACCategoryScreen* m_aacCategoryScreen = nullptr;
    AACSymbolGridScreen* m_aacSymbolScreen = nullptr;

    void wireScreens();
    void applyAACDefaults();
    void switchToScreen(QWidget* w);
};
