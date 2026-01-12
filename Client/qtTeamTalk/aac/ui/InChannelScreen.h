#pragma once
#include <QWidget>

class InChannelScreen : public QWidget {
    Q_OBJECT
public:
    explicit InChannelScreen(QWidget* parent = nullptr);

    void setTransmitState(bool active);
    void showMessageBubble(int fromUserId, const QString& message);

private:
    QPushButton* m_transmitBtn = nullptr;
};
