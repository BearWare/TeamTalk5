#pragma once

#include "AACPredictionEngine.h"
#include <QPushButton>
#include <QtGlobal>
#include <QObject>
#include <QPointer>
#include <QList>
#include <QMap>
#include <QElapsedTimer>
#include <QRect>
#include <QSoundEffect>
#include <QTimer>
#include <QStringList>

class QTextToSpeech;
class QWidget;
class QLayout;
class QAbstractButton;

// Forward declarations
class AACAccessibilityManager;
class AACVocabularyManager;
class AACLayoutEngine;
class AACInputController;
class AACFeedbackEngine;
class AACSpeechEngine;
class AACMessageHistory;
class AACPredictionEngine;

// -------------------------
// AACButton
// -------------------------
class AACButton : public QPushButton {
    Q_OBJECT
public:
    explicit AACButton(AACAccessibilityManager* aac, QWidget* parent = nullptr);

    void setDeepWell(bool enabled);
    bool isDeepWell() const;

public slots:
    void setDwellProgress(float p); // 0.0–1.0

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void focusInEvent(QFocusEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    AACAccessibilityManager* m_aac = nullptr;
    bool m_deepWell = false;
    float m_dwellProgress = 0.0f;
};

// -------------------------
// High-level AAC mode flags
// -------------------------
struct AACModeFlags {
    bool largeTargets = false;
    bool dwell = false;
    bool scanning = false;
    bool auditoryFeedback = false;
    bool hapticFeedback = false;
    bool deepWells = false;
    bool oneHandLayout = false;
    bool ultraMinimal = false;
    bool predictiveStrip = false;
};

struct AACDwellConfig {
    int dwellDurationMs = 800;
};

struct AACScanningConfig {
    int stepIntervalMs = 1000;
};

struct AACLayoutConfig {
    bool oneHandRightSide = true;
};

// -------------------------
// AACAccessibilityManager
// -------------------------
class AACAccessibilityManager : public QObject {
    Q_OBJECT
public:
    explicit AACAccessibilityManager(QObject* parent = nullptr);

    const AACModeFlags& modes() const { return m_modes; }
    void setModes(const AACModeFlags& modes);

    const AACDwellConfig& dwellConfig() const { return m_dwellConfig; }
    void setDwellConfig(const AACDwellConfig& cfg);

    const AACScanningConfig& scanningConfig() const { return m_scanningConfig; }
    void setScanningConfig(const AACScanningConfig& cfg);

    const AACLayoutConfig& layoutConfig() const { return m_layoutConfig; }
    void setLayoutConfig(const AACLayoutConfig& cfg);

    AACLayoutEngine* layoutEngine() const { return m_layoutEngine; }
    AACInputController* inputController() const { return m_inputController; }
    AACFeedbackEngine* feedbackEngine() const { return m_feedbackEngine; }

    AACSpeechEngine* speechEngine() const { return m_speechEngine; }
    AACMessageHistory* history() const { return m_history; }

    AACVocabularyManager* vocabularyManager() const { return m_vocabularyManager; }

AACPredictionEngine* predictionEngine() const { return m_predictionEngine; }

    bool predictionEnabled() const { return m_predictionEnabled; }
    void setPredictionEnabled(bool enabled);

    void boostPredictionVocabulary();
    void loadPredictionForUser(const QString& userId);
    void savePredictionForUser(const QString& userId);

signals:
    void modesChanged(const AACModeFlags& modes);
    void dwellConfigChanged(const AACDwellConfig& cfg);
    void dwellProgressChanged(QWidget* target, float progress);
    void scanningConfigChanged(const AACScanningConfig& cfg);
    void layoutConfigChanged(const AACLayoutConfig& cfg);

    void speechStarted(const QString& text);
    void speechFinished(const QString& text);
    void historyChanged(const QStringList& history);
    void predictionEnabledChanged(bool enabled);

private:
    AACModeFlags m_modes;
    AACDwellConfig m_dwellConfig;
    AACScanningConfig m_scanningConfig;
    AACLayoutConfig m_layoutConfig;

    AACLayoutEngine* m_layoutEngine = nullptr;
    AACInputController* m_inputController = nullptr;
    AACFeedbackEngine* m_feedbackEngine = nullptr;

    AACSpeechEngine* m_speechEngine = nullptr;
    AACMessageHistory* m_history = nullptr;
    AACVocabularyManager* m_vocabularyManager = nullptr;

    AACPredictionEngine* m_predictionEngine = nullptr;
    bool m_predictionEnabled = true;
};

// -------------------------
// AACScreenAdapter
// -------------------------
class AACScreenAdapter {
public:
    virtual ~AACScreenAdapter() = default;

    virtual QList<QWidget*> interactiveWidgets() const = 0;
    virtual QList<QWidget*> primaryWidgets() const = 0;
    virtual QLayout* rootLayout() const = 0;
    virtual QWidget* predictiveStripContainer() const { return nullptr; }
};

// -------------------------
// AACLayoutEngine
// -------------------------
class AACLayoutEngine : public QObject {
    Q_OBJECT
public:
    explicit AACLayoutEngine(AACAccessibilityManager* mgr, QObject* parent = nullptr);

    void applyLayout(AACScreenAdapter* screen);

    static constexpr int AAC_MIN_TARGET = 140;
    static constexpr int AAC_MIN_SPACING = 24;
    static constexpr qreal AAC_FONT_SCALE = 1.3;

private:
    AACAccessibilityManager* m_mgr = nullptr;

    void applyLargeTargets(AACScreenAdapter* screen);
    void applyOneHandLayout(AACScreenAdapter* screen);
    void applyUltraMinimal(AACScreenAdapter* screen);
    void applyPredictiveStrip(AACScreenAdapter* screen);

    void scaleInteractiveWidget(QWidget* w, bool enabled);
    void scaleLayout(QLayout* layout, bool enabled);
};

// -------------------------
// AACInputController
// -------------------------
class AACInputController : public QObject {
    Q_OBJECT
public:
    explicit AACInputController(AACAccessibilityManager* mgr, QObject* parent = nullptr);

    void attachScreen(AACScreenAdapter* screen);
    void detachScreen(AACScreenAdapter* screen);

    void onSwitchActivate();
    void onSwitchNext();

    void startDwellOn(QWidget* w);
    void stopDwellOn(QWidget* w);

signals:
    void dwellActivated(QWidget* w);
    void scanningFocused(QWidget* w);
    void deepWellActivated(QWidget* w);

private slots:
    void onDwellTick();
    void onScanningTick();

private:
    AACAccessibilityManager* m_mgr = nullptr;
    AACScreenAdapter* m_currentScreen = nullptr;

    QTimer m_dwellTimer;
    QPointer<QWidget> m_dwellTarget;
    QElapsedTimer m_dwellElapsed;

    QTimer m_scanningTimer;
    QList<QPointer<QWidget>> m_scanningWidgets;
    int m_scanningIndex = -1;

    void rebuildScanningList();
    void startDwell(QWidget* w);
    void stopDwell();
    void focusWidget(QWidget* w);

    bool isDeepWell(QWidget* w) const;
    void activateWidget(QWidget* w);
};

// -------------------------
// AACFeedbackEngine
// -------------------------
class AACFeedbackEngine : public QObject {
    Q_OBJECT
public:
    explicit AACFeedbackEngine(AACAccessibilityManager* mgr, QObject* parent = nullptr);

    void playClick();
    void playFocus();
    void playError();
    void playDwellComplete();

    void hapticSoft();
    void hapticStrong();
    void hapticError();

private:
    AACAccessibilityManager* m_mgr = nullptr;

    QSoundEffect m_clickSound;
    QSoundEffect m_focusSound;
    QSoundEffect m_errorSound;
    QSoundEffect m_dwellSound;

    void initSounds();
    void doHaptic(int strength);
};

// -------------------------
// AACSpeechEngine
// -------------------------
class AACSpeechEngine : public QObject {
    Q_OBJECT
public:
    explicit AACSpeechEngine(AACAccessibilityManager* mgr, QObject* parent = nullptr);

    void speak(const QString& text);
    void stop();

    void setVoice(const QString& voiceName);
    void setRate(double rate);
    void setPitch(double pitch);

    void setSpeakAsYouType(bool enabled);
    bool speakAsYouType() const { return m_speakAsYouType; }

    void speakLetter(const QString& letter);

signals:
    void speechStarted(const QString& text);
    void speechFinished(const QString& text);

private:
    AACAccessibilityManager* m_mgr = nullptr;
    QTextToSpeech* m_tts = nullptr;
    bool m_speakAsYouType = false;
};

// -------------------------
// AACMessageHistory
// -------------------------
class AACMessageHistory : public QObject {
    Q_OBJECT
public:
    explicit AACMessageHistory(AACAccessibilityManager* mgr, QObject* parent = nullptr);

    void addMessage(const QString& msg);
    QStringList history() const;

    void replayMessage(int index);
    void replayLast();

signals:
    void historyChanged(const QStringList& history);

private:
    AACAccessibilityManager* m_mgr = nullptr;
    QStringList m_history;
};
