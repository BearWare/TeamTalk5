#include "ConnectScreen.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

ConnectScreen::ConnectScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreen(aac, parent)
{
    auto* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("Host:", this));
    m_hostEdit = new QLineEdit(this);
    m_hostEdit->setText("localhost");
    layout->addWidget(m_hostEdit);
    registerInteractive(m_hostEdit);

    layout->addWidget(new QLabel("Port:", this));
    m_portEdit = new QSpinBox(this);
    m_portEdit->setRange(1, 65535);
    m_portEdit->setValue(10333);
    layout->addWidget(m_portEdit);
    registerInteractive(m_portEdit);

    layout->addWidget(new QLabel("Username:", this));
    m_usernameEdit = new QLineEdit(this);
    layout->addWidget(m_usernameEdit);
    registerInteractive(m_usernameEdit);

    m_connectButton = new QPushButton("Connect", this);
    layout->addWidget(m_connectButton);
    registerInteractive(m_connectButton, true);

    m_errorLine = new QLineEdit(this);
    m_errorLine->setReadOnly(true);
    m_errorLine->setStyleSheet("color: red;");
    layout->addWidget(m_errorLine);

    connect(m_connectButton, &QPushButton::clicked, this, [this]() {
        emit connectRequested(m_hostEdit->text(),
                              m_portEdit->value(),
                              m_usernameEdit->text());
    });
}

void ConnectScreen::setError(const QString& message)
{
    m_errorLine->setText(message);
}
