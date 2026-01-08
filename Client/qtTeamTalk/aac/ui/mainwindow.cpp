#include "mainwindow.h"
#include "navigationcontroller.h"
#include "serverlistwidget.h"
#include "publicserverlistwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_nav(new NavigationController(this)),
      m_serverList(new ServerListWidget(this)),
      m_publicServers(new PublicServerListWidget(this))
{
    setupUi();
    setupConnections();
}

void MainWindow::setupUi() {
    setWindowTitle("TeamTalk AAC UI");
    resize(900, 600);

    // Default view: server list
    setCentralWidget(m_serverList);
}

void MainWindow::setupConnections() {
    connect(m_serverList, &ServerListWidget::requestPublicServers, this, [this]() {
        setCentralWidget(m_publicServers);
    });

    connect(m_publicServers, &PublicServerListWidget::backRequested, this, [this]() {
        setCentralWidget(m_serverList);
    });
}
