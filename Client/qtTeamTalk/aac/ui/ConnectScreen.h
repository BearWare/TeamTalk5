#pragma once

#include <QWidget>

class QLineEdit;
class QSpinBox;
class QPushButton;

class ConnectScreen : public QWidget {
    Q_OBJECT
public:
    explicit ConnectScreen(QWidget* parent = nullptr);

signals:
    void connectRequested();
    void connectToServer(const QString& host, int port);

private:
    QLineEdit* m_hostEdit;
    QSpinBox* m_portEdit;
    QPushButton* m_connectButton;
    QPushButton* m_startButton;
};
