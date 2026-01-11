#pragma once

#include <QMainWindow>
#include "backend/BackendEvents.h"

class BackendAdapter;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    void attachStateMachine(StateMachine* sm);

private:
    BackendAdapter* m_backend;
    StateMachine* m_stateMachine = nullptr;

    void setupBackendConnections();
    void attachStateMachine(class StateMachine* sm);

private slots:
    void onConnectionStateChanged(ConnectionState state);
    void onChannelEvent(const ChannelEvent& event);
    void onBackendError(const ErrorEvent& error);
};
