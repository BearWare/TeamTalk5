#include "ConnectingScreen.h"
#include <QVBoxLayout>
#include <QLabel>

ConnectingScreen::ConnectingScreen(QWidget* parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);

    auto label = new QLabel("Connecting…", this);
    label->setAlignment(Qt::AlignCenter);

    layout->addWidget(label);
}
