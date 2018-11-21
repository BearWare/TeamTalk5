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

#include <myace/MyACE.h>

#if defined(ENABLE_SOUNDSYSTEM)
#include "SoundSystem.h"
#endif

#if defined(ENABLE_SPEEXDSP)
#include <avstream/SpeexPreprocess.h>
#endif

#include <avstream/AudioResampler.h>

#include <vector>
#include <queue>

class SoundLoopback
#if defined(ENABLE_SOUNDSYSTEM)
    : public soundsystem::StreamDuplex
    , public soundsystem::StreamCapture
    , public soundsystem::StreamPlayer
#endif
{
public:
    SoundLoopback();
    virtual ~SoundLoopback();

    bool StartTest(int inputdevid, int outputdevid,
                   int samplerate, int channels
#if defined(ENABLE_SPEEX)
                   , bool enable_agc, const SpeexAGC& agc,
                   bool denoise, int denoise_level,
                   bool enable_aec, const SpeexAEC& aec
#endif
                   );

    bool StartDuplexTest(int inputdevid, int outputdevid,
                         int samplerate, int channels
#if defined(ENABLE_SPEEX)
                         , bool enable_agc, const SpeexAGC& agc,
                         bool denoise, int denoise_level,
                         bool enable_aec, const SpeexAEC& aec
#endif
                         );
    bool StopTest();

#if defined(ENABLE_SOUNDSYSTEM)
    void StreamCaptureCb(const soundsystem::InputStreamer& streamer,
                         const short* buffer, int samples);
    bool StreamPlayerCb(const soundsystem::OutputStreamer& streamer, 
                        short* buffer, int samples);
    void StreamPlayerCbEnded();

    void StreamDuplexEchoCb(const soundsystem::DuplexStreamer& streamer,
                            const short* input_buffer, 
                            const short* prev_output_buffer, int samples);
    void StreamDuplexCb(const soundsystem::DuplexStreamer& streamer,
                        const short* input_buffer, 
                        short* output_buffer, int samples);
#endif

private:
#if defined(ENABLE_SPEEX)
    bool SetAGC(int samplerate, int samples, int channels,
                bool enable_agc,
                const SpeexAGC& agc,
                bool denoise, int denoise_level,
                bool enable_aec, 
                const SpeexAEC& aec);
#endif
    bool m_active;
    int m_soundgrpid;
#if defined(ENABLE_SPEEX)
    SpeexPreprocess m_preprocess_left, m_preprocess_right;
#endif
    std::vector<short> m_resample_buffer, m_preprocess_buffer_left, 
        m_preprocess_buffer_right;
    std::queue< std::vector<short> > m_buf_queue;
    audio_resampler_t m_capture_resampler;
    ACE_Recursive_Thread_Mutex m_mutex;
};

#endif
