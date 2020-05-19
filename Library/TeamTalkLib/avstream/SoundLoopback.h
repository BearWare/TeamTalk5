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

#if !defined(SOUNDLOOPBACK_H)
#define SOUNDLOOPBACK_H

#include "SoundSystem.h"

#if defined(ENABLE_SPEEXDSP)
#include <avstream/SpeexPreprocess.h>
#endif
#include <avstream/AudioResampler.h>
#include <codec/MediaUtil.h>

#include <myace/MyACE.h>

#include <vector>
#include <queue>
#include <mutex>

class SoundLoopback
    : public soundsystem::StreamDuplex
    , public soundsystem::StreamCapture
    , public soundsystem::StreamPlayer
{
public:
    SoundLoopback();
    virtual ~SoundLoopback();

    bool StartTest(int inputdevid, int outputdevid,
                   int samplerate, int channels
#if defined(ENABLE_SPEEXDSP)
                   , bool enable_agc, const SpeexAGC& agc,
                   bool denoise, int denoise_level,
                   bool enable_aec, const SpeexAEC& aec
#endif
                   , int gainlevel, StereoMask stereo,
                   soundsystem::SoundDeviceFeatures sndfeatures);

    bool StartDuplexTest(int inputdevid, int outputdevid,
                         int samplerate, int channels
#if defined(ENABLE_SPEEXDSP)
                         , bool enable_agc, const SpeexAGC& agc,
                         bool denoise, int denoise_level,
                         bool enable_aec, const SpeexAEC& aec
#endif
                         , int gainlevel, StereoMask stereo,
                         soundsystem::SoundDeviceFeatures sndfeatures);
    bool StopTest();

    void StreamCaptureCb(const soundsystem::InputStreamer& streamer,
                         const short* buffer, int samples);
    bool StreamPlayerCb(const soundsystem::OutputStreamer& streamer, 
                        short* buffer, int samples);

    void StreamDuplexEchoCb(const soundsystem::DuplexStreamer& streamer,
                            const short* input_buffer, 
                            const short* prev_output_buffer, int samples);
    void StreamDuplexCb(const soundsystem::DuplexStreamer& streamer,
                        const short* input_buffer, 
                        short* output_buffer, int samples);

    soundsystem::SoundDeviceFeatures GetCaptureFeatures();
    soundsystem::SoundDeviceFeatures GetDuplexFeatures();
    
private:
#if defined(ENABLE_SPEEXDSP)
    bool SetAGC(int samplerate, int samples, int channels,
                bool enable_agc,
                const SpeexAGC& agc,
                bool denoise, int denoise_level,
                bool enable_aec, 
                const SpeexAEC& aec);
#endif
    bool m_active;
    soundsystem::soundsystem_t m_soundsystem;
    int m_soundgrpid;
    int m_gainlevel = GAIN_NORMAL;
    StereoMask m_stereo = STEREO_BOTH;
    soundsystem::SoundDeviceFeatures m_features = soundsystem::SOUNDDEVICEFEATURE_NONE;
#if defined(ENABLE_SPEEXDSP)
    SpeexPreprocess m_preprocess_left, m_preprocess_right;
#endif
    std::vector<short> m_preprocess_buffer_left, m_preprocess_buffer_right;
    std::queue< std::vector<short> > m_buf_queue;
    audio_resampler_t m_capture_resampler;
    std::mutex m_mutex;
};

#endif
