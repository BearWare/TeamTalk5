#include "InChannelScreen.h"
#include <QVBoxLayout>
#include <QPushButton>

InChannelScreen::InChannelScreen(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    m_transmitButton = new QPushButton("Start Talking", this);
    m_transmitButton->setMinimumHeight(80);

    m_leaveButton = new QPushButton("Leave Channel", this);
    m_leaveButton->setMinimumHeight(80);

    layout->addWidget(m_transmitButton);
    layout->addWidget(m_leaveButton);
    layout->addStretch();

    connect(m_transmitButton, &QPushButton::clicked, this, [this]() {
        m_transmitting = !m_transmitting;
        m_transmitButton->setText(m_transmitting ? "Stop Talking" : "Start Talking");
        emit transmitToggled(m_transmitting);
    });

    connect(m_leaveButton, &QPushButton::clicked,
            this, &InChannelScreen::leaveRequested);
}
