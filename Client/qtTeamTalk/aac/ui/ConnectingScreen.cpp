#include "ConnectingScreen.h"
#include <QVBoxLayout>
#include <QLabel>

ConnectingScreen::ConnectingScreen(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    m_label = new QLabel("Connecting…", this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setStyleSheet("font-size: 32px;");

    layout->addStretch();
    layout->addWidget(m_label);
    layout->addStretch();
}
