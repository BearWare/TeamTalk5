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

#if !defined(AUDIOTHREAD_H)
#define AUDIOTHREAD_H

#include <ace/Task.h>

#if defined(ENABLE_SPEEX)
#include <codec/SpeexEncoder.h>
#endif
#if defined(ENABLE_SPEEXDSP)
#include <avstream/SpeexPreprocess.h>
#endif
#if defined(ENABLE_WEBRTC)
#include <avstream/WebRTCPreprocess.h>
#endif
#if defined(ENABLE_OPUS)
#include <codec/OpusEncoder.h>
#endif

#include <codec/MediaUtil.h>
#include <teamtalk/Common.h>

#include <memory>
#include <mutex>

typedef std::function< void (const teamtalk::AudioCodec& codec,
                             const char* enc_data, int enc_len,
                             const std::vector<int>& enc_frame_sizes,
                             const media::AudioFrame& org_frame) > audioencodercallback_t;

class AudioThread : protected ACE_Task<ACE_MT_SYNCH>
{
public:
    AudioThread();
    virtual ~AudioThread();

    bool StartEncoder(audioencodercallback_t callback,
                      const teamtalk::AudioCodec& codec,
                      bool spawn_thread);
    void StopEncoder();

    void EnableTone(int frequency) { m_tone_frequency = frequency; }

    void QueueAudio(const media::AudioFrame& audframe);
    void QueueAudio(ACE_Message_Block* mb_audio);
    bool IsVoiceActive();
    int GetCurrentVoiceLevel();

    bool UpdatePreprocessor(const teamtalk::AudioPreprocessor& preprocess);

    int m_voiceactlevel = VU_METER_MIN;
    ACE_Time_Value m_voiceact_delay = ACE_Time_Value(1, 500000);

    //real maximum is 100 (when all samples are 32768)
    static const int VU_METER_MAX = 100;
    static const int VU_METER_MIN = 0;

    const teamtalk::AudioCodec& codec() const { return m_codec; }

    void ProcessQueue(ACE_Time_Value* tm);

    //voice gain
    int m_gainlevel = GAIN_NORMAL;    //GAIN_NORMAL == disabled

private:
    int close(u_long);
    int svc(void);
    void ProcessAudioFrame(media::AudioFrame& audblock);
    void MeasureVoiceLevel(const media::AudioFrame& audblock);

    void MuteSound(bool leftchannel, bool rightchannel);
    bool UpdatePreprocess(const teamtalk::SpeexDSP& speexdsp);
#if defined(ENABLE_SPEEXDSP)
    void PreprocessSpeex(media::AudioFrame& audblock);
#endif
#if defined(ENABLE_WEBRTC)
    void PreprocessWebRTC(media::AudioFrame& audblock);
#endif
#if defined(ENABLE_SPEEX)
    const char* ProcessSpeex(const media::AudioFrame& audblock,
                             std::vector<int>& enc_frame_sizes);
#endif
#if defined(ENABLE_OPUS)
    const char* ProcessOPUS(const media::AudioFrame& audblock,
                            std::vector<int>& env_frame_sizes);
#endif
    audioencodercallback_t m_callback;
    std::recursive_mutex m_preprocess_lock;
#if defined(ENABLE_SPEEXDSP)
    std::unique_ptr<SpeexPreprocess> m_preprocess_left, m_preprocess_right;
#endif
#if defined(ENABLE_WEBRTC)
    rtc::scoped_refptr<webrtc::AudioProcessing> m_apm;
    std::unique_ptr<webrtc::AudioProcessingStats> m_aps;
#endif
#if defined(ENABLE_SPEEX)
    std::unique_ptr<SpeexEncoder> m_speex;
#endif
#if defined(ENABLE_OPUS)
    std::unique_ptr<OpusEncode> m_opus;
#endif
    std::vector<char> m_encbuf;
    std::vector<short> m_echobuf;
    teamtalk::AudioCodec m_codec;

    int m_voicelevel = VU_METER_MIN;

    // TTAudioPreprocessor
    StereoMask m_stereo = STEREO_BOTH;

    //encoder state has been reset
    bool m_enc_cleared = true;

    //voice activation
    ACE_Time_Value m_lastActive;

    // tone generation
    ACE_UINT32 m_tone_sample_index = 0, m_tone_frequency = 0;
};

typedef std::shared_ptr< AudioThread > audio_thread_t;

#endif
