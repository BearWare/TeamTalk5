#pragma once

#include <QWidget>

class QPushButton;

class InChannelScreen : public QWidget
{
    Q_OBJECT
public:
    explicit InChannelScreen(QWidget* parent = nullptr);

signals:
    void leaveRequested();
    void transmitToggled(bool on);

private:
    QPushButton* m_leaveButton = nullptr;
    QPushButton* m_transmitButton = nullptr;
    bool m_transmitting = false;
};
