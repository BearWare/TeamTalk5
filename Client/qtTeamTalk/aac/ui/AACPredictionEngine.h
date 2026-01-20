#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include <unordered_map>
#include <vector>
#include <string>

class AACAccessibilityManager;

class AACPredictionEngine : public QObject
{
    Q_OBJECT
public:
    explicit AACPredictionEngine(AACAccessibilityManager* mgr,
                                 QObject* parent = nullptr);

    // Stage 2: learn from full utterances
    void learnUtterance(const QString& text);

    // Core prediction API used by InChannelScreen
    std::vector<std::string> Predict(const std::string& prefix,
                                     int maxSuggestions) const;

    // Stage 4: persistence
    bool loadFromFile(const QString& path);
    bool saveToFile(const QString& path) const;

    // Stage 5: vocabulary boosting (called from AACAccessibilityManager::boostPredictionVocabulary)
    void boostToken(const QString& token);

private:
    AACAccessibilityManager* m_mgr = nullptr;

    // unigram: word -> count
    std::unordered_map<std::string, int> m_unigram;

    // bigram: prevWord -> (nextWord -> count)
    std::unordered_map<std::string,
        std::unordered_map<std::string, int>> m_bigram;

    // Tokenization helpers (Stage 2 + 3)
    std::vector<std::string> tokenize(const QString& text) const;
    QString normalizePunctuation(const QString& text) const;
};
