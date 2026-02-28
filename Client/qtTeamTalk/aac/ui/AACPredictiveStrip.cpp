#include "PredictiveStrip.h"

#include "AACFramework.h"          // AACAccessibilityManager, AACButton, AACLayoutEngine, AACModeFlags
#include "AACPredictionEngine.h"   // AACPredictionEngine
#include "AACTextBar.h"            // AACTextBar

#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSizePolicy>
#include <QHash>
#include <QStringList>
#include <QTimer>

PredictiveStrip::PredictiveStrip(QWidget* parent)
    : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(8);

    m_debounceTimer.setSingleShot(true);
    connect(&m_debounceTimer, &QTimer::timeout,
            this, &PredictiveStrip::debouncedUpdate);
}

void PredictiveStrip::setManager(AACAccessibilityManager* mgr)
{
    m_mgr = mgr;
}

void PredictiveStrip::setTextBar(AACTextBar* tb)
{
    m_textBar = tb;
}

void PredictiveStrip::setContext(const QString& text)
{
    if (!m_mgr || !m_mgr->predictionEnabled())
        return;

    m_pendingContext = text;
    m_debounceTimer.start(50); // 40–60 ms: smooth for AAC users
}

void PredictiveStrip::onCharacterTyped(const QString& /*ch*/)
{
    if (!m_textBar)
        return;

    setContext(m_textBar->text());
}

void PredictiveStrip::debouncedUpdate()
{
    if (!m_mgr || !m_mgr->predictionEnabled())
        return;

    AACPredictionEngine* engine = m_mgr->predictionEngine();
    if (!engine)
        return;

    const std::string prefix = m_pendingContext.toStdString();
    auto suggestions = engine->Predict(prefix, 5);

    updateButtons(suggestions);
}

void PredictiveStrip::updateButtons(const std::vector<std::string>& suggestions)
{
    // Convert to QString list for easier comparison
    QStringList newList;
    newList.reserve(static_cast<int>(suggestions.size()));
    for (const auto& s : suggestions)
        newList << QString::fromStdString(s);

    // Map existing buttons by their text so we can reuse them
    QHash<QString, AACButton*> oldMap;
    for (AACButton* btn : m_buttons)
        oldMap.insert(btn->text(), btn);

    // Clear layout items but do not delete widgets yet
    while (m_layout->count() > 0) {
        QLayoutItem* item = m_layout->takeAt(0);
        delete item;
    }

    QList<AACButton*> newButtons;
    newButtons.reserve(newList.size());

    // Helper: micro-delayed fade-out and delete a button
    auto fadeOutAndDelete = [](AACButton* btn) {
        if (!btn)
            return;

        QTimer::singleShot(40, btn, [btn]() {
            auto* effect = qobject_cast<QGraphicsOpacityEffect*>(btn->graphicsEffect());
            if (!effect) {
                effect = new QGraphicsOpacityEffect(btn);
                effect->setOpacity(1.0);
                btn->setGraphicsEffect(effect);
            }

            auto* fade = new QPropertyAnimation(effect, "opacity", btn);
            fade->setDuration(100);
            fade->setStartValue(1.0);
            fade->setEndValue(0.0);
            fade->setEasingCurve(QEasingCurve::OutQuad);

            QObject::connect(fade, &QPropertyAnimation::finished, btn, &QObject::deleteLater);
            fade->start(QAbstractAnimation::DeleteWhenStopped);
        });
    };

    // Build the new strip, reusing buttons where possible
    for (const QString& word : newList) {
        AACButton* btn = nullptr;

        if (oldMap.contains(word)) {
            // Reuse existing button
            btn = oldMap.value(word);
            oldMap.remove(word);
        } else {
            // Create new button
            btn = new AACButton(m_mgr, this);
            btn->setText(word);
            btn->setDeepWell(true); // full dwell semantics
            btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

            // Dynamic dwell override based on word length
            if (m_mgr) {
                int base = m_mgr->dwellConfig().dwellDurationMs;
                int len  = word.length();
                int overrideMs = base;

                if (len <= 2)      overrideMs = static_cast<int>(base * 0.65);
                else if (len <= 4) overrideMs = static_cast<int>(base * 0.75);
                else if (len <= 7) overrideMs = static_cast<int>(base * 0.85);

                btn->setDwellOverrideMs(overrideMs);
            }

            // Confidence-based visual weighting + tooltip
            if (m_mgr && m_mgr->predictionEngine()) {
                float conf = m_mgr->predictionEngine()->confidenceFor(word.toStdString());
                if (conf < 0.0f) conf = 0.0f;
                if (conf > 1.0f) conf = 1.0f;

                int shade = 232 - static_cast<int>(conf * 24); // 232 → 208
                int focusShade = shade - 8;

                btn->setStyleSheet(QString(
                    "AACButton { "
                    "  background-color: rgb(%1, %1, %1); "
                    "  border-radius: 12px; "
                    "  border: 1px solid #c8c8c8; "
                    "} "
                    "AACButton:focus { "
                    "  background-color: rgb(%2, %2, %2); "
                    "}"
                ).arg(shade).arg(focusShade));

                btn->setToolTip(QString("Prediction confidence: %1%")
                                    .arg(conf * 100.0f, 0, 'f', 1));
            } else {
                // Fallback subtle visual distinction
                btn->setStyleSheet(
                    "AACButton { "
                    "  background-color: #e8e8e8; "
                    "  border-radius: 12px; "
                    "  border: 1px solid #c8c8c8; "
                    "} "
                    "AACButton:focus { "
                    "  background-color: #dcdcdc; "
                    "}"
                );
            }

            applyAdaptiveSizing(btn);

            connect(btn, &QPushButton::clicked, this, [this, word]() {
                emit suggestionChosen(word);
                if (m_mgr && m_mgr->predictionEngine()) {
                    const QString trimmed = word.trimmed();
                    if (!trimmed.isEmpty())
                        m_mgr->predictionEngine()->learnUtterance(trimmed);
                }
            });

            // Pinning: long dwell-hold pins the suggestion
            connect(btn, &AACButton::dwellHoldActivated, this, [this, word, btn]() {
                btn->setProperty("pinned", true);
                btn->setStyleSheet(
                    "AACButton { "
                    "  background-color: #d0e8ff; "
                    "  border-radius: 12px; "
                    "  border: 2px solid #7ab8ff; "
                    "} "
                    "AACButton:focus { "
                    "  background-color: #c0dcff; "
                    "}"
                );
            });

            // Animate new buttons in
            animateButtonIn(btn);
        }

        m_layout->addWidget(btn);
        newButtons.append(btn);
    }

    // Any buttons left in oldMap were not reused → fade them out and delete,
    // unless they are pinned
    for (AACButton* leftover : oldMap.values()) {
        if (leftover->property("pinned").toBool())
            continue;
        fadeOutAndDelete(leftover);
    }

    m_buttons = newButtons;
    m_layout->addStretch();
}

void PredictiveStrip::applyAdaptiveSizing(AACButton* btn)
{
    if (!m_mgr)
        return;

    const AACModeFlags modes = m_mgr->modes();
    if (!modes.largeTargets)
        return;

    // Match AACLayoutEngine defaults
    btn->setMinimumSize(AACLayoutEngine::AAC_MIN_TARGET,
                        AACLayoutEngine::AAC_MIN_TARGET);

    QFont f = btn->font();
    f.setPointSizeF(f.pointSizeF() * AACLayoutEngine::AAC_FONT_SCALE);
    btn->setFont(f);
}

void PredictiveStrip::animateButtonIn(AACButton* btn)
{
    // Opacity effect
    auto* effect = new QGraphicsOpacityEffect(btn);
    effect->setOpacity(0.0);
    btn->setGraphicsEffect(effect);

    auto* fade = new QPropertyAnimation(effect, "opacity", btn);
    fade->setDuration(120);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    fade->setEasingCurve(QEasingCurve::OutQuad);
    fade->start(QAbstractAnimation::DeleteWhenStopped);

    // Slide-in (small vertical offset)
    QPoint startPos = btn->pos() + QPoint(0, 6);
    QPoint endPos   = btn->pos();

    auto* slide = new QPropertyAnimation(btn, "pos", btn);
    slide->setDuration(120);
    slide->setStartValue(startPos);
    slide->setEndValue(endPos);
    slide->setEasingCurve(QEasingCurve::OutQuad);
    slide->start(QAbstractAnimation::DeleteWhenStopped);
}
