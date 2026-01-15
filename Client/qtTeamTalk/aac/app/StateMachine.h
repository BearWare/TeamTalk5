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

public slots:
    void connectRequested(const QString& host, int port, const QString& username);
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
    void onOtherUserVoiceEvent(const OtherUserVoiceEvent& event);

signals:
    void uiShouldShowConnecting();
    void uiShouldShowConnected();
    void uiShouldShowInChannelScreen();
    void uiShouldShowDisconnected();
    void uiShouldShowError(const QString& message);

    void channelListChanged(const QList<ChannelInfo>& channels);
    void selfVoiceStateChanged(SelfVoiceState state);
    void otherUserVoiceStateChanged(const OtherUserVoiceEvent& event);

    void requestConnect(const QString& host, int port, const QString& username);

private:
    UiConnectionState m_state = UiConnectionState::Idle;
    BackendAdapter* m_backend = nullptr;
    QList<ChannelInfo> m_channels;
};
