#include "DisconnectedScreen.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

DisconnectedScreen::DisconnectedScreen(QWidget* parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);

    auto label = new QLabel("Not connected", this);
    label->setAlignment(Qt::AlignCenter);

    auto btn = new QPushButton("Connect", this);

    layout->addWidget(label);
    layout->addWidget(btn);

    //
    // UI → signal wiring
    //
    connect(btn, &QPushButton::clicked, this, [this]() {
        emit connectRequested();
    });
}
