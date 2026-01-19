#pragma once
#include <QObject>
#include <QHash>
#include <QStringList>

class AACPredictionEngine : public QObject {
    Q_OBJECT
public:
    explicit AACPredictionEngine(QObject* parent = nullptr);

    QStringList suggest(const QString& context, int max = 5) const;
    void learnUtterance(const QString& utterance);

private:
    // lastWord -> (word -> count)
    QHash<QString, QHash<QString, int>> m_nextWordCounts;
    // global word frequency
    QHash<QString, int> m_globalCounts;
};
