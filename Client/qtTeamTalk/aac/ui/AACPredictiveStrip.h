#pragma once

#include <QWidget>
#include <QStringList>

class AACAccessibilityManager;

class AACPredictiveStrip : public QWidget {
    Q_OBJECT
public:
    explicit AACPredictiveStrip(AACAccessibilityManager* aac, QWidget* parent = nullptr);

    void setSuggestions(const QStringList& words);

signals:
    void suggestionActivated(const QString& word);
    void suggestionsUpdated(const QStringList& suggestions);

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;

private:
    AACAccessibilityManager* m_aac = nullptr;
    QStringList m_words;
};
