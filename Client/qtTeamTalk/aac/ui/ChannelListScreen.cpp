#include "ChannelListScreen.h"

#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QListWidgetItem>
#include <QFont>

ChannelListScreen::ChannelListScreen(QWidget* parent)
    : AACScreen(parent)
{
    auto* layout = new QVBoxLayout(this);

    m_list = new QListWidget(this);
    m_refreshButton = new QPushButton("Refresh", this);

    layout->addWidget(m_list);
    layout->addWidget(m_refreshButton);

    connect(m_refreshButton, &QPushButton::clicked,
            this, &ChannelListScreen::refreshRequested);

    connect(m_list, &QListWidget::itemActivated,
            this, [this](QListWidgetItem* item) {
                emit joinChannelRequested(item->data(Qt::UserRole).toInt());
            });
}

void ChannelListScreen::setChannels(const QList<ChannelInfo>& channels)
{
    m_list->clear();

    for (const ChannelInfo& ci : channels) {
        auto* item = new QListWidgetItem(ci.name, m_list);
        item->setData(Qt::UserRole, ci.id);
    }
}

//
// Large‑Target Mode
// -----------------
// We rely on AACScreen for:
//   - scaling the Refresh button
//   - scaling layout spacing/margins
//
// But we add custom behaviour for:
//   - list item minimum height
//

void ChannelListScreen::applyLargeTargetMode(bool enabled)
{
    // First apply default AAC scaling
    AACScreen::applyLargeTargetMode(enabled);

    //
    // Now apply custom row height scaling
    //
    if (m_list) {
        if (enabled) {
            // Increase row height for motor accessibility
            m_list->setStyleSheet(QStringLiteral(
                "QListWidget::item { min-height: %1px; }"
            ).arg(AAC_MIN_TARGET));
        } else {
            // Reset to default
            m_list->setStyleSheet(QStringLiteral(
                "QListWidget::item { min-height: 32px; }"
            ));
        }
    }
}
