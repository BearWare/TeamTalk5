#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>

class AACAccessibilityManager;

class AACPredictionEngine : public QObject
{
    Q_OBJECT
public:
    explicit AACPredictionEngine(AACAccessibilityManager* mgr,
                                 QObject* parent = nullptr);

    // Stage 2: learn from full utterances
    void learnUtterance(const QString& text);

    // Core prediction API used by InChannelScreen and predictive strip
    std::vector<std::string> Predict(const std::string& prefix,
                                     int maxSuggestions) const;

    // Stage 4: persistence
    bool loadFromFile(const QString& path);
    bool saveToFile(const QString& path) const;

    // Stage 5: vocabulary boosting
    void boostToken(const QString& token);

private:
    AACAccessibilityManager* m_mgr = nullptr;

    // Stage 2: unigram + bigram
    std::unordered_map<std::string, int> m_unigram;
    std::unordered_map<std::string,
        std::unordered_map<std::string, int>> m_bigram;

    // Stage 8: trigram
    std::unordered_map<std::string,
        std::unordered_map<std::string,
            std::unordered_map<std::string, int>>> m_trigram;

    // Stage 10: personal dictionary (Option B: repeated words only)
    std::unordered_map<std::string, int> m_wordSeenCount;
    std::unordered_set<std::string> m_customWords;

    // Stage 7: recency + phrase memory
    struct PhraseEntry {
        std::string phrase;
        int count = 0;
        int recencyTick = 0;
    };
    mutable int m_recencyCounter = 0;
    std::unordered_map<std::string, PhraseEntry> m_phrases; // full utterances

    // Tokenization helpers (Stage 2 + 3)
    std::vector<std::string> tokenize(const QString& text) const;
    QString normalizePunctuation(const QString& text) const;

    // Stage 7 helpers
    void learnPhrase(const QString& text);
    std::vector<std::string> phraseSuggestions(const std::string& prefix,
                                               int maxSuggestions) const;

    // Stage 11: fuzzy matching helper
    int fuzzyDistance(const std::string& a, const std::string& b) const;
    bool fuzzyCloseEnough(const std::string& typed,
                          const std::string& candidate) const;
};
