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

#include "SpeexPreprocess.h"
#include <assert.h>

#include <cstddef>

SpeexPreprocess::SpeexPreprocess()
: m_preprocess_state(NULL)
, m_echo_state(NULL)
, m_framesize(0)
, m_samplerate(0)
{
}

SpeexPreprocess::~SpeexPreprocess()
{
    Close();
}

bool SpeexPreprocess::Initialize(int samplerate, int framesize)
{
    assert(m_preprocess_state == NULL);
    if(m_preprocess_state)
        return false;

    m_samplerate = samplerate;
    m_framesize = framesize;
    assert(m_framesize>0);
    m_preprocess_state = speex_preprocess_state_init(framesize, samplerate);
    assert(m_preprocess_state);

    return true;
}

void SpeexPreprocess::Close()
{
    //destroy echo cancel
    EnableEchoCancel(false);

    //destroy preprocessor
    if(m_preprocess_state)
        speex_preprocess_state_destroy(m_preprocess_state); 
    m_preprocess_state = NULL;

    m_samplerate = m_framesize = 0;
}

bool SpeexPreprocess::EnableDenoise(bool enable)
{
    assert(m_preprocess_state);
    if(m_preprocess_state)
    {
        int n = enable? 1 : 0;
        bool b = speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_SET_DENOISE, &n) == 0;
        return b;
    }
    return false;
}

bool SpeexPreprocess::IsDenoising()
{
    int n = 0;
    assert(m_preprocess_state);
    if(m_preprocess_state)
        speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_GET_DENOISE, &n);
    return n != 0;
}

bool SpeexPreprocess::SetDenoiseLevel(int level)
{
    assert(m_preprocess_state);
    if(m_preprocess_state)
        return speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &level) == 0;
    return false;
}

int SpeexPreprocess::GetDenoiseLevel()
{
    assert(m_preprocess_state);
    int n = 0;
    if(m_preprocess_state)
    {
        bool b = speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_GET_NOISE_SUPPRESS, &n) == 0;
        assert(b);
    }
    return n;
}

bool SpeexPreprocess::EnableDereverb(bool enable)
{
    int n = enable? 1 : 0;
    assert(m_preprocess_state);
    if(m_preprocess_state)
        return speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_SET_DEREVERB, &n) == 0;
    return false;
}

bool SpeexPreprocess::IsDereverbing()
{
    int n = 0;
    assert(m_preprocess_state);
    if(m_preprocess_state)
        speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_GET_DEREVERB, &n);
    return n != 0;
}

bool SpeexPreprocess::EnableAGC(bool enable)
{
#ifdef DISABLE_SPEEX_AGC
    return false;
#else
    int n = enable? 1 : 0;
    assert(m_preprocess_state);
    if(m_preprocess_state)
        return speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_SET_AGC, &n) == 0;
    return false;
#endif
}

bool SpeexPreprocess::IsAGC() const
{
#ifdef DISABLE_SPEEX_AGC
    return false;
#else
    int n = 0;
    if(m_preprocess_state)
        speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_GET_AGC, &n);
    return n != 0;
#endif
}

bool SpeexPreprocess::SetAGCSettings(const SpeexAGC& agc)
{
#ifdef DISABLE_SPEEX_AGC
    return false;
#else
    assert(m_preprocess_state);
    if(m_preprocess_state)
    {
        bool b = true;
        float val = agc.gain_level;
        int n = 0;
        b &= speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_SET_AGC_LEVEL, &val) == 0;
        n = agc.max_increment;
        b &= speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_SET_AGC_INCREMENT, &n) == 0;
        n = agc.max_decrement;
        b &= speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_SET_AGC_DECREMENT, &n) == 0;
        n = agc.max_gain;
        b &= speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_SET_AGC_MAX_GAIN, &n) == 0;
        //n = agc.agc_target;
        //b &= speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_SET_AGC_TARGET, &n) == 0;
        //assert(b);
        return b;
    }
    return false;
#endif
}

bool SpeexPreprocess::GetAGCSettings(SpeexAGC& agc)
{
#ifdef DISABLE_SPEEX_AGC
    return false;
#else
    if(m_preprocess_state)
    {
        assert(IsAGC());

        bool b = true;
        b &= speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_GET_AGC_LEVEL, &agc.gain_level) == 0;
        assert(b);
        b &= speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_GET_AGC_INCREMENT, &agc.max_increment) == 0;
        assert(b);
        b &= speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_GET_AGC_DECREMENT, &agc.max_decrement) == 0;
        assert(b);
        b &= speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_GET_AGC_MAX_GAIN, &agc.max_gain) == 0;
        assert(b);
        //b &= speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_GET_AGC_TARGET, &agc.agc_target) == 0;
        //assert(b);
        return b;
    }
    return false;
#endif
}

bool SpeexPreprocess::EnableEchoCancel(bool enable)
{
    if(enable)
    {
        if(!m_preprocess_state || !m_framesize || !m_samplerate)
            return false;

        if(m_echo_state)
        {
            //destroy old state
            EnableEchoCancel(false);
        }

        m_echo_state = speex_echo_state_init(m_framesize, m_samplerate/2);
        assert(m_echo_state);
        int ret = speex_echo_ctl(m_echo_state, SPEEX_ECHO_SET_SAMPLING_RATE, &m_samplerate);
        assert(ret == 0);
        ret = speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_SET_ECHO_STATE, m_echo_state);
        assert(ret == 0);
        return true;
    }
    else
    {
        if(m_preprocess_state)
        {
            int ret = speex_preprocess_ctl(m_preprocess_state, SPEEX_PREPROCESS_SET_ECHO_STATE, NULL);
            assert(ret == 0);
        }
        if(m_echo_state)
        {
            speex_echo_state_destroy(m_echo_state);
            m_echo_state = NULL;
        }
        return m_preprocess_state != NULL;
    }
}

bool SpeexPreprocess::IsEchoCancel() const
{
    return m_echo_state != NULL;
}

bool SpeexPreprocess::SetEchoSuppressLevel(int level)
{
    if(m_preprocess_state)
    {
        int ret = speex_preprocess_ctl(m_preprocess_state, 
            SPEEX_PREPROCESS_SET_ECHO_SUPPRESS, &level);
        assert(ret == 0);
        return ret == 0;
    }
    return false;
}

int SpeexPreprocess::GetEchoSuppressLevel()
{
    int n = 0;
    if(m_preprocess_state)
    {
        int ret = speex_preprocess_ctl(m_preprocess_state, 
            SPEEX_PREPROCESS_GET_ECHO_SUPPRESS, &n);
        assert(ret == 0);
    }
    return n;
}

bool SpeexPreprocess::SetEchoSuppressActive(int level)
{
    if(m_preprocess_state)
    {
        int ret = speex_preprocess_ctl(m_preprocess_state, 
            SPEEX_PREPROCESS_SET_ECHO_SUPPRESS_ACTIVE, &level);
        assert(ret == 0);
        return ret == 0;
    }
    return false;
}

int SpeexPreprocess::GetEchoSuppressActive()
{
    int n = 0;
    if(m_preprocess_state)
    {
        int ret = speex_preprocess_ctl(m_preprocess_state, 
            SPEEX_PREPROCESS_GET_ECHO_SUPPRESS_ACTIVE, &n);
        assert(ret == 0);
    }
    return n;
}

void SpeexPreprocess::ResetEcho()
{
    assert(m_echo_state);
    if(m_echo_state)
    {
        speex_echo_state_reset(m_echo_state);
    }
}

void SpeexPreprocess::EchoCancel(const short* capture, 
                                 const short* speaker, 
                                 short* cancelled)
{
    assert(speaker);
    assert(capture);
    assert(cancelled);
    assert(m_echo_state);
    if(m_echo_state)
        speex_echo_cancellation(m_echo_state, capture, speaker, cancelled);
}

void SpeexPreprocess::EchoPlayback(const short* speaker)
{
    assert(m_echo_state);
    if(m_echo_state)
        speex_echo_playback(m_echo_state, speaker);
}

void SpeexPreprocess::EchoCapture(const short* capture,
                                  short* cancelled)
{
    assert(m_echo_state);
    if(m_echo_state)
        speex_echo_capture(m_echo_state, capture, cancelled);
}

bool SpeexPreprocess::Preprocess(short* input_buffer)
{
    assert(m_preprocess_state);
    if(m_preprocess_state)
    {
        speex_preprocess_run(m_preprocess_state, input_buffer); 
        return true;
    }
    return false;
}
