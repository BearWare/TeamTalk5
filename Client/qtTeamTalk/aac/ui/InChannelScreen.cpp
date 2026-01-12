#include "InChannelScreen.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

InChannelScreen::InChannelScreen(QWidget* parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);

    m_transmitBtn = new QPushButton("Not Speaking", this);
    m_transmitBtn->setCheckable(true);

    layout->addWidget(m_transmitBtn);
}

void InChannelScreen::setTransmitState(bool active)
{
    m_transmitBtn->setChecked(active);
    m_transmitBtn->setText(active ? "Speaking" : "Not Speaking");
}

void InChannelScreen::showMessageBubble(int fromUserId, const QString& message)
{
    // TODO: add message bubble widget
}
