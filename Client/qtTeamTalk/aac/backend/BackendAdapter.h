#pragma once

#include "../channelinfo.h"
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
    // AAC-native: actively request a fresh snapshot of all channels
    Q_INVOKABLE void refreshChannels();

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
    // AAC-native: backend → UI snapshot of all channels
    void channelsEnumerated(const QList<ChannelInfo>& channels);

    // AAC‑relevant new signals
    void selfVoiceEvent(const SelfVoiceEvent& event);
    void audioDeviceEvent(const AudioDeviceEvent& event);
    void textMessageEvent(const TextMessageEvent& event);

private slots:
    void pollTeamTalk();

private:
    BackendState m_state;
    QList<ChannelInfo> enumerateChannels() const;

    TTInstance* m_tt = nullptr;
    QTimer* m_pollTimer = nullptr;
};
