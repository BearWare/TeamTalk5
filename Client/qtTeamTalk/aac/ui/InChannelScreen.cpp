#include "InChannelScreen.h"

#include <QLabel>
#include <QHBoxLayout>

#include "backend/BackendAdapter.h"
#include "aac/AACMainScreen.h"

InChannelScreen::InChannelScreen(AACAccessibilityManager* aac,
                                 BackendAdapter* backend,
                                 QWidget* parent)
    : QWidget(parent)
    , m_aac(aac)
    , m_backend(backend)
{
    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(8, 8, 8, 8);
    m_rootLayout->setSpacing(8);

    m_channelLabel = new QLabel(tr("Channel"), this);
    m_eventLabel   = new QLabel(this);

    m_aacMain = new AACMainScreen(m_aac, this);
    connect(m_aacMain, &AACMainScreen::textCommitted,
            this, &InChannelScreen::onTextCommitted);

    QHBoxLayout* controls = new QHBoxLayout();
    m_leaveButton = new QPushButton(tr("Leave"), this);
    m_pttButton   = new QPushButton(tr("PTT"), this);
    m_pttButton->setCheckable(true);

    connect(m_leaveButton, &QPushButton::clicked,
            this, &InChannelScreen::onLeaveClicked);
    connect(m_pttButton, &QPushButton::toggled,
            this, &InChannelScreen::onPTToggled);

    controls->addWidget(m_leaveButton);
    controls->addWidget(m_pttButton);
    controls->addStretch(1);

    m_rootLayout->addWidget(m_channelLabel);
    m_rootLayout->addWidget(m_eventLabel);
    m_rootLayout->addWidget(m_aacMain);
    m_rootLayout->addLayout(controls);
    m_rootLayout->addStretch(1);
}

QList<QWidget*> InChannelScreen::interactiveWidgets() const
{
    QList<QWidget*> out;
    out << const_cast<AACMainScreen*>(m_aacMain);
    out << const_cast<QPushButton*>(m_leaveButton);
    out << const_cast<QPushButton*>(m_pttButton);
    return out;
}

QList<QWidget*> InChannelScreen::primaryWidgets() const
{
    QList<QWidget*> out;
    out << const_cast<AACMainScreen*>(m_aacMain);
    return out;
}

QLayout* InChannelScreen::rootLayout() const
{
    return m_rootLayout;
}

QWidget* InChannelScreen::predictiveStripContainer() const
{
    return m_aacMain->predictiveStripContainer();
}

void InChannelScreen::setChannelName(const QString& name)
{
    m_channelLabel->setText(tr("Channel: %1").arg(name));
}

void InChannelScreen::updateSelfVoiceState(const SelfVoiceState& state)
{
    Q_UNUSED(state);
    // hook for visual feedback if desired
}

void InChannelScreen::updateOtherUserVoiceState(const OtherUserVoiceEvent& event)
{
    Q_UNUSED(event);
    // hook for visual feedback if desired
}

void InChannelScreen::setEventMessage(const QString& msg)
{
    m_eventLabel->setText(msg);
}

void InChannelScreen::onTextCommitted(const QString& text)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty())
        return;

    if (m_aac && m_aac->speechEngine())
        m_aac->speechEngine()->speak(trimmed);

    if (m_backend)
        m_backend->sendChannelMessage(trimmed);
}

void InChannelScreen::onLeaveClicked()
{
    emit leaveChannelRequested();
}

void InChannelScreen::onPTToggled(bool checked)
{
    emit transmitToggled(checked);
}
