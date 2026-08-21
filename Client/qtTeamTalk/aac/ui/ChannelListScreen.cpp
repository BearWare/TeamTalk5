#include "ChannelListScreen.h"

#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QListWidgetItem>

ChannelListScreen::ChannelListScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreen(aac, parent)
{
    auto* layout = new QVBoxLayout(this);

    m_list = new QListWidget(this);
    layout->addWidget(m_list);
    registerInteractive(m_list);

    m_refreshButton = new QPushButton(tr("Refresh"), this);
    layout->addWidget(m_refreshButton);
    registerInteractive(m_refreshButton, true);

    // New: Settings + AAC Access buttons
    auto* settingsButton = new QPushButton(tr("Settings"), this);
    layout->addWidget(settingsButton);
    registerInteractive(settingsButton);

    auto* aacButton = new QPushButton(tr("AAC Access"), this);
    layout->addWidget(aacButton);
    registerInteractive(aacButton);

    connect(m_refreshButton, &QPushButton::clicked,
            this, &ChannelListScreen::refreshRequested);

    connect(settingsButton, &QPushButton::clicked,
            this, &ChannelListScreen::settingsRequested);

    connect(aacButton, &QPushButton::clicked,
            this, &ChannelListScreen::aacSettingsRequested);

    connect(m_list, &QListWidget::itemActivated,
            this, [this](QListWidgetItem* item) {
                emit joinChannelRequested(item->data(Qt::UserRole).toInt());
            });

    if (m_aac) {
        connect(m_aac, &AACAccessibilityManager::modesChanged,
                this, [this](const AACModeFlags&) { updateRowHeight(); });
    }

    updateRowHeight();
}

void ChannelListScreen::setChannels(const QList<ChannelInfo>& channels)
{
    m_list->clear();

    for (const ChannelInfo& ci : channels) {
        auto* item = new QListWidgetItem(ci.name, m_list);
        item->setData(Qt::UserRole, ci.id);
    }
}

void ChannelListScreen::updateRowHeight()
{
    if (!m_list || !m_aac)
        return;

    const bool large = m_aac->modes().largeTargets;

    if (large) {
        m_list->setStyleSheet(QStringLiteral(
            "QListWidget::item { min-height: %1px; }"
        ).arg(AACLayoutEngine::AAC_MIN_TARGET));
    } else {
        m_list->setStyleSheet(QStringLiteral(
            "QListWidget::item { min-height: 32px; }"
        ));
    }
}
