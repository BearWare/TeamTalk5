#include "AACFramework.h"
#include <QPainter>
#include <QMouseEvent>

AACPredictiveStrip::AACPredictiveStrip(AACAccessibilityManager* aac, QWidget* parent)
    : QWidget(parent)
    , m_aac(aac)
{
    setMinimumHeight(40);
}

void AACPredictiveStrip::setSuggestions(const QStringList& words)
{
    m_words = words;
    update();
}

void AACPredictiveStrip::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);

    if (m_words.isEmpty())
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    int count = m_words.size();
    if (count <= 0)
        return;

    int w = width() / count;
    for (int i = 0; i < count; ++i) {
        QRect r(i * w, 0, w, height());
        p.fillRect(r.adjusted(2, 2, -2, -2), QColor("#F0F0F0"));
        p.setPen(Qt::black);
        p.drawText(r, Qt::AlignCenter, m_words.at(i));
    }
}

void AACPredictiveStrip::mousePressEvent(QMouseEvent* e)
{
    if (m_words.isEmpty())
        return;

    int count = m_words.size();
    int w = width() / count;
    int index = e->pos().x() / w;
    if (index >= 0 && index < count)
        emit suggestionActivated(m_words.at(index));

    QWidget::mousePressEvent(e);
}
