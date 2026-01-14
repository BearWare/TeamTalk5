#pragma once

#include <QWidget>

class QLineEdit;
class QSpinBox;
class QPushButton;

class ConnectScreen : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectScreen(QWidget* parent = nullptr);

signals:
    void connectRequested(const QString& host, int port);

private:
    QLineEdit* m_hostEdit = nullptr;
    QSpinBox* m_portSpin = nullptr;
    QPushButton* m_connectButton = nullptr;
};
