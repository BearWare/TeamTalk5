#include "MainWindow.h"
#include <QMessageBox>

void MainWindow::showConnecting()
{
    setWindowTitle("Connecting…");
}

void MainWindow::showConnected()
{
    setWindowTitle("Connected");
}

void MainWindow::showDisconnected()
{
    setWindowTitle("Disconnected");
}

void MainWindow::showErrorMessage(const QString& msg)
{
    QMessageBox::critical(this, "Error", msg);
}
