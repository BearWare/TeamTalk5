#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

class PredictiveTextEngine
{
public:
    PredictiveTextEngine();

    void Train(const std::string& text);
    std::vector<std::string> Predict(const std::string& text, int topK = 3);

private:
    std::vector<std::string> Tokenize(const std::string& text);

    // unigram: token → next-token → count
    std::unordered_map<std::string, std::map<std::string, int>> unigram_;

    // bigram: (token1 + "\n" + token2) → next-token → count
    std::unordered_map<std::string, std::map<std::string, int>> bigram_;

    const std::string punct_ = ".,!?";
};
