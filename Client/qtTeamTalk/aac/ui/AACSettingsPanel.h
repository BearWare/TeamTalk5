#pragma once

#include <QWidget>

class AACAccessibilityManager;
class QSlider;
class QCheckBox;

class AACSettingsPanel : public QWidget {
    Q_OBJECT
public:
    explicit AACSettingsPanel(AACAccessibilityManager* aac, QWidget* parent = nullptr);

private slots:
    void onDwellChanged(int ms);
    void onScanningChanged(int ms);
    void onLargeTargetsToggled(bool on);
    void onAuditoryToggled(bool on);
    void onHapticToggled(bool on);

private:
    AACAccessibilityManager* m_aac = nullptr;
    QSlider* m_dwellSlider = nullptr;
    QSlider* m_scanningSlider = nullptr;
    QCheckBox* m_largeTargets = nullptr;
    QCheckBox* m_auditory = nullptr;
    QCheckBox* m_haptic = nullptr;
};
