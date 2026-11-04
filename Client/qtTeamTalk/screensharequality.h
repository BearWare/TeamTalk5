// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSHAREQUALITY_H
#define SCREENSHAREQUALITY_H

#include "utiltt.h"

// The media encoder uses the channel codec; presets cannot add sample rate
// or stereo channels which are absent from that codec.
inline int screenShareAudioBitrate(const AudioCodec& codec, int requested)
{
    if (codec.nCodec != OPUS_CODEC || codec.opus.nTxIntervalMSec <= 0)
        return 0;
    // Match GetAudioCodecMaxPacketBitrate() exactly: round the byte budget
    // before converting to bits. Rounding afterward can exceed the SDK cap.
    const int packetLimit = 8 * ((0xFFF * 1000) / codec.opus.nTxIntervalMSec);
    if (packetLimit < OPUS_MIN_BITRATE)
        return 0;
    return qBound(OPUS_MIN_BITRATE, qMin(requested, packetLimit), OPUS_MAX_BITRATE);
}

#endif