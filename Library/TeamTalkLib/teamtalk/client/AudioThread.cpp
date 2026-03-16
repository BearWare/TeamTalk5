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

#include "AudioThread.h"

#include "myace/MyACE.h"
#include "teamtalk/CodecCommon.h"
#include "teamtalk/PacketLayout.h"
#include "teamtalk/TTAssert.h"

#if defined(ENABLE_WEBRTC)
#include "avstream/WebRTCPreprocess.h"
#include <api/audio/builtin_audio_processing_builder.h>
#include <api/environment/environment_factory.h>
#endif

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

using namespace teamtalk;

AudioThread::AudioThread()
{
    m_encbuf.resize(MAX_ENC_FRAMESIZE);
}

AudioThread::~AudioThread()
{
    MYTRACE(ACE_TEXT("AudioThread\n"));
}

bool AudioThread::StartEncoder(const audioencodercallback_t& callback,
                               const teamtalk::AudioCodec& codec,
                               bool spawn_thread)
{
    if(this->thr_count() != 0)
        return false;

    TTASSERT(this->msg_queue()->is_empty());

    int const callback_samples = GetAudioCodecCbSamples(codec);
    int const sample_rate = GetAudioCodecSampleRate(codec);
    int const channels = GetAudioCodecChannels(codec);

    switch(codec.codec)
    {
    case CODEC_NO_CODEC :
    {
        m_codec = codec;
        m_callback = callback;
        return true;
    }
    break;
    case CODEC_SPEEX :
#if defined(ENABLE_SPEEX)
    {
        TTASSERT(callback_samples);
        TTASSERT(sample_rate);
        TTASSERT(channels);

        m_speex = std::make_unique<SpeexEncoder>();
        if(!m_speex->Initialize(codec.speex.bandmode,
                                DEFAULT_SPEEX_COMPLEXITY,
                                codec.speex.quality))
        {
            StopEncoder();
            return false;
        }
    }
    break;
#else
    return false;
#endif
    case CODEC_SPEEX_VBR :
#if defined(ENABLE_SPEEX)
    {
        TTASSERT(callback_samples);
        TTASSERT(sample_rate);
        TTASSERT(channels);

        m_speex = std::make_unique<SpeexEncoder>();
        if(!m_speex->Initialize(codec.speex_vbr.bandmode,
                                DEFAULT_SPEEX_COMPLEXITY,
                                (float)codec.speex_vbr.vbr_quality,
                                codec.speex_vbr.bitrate,
                                codec.speex_vbr.max_bitrate,
                                codec.speex_vbr.dtx))
        {
            StopEncoder();
            return false;
        }
    }
    break;
#else
    return false;
#endif
#if defined(ENABLE_OPUS)
    case CODEC_OPUS :
    {
        TTASSERT(callback_samples);
        TTASSERT(sample_rate);
        TTASSERT(channels);

        m_opus = std::make_unique<OpusEncode>();
        if(!m_opus->Open(codec.opus.samplerate, codec.opus.channels,
                         codec.opus.application) ||
           !m_opus->SetComplexity(codec.opus.complexity) ||
           !m_opus->SetFEC(codec.opus.fec) ||
           !m_opus->SetDTX(codec.opus.dtx) ||
           !m_opus->SetBitrate(codec.opus.bitrate) ||
           !m_opus->SetVBR(codec.opus.vbr) ||
           !m_opus->SetVBRConstraint(codec.opus.vbr_constraint))
        {
            StopEncoder();
            return false;
        }
    }
    break;
#else
    return false;
#endif
    default:
        TTASSERT(codec.codec == CODEC_SPEEX);
    }

    TTASSERT(sample_rate);

    if((sample_rate == 0) || (callback_samples == 0))
        return false;

    m_codec = codec;
    m_callback = callback;

    //allow one second of audio to build up in the queue
    int max_queue = PCM16_BYTES(sample_rate, GetAudioCodecChannels(codec));
    max_queue += (1 + (sample_rate / callback_samples)) * sizeof(media::AudioFrame);

    this->msg_queue()->activate();
    this->msg_queue()->high_water_mark(max_queue);
    this->msg_queue()->low_water_mark(max_queue);

    if(spawn_thread && this->activate() < 0)
    {
        StopEncoder();
        return false;
    }

    MYTRACE_COND(codec.codec == CODEC_SPEEX,
                 ACE_TEXT("Launched Speex encoder, samplerate %d, bitrate %d, cb %d, fpp %d\n"),
                 GetAudioCodecSampleRate(codec), GetAudioCodecBitRate(codec),
                 GetAudioCodecFrameSize(codec), GetAudioCodecFramesPerPacket(codec));

    MYTRACE_COND(codec.codec == CODEC_SPEEX_VBR,
                 ACE_TEXT("Launched Speex VBR encoder, samplerate %d, bitrate %d, cb %d, fpp %d\n"),
                 GetAudioCodecSampleRate(codec), GetAudioCodecBitRate(codec),
                 GetAudioCodecFrameSize(codec), GetAudioCodecFramesPerPacket(codec));

    MYTRACE_COND(codec.codec == CODEC_OPUS,
                 ACE_TEXT("Launched OPUS VBR encoder, samplerate %d, bitrate %d, cb %d, channels %d\n"),
                 GetAudioCodecSampleRate(codec), GetAudioCodecBitRate(codec),
                 GetAudioCodecFrameSize(codec), GetAudioCodecChannels(codec));

    return true;
}

void AudioThread::StopEncoder()
{
    int const ret = this->msg_queue()->close();
    TTASSERT(ret >= 0);
    wait();

#if defined(ENABLE_SPEEXDSP)
    m_preprocess_left.reset();
    m_preprocess_right.reset();
#endif

#if defined(ENABLE_WEBRTC)
    m_apm.release();
    m_aps.reset();
#endif

#if defined(ENABLE_SPEEX)
    m_speex.reset();
#endif

#if defined(ENABLE_OPUS)
    m_opus.reset();
#endif
    m_enc_cleared = true;

    m_echobuf.clear();

    m_callback = {};

    memset(&m_codec, 0, sizeof(m_codec));
    m_codec.codec = teamtalk::CODEC_NO_CODEC;
}

int AudioThread::close(u_long /*flags*/)
{
    MYTRACE( ACE_TEXT("Audio Encoder thread closed\n") );
    return 0;
}

bool AudioThread::UpdatePreprocessor(const teamtalk::AudioPreprocessor& preprocess)
{
    //set AGC
    std::unique_lock<std::recursive_mutex> const g(m_preprocess_lock);

    if (preprocess.preprocessor != AUDIOPREPROCESSOR_TEAMTALK)
        MuteSound(false, false);

#if defined(ENABLE_SPEEXDSP)
    if (preprocess.preprocessor != AUDIOPREPROCESSOR_SPEEXDSP)
    {
        m_preprocess_left.reset();
        m_preprocess_right.reset();
    }
#endif

#if defined(ENABLE_WEBRTC)
    if (preprocess.preprocessor != AUDIOPREPROCESSOR_WEBRTC)
    {
        m_apm.release();
        m_aps.reset();
    }
#endif

    // just ignore preprocessor if not audio codec is set
    if (Codec().codec == CODEC_NO_CODEC)
        return true;

    MYTRACE(ACE_TEXT("Setting up audio preprocessor: %d\n"), preprocess.preprocessor);
    switch (preprocess.preprocessor)
    {
    case AUDIOPREPROCESSOR_WEBRTC_OBSOLETE_R4332 :
        return false;
    case AUDIOPREPROCESSOR_NONE :
        // 'm_gainlevel' should not be reset
        return true;
    case AUDIOPREPROCESSOR_SPEEXDSP :
        // 'm_gainlevel' should not be reset
        return UpdatePreprocess(preprocess.speexdsp);
    case AUDIOPREPROCESSOR_TEAMTALK :
        MuteSound(preprocess.ttpreprocessor.muteleft, preprocess.ttpreprocessor.muteright);
        m_gainlevel = preprocess.ttpreprocessor.gainlevel;
        return true;
    case AUDIOPREPROCESSOR_WEBRTC :
#if defined(ENABLE_WEBRTC)
        // WebRTC requires 10 msec audio frames
        if (GetAudioCodecCbMillis(m_codec) % 10 != 0)
        {
            MYTRACE(ACE_TEXT("Failed to initialize WebRTC audio preprocessor. Not 10 msec frames.\n"));
            return false;
        }

        if (!m_apm)
            m_apm = webrtc::BuiltinAudioProcessingBuilder().Build(webrtc::CreateEnvironment());
        m_apm->ApplyConfig(preprocess.webrtc);
        if (m_apm->Initialize() != webrtc::AudioProcessing::kNoError)
        {
            m_apm.release();
            MYTRACE(ACE_TEXT("Failed to initialize WebRTC audio preprocessor\n"));
            return false;
        }
        
                    MYTRACE(ACE_TEXT("Initialized WebRTC: gain2=%d level=%g, denoise=%d suppress=%d, echo%d\n"),
                    int(m_apm->GetConfig().gain_controller2.enabled),
                    double(m_apm->GetConfig().gain_controller2.fixed_digital.gain_db),
                    int(m_apm->GetConfig().noise_suppression.enabled),
                    int(m_apm->GetConfig().noise_suppression.level),
                    int(m_apm->GetConfig().echo_canceller.enabled));
       
        m_aps = std::make_unique<webrtc::AudioProcessingStats>();
        return true;
#else
        return false;
#endif
    }

    return false;
}

bool AudioThread::UpdatePreprocess(const teamtalk::SpeexDSP& speexdsp)
{
#if defined(ENABLE_SPEEXDSP)
    assert(Codec().codec != CODEC_NO_CODEC);

    int const callback_samples = GetAudioCodecCbSamples(Codec());
    int const sample_rate = GetAudioCodecSampleRate(Codec());
    int const channels = GetAudioCodecChannels(Codec());

    if (!m_preprocess_left)
    {
        if (channels == 2)
        {
            m_preprocess_left = std::make_unique<SpeexPreprocess>();
            m_preprocess_right = std::make_unique<SpeexPreprocess>();

            if (!m_preprocess_left->Initialize(sample_rate, callback_samples) ||
                !m_preprocess_right->Initialize(sample_rate, callback_samples))
            {
                m_preprocess_left.reset();
                m_preprocess_right.reset();
                return false;
            }

            //Speex has denoise on by default, so disable
            m_preprocess_left->EnableDenoise(false);
            m_preprocess_right->EnableDenoise(false);
        }
        else
        {
            m_preprocess_left = std::make_unique<SpeexPreprocess>();
            if (!m_preprocess_left->Initialize(sample_rate, callback_samples))
            {
                m_preprocess_left.reset();
                return false;
            }
            m_preprocess_left->EnableDenoise(false);
        }
    }

    SpeexAGC agc;
    agc.gain_level = (float)speexdsp.agc_gainlevel;
    agc.max_increment = speexdsp.agc_maxincdbsec;
    agc.max_decrement = speexdsp.agc_maxdecdbsec;
    agc.max_gain = speexdsp.agc_maxgaindb;

    //AGC
    bool agc_success = true;
    agc_success &= m_preprocess_left->EnableAGC(speexdsp.enable_agc);
    agc_success &= (channels == 1 || m_preprocess_right->EnableAGC(speexdsp.enable_agc));
    agc_success &= m_preprocess_left->SetAGCSettings(agc);
    agc_success &= (channels == 1 || m_preprocess_right->SetAGCSettings(agc));

    //denoise
    bool denoise_success = true;
    denoise_success &= m_preprocess_left->EnableDenoise(speexdsp.enable_denoise);
    denoise_success &= (channels == 1 || m_preprocess_right->EnableDenoise(speexdsp.enable_denoise));
    denoise_success &= m_preprocess_left->SetDenoiseLevel(speexdsp.maxnoisesuppressdb);
    denoise_success &= (channels == 1 || m_preprocess_right->SetDenoiseLevel(speexdsp.maxnoisesuppressdb));

    //set AEC
    bool aec_success = true;
    aec_success &= m_preprocess_left->EnableEchoCancel(speexdsp.enable_aec);
    aec_success &= (channels == 1 || m_preprocess_right->EnableEchoCancel(speexdsp.enable_aec));

    aec_success &= m_preprocess_left->SetEchoSuppressLevel(speexdsp.aec_suppress_level);
    aec_success &= (channels == 1 || m_preprocess_right->SetEchoSuppressLevel(speexdsp.aec_suppress_level));
    aec_success &= m_preprocess_left->SetEchoSuppressActive(speexdsp.aec_suppress_active);
    aec_success &= (channels == 1 || m_preprocess_right->SetEchoSuppressActive(speexdsp.aec_suppress_active));

    //set dereverb
    bool const dereverb = true;
    m_preprocess_left->EnableDereverb(dereverb);
    if(channels == 2)
        m_preprocess_right->EnableDereverb(dereverb);

    MYTRACE_COND(!agc_success && speexdsp.enable_agc,
                 ACE_TEXT("Failed to set SpeexDSP AGC settings\n"));
    MYTRACE_COND(!denoise_success && speexdsp.enable_denoise,
                 ACE_TEXT("Failed to set SpeexDSP denoise settings\n"));
    MYTRACE_COND(!aec_success && speexdsp.enable_aec,
                 ACE_TEXT("Failed to set SpeexDSP AEC settings\n"));

    if ((speexdsp.enable_agc && !agc_success) ||
        (speexdsp.enable_denoise && !denoise_success) ||
        (speexdsp.enable_aec && !aec_success))
        return false;

    MYTRACE(ACE_TEXT("Set audio cfg. AGC: %d, %d, %d, %d, %d. Denoise: %d, %d. AEC: %d, %d, %d.\n"),
            static_cast<int>(speexdsp.enable_agc), (int)speexdsp.agc_gainlevel,
            speexdsp.agc_maxincdbsec, speexdsp.agc_maxdecdbsec,
            speexdsp.agc_maxgaindb, static_cast<int>(speexdsp.enable_denoise),
            speexdsp.maxnoisesuppressdb, static_cast<int>(speexdsp.enable_aec),
            speexdsp.aec_suppress_level, speexdsp.aec_suppress_active);

    return true;
#else
    return false;
#endif
}

void AudioThread::MuteSound(bool leftchannel, bool rightchannel)
{
    m_stereo = ToStereoMask(leftchannel, rightchannel);
}

void AudioThread::QueueAudio(const media::AudioFrame& audframe)
{
    TTASSERT(m_codec.codec != CODEC_NO_CODEC);
    assert(audframe.inputfmt.channels == audframe.outputfmt.channels || audframe.outputfmt.channels == 0);
    assert(audframe.input_samples == audframe.output_samples || audframe.output_samples == 0);

    ACE_Message_Block* mb = AudioFrameToMsgBlock(audframe);
    if (mb != nullptr)
        QueueAudio(mb);
}

void AudioThread::QueueAudio(ACE_Message_Block* mb_audio)
{
    //add audio
    ACE_Time_Value tv;
    if(putq(mb_audio, &tv)<0)
    {
        MYTRACE(ACE_TEXT("AudioThread msg_q full, dropped frame\n"));
        mb_audio->release();
    }
}

bool AudioThread::IsVoiceActive()
{
#if defined(ENABLE_WEBRTC)
    std::unique_lock<std::recursive_mutex> const g(m_preprocess_lock);

    if (m_apm)
    {
        assert(m_aps);
        return m_aps->voice_detected.value_or(false) ||
            m_lastActive + m_voiceact_delay > ACE_OS::gettimeofday();
    }
#endif
    return m_voicelevel >= m_voiceactlevel ||
        m_lastActive + m_voiceact_delay > ACE_OS::gettimeofday();
}

int AudioThread::GetCurrentVoiceLevel() const
{
    return m_voicelevel;
}

void AudioThread::ProcessQueue(ACE_Time_Value* tm)
{
    TTASSERT(m_codec.codec != CODEC_NO_CODEC);
    TTASSERT(m_callback);
    ACE_Message_Block* mb = nullptr;
    while (getq(mb, tm) >= 0)
    {
        MBGuard const g(mb);
        media::AudioFrame af(mb);
        ProcessAudioFrame(af);
    }
}

int AudioThread::svc()
{
    ProcessQueue(nullptr);
    return 0;
}

void AudioThread::ProcessAudioFrame(media::AudioFrame& audblock)
{
    if(m_tone_frequency != 0u)
         m_tone_sample_index = GenerateTone(audblock, m_tone_sample_index, m_tone_frequency);

    SOFTGAIN(audblock.input_buffer, audblock.input_samples,
             audblock.inputfmt.channels, m_gainlevel, GAIN_NORMAL);

#if defined(ENABLE_SPEEXDSP)
    PreprocessSpeex(audblock);
#endif

#if defined(ENABLE_WEBRTC)
    if (m_gainlevel > 0)
    {
        // WebRTC preprocessing (especially AEC) is very CPU-intensive
        // Only do it if the input is not muted.
        // This allows client apps that use PTT to close the MIC input when there's
        // no PTT and thus prevent the processing hit.
        // AEC still functions fine if it's activated like this, although there's
        // minute echo fragment at the (re)start of the preprocessing
        PreprocessWebRTC(audblock);
    }
#endif

    MeasureVoiceLevel(audblock);

    // mute left or right speaker (if enabled)
    if(audblock.inputfmt.channels == 2)
        SelectStereo(m_stereo, audblock.input_buffer, audblock.input_samples);

    if ((IsVoiceActive() && audblock.voiceact_enc) || audblock.force_enc)
    {
        //encode
        const char* enc_data = nullptr;
        std::vector<int> enc_frame_sizes;
        switch(m_codec.codec)
        {
        case CODEC_SPEEX :
        case CODEC_SPEEX_VBR :
#if defined(ENABLE_SPEEX)
            enc_data = ProcessSpeex(audblock, enc_frame_sizes);
#endif
            break;
        case CODEC_OPUS :
#if defined(ENABLE_OPUS)
            enc_data = ProcessOPUS(audblock, enc_frame_sizes);
#endif
            break;
        case CODEC_NO_CODEC :
        case CODEC_WEBM_VP8 :
            break;
        }
        if(enc_data != nullptr)
        {
            int nbBytes = 0;
            for(int enc_frame_size : enc_frame_sizes)
                nbBytes += enc_frame_size;

            m_callback(m_codec, enc_data, nbBytes,
                       enc_frame_sizes, audblock);
        }
        m_enc_cleared = false;
    }
    else
    {
        //clear encoder state
        if(!m_enc_cleared)
        {
#if defined(ENABLE_SPEEX)
            if(m_speex)
                m_speex->Reset();
#endif
#if defined(ENABLE_OPUS)
            if (m_opus)
                m_opus->Reset();
#endif
            m_enc_cleared = true;
        }

        m_callback(m_codec, nullptr, 0, std::vector<int>(), audblock);
    }
}

void AudioThread::MeasureVoiceLevel(const media::AudioFrame& audblock)
{
    const int VU_MAX_VOLUME = 8000; //real maximum is if all samples are 32768
    const int VOICEACT_STOPDELAY = 1500;//msecs to wait before stopping after voiceact has been disabled

    int lsum = 0;
    int rsum = 0;
    int sum = 0;
    int const samples_total = audblock.input_samples * audblock.inputfmt.channels;
    if (audblock.inputfmt.channels == 2)
    {
        for (int i = 0; i < samples_total; i += 2)
        {
            lsum += abs(audblock.input_buffer[i]);
            rsum += abs(audblock.input_buffer[i + 1]);
        }
        switch (m_stereo)
        {
        case STEREO_BOTH:
            sum = (lsum + rsum) / 2;
            break;
        case STEREO_LEFT:
            sum = lsum;
            break;
        case STEREO_RIGHT:
            sum = rsum;
            break;
        case STEREO_NONE:
            sum = 0;
            break;
        }
    }
    else
    {
        for (int i = 0; i < samples_total; ++i)
            sum += abs(audblock.input_buffer[i]);
    }
    int avg = sum / audblock.input_samples;
    avg = 100 * avg / VU_MAX_VOLUME;
    this->m_voicelevel = avg > VU_METER_MAX ? VU_METER_MAX : avg;

    if (this->m_voicelevel >= this->m_voiceactlevel)
        m_lastActive = ACE_OS::gettimeofday();
}

#if defined(ENABLE_SPEEXDSP)
void AudioThread::PreprocessSpeex(media::AudioFrame& audblock)
{
    std::unique_lock<std::recursive_mutex> const g(m_preprocess_lock);

    bool preprocess = false;

    if (!m_preprocess_left)
        return;

    preprocess |= m_preprocess_left->IsEchoCancel();
    preprocess |= m_preprocess_left->IsDenoising();
    //don't include dereverb since it's not user configurable
//    preprocess |= m_preprocess_left->IsDereverbing();
    preprocess |= m_preprocess_left->IsAGC();

    if(!preprocess)
        return;

    if(audblock.inputfmt.channels == 1)
    {
        if (m_preprocess_left->IsEchoCancel() &&
            audblock.outputfmt.channels == 1 && (audblock.output_buffer != nullptr))
        {
            if(m_echobuf.size() != (size_t)audblock.input_samples)
                m_echobuf.resize(audblock.input_samples);

            m_preprocess_left->EchoCancel(audblock.input_buffer,
                                          audblock.output_buffer,
                                          m_echobuf.data());
            audblock.input_buffer = m_echobuf.data();
        }
        m_preprocess_left->Preprocess(audblock.input_buffer); //denoise, AGC, etc
    }
    else if(audblock.inputfmt.channels == 2)
    {
        assert(m_preprocess_right);
        std::vector<short> in_leftchan(audblock.input_samples);
        std::vector<short> in_rightchan(audblock.input_samples);
        SplitStereo(audblock.input_buffer, audblock.input_samples, in_leftchan, in_rightchan);

        if(m_preprocess_left->IsEchoCancel() && m_preprocess_right->IsEchoCancel() &&
           audblock.outputfmt.channels == 2 && (audblock.output_buffer != nullptr))
        {
            assert(audblock.input_samples == audblock.output_samples);

            std::vector<short> out_leftchan(audblock.output_samples);
            std::vector<short> out_rightchan(audblock.output_samples);
            std::vector<short> echobuf_left(audblock.output_samples);
            std::vector<short> echobuf_right(audblock.output_samples);
            SplitStereo(audblock.output_buffer, audblock.output_samples,
                        out_leftchan, out_rightchan);

            m_preprocess_left->EchoCancel(in_leftchan.data(), out_leftchan.data(),
                                         echobuf_left.data());
            in_leftchan.swap(echobuf_left);
            m_preprocess_right->EchoCancel(in_rightchan.data(), out_rightchan.data(),
                                         echobuf_right.data());
            in_rightchan.swap(echobuf_right);
        }

        m_preprocess_left->Preprocess(in_leftchan.data()); //denoise, AGC, etc
        m_preprocess_right->Preprocess(in_rightchan.data()); //denoise, AGC, etc

        MergeStereo(in_leftchan, in_rightchan, audblock.input_buffer,
                    audblock.input_samples);
    }
}
#endif

#if defined(ENABLE_WEBRTC)
void AudioThread::PreprocessWebRTC(media::AudioFrame& audblock)
{
    std::unique_lock<std::recursive_mutex> const g(m_preprocess_lock);

    if (!m_apm)
        return;

    if (WebRTCPreprocess(*m_apm, audblock, audblock, m_aps.get()) != audblock.input_samples)
    {
        MYTRACE(ACE_TEXT("WebRTC failed to process audio\n"));
    }
}
#endif

#if defined(ENABLE_SPEEX)
const char* AudioThread::ProcessSpeex(const media::AudioFrame& audblock,
                                      std::vector<int>& enc_frame_sizes)
{
    TTASSERT(m_speex);

    int const framesize = GetAudioCodecFrameSize(m_codec);
    int nbBytes = 0;
    int n_processed = 0;
    int ret;
    int const fpp = GetAudioCodecFramesPerPacket(m_codec);
    int enc_frm_size = 0;

    assert(fpp);
    assert(framesize>0);
    if (framesize <= 0 || fpp <= 0)
        return nullptr;

    enc_frm_size = int(m_encbuf.size()) / fpp;

    while(n_processed < audblock.input_samples)
    {
        assert(nbBytes + enc_frm_size <= int(m_encbuf.size()));
        ret = m_speex->Encode(&audblock.input_buffer[n_processed],
                              &m_encbuf[nbBytes], enc_frm_size);
        assert(ret>0);
        if(ret <= 0)
            return nullptr;

        enc_frame_sizes.push_back(ret);
        n_processed += framesize;
        nbBytes += ret;
    }
    TTASSERT(nbBytes <= m_encbuf.size());
    return m_encbuf.data();
}
#endif

#if defined(ENABLE_OPUS)
const char* AudioThread::ProcessOPUS(const media::AudioFrame& audblock,
                                     std::vector<int>& enc_frame_sizes)
{
    TTASSERT(m_opus);
    TTASSERT(audblock.input_samples == GetAudioCodecCbSamples(m_codec));
    int const framesize = GetAudioCodecFrameSize(m_codec);
    int const channels = GetAudioCodecChannels(m_codec);
    int const fpp = GetAudioCodecFramesPerPacket(m_codec);
    int nbBytes = 0;
    int n_processed = 0;
    int ret;
    int enc_frm_size = 0;

    assert(fpp);
    assert(framesize>0);
    if (framesize <= 0 || fpp <= 0)
        return nullptr;

    enc_frm_size = int(m_encbuf.size()) / fpp;

    while(n_processed < audblock.input_samples)
    {
        assert(nbBytes + enc_frm_size <= int(m_encbuf.size()));
        ret = m_opus->Encode(&audblock.input_buffer[n_processed*channels],
                             framesize, &m_encbuf[nbBytes], enc_frm_size);
        assert(ret>0);
        if(ret <= 0)
            return nullptr;

        // enc_frm_size -= ret; /* stay within MAX_ENC_FRAMESIZE */
        enc_frame_sizes.push_back(ret);
        n_processed += framesize;
        nbBytes += ret;
    }
    TTASSERT(nbBytes <= m_encbuf.size());
    return m_encbuf.data();
}
#endif
