#include "InChannelScreen.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QIcon>
#include <QFont>
#include <QResizeEvent>

InChannelScreen::InChannelScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreen(aac, parent)
{
    auto* rootLayout = new QVBoxLayout(this);

    m_channelLabel = new QLabel(tr("In Channel"), this);
    m_channelLabel->setAlignment(Qt::AlignCenter);
    m_channelLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    rootLayout->addWidget(m_channelLabel);

    m_speakingBanner = new QLabel("", this);
    m_speakingBanner->setAlignment(Qt::AlignCenter);
    m_speakingBanner->setStyleSheet("font-size: 16px; color: #00aa00;");
    rootLayout->addWidget(m_speakingBanner);

    m_quietBanner = new QLabel("", this);
    m_quietBanner->setAlignment(Qt::AlignCenter);
    m_quietBanner->setStyleSheet("font-size: 14px; color: #888;");
    rootLayout->addWidget(m_quietBanner);

    m_participantCountLabel = new QLabel("", this);
    m_participantCountLabel->setAlignment(Qt::AlignCenter);
    m_participantCountLabel->setStyleSheet("font-size: 14px; color: #cccccc;");
    rootLayout->addWidget(m_participantCountLabel);

    m_eventBanner = new QLabel("", this);
    m_eventBanner->setAlignment(Qt::AlignCenter);
    m_eventBanner->setStyleSheet("font-size: 14px; color: #ffaa00;");
    rootLayout->addWidget(m_eventBanner);

    m_participantList = new QListWidget(this);
    m_participantList->setSelectionMode(QAbstractItemView::NoSelection);
    rootLayout->addWidget(m_participantList, 1);
    registerInteractive(m_participantList);

    auto* bottomRow = new QHBoxLayout();

    m_transmitButton = new QPushButton(tr("Transmit"), this);
    m_transmitButton->setCheckable(true);
    bottomRow->addWidget(m_transmitButton, 2);
    registerInteractive(m_transmitButton, true);

    m_leaveButton = new QPushButton(tr("Leave channel"), this);
    bottomRow->addWidget(m_leaveButton, 1);
    registerInteractive(m_leaveButton);

    rootLayout->addLayout(bottomRow);

    connect(m_leaveButton, &QPushButton::clicked,
            this, &InChannelScreen::onLeaveClicked);

    connect(m_transmitButton, &QPushButton::clicked,
            this, &InChannelScreen::onTransmitClicked);

    m_quietTimer.setInterval(1000);
    connect(&m_quietTimer, &QTimer::timeout,
            this, &InChannelScreen::onQuietTimerTick);
    m_quietTimer.start();

    if (m_aac) {
        connect(m_aac, &AACAccessibilityManager::modesChanged,
                this, [this](const AACModeFlags&) { updateRowHeight(); });
    }

    // Floating AAC settings gear
    m_floatingSettingsButton = new QPushButton("⚙", this);
    m_floatingSettingsButton->setFixedSize(64, 64);
    m_floatingSettingsButton->setStyleSheet(
        "border-radius: 32px;"
        "background-color: #444;"
        "color: white;"
        "font-size: 28px;"
    );
    m_floatingSettingsButton->raise();
    m_floatingSettingsButton->move(width() - 80, height() - 80);
    m_floatingSettingsButton->show();

    registerInteractive(m_floatingSettingsButton);

    connect(m_floatingSettingsButton, &QPushButton::clicked,
            this, [this]() { emit settingsRequested(); });

    updateParticipantCount();
    updateTransmitUi();
    updateRowHeight();
}

void InChannelScreen::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    if (m_floatingSettingsButton)
        m_floatingSettingsButton->move(width() - 80, height() - 80);
}

void InChannelScreen::setChannelName(const QString& name)
{
    m_channelLabel->setText(name);
}

void InChannelScreen::onLeaveClicked()
{
    emit leaveChannelRequested();
}

void InChannelScreen::onTransmitClicked()
{
    m_transmitEnabled = m_transmitButton->isChecked();
    emit transmitToggled(m_transmitEnabled);
    updateTransmitUi();
}

void InChannelScreen::updateSelfVoiceState(SelfVoiceState state)
{
    m_selfVoiceState = state;

    if (!m_transmitEnabled) {
        m_transmitUiState = TransmitUiState::Idle;
    } else {
        if (m_selfVoiceState == SelfVoiceState::Transmitting)
            m_transmitUiState = TransmitUiState::Speaking;
        else
            m_transmitUiState = TransmitUiState::Armed;
    }

    updateTransmitUi();
}

void InChannelScreen::updateOtherUserVoiceState(const OtherUserVoiceEvent& event)
{
    Participant p;

    if (m_participants.contains(event.userId)) {
        p = m_participants[event.userId];
    } else {
        p.userId = event.userId;
        p.username = event.username;
        p.voiceState = OtherUserVoiceState::Silent;
        p.lastSpoke = QDateTime();
        p.item = new QListWidgetItem(m_participantList);
        m_participants.insert(event.userId, p);
    }

    Participant& ref = m_participants[event.userId];
    ref.username = event.username;
    ref.voiceState = event.state;

    if (event.state == OtherUserVoiceState::Speaking)
        ref.lastSpoke = QDateTime::currentDateTime();

    updateParticipantItem(ref);
    resortParticipants();
    updateSpeakingBanner();
    updateParticipantCount();
}

void InChannelScreen::clearParticipants()
{
    m_participantList->clear();
    m_participants.clear();
    m_speakingBanner->clear();
    updateParticipantCount();
}

void InChannelScreen::setEventMessage(const QString& message)
{
    m_eventBanner->setText(message);
}

void InChannelScreen::updateTransmitUi()
{
    QString text;
    QString style;

    switch (m_transmitUiState) {
    case TransmitUiState::Idle:
        text = tr("Transmit");
        style = "background-color: #444; color: white; font-size: 18px;";
        m_transmitButton->setChecked(false);
        break;
    case TransmitUiState::Armed:
        text = tr("Ready");
        style = "background-color: #0066cc; color: white; font-size: 18px;";
        m_transmitButton->setChecked(true);
        break;
    case TransmitUiState::Speaking:
        text = tr("Speaking");
        style = "background-color: #00aa00; color: white; font-size: 18px;";
        m_transmitButton->setChecked(true);
        break;
    }

    m_transmitButton->setText(text);
    m_transmitButton->setStyleSheet(style);
}

void InChannelScreen::updateParticipantItem(Participant& p)
{
    if (!p.item)
        return;

    QString label = p.username;

    if (p.voiceState == OtherUserVoiceState::Speaking)
        label += tr("  — speaking");

    if (p.lastSpoke.isValid()) {
        label += tr("  (last spoke: %1)")
                     .arg(p.lastSpoke.time().toString("HH:mm:ss"));
    }

    p.item->setText(label);

    QPixmap bar(12, 32);
    if (p.voiceState == OtherUserVoiceState::Speaking)
        bar.fill(QColor("#00aa00"));
    else
        bar.fill(QColor("#333333"));

    p.item->setIcon(QIcon(bar));

    if (p.voiceState == OtherUserVoiceState::Speaking) {
        p.item->setBackground(QColor("#003300"));
        p.item->setForeground(Qt::white);
        QFont f = p.item->font();
        f.setBold(true);
        p.item->setFont(f);
    } else {
        p.item->setBackground(Qt::black);
        p.item->setForeground(Qt::white);
        QFont f = p.item->font();
        f.setBold(false);
        p.item->setFont(f);
    }
}

void InChannelScreen::resortParticipants()
{
    QList<Participant> list = m_participants.values();

    std::sort(list.begin(), list.end(), [](const Participant& a, const Participant& b) {
        if (a.voiceState == OtherUserVoiceState::Speaking &&
            b.voiceState != OtherUserVoiceState::Speaking)
            return true;
        if (b.voiceState == OtherUserVoiceState::Speaking &&
            a.voiceState != OtherUserVoiceState::Speaking)
            return false;

        if (a.lastSpoke.isValid() && b.lastSpoke.isValid())
            return a.lastSpoke > b.lastSpoke;

        if (a.lastSpoke.isValid() && !b.lastSpoke.isValid())
            return true;
        if (!a.lastSpoke.isValid() && b.lastSpoke.isValid())
            return false;

        return a.username.toLower() < b.username.toLower();
    });

    m_participantList->clear();

    for (Participant& p : list) {
        if (!p.item)
            p.item = new QListWidgetItem(m_participantList);
        else
            m_participantList->addItem(p.item);

        updateParticipantItem(p);
        m_participants[p.userId] = p;
    }
}

void InChannelScreen::updateSpeakingBanner()
{
    QStringList speaking;

    for (const auto& p : m_participants) {
        if (p.voiceState == OtherUserVoiceState::Speaking)
            speaking << p.username;
    }

    if (speaking.isEmpty()) {
        m_speakingBanner->clear();
    } else if (speaking.size() == 1) {
        m_speakingBanner->setText(tr("%1 is speaking").arg(speaking.first()));
    } else {
        m_speakingBanner->setText(tr("Multiple people are speaking"));
    }
}

void InChannelScreen::updateParticipantCount()
{
    const int count = m_participants.size();
    if (count == 0)
        m_participantCountLabel->setText(tr("No other participants"));
    else if (count == 1)
        m_participantCountLabel->setText(tr("1 other participant"));
    else
        m_participantCountLabel->setText(tr("%1 other participants").arg(count));
}

void InChannelScreen::onQuietTimerTick()
{
    bool anyRecent = false;
    const QDateTime now = QDateTime::currentDateTime();

    for (const auto& p : m_participants) {
        if (p.lastSpoke.isValid() &&
            p.lastSpoke.secsTo(now) < 10) {
            anyRecent = true;
            break;
        }
    }

    if (!anyRecent)
        m_quietBanner->setText(tr("Quiet channel"));
    else
        m_quietBanner->clear();
}

void InChannelScreen::updateRowHeight()
{
    if (!m_participantList || !m_aac)
        return;

    const bool large = m_aac->modes().largeTargets;

    if (large) {
        m_participantList->setStyleSheet(QStringLiteral(
            "QListWidget::item { min-height: %1px; }"
        ).arg(AACLayoutEngine::AAC_MIN_TARGET));
    } else {
        m_participantList->setStyleSheet(QStringLiteral(
            "QListWidget::item { min-height: 32px; }"
        ));
    }
}
