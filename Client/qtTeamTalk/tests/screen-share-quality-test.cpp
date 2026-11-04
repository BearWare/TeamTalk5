// SPDX-License-Identifier: GPL-3.0-or-later
#include "screensharequality.h"
#include <iostream>

int main()
{
    int failures = 0;
    const auto check = [&failures](bool success, const char* label)
    {
        std::cout << (success ? "PASS: " : "FAIL: ") << label << '\n';
        failures += !success;
    };
    AudioCodec codec = {};
    codec.nCodec = OPUS_CODEC;
    codec.opus.nTxIntervalMSec = 20;
    check(screenShareAudioBitrate(codec, 128000) == 128000, "Low bitrate");
    check(screenShareAudioBitrate(codec, 320000) == 320000, "Medium bitrate");
    check(screenShareAudioBitrate(codec, 510000) == 510000, "High bitrate");
    codec.opus.nTxIntervalMSec = 160;
    check(screenShareAudioBitrate(codec, 510000) == 204744, "packet byte budget is rounded before conversion to bits");
    codec.opus.nTxIntervalMSec = 0;
    check(screenShareAudioBitrate(codec, 510000) == 0, "invalid packet interval");
    codec.nCodec = SPEEX_CODEC;
    check(screenShareAudioBitrate(codec, 510000) == 0, "no Opus override for Speex");
    return failures ? 1 : 0;
}
