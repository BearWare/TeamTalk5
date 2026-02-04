#pragma once

#include <QObject>
#include <QString>

#include "aac/backend/channelinfo.h"
#include "aac/backend/BackendEvents.h"

// Very thin state machine wrapper.
// Your BackendAdapter already drives most transitions.
// This class exists mainly so MainWindow can remain clean.

class StateMachine : public QObject {
    Q_OBJECT
public:
    explicit StateMachine(QObject* parent = nullptr) = default;

signals:
    // High‑level navigation signals (MainWindow listens to these)
    void goToConnect();
    void goToConnecting();
    void goToChannelList();
    void goToInChannel(int channelId, const QString& channelName);

    // Backend‑driven events
    void connected();
    void connectionFailed(const QString& reason);
    void disconnected();
    void channelsUpdated(const QList<ChannelInfo>& channels);
    void joinedChannel(int channelId, const QString& channelName);
    void leftChannel();
    void selfVoiceState(SelfVoiceState state);
    void otherUserVoiceState(const OtherUserVoiceEvent& event);
    void eventMessage(const QString& message);

public slots:
    // These slots are called by BackendAdapter
    void onConnected()                { emit connected(); }
    void onConnectionFailed(const QString& r) { emit connectionFailed(r); }
    void onDisconnected()             { emit disconnected(); }
    void onChannelsUpdated(const QList<ChannelInfo>& c) { emit channelsUpdated(c); }
    void onJoinedChannel(int id, const QString& name) { emit joinedChannel(id, name); }
    void onLeftChannel()              { emit leftChannel(); }
    void onSelfVoiceState(SelfVoiceState s) { emit selfVoiceState(s); }
    void onOtherUserVoiceState(const OtherUserVoiceEvent& e) { emit otherUserVoiceState(e); }
    void onEventMessage(const QString& m) { emit eventMessage(m); }
};
