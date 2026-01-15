#pragma once

#include <QWidget>
#include "../backend/BackendEvents.h"

class QPushButton;
class QLabel;

class InChannelScreen : public QWidget {
    Q_OBJECT
public:
    explicit InChannelScreen(QWidget* parent = nullptr);

public slots:
    void setSelfVoiceState(SelfVoiceState state);

signals:
    void transmitToggled(bool enabled);
    void leaveRequested();

private:
    QPushButton* m_transmitButton;
    QPushButton* m_leaveButton;
    QLabel* m_voiceIndicator;
};
