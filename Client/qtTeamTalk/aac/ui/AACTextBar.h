#pragma once

#include <QWidget>
#include <QPointer>

class QLineEdit;
class QPushButton;
class QSlider;
class QComboBox;
class QToolButton;
class QMenu;
class QAction;
class AACAccessibilityManager;
class AACSpeechEngine;
class AACMessageHistory;

class AACTextBar : public QWidget {
    Q_OBJECT
public:
    explicit AACTextBar(AACAccessibilityManager* mgr, QWidget* parent = nullptr);

    QString text() const;
    void setText(const QString& t);

signals:
    void speakRequested(const QString& text);
    void stopRequested();
    void rateChanged(double rate);
    void pitchChanged(double pitch);
    void voiceChanged(const QString& voiceName);

private slots:
    void onSpeak();
    void onStop();
    void onRateChanged(int value);
    void onPitchChanged(int value);
    void onVoiceSelected(int index);
    void onSpeakAsYouTypeChanged(const QString& text);
    void onHistoryTriggered();

private:
    void buildUi();
    void connectSignals();
    void populateVoices();
    void rebuildHistoryMenu();

    AACAccessibilityManager* m_mgr = nullptr;
    AACSpeechEngine* m_speech = nullptr;
    AACMessageHistory* m_history = nullptr;

    QLineEdit* m_edit = nullptr;
    QPushButton* m_speakBtn = nullptr;
    QPushButton* m_stopBtn = nullptr;

    QSlider* m_rateSlider = nullptr;
    QSlider* m_pitchSlider = nullptr;

    QComboBox* m_voiceCombo = nullptr;
    QToolButton* m_historyBtn = nullptr;
    QMenu* m_historyMenu = nullptr;

    QToolButton* m_sayAsTypeToggle = nullptr;
};
