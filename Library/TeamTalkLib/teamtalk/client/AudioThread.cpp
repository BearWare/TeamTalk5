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

#include <myace/MyACE.h>
#include <teamtalk/ttassert.h>
#include <teamtalk/CodecCommon.h>
#include <codec/MediaUtil.h>

using namespace std;
using namespace teamtalk;

AudioThread::AudioThread()
: m_voicelevel(VU_METER_MIN)
, m_voiceactlevel(VU_METER_MIN)
, m_gainlevel(GAIN_NORMAL)
, m_enc_cleared(true)
, m_voiceact_delay(1, 500000)
, m_tone_sample_index(0)
, m_tone_frequency(0)
{
    memset(&m_codec, 0, sizeof(m_codec));
    m_codec.codec = teamtalk::CODEC_NO_CODEC;
    m_encbuf.resize(MAX_ENC_FRAMESIZE);
}

AudioThread::~AudioThread()
{
}

bool AudioThread::StartEncoder(audioencodercallback_t callback, 
                               const teamtalk::AudioCodec& codec,
                               bool spawn_thread)
{
    if(this->thr_count() != 0)
        return false;

    TTASSERT(this->msg_queue()->is_empty());

    int callback_samples = GetAudioCodecCbSamples(codec);
    int sample_rate = GetAudioCodecSampleRate(codec);
    int channels = GetAudioCodecChannels(codec);

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

        m_speex.reset(new SpeexEncoder());
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

        m_speex.reset(new SpeexEncoder());
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

        m_opus.reset(new OpusEncode());
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

    if(!sample_rate || !callback_samples)
        return false;

#if defined(ENABLE_SPEEXDSP)
    if(channels == 2)
    {
        if(!m_preprocess_left.Initialize(sample_rate, callback_samples) ||
           !m_preprocess_right.Initialize(sample_rate, callback_samples))
        {
            StopEncoder();
            return false;
        }
        
        //Speex has denoise on by default, so disable
        m_preprocess_left.EnableDenoise(false);
        m_preprocess_right.EnableDenoise(false);
    }
    else
    {
        if(!m_preprocess_left.Initialize(sample_rate, callback_samples))
        {
            StopEncoder();
            return false;
        }
        m_preprocess_left.EnableDenoise(false);
    }
#endif

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
    int ret = this->msg_queue()->close();
    TTASSERT(ret >= 0);
    wait();

#if defined(ENABLE_SPEEXDSP)
    m_preprocess_left.Close();
    m_preprocess_right.Close();
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

int AudioThread::close(u_long)
{
    MYTRACE( ACE_TEXT("Audio Encoder thread closed\n") );
    return 0;
}

bool AudioThread::UpdatePreprocessor(const teamtalk::AudioPreprocessor& preprocess)
{
    //set AGC
    std::unique_lock<std::recursive_mutex> g(m_preprocess_lock);

    switch (preprocess.preprocessor)
    {
    case AUDIOPREPROCESSOR_NONE :
        MuteSound(false, false);
        UpdatePreprocess(teamtalk::SpeexDSP());
        return true;
    case AUDIOPREPROCESSOR_SPEEXDSP :
        MuteSound(false, false);
        return UpdatePreprocess(preprocess.speexdsp);
    case AUDIOPREPROCESSOR_TEAMTALK :
        MuteSound(preprocess.ttpreprocessor.muteleft, preprocess.ttpreprocessor.muteright);
        m_gainlevel = preprocess.ttpreprocessor.gainlevel;
        UpdatePreprocess(teamtalk::SpeexDSP()); // disable SpeexDSP
        return true;
    }
    
    return false;
}

bool AudioThread::UpdatePreprocess(const teamtalk::SpeexDSP& speexdsp)
{
#if defined(ENABLE_SPEEXDSP)
    //if audio thread isn't running, then Speex preprocess is not set up
    if(codec().codec == CODEC_NO_CODEC)
        return true;

    int channels = GetAudioCodecChannels(codec());

    SpeexAGC agc;
    agc.gain_level = (float)speexdsp.agc_gainlevel;
    agc.max_increment = speexdsp.agc_maxincdbsec;
    agc.max_decrement = speexdsp.agc_maxdecdbsec;
    agc.max_gain = speexdsp.agc_maxgaindb;

    //AGC
    bool agc_success = true;
    agc_success &= m_preprocess_left.EnableAGC(speexdsp.enable_agc);
    agc_success &= (channels == 1 || m_preprocess_right.EnableAGC(speexdsp.enable_agc));
    agc_success &= m_preprocess_left.SetAGCSettings(agc);
    agc_success &= (channels == 1 || m_preprocess_right.SetAGCSettings(agc));

    //denoise
    bool denoise_success = true;
    denoise_success &= m_preprocess_left.EnableDenoise(speexdsp.enable_denoise);
    denoise_success &= (channels == 1 || m_preprocess_right.EnableDenoise(speexdsp.enable_denoise));
    denoise_success &= m_preprocess_left.SetDenoiseLevel(speexdsp.maxnoisesuppressdb);
    denoise_success &= (channels == 1 || m_preprocess_right.SetDenoiseLevel(speexdsp.maxnoisesuppressdb));

    //set AEC
    bool aec_success = true;
    aec_success &= m_preprocess_left.EnableEchoCancel(speexdsp.enable_aec);
    aec_success &= (channels == 1 || m_preprocess_right.EnableEchoCancel(speexdsp.enable_aec));

    aec_success &= m_preprocess_left.SetEchoSuppressLevel(speexdsp.aec_suppress_level);
    aec_success &= (channels == 1 || m_preprocess_right.SetEchoSuppressLevel(speexdsp.aec_suppress_level));
    aec_success &= m_preprocess_left.SetEchoSuppressActive(speexdsp.aec_suppress_active);
    aec_success &= (channels == 1 || m_preprocess_right.SetEchoSuppressActive(speexdsp.aec_suppress_active));

    //set dereverb
    bool dereverb = true;
    m_preprocess_left.EnableDereverb(dereverb);
    if(channels == 2)
        m_preprocess_right.EnableDereverb(dereverb);

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
            speexdsp.enable_agc, (int)speexdsp.agc_gainlevel,
            speexdsp.agc_maxincdbsec, speexdsp.agc_maxdecdbsec,
            speexdsp.agc_maxgaindb, speexdsp.enable_denoise,
            speexdsp.maxnoisesuppressdb, speexdsp.enable_aec,
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
    if (mb)
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


bool AudioThread::IsVoiceActive() const
{ 
    return m_voicelevel >= m_voiceactlevel || 
        m_lastActive + m_voiceact_delay > ACE_OS::gettimeofday();
}

void AudioThread::ProcessQueue(ACE_Time_Value* tm)
{
    TTASSERT(m_codec.codec != CODEC_NO_CODEC);
    TTASSERT(m_callback);
    ACE_Message_Block* mb;
    while(getq(mb, tm) >= 0)
    {
        media::AudioFrame* audframe = reinterpret_cast<media::AudioFrame*>(mb->rd_ptr());
        ProcessAudioFrame(*audframe);
        mb->release();
    }
}

int AudioThread::svc(void)
{
    ProcessQueue(NULL);
    return 0;
}

void AudioThread::ProcessAudioFrame(media::AudioFrame& audblock)
{
    if(m_tone_frequency)
         m_tone_sample_index = GenerateTone(audblock, m_tone_sample_index, m_tone_frequency);

    SOFTGAIN(audblock.input_buffer, audblock.input_samples, 
             audblock.inputfmt.channels, m_gainlevel, GAIN_NORMAL);

#if defined(ENABLE_SPEEXDSP)
    PreprocessAudioFrame(audblock);
#endif

    /*  Measure voice activity */
    const int VU_MAX_VOLUME = 8000; //real maximum is if all samples are 32768
    const int VOICEACT_STOPDELAY = 1500;//msecs to wait before stopping after voiceact has been disabled

    int lsum = 0, rsum = 0, sum = 0;
    int samples_total = audblock.input_samples * audblock.inputfmt.channels;
    if (audblock.inputfmt.channels == 2)
    {
        for(int i=0;i<samples_total;i+=2)
        {
            lsum += abs(audblock.input_buffer[i]);
            rsum += abs(audblock.input_buffer[i+1]);
        }
        switch (m_stereo)
        {
        case STEREO_BOTH :
            sum = (lsum + rsum) / 2;
            break;
        case STEREO_LEFT :
            sum = lsum;
            break;
        case STEREO_RIGHT :
            sum = rsum;
            break;
        case STEREO_NONE :
            sum = 0;
            break;
        }
    }
    else
    {
        for(int i=0;i<samples_total;++i)
            sum += abs(audblock.input_buffer[i]);
    }
    int avg = sum / audblock.input_samples;
    avg = 100 * avg / VU_MAX_VOLUME;
    this->m_voicelevel = avg>VU_METER_MAX? VU_METER_MAX : avg;

    if(this->m_voicelevel >= this->m_voiceactlevel)
        m_lastActive = ACE_OS::gettimeofday();
    /*  Measure voice activity - end */

    // mute left or right speaker (if enabled)
    if(audblock.inputfmt.channels == 2)
        SelectStereo(m_stereo, audblock.input_buffer, audblock.input_samples);

    if((this->IsVoiceActive() && audblock.voiceact_enc) || audblock.force_enc)
    {
        //encode
        const char* enc_data = NULL;
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
        if(enc_data)
        {
            int nbBytes = 0;
            for(size_t i=0;i<enc_frame_sizes.size();i++)
                nbBytes += enc_frame_sizes[i];

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

        m_callback(m_codec, NULL, 0, std::vector<int>(), audblock);
    }
}

#if defined(ENABLE_SPEEXDSP)
void AudioThread::PreprocessAudioFrame(media::AudioFrame& audblock)
{
    std::unique_lock<std::recursive_mutex> g(m_preprocess_lock);

    bool preprocess = false;

    preprocess |= m_preprocess_left.IsEchoCancel();
    preprocess |= m_preprocess_left.IsDenoising();
    //don't include dereverb since it's not user configurable
//    preprocess |= m_preprocess_left.IsDereverbing();
    preprocess |= m_preprocess_left.IsAGC();

    if(!preprocess)
        return;

    if(audblock.inputfmt.channels == 1)
    {
        if(m_preprocess_left.IsEchoCancel() &&
           audblock.outputfmt.channels == 1 && audblock.output_buffer)
        {
            if(m_echobuf.size() != (size_t)audblock.input_samples)
                m_echobuf.resize(audblock.input_samples);

            m_preprocess_left.EchoCancel(audblock.input_buffer,
                                         audblock.output_buffer,
                                         &m_echobuf[0]);
            audblock.input_buffer = &m_echobuf[0];
        }
        m_preprocess_left.Preprocess(audblock.input_buffer); //denoise, AGC, etc
    }
    else if(audblock.inputfmt.channels == 2)
    {
        vector<short> in_leftchan(audblock.input_samples), 
                      in_rightchan(audblock.input_samples);
        SplitStereo(audblock.input_buffer, audblock.input_samples, in_leftchan, in_rightchan);

        if(m_preprocess_left.IsEchoCancel() && m_preprocess_right.IsEchoCancel() &&
           audblock.outputfmt.channels == 2 && audblock.output_buffer)
        {
            assert(audblock.input_samples == audblock.output_samples);

            vector<short> out_leftchan(audblock.output_samples), 
                          out_rightchan(audblock.output_samples),
                          echobuf_left(audblock.output_samples),
                          echobuf_right(audblock.output_samples);
            SplitStereo(audblock.output_buffer, audblock.output_samples,
                        out_leftchan, out_rightchan);

            m_preprocess_left.EchoCancel(&in_leftchan[0], &out_leftchan[0], 
                                         &echobuf_left[0]);
            in_leftchan.swap(echobuf_left);
            m_preprocess_right.EchoCancel(&in_rightchan[0], &out_rightchan[0], 
                                         &echobuf_right[0]);
            in_rightchan.swap(echobuf_right);
        }

        m_preprocess_left.Preprocess(&in_leftchan[0]); //denoise, AGC, etc
        m_preprocess_right.Preprocess(&in_rightchan[0]); //denoise, AGC, etc

        MergeStereo(in_leftchan, in_rightchan, audblock.input_buffer, 
                    audblock.input_samples);
    }
}
#endif


#if defined(ENABLE_SPEEX)
const char* AudioThread::ProcessSpeex(const media::AudioFrame& audblock,
                                      std::vector<int>& enc_frame_sizes)
{
    TTASSERT(m_speex);
    
    int framesize = GetAudioCodecFrameSize(m_codec);
    int nbBytes = 0, n_processed = 0, ret;
    int fpp = GetAudioCodecFramesPerPacket(m_codec);
    int enc_frm_size;

    assert(fpp);
    assert(framesize>0);
    if (framesize <= 0 || fpp <= 0)
        return nullptr;

    enc_frm_size = int(m_encbuf.size()) / fpp;

    while(n_processed < audblock.input_samples)
    {
        assert(nbBytes + enc_frm_size <= m_encbuf.size());
        ret = m_speex->Encode(&audblock.input_buffer[n_processed], 
                              &m_encbuf[nbBytes], enc_frm_size);
        assert(ret>0);
        if(ret <= 0)
            return nullptr;

        enc_frame_sizes.push_back(ret);
        n_processed += framesize;
        nbBytes += ret;
    }
    TTASSERT(nbBytes <= (int)m_encbuf.size());
    return &m_encbuf[0];
}
#endif

#if defined(ENABLE_OPUS)
const char* AudioThread::ProcessOPUS(const media::AudioFrame& audblock, 
                                     std::vector<int>& enc_frame_sizes)
{
    TTASSERT(m_opus);
    TTASSERT(audblock.input_samples == GetAudioCodecCbSamples(m_codec));
    int framesize = GetAudioCodecFrameSize(m_codec);
    int channels = GetAudioCodecChannels(m_codec);
    int fpp = GetAudioCodecFramesPerPacket(m_codec);
    int nbBytes = 0, n_processed = 0, ret;
    int enc_frm_size;

    assert(fpp);
    assert(framesize>0);
    if (framesize <= 0 || fpp <= 0)
        return nullptr;

    enc_frm_size = int(m_encbuf.size()) / fpp;
    
    while(n_processed < audblock.input_samples)
    {
        assert(nbBytes + enc_frm_size <= m_encbuf.size());
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
    TTASSERT(nbBytes <= (int)m_encbuf.size());
    return &m_encbuf[0];
}
#endif
