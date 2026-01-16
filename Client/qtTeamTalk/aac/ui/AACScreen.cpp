#include "AACScreen.h"
#include <QLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QAbstractButton>
#include <QFont>

AACScreen::AACScreen(QWidget* parent)
    : QWidget(parent)
{
}

//
// Default Large‑Target Mode implementation
// ----------------------------------------
// This scales:
//   - QPushButton
//   - QLineEdit
//   - QSpinBox
//   - Any QAbstractButton
//   - Layout spacing/margins
//
// It does NOT scale:
//   - QLabel
//   - Informational banners
//   - Non‑interactive text
//

void AACScreen::applyLargeTargetMode(bool enabled)
{
    //
    // Scale all child widgets that are interactive
    //
    const auto children = findChildren<QWidget*>();
    for (QWidget* w : children) {
        scaleInteractiveWidget(w, enabled);
    }

    //
    // Scale all layouts
    //
    if (auto* lay = layout())
        scaleLayout(lay, enabled);
}

//
// Scale a single interactive widget
//

void AACScreen::scaleInteractiveWidget(QWidget* w, bool enabled)
{
    if (!w)
        return;

    const bool isButton = qobject_cast<QAbstractButton*>(w) != nullptr;
    const bool isLineEdit = qobject_cast<QLineEdit*>(w) != nullptr;
    const bool isSpinBox = qobject_cast<QSpinBox*>(w) != nullptr;

    if (!isButton && !isLineEdit && !isSpinBox)
        return; // ignore non‑interactive widgets

    if (enabled) {
        w->setMinimumSize(AAC_MIN_TARGET, AAC_MIN_TARGET);

        QFont f = w->font();
        f.setPointSizeF(f.pointSizeF() * AAC_FONT_SCALE);
        w->setFont(f);
    } else {
        // Reset to default minimums
        w->setMinimumSize(0, 0);

        // Reset font to default (Qt will resolve)
        QFont f = w->font();
        f.setPointSizeF(f.pointSizeF() / AAC_FONT_SCALE);
        w->setFont(f);
    }
}

//
// Scale layout spacing + margins
//

void AACScreen::scaleLayout(QLayout* lay, bool enabled)
{
    if (!lay)
        return;

    if (enabled) {
        lay->setSpacing(AAC_MIN_SPACING);
        lay->setContentsMargins(
            AAC_MIN_SPACING,
            AAC_MIN_SPACING,
            AAC_MIN_SPACING,
            AAC_MIN_SPACING
        );
    } else {
        lay->setSpacing(8);
        lay->setContentsMargins(8, 8, 8, 8);
    }
}
