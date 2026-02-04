#include "AACKeyboardScreen.h"
#include "AACKeyButton.h"
#include "AACAccessibilityManager.h"

#include <QGridLayout>

AACKeyboardScreen::AACKeyboardScreen(AACAccessibilityManager* mgr, QWidget* parent)
    : AACScreen(mgr, parent)
{
    m_grid = new QGridLayout(this);
    setLayout(m_grid);
    buildKeyboard();
}

void AACKeyboardScreen::buildKeyboard()
{
    // Clear any existing layout if needed (not strictly necessary on first build)
    // ...

    // QWERTY rows
    addRow({"Q","W","E","R","T","Y","U","I","O","P"}, 0);
    addRow({"A","S","D","F","G","H","J","K","L"},     1);
    addRow({"Z","X","C","V","B","N","M"},            2);

    // Control row: space, backspace, clear, delete word, arrows
    addControlRow(3);
}

void AACKeyboardScreen::addRow(const QStringList& keys, int row)
{
    QVector<AACKeyButton*> rowButtons;

    int col = 0;
    for (const QString& key : keys) {
        AACKeyButton* btn = new AACKeyButton(key, m_aac, this);

        // Character typing
        connect(btn, &AACKeyButton::clicked,
                this, [this, key]() {
                    if (!key.isEmpty())
                        emit characterTyped(key[0]);
                });

        m_grid->addWidget(btn, row, col++);
        registerInteractive(btn, true);

        // Store in map for highlighting / context
        if (!key.isEmpty())
            m_keyMap.insert(key[0].toUpper(), btn);

        rowButtons.append(btn);
    }

    m_rows.append(rowButtons);
}

void AACKeyboardScreen::addControlRow(int row)
{
    int col = 0;

    // Space
    m_spaceBtn = new AACKeyButton("Space", m_aac, this);
    connect(m_spaceBtn, &AACKeyButton::clicked,
            this, [this]() { emit spaceRequested(); });
    m_grid->addWidget(m_spaceBtn, row, col++, 1, 2); // span 2 columns
    registerInteractive(m_spaceBtn, true);

    // Backspace
    m_backspaceBtn = new AACKeyButton("⌫", m_aac, this);
    connect(m_backspaceBtn, &AACKeyButton::clicked,
            this, [this]() { emit backspaceRequested(); });
    m_grid->addWidget(m_backspaceBtn, row, col++);
    registerInteractive(m_backspaceBtn, true);

    // Clear utterance
    m_clearBtn = new AACKeyButton("Clear", m_aac, this);
    connect(m_clearBtn, &AACKeyButton::clicked,
            this, [this]() { emit clearRequested(); });
    m_grid->addWidget(m_clearBtn, row, col++);
    registerInteractive(m_clearBtn, true);

    // Delete last word
    m_deleteWordBtn = new AACKeyButton("DelWord", m_aac, this);
    connect(m_deleteWordBtn, &AACKeyButton::clicked,
            this, [this]() { emit deleteWordRequested(); });
    m_grid->addWidget(m_deleteWordBtn, row, col++);
    registerInteractive(m_deleteWordBtn, true);

    // Cursor left
    AACKeyButton* leftBtn = new AACKeyButton("◀", m_aac, this);
    connect(leftBtn, &AACKeyButton::clicked,
            this, [this]() { emit moveCursorLeft(); });
    m_grid->addWidget(leftBtn, row, col++);
    registerInteractive(leftBtn, true);

    // Cursor right
    AACKeyButton* rightBtn = new AACKeyButton("▶", m_aac, this);
    connect(rightBtn, &AACKeyButton::clicked,
            this, [this]() { emit moveCursorRight(); });
    m_grid->addWidget(rightBtn, row, col++);
    registerInteractive(rightBtn, true);

    QVector<AACKeyButton*> rowButtons;
    rowButtons << m_spaceBtn << m_backspaceBtn << m_clearBtn << m_deleteWordBtn
               << leftBtn << rightBtn;
    m_rows.append(rowButtons);
}

// Called by AACMainScreen when the text bar changes
void AACKeyboardScreen::setText(const QString& text)
{
    m_currentText = text;

    updateCursorHighlight();
    updateContextualKeys();
    updateScanningState();
    updatePredictionHints();
}

// Optional: called when predictive strip updates suggestions
void AACKeyboardScreen::setSuggestions(const QStringList& suggestions)
{
    m_suggestions = suggestions;
    updatePredictionHints();
}

// --- Highlighting helpers ---

void AACKeyboardScreen::clearAllHighlights()
{
    for (auto btn : m_keyMap.values())
        setButtonHighlighted(btn, false);

    if (m_spaceBtn)
        setButtonHighlighted(m_spaceBtn, false);
}

void AACKeyboardScreen::setKeyHighlighted(QChar ch, bool on)
{
    ch = ch.toUpper();
    if (m_keyMap.contains(ch))
        setButtonHighlighted(m_keyMap.value(ch), on);
}

void AACKeyboardScreen::setButtonHighlighted(AACKeyButton* btn, bool on)
{
    if (!btn)
        return;

    // Simple visual: use a property or stylesheet
    btn->setProperty("highlighted", on);
    btn->setStyleSheet(on ? "background-color: yellow;" : "");
}

// --- Future‑proofed behaviour blocks ---

void AACKeyboardScreen::updateCursorHighlight()
{
    clearAllHighlights();

    // Very simple heuristic for "next likely character":
    // If text is empty → highlight a vowel (e.g. 'A')
    // If last char is not space → highlight Space
    // If last char is space → highlight 'I' as a common starter
    if (m_currentText.isEmpty()) {
        setKeyHighlighted('A', true);
        return;
    }

    const QChar last = m_currentText.back();
    if (!last.isSpace()) {
        if (m_spaceBtn)
            setButtonHighlighted(m_spaceBtn, true);
    } else {
        setKeyHighlighted('I', true);
    }
}

void AACKeyboardScreen::updateContextualKeys()
{
    const bool empty = m_currentText.trimmed().isEmpty();

    if (m_backspaceBtn)
        m_backspaceBtn->setEnabled(!empty);

    if (m_clearBtn)
        m_clearBtn->setEnabled(!empty);

    if (m_deleteWordBtn)
        m_deleteWordBtn->setEnabled(!empty);
}

void AACKeyboardScreen::updateScanningState()
{
    // You’re already integrated with dwell/scanning via registerInteractive().
    // This method is the hook for more advanced scanning:
    //
    // - row/column scanning order (m_rows)
    // - passing structured layout to AACAccessibilityManager
    //
    // Example (pseudo‑API):
    //
    // if (m_aac) {
    //     m_aac->setScanningLayout(m_rows);
    // }
}

void AACKeyboardScreen::updatePredictionHints()
{
    // Clear previous prediction highlights, but keep cursor highlight logic
    clearAllHighlights();
    updateCursorHighlight();

    if (m_suggestions.isEmpty())
        return;

    // Highlight keys whose label matches the first letter of any suggestion
    QSet<QChar> firstLetters;
    for (const QString& s : m_suggestions) {
        if (!s.isEmpty())
            firstLetters.insert(s[0].toUpper());
    }

    for (QChar ch : firstLetters)
        setKeyHighlighted(ch, true);
}
