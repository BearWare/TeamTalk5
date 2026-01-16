#pragma once

#include "AACScreen.h"

class QLineEdit;
class QSpinBox;
class QPushButton;

// Connect screen with:
// - Host
// - Port
// - Username
// - Connect button
// - Error line
// - Settings (App Settings)
// - AAC Access (AAC Settings)
class ConnectScreen : public AACScreen {
    Q_OBJECT
public:
    explicit ConnectScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

    void setError(const QString& message);

signals:
    void connectRequested(const QString& host, int port, const QString& username);
    void settingsRequested();
    void aacSettingsRequested();

private:
    QLineEdit*  m_hostEdit     = nullptr;
    QSpinBox*   m_portEdit     = nullptr;
    QLineEdit*  m_usernameEdit = nullptr;
    QPushButton* m_connectButton = nullptr;
    QLineEdit*  m_errorLine    = nullptr;
};
