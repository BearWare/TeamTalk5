#include "ChannelListScreen.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>

ChannelListScreen::ChannelListScreen(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    m_list = new QListWidget(this);

    m_refreshButton = new QPushButton("Refresh Channels", this);
    m_refreshButton->setMinimumHeight(60);

    m_joinButton = new QPushButton("Join Selected Channel", this);
    m_joinButton->setMinimumHeight(60);

    layout->addWidget(m_list);
    layout->addWidget(m_refreshButton);
    layout->addWidget(m_joinButton);

    connect(m_refreshButton, &QPushButton::clicked,
            this, &ChannelListScreen::refreshRequested);

    connect(m_joinButton, &QPushButton::clicked, this, [this]() {
        auto* item = m_list->currentItem();
        if (!item) return;
        int channelId = item->data(Qt::UserRole).toInt();
        emit joinChannelRequested(channelId);
    });
}

void ChannelListScreen::setChannels(const QList<QPair<int, QString>>& channels)
{
    m_list->clear();
    for (const auto& pair : channels) {
        auto* item = new QListWidgetItem(pair.second);
        item->setData(Qt::UserRole, pair.first);
        m_list->addItem(item);
    }
}
