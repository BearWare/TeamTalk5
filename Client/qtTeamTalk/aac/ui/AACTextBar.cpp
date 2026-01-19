#include "AACTextBar.h"
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QHBoxLayout>

#include "AACFramework.h" // for AACAccessibilityManager, AACSpeechEngine, AACMessageHistory

AACTextBar::AACTextBar(AACAccessibilityManager* mgr, QWidget* parent)
    : QWidget(parent),
      m_mgr(mgr),
      m_speech(mgr ? mgr->speechEngine() : nullptr),
      m_history(mgr ? mgr->history() : nullptr)
{
    buildUi();
    populateVoices();
    connectSignals();
}

QString AACTextBar::text() const {
    return m_edit->text();
}

void AACTextBar::setText(const QString& t) {
    m_edit->setText(t);
}

void AACTextBar::buildUi() {
    m_edit = new QLineEdit(this);

    m_speakBtn = new QPushButton(tr("Speak"), this);
    m_stopBtn = new QPushButton(tr("Stop"), this);

    m_rateSlider = new QSlider(Qt::Horizontal, this);
    m_rateSlider->setRange(-100, 100);
    m_rateSlider->setValue(0);

    m_pitchSlider = new QSlider(Qt::Horizontal, this);
    m_pitchSlider->setRange(-100, 100);
    m_pitchSlider->setValue(0);

    m_voiceCombo = new QComboBox(this);

    m_sayAsTypeToggle = new QToolButton(this);
    m_sayAsTypeToggle->setText("🔊");
    m_sayAsTypeToggle->setCheckable(true);
    m_sayAsTypeToggle->setChecked(m_speech && m_speech->speakAsYouType());

    m_historyBtn = new QToolButton(this);
    m_historyBtn->setText("🕘");
    m_historyMenu = new QMenu(this);
    m_historyBtn->setMenu(m_historyMenu);
    m_historyBtn->setPopupMode(QToolButton::InstantPopup);

    auto* layout = new QHBoxLayout(this);
    layout->addWidget(m_edit);
    layout->addWidget(m_speakBtn);
    layout->addWidget(m_stopBtn);
    layout->addWidget(m_rateSlider);
    layout->addWidget(m_pitchSlider);
    layout->addWidget(m_voiceCombo);
    layout->addWidget(m_sayAsTypeToggle);
    layout->addWidget(m_historyBtn);

    setLayout(layout);
}

void AACTextBar::connectSignals() {
    connect(m_speakBtn, &QPushButton::clicked, this, &AACTextBar::onSpeak);
    connect(m_stopBtn, &QPushButton::clicked, this, &AACTextBar::onStop);

    connect(m_rateSlider, &QSlider::valueChanged, this, &AACTextBar::onRateChanged);
    connect(m_pitchSlider, &QSlider::valueChanged, this, &AACTextBar::onPitchChanged);

    connect(m_voiceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AACTextBar::onVoiceSelected);

    connect(m_edit, &QLineEdit::textEdited,
            this, &AACTextBar::onSpeakAsYouTypeChanged);

    connect(m_historyBtn, &QToolButton::clicked,
            this, &AACTextBar::onHistoryTriggered);

    if (m_history) {
        connect(m_history, &AACMessageHistory::historyChanged,
                this, &AACTextBar::rebuildHistoryMenu);
    }
}

void AACTextBar::populateVoices() {
    if (!m_speech)
        return;

    const auto voices = m_speech->availableVoices();
    for (const auto& v : voices)
        m_voiceCombo->addItem(v.name());

    if (!voices.isEmpty())
        m_voiceCombo->setCurrentIndex(0);
}

void AACTextBar::onSpeak() {
    const QString t = m_edit->text();
    emit speakRequested(t);
    if (m_speech)
        m_speech->speak(t);
}

void AACTextBar::onStop() {
    emit stopRequested();
    if (m_speech)
        m_speech->stop();
}

void AACTextBar::onRateChanged(int value) {
    const double rate = value / 100.0;
    emit rateChanged(rate);
    if (m_speech)
        m_speech->setRate(rate);
}

void AACTextBar::onPitchChanged(int value) {
    const double pitch = value / 100.0;
    emit pitchChanged(pitch);
    if (m_speech)
        m_speech->setPitch(pitch);
}

void AACTextBar::onVoiceSelected(int index) {
    if (!m_speech)
        return;

    const QString voiceName = m_voiceCombo->itemText(index);
    emit voiceChanged(voiceName);
    m_speech->setVoice(voiceName);
}

void AACTextBar::onSpeakAsYouTypeChanged(const QString& text) {
    if (!m_speech)
        return;

    if (m_sayAsTypeToggle->isChecked())
        m_speech->speakLetter(text.right(1));
}

void AACTextBar::rebuildHistoryMenu() {
    if (!m_history)
        return;

    m_historyMenu->clear();

    const QStringList list = m_history->history();
    for (int i = 0; i < list.size(); ++i) {
        QAction* act = m_historyMenu->addAction(list[i]);
        connect(act, &QAction::triggered, this, [this, i]() {
            if (m_history)
                m_history->replayMessage(i);
        });
    }
}

void AACTextBar::onHistoryTriggered() {
    rebuildHistoryMenu();
}
void AACTextBar::insertCharacter(QChar ch)
{
    int pos = m_edit->cursorPosition();
    QString t = m_edit->text();
    t.insert(pos, ch);
    m_edit->setText(t);
    m_edit->setCursorPosition(pos + 1);
    emit textChanged(t);
}

void AACTextBar::insertSpace()
{
    insertCharacter(' ');
}

void AACTextBar::backspace()
{
    int pos = m_edit->cursorPosition();
    if (pos == 0)
        return;

    QString t = m_edit->text();
    t.remove(pos - 1, 1);
    m_edit->setText(t);
    m_edit->setCursorPosition(pos - 1);
    emit textChanged(t);
}

void AACTextBar::moveCursorLeft()
{
    int pos = m_edit->cursorPosition();
    if (pos > 0)
        m_edit->setCursorPosition(pos - 1);
}

void AACTextBar::moveCursorRight()
{
    int pos = m_edit->cursorPosition();
    if (pos < m_edit->text().length())
        m_edit->setCursorPosition(pos + 1);
}

void AACTextBar::appendWord(const QString& word)
{
    QString t = m_edit->text();
    if (!t.isEmpty())
        t += " ";
    t += word;

    m_edit->setText(t);
    m_edit->setCursorPosition(t.length());
    emit textChanged(t);
}
