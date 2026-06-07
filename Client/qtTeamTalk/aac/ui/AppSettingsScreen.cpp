#include "AppSettingsScreen.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>

AppSettingsScreen::AppSettingsScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreen(aac, parent)
{
    auto* layout = new QVBoxLayout(this);

    auto addRow = [&](const QString& label, QWidget* field) {
        auto* row = new QHBoxLayout();
        row->addWidget(new QLabel(label, this));
        row->addWidget(field);
        layout->addLayout(row);
        registerInteractive(field);
    };

    m_nickname = new QLineEdit(this);
    addRow(tr("Nickname:"), m_nickname);

    m_host = new QLineEdit(this);
    addRow(tr("Server host:"), m_host);

    m_port = new QSpinBox(this);
    m_port->setRange(1, 65535);
    addRow(tr("Server port:"), m_port);

    m_autoReconnect = new QCheckBox(tr("Auto-reconnect"), this);
    layout->addWidget(m_autoReconnect);
    registerInteractive(m_autoReconnect);

    m_theme = new QComboBox(this);
    m_theme->addItem(tr("System default"));
    m_theme->addItem(tr("Light"));
    m_theme->addItem(tr("Dark"));
    addRow(tr("Theme:"), m_theme);

    m_language = new QComboBox(this);
    m_language->addItem(tr("System default"));
    addRow(tr("Language:"), m_language);

    m_saveButton = new QPushButton(tr("Save"), this);
    layout->addWidget(m_saveButton);
    registerInteractive(m_saveButton, true);

    m_backButton = new QPushButton(tr("Back"), this);
    layout->addWidget(m_backButton);
    registerInteractive(m_backButton);

    connect(m_backButton, &QPushButton::clicked, this, &AppSettingsScreen::backRequested);
    connect(m_saveButton, &QPushButton::clicked, this, &AppSettingsScreen::applySettings);
}

void AppSettingsScreen::applySettings()
{
    emit backRequested();
}
