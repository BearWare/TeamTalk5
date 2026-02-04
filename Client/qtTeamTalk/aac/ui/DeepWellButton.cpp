#include "DeepWellButton.h"
#include <QMouseEvent>

DeepWellButton::DeepWellButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    setProperty("aacDeepWell", true);
    connect(&m_timer, &QTimer::timeout, this, &DeepWellButton::onTick);
    m_timer.setInterval(30);
}

void DeepWellButton::setHoldDurationMs(int ms)
{
    m_holdMs = ms;
}

void DeepWellButton::mousePressEvent(QMouseEvent* e)
{
    QPushButton::mousePressEvent(e);
    m_inside = true;
    m_elapsed.restart();
    m_timer.start();
}

void DeepWellButton::mouseReleaseEvent(QMouseEvent* e)
{
    QPushButton::mouseReleaseEvent(e);
    m_timer.stop();
    m_inside = false;
}

void DeepWellButton::leaveEvent(QEvent* e)
{
    QPushButton::leaveEvent(e);
    m_inside = false;
}

void DeepWellButton::onTick()
{
    if (!m_inside)
        return;

    if (m_elapsed.elapsed() >= m_holdMs) {
        m_timer.stop();
        click();
    }
}
