#pragma once

#include "AACScreen.h"

class QLineEdit;
class QSpinBox;
class QPushButton;

class ConnectScreen : public AACScreen {
    Q_OBJECT
public:
    explicit ConnectScreen(QWidget* parent = nullptr);

signals:
    void connectRequested(const QString& host, int port, const QString& username);

public slots:
    // Override only if needed — default AACScreen behaviour is enough
    void applyLargeTargetMode(bool enabled) override;

private:
    QLineEdit* m_hostEdit = nullptr;
    QSpinBox* m_portEdit = nullptr;
    QLineEdit* m_usernameEdit = nullptr;
    QPushButton* m_connectButton = nullptr;
};
