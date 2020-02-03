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

#include "AudioMuxer.h"
#include <teamtalk/ttassert.h>

#define AUDIOBLOCK_QUEUE_MSEC 1000

using namespace teamtalk;

struct AudioMuxBlock
{
    short* audio;
    ACE_UINT32 sample_no;
    bool last;
    AudioMuxBlock() : audio(NULL), sample_no(0), last(false) {}
};

AudioMuxer::AudioMuxer()
{
}

AudioMuxer::~AudioMuxer()
{
    StopThread();
    MYTRACE(ACE_TEXT("~AudioMuxer()\n"));
}

bool AudioMuxer::Init(const teamtalk::AudioCodec& codec)
{
    if (m_codec == codec)
        return true;

    return StartThread(codec);
}

bool AudioMuxer::StartThread(const teamtalk::AudioCodec& codec)
{
    TTASSERT(m_codec == AudioCodec());

    TTASSERT(!m_thread);
    if (m_thread)
        return false;

    //mux interval
    int samples = GetAudioCodecCbTotalSamples(codec);
    TTASSERT(samples>0);
    if(samples <= 0)
        return false;

    MYTRACE(ACE_TEXT("Starting AudioMuxer with sample rate %d and callback %d\n"),
            GetAudioCodecSampleRate(codec), GetAudioCodecCbSamples(codec));

    m_muxed_audio.resize(samples);

    m_codec = codec;

    m_sample_no = 0;
    m_last_flush_time = GETTIMESTAMP();

    m_thread.reset(new std::thread(&AudioMuxer::Run, this));
    return true;
}

void AudioMuxer::StopThread()
{
    if (m_thread)
    {
        int ret;
        ret = m_reactor.end_reactor_event_loop();
        TTASSERT(ret >= 0);

        m_thread->join();
        m_thread.reset();

        m_reactor.reset_reactor_event_loop();

        //flush remaining data
        ProcessAudioQueues(true);
    }

    m_codec = AudioCodec();
    m_audio_queue.clear();
    m_user_queue.clear();
}

bool AudioMuxer::RegisterMuxCallback(const teamtalk::AudioCodec& codec,
                                     audiomuxer_callback_t cb)
{
    if (m_muxcallback)
        return false;

    m_muxcallback = cb;

    if (!Init(codec))
    {
        m_muxcallback = {};
        return false;
    }
    return true;
}

void AudioMuxer::UnregisterMuxCallback()
{
    m_muxcallback = {};

    if (!FileActive())
        StopThread();
}

bool AudioMuxer::FileActive()
{
    bool fileactive = false;

    fileactive |= bool(m_wavefile);

#if defined(ENABLE_MEDIAFOUNDATION)
    fileactive |= bool(m_mp3encoder);
#endif

#if defined(ENABLE_SPEEXFILE)
    fileactive |= bool(m_speexfile);
#endif

#if defined(ENABLE_OPUSFILE)
    fileactive |= bool(m_opusfile);
#endif

    return fileactive;
}

bool AudioMuxer::SaveFile(const teamtalk::AudioCodec& codec,
                          const ACE_TString& filename,
                          teamtalk::AudioFileFormat aff)
{
    if (FileActive() || !Init(codec))
        return false;

    int samplerate = GetAudioCodecSampleRate(m_codec);
    int channels = GetAudioCodecChannels(m_codec);
    if (GetAudioCodecSimulateStereo(m_codec))
        channels = 2;

    bool success = false;
    switch(aff)
    {
    case AFF_WAVE_FORMAT :
        m_wavefile.reset(new WavePCMFile());
        success = m_wavefile->NewFile(filename, samplerate, channels);
        if (!success)
            m_wavefile.reset();
        break;
    case AFF_MP3_16KBIT_FORMAT :
    case AFF_MP3_32KBIT_FORMAT :
    case AFF_MP3_64KBIT_FORMAT :
    case AFF_MP3_128KBIT_FORMAT :
    case AFF_MP3_256KBIT_FORMAT :
    {
#if defined(ENABLE_MEDIAFOUNDATION)
        int mp3bitrate = AFFToMP3Bitrate(aff);
        media::AudioFormat fmt(samplerate, channels);
        m_mp3encoder = MFTransform::CreateMP3(fmt, mp3bitrate, filename.c_str());
        success = bool(m_mp3encoder);
#endif
    }
    break;
    case AFF_CHANNELCODEC_FORMAT :
    {
        int bitrate = 0, maxbitrate = 0;
        bool dtx = false;
        switch(m_codec.codec)
        {
        case CODEC_SPEEX_VBR :
            bitrate = m_codec.speex_vbr.bitrate;
            maxbitrate = m_codec.speex_vbr.max_bitrate;
            dtx = m_codec.speex_vbr.dtx;
        case CODEC_SPEEX :
#if ENABLE_SPEEXFILE
            m_speexfile.reset(new SpeexEncFile());
            success = m_speexfile->Open(filename,
                                        GetSpeexBandMode(m_codec),
                                        DEFAULT_SPEEX_COMPLEXITY,
                                        (float)GetSpeexQuality(m_codec),
                                        bitrate, maxbitrate, dtx);
            if (!success)
                m_speexfile.reset();
#endif
            break;
        case CODEC_OPUS :
#if defined(ENABLE_OPUSFILE)
            m_opusfile.reset(new OpusEncFile());
            success = m_opusfile->Open(filename, channels, samplerate,
                                       GetAudioCodecFrameSize(m_codec),
                                       m_codec.opus.application);
            if (!success)
                m_opusfile.reset();
#endif
            break;
        default : // unsupported codec
            break;
        }
    }
    break; // AFF_CHANNELCODEC_FORMAT

    default : // unsupported AFF format
        break;
    }

    if (!success)
        CloseFile();

    return success;
}

void AudioMuxer::CloseFile()
{
    // write a silence block as the ending.
    std::vector<short> ending(GetAudioCodecCbTotalSamples(m_codec));
    int samples = GetAudioCodecCbSamples(m_codec);

    if (!m_muxcallback)
        StopThread();

#if defined(ENABLE_OPUSFILE)
    if (m_opusfile)
    {
        m_opusfile->Encode(&ending[0], samples, true);
        m_opusfile->Close();
        m_opusfile.reset();
    }
#endif

#if defined(ENABLE_SPEEXFILE)
    if (m_speexfile)
    {
        m_speexfile->Encode(&ending[0], true);
        m_speexfile->Close();
        m_speexfile.reset();
    }
#endif

    if (m_wavefile)
    {
        m_wavefile->Close();
        m_wavefile.reset();
    }

#if defined(ENABLE_MEDIAFOUNDATION)
    m_mp3encoder.reset();
#endif
}

void AudioMuxer::QueueUserAudio(int userid, const short* rawAudio,
                                ACE_UINT32 sample_no, bool last,
                                const teamtalk::AudioCodec& codec)
{
    QueueUserAudio(userid, rawAudio, sample_no, last,
                   GetAudioCodecCbSamples(codec),
                   GetAudioCodecChannels(codec));
}

void AudioMuxer::QueueUserAudio(int userid, const short* rawAudio,
                                ACE_UINT32 sample_no, bool last,
                                int n_samples, int n_channels)
{
    //if thread isn't running just ignore
    if (!m_thread)
        return;

    //audio must be same format as 'm_codec' but allow 'n_samples' and
    //'n_channels' to be 0 to terminate a stream
    if(GetAudioCodecCbSamples(m_codec) != n_samples && n_samples != 0)
        return;
    if(GetAudioCodecChannels(m_codec) != n_channels && n_channels != 0)
        return;

    std::unique_lock<std::recursive_mutex> g(m_mutex);

    // MYTRACE(ACE_TEXT("Add audio from #%d to audiomuxer %p. Offset %u. Samples %d\n"),
    //         userid, this, sample_no, n_samples);

    ACE_Message_Queue<ACE_MT_SYNCH>* q;
    user_audio_queue_t::iterator ii = m_audio_queue.find(userid);
    if(ii == m_audio_queue.end())
    {
        //allow buffer of one second audio
        int bytes = GetAudioCodecCbBytes(m_codec);
        int msec = GetAudioCodecCbMillis(m_codec);
        if(!msec)
            return;

        bytes = bytes * ((AUDIOBLOCK_QUEUE_MSEC / msec) + 1);
        ACE_NEW(q, ACE_Message_Queue<ACE_MT_SYNCH>());
        m_audio_queue[userid] = message_queue_t(q);
        q->high_water_mark(bytes);
    }
    else
        q = ii->second.get();

    int bytes = 0;
    if(rawAudio)
        bytes = GetAudioCodecCbBytes(m_codec);

    ACE_Message_Block* mb;
    ACE_NEW(mb, ACE_Message_Block(sizeof(AudioMuxBlock) + bytes));

    AudioMuxBlock aud;
    if(rawAudio)
        aud.audio = (short*)&mb->rd_ptr()[sizeof(aud)];
    else
        aud.audio = NULL;

    aud.sample_no = sample_no;
    aud.last = last;
    int ret = mb->copy((const char*)&aud, sizeof(aud));
    TTASSERT(ret >= 0);
    if(rawAudio)
    {
        ret = mb->copy((const char*)rawAudio, bytes);
        TTASSERT(ret >= 0);
    }

    ACE_Time_Value tm;
    if(q->enqueue(mb, &tm)<0)
    {
        MYTRACE(ACE_TEXT("Buffer depleted for user #%d AudioMuxBlock %u, is last: %s. Dropped %u bytes\n"),
                userid, sample_no, (last? ACE_TEXT("true"):ACE_TEXT("false")), (unsigned)q->message_bytes());
        q->flush();
        //insert after flush, so it will appear as a new stream
        if(q->enqueue(mb, &tm)<0)
        {
            mb->release();
            m_audio_queue.erase(userid);
        }
        //clear sample no tracker
        m_user_queue.erase(userid);
    }
}

void AudioMuxer::Run()
{
    m_reactor.owner (ACE_OS::thr_self ());

    const time_t MUX_INTERVAL_MSEC = AUDIOBLOCK_QUEUE_MSEC / 3;
    auto tv = ACE_Time_Value(MUX_INTERVAL_MSEC/1000, (MUX_INTERVAL_MSEC % 1000) * 1000);

    TimerHandler th(*this, 577);
    long timerid = m_reactor.schedule_timer(&th, 0, tv, tv);
    TTASSERT(timerid >= 0);
    m_reactor.run_reactor_event_loop ();

    if (timerid >= 0)
    {
        int ret = m_reactor.cancel_timer(timerid);
        TTASSERT(ret >= 0);
    }
}

int AudioMuxer::TimerEvent(ACE_UINT32 timer_event_id, long userdata)
{
    ProcessAudioQueues(false);
    return 0;
}

void AudioMuxer::ProcessAudioQueues(bool flush)
{
    //Make map of userid -> sample no.
    //Check if all expected audio is there (from every user)
    //If there -> mux
    //Else wait until next round (what if dead??)

    ACE_UINT32 now = GETTIMESTAMP();
    ACE_UINT32 diff = now - m_last_flush_time;

    int cb_msec = GetAudioCodecCbMillis(m_codec);
    int cb_samples = GetAudioCodecCbSamples(m_codec);
    TTASSERT(cb_msec>0);
    TTASSERT(cb_samples>0);
    if(!cb_msec || !cb_samples)
        return;

    int cb_count = (int)diff / cb_msec;
    int remain_msec = (int)diff % cb_msec;
    while(cb_count)
    {
        if(CanMuxUserAudio())
            MuxUserAudio(); //write muxed audio
        else
        {
            std::unique_lock<std::recursive_mutex> g(m_mutex);
            if(m_audio_queue.empty())
            {
                //write silence
                short zero = 0;
                m_muxed_audio.assign(m_muxed_audio.size(), zero);
                //MYTRACE(ACE_TEXT("No audio to mux at %u. Writing %d msec silence\n"),
                //        now, cb_msec);
            }
            else //no data has arrived in time
                break;
        }

        WriteAudio(cb_samples);
        cb_count--;
    }

    if (flush)
    {
        RemoveEmptyMuxUsers();

        while (CanMuxUserAudio())
        {
            MuxUserAudio();
            WriteAudio(cb_samples);
            RemoveEmptyMuxUsers();
        }
    }

    // MYTRACE(ACE_TEXT("Queued %d msec at %u\n"), (cb_count * cb_msec) - remain_msec, now);
    m_last_flush_time = now - ((cb_count * cb_msec) + remain_msec);
}

bool AudioMuxer::CanMuxUserAudio()
{
    std::unique_lock<std::recursive_mutex> g(m_mutex);

    user_audio_queue_t::iterator ii = m_audio_queue.begin();
    while(ii != m_audio_queue.end())
    {
        if(ii->second->is_empty())
            return false;
        ii++;
    }
    return m_audio_queue.size();
}

void AudioMuxer::RemoveEmptyMuxUsers()
{
    std::unique_lock<std::recursive_mutex> g(m_mutex);

    // get rid of users who haven't supplied data in time for
    // flush
    for (auto i = m_audio_queue.begin();i != m_audio_queue.end();)
    {
        if (i->second->is_empty())
        {
            MYTRACE(ACE_TEXT("AudioMuxer removed empty audio queue for #%d\n"), i->first);
            i = m_audio_queue.erase(i);
        }
        else
        {
            MYTRACE(ACE_TEXT("AudioMuxer still has audio queue for #%d. Items: %d\n"),
                    i->first, int(i->second->message_count()));
            ++i;
        }
    }
}

bool AudioMuxer::MuxUserAudio()
{
    std::unique_lock<std::recursive_mutex> g(m_mutex);

    TTASSERT(m_audio_queue.size());
    TTASSERT(m_muxed_audio.size());

    const int SAMPLES = GetAudioCodecCbSamples(m_codec);
    std::vector<ACE_Message_Block*> audio_blocks;
    user_audio_queue_t::iterator ii = m_audio_queue.begin();
    while(ii != m_audio_queue.end())
    {
        ACE_Message_Block* mb;
        ACE_Time_Value tm;
        if(ii->second->peek_dequeue_head(mb, &tm) >= 0)
        {
            AudioMuxBlock* aud = reinterpret_cast<AudioMuxBlock*>(mb->rd_ptr());

            //ensure it's the AudioMuxBlock we're expecting
            user_queued_audio_t::iterator ui = m_user_queue.find(ii->first);
            if(ui != m_user_queue.end())
            {
                TTASSERT(W32_GEQ(aud->sample_no, ui->second));
                if (W32_GT(aud->sample_no, ui->second + SAMPLES))
                {
                    MYTRACE(ACE_TEXT("Missing audio block from #%d. Got %u, expected %u\n"),
                            ii->first, aud->sample_no, ui->second + SAMPLES);
                    m_user_queue[ii->first] = ui->second + SAMPLES;
                    ii++;
                    continue; //skip it
                }
                else if (W32_LT(aud->sample_no, ui->second + SAMPLES))
                {
                    MYTRACE(ACE_TEXT("Got delayed audio block from #%d. Contains %u, expected %u. Dropping user.\n"),
                            ii->first, aud->sample_no, ui->second + SAMPLES);
                    //this should never happen - clear user
                    m_user_queue.erase(ii->first);
                    m_audio_queue.erase(ii++);
                    continue;
                }
            }
            //got the right audio block
            tm = ACE_Time_Value::zero;
            if(ii->second->dequeue(mb, &tm)<0)
            {
                TTASSERT(0);//this should never happen, since we already peeked.
                ii++;
                continue;
            }
            m_user_queue[ii->first] = aud->sample_no;
            if(aud->audio == NULL)
            {
                if(aud->last) //no more audio will come, so remove
                {
                    m_user_queue.erase(ii->first);
                    m_audio_queue.erase(ii++);
                }
                else ii++;

                mb->release();
            }
            else
            {
                audio_blocks.push_back(mb);
                ii++;
            }
        }
        else
        {
            //Something is wrong. There should be data for all users otherwise
            //CanMuxUserAudio() should have returned false.
            TTASSERT(0);
            m_audio_queue.clear();
            for(size_t i=0;i<audio_blocks.size();i++)
                audio_blocks[i]->release();
            return false;
        }
    }

    g.unlock(); //don't touch 'm_audio_queue' after this!

    if(audio_blocks.empty())
    {
        short zero = 0;
        m_muxed_audio.assign(m_muxed_audio.size(), zero);
    }
    else
    {
        AudioMuxBlock* aud = reinterpret_cast<AudioMuxBlock*>(audio_blocks[0]->rd_ptr());
        TTASSERT((int)m_muxed_audio.size() ==
                 GetAudioCodecCbSamples(m_codec) * GetAudioCodecChannels(m_codec));
        TTASSERT(aud->audio);
        m_muxed_audio.assign(aud->audio, aud->audio+m_muxed_audio.size());

        //this is where we mux if there's more than one user
        if(audio_blocks.size()>1)
        {
            for(size_t i=0;i<m_muxed_audio.size();i++)
            {
                int val = m_muxed_audio[i];
                for(size_t a=1;a<audio_blocks.size();a++)
                {
                    aud = reinterpret_cast<AudioMuxBlock*>(audio_blocks[a]->rd_ptr());
                    TTASSERT(aud->audio);
                    val += aud->audio[i];
                }
                if(val > 32767)
                    m_muxed_audio[i] = 32767;
                else if(val < -32768)
                    m_muxed_audio[i] = -32768;
                else
                    m_muxed_audio[i] = val;
            }
        }

        for(size_t i=0;i<audio_blocks.size();i++)
            audio_blocks[i]->release();

        return true;
    }
    return false;
}

void AudioMuxer::WriteAudio(int cb_samples)
{
    int channels = GetAudioCodecChannels(m_codec);
    if(GetAudioCodecSimulateStereo(m_codec))
        channels = 2;
    int framesize = GetAudioCodecFrameSize(m_codec);
    int samplerate = GetAudioCodecSampleRate(m_codec);
    TTASSERT(cb_samples == GetAudioCodecFramesPerPacket(m_codec)*framesize);

    media::AudioFrame frame(media::AudioFormat(samplerate, channels),
                            &m_muxed_audio[0], cb_samples, m_sample_no);

    if (m_muxcallback)
    {
        m_muxcallback(frame);
    }

#if ENABLE_SPEEXFILE
    if(m_speexfile && framesize)
    {
        int ret = 0;
        for(int i=0;i<cb_samples/framesize && ret >= 0;i++)
        {
            ret = m_speexfile->Encode(&m_muxed_audio[i*framesize*channels]);
        }
    }
#endif

#if defined(ENABLE_OPUSFILE)
    if(m_opusfile && framesize)
    {
        int ret = 0;
        for(int i=0;i<cb_samples/framesize && ret >= 0;i++)
            ret = m_opusfile->Encode(&m_muxed_audio[i*framesize*channels], framesize);
    }
#endif

#if defined(ENABLE_MEDIAFOUNDATION)
    if(m_mp3encoder && framesize)
    {
        m_mp3encoder->ProcessAudioEncoder(frame, true);
    }
#endif

    if(m_wavefile)
        m_wavefile->AppendSamples(&m_muxed_audio[0], cb_samples);

    m_sample_no += cb_samples;
}

ChannelAudioMuxer::ChannelAudioMuxer()
{
}

ChannelAudioMuxer::~ChannelAudioMuxer()
{
    assert(m_userchan.empty());
}

bool ChannelAudioMuxer::SaveFile(int channelid, const teamtalk::AudioCodec& codec,
                                 const ACE_TString& filename,
                                 teamtalk::AudioFileFormat aff)
{
    std::unique_lock<std::recursive_mutex> g(m_mutex);

    if (m_muxers.find(channelid) != m_muxers.end())
        return false;

    audiomuxer_t muxer(new AudioMuxer());
    bool ret = muxer->SaveFile(codec, filename, aff);
    if (!ret)
        return false;

    m_muxers[channelid] = muxer;
    return ret;
}

bool ChannelAudioMuxer::CloseFile(int channelid)
{
    audiomuxer_t muxer;
    {
        std::unique_lock<std::recursive_mutex> g(m_mutex);
        auto mi = m_muxers.find(channelid);
        if (mi != m_muxers.end())
        {
            muxer = mi->second;
            m_muxers.erase(mi);
        }
    }
    return muxer.get();
}

bool ChannelAudioMuxer::AddUser(int userid, int channelid)
{
    std::unique_lock<std::recursive_mutex> g(m_mutex);
    assert(m_userchan.find(userid) == m_userchan.end());

    if (m_userchan.find(userid) == m_userchan.end())
    {
        m_userchan[userid] = channelid;
        return true;
    }
    return false;
}

bool ChannelAudioMuxer::RemoveUser(int userid)
{
    std::unique_lock<std::recursive_mutex> g(m_mutex);
    return m_userchan.erase(userid);
}

void ChannelAudioMuxer::QueueUserAudio(int userid, const short* rawAudio,
                                       ACE_UINT32 sample_no, bool last,
                                       int n_samples, int n_channels)
{
    audiomuxer_t chanmuxer, fixedmuxer;
    {
        std::unique_lock<std::recursive_mutex> g(m_mutex);
        if (m_userchan.find(userid) != m_userchan.end())
        {
            int chanid = m_userchan[userid];
            
            // check for active channel recording
            auto im = m_muxers.find(chanid);
            if (im != m_muxers.end())
                chanmuxer = im->second;
        }

        // 'fixedmuxer' is audio codec specified recorder
        auto ifm = m_muxers.find(FIXED_AUDIOCODEC_CHANNELID);
        if (ifm != m_muxers.end())
            fixedmuxer = ifm->second;
    }

    if (chanmuxer)
        chanmuxer->QueueUserAudio(userid, rawAudio, sample_no,
                                  last, n_samples, n_channels);

    if (fixedmuxer)
        fixedmuxer->QueueUserAudio(userid, rawAudio, sample_no,
                                   last, n_samples, n_channels);
}
