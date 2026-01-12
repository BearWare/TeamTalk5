#pragma once

#include <QObject>
#include "backend/BackendEvents.h"

class BackendAdapter;

class StateMachine : public QObject {
    Q_OBJECT
public:
    explicit StateMachine(QObject* parent = nullptr);

    // High-level UI state for the connection
    enum class UiConnectionState {
        Idle,
        Connecting,
        Connected
    };
    Q_ENUM(UiConnectionState)

    void attachBackend(BackendAdapter* backend);

public slots:
    // User-intent entry points
    void connectRequested();
    void disconnectRequested();

    // Backend event entry points
    void onConnectionStateChanged(ConnectionState state);
    void onChannelEvent(const ChannelEvent& event);
    void onBackendError(const ErrorEvent& error);

signals:
    // UI-driving signals
    void uiShouldShowConnecting();
    void uiShouldShowConnected();
    void uiShouldShowDisconnected();
    void uiShouldShowError(const QString& message);

private:
    UiConnectionState m_state = UiConnectionState::Idle;
    BackendAdapter* m_backend = nullptr;
};
