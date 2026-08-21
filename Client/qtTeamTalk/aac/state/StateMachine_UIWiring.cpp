#include "StateMachine.h"
#include "MainWindow.h"

void StateMachine::attachMainWindow(MainWindow* window)
{
    m_window = window;

    // StateMachine → UI
    connect(this, &StateMachine::uiShouldShowConnecting,
            window, &MainWindow::showConnecting);

    connect(this, &StateMachine::uiShouldShowConnected,
            window, &MainWindow::showConnected);

    connect(this, &StateMachine::uiShouldShowDisconnected,
            window, &MainWindow::showDisconnected);

    connect(this, &StateMachine::uiShouldShowError,
            window, &MainWindow::showErrorMessage);
}
