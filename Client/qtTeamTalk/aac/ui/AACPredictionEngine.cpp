#include "AACPredictionEngine.h"
#include "AACFramework.h"

#include <QRegularExpression>
#include <QFile>
#include <QTextStream>

#include <algorithm>
#include <cctype>
#include <limits>

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
        int& seen = m_wordSeenCount[w];
        ++seen;
        // Stage 10 (Option B): add to custom dictionary on second occurrence
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
    learnPhrase(text);
}

void AACPredictionEngine::learnPhrase(const QString& text)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty())
        return;

    std::string key = trimmed.toLower().toStdString();
    auto& entry = m_phrases[key];
    entry.phrase = key;
    entry.count += 1;
    entry.recencyTick = ++m_recencyCounter;
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
}

// -------------------------
// Stage 11: fuzzy matching
// -------------------------

int AACPredictionEngine::fuzzyDistance(const std::string& a,
                                       const std::string& b) const
{
    // Simple mismatch + length difference heuristic
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
// Stage 2,3,7,8,9,10,11: prediction
// -------------------------

std::vector<std::string> AACPredictionEngine::Predict(const std::string& prefix,
                                                      int maxSuggestions) const
{
    std::vector<std::string> result;
    if (maxSuggestions <= 0)
        return result;

    // Stage 7: phrase-level suggestions first
    auto phraseRes = phraseSuggestions(prefix, maxSuggestions);
    for (auto& p : phraseRes)
        result.push_back(p);
    if ((int)result.size() >= maxSuggestions)
        return result;

    if (prefix.empty()) {
        // No context: top unigrams (with recency implicitly baked into counts)
        std::vector<std::pair<std::string,int>> uni(m_unigram.begin(), m_unigram.end());
        std::sort(uni.begin(), uni.end(),
                  [](auto& a, auto& b){ return a.second > b.second; });

        for (auto& u : uni) {
            result.push_back(u.first);
            if ((int)result.size() >= maxSuggestions)
                break;
        }
        return result;
    }

    // Punctuation-aware context (Stage 3 + 9)
    QString qPrefix = QString::fromStdString(prefix);
    QString cleaned = normalizePunctuation(qPrefix);

    QStringList parts = cleaned.split(QRegularExpression("\\s+"),
                                      Qt::SkipEmptyParts);

    bool sentenceStart = false;
    std::string lastWord;

    if (!parts.isEmpty()) {
        QString last = parts.last();
        if (last == "." || last == "!" || last == "?") {
            sentenceStart = true;
        } else {
            lastWord = last.toLower().toStdString();
        }
    } else {
        sentenceStart = true;
    }

    // Stage 8: Trigram context (w1, w2 -> w3)
    if (parts.size() >= 3) {
        std::string w1 = parts[parts.size() - 3].toLower().toStdString();
        std::string w2 = parts[parts.size() - 2].toLower().toStdString();

        auto it1 = m_trigram.find(w1);
        if (it1 != m_trigram.end()) {
            auto it2 = it1->second.find(w2);
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

        if ((int)result.size() >= maxSuggestions)
            return result;
    }

    // 1) Bigram context (Stage 2)
    if (!lastWord.empty()) {
        auto itBig = m_bigram.find(lastWord);
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
    if (!lastWord.empty() && (int)result.size() < maxSuggestions) {
        for (const auto& w : m_customWords) {
            if (fuzzyCloseEnough(lastWord, w)) {
                if (std::find(result.begin(), result.end(), w) == result.end()) {
                    result.push_back(w);
                    if ((int)result.size() >= maxSuggestions)
                        break;
                }
            }
        }
    }

    // 3) Fill with top unigrams (Stage 2,12)
    if ((int)result.size() < maxSuggestions) {
        std::vector<std::pair<std::string,int>> uni(m_unigram.begin(), m_unigram.end());
        std::sort(uni.begin(), uni.end(),
                  [](auto& a, auto& b){ return a.second > b.second; });

        for (auto& u : uni) {
            if (std::find(result.begin(), result.end(), u.first) == result.end()) {
                result.push_back(u.first);
                if ((int)result.size() >= maxSuggestions)
                    break;
            }
        }
    }

    // Stage 9: Capitalize at sentence start
    if (sentenceStart) {
        for (auto& s : result) {
            if (!s.empty())
                s[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));
        }
    }

    return result;
}

// Stage 7: phrase suggestions (recency + frequency)
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
            int score = entry.count * 10 + entry.recencyTick; // simple freq+recency
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

    // Trigrams (Stage 8)
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

    // Personal dictionary (Stage 10)
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
