#include "ConnectScreen.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>

ConnectScreen::ConnectScreen(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    m_hostEdit = new QLineEdit(this);
    m_hostEdit->setPlaceholderText("Server host");

    m_portEdit = new QSpinBox(this);
    m_portEdit->setRange(1, 65535);
    m_portEdit->setValue(10333);

    m_connectButton = new QPushButton("Connect", this);
    m_startButton = new QPushButton("Start", this);

    layout->addWidget(m_hostEdit);
    layout->addWidget(m_portEdit);
    layout->addWidget(m_connectButton);
    layout->addWidget(m_startButton);

    connect(m_connectButton, &QPushButton::clicked,
            this, &ConnectScreen::connectRequested);

    connect(m_startButton, &QPushButton::clicked, this, [this] {
        emit connectToServer(m_hostEdit->text(), m_portEdit->value());
    });
}
