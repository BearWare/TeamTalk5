#include "PredictiveStrip.h"
#include "AACAccessibilityManager.h"
#include "AACPredictionEngine.h"
#include "AACButton.h"

#include <QDebug>

PredictiveStrip::PredictiveStrip(AACAccessibilityManager* mgr, QWidget* parent)
    : QWidget(parent),
      m_mgr(mgr)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(4, 4, 4, 4);
    m_layout->setSpacing(8);
}

QString PredictiveStrip::lastWord(const QString& text) const
{
    QString trimmed = text.trimmed();
    if (trimmed.isEmpty())
        return QString();

    QStringList parts = trimmed.split(' ', Qt::SkipEmptyParts);
    return parts.isEmpty() ? QString() : parts.last();
}

void PredictiveStrip::setContext(const QString& text)
{
    m_lastContext = text;

    if (!m_mgr || !m_mgr->predictionEngine())
        return;

    QString key = lastWord(text);
    if (key.isEmpty()) {
        rebuild({});
        return;
    }

    QStringList suggestions =
        m_mgr->predictionEngine()->suggest(text, 5);

    rebuild(suggestions);
}

void PredictiveStrip::onCharacterTyped(QChar ch)
{
    // Optional refinement: just call setContext again
    setContext(m_lastContext + ch);
}

void PredictiveStrip::rebuild(const QStringList& suggestions)
{
    // Clear old buttons
    QLayoutItem* item;
    while ((item = m_layout->takeAt(0)) != nullptr) {
        if (QWidget* w = item->widget())
            w->deleteLater();
        delete item;
    }

    // Add new suggestion buttons
    for (const QString& word : suggestions) {
        auto* btn = new AACButton(m_mgr, this);
        btn->setText(word);
        btn->setMinimumHeight(48);

        connect(btn, &QPushButton::clicked, this, [this, word]() {
            emit suggestionChosen(word);
        });

        m_layout->addWidget(btn);
    }

    // If no suggestions, leave strip empty
}
