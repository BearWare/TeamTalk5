#include "InChannelScreen.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

InChannelScreen::InChannelScreen(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    m_transmitButton = new QPushButton("Transmit", this);
    m_transmitButton->setCheckable(true);

    m_leaveButton = new QPushButton("Leave Channel", this);

    m_voiceIndicator = new QLabel("Silent", this);

    layout->addWidget(m_transmitButton);
    layout->addWidget(m_leaveButton);
    layout->addWidget(m_voiceIndicator);

    connect(m_transmitButton, &QPushButton::toggled,
            this, &InChannelScreen::transmitToggled);

    connect(m_leaveButton, &QPushButton::clicked,
            this, &InChannelScreen::leaveRequested);
}

void InChannelScreen::setSelfVoiceState(SelfVoiceState state)
{
    if (state == SelfVoiceState::Talking)
        m_voiceIndicator->setText("Talking");
    else
        m_voiceIndicator->setText("Silent");
}
