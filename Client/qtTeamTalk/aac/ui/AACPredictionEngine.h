#ifndef AACPREDICTIONENGINE_H
#define AACPREDICTIONENGINE_H

#include <QObject>
#include <QString>
#include <QSet>

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>

class AACAccessibilityManager;

class AACPredictionEngine : public QObject
{
    Q_OBJECT
public:
    explicit AACPredictionEngine(AACAccessibilityManager* mgr,
                                 QObject* parent = nullptr);

    // Learning (Stages 1–2, 5, 7, 8, 10)
    void learnUtterance(const QString& text);
    void boostToken(const QString& token);

    // Prediction (Stages 1–20 core prediction path)
    std::vector<std::string> Predict(const std::string& prefix,
                                     int maxSuggestions) const;

    // Stage 12: confidence API for UI
    float confidenceFor(const std::string& token) const;

    // Stage 13: positive reinforcement (chosen prediction)
    void reinforceChoice(const std::string& prev,
                         const std::string& chosen);

    // Stage 14: negative reinforcement
    void penalizeIgnored(const std::string& prev,
                         const std::vector<std::string>& shown,
                         const std::string& actualTyped);
    void onUserSelected(const std::string& prev,
                        const std::string& chosen,
                        const std::vector<std::string>& shown);
    void onUserDeletedAutocompleted(const std::string& token);
    void onPredictionBarShown();
    void tick(); // call periodically for ignore‑timeout

    // Stage 20: dwell reinforcement (same as click, but stronger in future)
    void reinforceDwellChoice(const std::string& prev,
                              const std::string& chosen);

    // Stage 4: persistence
    bool saveToFile(const QString& path) const;
    bool loadFromFile(const QString& path);

    // Stage 18: AAC context hooks (to be wired from AACFramework)
    void setCurrentCategory(const QString& category);
    void setLastSymbolWord(const QString& word);

    // Stage 19: stability / freeze control (optional external use)
    void freezePredictions();
    void unfreezePredictions();

private:
    // Context struct for Stage 17 (multi-word + punctuation context)
    struct Context {
        std::string lastWord;    // w_{-1}
        std::string prevWord;    // w_{-2}
        bool sentenceStart = false;
        bool afterComma = false;
        bool afterQuestion = false;
    };

    // Helpers
    QString normalizePunctuation(const QString& text) const;
    std::vector<std::string> tokenize(const QString& text) const;

    int fuzzyDistance(const std::string& a,
                      const std::string& b) const;
    bool fuzzyCloseEnough(const std::string& typed,
                          const std::string& candidate) const;

    std::vector<std::string> phraseSuggestions(const std::string& prefix,
                                               int maxSuggestions) const;

    // Stage 17: build context from prefix
    Context buildContext(const QStringList& parts) const;

    // Stage 12+17+18: scoring helper
    float scoreCandidate(const Context& ctx,
                         const std::string& candidate) const;

    // Stage 14: internal penalty helper
    void applyPenalty(const std::string& token, float basePenalty);

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

    // Stage 18: AAC context
    QString m_currentCategory;          // e.g. "Food", "People"
    std::string m_lastSymbolWord;       // last symbol's associated word (if any)

    // Stage 19: stability
    mutable std::vector<std::string> m_lastStable;
    bool m_predictionsFrozen = false;
    float m_stabilityThreshold = 0.7f;  // overlap threshold for hysteresis

    // Stage 14: negative reinforcement state
    struct PenaltyConfig {
        float globalPenaltyFactor = 0.2f;
        float decayFactor = 0.95f;
        std::unordered_map<std::string, float> categoryPenalty;
    };

    PenaltyConfig m_penaltyCfg;
    std::unordered_map<std::string, int> m_positiveCount;
    std::unordered_map<std::string, int> m_negativeCount;

    bool m_predictionBarVisible = false;
    std::chrono::steady_clock::time_point m_predictionBarShown;
    int m_ignoreThresholdMs = 2000;
    std::string m_lastTopPrediction;
};

#endif // AACPREDICTIONENGINE_H
