#include "AAPredictiveStrip.h"
#include "AACAccessibilityManager.h"

#include <QPainter>
#include <QMouseEvent>

AAPredictiveStrip::AAPredictiveStrip(AACAccessibilityManager* aac, QWidget* parent)
    : QWidget(parent),
      m_aac(aac)
{
    setMinimumHeight(48);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Integrate with dwell/scanning
    if (m_aac)
        m_aac->registerInteractive(this, true);
}

void AAPredictiveStrip::setSuggestions(const QStringList& words)
{
    m_words = words;
    update();

    // Notify keyboard and other components
    emit suggestionsUpdated(m_words);
}

void AAPredictiveStrip::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int count = m_words.size();
    if (count == 0)
        return;

    const int w = width() / count;
    const int h = height();

    for (int i = 0; i < count; ++i) {
        QRect rect(i * w, 0, w, h);

        // Background
        p.setBrush(QColor(240, 240, 240));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(rect.adjusted(4, 4, -4, -4), 6, 6);

        // Text
        p.setPen(Qt::black);
        p.drawText(rect, Qt::AlignCenter, m_words[i]);
    }
}

void AAPredictiveStrip::mousePressEvent(QMouseEvent* e)
{
    if (m_words.isEmpty())
        return;

    const int count = m_words.size();
    const int w = width() / count;

    int index = e->x() / w;
    if (index >= 0 && index < m_words.size()) {
        const QString word = m_words[index];
        emit suggestionActivated(word);

        // Accessibility: treat this as an activation event
        if (m_aac)
            m_aac->announce(word);
    }
}
