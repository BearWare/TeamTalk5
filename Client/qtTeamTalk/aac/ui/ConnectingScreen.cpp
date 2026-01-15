#include "ConnectingScreen.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

ConnectingScreen::ConnectingScreen(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* label = new QLabel("Connecting...", this);
    m_cancelButton = new QPushButton("Cancel", this);

    layout->addWidget(label);
    layout->addWidget(m_cancelButton);

    connect(m_cancelButton, &QPushButton::clicked,
            this, &ConnectingScreen::cancelRequested);
}
