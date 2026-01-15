#pragma once

#include <QObject>
#include "BackendEvents.h"

class BackendAdapter : public QObject {
    Q_OBJECT
public:
    explicit BackendAdapter(QObject* parent = nullptr);

    // Real-value connection
    void connectToServer(const QString& host, int port, const QString& username);
    void disconnectFromServer();

    // Channel operations
    void refreshChannels();
    void joinChannel(int channelId);
    void leaveChannel();

    // Voice
    void setTransmitEnabled(bool enabled);

signals:
    // Backend → StateMachine
    void channelsEnumerated(const QList<ChannelInfo>& channels);
    void connectionStateChanged(ConnectionState state);
    void channelEvent(const ChannelEvent& event);
    void backendError(const ErrorEvent& error);
    void selfVoiceEvent(const SelfVoiceEvent& event);

private:
    TTInstance* m_tt = nullptr;
    BackendState m_state;
};
