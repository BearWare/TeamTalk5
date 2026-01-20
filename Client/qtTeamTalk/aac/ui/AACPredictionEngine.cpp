#include "AACPredictionEngine.h"
#include <sstream>
#include <cctype>
#include <algorithm>

PredictiveTextEngine::PredictiveTextEngine()
{
}

std::vector<std::string> PredictiveTextEngine::Tokenize(const std::string& text)
{
    std::vector<std::string> tokens;
    std::string current;

    for (char c : text)
    {
        if (std::isspace(c))
        {
            if (!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }
        }
        else if (punct_.find(c) != std::string::npos)
        {
            if (!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }
            tokens.push_back(std::string(1, c));
        }
        else
        {
            current.push_back(std::tolower(c));
        }
    }

    if (!current.empty())
        tokens.push_back(current);

    return tokens;
}

void PredictiveTextEngine::Train(const std::string& text)
{
    auto tokens = Tokenize(text);
    if (tokens.size() < 2)
        return;

    // unigram
    for (size_t i = 0; i + 1 < tokens.size(); ++i)
        unigram_[tokens[i]][tokens[i + 1]]++;

    // bigram
    for (size_t i = 0; i + 2 < tokens.size(); ++i)
    {
        std::string key = tokens[i] + "\n" + tokens[i + 1];
        bigram_[key][tokens[i + 2]]++;
    }
}

std::vector<std::string> PredictiveTextEngine::Predict(const std::string& text, int topK)
{
    auto tokens = Tokenize(text);
    if (tokens.empty())
        return {};

    std::string last = tokens.back();
    std::string bigramKey;

    if (tokens.size() >= 2)
        bigramKey = tokens[tokens.size() - 2] + "\n" + tokens[tokens.size() - 1];

    std::vector<std::string> results;

    // 1. Try bigram
    if (!bigramKey.empty() && bigram_.count(bigramKey))
    {
        auto& m = bigram_[bigramKey];
        for (auto it = m.rbegin(); it != m.rend() && results.size() < (size_t)topK; ++it)
            results.push_back(it->first);

        if (!results.empty())
            return results;
    }

    // 2. Fallback to unigram
    if (unigram_.count(last))
    {
        auto& m = unigram_[last];
        for (auto it = m.rbegin(); it != m.rend() && results.size() < (size_t)topK; ++it)
            results.push_back(it->first);

        return results;
    }

    // 3. No prediction
    return {};
}
