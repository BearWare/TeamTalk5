#pragma once

#include <QWidget>
#include <QList>
#include <QMap>
#include <QDateTime>

#include "aac/backend/BackendEvents.h"

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;

class InChannelScreen : public QWidget {
    Q_OBJECT
public:
    explicit InChannelScreen(QWidget* parent = nullptr);

signals:
    void leaveChannelRequested();
    void transmitToggled(bool enabled);

public slots:
    void updateSelfVoiceState(SelfVoiceState state);
    void updateOtherUserVoiceState(const OtherUserVoiceEvent& event);

private slots:
    void onLeaveClicked();
    void onTransmitClicked();

private:
    enum class TransmitUiState {
        Idle,       // not armed
        Armed,      // armed but not speaking
        Speaking    // speaking
    };

    struct Participant {
        int userId;
        QString username;
        OtherUserVoiceState voiceState;
        QDateTime lastSpoke;
        QListWidgetItem* item = nullptr;
    };

    void updateTransmitUi();
    void updateParticipantItem(Participant& p);
    void resortParticipants();

    // UI
    QLabel* m_channelLabel = nullptr;
    QListWidget* m_participantList = nullptr;
    QPushButton* m_transmitButton = nullptr;
    QPushButton* m_leaveButton = nullptr;

    // State
    bool m_transmitEnabled = false;
    SelfVoiceState m_selfVoiceState = SelfVoiceState::Silent;
    TransmitUiState m_transmitUiState = TransmitUiState::Idle;

    QMap<int, Participant> m_participants; // key: userId
};
