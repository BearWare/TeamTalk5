#pragma once

#include "AACScreen.h"

class QPushButton;

// Simple connecting screen:
// - “Connecting…” label
// - Cancel button
// - No settings buttons (per Option A1)
class ConnectingScreen : public AACScreen {
    Q_OBJECT
public:
    explicit ConnectingScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

signals:
    void cancelRequested();

private:
    QPushButton* m_cancelButton = nullptr;
};
