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

#if !defined(SPEEXPREPROCESS_H)
#define SPEEXPREPROCESS_H

#include <speex/speex_preprocess.h>
#include <speex/speex_echo.h>

#define SPEEX_DENOISE_LEVEL_DEFAULT -15

struct SpeexAGC
{
    float gain_level;
    int max_increment;
    int max_decrement;
    int max_gain;
    //int agc_target;
    SpeexAGC()
    {
        gain_level = 8000.0;
        max_increment = 12;
        max_decrement = -40;
        max_gain = 30;
        //agc_target = 8000;
    }
};

struct SpeexAEC
{
    int suppress_level;
    int suppress_active;
    SpeexAEC()
    {
        suppress_level = -40;
        suppress_active = -15;
    }
};

class SpeexPreprocess
{
public:
    SpeexPreprocess();
    ~SpeexPreprocess();

    bool Initialize(int samplerate, int framesize);
    void Close();

    bool EnableDenoise(bool enable);
    bool IsDenoising();
    bool SetDenoiseLevel(int level);
    int GetDenoiseLevel();

    bool EnableDereverb(bool enable);
    bool IsDereverbing();

    bool EnableAGC(bool enable);
    bool IsAGC() const;
    bool SetAGCSettings(const SpeexAGC& agc);
    bool GetAGCSettings(SpeexAGC& agc);

    bool EnableEchoCancel(bool enable);
    bool IsEchoCancel() const;
    bool SetEchoSuppressLevel(int level);
    int GetEchoSuppressLevel();
    bool SetEchoSuppressActive(int level);
    int GetEchoSuppressActive();
    void ResetEcho();

    void EchoCancel(const short* capture, const short* speaker, short* cancelled);

    void EchoPlayback(const short* speaker);
    void EchoCapture(const short* capture, short* cancelled);

    bool Preprocess(short* input_buffer);

private:
    SpeexPreprocessState* m_preprocess_state; 
    SpeexEchoState* m_echo_state;
    int m_framesize, m_samplerate;
};


#endif

