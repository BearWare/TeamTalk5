#pragma once

#include <QObject>
#include "BackendEvents.h"

class BackendAdapter;

class StateMachine : public QObject {
    Q_OBJECT
public:
    explicit StateMachine(QObject* parent = nullptr);

    enum class UiConnectionState {
        Idle,
        Connecting,
        Connected
    };
    Q_ENUM(UiConnectionState)

    void attachBackend(BackendAdapter* backend);

    //
    // Large‑Target Mode (AAC accessibility)
    //
    bool largeTargetModeEnabled() const { return m_largeTargetModeEnabled; }

public slots:
    // User → StateMachine
    void connectRequested(const QString& host, int port, const QString& username);
    void disconnectRequested();
    void onRefreshChannelsRequested();
    void onJoinChannelRequested(int channelId);
    void onLeaveChannelRequested();
    void onTransmitToggled(bool enabled);

    //
    // Large‑Target Mode toggles
    //
    void enableLargeTargetMode();
    void disableLargeTargetMode();
    void toggleLargeTargetMode();

    // Backend → StateMachine
    void onChannelsEnumerated(const QList<ChannelInfo>& channels);
    void onConnectionStateChanged(ConnectionState state);
    void onChannelEvent(const ChannelEvent& event);
    void onBackendError(const ErrorEvent& error);
    void onSelfVoiceEvent(const SelfVoiceEvent& event);
    void onOtherUserVoiceEvent(const OtherUserVoiceEvent& event);

signals:
    //
    // Screen navigation
    //
    void uiShouldShowConnecting();
    void uiShouldShowConnected();
    void uiShouldShowInChannelScreen();
    void uiShouldShowDisconnected();
    void uiShouldShowError(const QString& message);

    //
    // In‑channel context
    //
    void currentChannelNameChanged(const QString& name);
    void inChannelEventMessage(const QString& message);
    void clearInChannelScreen();
    void inChannelError(const QString& message);

    //
    // Data to UI
    //
    void channelListChanged(const QList<ChannelInfo>& channels);
    void selfVoiceStateChanged(SelfVoiceState state);
    void otherUserVoiceStateChanged(const OtherUserVoiceEvent& event);

    //
    // Actions to backend
    //
    void requestConnect(const QString& host, int port, const QString& username);

    //
    // Large‑Target Mode → UI
    //
    void largeTargetModeChanged(bool enabled);

private:
    UiConnectionState m_state = UiConnectionState::Idle;
    BackendAdapter* m_backend = nullptr;
    QList<ChannelInfo> m_channels;
    int m_currentChannelId = -1;

    //
    // AAC accessibility state
    //
    bool m_largeTargetModeEnabled = false;
};
