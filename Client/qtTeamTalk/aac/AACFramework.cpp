#include "AACFramework.h"

#include <QTextToSpeech>
#include <QtGlobal>
#include <QPainter>
#include <QPen>
#include <QEnterEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QWidget>
#include <QLayout>
#include <QAbstractButton>
#include <QFont>
#include <QGuiApplication>
#include <QScreen>
#include <QUrl>

// -------------------------
// AACAccessibilityManager
// -------------------------

AACAccessibilityManager::AACAccessibilityManager(QObject* parent)
    : QObject(parent)
{
    m_layoutEngine = new AACLayoutEngine(this, this);
    m_inputController = new AACInputController(this, this);
    m_feedbackEngine = new AACFeedbackEngine(this, this);
m_speechEngine = new AACSpeechEngine(this, this);
m_history = new AACMessageHistory(this, this);
    m_vocabularyManager = new AACVocabularyManager(this);
    m_vocabularyManager->initialize();

connect(m_speechEngine, &AACSpeechEngine::speechStarted,
        this, &AACAccessibilityManager::speechStarted);
connect(m_speechEngine, &AACSpeechEngine::speechFinished,
        this, &AACAccessibilityManager::speechFinished);

connect(m_history, &AACMessageHistory::historyChanged,
        this, &AACAccessibilityManager::historyChanged);
}

void AACAccessibilityManager::setModes(const AACModeFlags& modes)
{
    if (m_modes.largeTargets == modes.largeTargets &&
        m_modes.dwell == modes.dwell &&
        m_modes.scanning == modes.scanning &&
        m_modes.auditoryFeedback == modes.auditoryFeedback &&
        m_modes.hapticFeedback == modes.hapticFeedback &&
        m_modes.deepWells == modes.deepWells &&
        m_modes.oneHandLayout == modes.oneHandLayout &&
        m_modes.ultraMinimal == modes.ultraMinimal &&
        m_modes.predictiveStrip == modes.predictiveStrip)
        return;

    m_modes = modes;
    emit modesChanged(m_modes);
}

void AACAccessibilityManager::setDwellConfig(const AACDwellConfig& cfg)
{
    m_dwellConfig = cfg;
    emit dwellConfigChanged(m_dwellConfig);
}

void AACAccessibilityManager::setScanningConfig(const AACScanningConfig& cfg)
{
    m_scanningConfig = cfg;
    emit scanningConfigChanged(m_scanningConfig);
}

void AACAccessibilityManager::setLayoutConfig(const AACLayoutConfig& cfg)
{
    m_layoutConfig = cfg;
    emit layoutConfigChanged(m_layoutConfig);
}

// -------------------------
// AACLayoutEngine
// -------------------------

AACLayoutEngine::AACLayoutEngine(AACAccessibilityManager* mgr, QObject* parent)
    : QObject(parent)
    , m_mgr(mgr)
{
}

void AACLayoutEngine::applyLayout(AACScreenAdapter* screen)
{
    if (!screen)
        return;

    const AACModeFlags modes = m_mgr->modes();

    applyLargeTargets(screen);

    if (modes.oneHandLayout)
        applyOneHandLayout(screen);

    if (modes.ultraMinimal)
        applyUltraMinimal(screen);

    if (modes.predictiveStrip)
        applyPredictiveStrip(screen);
}

void AACLayoutEngine::applyLargeTargets(AACScreenAdapter* screen)
{
    const bool enabled = m_mgr->modes().largeTargets;

    for (QWidget* w : screen->interactiveWidgets())
        scaleInteractiveWidget(w, enabled);

    if (QLayout* lay = screen->rootLayout())
        scaleLayout(lay, enabled);
}

void AACLayoutEngine::applyOneHandLayout(AACScreenAdapter* screen)
{
    QLayout* lay = screen->rootLayout();
    if (!lay)
        return;

    // Minimal implementation: add extra bottom margin to bias controls downward.
    QMargins m = lay->contentsMargins();
    if (m_mgr->layoutConfig().oneHandRightSide) {
        m.setBottom(m.bottom() + 40);
    } else {
        m.setBottom(m.bottom() + 40);
    }
    lay->setContentsMargins(m);
}

void AACLayoutEngine::applyUltraMinimal(AACScreenAdapter* screen)
{
    const bool enabled = m_mgr->modes().ultraMinimal;
    if (!enabled)
        return;

    const QList<QWidget*> primary = screen->primaryWidgets();
    const QSet<QWidget*> primarySet(primary.begin(), primary.end());

    for (QWidget* w : screen->interactiveWidgets()) {
        if (!primarySet.contains(w))
            w->setVisible(false);
    }
}

void AACLayoutEngine::applyPredictiveStrip(AACScreenAdapter* screen)
{
    Q_UNUSED(screen);
    // Placeholder: predictive strip widget can be added later and
    // placed into screen->predictiveStripContainer().
}

void AACLayoutEngine::scaleInteractiveWidget(QWidget* w, bool enabled)
{
    if (!w)
        return;

    const bool isButton = qobject_cast<QAbstractButton*>(w) != nullptr;

    if (!isButton)
        return;

    if (enabled) {
        w->setMinimumSize(AAC_MIN_TARGET, AAC_MIN_TARGET);
        QFont f = w->font();
        f.setPointSizeF(f.pointSizeF() * AAC_FONT_SCALE);
        w->setFont(f);
    } else {
        w->setMinimumSize(0, 0);
        QFont f = w->font();
        f.setPointSizeF(f.pointSizeF() / AAC_FONT_SCALE);
        w->setFont(f);
    }
}

void AACLayoutEngine::scaleLayout(QLayout* lay, bool enabled)
{
    if (!lay)
        return;

    if (enabled) {
        lay->setSpacing(AAC_MIN_SPACING);
        lay->setContentsMargins(
            AAC_MIN_SPACING,
            AAC_MIN_SPACING,
            AAC_MIN_SPACING,
            AAC_MIN_SPACING
        );
    } else {
        lay->setSpacing(8);
        lay->setContentsMargins(8, 8, 8, 8);
    }
}

// -------------------------
// AACInputController
// -------------------------

AACInputController::AACInputController(AACAccessibilityManager* mgr, QObject* parent)
    : QObject(parent)
    , m_mgr(mgr)
{
    connect(&m_dwellTimer, &QTimer::timeout,
            this, &AACInputController::onDwellTick);

    connect(&m_scanningTimer, &QTimer::timeout,
            this, &AACInputController::onScanningTick);
}

void AACInputController::attachScreen(AACScreenAdapter* screen)
{
    m_currentScreen = screen;
    rebuildScanningList();
}

void AACInputController::detachScreen(AACScreenAdapter* screen)
{
    if (m_currentScreen == screen)
        m_currentScreen = nullptr;
    m_scanningWidgets.clear();
    m_scanningIndex = -1;
    stopDwell();
}

void AACInputController::rebuildScanningList()
{
    m_scanningWidgets.clear();
    m_scanningIndex = -1;

    if (!m_currentScreen)
        return;

    for (QWidget* w : m_currentScreen->interactiveWidgets())
        m_scanningWidgets << w;
}

void AACInputController::startDwell(QWidget* w)
{
    if (!w || !m_mgr->modes().dwell)
        return;

    m_dwellTarget = w;
    m_dwellElapsed.restart();
    m_dwellTimer.start(30);
}

void AACInputController::stopDwell()
{
    m_dwellTimer.stop();
    m_dwellTarget.clear();
}

void AACInputController::onDwellTick()
{
    if (!m_dwellTarget)
        return;

    const int elapsed = static_cast<int>(m_dwellElapsed.elapsed());
    const int target = m_mgr->dwellConfig().dwellDurationMs;

    // Dwell progress: 0.0–1.0
    float progress = 0.0f;
    if (target > 0)
        progress = qBound(0.0f, float(elapsed) / float(target), 1.0f);

    emit dwellProgressChanged(m_dwellTarget, progress);

    if (elapsed >= target) {
        QWidget* w = m_dwellTarget;
        stopDwell();
        emit dwellActivated(w);
        activateWidget(w);
    }
}

void AACInputController::onScanningTick()
{
    if (!m_mgr->modes().scanning)
        return;

    if (m_scanningWidgets.isEmpty())
        return;

    m_scanningIndex = (m_scanningIndex + 1) % m_scanningWidgets.size();
    QWidget* w = m_scanningWidgets[m_scanningIndex];
    focusWidget(w);
    emit scanningFocused(w);
}

void AACInputController::focusWidget(QWidget* w)
{
    if (!w)
        return;
    w->setFocus(Qt::TabFocusReason);
}

void AACInputController::onSwitchActivate()
{
    if (!m_mgr->modes().scanning)
        return;

    if (m_scanningIndex < 0 || m_scanningIndex >= m_scanningWidgets.size())
        return;

    QWidget* w = m_scanningWidgets[m_scanningIndex];
    if (!w)
        return;

    activateWidget(w);
}

void AACInputController::onSwitchNext()
{
    if (!m_mgr->modes().scanning)
        return;

    if (!m_scanningTimer.isActive()) {
        m_scanningTimer.start(m_mgr->scanningConfig().stepIntervalMs);
    } else {
        onScanningTick();
    }
}

bool AACInputController::isDeepWell(QWidget* w) const
{
    if (!w)
        return false;
    // Convention: widgets with property "aacDeepWell" = true require dwell/hold semantics.
    return w->property("aacDeepWell").toBool();
}

void AACInputController::activateWidget(QWidget* w)
{
    if (!w)
        return;

    if (auto* btn = qobject_cast<QAbstractButton*>(w)) {
        if (isDeepWell(w)) {
            // For deep wells, we already required dwell or scanning focus before activation.
            btn->click();
            emit deepWellActivated(w);
        } else {
            btn->click();
        }
    }
}

void AACInputController::startDwellOn(QWidget* w)
{
    startDwell(w);
}

void AACInputController::stopDwellOn(QWidget* w)
{
    Q_UNUSED(w);
    stopDwell();
}

// -------------------------
// AACFeedbackEngine
// -------------------------

AACFeedbackEngine::AACFeedbackEngine(AACAccessibilityManager* mgr, QObject* parent)
    : QObject(parent)
    , m_mgr(mgr)
{
    initSounds();
}

void AACFeedbackEngine::initSounds()
{
    m_clickSound.setSource(QUrl("qrc:/sounds/aac_click.wav"));
    m_focusSound.setSource(QUrl("qrc:/sounds/aac_focus.wav"));
    m_errorSound.setSource(QUrl("qrc:/sounds/aac_error.wav"));
    m_dwellSound.setSource(QUrl("qrc:/sounds/aac_dwell.wav"));
}

void AACFeedbackEngine::playClick()
{
    if (!m_mgr->modes().auditoryFeedback)
        return;
    m_clickSound.play();
}

void AACFeedbackEngine::playFocus()
{
    if (!m_mgr->modes().auditoryFeedback)
        return;
    m_focusSound.play();
}

void AACFeedbackEngine::playError()
{
    if (!m_mgr->modes().auditoryFeedback)
        return;
    m_errorSound.play();
}

void AACFeedbackEngine::playDwellComplete()
{
    if (!m_mgr->modes().auditoryFeedback)
        return;
    m_dwellSound.play();
}

void AACFeedbackEngine::doHaptic(int strength)
{
    Q_UNUSED(strength);
    if (!m_mgr->modes().hapticFeedback)
        return;
    // Hook up platform-specific haptics here (QFeedbackHapticsEffect, etc.)
}

void AACFeedbackEngine::hapticSoft()
{
    doHaptic(1);
}

void AACFeedbackEngine::hapticStrong()
{
    doHaptic(2);
}

void AACFeedbackEngine::hapticError()
{
    doHaptic(3);
}
// -------------------------
// AACButton
// -------------------------

AACButton::AACButton(AACAccessibilityManager* aac, QWidget* parent)
    : QPushButton(parent)
    , m_aac(aac)
{
    setFocusPolicy(Qt::StrongFocus);

    if (m_aac) {
        connect(m_aac->inputController(), &AACInputController::dwellProgressChanged,
                this, [this](QWidget* target, float p) {
                    if (target == this)
                        setDwellProgress(p);
                });
    }
}

void AACButton::setDeepWell(bool enabled)
{
    m_deepWell = enabled;
    setProperty("aacDeepWell", enabled);
}

bool AACButton::isDeepWell() const { return m_deepWell; }

void AACButton::setDwellProgress(float p)
{
    m_dwellProgress = p;
    update();
}

void AACButton::enterEvent(QEnterEvent* e)
{
    QPushButton::enterEvent(e);
    if (m_aac)
        m_aac->inputController()->startDwellOn(this);
}

void AACButton::leaveEvent(QEvent* e)
{
    QPushButton::leaveEvent(e);
    if (m_aac)
        m_aac->inputController()->stopDwellOn(this);
}

void AACButton::focusInEvent(QFocusEvent* e)
{
    QPushButton::focusInEvent(e);
    if (m_aac)
        m_aac->feedbackEngine()->playFocus();
}

void AACButton::mousePressEvent(QMouseEvent* e)
{
    if (m_aac)
        m_aac->feedbackEngine()->playClick();
    QPushButton::mousePressEvent(e);
}

void AACButton::paintEvent(QPaintEvent* e)
{
    QPushButton::paintEvent(e);

    if (m_dwellProgress <= 0.0f)
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QRectF r = rect().adjusted(4, 4, -4, -4);
    QPen pen(QColor("#00AEEF"));
    pen.setWidth(4);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    int startAngle = 90 * 16;
    int spanAngle = -int(360 * 16 * m_dwellProgress);
    p.drawArc(r, startAngle, spanAngle);
}
// -------------------------
// AACSpeechEngine
// -------------------------

AACSpeechEngine::AACSpeechEngine(AACAccessibilityManager* mgr, QObject* parent)
    : QObject(parent)
    , m_mgr(mgr)
{
    m_tts = new QTextToSpeech(this);
}

void AACSpeechEngine::speak(const QString& text)
{
    if (!m_tts || text.isEmpty())
        return;

    emit speechStarted(text);
    m_tts->say(text);

    // We don't get a per-utterance finished signal easily here,
    // so we emit finished immediately after scheduling.
    emit speechFinished(text);

    if (m_mgr && !text.trimmed().isEmpty()) {
        m_mgr->history()->addMessage(text);
    }
}

void AACSpeechEngine::stop()
{
    if (!m_tts)
        return;
    m_tts->stop();
}

void AACSpeechEngine::setVoice(const QString& voiceName)
{
    if (!m_tts)
        return;

    const auto voices = m_tts->availableVoices();
    for (const QVoice& v : voices) {
        if (v.name() == voiceName) {
            m_tts->setVoice(v);
            break;
        }
    }
}

void AACSpeechEngine::setRate(double rate)
{
    if (!m_tts)
        return;
    m_tts->setRate(rate);
}

void AACSpeechEngine::setPitch(double pitch)
{
    if (!m_tts)
        return;
    m_tts->setPitch(pitch);
}

void AACSpeechEngine::setSpeakAsYouType(bool enabled)
{
    m_speakAsYouType = enabled;
}

void AACSpeechEngine::speakLetter(const QString& letter)
{
    if (!m_tts || !m_speakAsYouType)
        return;

    if (letter.isEmpty())
        return;

    m_tts->say(letter);
}
// -------------------------
// AACMessageHistory
// -------------------------

AACMessageHistory::AACMessageHistory(AACAccessibilityManager* mgr, QObject* parent)
    : QObject(parent)
    , m_mgr(mgr)
{
}

void AACMessageHistory::addMessage(const QString& msg)
{
    if (msg.trimmed().isEmpty())
        return;

    m_history << msg;
    emit historyChanged(m_history);
}

QStringList AACMessageHistory::history() const
{
    return m_history;
}

void AACMessageHistory::replayMessage(int index)
{
    if (!m_mgr || index < 0 || index >= m_history.size())
        return;

    const QString msg = m_history.at(index);
    if (AACSpeechEngine* s = m_mgr->speechEngine())
        s->speak(msg);
}

void AACMessageHistory::replayLast()
{
    if (!m_mgr || m_history.isEmpty())
        return;

    const QString msg = m_history.last();
    if (AACSpeechEngine* s = m_mgr->speechEngine())
        s->speak(msg);
}
