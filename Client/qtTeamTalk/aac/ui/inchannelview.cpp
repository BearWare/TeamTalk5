#include "inchannelview.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListView>
#include <QTextEdit>

InChannelView::InChannelView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupConnections();
}

void InChannelView::setupUi() {
    auto *layout = new QVBoxLayout(this);

    m_channelLabel = new QLabel(tr("Channel"), this);

    m_transmitButton = new QPushButton(tr("Transmit"), this);
    m_muteButton = new QPushButton(tr("Mute"), this);
    m_leaveButton = new QPushButton(tr("Leave"), this);
    m_settingsButton = new QPushButton(tr("Settings"), this);

    auto *topRow = new QHBoxLayout;
    topRow->addWidget(m_channelLabel);
    topRow->addStretch();
    topRow->addWidget(m_settingsButton);

    auto *controlRow = new QHBoxLayout;
    controlRow->addWidget(m_transmitButton);
    controlRow->addWidget(m_muteButton);
    controlRow->addStretch();
    controlRow->addWidget(m_leaveButton);

    m_userList = new QListView(this);
    m_chatPlaceholder = new QTextEdit(this);
    m_chatPlaceholder->setReadOnly(true);
    m_chatPlaceholder->setPlainText(tr("Text chat (not implemented yet)."));

    layout->addLayout(topRow);
    layout->addLayout(controlRow);
    layout->addWidget(m_userList);
    layout->addWidget(m_chatPlaceholder);

    updateTransmitVisual(false);
    updateMuteVisual(false);
}

void InChannelView::setupConnections() {
    connect(m_leaveButton, &QPushButton::clicked,
            this, &InChannelView::leaveChannelRequested);

    connect(m_settingsButton, &QPushButton::clicked,
            this, &InChannelView::openSettingsRequested);

    connect(m_transmitButton, &QPushButton::clicked, this, [this]() {
        emit toggleTransmitRequested();
        // PR#7: UI only; logic to update visual will be driven by state machine in later PR.
    });

    connect(m_muteButton, &QPushButton::clicked, this, [this]() {
        emit toggleMuteRequested();
    });
}

void InChannelView::setChannelLabel(const QString &label) {
    m_channelLabel->setText(tr("In %1").arg(label));
}

void InChannelView::setUserListModel(QAbstractItemModel *model) {
    m_userList->setModel(model);
}

void InChannelView::updateTransmitVisual(bool active) {
    m_transmitButton->setText(active ? tr("Stop Transmit") : tr("Transmit"));
}

void InChannelView::updateMuteVisual(bool muted) {
    m_muteButton->setText(muted ? tr("Unmute") : tr("Mute"));
}
