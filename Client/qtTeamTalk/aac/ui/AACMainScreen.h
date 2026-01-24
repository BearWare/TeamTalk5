#pragma once

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QString>
#include <QRegularExpression>

#include "AACFramework.h" // for AACScreenAdapter, AACAccessibilityManager, AACPredictionEngine

class QLabel;

/**
 * AACMainScreen
 *
 * Main AAC text composition screen:
 * - Text input buffer
 * - Prediction bar
 * - Hooks into AACPredictionEngine via AACAccessibilityManager
 * - Implements AACScreenAdapter for layout + scanning
 */
class AACMainScreen : public QWidget, public AACScreenAdapter
{
    Q_OBJECT
public:
    explicit AACMainScreen(AACAccessibilityManager* mgr, QWidget* parent = nullptr);

    // AACScreenAdapter implementation
    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QLayout* rootLayout() const override;
    QWidget* predictiveStripContainer() const override;

signals:
    void textCommitted(const QString& text);

public slots:
    void setText(const QString& text);

private slots:
    void onTextChanged(const QString& text);
    void onPredictionClicked(QListWidgetItem* item);
    void onCommitButtonClicked();
    void onDeleteButtonClicked();

private:
    QString extractPrevWord(const QString& text) const;
    QString extractSecondLastWord(const QString& text) const;

    AACAccessibilityManager* m_mgr = nullptr;
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
