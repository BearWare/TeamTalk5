#pragma once

#include <QMainWindow>
#include "backend/BackendEvents.h"

class BackendAdapter;
class StateMachine;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    BackendAdapter* m_backend = nullptr;
    StateMachine* m_stateMachine = nullptr;

private slots:
    void onConnectionStateChanged(ConnectionState state);
    void onChannelEvent(const ChannelEvent& event);
    void onBackendError(const ErrorEvent& error);
};
