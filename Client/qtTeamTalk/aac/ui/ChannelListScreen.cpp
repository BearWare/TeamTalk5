#include "ChannelListScreen.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>

ChannelListScreen::ChannelListScreen(QWidget* parent)
    : QWidget(parent)
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
