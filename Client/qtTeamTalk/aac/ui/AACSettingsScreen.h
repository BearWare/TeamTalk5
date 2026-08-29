#pragma once

#include "AACScreen.h"

class QCheckBox;
class QComboBox;
class QPushButton;

// Full AAC Settings Screen:
// - 8 AAC mode toggles
// - One‑hand side selector
// - Presets (Touch, Eye‑Gaze, Switch, Cognitive‑Low)
// - Reset to defaults
// - Back button
class AACSettingsScreen : public AACScreen {
    Q_OBJECT
public:
    explicit AACSettingsScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

signals:
    void backRequested();

private slots:
    void onModesChanged(const AACModeFlags& modes);
    void applyToManager();

    void applyRecommendedDefaults();
    void applyPresetTouch();
    void applyPresetEyeGaze();
    void applyPresetSwitch();
    void applyPresetCognitiveLow();

private:
    QCheckBox* m_largeTargets   = nullptr;
    QCheckBox* m_dwell          = nullptr;
    QCheckBox* m_scanning       = nullptr;
    QCheckBox* m_auditory       = nullptr;
    QCheckBox* m_haptics        = nullptr;
    QCheckBox* m_ultraMinimal   = nullptr;
    QCheckBox* m_oneHand        = nullptr;
    QCheckBox* m_predictive     = nullptr;

    QComboBox* m_oneHandSide    = nullptr;

    QPushButton* m_resetButton          = nullptr;
    QPushButton* m_presetTouch          = nullptr;
    QPushButton* m_presetEyeGaze        = nullptr;
    QPushButton* m_presetSwitch         = nullptr;
    QPushButton* m_presetCognitiveLow   = nullptr;
    QPushButton* m_backButton           = nullptr;
};
