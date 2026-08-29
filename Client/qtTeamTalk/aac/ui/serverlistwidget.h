#pragma once
#include <QWidget>

class ServerListModel;

class ServerListWidget : public QWidget {
    Q_OBJECT
public:
    explicit ServerListWidget(QWidget *parent = nullptr);

signals:
    void requestAddServer();
    void requestConnect(int index);
    void requestPublicServers();

private:
    ServerListModel *m_model;
    void setupUi();
    void setupConnections();
};
