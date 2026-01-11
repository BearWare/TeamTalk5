#include "MainWindow.h"
#include "backend/BackendAdapter.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_backend(new BackendAdapter(this))
{
    setupBackendConnections();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupBackendConnections()
{
    connect(m_backend, &BackendAdapter::connectionStateChanged,
            this, &MainWindow::onConnectionStateChanged);

    connect(m_backend, &BackendAdapter::channelEvent,
            this, &MainWindow::onChannelEvent);

    connect(m_backend, &BackendAdapter::errorOccurred,
            this, &MainWindow::onBackendError);
}

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
