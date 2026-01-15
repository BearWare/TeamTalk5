#include "ConnectScreen.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

ConnectScreen::ConnectScreen(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    //
    // Host
    //
    layout->addWidget(new QLabel("Host:", this));
    m_hostEdit = new QLineEdit(this);
    m_hostEdit->setText("localhost");   // sensible default
    layout->addWidget(m_hostEdit);

    //
    // Port
    //
    layout->addWidget(new QLabel("Port:", this));
    m_portEdit = new QSpinBox(this);
    m_portEdit->setRange(1, 65535);
    m_portEdit->setValue(10333);        // sensible default
    layout->addWidget(m_portEdit);

    //
    // Username
    //
    layout->addWidget(new QLabel("Username:", this));
    m_usernameEdit = new QLineEdit(this);
    layout->addWidget(m_usernameEdit);

    //
    // Connect button
    //
    m_connectButton = new QPushButton("Connect", this);
    layout->addWidget(m_connectButton);

    //
    // Emit real values
    //
    connect(m_connectButton, &QPushButton::clicked, this, [this]() {
        const QString host = m_hostEdit->text();
        const int port = m_portEdit->value();
        const QString username = m_usernameEdit->text();

        emit connectRequested(host, port, username);
    });
}
