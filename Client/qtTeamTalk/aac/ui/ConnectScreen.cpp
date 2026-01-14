#include "ConnectScreen.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>

ConnectScreen::ConnectScreen(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* hostLabel = new QLabel("Server Address:", this);
    m_hostEdit = new QLineEdit(this);
    m_hostEdit->setText("localhost"); // default or your choice

    auto* portLabel = new QLabel("Port:", this);
    m_portSpin = new QSpinBox(this);
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(10333);

    m_connectButton = new QPushButton("Connect", this);
    m_connectButton->setMinimumHeight(80); // AAC-friendly size

    layout->addWidget(hostLabel);
    layout->addWidget(m_hostEdit);
    layout->addWidget(portLabel);
    layout->addWidget(m_portSpin);
    layout->addWidget(m_connectButton);
    layout->addStretch();

    connect(m_connectButton, &QPushButton::clicked, this, [this]() {
        emit connectRequested(m_hostEdit->text(), m_portSpin->value());
    });
}
