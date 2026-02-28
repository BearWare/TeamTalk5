#pragma once
#include <QObject>

class QWidget;

class NavigationController : public QObject {
    Q_OBJECT
public:
    explicit NavigationController(QObject *parent = nullptr);

    void showServerList();
    void showPublicServers();
    void showConnectionView();
    void showChannelView();
    void showInChannelView();

signals:
    void requestScreen(QWidget *screen);
};
