#include "AACSettingsScreen.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

AACSettingsScreen::AACSettingsScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreen(aac, parent)
{
    auto* layout = new QVBoxLayout(this);

    auto addCheck = [&](const QString& label, QCheckBox*& out) {
        out = new QCheckBox(label, this);
        layout->addWidget(out);
        registerInteractive(out);
    };

    addCheck(tr("Large targets"), m_largeTargets);
    addCheck(tr("Dwell to activate"), m_dwell);
    addCheck(tr("Switch scanning"), m_scanning);
    addCheck(tr("Auditory feedback"), m_auditory);
    addCheck(tr("Haptic feedback"), m_haptics);
    addCheck(tr("Ultra-minimal mode"), m_ultraMinimal);
    addCheck(tr("One-hand layout"), m_oneHand);
    addCheck(tr("Predictive action strip"), m_predictive);

    auto* oneHandRow = new QHBoxLayout();
    oneHandRow->addWidget(new QLabel(tr("One-hand side:"), this));
    m_oneHandSide = new QComboBox(this);
    m_oneHandSide->addItem(tr("Right"), true);
    m_oneHandSide->addItem(tr("Left"), false);
    oneHandRow->addWidget(m_oneHandSide);
    layout->addLayout(oneHandRow);
    registerInteractive(m_oneHandSide);

    auto* backBtn = new QPushButton(tr("Back"), this);
    layout->addWidget(backBtn);
    registerInteractive(backBtn, true);

    connect(backBtn, &QPushButton::clicked, this, &AACSettingsScreen::backRequested);

    if (m_aac) {
        connect(m_aac, &AACAccessibilityManager::modesChanged,
                this, &AACSettingsScreen::onModesChanged);
        onModesChanged(m_aac->modes());
    }

    connect(m_largeTargets, &QCheckBox::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_dwell, &QCheckBox::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_scanning, &QCheckBox::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_auditory, &QCheckBox::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_haptics, &QCheckBox::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_ultraMinimal, &QCheckBox::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_oneHand, &QCheckBox::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_predictive, &QCheckBox::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_oneHandSide, &QComboBox::currentIndexChanged, this, &AACSettingsScreen::applyToManager);
}

void AACSettingsScreen::onModesChanged(const AACModeFlags& modes)
{
    m_largeTargets->setChecked(modes.largeTargets);
    m_dwell->setChecked(modes.dwell);
    m_scanning->setChecked(modes.scanning);
    m_auditory->setChecked(modes.auditoryFeedback);
    m_haptics->setChecked(modes.hapticFeedback);
    m_ultraMinimal->setChecked(modes.ultraMinimal);
    m_oneHand->setChecked(modes.oneHandLayout);
    m_predictive->setChecked(modes.predictiveStrip);
}

void AACSettingsScreen::applyToManager()
{
    if (!m_aac)
        return;

    AACModeFlags modes = m_aac->modes();
    modes.largeTargets = m_largeTargets->isChecked();
    modes.dwell = m_dwell->isChecked();
    modes.scanning = m_scanning->isChecked();
    modes.auditoryFeedback = m_auditory->isChecked();
    modes.hapticFeedback = m_haptics->isChecked();
    modes.ultraMinimal = m_ultraMinimal->isChecked();
    modes.oneHandLayout = m_oneHand->isChecked();
    modes.predictiveStrip = m_predictive->isChecked();
    m_aac->setModes(modes);

    AACLayoutConfig lc = m_aac->layoutConfig();
    lc.oneHandRightSide = m_oneHandSide->currentData().toBool();
    m_aac->setLayoutConfig(lc);
}
