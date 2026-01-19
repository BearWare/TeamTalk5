#include "AACPredictionEngine.h"

AACPredictionEngine::AACPredictionEngine(QObject* parent)
    : QObject(parent)
{}

void AACPredictionEngine::learnUtterance(const QString& utterance)
{
    const QStringList words = utterance.split(QRegularExpression("\\s+"),
                                              Qt::SkipEmptyParts);
    for (int i = 0; i < words.size(); ++i) {
        const QString w = words[i].toLower();
        m_globalCounts[w]++;

        if (i + 1 < words.size()) {
            const QString next = words[i+1].toLower();
            m_nextWordCounts[w][next]++;
        }
    }
}

QStringList AACPredictionEngine::suggest(const QString& context, int max) const
{
    QString last;
    {
        const QStringList words = context.split(QRegularExpression("\\s+"),
                                               Qt::SkipEmptyParts);
        if (!words.isEmpty())
            last = words.last().toLower();
    }

    QList<QPair<QString,int>> candidates;

    if (!last.isEmpty() && m_nextWordCounts.contains(last)) {
        const auto& map = m_nextWordCounts[last];
        for (auto it = map.begin(); it != map.end(); ++it)
            candidates.append({it.key(), it.value()});
    } else {
        for (auto it = m_globalCounts.begin(); it != m_globalCounts.end(); ++it)
            candidates.append({it.key(), it.value()});
    }

    std::sort(candidates.begin(), candidates.end(),
              [](auto a, auto b){ return a.second > b.second; });

    QStringList out;
    for (int i = 0; i < candidates.size() && out.size() < max; ++i)
        out << candidates[i].first;
    return out;
}
