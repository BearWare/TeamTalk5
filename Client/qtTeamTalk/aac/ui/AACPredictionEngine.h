#ifndef AACPREDICTIONENGINE_H
#define AACPREDICTIONENGINE_H

#include <QObject>
#include <QString>
#include <QSet>

#include <string>
#include <vector>
#include <unordered_map>

class AACAccessibilityManager;

class AACPredictionEngine : public QObject
{
    Q_OBJECT
public:
    explicit AACPredictionEngine(AACAccessibilityManager* mgr,
                                 QObject* parent = nullptr);

    // Stage 1–2: learning
    void learnUtterance(const QString& text);
    void boostToken(const QString& token);

    // Stage 1–12: prediction
    std::vector<std::string> Predict(const std::string& prefix,
                                     int maxSuggestions) const;

    // Stage 12: confidence API for UI
    float confidenceFor(const std::string& token) const;

    // Stage 13: positive reinforcement (chosen prediction)
    void reinforceChoice(const std::string& prev,
                         const std::string& chosen);

    // Stage 14: negative reinforcement (ignored predictions)
    void penalizeIgnored(const std::string& prev,
                         const std::vector<std::string>& shown,
                         const std::string& actualTyped);

    // Stage 20: dwell reinforcement (same as click)
    void reinforceDwellChoice(const std::string& prev,
                              const std::string& chosen);

    // Stage 4: persistence
    bool saveToFile(const QString& path) const;
    bool loadFromFile(const QString& path);

private:
    // Helpers
    QString normalizePunctuation(const QString& text) const;
    std::vector<std::string> tokenize(const QString& text) const;

    int fuzzyDistance(const std::string& a,
                      const std::string& b) const;
    bool fuzzyCloseEnough(const std::string& typed,
                          const std::string& candidate) const;

    std::vector<std::string> phraseSuggestions(const std::string& prefix,
                                               int maxSuggestions) const;

    // Stage 12+ scoring helper
    float scoreCandidate(const std::string& prev,
                         const std::string& candidate) const;

private:
    AACAccessibilityManager* m_mgr = nullptr;

    // Stage 2: unigrams, bigrams
    std::unordered_map<std::string, int> m_unigram;
    std::unordered_map<std::string,
        std::unordered_map<std::string, int>> m_bigram;

    // Stage 8: trigrams
    std::unordered_map<std::string,
        std::unordered_map<std::string,
            std::unordered_map<std::string, int>>> m_trigram;

    // Stage 7: phrase memory
    struct PhraseEntry {
        std::string phrase;
        int count = 0;
        int recencyTick = 0;
    };
    std::unordered_map<std::string, PhraseEntry> m_phrases;
    int m_recencyCounter = 0;

    // Stage 10: personal dictionary
    QSet<std::string> m_customWords;
    std::unordered_map<std::string, int> m_wordSeenCount;

    // Stage 12: cached total for confidence
    mutable int m_cachedTotalUnigrams = -1;

    // Stage 15: session boosts
    std::unordered_map<std::string, float> m_sessionBoost;
    std::unordered_map<std::string,
        std::unordered_map<std::string, float>> m_sessionBigramBoost;

    // Stage 19: stability smoothing (hook, not yet used heavily)
    mutable std::vector<std::string> m_lastStable;
};

#endif // AACPREDICTIONENGINE_H
