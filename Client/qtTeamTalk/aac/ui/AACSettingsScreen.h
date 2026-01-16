#pragma once

#include "AACScreen.h"

class QCheckBox;
class QComboBox;

class AACSettingsScreen : public AACScreen {
    Q_OBJECT
public:
    explicit AACSettingsScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

signals:
    void backRequested();

private slots:
    void onModesChanged(const AACModeFlags& modes);

private:
    QCheckBox* m_largeTargets = nullptr;
    QCheckBox* m_dwell = nullptr;
    QCheckBox* m_scanning = nullptr;
    QCheckBox* m_auditory = nullptr;
    QCheckBox* m_haptics = nullptr;
    QCheckBox* m_ultraMinimal = nullptr;
    QCheckBox* m_oneHand = nullptr;
    QCheckBox* m_predictive = nullptr;

    QComboBox* m_oneHandSide = nullptr;

    void applyToManager();
};
