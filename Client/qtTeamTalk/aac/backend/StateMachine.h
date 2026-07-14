#pragma once

#include <QObject>
#include <QTimer>
#include "BackendEvents.h"
#include "BackendAdapter.h"
#include "BackendState.h"

class StateMachine : public QObject {
    Q_OBJECT
public:
    explicit StateMachine(QObject* parent = nullptr);
    void stopAutoReconnect();

    void attachBackend(BackendAdapter* backend);

signals:
    // UI-facing signals
    void channelListChanged(const QList<QPair<int, QString>>& channels);
    void connectionStateChanged(ConnectionState state);
    void channelChanged(int channelId);
    void errorOccurred(const QString& message);

    void notifyUser(const QString& message);
    void reconnecting(int attempt, int delayMs);
    void reconnectStopped();

    void selfVoiceStateChanged(SelfVoiceState state);
    void audioDeviceStateChanged(AudioDeviceEventType type);
    void incomingTextMessage(int fromUserId, const QString& message);

public slots:
    // UI → StateMachine
    void onConnectRequested(const QString& host, int port);
    void onRefreshChannelsRequested();
    void onJoinChannelRequested(int channelId);
    void onLeaveChannelRequested();
    void onTransmitToggled(bool on);

private slots:
    void onConnectionStateChanged(ConnectionState state);
    void attemptReconnect();
    void onChannelEvent(const ChannelEvent& event);
    void onErrorOccurred(const ErrorEvent& error);
    void onChannelsEnumerated(const QList<ChannelInfo>& channels);

    void onSelfVoiceEvent(const SelfVoiceEvent& event);
    void onAudioDeviceEvent(const AudioDeviceEvent& event);
    void onTextMessageEvent(const TextMessageEvent& event);

private:
    QTimer* m_reconnectTimer = nullptr;
    int m_reconnectAttempts = 0;
    int m_maxReconnectAttempts = 10;

    QString m_lastHost;
    int m_lastPort = 0;

    BackendAdapter* m_backend = nullptr;
    BackendState m_state;
};
