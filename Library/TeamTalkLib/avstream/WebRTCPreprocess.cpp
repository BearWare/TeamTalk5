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

#include "WebRTCPreprocess.h"

#include <myace/MyACE.h>

// webrtc::GainControlImpl queries this feature. Field trials is
// excluded by passing rtc_exclude_field_trial_default=true to GN.
namespace webrtc { namespace field_trial {
std::string FindFullName(const std::string& trial)
{
    MYTRACE(ACE_TEXT("Querying feature: %s\n"), trial.c_str());
    return "Disabled";
}
} }


WebRTCPreprocess::WebRTCPreprocess()
{
}

WebRTCPreprocess::~WebRTCPreprocess()
{
}

bool IsEnabled(const webrtc::AudioProcessing::Config& cfg)
{
    return cfg.gain_controller1.enabled ||
        cfg.result.gain_controller2.enabled ||
        cfg.noise_suppression.enabled;
}
