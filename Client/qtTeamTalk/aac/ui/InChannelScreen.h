#pragma once

#include "AACScreen.h"

#include <QMap>
#include <QDateTime>
#include <QTimer>
#include "aac/backend/BackendEvents.h"

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;

class InChannelScreen : public AACScreen {
    Q_OBJECT
public:
    explicit InChannelScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

    void setChannelName(const QString& name);

signals:
    void leaveChannelRequested();
    void transmitToggled(bool enabled);
    void settingsRequested();

public slots:
    void updateSelfVoiceState(SelfVoiceState state);
    void updateOtherUserVoiceState(const OtherUserVoiceEvent& event);
    void clearParticipants();
    void setEventMessage(const QString& message);

protected:
    void resizeEvent(QResizeEvent* e) override;

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

    // -------------------------
    // Prediction UI
    // -------------------------
    QLineEdit* m_inputEdit = nullptr;   // text entry box
    QLabel* m_ghostLabel = nullptr;     // faint prediction text
    QString m_ghostText;                // current suggestion

    void onInputTextChanged(const QString& text);
    void onAcceptGhost();
    void renderGhostText(const QString& committed, const QString& ghost);

    // -------------------------
    // Participant + UI state
    // -------------------------
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
    void updateRowHeight();

    QLabel* m_channelLabel = nullptr;
    QLabel* m_speakingBanner = nullptr;
    QLabel* m_quietBanner = nullptr;
    QLabel* m_participantCountLabel = nullptr;
    QLabel* m_eventBanner = nullptr;
    QListWidget* m_participantList = nullptr;
    QPushButton* m_transmitButton = nullptr;
    QPushButton* m_leaveButton = nullptr;

    QPushButton* m_floatingSettingsButton = nullptr;

    bool m_transmitEnabled = false;
    SelfVoiceState m_selfVoiceState = SelfVoiceState::Silent;
    TransmitUiState m_transmitUiState = TransmitUiState::Idle;

    QMap<int, Participant> m_participants;
    QTimer m_quietTimer;
};
