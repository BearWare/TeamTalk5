#include "channelview.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QPushButton>
#include <QLabel>

ChannelView::ChannelView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupConnections();
}

void ChannelView::setupUi() {
    auto *layout = new QVBoxLayout(this);

    m_titleLabel = new QLabel(tr("Channels"), this);
    m_channelList = new QListView(this);

    m_joinButton = new QPushButton(tr("Join"), this);
    m_backButton = new QPushButton(tr("Back"), this);
    m_refreshButton = new QPushButton(tr("Refresh"), this);

    auto *buttonRow = new QHBoxLayout;
    buttonRow->addWidget(m_backButton);
    buttonRow->addStretch();
    buttonRow->addWidget(m_refreshButton);
    buttonRow->addWidget(m_joinButton);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_channelList);
    layout->addLayout(buttonRow);
}

void ChannelView::setupConnections() {
    connect(m_backButton, &QPushButton::clicked,
            this, &ChannelView::backToServersRequested);

    connect(m_refreshButton, &QPushButton::clicked,
            this, &ChannelView::refreshRequested);

    connect(m_joinButton, &QPushButton::clicked, this, [this]() {
        auto index = m_channelList->currentIndex();
        if (index.isValid())
            emit joinChannelRequested(index);
    });
}

void ChannelView::setChannelModel(QAbstractItemModel *model) {
    m_channelList->setModel(model);
}

void ChannelView::setServerLabel(const QString &label) {
    m_titleLabel->setText(tr("Channels on %1").arg(label));
}
