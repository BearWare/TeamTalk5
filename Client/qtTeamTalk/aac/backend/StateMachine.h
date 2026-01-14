#pragma once

#include <QObject>
#include "BackendEvents.h"
#include "BackendAdapter.h"
#include "BackendState.h"

class StateMachine : public QObject {
    Q_OBJECT
public:
    explicit StateMachine(QObject* parent = nullptr);

    void attachBackend(BackendAdapter* backend);

signals:
    // UI-facing signals
    void connectionStateChanged(ConnectionState state);
    void channelChanged(int channelId);
    void errorOccurred(const QString& message);

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
    void onChannelEvent(const ChannelEvent& event);
    void onErrorOccurred(const ErrorEvent& error);

    void onSelfVoiceEvent(const SelfVoiceEvent& event);
    void onAudioDeviceEvent(const AudioDeviceEvent& event);
    void onTextMessageEvent(const TextMessageEvent& event);

private:
    BackendAdapter* m_backend = nullptr;
    BackendState m_state;
};
