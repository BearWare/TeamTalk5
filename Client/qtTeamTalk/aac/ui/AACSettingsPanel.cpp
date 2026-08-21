#include "AACFramework.h"
#include <QSlider>
#include <QCheckBox>
#include <QFormLayout>

AACSettingsPanel::AACSettingsPanel(AACAccessibilityManager* aac, QWidget* parent)
    : QWidget(parent)
    , m_aac(aac)
{
    auto* lay = new QFormLayout(this);

    m_dwellSlider = new QSlider(Qt::Horizontal, this);
    m_dwellSlider->setRange(200, 2000);
    m_dwellSlider->setValue(aac->dwellConfig().dwellDurationMs);
    connect(m_dwellSlider, &QSlider::valueChanged,
            this, &AACSettingsPanel::onDwellChanged);
    lay->addRow(tr("Dwell (ms)"), m_dwellSlider);

    m_scanningSlider = new QSlider(Qt::Horizontal, this);
    m_scanningSlider->setRange(200, 3000);
    m_scanningSlider->setValue(aac->scanningConfig().stepIntervalMs);
    connect(m_scanningSlider, &QSlider::valueChanged,
            this, &AACSettingsPanel::onScanningChanged);
    lay->addRow(tr("Scanning (ms)"), m_scanningSlider);

    m_largeTargets = new QCheckBox(tr("Large targets"), this);
    m_largeTargets->setChecked(aac->modes().largeTargets);
    connect(m_largeTargets, &QCheckBox::toggled,
            this, &AACSettingsPanel::onLargeTargetsToggled);
    lay->addRow(m_largeTargets);

    m_auditory = new QCheckBox(tr("Auditory feedback"), this);
    m_auditory->setChecked(aac->modes().auditoryFeedback);
    connect(m_auditory, &QCheckBox::toggled,
            this, &AACSettingsPanel::onAuditoryToggled);
    lay->addRow(m_auditory);

    m_haptic = new QCheckBox(tr("Haptic feedback"), this);
    m_haptic->setChecked(aac->modes().hapticFeedback);
    connect(m_haptic, &QCheckBox::toggled,
            this, &AACSettingsPanel::onHapticToggled);
    lay->addRow(m_haptic);
}

void AACSettingsPanel::onDwellChanged(int ms)
{
    if (!m_aac) return;
    AACDwellConfig cfg = m_aac->dwellConfig();
    cfg.dwellDurationMs = ms;
    m_aac->setDwellConfig(cfg);
}

void AACSettingsPanel::onScanningChanged(int ms)
{
    if (!m_aac) return;
    AACScanningConfig cfg = m_aac->scanningConfig();
    cfg.stepIntervalMs = ms;
    m_aac->setScanningConfig(cfg);
}

void AACSettingsPanel::onLargeTargetsToggled(bool on)
{
    if (!m_aac) return;
    AACModeFlags m = m_aac->modes();
    m.largeTargets = on;
    m_aac->setModes(m);
}

void AACSettingsPanel::onAuditoryToggled(bool on)
{
    if (!m_aac) return;
    AACModeFlags m = m_aac->modes();
    m.auditoryFeedback = on;
    m_aac->setModes(m);
}

void AACSettingsPanel::onHapticToggled(bool on)
{
    if (!m_aac) return;
    AACModeFlags m = m_aac->modes();
    m.hapticFeedback = on;
    m_aac->setModes(m);
}
