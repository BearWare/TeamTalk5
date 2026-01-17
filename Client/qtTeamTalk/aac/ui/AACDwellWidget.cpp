#include "AACFramework.h"
#include <QEnterEvent>
#include <QFocusEvent>
#include <QMouseEvent>

AACDwellWidget::AACDwellWidget(AACAccessibilityManager* aac, QWidget* parent)
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

void AACDwellWidget::setDeepWell(bool enabled)
{
    m_deepWell = enabled;
    setProperty("aacDeepWell", enabled);
}

bool AACDwellWidget::isDeepWell() const
{
    return m_deepWell;
}

void AACDwellWidget::setDwellProgress(float p)
{
    m_dwellProgress = p;
    update();
}

void AACDwellWidget::enterEvent(QEnterEvent* e)
{
    QWidget::enterEvent(e);
    if (m_aac)
        m_aac->inputController()->startDwellOn(this);
}

void AACDwellWidget::leaveEvent(QEvent* e)
{
    QWidget::leaveEvent(e);
    if (m_aac)
        m_aac->inputController()->stopDwellOn(this);
}

void AACDwellWidget::focusInEvent(QFocusEvent* e)
{
    QWidget::focusInEvent(e);
    if (m_aac)
        m_aac->feedbackEngine()->playFocus();
}

void AACDwellWidget::mousePressEvent(QMouseEvent* e)
{
    if (m_aac)
        m_aac->feedbackEngine()->playClick();
    QWidget::mousePressEvent(e);
}
