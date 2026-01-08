#pragma once
#include <QMainWindow>

class NavigationController;
class ServerListWidget;
class PublicServerListWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    NavigationController *m_nav;
    ServerListWidget *m_serverList;
    PublicServerListWidget *m_publicServers;

    void setupUi();
    void setupConnections();
};
