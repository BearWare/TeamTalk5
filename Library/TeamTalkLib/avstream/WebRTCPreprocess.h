/*
 * Copyright (c) 2005-2020, BearWare.dk
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

#if !defined(WEBRTCPREPROCESS_H)
#define WEBRTCPREPROCESS_H

#include <mystd/MyStd.h>

#include <audio_processing.h>

#include <codec/MediaUtil.h>

bool IsEnabled(const webrtc::AudioProcessing::Config& cfg);

int WebRTCPreprocess(webrtc::AudioProcessing& apm, const media::AudioFrame& infrm,
                     media::AudioFrame& outfrm);
#endif
