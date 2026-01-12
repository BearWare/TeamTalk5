#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

// TeamTalk 5 Classic C API
#include "TeamTalk.h"

#include "BackendState.h"
#include "BackendEvents.h"

class BackendAdapter : public QObject {
    Q_OBJECT
public:
    explicit BackendAdapter(QObject* parent = nullptr);

    // UI → Backend
    void connectToServer(const QString& host, int port);
    void disconnectFromServer();

    void joinChannel(int channelId);
    void leaveChannel();

    void setTransmitEnabled(bool enabled);

signals:
    // Backend → UI
    void connectionStateChanged(ConnectionState state);
    void channelEvent(const ChannelEvent& event);
    void errorOccurred(const ErrorEvent& error);

    // AAC‑relevant new signals
    void selfVoiceEvent(const SelfVoiceEvent& event);
    void audioDeviceEvent(const AudioDeviceEvent& event);
    void textMessageEvent(const TextMessageEvent& event);

private slots:
    void pollTeamTalk();

private:
    BackendState m_state;

    TTInstance* m_tt = nullptr;
    QTimer* m_pollTimer = nullptr;
};
