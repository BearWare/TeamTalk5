#pragma once

#include <QObject>

class BackendAdapter;
class StateMachine;
class MainWindow;

class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(QObject* parent = nullptr);

    // Entry point for the AAC‑native UI
    void start();

private:
    BackendAdapter* m_backend = nullptr;
    StateMachine*   m_stateMachine = nullptr;
    MainWindow*     m_mainWindow = nullptr;
};
