#pragma once

#include <QObject>
#include "backend/BackendEvents.h"

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

public slots:
    // User-intent entry points
    void connectRequested();
    void disconnectRequested();
    void onRefreshChannelsRequested();
    void onJoinChannelRequested(int channelId);
    void onLeaveChannelRequested();
    void onTransmitToggled(bool enabled);

    // Backend → StateMachine
    void onChannelsEnumerated(const QList<ChannelInfo>& channels);
    void onConnectionStateChanged(ConnectionState state);
    void onChannelEvent(const ChannelEvent& event);
    void onBackendError(const ErrorEvent& error);
    void onSelfVoiceEvent(const SelfVoiceEvent& event);

signals:
    // UI-driving signals
    void uiShouldShowConnecting();
    void uiShouldShowConnected();
    void uiShouldShowDisconnected();
    void uiShouldShowError(const QString& message);

    void channelListChanged(const QList<ChannelInfo>& channels);
    void selfVoiceStateChanged(SelfVoiceState state);

    // Reconnect arc
    void reconnecting(int attempt, int delayMs);
    void reconnectStopped();
    void notifyUser(const QString& message);

private:
    UiConnectionState m_state = UiConnectionState::Idle;
    BackendAdapter* m_backend = nullptr;
    QList<ChannelInfo> m_channels;
};
