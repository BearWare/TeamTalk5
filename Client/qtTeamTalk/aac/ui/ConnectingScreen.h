#pragma once

#include "AACScreen.h"

class QPushButton;

class ConnectingScreen : public AACScreen {
    Q_OBJECT
public:
    explicit ConnectingScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

signals:
    void cancelRequested();

private:
    QPushButton* m_cancelButton = nullptr;
};
