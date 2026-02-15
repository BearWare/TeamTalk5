#include "ConnectingScreen.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

ConnectingScreen::ConnectingScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreen(aac, parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* label = new QLabel(tr("Connecting..."), this);
    layout->addWidget(label);

    m_cancelButton = new QPushButton(tr("Cancel"), this);
    layout->addWidget(m_cancelButton);
    registerInteractive(m_cancelButton, true);

    connect(m_cancelButton, &QPushButton::clicked,
            this, &ConnectingScreen::cancelRequested);
}
