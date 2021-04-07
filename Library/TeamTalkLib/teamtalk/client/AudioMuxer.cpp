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

#include <cstring>

#define AUDIOBLOCK_QUEUE_MSEC 1000

#define DEBUG_AUDIOMUXER 0

using namespace teamtalk;

uint32_t GenKey(int userid, teamtalk::StreamType streamtype)
{
    assert(userid < 0x10000);
    assert((streamtype & 0xffff0000) == 0);
    return (userid << 16) | streamtype;
}

int GetUserID(uint32_t key)
{
    return key >> 16;
}

teamtalk::StreamType GetStreamType(uint32_t key)
{
    return teamtalk::StreamType(key & 0xffff);
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

    m_muxed_buffer.resize(samples);

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
    m_usermux_queue.clear();
    m_usermux_progress.clear();
    m_userresample_queue.Reset();
    while (m_userresample_block.size())
    {
        m_userresample_block.begin()->second->release();
        m_userresample_block.erase(m_userresample_block.begin());
    }
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

void AudioMuxer::RegisterMuxTick(audiomuxer_tick_t cb)
{
    assert(m_codec.codec == CODEC_NO_CODEC);
    m_tickcallback = cb;
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
    TTASSERT((frm.input_buffer == nullptr && frm.input_samples == 0) || (frm.input_buffer && frm.input_samples));

    //if thread isn't running just ignore
    if (!m_thread)
        return false;

    // check that this stream type has been selected for muxing
    if ((m_streamtypes & st) == teamtalk::STREAMTYPE_NONE)
        return false;

    std::unique_lock<std::recursive_mutex> g(m_mutex);

    int key = GenKey(userid, st);

    if (GetAudioCodecCbSamples(m_codec) != frm.input_samples ||
        GetAudioCodecAudioFormat(m_codec) != frm.inputfmt ||
        m_userresample_queue.Exists(userid, st))
    {
        // invalid audio format or 0 samples can also mean stream terminator
        if (frm.input_samples > 0)
        {
            if (!m_userresample_queue.Exists(userid, st))
            {
                m_userresample_queue.AddAudioSource(userid, st, GetAudioCodecAudioFormat(m_codec));
                // allocate queue so muxer will wait for audio source to provide enough samples
                GetMuxQueue(key);
            }
        }

        if (m_userresample_queue.Exists(userid, st))
        {
            if (!m_userresample_queue.AddAudio(userid, st, frm))
            {
                MYTRACE_COND(DEBUG_AUDIOMUXER, ACE_TEXT("AudioMuxer failed to queue resample audio from #%d streamtype %d. Resetting.\n"),
                             userid, st);
                m_userresample_queue.RemoveAudioSource(userid, st);

                SubmitMuxAudioFrame(key, media::AudioFrame());
            }

            return true;
        }

        // only accept terminator
        assert(frm.input_samples == 0);
        assert(frm.input_buffer == nullptr);
    }

    SubmitMuxAudioFrame(key, frm);
    
    return true;
}

AudioMuxer::message_queue_t AudioMuxer::GetMuxQueue(int key)
{
    user_audio_queue_t::iterator ii = m_usermux_queue.find(key);
    if (ii == m_usermux_queue.end())
    {
        // setup audio buffer queue for user prior to mixing streams

        int bytes = GetAudioCodecCbBytes(m_codec);
        int msec = GetAudioCodecCbMillis(m_codec);
        assert(msec > 0);

        // bytes between each mux interval
        int buffersize = bytes * ((m_mux_interval.msec() / msec) + 1);
        // allow double of mux interval
        buffersize *= 2;
        // add header size
        buffersize += (buffersize / bytes) * sizeof(media::AudioFrame);

        m_usermux_queue[key].reset(new ACE_Message_Queue<ACE_MT_SYNCH>());
        auto q = m_usermux_queue[key];
        q->high_water_mark(buffersize);
        q->low_water_mark(buffersize);
    }

    return m_usermux_queue[key];
}

void AudioMuxer::SubmitMuxAudioFrame(int key, const media::AudioFrame& frm)
{
    assert(GetAudioCodecCbTotalSamples(m_codec) == (frm.input_samples * frm.inputfmt.channels) || frm.input_samples == 0);

    // MYTRACE(ACE_TEXT("Add audio from #%d to audiomuxer %p. Offset %u. Samples %d\n"),
    //         userid, this, sample_no, n_samples);

    message_queue_t q = GetMuxQueue(key);

    MYTRACE_COND(DEBUG_AUDIOMUXER, ACE_TEXT("Buffer size for user #%d, streamtype %d, %d/%d bytes\n"),
                 GetUserID(key), GetStreamType(key), int(q->message_length()), int(q->high_water_mark()));

    ACE_Message_Block* mb = AudioFrameToMsgBlock(frm);

    ACE_Time_Value tm;
    if (q->enqueue(mb, &tm) < 0)
    {
        MYTRACE_COND(DEBUG_AUDIOMUXER, ACE_TEXT("Buffer full for user #%d streamtype %d, sampleindex %u, is last: %s. Dropping queue containing %d msec, %u/%u bytes\n"),
                GetUserID(key), GetStreamType(key), frm.sample_no, (frm.input_samples == 0 ? ACE_TEXT("true"):ACE_TEXT("false")),
                q->message_count() * GetAudioCodecCbMillis(m_codec), unsigned(q->message_bytes()), unsigned(q->high_water_mark()));
        q->flush();
        TTASSERT(q->message_count() == 0);
        //insert after flush, so it will appear as a new stream
        tm = ACE_Time_Value::zero;
        if (q->enqueue(mb, &tm) < 0)
        {
            mb->release();
            m_usermux_queue.erase(key);
        }
        //clear sample no tracker
        m_usermux_progress.erase(key);
    }
    else
    {
        MYTRACE_COND(DEBUG_AUDIOMUXER, ACE_TEXT("Submitted #%d streamtype: %d from sample index %u, samples %d\n"),
                     GetUserID(key), GetStreamType(key), frm.sample_no, frm.input_samples);
    }
}

void AudioMuxer::ProcessResampleAudio()
{
    std::vector<int> keys;
    // store keys since we erase 'm_usermux_queue' elements during processing
    auto ii = m_usermux_queue.begin();
    for (;ii != m_usermux_queue.end();++ii)
        keys.push_back(ii->first);

    for (auto key : keys)
    {
        int userid = GetUserID(key);
        teamtalk::StreamType st = GetStreamType(key);

        // extract all AudioFrames from user w. StreamType starting with
        // the one in 'm_userresample_block'
        std::vector<ACE_Message_Block*> mbs;
        if (m_userresample_block.find(key) != m_userresample_block.end())
        {
            MYTRACE_COND(DEBUG_AUDIOMUXER, ACE_TEXT("Resuming #%d streamtype: %d from sample index %u\n"),
                         userid, st, media::AudioFrame(m_userresample_block[key]).sample_no);
            mbs.push_back(m_userresample_block[key]);
        }
        m_userresample_block.erase(key);

        ACE_Message_Block* mb;
        while ((mb = m_userresample_queue.AcquireAudioFrame(userid, st)))
            mbs.push_back(mb);

        // Submit as many "mux-ready" frames as possible
        while ((mb = BuildMuxAudioFrame(mbs)))
        {
            MBGuard g(mb);
            media::AudioFrame frm(mb);
            MYTRACE_COND(DEBUG_AUDIOMUXER, ACE_TEXT("Submitting #%d streamtype: %d from sample index %u\n"),
                         userid, st, frm.sample_no);
            SubmitMuxAudioFrame(key, frm);
        }

        // If any AudioBlocks remain then store one AudioFrame in 'm_userresample_block'
        if (mbs.size() == 1)
        {
            m_userresample_block[key] = mbs[0];
            mbs.clear();
            MYTRACE_COND(DEBUG_AUDIOMUXER, ACE_TEXT("Store #%d streamtype: %d from sample index %u\n"),
                         userid, st, media::AudioFrame(m_userresample_block[key]).sample_no);
        }
        else if (mbs.size() > 1)
        {
            mb = AudioFramesMerge(mbs);
            for (auto m : mbs)
                m->release();
            mbs.clear();
            m_userresample_block[key] = mb;
            MYTRACE_COND(DEBUG_AUDIOMUXER, ACE_TEXT("Store jumbo #%d streamtype: %d from sample index %u\n"),
                         userid, st, media::AudioFrame(m_userresample_block[key]).sample_no);
        }
    }
}

ACE_Message_Block* AudioMuxer::BuildMuxAudioFrame(std::vector<ACE_Message_Block*>& mbs)
{
    if (mbs.empty())
        return nullptr;

    // check if it's a terminator frame
    if (media::AudioFrame(mbs[0]).input_samples == 0)
    {
        ACE_Message_Block* mb = mbs[0];
        mbs.erase(mbs.begin());
        return mb;
    }

    return AudioFrameFromList(GetAudioCodecCbSamples(m_codec), mbs);
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
        {
            std::unique_lock<std::recursive_mutex> g(m_mutex);

            // first process invalid format audio frames (i.e. different from 'm_codec')
            ProcessResampleAudio();

            if(CanMuxUserAudio())
                MuxUserAudio(); //write muxed audio
            else
            {
                if (m_usermux_queue.empty())
                {
                    //write silence
                    short zero = 0;
                    m_muxed_buffer.assign(m_muxed_buffer.size(), zero);
                    //MYTRACE(ACE_TEXT("No audio to mux at %u. Writing %d msec silence\n"),
                    //        now, cb_msec);
                }
                else //no data has arrived in time
                    break;
            }
        } // scope so we don't hold lock during callback

        WriteAudio(cb_samples);
        cb_count--;
    }

    if (flush)
    {
        while (true)
        {
            {
                std::unique_lock<std::recursive_mutex> g(m_mutex);
                RemoveEmptyMuxUsers();

                if (CanMuxUserAudio())
                    MuxUserAudio();
                else
                    break;
            }
            WriteAudio(cb_samples);
        }
    }

    // MYTRACE(ACE_TEXT("Queued %d msec at %u\n"), (cb_count * cb_msec) - remain_msec, now);
    m_last_flush_time = now - ((cb_count * cb_msec) + remain_msec);

    if (m_tickcallback)
        m_tickcallback(m_streamtypes, m_sample_no);
}

bool AudioMuxer::CanMuxUserAudio()
{
    user_audio_queue_t::iterator ii = m_usermux_queue.begin();
    while(ii != m_usermux_queue.end())
    {
        if(ii->second->is_empty())
        {
            MYTRACE_COND(DEBUG_AUDIOMUXER && m_usermux_progress.find(ii->first) != m_usermux_progress.end(),
                         ACE_TEXT("User #%d has submitted no audio to AudioMuxer. Delaying muxer...\n"),
                         GetUserID(ii->first));
            MYTRACE_COND(DEBUG_AUDIOMUXER && m_usermux_progress.find(ii->first) == m_usermux_progress.end(),
                         ACE_TEXT("User #%d has submitted no audio to AudioMuxer. No sample no available\n"), GetUserID(ii->first));
            return false;
        }
        ii++;
    }
    return m_usermux_queue.size();
}

void AudioMuxer::RemoveEmptyMuxUsers()
{
    // get rid of users who haven't supplied data in time for
    // flush
    for (auto i = m_usermux_queue.begin();i != m_usermux_queue.end();++i)
    {
        if (i->second->is_empty())
        {
            MYTRACE(ACE_TEXT("AudioMuxer removed empty audio queue for #%d\n"), GetUserID(i->first));
            SubmitMuxAudioFrame(i->first, media::AudioFrame());
        }
        else
        {
            MYTRACE(ACE_TEXT("AudioMuxer still has audio queue for #%d. Items: %d\n"),
                    GetUserID(i->first), int(i->second->message_count()));
        }
    }
}

bool AudioMuxer::MuxUserAudio()
{
    TTASSERT(m_usermux_queue.size());
    TTASSERT(m_muxed_buffer.size());

    const int SAMPLES = GetAudioCodecCbSamples(m_codec);
    std::vector<ACE_Message_Block*> audio_blocks;
    user_audio_queue_t::iterator ii = m_usermux_queue.begin();
    while(ii != m_usermux_queue.end())
    {
        ACE_Message_Block* mb;
        ACE_Time_Value tm;
        if (ii->second->dequeue(mb, &tm) >= 0)
        {
            media::AudioFrame frm(mb);

            //ensure it's the AudioMuxBlock we're expecting
            user_muxprogress_t::iterator ui = m_usermux_progress.find(ii->first);
            if(ui != m_usermux_progress.end() && frm.input_samples > 0)
            {
                MYTRACE_COND(frm.sample_no != ui->second + SAMPLES,
                             ACE_TEXT("Unexpected sample no for #%d streamtype %u. Found sample index %u. Should be %u\n"),
                             GetUserID(ii->first), GetStreamType(ii->first), frm.sample_no, ui->second + SAMPLES);
                TTASSERT(frm.sample_no == ui->second + SAMPLES);

                if (frm.sample_no != ui->second + SAMPLES)
                {
                    //this should never happen - clear user
                    m_usermux_progress.erase(ui);
                    m_usermux_queue.erase(ii++);
                    continue;
                }
            }

            MYTRACE_COND(DEBUG_AUDIOMUXER && ui != m_usermux_progress.end(),
                         ACE_TEXT("Processing #%d streamtype %d sampleindex: %u\n"),
                         GetUserID(ii->first), GetStreamType(ii->first), frm.sample_no);

            if (frm.input_samples == 0)
            {
                MYTRACE_COND(DEBUG_AUDIOMUXER, ACE_TEXT("Clearing #%d streamtype %d, sampleindex %u\n"),
                             GetUserID(ii->first), GetStreamType(ii->first), frm.sample_no);

                // remove expected sample-offset for next run
                m_usermux_progress.erase(ii->first);

                //stream ended from user
                if (m_usermux_queue[ii->first]->is_empty())
                    m_usermux_queue.erase(ii++);

                mb->release();
            }
            else
            {
                MYTRACE_COND(DEBUG_AUDIOMUXER, ACE_TEXT("Adding #%d streamtype %d, sampleindex %u\n"),
                             GetUserID(ii->first), GetStreamType(ii->first), frm.sample_no);
                m_usermux_progress[ii->first] = frm.sample_no;
                audio_blocks.push_back(mb);
                ++ii;
            }
        }
        else
        {
            //Something is wrong. There should be data for all users otherwise
            //CanMuxUserAudio() should have returned false.
            TTASSERT(0);
            m_usermux_queue.clear();
            m_usermux_progress.clear();
            for(size_t i=0;i<audio_blocks.size();i++)
                audio_blocks[i]->release();
            return false;
        }
    }

    TTASSERT(int(m_muxed_buffer.size()) == GetAudioCodecCbTotalSamples(m_codec));

    if(audio_blocks.empty())
    {
        short zero = 0;
        m_muxed_buffer.assign(m_muxed_buffer.size(), zero);
    }
    else
    {
        media::AudioFrame frm(audio_blocks[0]);
        frm.ApplyGain();
        TTASSERT(frm.input_samples);
        TTASSERT(frm.input_samples == GetAudioCodecCbSamples(m_codec));
        m_muxed_buffer.assign(frm.input_buffer, frm.input_buffer + m_muxed_buffer.size());

        //this is where we mux if there's more than one user
        if(audio_blocks.size()>1)
        {
            for(size_t a=1;a<audio_blocks.size();a++)
            {
                media::AudioFrame mfrm(audio_blocks[a]);
                mfrm.ApplyGain();
                TTASSERT(mfrm.input_buffer);
                TTASSERT(mfrm.input_samples == GetAudioCodecCbSamples(m_codec));
                for(size_t i=0;i<m_muxed_buffer.size();i++)
                {
                    int val = int(m_muxed_buffer[i]) + mfrm.input_buffer[i];
                    if(val > 32767)
                        m_muxed_buffer[i] = 32767;
                    else if(val < -32768)
                        m_muxed_buffer[i] = -32768;
                    else
                        m_muxed_buffer[i] = val;
                }
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
                            &m_muxed_buffer[0], cb_samples, m_sample_no);

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
            ret = m_speexfile->Encode(&m_muxed_buffer[i*framesize*channels]);
        }
    }
#endif

#if defined(ENABLE_OPUSFILE)
    if(m_opusfile && framesize)
    {
        int ret = 0;
        for(int i=0;i<cb_samples/framesize && ret >= 0;i++)
            ret = m_opusfile->Encode(&m_muxed_buffer[i*framesize*channels], framesize, false);
    }
#endif

#if defined(ENABLE_MEDIAFOUNDATION)
    if(m_mp3encoder && framesize)
    {
        m_mp3encoder->ProcessAudioEncoder(frame, true);
    }
#endif

    if(m_wavefile)
        m_wavefile->AppendSamples(&m_muxed_buffer[0], cb_samples);

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
