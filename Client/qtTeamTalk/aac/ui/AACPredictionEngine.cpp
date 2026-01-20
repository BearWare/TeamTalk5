#include "AACPredictionEngine.h"
#include "AACFramework.h"

#include <QRegularExpression>
#include <QFile>
#include <QTextStream>

#include <algorithm>
#include <cctype>

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
// Stage 2 + 7: learning
// -------------------------

void AACPredictionEngine::learnUtterance(const QString& text)
{
    auto tokens = tokenize(text);
    if (tokens.empty())
        return;

    // Unigrams
    for (const auto& w : tokens)
        ++m_unigram[w];

    // Bigrams
    for (size_t i = 1; i < tokens.size(); ++i) {
        const std::string& prev = tokens[i - 1];
        const std::string& next = tokens[i];
        ++m_bigram[prev][next];
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
// Stage 2,3,7: prediction
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

    // Punctuation-aware context
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

    // 1) Bigram context
    if (!lastWord.empty()) {
        auto itBig = m_bigram.find(lastWord);
        if (itBig != m_bigram.end()) {
            std::vector<std::pair<std::string,int>> candidates(
                itBig->second.begin(), itBig->second.end());

            std::sort(candidates.begin(), candidates.end(),
                      [](auto& a, auto& b){ return a.second > b.second; });

            for (auto& c : candidates) {
                result.push_back(c.first);
                if ((int)result.size() >= maxSuggestions)
                    break;
            }
        }
    }

    // 2) Fill with top unigrams
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

    // Capitalize at sentence start
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

    // Phrases
    out << "PHRASE\n";
    for (const auto& kv : m_phrases) {
        const auto& e = kv.second;
        out << QString::fromStdString(e.phrase)
            << " " << e.count
            << " " << e.recencyTick << "\n";
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
    m_phrases.clear();
    m_recencyCounter = 0;

    QTextStream in(&f);
    QString section;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        if (line == "UNIGRAM" || line == "BIGRAM" || line == "PHRASE") {
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
        } else if (section == "PHRASE") {
            if (parts.size() < 3)
                continue;
            // phrase may contain spaces; reconstruct
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
        }
    }

    return true;
}
