#include "AppController.h"
#include "MainWindow.h"
#include "StateMachine.h"

AppController::AppController(QObject* parent)
    : QObject(parent),
      m_mainWindow(nullptr),
      m_stateMachine(nullptr)
{
}

void AppController::start()
{
    m_mainWindow = new MainWindow();
    m_stateMachine = new StateMachine(m_mainWindow);

    // Wire StateMachine into MainWindow
    m_mainWindow->attachStateMachine(m_stateMachine);

    m_mainWindow->show();
}
