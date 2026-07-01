#pragma once

#include <QObject>
#include "BackendEvents.h"

class BackendAdapter;
class MainWindow;

class StateMachine : public QObject {
    Q_OBJECT
public:
    explicit StateMachine(QObject* parent = nullptr);

    // AppController attaches components here
    void attachBackend(BackendAdapter* backend);
    void attachMainWindow(MainWindow* window);

signals:
    // StateMachine → UI (MainWindow reacts to these)
    void uiShouldShowConnecting();
    void uiShouldShowConnected();
    void uiShouldShowDisconnected();
    void uiShouldShowError(const QString& message);

public slots:
    // Backend → StateMachine
    void onConnectionStateChanged(ConnectionState state);
    void onChannelEvent(const ChannelEvent& event);
    void onErrorOccurred(const ErrorEvent& error);

private:
    BackendAdapter* m_backend = nullptr;
    MainWindow* m_window = nullptr;

    // Internal state
    ConnectionState m_connectionState = ConnectionState::Disconnected;
    int m_currentChannelId = -1;

    // Internal helper to update UI based on state
    void updateUI();
};
