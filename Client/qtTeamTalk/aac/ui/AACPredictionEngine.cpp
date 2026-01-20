#include "AACPredictionEngine.h"
#include <cctype>
#include <fstream>
#include <sstream>

AACPredictionEngine::AACPredictionEngine()
{
}

std::vector<std::string> AACPredictionEngine::Tokenize(const std::string& text)
{
    std::vector<std::string> tokens;
    std::string current;

    for (char c : text)
    {
        if (std::isspace(static_cast<unsigned char>(c)))
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
            current.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
    }

    if (!current.empty())
        tokens.push_back(current);

    return tokens;
}

void AACPredictionEngine::Train(const std::string& text)
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

    // trigram
    for (size_t i = 0; i + 3 < tokens.size(); ++i)
    {
        std::string key = tokens[i] + "\n" + tokens[i + 1] + "\n" + tokens[i + 2];
        trigram_[key][tokens[i + 3]]++;
    }
}

std::vector<std::string> AACPredictionEngine::Predict(const std::string& text, int topK)
{
    auto tokens = Tokenize(text);
    std::vector<std::string> results;
    if (tokens.empty() || topK <= 0)
        return results;

    // ---- trigram backoff ----
    if (tokens.size() >= 3)
    {
        std::string triKey = tokens[tokens.size() - 3] + "\n" +
                             tokens[tokens.size() - 2] + "\n" +
                             tokens[tokens.size() - 1];

        auto itTri = trigram_.find(triKey);
        if (itTri != trigram_.end())
        {
            auto& m = itTri->second;
            for (auto it = m.rbegin(); it != m.rend() && results.size() < static_cast<size_t>(topK); ++it)
                results.push_back(it->first);
        }
    }

    if (results.size() >= static_cast<size_t>(topK))
        return results;

    // ---- bigram backoff ----
    if (tokens.size() >= 2)
    {
        std::string biKey = tokens[tokens.size() - 2] + "\n" + tokens[tokens.size() - 1];
        auto itBi = bigram_.find(biKey);
        if (itBi != bigram_.end())
        {
            auto& m = itBi->second;
            for (auto it = m.rbegin(); it != m.rend() && results.size() < static_cast<size_t>(topK); ++it)
                results.push_back(it->first);
        }
    }

    if (results.size() >= static_cast<size_t>(topK))
        return results;

    // ---- unigram backoff ----
    std::string last = tokens.back();
    auto itUni = unigram_.find(last);
    if (itUni != unigram_.end())
    {
        auto& m = itUni->second;
        for (auto it = m.rbegin(); it != m.rend() && results.size() < static_cast<size_t>(topK); ++it)
            results.push_back(it->first);
    }

    return results;
}

bool AACPredictionEngine::Save(const std::string& path) const
{
    std::ofstream out(path);
    if (!out)
        return false;

    out << "UNIGRAM\n";
    for (const auto& u : unigram_)
        for (const auto& n : u.second)
            out << u.first << '\t' << n.first << '\t' << n.second << '\n';

    out << "BIGRAM\n";
    for (const auto& b : bigram_)
        for (const auto& n : b.second)
            out << b.first << '\t' << n.first << '\t' << n.second << '\n';

    out << "TRIGRAM\n";
    for (const auto& t : trigram_)
        for (const auto& n : t.second)
            out << t.first << '\t' << n.first << '\t' << n.second << '\n';

    return true;
}

bool AACPredictionEngine::Load(const std::string& path)
{
    std::ifstream in(path);
    if (!in)
        return false;

    unigram_.clear();
    bigram_.clear();
    trigram_.clear();

    std::string section;
    if (!std::getline(in, section))
        return false;

    std::string line;
    while (std::getline(in, line))
    {
        if (line == "BIGRAM" || line == "TRIGRAM")
        {
            section = line;
            continue;
        }

        std::istringstream iss(line);
        std::string key, next;
        int count = 0;
        if (!(iss >> key >> next >> count))
            continue;

        if (section == "UNIGRAM")
            unigram_[key][next] += count;
        else if (section == "BIGRAM")
            bigram_[key][next] += count;
        else if (section == "TRIGRAM")
            trigram_[key][next] += count;
    }

    return true;
}

void AACPredictionEngine::BoostToken(const std::string& token, int amount)
{
    if (amount <= 0)
        return;

    for (auto& u : unigram_)
        u.second[token] += amount;
}
