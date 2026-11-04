/*
 * Copyright (c) 2005-2018, BearWare.dk
 *
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#include "AudioAnalyze.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cmath>

std::vector<float> LoadAudioAsFloat(WavePCMFile& wavefile)
{
    if (wavefile.GetChannels() <= 0)
        return {};

    constexpr auto BUFSIZE = 1024;
    auto n_samples = 0;
    std::array<short, BUFSIZE> buff;
    std::vector<float> result;
    while ((n_samples = wavefile.ReadSamples(buff.data(), buff.size()/wavefile.GetChannels())) > 0)
    {
        std::transform(buff.begin(), buff.begin() + (n_samples * wavefile.GetChannels()), std::back_inserter(result), [](short v) { return v; });
    }

    return result;
}

void NormalizeAudio(std::vector<float>& samples, size_t channels)
{
    std::vector<float> channel_peak(channels, std::numeric_limits<float>::min());
    size_t index = 0;
    for (auto& v : samples) {
        auto channel = index++ % channels;
        channel_peak[channel] = std::max(channel_peak[channel], std::fabs(v));
    }
    index = 0;
    for (float& v : samples)
    {
        auto channel = index++ % channels;
        v /= channel_peak[channel];
    }
}

double CompareAudioRMSE(std::vector<float>& samplesA, std::vector<float>& samplesB, size_t channel, size_t channels)
{
    std::vector<double> sum(channels, 0.0);
    REQUIRE(samplesA.size() == samplesB.size());
    for (size_t i=0; i < samplesA.size()*channels; ++i)
    {
        double d = samplesA[i] - samplesB[i];
        sum[i%channels] += d * d;
    }
    return std::sqrt(sum[channel] / samplesA.size() / channels);
}
