#include "AACPredictionEngine.h"
#include "AACFramework.h"

#include <QRegularExpression>
#include <QFile>
#include <QTextStream>

#include <algorithm>
#include <cctype>
#include <limits>
#include <cmath>

AACPredictionEngine::AACPredictionEngine(AACAccessibilityManager* mgr,
                                         QObject* parent)
    : QObject(parent)
    , m_mgr(mgr)
{
}

// -------------------------
// Tokenization helpers (2,3)
// -------------------------

QString AACPredictionEngine::normalizePunctuation(const QString& text) const
{
    QString cleaned = text;
    cleaned.replace(".", " . ");
    cleaned.replace("!", " ! ");
    cleaned.replace("?", " ? ");
    cleaned.replace(",", " , "); // for afterComma context
    return cleaned;
}

std::vector<std::string> AACPredictionEngine::tokenize(const QString& text) const
{
    QString cleaned = normalizePunctuation(text);

    QStringList parts = cleaned.split(QRegularExpression("\\s+"),
                                      Qt::SkipEmptyParts);

    std::vector<std::string> out;
    out.reserve(parts.size());
    for (const auto& p : parts)
        out.push_back(p.toLower().toStdString());

    return out;
}

// -------------------------
// Stage 2 + 7 + 8 + 10: learning
// -------------------------

void AACPredictionEngine::learnUtterance(const QString& text)
{
    auto tokens = tokenize(text);
    if (tokens.empty())
        return;

    // Unigrams + word seen counts (Stage 2 + 10)
    for (const auto& w : tokens) {
        ++m_unigram[w];
        m_cachedTotalUnigrams = -1; // invalidate cache

        int& seen = m_wordSeenCount[w];
        ++seen;
        // Stage 10: add to custom dictionary on second occurrence
        if (seen == 2)
            m_customWords.insert(w);
    }

    // Bigrams (Stage 2)
    for (size_t i = 1; i < tokens.size(); ++i) {
        const std::string& prev = tokens[i - 1];
        const std::string& next = tokens[i];
        ++m_bigram[prev][next];
    }

    // Trigrams (Stage 8)
    if (tokens.size() >= 3) {
        for (size_t i = 2; i < tokens.size(); ++i) {
            const std::string& w1 = tokens[i - 2];
            const std::string& w2 = tokens[i - 1];
            const std::string& w3 = tokens[i];
            ++m_trigram[w1][w2][w3];
        }
    }

    // Stage 7: phrase memory
    const QString trimmed = text.trimmed();
    if (!trimmed.isEmpty()) {
        std::string key = trimmed.toLower().toStdString();
        auto& entry = m_phrases[key];
        entry.phrase = key;
        entry.count += 1;
        entry.recencyTick = ++m_recencyCounter;
    }
}

// -------------------------
// Stage 5: vocabulary boosting
// -------------------------

void AACPredictionEngine::boostToken(const QString& token)
{
    std::string w = token.trimmed().toLower().toStdString();
    if (w.empty())
        return;

    m_unigram[w] += 10;
    m_cachedTotalUnigrams = -1;
}

// -------------------------
// Stage 11: fuzzy matching
// -------------------------

int AACPredictionEngine::fuzzyDistance(const std::string& a,
                                       const std::string& b) const
{
    int mismatches = 0;
    size_t len = std::min(a.size(), b.size());
    for (size_t i = 0; i < len; ++i) {
        if (a[i] != b[i])
            ++mismatches;
    }
    mismatches += static_cast<int>(std::max(a.size(), b.size()) - len);
    return mismatches;
}

bool AACPredictionEngine::fuzzyCloseEnough(const std::string& typed,
                                           const std::string& candidate) const
{
    if (typed.empty())
        return false;
    int d = fuzzyDistance(typed, candidate);
    return d <= 2;
}

// -------------------------
// Stage 7: phrase suggestions (recency + frequency)
// -------------------------

std::vector<std::string> AACPredictionEngine::phraseSuggestions(const std::string& prefix,
                                                                int maxSuggestions) const
{
    std::vector<std::string> out;
    if (maxSuggestions <= 0)
        return out;

    const std::string lowerPrefix = QString::fromStdString(prefix)
                                        .trimmed()
                                        .toLower()
                                        .toStdString();

    if (lowerPrefix.empty())
        return out;

    struct Scored {
        std::string phrase;
        int score;
        int recency;
    };

    std::vector<Scored> candidates;
    candidates.reserve(m_phrases.size());

    for (const auto& kv : m_phrases) {
        const auto& entry = kv.second;
        if (entry.phrase.rfind(lowerPrefix, 0) == 0) { // starts with prefix
            int score = entry.count * 10 + entry.recencyTick; // freq + recency
            candidates.push_back({ entry.phrase, score, entry.recencyTick });
        }
    }

    std::sort(candidates.begin(), candidates.end(),
              [](const Scored& a, const Scored& b) {
                  return a.score > b.score;
              });

    for (const auto& c : candidates) {
        out.push_back(c.phrase);
        if ((int)out.size() >= maxSuggestions)
            break;
    }

    return out;
}

// -------------------------
// Stage 17: build context
// -------------------------

AACPredictionEngine::Context
AACPredictionEngine::buildContext(const QStringList& parts) const
{
    Context ctx;

    if (parts.isEmpty()) {
        ctx.sentenceStart = true;
        return ctx;
    }

    QString last = parts.last();
    if (last == "." || last == "!" || last == "?") {
        ctx.sentenceStart = true;
    } else {
        ctx.lastWord = last.toLower().toStdString();
    }

    if (parts.size() >= 2) {
        QString prev = parts[parts.size() - 2];
        ctx.prevWord = prev.toLower().toStdString();
        if (prev == ",")
            ctx.afterComma = true;
        if (prev == "?")
            ctx.afterQuestion = true;
    }

    return ctx;
}

// -------------------------
// Stage 12+17+18+14: scoring helper
// -------------------------

float AACPredictionEngine::scoreCandidate(const Context& ctx,
                                          const std::string& candidate) const
{
    float score = 0.0f;

    // Unigram weight
    auto itU = m_unigram.find(candidate);
    if (itU != m_unigram.end())
        score += itU->second * 1.0f;

    // Bigram weight (lastWord -> candidate)
    if (!ctx.lastWord.empty()) {
        auto itB = m_bigram.find(ctx.lastWord);
        if (itB != m_bigram.end()) {
            auto it2 = itB->second.find(candidate);
            if (it2 != itB->second.end())
                score += it2->second * 2.0f;
        }
    }

    // Trigram weight (prevWord, lastWord -> candidate)
    if (!ctx.prevWord.empty() && !ctx.lastWord.empty()) {
        auto it1 = m_trigram.find(ctx.prevWord);
        if (it1 != m_trigram.end()) {
            auto it2 = it1->second.find(ctx.lastWord);
            if (it2 != it1->second.end()) {
                auto it3 = it2->second.find(candidate);
                if (it3 != it2->second.end())
                    score += it3->second * 3.0f;
            }
        }
    }

    // Session boosts (Stage 15)
    auto itSB = m_sessionBoost.find(candidate);
    if (itSB != m_sessionBoost.end())
        score += itSB->second;

    if (!ctx.lastWord.empty()) {
        auto itSBB = m_sessionBigramBoost.find(ctx.lastWord);
        if (itSBB != m_sessionBigramBoost.end()) {
            auto it2 = itSBB->second.find(candidate);
            if (it2 != itSBB->second.end())
                score += it2->second;
        }
    }

    // Stage 18: AAC category weighting (simple, safe)
    if (!m_currentCategory.isEmpty()) {
        auto itP = m_phrases.find(m_currentCategory.toLower().toStdString());
        if (itP != m_phrases.end()) {
            score += 0.5f;
        }
    }

    // Stage 18: last symbol word weighting
    if (!m_lastSymbolWord.empty() && candidate == m_lastSymbolWord) {
        score += 2.0f; // strong bias toward repeating last symbol word
    }

    // Punctuation context: after question, boost "yes/no" style words slightly
    if (ctx.afterQuestion) {
        if (candidate == "yes" || candidate == "no")
            score += 1.5f;
    }

    // Stage 14: negative reinforcement penalty
    auto negIt = m_negativeCount.find(candidate);
    if (negIt != m_negativeCount.end()) {
        float penalty = m_penaltyCfg.globalPenaltyFactor * negIt->second;
        score -= penalty;
    }

    return score;
}

// -------------------------
// Stage 2,3,7,8,9,10,11,12,17,18,19: prediction
// -------------------------

std::vector<std::string> AACPredictionEngine::Predict(const std::string& prefix,
                                                      int maxSuggestions) const
{
    std::vector<std::string> result;
    if (maxSuggestions <= 0)
        return result;

    // Stage 19: freeze — if frozen and we have a stable list, return it
    if (m_predictionsFrozen && !m_lastStable.empty())
        return m_lastStable;

    // Stage 7: phrase-level suggestions first
    auto phraseRes = phraseSuggestions(prefix, maxSuggestions);
    for (auto& p : phraseRes)
        result.push_back(p);
    if ((int)result.size() >= maxSuggestions) {
        if (!result.empty())
            m_lastTopPrediction = result.front();
        m_lastStable = result;
        return result;
    }

    if (prefix.empty()) {
        // No context: top unigrams
        std::vector<std::pair<std::string,int>> uni(m_unigram.begin(), m_unigram.end());
        std::sort(uni.begin(), uni.end(),
                  [](auto& a, auto& b){ return a.second > b.second; });

        for (auto& u : uni) {
            result.push_back(u.first);
            if ((int)result.size() >= maxSuggestions)
                break;
        }
        if (!result.empty())
            m_lastTopPrediction = result.front();
        m_lastStable = result;
        return result;
    }

    // Punctuation-aware context (Stage 3 + 9 + 17)
    QString qPrefix = QString::fromStdString(prefix);
    QString cleaned = normalizePunctuation(qPrefix);

    QStringList parts = cleaned.split(QRegularExpression("\\s+"),
                                      Qt::SkipEmptyParts);

    Context ctx = buildContext(parts);

    // Stage 8: Trigram context (w1, w2 -> w3)
    if (!ctx.prevWord.empty() && !ctx.lastWord.empty()) {
        auto it1 = m_trigram.find(ctx.prevWord);
        if (it1 != m_trigram.end()) {
            auto it2 = it1->second.find(ctx.lastWord);
            if (it2 != it1->second.end()) {
                std::vector<std::pair<std::string,int>> triCandidates(
                    it2->second.begin(), it2->second.end());

                std::sort(triCandidates.begin(), triCandidates.end(),
                          [](auto& a, auto& b){ return a.second > b.second; });

                for (auto& c : triCandidates) {
                    if (std::find(result.begin(), result.end(), c.first) == result.end()) {
                        result.push_back(c.first);
                        if ((int)result.size() >= maxSuggestions)
                            break;
                    }
                }
            }
        }

        if ((int)result.size() >= maxSuggestions) {
            if (!result.empty())
                m_lastTopPrediction = result.front();
            m_lastStable = result;
            return result;
        }
    }

    // 1) Bigram context (Stage 2)
    if (!ctx.lastWord.empty()) {
        auto itBig = m_bigram.find(ctx.lastWord);
        if (itBig != m_bigram.end()) {
            std::vector<std::pair<std::string,int>> candidates(
                itBig->second.begin(), itBig->second.end());

            std::sort(candidates.begin(), candidates.end(),
                      [](auto& a, auto& b){ return a.second > b.second; });

            for (auto& c : candidates) {
                if (std::find(result.begin(), result.end(), c.first) == result.end()) {
                    result.push_back(c.first);
                    if ((int)result.size() >= maxSuggestions)
                        break;
                }
            }
        }
    }

    // 2) Personal dictionary + fuzzy matching (Stage 10 + 11)
    if (!ctx.lastWord.empty() && (int)result.size() < maxSuggestions) {
        for (const auto& w : m_customWords) {
            if (fuzzyCloseEnough(ctx.lastWord, w)) {
                if (std::find(result.begin(), result.end(), w) == result.end()) {
                    result.push_back(w);
                    if ((int)result.size() >= maxSuggestions)
                        break;
                }
            }
        }
    }

    // 3) Fill with top unigrams (Stage 2,12,17,18,14)
    if ((int)result.size() < maxSuggestions) {
        std::vector<std::pair<std::string,int>> uni(m_unigram.begin(), m_unigram.end());
        std::sort(uni.begin(), uni.end(),
                  [this, &ctx](auto& a, auto& b){
                      float sa = scoreCandidate(ctx, a.first);
                      float sb = scoreCandidate(ctx, b.first);
                      if (sa == sb)
                          return a.second > b.second;
                      return sa > sb;
                  });

        for (auto& u : uni) {
            if (std::find(result.begin(), result.end(), u.first) == result.end()) {
                result.push_back(u.first);
                if ((int)result.size() >= maxSuggestions)
                    break;
            }
        }
    }

    // Stage 9: Capitalize at sentence start
    if (ctx.sentenceStart) {
        for (auto& s : result) {
            if (!s.empty())
                s[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));
        }
    }

    // Stage 19: stability layer (hysteresis)
    if (!m_lastStable.empty()) {
        int overlap = 0;
        for (const auto& s : result) {
            if (std::find(m_lastStable.begin(), m_lastStable.end(), s) != m_lastStable.end())
                ++overlap;
        }
        float overlapRatio = 0.0f;
        int denom = std::max((int)result.size(), (int)m_lastStable.size());
        if (denom > 0)
            overlapRatio = static_cast<float>(overlap) / static_cast<float>(denom);

        if (overlapRatio >= m_stabilityThreshold) {
            return m_lastStable;
        }
    }

    if (!result.empty())
        m_lastTopPrediction = result.front();
    m_lastStable = result;
    return result;
}

// -------------------------
// Stage 12: confidence API
// -------------------------

float AACPredictionEngine::confidenceFor(const std::string& token) const
{
    if (token.empty())
        return 0.0f;

    auto it = m_unigram.find(token);
    if (it == m_unigram.end())
        return 0.0f;

    if (m_cachedTotalUnigrams < 0) {
        int total = 0;
        for (const auto& kv : m_unigram)
            total += kv.second;
        m_cachedTotalUnigrams = std::max(total, 1);
    }

    float freq = static_cast<float>(it->second) /
                 static_cast<float>(m_cachedTotalUnigrams);

    if (freq > 0.2f) freq = 0.2f;
    return freq / 0.2f;
}

// -------------------------
// Stage 13: positive reinforcement
// -------------------------

void AACPredictionEngine::reinforceChoice(const std::string& prev,
                                          const std::string& chosen)
{
    if (chosen.empty())
        return;

    m_unigram[chosen] += 3;
    m_cachedTotalUnigrams = -1;

    if (!prev.empty())
        m_bigram[prev][chosen] += 5;

    m_sessionBoost[chosen] += 1.0f;
    if (!prev.empty())
        m_sessionBigramBoost[prev][chosen] += 1.5f;

    ++m_recencyCounter;
}

// -------------------------
// Stage 14: negative reinforcement
// -------------------------

void AACPredictionEngine::applyPenalty(const std::string& token, float basePenalty)
{
    int inc = static_cast<int>(std::ceil(basePenalty));
    m_negativeCount[token] += inc;

    if (m_negativeCount[token] > 50)
        m_negativeCount[token] = 50;
}

void AACPredictionEngine::penalizeIgnored(const std::string& prev,
                                          const std::vector<std::string>& shown,
                                          const std::string& actualTyped)
{
    Q_UNUSED(prev);

    if (actualTyped.empty())
        return;

    bool actualWasShown =
        std::find(shown.begin(), shown.end(), actualTyped) != shown.end();

    if (!actualWasShown) {
        for (const auto& s : shown)
            applyPenalty(s, 1.0f);
    }
}

void AACPredictionEngine::onUserSelected(const std::string& prev,
                                         const std::string& chosen,
                                         const std::vector<std::string>& shown)
{
    bool wasShown =
        std::find(shown.begin(), shown.end(), chosen) != shown.end();

    if (wasShown) {
        m_positiveCount[chosen] += 1;

        m_sessionBoost[chosen] += 0.3f;
        if (!prev.empty())
            m_sessionBigramBoost[prev][chosen] += 0.3f;

        auto it = m_negativeCount.find(chosen);
        if (it != m_negativeCount.end() && it->second > 0)
            it->second -= 1;
    }
}

void AACPredictionEngine::onUserDeletedAutocompleted(const std::string& token)
{
    applyPenalty(token, 2.0f);
}

void AACPredictionEngine::onPredictionBarShown()
{
    m_predictionBarVisible = true;
    m_predictionBarShown = std::chrono::steady_clock::now();
}

void AACPredictionEngine::tick()
{
    if (!m_predictionBarVisible)
        return;

    auto now = std::chrono::steady_clock::now();
    auto elapsedMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_predictionBarShown).count();

    if (elapsedMs >= m_ignoreThresholdMs) {

        if (!m_lastTopPrediction.empty())
            applyPenalty(m_lastTopPrediction, 1.0f);

        for (auto& kv : m_negativeCount) {
            kv.second = static_cast<int>(
                std::floor(kv.second * m_penaltyCfg.decayFactor));
        }

        m_predictionBarVisible = false;
    }
}

// -------------------------
// Stage 20: dwell reinforcement
// -------------------------

void AACPredictionEngine::reinforceDwellChoice(const std::string& prev,
                                               const std::string& chosen)
{
    m_positiveCount[chosen] += 2;
    m_sessionBoost[chosen]  += 0.6f;

    if (!prev.empty())
        m_sessionBigramBoost[prev][chosen] += 0.6f;

    auto it = m_negativeCount.find(chosen);
    if (it != m_negativeCount.end() && it->second > 0) {
        it->second -= 2;
        if (it->second < 0)
            it->second = 0;
    }
}

// -------------------------
// Stage 18: AAC context hooks
// -------------------------

void AACPredictionEngine::setCurrentCategory(const QString& category)
{
    m_currentCategory = category;
}

void AACPredictionEngine::setLastSymbolWord(const QString& word)
{
    m_lastSymbolWord = word.trimmed().toLower().toStdString();
}

// -------------------------
// Stage 19: freeze control
// -------------------------

void AACPredictionEngine::freezePredictions()
{
    m_predictionsFrozen = true;
}

void AACPredictionEngine::unfreezePredictions()
{
    m_predictionsFrozen = false;
}

// -------------------------
// Stage 4: persistence
// -------------------------

bool AACPredictionEngine::saveToFile(const QString& path) const
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&f);

    // Unigrams
    out << "UNIGRAM\n";
    for (const auto& kv : m_unigram)
        out << QString::fromStdString(kv.first) << " " << kv.second << "\n";

    // Bigrams
    out << "BIGRAM\n";
    for (const auto& pkv : m_bigram) {
        QString prev = QString::fromStdString(pkv.first);
        for (const auto& nkv : pkv.second) {
            QString next = QString::fromStdString(nkv.first);
            out << prev << " " << next << " " << nkv.second << "\n";
        }
    }

    // Trigrams
    out << "TRIGRAM\n";
    for (const auto& w1kv : m_trigram) {
        QString w1 = QString::fromStdString(w1kv.first);
        for (const auto& w2kv : w1kv.second) {
            QString w2 = QString::fromStdString(w2kv.first);
            for (const auto& w3kv : w2kv.second) {
                QString w3 = QString::fromStdString(w3kv.first);
                out << w1 << " " << w2 << " " << w3 << " " << w3kv.second << "\n";
            }
        }
    }

    // Phrases
    out << "PHRASE\n";
    for (const auto& kv : m_phrases) {
        const auto& e = kv.second;
        out << QString::fromStdString(e.phrase)
            << " " << e.count
            << " " << e.recencyTick << "\n";
    }

    // Personal dictionary
    out << "CUSTOM\n";
    for (const auto& w : m_customWords) {
        int seen = 0;
        auto it = m_wordSeenCount.find(w);
        if (it != m_wordSeenCount.end())
            seen = it->second;
        out << QString::fromStdString(w) << " " << seen << "\n";
    }

    return true;
}

bool AACPredictionEngine::loadFromFile(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    m_unigram.clear();
    m_bigram.clear();
    m_trigram.clear();
    m_phrases.clear();
    m_customWords.clear();
    m_wordSeenCount.clear();
    m_recencyCounter = 0;
    m_cachedTotalUnigrams = -1;
    m_sessionBoost.clear();
    m_sessionBigramBoost.clear();
    m_lastStable.clear();
    m_predictionsFrozen = false;
    m_currentCategory.clear();
    m_lastSymbolWord.clear();
    m_negativeCount.clear();
    m_positiveCount.clear();
    m_lastTopPrediction.clear();

    QTextStream in(&f);
    QString section;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        if (line == "UNIGRAM" || line == "BIGRAM" || line == "TRIGRAM" ||
            line == "PHRASE" || line == "CUSTOM") {
            section = line;
            continue;
        }

        QStringList parts = line.split(' ', Qt::SkipEmptyParts);

        if (section == "UNIGRAM") {
            if (parts.size() != 2)
                continue;
            std::string word = parts[0].toLower().toStdString();
            int count = parts[1].toInt();
            m_unigram[word] = count;
        } else if (section == "BIGRAM") {
            if (parts.size() != 3)
                continue;
            std::string prev = parts[0].toLower().toStdString();
            std::string next = parts[1].toLower().toStdString();
            int count = parts[2].toInt();
            m_bigram[prev][next] = count;
        } else if (section == "TRIGRAM") {
            if (parts.size() != 4)
                continue;
            std::string w1 = parts[0].toLower().toStdString();
            std::string w2 = parts[1].toLower().toStdString();
            std::string w3 = parts[2].toLower().toStdString();
            int count = parts[3].toInt();
            m_trigram[w1][w2][w3] = count;
        } else if (section == "PHRASE") {
            if (parts.size() < 3)
                continue;
            bool okCount = false;
            bool okRec = false;
            int count = parts[parts.size() - 2].toInt(&okCount);
            int rec = parts[parts.size() - 1].toInt(&okRec);
            if (!okCount || !okRec)
                continue;

            QString phrase;
            for (int i = 0; i < parts.size() - 2; ++i) {
                if (i > 0) phrase += " ";
                phrase += parts[i];
            }

            std::string key = phrase.toLower().toStdString();
            PhraseEntry e;
            e.phrase = key;
            e.count = count;
            e.recencyTick = rec;
            m_phrases[key] = e;
            m_recencyCounter = std::max(m_recencyCounter, rec);
        } else if (section == "CUSTOM") {
            if (parts.size() != 2)
                continue;
            std::string word = parts[0].toLower().toStdString();
            int seen = parts[1].toInt();
            m_wordSeenCount[word] = seen;
            if (seen >= 2)
                m_customWords.insert(word);
        }
    }

    return true;
}
