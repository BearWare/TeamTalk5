#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QStringList>

class AACAccessibilityManager;
class AACButton;

class PredictiveStrip : public QWidget {
    Q_OBJECT
public:
    explicit PredictiveStrip(AACAccessibilityManager* mgr, QWidget* parent = nullptr);

public slots:
    void setContext(const QString& text);          // called when text bar changes
    void onCharacterTyped(QChar ch);               // optional: refine predictions

signals:
    void suggestionChosen(const QString& word);    // emitted when user taps a suggestion

private:
    AACAccessibilityManager* m_mgr = nullptr;
    QHBoxLayout* m_layout = nullptr;
    QString m_lastContext;

    void rebuild(const QStringList& suggestions);
    QString lastWord(const QString& text) const;
};
