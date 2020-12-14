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
#if defined(UNICODE)
    ACE_TString str = LocalToUnicode(trial.c_str());
#else
    ACE_TString str = trial.c_str();
#endif
    MYTRACE(ACE_TEXT("Querying feature: %s\n"), str.c_str());
    return "Disabled";
    //return "Enabled";
}
} }

bool IsEnabled(const webrtc::AudioProcessing::Config& cfg)
{
    return cfg.gain_controller1.enabled ||
        cfg.gain_controller2.enabled ||
        cfg.noise_suppression.enabled ||
        cfg.echo_canceller.enabled;
}

int WebRTCPreprocess(webrtc::AudioProcessing& apm, const media::AudioFrame& infrm,
                     media::AudioFrame& outfrm)
{
    assert(!outfrm.inputfmt.IsValid() || infrm.inputfmt == outfrm.inputfmt);

    webrtc::StreamConfig in_cfg(infrm.inputfmt.samplerate, infrm.inputfmt.channels),
        out_cfg(infrm.inputfmt.samplerate, infrm.inputfmt.channels),
        echo_cfg(infrm.outputfmt.samplerate, infrm.outputfmt.channels);

    bool echo = apm.GetConfig().echo_canceller.enabled;
    assert(!echo || infrm.outputfmt.IsValid());
    assert(!echo || infrm.inputfmt == infrm.outputfmt);

    if (echo)
    {
        if (apm.stream_delay_ms() == 0)
        {
            int ms = PCM16_SAMPLES_DURATION(infrm.output_samples, infrm.outputfmt.samplerate);
            int ret = apm.set_stream_delay_ms(ms);
            MYTRACE_COND(ret != webrtc::AudioProcessing::kNoError,
                         ACE_TEXT("WebRTC failed to stream delay for echo cancellation. Result: %d\n"), ret);
            if (ret != webrtc::AudioProcessing::kNoError)
                return -1;
        }
    }

    int in_index = 0, out_index = 0;
    while (in_index + in_cfg.num_frames() <= infrm.input_samples)
    {
        int ret;

        if (echo)
        {
            ret = apm.ProcessReverseStream(&infrm.output_buffer[in_index * echo_cfg.num_channels()],
                                           echo_cfg, echo_cfg,
                                           const_cast<int16_t*>(&infrm.output_buffer[in_index * echo_cfg.num_channels()]));
            MYTRACE_COND(ret != webrtc::AudioProcessing::kNoError,
                         ACE_TEXT("WebRTC failed to process reverse audio frame. Result: %d\n"), ret);
        }

        ret = apm.ProcessStream(&infrm.input_buffer[in_index * in_cfg.num_channels()],
                                in_cfg, out_cfg, &outfrm.input_buffer[out_index * out_cfg.num_channels()]);
        MYTRACE_COND(ret != webrtc::AudioProcessing::kNoError,
                     ACE_TEXT("WebRTC failed to process audio frame. Result: %d\n"), ret);

        //MYTRACE(ACE_TEXT("Gain1=%d, gain %d. Gain2=%d, gain: %g\n"),
        //        int(apm.GetConfig().gain_controller1.enabled),
        //        apm.GetConfig().gain_controller1.target_level_dbfs,
        //        int(apm.GetConfig().gain_controller2.enabled),
        //        apm.GetConfig().gain_controller2.fixed_digital.gain_db);
        assert(ret == webrtc::AudioProcessing::kNoError);
        if (ret != webrtc::AudioProcessing::kNoError)
            return -1;

        in_index += in_cfg.num_frames();
        out_index += out_cfg.num_frames();
    }

    return infrm.input_samples;
}
