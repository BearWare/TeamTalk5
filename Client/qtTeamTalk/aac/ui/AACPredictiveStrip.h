#pragma once

#include <QWidget>
#include <QTimer>
#include <QList>
#include <vector>
#include <string>

class QHBoxLayout;
class AACAccessibilityManager;
class AACTextBar;
class AACButton;

class PredictiveStrip : public QWidget
{
    Q_OBJECT
public:
    explicit PredictiveStrip(QWidget* parent = nullptr);

    void setManager(AACAccessibilityManager* mgr);
    void setTextBar(AACTextBar* tb);

public slots:
    // Core logic: called when the text bar changes
    void setContext(const QString& text);

    // Optional logic: called when the keyboard emits characterTyped()
    void onCharacterTyped(const QString& ch);

signals:
    void suggestionChosen(const QString& word);

private slots:
    void debouncedUpdate();

private:
    void updateButtons(const std::vector<std::string>& suggestions);
    void animateButtonIn(AACButton* btn);
    void applyAdaptiveSizing(AACButton* btn);

    AACAccessibilityManager* m_mgr = nullptr;
    AACTextBar* m_textBar = nullptr;
    QHBoxLayout* m_layout = nullptr;

    QTimer m_debounceTimer;
    QString m_pendingContext;

    QList<AACButton*> m_buttons;
};
