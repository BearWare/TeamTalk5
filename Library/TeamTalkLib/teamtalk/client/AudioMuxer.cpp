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

#define DEBUG_AUDIOMUXER 0

using namespace teamtalk;

uint32_t GenKey(int userid, teamtalk::StreamType streamtype)
{
    assert(userid < 0x10000);
    assert((streamtype & 0xffff0000) == 0);
    return (userid << 16) | streamtype;
}

AudioMuxer::AudioMuxer(teamtalk::StreamTypes sts)
    : m_streamtypes(sts)
{
    m_mux_interval = ToTimeValue(AUDIOBLOCK_QUEUE_MSEC / 3);
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

bool AudioMuxer::QueueUserAudio(int userid, teamtalk::StreamType st,
                                const media::AudioFrame& frm)
{
    //if thread isn't running just ignore
    if (!m_thread)
        return false;

    // check that this stream type has been selected for muxing
    if ((m_streamtypes & st) == teamtalk::STREAMTYPE_NONE)
        return false;

    //audio must be same format as 'm_codec' but allow 'n_samples' and
    //'n_channels' to be 0 to terminate a stream
    if (GetAudioCodecCbSamples(m_codec) != frm.input_samples && frm.input_samples != 0)
    {
        return false;
    }
    if (GetAudioCodecChannels(m_codec) != frm.inputfmt.channels && frm.inputfmt.channels != 0)
    {
        return false;
    }

    assert(GetAudioCodecCbTotalSamples(m_codec) == (frm.input_samples * frm.inputfmt.channels) || frm.input_samples == 0);

    std::unique_lock<std::recursive_mutex> g(m_mutex);

    // MYTRACE(ACE_TEXT("Add audio from #%d to audiomuxer %p. Offset %u. Samples %d\n"),
    //         userid, this, sample_no, n_samples);

    int key = GenKey(userid, st);

    ACE_Message_Queue<ACE_MT_SYNCH>* q = nullptr;
    user_audio_queue_t::iterator ii = m_audio_queue.find(key);
    if(ii == m_audio_queue.end())
    {
        // setup audio buffer queue for user prior to mixing streams
        
        int bytes = GetAudioCodecCbBytes(m_codec);
        int msec = GetAudioCodecCbMillis(m_codec);
        int chans = GetAudioCodecChannels(m_codec);
        int sr = GetAudioCodecSampleRate(m_codec);
        if (!msec)
            return false;

        // bytes between each mux interval
        int buffersize = bytes * ((m_mux_interval.msec() / msec) + 1);
        // allow double of mux interval
        buffersize *= 2;
        MYTRACE_COND(DEBUG_AUDIOMUXER, ACE_TEXT("Buffer duration for user #%d, streamtype %d,  %d msec\n"),
                     userid, st, PCM16_BYTES_DURATION(buffersize, chans, sr));
        // add header size
        buffersize += (buffersize / bytes) * sizeof(media::AudioFrame);
        
        m_audio_queue[key].reset(new ACE_Message_Queue<ACE_MT_SYNCH>());
        q = m_audio_queue[key].get();
        q->high_water_mark(buffersize);
    }
    else
        q = ii->second.get();

    ACE_Message_Block* mb = AudioFrameToMsgBlock(frm);

    ACE_Time_Value tm;
    if (q->enqueue(mb, &tm) < 0)
    {
        MYTRACE(ACE_TEXT("Buffer depleted for user #%d AudioMuxBlock %u, streamtype %d, is last: %s. Dropping queue containing %d msec, %u/%u bytes\n"),
                userid, st, frm.sample_no, (frm.input_buffer == nullptr ? ACE_TEXT("true"):ACE_TEXT("false")),
                q->message_count() * GetAudioCodecCbMillis(m_codec), unsigned(q->message_bytes()), unsigned(q->high_water_mark()));
        q->flush();
        //insert after flush, so it will appear as a new stream
        if(q->enqueue(mb, &tm)<0)
        {
            mb->release();
            m_audio_queue.erase(key);
        }
        //clear sample no tracker
        m_user_queue.erase(key);
    }
    
    return true;
}

void AudioMuxer::SetMuxInterval(int msec)
{
    // must be set prior to thread start
    assert(!m_thread);

    int cbmsec = GetAudioCodecCbMillis(m_codec);
    cbmsec = std::max(msec, cbmsec);
    m_mux_interval = ToTimeValue(cbmsec);
}

void AudioMuxer::Run()
{
    m_reactor.owner (ACE_OS::thr_self ());

    MYTRACE_COND(DEBUG_AUDIOMUXER, ACE_TEXT("AudioMuxer interval: %d msec\n"), m_mux_interval.msec());
    
    TimerHandler th(*this, 577);
    int timerid = m_reactor.schedule_timer(&th, 0, m_mux_interval, m_mux_interval);
    TTASSERT(timerid >= 0);
    m_reactor.run_reactor_event_loop ();

    if (timerid >= 0)
    {
        int ret = m_reactor.cancel_timer(timerid);
        TTASSERT(ret >= 0);
    }

    //flush remaining data
    ProcessAudioQueues(true);
}

int AudioMuxer::TimerEvent(ACE_UINT32 timer_event_id, long userdata)
{
    ProcessAudioQueues(false);
    return 0;
}

void AudioMuxer::ProcessAudioQueues(bool flush)
{
    //Make map of user -> sample no.
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
        {
            MYTRACE_COND(DEBUG_AUDIOMUXER && m_user_queue.find(ii->first) != m_user_queue.end(),
                         ACE_TEXT("User #%d has submitted no audio to AudioMuxer. Delaying muxer at sample no %u\n"),
                         ii->first, m_user_queue[ii->first]);
            MYTRACE_COND(DEBUG_AUDIOMUXER && m_user_queue.find(ii->first) == m_user_queue.end(),
                         ACE_TEXT("User #%d has submitted no audio to AudioMuxer. No sample no available\n"), ii->first);
            return false;
        }
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
            media::AudioFrame frm(mb);

            //ensure it's the AudioMuxBlock we're expecting
            user_queued_audio_t::iterator ui = m_user_queue.find(ii->first);
            if(ui != m_user_queue.end())
            {
                TTASSERT(W32_GEQ(frm.sample_no, ui->second));
                if (W32_GT(frm.sample_no, ui->second + SAMPLES))
                {
                    MYTRACE(ACE_TEXT("Missing audio block from #%d. Got %u, expected %u\n"),
                            ii->first, frm.sample_no, ui->second + SAMPLES);
                    m_user_queue[ii->first] = ui->second + SAMPLES;
                    ii++;
                    continue; //skip it
                }
                else if (W32_LT(frm.sample_no, ui->second + SAMPLES))
                {
                    MYTRACE(ACE_TEXT("Got delayed audio block from #%d. Contains %u, expected %u. Dropping user.\n"),
                            ii->first, frm.sample_no, ui->second + SAMPLES);
                    //this should never happen - clear user
                    m_user_queue.erase(ii->first);
                    m_audio_queue.erase(ii++);
                    continue;
                }
            }
            //got the right audio block
            tm = ACE_Time_Value::zero;
            if (ii->second->dequeue(mb, &tm) < 0)
            {
                TTASSERT(0);//this should never happen, since we already peeked.
                ii++;
                continue;
            }
            m_user_queue[ii->first] = frm.sample_no;
            if (frm.input_buffer == nullptr)
            {
                // remove expected sample-offset for next run
                m_user_queue.erase(ii->first);

                //stream ended from user
                if (m_audio_queue[ii->first]->is_empty())
                    m_audio_queue.erase(ii++);

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
        media::AudioFrame frm(audio_blocks[0]);
        frm.ApplyGain();
        TTASSERT((int)m_muxed_audio.size() == GetAudioCodecCbSamples(m_codec) * GetAudioCodecChannels(m_codec));
        TTASSERT(frm.input_samples);
        m_muxed_audio.assign(frm.input_buffer, frm.input_buffer + m_muxed_audio.size());

        //this is where we mux if there's more than one user
        if(audio_blocks.size()>1)
        {
            for(size_t i=0;i<m_muxed_audio.size();i++)
            {
                int val = m_muxed_audio[i];
                for(size_t a=1;a<audio_blocks.size();a++)
                {
                    media::AudioFrame mfrm(audio_blocks[a]);
                    mfrm.ApplyGain();
                    TTASSERT(mfrm.input_buffer);
                    val += mfrm.input_buffer[i];
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
        m_muxcallback(m_streamtypes, frame);
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
            ret = m_opusfile->Encode(&m_muxed_audio[i*framesize*channels], framesize, false);
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
                                 teamtalk::StreamTypes sts,
                                 const ACE_TString& filename,
                                 teamtalk::AudioFileFormat aff)
{
    std::unique_lock<std::recursive_mutex> g(m_mutex);

    if (m_muxers.find(channelid) != m_muxers.end())
        return false;

    audiomuxer_t muxer(new AudioMuxer(sts));
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

bool ChannelAudioMuxer::AddUser(int userid, teamtalk::StreamType st, int channelid)
{
    std::unique_lock<std::recursive_mutex> g(m_mutex);
    int key = GenKey(userid, st);

    assert(m_userchan.find(key) == m_userchan.end());

    if (m_userchan.find(key) == m_userchan.end())
    {
        m_userchan[key] = channelid;
        return true;
    }
    return false;
}

bool ChannelAudioMuxer::RemoveUser(int userid, teamtalk::StreamType st)
{
    std::unique_lock<std::recursive_mutex> g(m_mutex);
    int key = GenKey(userid, st);
    return m_userchan.erase(key);
}

void ChannelAudioMuxer::QueueUserAudio(int userid, teamtalk::StreamType st,
                                       const media::AudioFrame& frm)
{
    audiomuxer_t chanmuxer, fixedmuxer;
    {
        int key = GenKey(userid, st);

        std::unique_lock<std::recursive_mutex> g(m_mutex);
        if (m_userchan.find(key) != m_userchan.end())
        {
            int chanid = m_userchan[key];
            
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
        chanmuxer->QueueUserAudio(userid, st, frm);

    if (fixedmuxer)
        fixedmuxer->QueueUserAudio(userid, st, frm);
}
