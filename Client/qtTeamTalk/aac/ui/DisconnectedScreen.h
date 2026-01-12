#pragma once
#include <QWidget>

class DisconnectedScreen : public QWidget {
    Q_OBJECT
public:
    explicit DisconnectedScreen(QWidget* parent = nullptr);
signals:
    void connectRequested();
};
