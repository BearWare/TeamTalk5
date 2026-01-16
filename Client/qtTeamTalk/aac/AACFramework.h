#pragma once

#include <QObject>
#include <QPointer>
#include <QList>
#include <QMap>
#include <QElapsedTimer>
#include <QRect>
#include <QSoundEffect>
#include <QTimer>

class QWidget;
class QLayout;
class QAbstractButton;

// High-level AAC mode flags
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

// Per-mode configuration
struct AACDwellConfig {
    int dwellDurationMs = 800;
};

struct AACScanningConfig {
    int stepIntervalMs = 1000;
};

struct AACLayoutConfig {
    bool oneHandRightSide = true;
};

// Forward declarations
class AACLayoutEngine;
class AACInputController;
class AACFeedbackEngine;

// Central manager: owns modes + engines
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

signals:
    void modesChanged(const AACModeFlags& modes);
    void dwellConfigChanged(const AACDwellConfig& cfg);
    void scanningConfigChanged(const AACScanningConfig& cfg);
    void layoutConfigChanged(const AACLayoutConfig& cfg);

private:
    AACModeFlags m_modes;
    AACDwellConfig m_dwellConfig;
    AACScanningConfig m_scanningConfig;
    AACLayoutConfig m_layoutConfig;

    AACLayoutEngine* m_layoutEngine = nullptr;
    AACInputController* m_inputController = nullptr;
    AACFeedbackEngine* m_feedbackEngine = nullptr;
};

// A screen exposes its interactive widgets + layout to the framework
class AACScreenAdapter {
public:
    virtual ~AACScreenAdapter() = default;

    // All interactive widgets on this screen
    virtual QList<QWidget*> interactiveWidgets() const = 0;

    // Widgets that should be considered "primary" for ultra-minimal mode
    virtual QList<QWidget*> primaryWidgets() const = 0;

    // Root layout for layout engine
    virtual QLayout* rootLayout() const = 0;

    // Optional predictive strip container (may be nullptr)
    virtual QWidget* predictiveStripContainer() const { return nullptr; }
};

// Layout engine: large targets, one-hand, ultra-minimal, predictive strip
class AACLayoutEngine : public QObject {
    Q_OBJECT
public:
    explicit AACLayoutEngine(AACAccessibilityManager* mgr, QObject* parent = nullptr);

    void applyLayout(AACScreenAdapter* screen);

    // Constants reused from your previous AACScreen
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

// Input controller: dwell, scanning, deep wells
class AACInputController : public QObject {
    Q_OBJECT
public:
    explicit AACInputController(AACAccessibilityManager* mgr, QObject* parent = nullptr);

    // Called by screens when they are shown
    void attachScreen(AACScreenAdapter* screen);
    void detachScreen(AACScreenAdapter* screen);

    // Switch input (e.g. from keyboard, external device)
    void onSwitchActivate();
    void onSwitchNext();

    // Pointer-based dwell hooks (screens can call these on hover enter/leave)
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

    // Dwell
    QTimer m_dwellTimer;
    QPointer<QWidget> m_dwellTarget;
    QElapsedTimer m_dwellElapsed;

    // Scanning
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

// Feedback engine: auditory + haptic
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
