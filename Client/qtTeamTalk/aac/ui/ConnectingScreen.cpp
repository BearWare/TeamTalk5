#include "ConnectingScreen.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

ConnectingScreen::ConnectingScreen(QWidget* parent)
    : AACScreen(parent)
{
    auto* layout = new QVBoxLayout(this);

    //
    // Informational label (NOT scaled in B1)
    //
    auto* label = new QLabel("Connecting...", this);
    layout->addWidget(label);

    //
    // Cancel button (interactive → scaled)
    //
    m_cancelButton = new QPushButton("Cancel", this);
    layout->addWidget(m_cancelButton);

    connect(m_cancelButton, &QPushButton::clicked,
            this, &ConnectingScreen::cancelRequested);
}

//
// Large‑Target Mode
// -----------------
// We rely on AACScreen for:
//   - scaling the Cancel button
//   - scaling layout spacing/margins
//
// The “Connecting…” label remains unscaled (per B1).
//

void ConnectingScreen::applyLargeTargetMode(bool enabled)
{
    AACScreen::applyLargeTargetMode(enabled);
}
