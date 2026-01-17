#include "AACFramework.h"
#include <QPainter>
#include <QPen>
#include <QEnterEvent>
#include <QFocusEvent>
#include <QMouseEvent>

AACUserCard::AACUserCard(AACAccessibilityManager* aac, QWidget* parent)
    : QWidget(parent)
    , m_aac(aac)
{
    setFocusPolicy(Qt::StrongFocus);

    if (m_aac) {
        connect(m_aac->inputController(), &AACInputController::dwellProgressChanged,
                this, [this](QWidget* target, float p) {
                    if (target == this)
                        setDwellProgress(p);
                });
    }
}

void AACUserCard::setDeepWell(bool enabled)
{
    m_deepWell = enabled;
    setProperty("aacDeepWell", enabled);
}

bool AACUserCard::isDeepWell() const { return m_deepWell; }

void AACUserCard::setDwellProgress(float p)
{
    m_dwellProgress = p;
    update();
}

void AACUserCard::enterEvent(QEnterEvent* e)
{
    QWidget::enterEvent(e);
    if (m_aac)
        m_aac->inputController()->startDwellOn(this);
}

void AACUserCard::leaveEvent(QEvent* e)
{
    QWidget::leaveEvent(e);
    if (m_aac)
        m_aac->inputController()->stopDwellOn(this);
}

void AACUserCard::focusInEvent(QFocusEvent* e)
{
    QWidget::focusInEvent(e);
    if (m_aac)
        m_aac->feedbackEngine()->playFocus();
}

void AACUserCard::mousePressEvent(QMouseEvent* e)
{
    if (m_aac)
        m_aac->feedbackEngine()->playClick();
    QWidget::mousePressEvent(e);
}

void AACUserCard::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);

    if (m_dwellProgress <= 0.0f)
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Horizontal bar at bottom
    int barHeight = 6;
    QRectF barRect(0, height() - barHeight, width() * m_dwellProgress, barHeight);

    p.fillRect(barRect, QColor("#00AEEF"));
}
