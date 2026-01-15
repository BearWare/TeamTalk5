#include "MessageBubble.h"
#include <QLabel>
#include <QVBoxLayout>

MessageBubble::MessageBubble(const QString& text, QWidget* parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    auto label = new QLabel(text, this);
    label->setWordWrap(true);
    layout->addWidget(label);
}
