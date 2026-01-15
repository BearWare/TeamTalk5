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
    void connectRequested(const QString& host, int port, const QString& username);

private:
    QLineEdit* m_hostEdit;
    QSpinBox* m_portEdit;
    QLineEdit* m_usernameEdit;   // ← Add this
    QPushButton* m_connectButton;
};
