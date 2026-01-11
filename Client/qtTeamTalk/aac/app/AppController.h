#pragma once

#include <QObject>

class MainWindow;
class StateMachine;

class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(QObject* parent = nullptr);

    void start();

private:
    MainWindow* m_mainWindow;
    StateMachine* m_stateMachine;
};
