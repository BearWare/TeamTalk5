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

    // Learning
    void learnUtterance(const QString& text);
    void boostToken(const QString& token);

    // Prediction
    std::vector<std::string> Predict(const std::string& prefix,
                                     int maxSuggestions) const;

    // Stage 12: confidence API for UI
    float confidenceFor(const std::string& token) const;

    // Stage 4: persistence
    bool saveToFile(const QString& path) const;
    bool loadFromFile(const QString& path);

    // --- Future adaptive hooks (Stages 13–20) ---
    // These compile as no-ops for now; you can wire them up later
    void reinforceChoice(const std::string& /*prev*/,
                         const std::string& /*chosen*/) {}
    void penalizeIgnored(const std::string& /*prev*/,
                         const std::vector<std::string>& /*shown*/,
                         const std::string& /*actualTyped*/) {}
    void reinforceDwellChoice(const std::string& /*prev*/,
                              const std::string& /*chosen*/) {}

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

    // Internal data structures
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
};

#endif // AACPREDICTIONENGINE_H
