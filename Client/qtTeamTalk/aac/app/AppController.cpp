#include "AppController.h"
#include "BackendAdapter.h"
#include "StateMachine.h"
#include "MainWindow.h"

AppController::AppController(QObject* parent)
    : QObject(parent)
{
    m_backend = new BackendAdapter(this);
    m_stateMachine = new StateMachine(this);
    m_mainWindow = new MainWindow();

    // Wire components together
    m_stateMachine->attachBackend(m_backend);
    m_stateMachine->attachMainWindow(m_mainWindow);
}

void AppController::start()
{
    m_mainWindow->show();
}
