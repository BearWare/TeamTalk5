#include "MainWindow.h"
#include "backend/BackendAdapter.h"
#include "backend/StateMachine.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Create backend + state machine
    m_backend = new BackendAdapter(this);
    m_stateMachine = new StateMachine(this);

    // Wire backend → state machine
    m_stateMachine->attachBackend(m_backend);

    // Wire state machine → UI
    connect(m_stateMachine, &StateMachine::connectionStateChanged,
            this, &MainWindow::onConnectionStateChanged);

    connect(m_stateMachine, &StateMachine::channelChanged,
            this, &MainWindow::onChannelEvent);

    connect(m_stateMachine, &StateMachine::errorOccurred,
            this, &MainWindow::onBackendError);

    // UI → state machine wiring will go here later
}

MainWindow::~MainWindow() = default;

void MainWindow::onConnectionStateChanged(ConnectionState state)
{
    // TODO: update AAC-native UI to reflect connection state
}

void MainWindow::onChannelEvent(const ChannelEvent& event)
{
    // TODO: update AAC-native UI to reflect channel join/leave
}

void MainWindow::onBackendError(const ErrorEvent& error)
{
    QMessageBox::warning(this, tr("Error"), error.message);
}
