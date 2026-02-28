#pragma once

#include <QObject>
#include <QFile>
#include <QDateTime>
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

    // Event pump
    void processEvents();

signals:
    // Backend → StateMachine
    void channelsEnumerated(const QList<ChannelInfo>& channels);
    void connectionStateChanged(ConnectionState state);
    void channelEvent(const ChannelEvent& event);
    void backendError(const ErrorEvent& error);
    void selfVoiceEvent(const SelfVoiceEvent& event);
    void otherUserVoiceEvent(const OtherUserVoiceEvent& event);

private:
    TTInstance* m_tt = nullptr;

    // Helpers
    QString mapErrorCode(int code, const QString& raw);
    void log(const QString& message);

    struct {
        QString username;
    } m_state;
};
