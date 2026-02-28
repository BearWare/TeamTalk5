#include "MainWindow.h"
#include "StateMachine.h"

void MainWindow::attachStateMachine(StateMachine* sm)
{
    m_stateMachine = sm;

    // UI reacts to StateMachine signals
    connect(sm, &StateMachine::uiShouldShowConnecting,
            this, &MainWindow::showConnecting);

    connect(sm, &StateMachine::uiShouldShowConnected,
            this, &MainWindow::showConnected);

    connect(sm, &StateMachine::uiShouldShowDisconnected,
            this, &MainWindow::showDisconnected);

    connect(sm, &StateMachine::uiShouldShowError,
            this, &MainWindow::showErrorMessage);
}
