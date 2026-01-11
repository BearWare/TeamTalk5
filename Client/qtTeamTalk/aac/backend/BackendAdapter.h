#pragma once

#include <QObject>
#include <QString>
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

private:
    BackendState m_state;

    // TODO: Add TeamTalk backend instance pointer later
};
