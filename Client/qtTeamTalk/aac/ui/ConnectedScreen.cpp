#include "ConnectedScreen.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>

ConnectedScreen::ConnectedScreen(QWidget* parent)
    : QWidget(parent)
{
    auto root = new QVBoxLayout(this);

    auto title = new QLabel("Select a channel", this);
    title->setAlignment(Qt::AlignCenter);
    root->addWidget(title);

    //
    // Scrollable AAC-friendly channel list
    //
    auto scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto container = new QWidget(this);
    m_channelListLayout = new QVBoxLayout(container);
    m_channelListLayout->setSpacing(24);   // large touch targets
    m_channelListLayout->setContentsMargins(24, 24, 24, 24);

    scroll->setWidget(container);
    root->addWidget(scroll);

    //
    // TEMPORARY: Populate with placeholder channels
    // Later, MainWindow will call a method to update this list dynamically.
    //
    auto addChannel = [&](int id, const QString& name) {
        auto btn = new QPushButton(name, this);
        btn->setMinimumHeight(80);          // AAC large target
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        connect(btn, &QPushButton::clicked, this, [this, id]() {
            emit joinChannelRequested(id);
        });

        m_channelListLayout->addWidget(btn);
    };

    addChannel(1, "General Chat");
    addChannel(2, "Music Room");
    addChannel(3, "Gaming");
    addChannel(4, "AFK");

    m_channelListLayout->addStretch();
void ConnectedScreen::setChannels(const QList<QPair<int, QString>>& channels)
{
    // Clear old items
    QLayoutItem* item;
    while ((item = m_channelListLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Add new channels
    for (const auto& ch : channels) {
        int id = ch.first;
        QString name = ch.second;

        auto btn = new QPushButton(name, this);
        btn->setMinimumHeight(80);

        connect(btn, &QPushButton::clicked, this, [this, id]() {
            emit joinChannelRequested(id);
        });

        m_channelListLayout->addWidget(btn);
    }

    m_channelListLayout->addStretch();
}
