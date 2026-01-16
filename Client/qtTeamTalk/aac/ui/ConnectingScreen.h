#pragma once

#include "AACScreen.h"

class QPushButton;

class ConnectingScreen : public AACScreen {
    Q_OBJECT
public:
    explicit ConnectingScreen(QWidget* parent = nullptr);

signals:
    void cancelRequested();

public slots:
    void applyLargeTargetMode(bool enabled) override;

private:
    QPushButton* m_cancelButton = nullptr;
};
