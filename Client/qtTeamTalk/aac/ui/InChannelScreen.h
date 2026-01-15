#pragma once

#include <QWidget>
#include <QList>
#include <QMap>
#include <QDateTime>
#include <QTimer>

#include "aac/backend/BackendEvents.h"

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;

class InChannelScreen : public QWidget {
    Q_OBJECT
public:
    explicit InChannelScreen(QWidget* parent = nullptr);

    void setChannelName(const QString& name);

signals:
    void leaveChannelRequested();
    void transmitToggled(bool enabled);

public slots:
    void updateSelfVoiceState(SelfVoiceState state);
    void updateOtherUserVoiceState(const OtherUserVoiceEvent& event);
    void clearParticipants();
    void setEventMessage(const QString& message);

private slots:
    void onLeaveClicked();
    void onTransmitClicked();
    void onQuietTimerTick();

private:
    enum class TransmitUiState {
        Idle,
        Armed,
        Speaking
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
    void updateSpeakingBanner();
    void updateParticipantCount();

    // UI
    QLabel* m_channelLabel = nullptr;
    QLabel* m_speakingBanner = nullptr;
    QLabel* m_quietBanner = nullptr;
    QLabel* m_participantCountLabel = nullptr;
    QLabel* m_eventBanner = nullptr;
    QListWidget* m_participantList = nullptr;
    QPushButton* m_transmitButton = nullptr;
    QPushButton* m_leaveButton = nullptr;

    // State
    bool m_transmitEnabled = false;
    SelfVoiceState m_selfVoiceState = SelfVoiceState::Silent;
    TransmitUiState m_transmitUiState = TransmitUiState::Idle;

    QMap<int, Participant> m_participants;
    QTimer m_quietTimer;
};
