#pragma once
#include <QWidget>

class MessageBubble : public QWidget {
    Q_OBJECT
public:
    explicit MessageBubble(const QString& text, QWidget* parent = nullptr);
};
