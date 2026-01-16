#pragma once

#include "AACScreen.h"

class QLineEdit;
class QSpinBox;
class QCheckBox;
class QComboBox;
class QPushButton;

// Top-level App Settings screen (non-AAC-specific):
// - Nickname
// - Server host
// - Server port
// - Auto-reconnect (placeholder)
// - Theme
// - Language
class AppSettingsScreen : public AACScreen {
    Q_OBJECT
public:
    explicit AppSettingsScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

signals:
    void backRequested();

private slots:
    void applySettings();

private:
    QLineEdit*  m_nickname      = nullptr;
    QLineEdit*  m_host          = nullptr;
    QSpinBox*   m_port          = nullptr;
    QCheckBox*  m_autoReconnect = nullptr;
    QComboBox*  m_theme         = nullptr;
    QComboBox*  m_language      = nullptr;

    QPushButton* m_saveButton   = nullptr;
    QPushButton* m_backButton   = nullptr;
};
