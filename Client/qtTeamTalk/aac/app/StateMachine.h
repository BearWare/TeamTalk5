#pragma once

#include <QObject>
#include "backend/BackendEvents.h"

class StateMachine : public QObject {
    Q_OBJECT
public:
    explicit StateMachine(QObject* parent = nullptr);

public slots:
    void onConnectionStateChanged(ConnectionState state);
    void onChannelEvent(const ChannelEvent& event);
    void onBackendError(const ErrorEvent& error);

signals:
    void requestUiUpdate();
};
