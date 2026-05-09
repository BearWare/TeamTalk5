#pragma once

#include "AACScreen.h"

#include <QMap>
#include <QVector>

class QGridLayout;
class AACKeyButton;

class AACKeyboardScreen : public AACScreen {
    Q_OBJECT
public:
    explicit AACKeyboardScreen(AACAccessibilityManager* mgr, QWidget* parent = nullptr);

signals:
    void characterTyped(QChar ch);
    void backspaceRequested();
    void moveCursorLeft();
    void moveCursorRight();
    void spaceRequested();
    void clearRequested();
    void deleteWordRequested();

public slots:
    // Called by AACMainScreen when the text bar changes
    void setText(const QString& text);

    // Optional: called when predictive strip updates suggestions
    void setSuggestions(const QStringList& suggestions);

private:
    void buildKeyboard();
    void addRow(const QStringList& keys, int row);
    void addControlRow(int row);

    void updateCursorHighlight();
    void updateContextualKeys();
    void updateScanningState();
    void updatePredictionHints();

    void clearAllHighlights();
    void setKeyHighlighted(QChar ch, bool on);
    void setButtonHighlighted(AACKeyButton* btn, bool on);

    QGridLayout* m_grid = nullptr;

    // Current text from AACTextBar
    QString m_currentText;

    // Current suggestions from PredictiveStrip
    QStringList m_suggestions;

    // Map from character to key button (for highlighting, enabling, etc.)
    QMap<QChar, AACKeyButton*> m_keyMap;

    // Special keys
    AACKeyButton* m_backspaceBtn = nullptr;
    AACKeyButton* m_spaceBtn     = nullptr;
    AACKeyButton* m_clearBtn     = nullptr;
    AACKeyButton* m_deleteWordBtn = nullptr;

    // For scanning / row/column logic
    QVector<QVector<AACKeyButton*>> m_rows;
};
