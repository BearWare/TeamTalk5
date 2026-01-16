#pragma once

#include <QWidget>
#include <QPointer>

//
// AACScreen
// ----------
// Base class for all AAC‑native UI screens.
// Provides default Large‑Target Mode behaviour (interactive controls only).
//

class AACScreen : public QWidget {
    Q_OBJECT
public:
    explicit AACScreen(QWidget* parent = nullptr);

public slots:
    // Called by MainWindow whenever Large‑Target Mode changes.
    virtual void applyLargeTargetMode(bool enabled);

protected:
    // Utility: scale a single interactive widget (buttons, fields, etc.)
    void scaleInteractiveWidget(QWidget* w, bool enabled);

    // Utility: scale layout spacing/margins
    void scaleLayout(QLayout* layout, bool enabled);

    // Constants for Large‑Target Mode
    static constexpr int AAC_MIN_TARGET = 140;     // px
    static constexpr int AAC_MIN_SPACING = 24;     // px
    static constexpr qreal AAC_FONT_SCALE = 1.3;   // multiplier
};

