#include "PredictiveStrip.h"
#include <QHBoxLayout>
#include <QPushButton>

PredictiveStrip::PredictiveStrip(QWidget* parent)
    : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(8);
}

void PredictiveStrip::setActions(const QVector<PredictiveAction>& actions)
{
    QLayoutItem* child;
    while ((child = m_layout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    for (const auto& a : actions) {
        auto* btn = new QPushButton(a.label, this);
        m_layout->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, [this, a]() {
            emit actionTriggered(a.id);
        });
    }
}
