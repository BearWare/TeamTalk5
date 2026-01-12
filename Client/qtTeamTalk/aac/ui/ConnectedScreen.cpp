#include "ConnectedScreen.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

ConnectedScreen::ConnectedScreen(QWidget* parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);

    auto label = new QLabel("Connected", this);
    label->setAlignment(Qt::AlignCenter);

    auto joinBtn = new QPushButton("Join Channel", this);

    layout->addWidget(label);
    layout->addWidget(joinBtn);
}
