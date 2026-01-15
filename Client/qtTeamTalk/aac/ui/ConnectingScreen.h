#pragma once

#include <QWidget>

class QPushButton;

class ConnectingScreen : public QWidget {
    Q_OBJECT
public:
    explicit ConnectingScreen(QWidget* parent = nullptr);

signals:
    void cancelRequested();

private:
    QPushButton* m_cancelButton;
};
