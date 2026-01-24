#pragma once

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QString>
#include <QRegularExpression>

#include "aac/AACFramework.h"   // AACScreenAdapter, AACAccessibilityManager, AACPredictionEngine

class QLabel;
class QPushButton;

/**
 * AACMainScreen
 *
 * AAC-native text composition screen:
 *  - Text input buffer
 *  - Prediction bar
 *  - Full wiring to AACPredictionEngine
 *  - Implements AACScreenAdapter for layout + scanning
 */
class AACMainScreen : public QWidget, public AACScreenAdapter
{
    Q_OBJECT
public:
    explicit AACMainScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

    // AACScreenAdapter implementation
    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QLayout* rootLayout() const override;
    QWidget* predictiveStripContainer() const override;

signals:
    // Emitted when the user commits the current text (to speak/send)
    void textCommitted(const QString& text);

public slots:
    // External screens (symbol grid) can append words
    void setText(const QString& text);
    void appendWord(const QString& word);
    void onPredictionDwellActivated(QWidget* w); // called by AACInputController

private slots:
    void onTextChanged(const QString& text);
    void onPredictionClicked(QListWidgetItem* item);
    void onCommitButtonClicked();
    void onDeleteButtonClicked();

private:
    QString extractPrevWord(const QString& text) const;
    QString extractSecondLastWord(const QString& text) const;
    float confidenceForItem(QListWidgetItem* item) const;

    AACAccessibilityManager* m_aac = nullptr;
    AACPredictionEngine*     m_pred = nullptr;

    QVBoxLayout*   m_rootLayout = nullptr;
    QWidget*       m_predictiveStripContainer = nullptr;

    QLabel*        m_label = nullptr;
    QLineEdit*     m_text = nullptr;
    QListWidget*   m_predictionBar = nullptr;
    QPushButton*   m_commitButton = nullptr;
    QPushButton*   m_deleteButton = nullptr;

    QTimer         m_tickTimer;
};
