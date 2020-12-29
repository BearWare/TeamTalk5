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

#include "SoundLoopback.h"
#include <codec/MediaUtil.h>
#include <assert.h>
#include <cstring>

using namespace std;
using namespace soundsystem;

#define CALLBACK_FRAMESIZE(samplerate) (int)(samplerate * 0.04)

SoundLoopback::SoundLoopback()
    : m_active(false)
{
    m_soundsystem = soundsystem::GetInstance();
    
    m_soundgrpid = m_soundsystem->OpenSoundGroup();
}

SoundLoopback::~SoundLoopback()
{
    if(m_active)
        StopTest();
    m_soundsystem->RemoveSoundGroup(m_soundgrpid);
}

bool SoundLoopback::StartTest(int inputdevid, int outputdevid, 
                              int samplerate, int channels
#if defined(ENABLE_SPEEXDSP)
                              , bool enable_agc, const SpeexAGC& agc,
                              bool denoise, int denoise_level,
                              bool enable_aec, const SpeexAEC& aec
#endif
                              , int gainlevel, StereoMask stereo,
                              soundsystem::SoundDeviceFeatures sndfeatures)
{
    assert(!m_active);
    if(m_active)
        return false;

#if defined(ENABLE_SPEEXDSP)
    // Echo cancel requires duplex mode
    if(enable_aec)
        return false;
#endif

    DeviceInfo in_dev, out_dev;
    if(!m_soundsystem->GetDevice(inputdevid, in_dev) ||
       !m_soundsystem->GetDevice(outputdevid, out_dev) ||
       in_dev.default_samplerate == 0 ||
       out_dev.default_samplerate == 0)
       return false;
    
    int output_channels = channels;
    int output_samplerate = samplerate;
    if (!out_dev.SupportsOutputFormat(channels, samplerate))
    {
        output_channels = out_dev.GetSupportedOutputChannels(channels);
        output_samplerate = out_dev.default_samplerate;
    }

    int output_samples = CALLBACK_FRAMESIZE(output_samplerate);

    //adapt capture device to playback device
    int input_channels = output_channels;
    int input_samplerate = output_samplerate;
    int input_samples = output_samples;

    if (!in_dev.SupportsInputFormat(output_channels, output_samplerate))
    {
        input_channels = in_dev.GetSupportedInputChannels(channels);
        input_samplerate = in_dev.default_samplerate;

        input_samples = CalcSamples(output_samplerate, output_samples,
                                    input_samplerate);
        media::AudioFormat infmt(input_samplerate, input_channels),
            outfmt(output_samplerate, output_channels);
        m_capture_resampler = MakeAudioResampler(infmt, outfmt, input_samples);
        if (!m_capture_resampler)
            return false;
    }

    m_preprocess_buffer_left.resize(output_samples);
    if(channels == 2)
        m_preprocess_buffer_right.resize(output_samples);

#if defined(ENABLE_SPEEXDSP)
    if(!SetAGC(samplerate, output_samples, channels, 
               enable_agc, agc, denoise, denoise_level, enable_aec, aec))
    {
        StopTest();
        return false;
    }
#endif

    m_features = sndfeatures;
    m_gainlevel = gainlevel;
    m_stereo = stereo;

    if(!m_soundsystem->OpenOutputStream(this, outputdevid, m_soundgrpid,
                                      output_samplerate, output_channels,
                                      output_samples))
    {
        StopTest();
        return false;
    }

    if(!m_soundsystem->StartStream(this))
    {
        StopTest();
        return false;
    }

    if(!m_soundsystem->OpenInputStream(this, inputdevid, m_soundgrpid, 
                                     input_samplerate, input_channels,
                                     input_samples))
    {
        StopTest();
        return false;
    }

    m_active = true;
    return m_active;
}

bool SoundLoopback::StartDuplexTest(int inputdevid, int outputdevid,
                                    int samplerate, int channels
#if defined(ENABLE_SPEEXDSP)
                                    , bool enable_agc, const SpeexAGC& agc,
                                    bool denoise, int denoise_level,
                                    bool enable_aec, const SpeexAEC& aec
#endif
                                    , int gainlevel, StereoMask stereo,
                                    soundsystem::SoundDeviceFeatures sndfeatures)
{
    DeviceInfo in_dev, out_dev;
    if (!m_soundsystem->GetDevice(outputdevid, out_dev) ||
        !m_soundsystem->GetDevice(inputdevid, in_dev))
       return false;

    // For WASAPI we want to force output device's sample rate
    if (!out_dev.SupportsOutputFormat(channels, samplerate))
        return false;

    int input_channels = in_dev.GetSupportedInputChannels(channels);
    int samples = CALLBACK_FRAMESIZE(samplerate);

    if (input_channels != channels)
    {
        media::AudioFormat infmt(samplerate, input_channels),
            outfmt(samplerate, channels);
        m_capture_resampler = MakeAudioResampler(infmt, outfmt, samples);
        if (!m_capture_resampler)
            return false;
    }
    
    
    m_preprocess_buffer_left.resize(samples);
    if(channels == 2)
        m_preprocess_buffer_right.resize(samples);

#if defined(ENABLE_SPEEXDSP)
    if(!SetAGC(samplerate, samples, channels, enable_agc, agc, 
              denoise, denoise_level, enable_aec, aec))
    {
        StopTest();
        return false;
    }
#endif

    m_features = sndfeatures;
    m_gainlevel = gainlevel;
    m_stereo = stereo;

    if(!m_soundsystem->OpenDuplexStream(this, inputdevid, outputdevid,
                                        m_soundgrpid, samplerate, 
                                        input_channels, channels, samples))
    {
        StopTest();
        return false;
    }
    return true;
}


bool SoundLoopback::StopTest()
{
    bool b = m_soundsystem->CloseDuplexStream(this);
    b |= m_soundsystem->CloseInputStream(this);
    b |= m_soundsystem->CloseOutputStream(this);

#if defined(ENABLE_SPEEXDSP)
    m_preprocess_left.Close();
    m_preprocess_right.Close();
#endif
    m_preprocess_buffer_left.clear();
    m_preprocess_buffer_right.clear();
    m_capture_resampler.reset();
    while(m_buf_queue.size())
        m_buf_queue.pop();
    m_active = false;
    m_features = soundsystem::SOUNDDEVICEFEATURE_NONE;
    return b;
}

void SoundLoopback::StreamCaptureCb(const soundsystem::InputStreamer& streamer,
                                    const short* buffer, int samples)
{
    int output_samples = int(m_preprocess_buffer_left.size());
    int output_channels = m_preprocess_buffer_right.size()? 2 : 1;

    // if resampler is active then we first need to convert input
    // stream to output stream format
    if (m_capture_resampler)
    {
        assert(output_samples > 0);

        int ret = 0;
        const short* resampled = m_capture_resampler->Resample(buffer, &ret);
        assert(resampled);
        MYTRACE_COND(ret != output_samples,
                     ACE_TEXT("Resampler output incorrect no. samples, expect %d, got %d\n"),
                     output_samples, ret);

        if (output_channels == 1)
        {
            m_preprocess_buffer_left.assign(resampled, resampled + output_samples);
        }
        else if(output_channels == 2)
        {
            SplitStereo(resampled, output_samples, 
                        m_preprocess_buffer_left, m_preprocess_buffer_right);
        }
    }
    else
    {
        assert(output_samples == samples);
        if(output_channels == 1)
        {
            m_preprocess_buffer_left.assign(buffer, buffer + samples);
        }
        else if(output_channels == 2)
        {
            assert((int)m_preprocess_buffer_right.size() == samples);
            SplitStereo(buffer, samples, 
                        m_preprocess_buffer_left,
                        m_preprocess_buffer_right);
        }
    }

#if defined(ENABLE_SPEEXDSP)
    m_preprocess_left.Preprocess(&m_preprocess_buffer_left[0]);
    if(output_channels == 2)
        m_preprocess_right.Preprocess(&m_preprocess_buffer_right[0]);
#endif
    
    if(output_channels == 1)
    {
        // TTAudioPreprocessor
        if (m_gainlevel != GAIN_NORMAL)
            SOFTGAIN(&m_preprocess_buffer_left[0], output_samples,
                     output_channels, m_gainlevel, GAIN_NORMAL);

        std::lock_guard<std::mutex> g(m_mutex);
        m_buf_queue.push(m_preprocess_buffer_left);
    }
    else if(output_channels == 2)
    {
        vector<short> tmp_buf(m_preprocess_buffer_left.size() * output_channels);
        MergeStereo(m_preprocess_buffer_left, m_preprocess_buffer_right,
                    &tmp_buf[0], output_samples);

        // TTAudioPreprocessor
        if (m_gainlevel != GAIN_NORMAL)
            SOFTGAIN(&tmp_buf[0], output_samples, output_channels,
                     m_gainlevel, GAIN_NORMAL);
        SelectStereo(m_stereo, &tmp_buf[0], output_samples);
        
        std::lock_guard<std::mutex> g(m_mutex);
        m_buf_queue.push(tmp_buf);
    }
}

bool SoundLoopback::StreamPlayerCb(const soundsystem::OutputStreamer& streamer, 
                                   short* buffer, int samples)
{
    std::lock_guard<std::mutex> g(m_mutex);
    int output_channels = m_preprocess_buffer_right.size()? 2 : 1;
    if(m_buf_queue.size())
    {
        assert((int)m_buf_queue.front().size() / output_channels == samples);
        std::memcpy(buffer, &m_buf_queue.front()[0], m_buf_queue.front().size()*sizeof(short));
        m_buf_queue.pop();
    }
    else
    {
        std::memset(buffer, 0, samples * sizeof(short) * output_channels);
    }

    //don't empty queue since OpenSLES may perform multiple play
    //callbacks without any capture callbacks

    // while(m_buf_queue.size()>1)
    //     m_buf_queue.pop();

    return true;
}

void SoundLoopback::StreamDuplexEchoCb(const soundsystem::DuplexStreamer& streamer,
                                       const short* input_buffer, 
                                       const short* prev_output_buffer, int samples)
{
    int output_samples = int(m_preprocess_buffer_left.size());
    int output_channels = m_preprocess_buffer_right.size()? 2 : 1;

    const short* tmp_input_buffer = input_buffer;
    if (m_capture_resampler)
    {
        int ret = 0;
        tmp_input_buffer = m_capture_resampler->Resample(input_buffer, &ret);
        assert(tmp_input_buffer);
        assert(ret <= output_samples);
        MYTRACE_COND(ret != output_samples,
                     ACE_TEXT("Resampler output incorrect no. samples, expect %d, got %d\n"),
                     output_samples, ret);
    }

    if(output_channels == 1)
    {
        assert((int)m_preprocess_buffer_left.size() == streamer.framesize);

#if defined(ENABLE_SPEEXDSP)
        if(m_preprocess_left.IsEchoCancel())
        {
            m_preprocess_left.EchoCancel(tmp_input_buffer, prev_output_buffer, 
                                         &m_preprocess_buffer_left[0]);
        }
        else
#endif
        {
            m_preprocess_buffer_left.assign(tmp_input_buffer, tmp_input_buffer + streamer.framesize);
        }
    }
    else if(output_channels == 2)
    {
#if defined(ENABLE_SPEEXDSP)
        if(m_preprocess_left.IsEchoCancel() && m_preprocess_right.IsEchoCancel())
        {
            vector<short> in_leftchan(output_samples), in_rightchan(output_samples);
            SplitStereo(tmp_input_buffer, output_samples, in_leftchan, in_rightchan);

            vector<short> out_leftchan(output_samples), out_rightchan(output_samples);
            if(streamer.output_channels == 1)
            {
                out_leftchan.assign(prev_output_buffer, prev_output_buffer+output_samples);
                out_rightchan.assign(prev_output_buffer, prev_output_buffer+output_samples);
            }
            else
            {
                SplitStereo(prev_output_buffer, streamer.framesize, out_leftchan, out_rightchan);
            }
            m_preprocess_left.EchoCancel(&in_leftchan[0], &out_leftchan[0], 
                                         &m_preprocess_buffer_left[0]);
            m_preprocess_right.EchoCancel(&in_rightchan[0], &out_rightchan[0], 
                                          &m_preprocess_buffer_right[0]);
        }
        else
#endif
        {
            SplitStereo(tmp_input_buffer, output_samples, m_preprocess_buffer_left,
                        m_preprocess_buffer_right);
        }
    }
}

void SoundLoopback::StreamDuplexCb(const soundsystem::DuplexStreamer& streamer,
                                   const short* input_buffer, 
                                   short* output_buffer, int samples)
{
    int output_samples = int(m_preprocess_buffer_left.size());
    int output_channels = m_preprocess_buffer_right.size()? 2 : 1;

    if(output_channels == 1)
    {
        assert((int)m_preprocess_buffer_left.size() == streamer.framesize);

#if defined(ENABLE_SPEEXDSP)
        m_preprocess_left.Preprocess(&m_preprocess_buffer_left[0]);
#endif
        // TTAudioPreprocessor
        if (m_gainlevel != GAIN_NORMAL)
            SOFTGAIN(&m_preprocess_buffer_left[0], output_samples,
                     output_channels, m_gainlevel, GAIN_NORMAL);
        
        std::memcpy(output_buffer, &m_preprocess_buffer_left[0], 
                    streamer.framesize * streamer.input_channels * sizeof(short));
    }
    else if(output_channels == 2)
    {
        assert((int)m_preprocess_buffer_left.size() == streamer.framesize);
        assert((int)m_preprocess_buffer_right.size() == streamer.framesize);

#if defined(ENABLE_SPEEXDSP)
        m_preprocess_left.Preprocess(&m_preprocess_buffer_left[0]);
        m_preprocess_right.Preprocess(&m_preprocess_buffer_right[0]);
#endif

        MergeStereo(m_preprocess_buffer_left, m_preprocess_buffer_right,
                    output_buffer, streamer.framesize);

        // TTAudioPreprocessor
        if (m_gainlevel != GAIN_NORMAL)
            SOFTGAIN(output_buffer, output_samples, output_channels,
                     m_gainlevel, GAIN_NORMAL);
        SelectStereo(m_stereo, output_buffer, output_samples);        
    }
}

SoundDeviceFeatures SoundLoopback::GetCaptureFeatures()
{
    return m_features;
}

SoundDeviceFeatures SoundLoopback::GetDuplexFeatures()
{
    return GetCaptureFeatures();
}

#if defined(ENABLE_SPEEXDSP)
bool SoundLoopback::SetAGC(int samplerate, int samples, int channels,
                           bool enable_agc,
                           const SpeexAGC& agc,
                           bool denoise, int denoise_level,
                           bool enable_aec,
                           const SpeexAEC& aec)
{
    bool init = m_preprocess_left.Initialize(samplerate, samples);
    if(channels == 2)
        init &= m_preprocess_right.Initialize(samplerate, samples);
    assert(init);
    
    bool initagc = m_preprocess_left.EnableAGC(enable_agc);
    if(channels == 2)
        initagc &= m_preprocess_right.EnableAGC(enable_agc);
    initagc &= m_preprocess_left.SetAGCSettings(agc);
    if(channels == 2)
        initagc &= m_preprocess_right.SetAGCSettings(agc);
    
    bool initdenoise = m_preprocess_left.EnableDenoise(denoise);
    if(channels == 2)
        initdenoise &= m_preprocess_right.EnableDenoise(denoise);
    initdenoise &= m_preprocess_left.SetDenoiseLevel(denoise_level);
    if(channels == 2)
        initdenoise &= m_preprocess_right.SetDenoiseLevel(denoise_level);
    
    bool initdereverb = m_preprocess_left.EnableDereverb(true);
    if(channels == 2)
        initdereverb &= m_preprocess_right.EnableDereverb(true);
    
    bool initaec = m_preprocess_left.EnableEchoCancel(enable_aec);
    if(channels == 2)
        initaec &= m_preprocess_right.EnableEchoCancel(enable_aec);
    initaec &= m_preprocess_left.SetEchoSuppressLevel(aec.suppress_level);
    if(channels == 2)
        initaec &= m_preprocess_right.SetEchoSuppressLevel(aec.suppress_level);
    initaec &= m_preprocess_left.SetEchoSuppressActive(aec.suppress_active);
    if(channels == 2)
        initaec &= m_preprocess_right.SetEchoSuppressActive(aec.suppress_active);

    // Fixed point SpeexDSP library doesn't support AGC and AEC so
    // only report error if requested.
    return init && ((initagc && enable_agc) || !enable_agc) &&
        ((initdenoise && denoise) || !denoise) && ((initaec && enable_aec) || !enable_aec);
}
#endif
