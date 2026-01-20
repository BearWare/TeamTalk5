#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

class AACPredictionEngine
{
public:
    AACPredictionEngine();

    // Train on a piece of text (e.g. a sent message)
    void Train(const std::string& text);

    // Get up to topK predictions for the given context text
    std::vector<std::string> Predict(const std::string& text, int topK = 3);

    // Persistence
    bool Save(const std::string& path) const;
    bool Load(const std::string& path);

    // Domain weighting (e.g. TeamTalk vocab)
    void BoostToken(const std::string& token, int amount = 5);

private:
    std::vector<std::string> Tokenize(const std::string& text);

    // unigram: token → next-token → count
    std::unordered_map<std::string, std::map<std::string, int>> unigram_;

    // bigram: "t1\nt2" → next-token → count
    std::unordered_map<std::string, std::map<std::string, int>> bigram_;

    // trigram: "t1\nt2\nt3" → next-token → count
    std::unordered_map<std::string, std::map<std::string, int>> trigram_;

    const std::string punct_ = ".,!?";
};
