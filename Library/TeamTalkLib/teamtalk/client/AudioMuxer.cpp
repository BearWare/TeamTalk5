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

#include "mystd/MyStd.h"
#include "teamtalk/CodecCommon.h"
#include "teamtalk/TTAssert.h"
#include "myace/MyACE.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <set>
#include <utility>

constexpr auto DEBUG_AUDIOMUXER = 0;

using namespace teamtalk;

static uint32_t GenKey(int userid, teamtalk::StreamType streamtype)
{
    TTASSERT(userid < 0x10000);
    TTASSERT((streamtype & 0xffff0000) == 0);
    return (userid << 16) | streamtype;
}

static int GetUserID(uint32_t key)
{
    return key >> 16;
}

static teamtalk::StreamType GetStreamType(uint32_t key)
{
    return teamtalk::StreamType(key & 0xffff);
}

AudioMuxer::AudioMuxer(teamtalk::StreamTypes sts)
    : m_streamtypes(sts)
{
}

AudioMuxer::~AudioMuxer()
{
    StopThread();
    MYTRACE(TTEXT("~AudioMuxer()\n"));
}

bool AudioMuxer::Init(const media::AudioInputFormat& fmt)
{
    if (m_inputformat == fmt)
        return true;

    return StartThread(fmt);
}

bool AudioMuxer::StartThread(const media::AudioInputFormat& fmt)
{
    TTASSERT(m_inputformat == media::AudioInputFormat());

    TTASSERT(!m_thread);
    if (m_thread)
        return false;

    //mux interval
    if (!fmt.IsValid())
        return false;

    m_muxed_buffer.assign(fmt.GetTotalSamples(), static_cast<short>(0));

    m_inputformat = fmt;

    SetMuxInterval(fmt.GetDurationMSec());

    m_sample_no = 0;
    m_last_flush_time = GETTIMESTAMP();

    MYTRACE(TTEXT("Starting AudioMuxer with sample rate %d, channels %d and callback %d\n"),
            fmt.fmt.samplerate, fmt.fmt.channels, fmt.samples);

    m_thread = std::make_shared<std::thread>(&AudioMuxer::Run, this);
    return true;
}

void AudioMuxer::StopThread()
{
    if (m_thread)
    {
        int ret = 0;
        ret = m_reactor.end_reactor_event_loop();
        TTASSERT(ret >= 0);

        m_thread->join();
        m_thread.reset();

        m_reactor.reset_reactor_event_loop();
    }

    m_inputformat = media::AudioInputFormat();
    m_usermux_queue.clear();
    m_usermux_progress.clear();
    m_preprocess_queue.Reset();
    while (!m_preprocess_block.empty())
    {
        m_preprocess_block.begin()->second = nullptr;
        m_preprocess_block.erase(m_preprocess_block.begin());
    }
}

bool AudioMuxer::RegisterMuxCallback(const media::AudioInputFormat& fmt,
                                     audiomuxer_callback_t cb)
{
    if (m_muxcallback)
        return false;

    m_muxcallback = std::move(cb);

    if (!Init(fmt))
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
    assert(!m_inputformat.IsValid());
    m_tickcallback = std::move(cb);
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
                          const tstring& filename,
                          teamtalk::AudioFileFormat aff)
{
    if (FileActive() || !Init(GetAudioCodecAudioInputFormat(codec)))
        return false;

    int const samplerate = GetAudioCodecSampleRate(codec);
    int channels = GetAudioCodecChannels(codec);
    if (GetAudioCodecSimulateStereo(codec))
        channels = 2;

    bool success = false;
    switch(aff)
    {
    case AFF_WAVE_FORMAT :
        m_wavefile = std::make_shared<WavePCMFile>();
        success = m_wavefile->NewFile(filename, samplerate, channels);
        if (!success)
            m_wavefile.reset();
        break;
    case AFF_MP3_16KBIT_FORMAT :
    case AFF_MP3_32KBIT_FORMAT :
    case AFF_MP3_64KBIT_FORMAT :
    case AFF_MP3_128KBIT_FORMAT :
    case AFF_MP3_256KBIT_FORMAT :
    case AFF_MP3_320KBIT_FORMAT :
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
        int bitrate = 0;
        int maxbitrate = 0;
        bool dtx = false;
        switch(codec.codec)
        {
        case CODEC_SPEEX_VBR :
            bitrate = codec.speex_vbr.bitrate;
            maxbitrate = codec.speex_vbr.max_bitrate;
            dtx = codec.speex_vbr.dtx;
        case CODEC_SPEEX :
#if defined(ENABLE_SPEEXFILE)
            m_speexfile = std::make_shared<SpeexEncFile>();
            success = m_speexfile->Open(filename,
                                        GetSpeexBandMode(codec),
                                        DEFAULT_SPEEX_COMPLEXITY,
                                        static_cast<float>(GetSpeexQuality(codec)),
                                        bitrate, maxbitrate, dtx);
            if (!success)
                m_speexfile.reset();
#endif
            break;
        case CODEC_OPUS :
#if defined(ENABLE_OPUSFILE)
            m_opusfile = std::make_shared<OpusEncFile>();
            success = m_opusfile->Open(filename, channels, samplerate,
                                       GetAudioCodecFrameSize(codec),
                                       codec.opus.application);
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
    std::vector<short> ending(m_inputformat.GetTotalSamples());
    int const samples = m_inputformat.samples;

    if (!m_muxcallback)
        StopThread();

#if defined(ENABLE_OPUSFILE)
    if (m_opusfile)
    {
        m_opusfile->Encode(ending.data(), samples, true);
        m_opusfile->Close();
        m_opusfile.reset();
    }
#endif

#if defined(ENABLE_SPEEXFILE)
    if (m_speexfile)
    {
        m_speexfile->Encode(ending.data(), true);
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

    // media::AudioFrame.userdata must contain StreamType
    assert(frm.userdata == st || frm.userdata == teamtalk::STREAMTYPE_NONE);

    int const key = GenKey(userid, st);

    std::unique_lock<std::recursive_mutex> const g(m_mutex1_preprocess);
    
    if (!m_preprocess_queue.Exists(userid, st))
    {
        MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("Adding audio source #%d, streamtype: 0x%x\n"),
            GetUserID(key), GetStreamType(key));
        m_preprocess_queue.AddAudioSource(userid, st, m_inputformat.fmt);

        // allocate queue so muxer will wait for audio source to provide enough samples
        std::unique_lock<std::recursive_mutex> const g(m_mutex2_mux);
        GetMuxQueue(key);
    }

    MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("Queueing #%d streamtype: 0x%x sample index %u\n"),
                 GetUserID(key), GetStreamType(key), frm.sample_no);
    if (!m_preprocess_queue.AddAudio(userid, st, frm))
    {
        MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("AudioMuxer failed to queue resample audio from #%d streamtype 0x%x. Resetting.\n"),
            userid, st);
        m_preprocess_queue.RemoveAudioSource(userid, st);

        // clear as mux source
        std::unique_lock<std::recursive_mutex> const g(m_mutex2_mux);
        SubmitMuxAudioFrame(key, media::AudioFrame());
    }

    return true;
}

AudioMuxer::message_queue_t AudioMuxer::GetMuxQueue(int key)
{
    auto const ii = m_usermux_queue.find(key);
    if (ii == m_usermux_queue.end())
    {
        // setup audio buffer queue for user prior to mixing streams
        int const msec = m_inputformat.GetDurationMSec();
        assert(msec > 0);

        // Set mux buffer to 1 second as maximum.
        //
        // If 'm_preprocess_queue' contains more than 1 second of audio
        // then it will cause the AudioMuxer to overflow its buffer.
        // Therefore the transmit-interval must never exceed 1 second.
        int MUXBUFFER_MAX = media::AudioInputFormat(m_inputformat.fmt,
                                                    m_inputformat.fmt.samplerate).GetBytes();
        int const n_frames = (int(m_mux_interval.count()) / msec) + 1;
        MUXBUFFER_MAX += sizeof(media::AudioFrame) * n_frames;

        m_usermux_queue[key] = std::make_shared<MsgQueue>();
        auto q = m_usermux_queue[key];

        q->SetWaterMark(MUXBUFFER_MAX);
    }

    return m_usermux_queue[key];
}

int AudioMuxer::SubmitMuxAudioFrame(int key, const media::AudioFrame& frm)
{
    assert((m_inputformat.fmt == frm.inputfmt && m_inputformat.samples == frm.input_samples) || frm.input_samples == 0);

    // MYTRACE(TTEXT("Add audio from #%d to audiomuxer %p. Offset %u. Samples %d\n"),
    //         userid, this, sample_no, n_samples);

    message_queue_t const q = GetMuxQueue(key);

    MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("Buffer size for user #%d, streamtype 0x%x, %d/%d bytes\n"),
                 GetUserID(key), GetStreamType(key), int(q->BytesQueued()), int(q->GetWaterMark()));

    if (!q->Put(AudioFrameToMsgBlock(frm)))
    {
        MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("Buffer full for user #%d streamtype 0x%x, sampleindex %u, is last: %s. Dropping queue containing %d msec, %u/%u bytes\n"),
                GetUserID(key), GetStreamType(key), frm.sample_no, (frm.input_samples == 0 ? TTEXT("true"):TTEXT("false")),
                q->Size() * m_inputformat.GetDurationMSec(), unsigned(q->BytesQueued()), unsigned(q->GetWaterMark()));
        //flush queue
        q->Flush();
        TTASSERT(q->Size() == 0);
        //insert after flush, so it will appear as a new stream
        if (!q->Put(AudioFrameToMsgBlock(frm)))
        {
            m_usermux_queue.erase(key);
        }
        //clear sample no tracker
        m_usermux_progress.erase(key);
    }
    else
    {
        MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("Submitted #%d streamtype: 0x%x from sample index %u, samples %d\n"),
                     GetUserID(key), GetStreamType(key), frm.sample_no, frm.input_samples);
    }
    return static_cast<int>(q->Size());
}

void AudioMuxer::SubmitPreprocessQueue()
{
    std::vector<int> keys;
    // store keys since we erase 'm_usermux_queue' elements during processing
    auto ii = m_usermux_queue.begin();
    for (;ii != m_usermux_queue.end();++ii)
        keys.push_back(ii->first);

    for (auto key : keys)
    {
        int const userid = GetUserID(key);
        teamtalk::StreamType const st = GetStreamType(key);

        // extract all AudioFrames from user w. StreamType starting with
        // the one in 'm_userresample_block'
        std::vector<std::unique_ptr<MsgBlock>> mbs;
        if (m_preprocess_block.contains(key))
        {
            MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("Resuming #%d streamtype: 0x%x from sample index %u\n"),
                         userid, st, media::AudioFrame(m_preprocess_block[key].get()).sample_no);
            mbs.push_back(std::move(m_preprocess_block[key]));
        }
        m_preprocess_block.erase(key);

        std::unique_ptr<MsgBlock> mb;
        while ((mb = m_preprocess_queue.AcquireAudioFrame(userid, st)) != nullptr)
        {
            MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("Dequeued #%d streamtype: 0x%x sample index %u\n"),
                GetUserID(key), GetStreamType(key), media::AudioFrame(mb.get()).sample_no);
            mbs.push_back(std::move(mb));
        }

        // Submit as many "mux-ready" frames as possible
        while ((mb = BuildMuxAudioFrame(mbs)) != nullptr)
        {
            media::AudioFrame const frm(mb.get());
            MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("Submitting #%d streamtype: 0x%x from sample index %u\n"),
                         userid, st, frm.sample_no);
            SubmitMuxAudioFrame(key, frm);
        }

        // If any AudioBlocks remain then store one AudioFrame in 'm_userresample_block'
        if (mbs.size() == 1)
        {
            m_preprocess_block[key] = std::move(mbs[0]);
            mbs.clear();
            MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("Store #%d streamtype: 0x%x from sample index %u\n"),
                         userid, st, media::AudioFrame(m_preprocess_block[key].get()).sample_no);
        }
        else if (mbs.size() > 1)
        {
            m_preprocess_block[key] = AudioFramesMerge(mbs);
            mbs.clear();
            MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("Store jumbo #%d streamtype: 0x%x from sample index %u\n"),
                         userid, st, media::AudioFrame(m_preprocess_block[key].get()).sample_no);
        }
    }
}

std::unique_ptr<MsgBlock> AudioMuxer::BuildMuxAudioFrame(std::vector<std::unique_ptr<MsgBlock>>& mbs) const
{
    if (mbs.empty())
        return nullptr;

    // check if it's a terminator frame
    media::AudioFrame const frm(mbs[0].get());
    if (frm.input_samples == 0)
    {
        auto mb = std::move(mbs[0]);
        mbs.erase(mbs.begin());
        return mb;
    }

    // check if it's already the correct format
    if (frm.inputfmt == m_inputformat.fmt && frm.input_samples == m_inputformat.samples)
    {
        auto mb = std::move(mbs[0]);
        mbs.erase(mbs.begin());
        return mb;
    }

    return AudioFrameFromList(m_inputformat.samples, mbs);
}

void AudioMuxer::SetMuxInterval(int msec)
{
    // must be set prior to thread start
    assert(!m_thread);

    int cbmsec = m_inputformat.IsValid() ? m_inputformat.GetDurationMSec() : 0;
    cbmsec = std::max(msec, cbmsec);
    assert(cbmsec > 0);
    m_mux_interval = std::chrono::milliseconds(cbmsec);
}

void AudioMuxer::Run()
{
    m_reactor.owner (ACE_OS::thr_self ());

    MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("AudioMuxer interval: %d msec\n"), int(m_mux_interval.count()));
    
    assert(m_mux_interval != std::chrono::milliseconds{});

    TimerHandler th(*this, 577);
    ACE_Time_Value const interval = ToAceDuration(m_mux_interval); // ACE reactor API
    int const timerid = m_reactor.schedule_timer(&th, nullptr, interval, interval);
    TTASSERT(timerid >= 0);
    m_reactor.run_reactor_event_loop ();

    if (timerid >= 0)
    {
        int const ret = m_reactor.cancel_timer(timerid);
        TTASSERT(ret >= 0);
    }

    //flush remaining data
    ProcessAudioQueues(true);
}

int AudioMuxer::TimerEvent(uint32_t  /*timer_event_id*/, long  /*userdata*/)
{
    ProcessAudioQueues(false);
    return 0;
}

void AudioMuxer::ProcessAudioQueues(bool flush)
{
    uint32_t const now = GETTIMESTAMP();
    uint32_t const diff = now - m_last_flush_time;

    // store the list of active sources prior to mux
    std::vector<int> oldkeys;
    {
        // 'm_mutex1_preprocess' protects 'm_mutex2_mux'
        std::unique_lock<std::recursive_mutex> const g(m_mutex1_preprocess);

        auto ii = m_usermux_queue.begin();
        for (; ii != m_usermux_queue.end(); ++ii)
            oldkeys.push_back(ii->first);

        // empty preprocess queue
        SubmitPreprocessQueue();
    }

    int const cb_msec = m_inputformat.GetDurationMSec();
    int const cb_samples = m_inputformat.samples;
    TTASSERT(cb_msec>0);
    TTASSERT(cb_samples>0);
    if((cb_msec == 0) || (cb_samples == 0))
        return;

    int cb_count = static_cast<int>(diff) / cb_msec;
    int const remain_msec = static_cast<int>(diff) % cb_msec;
    while(cb_count != 0)
    {
        StreamTypes sts = STREAMTYPE_NONE;
        {
            std::unique_lock<std::recursive_mutex> const g(m_mutex2_mux);

            if(CanMuxUserAudio())
            {
                sts = MuxUserAudio(); //write muxed audio
            }
            else
            {
                if (m_usermux_queue.empty())
                {
                    //write silence
                    short const zero = 0;
                    m_muxed_buffer.assign(m_muxed_buffer.size(), zero);
                    //MYTRACE(TTEXT("No audio to mux at %u. Writing %d msec silence\n"),
                    //        now, cb_msec);
                }
                else
                {
                    //no data has arrived in time
                    break;
                }
            }
        } // scope so we don't hold lock during callback

        WriteAudio(cb_samples, sts);
        cb_count--;
    }

    if (flush)
    {
        while (true)
        {
            StreamTypes sts = STREAMTYPE_NONE;
            {
                std::unique_lock<std::recursive_mutex> const g(m_mutex2_mux);
                RemoveEmptyMuxUsers();

                if (CanMuxUserAudio())
                    sts = MuxUserAudio();
                else
                    break;
            }
            WriteAudio(cb_samples, sts);
        }
    }

    // MYTRACE(TTEXT("Queued %d msec at %u\n"), (cb_count * cb_msec) - remain_msec, now);
    m_last_flush_time = now - ((cb_count * cb_msec) + remain_msec);

    // 'm_mutex1_preprocess' protects 'm_mutex2_mux'
    std::unique_lock<std::recursive_mutex> const g(m_mutex1_preprocess);

    // still active audio sources
    std::set<int> newkeys;
    auto ii = m_usermux_queue.begin();
    for (; ii != m_usermux_queue.end(); ++ii)
        newkeys.insert(ii->first);
    
    // clear dead audio sources
    for (auto key : oldkeys)
    {
        if (!newkeys.contains(key))
        {
            if (m_preprocess_queue.IsEmpty(GetUserID(key), GetStreamType(key)))
            {
                m_preprocess_queue.RemoveAudioSource(GetUserID(key), GetStreamType(key));
                MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("Removed audio source #%d, streamtype: 0x%x\n"),
                             GetUserID(key), GetStreamType(key));
            }
            else
            {
                // More audio maybe have been submitted to 'm_preprocess_queue'
                // so we must ensure that 'key' still exists in 'm_usermux_queue'.
                GetMuxQueue(key);
            }
        }
    }

    if (m_tickcallback)
        m_tickcallback(m_streamtypes, m_sample_no);
}

bool AudioMuxer::CanMuxUserAudio()
{
    auto ii = m_usermux_queue.begin();
    while(ii != m_usermux_queue.end())
    {
        if(ii->second->Size() == 0)
        {
            MYTRACE_COND(DEBUG_AUDIOMUXER && m_usermux_progress.find(ii->first) != m_usermux_progress.end(),
                         TTEXT("User #%d has submitted no audio to AudioMuxer. Delaying muxer...\n"),
                         GetUserID(ii->first));
            MYTRACE_COND(DEBUG_AUDIOMUXER && m_usermux_progress.find(ii->first) == m_usermux_progress.end(),
                         TTEXT("User #%d has submitted no audio to AudioMuxer. No sample no available\n"), GetUserID(ii->first));
            return false;
        }
        ii++;
    }

    return !m_usermux_queue.empty();
}

void AudioMuxer::RemoveEmptyMuxUsers()
{
    // get rid of users who haven't supplied data in time for
    // flush
    for (auto i = m_usermux_queue.begin();i != m_usermux_queue.end();++i)
    {
        if (i->second->Size() == 0)
        {
            MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("AudioMuxer removed empty audio queue for #%d\n"), GetUserID(i->first));
            SubmitMuxAudioFrame(i->first, media::AudioFrame());
        }
        else
        {
            MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("AudioMuxer still has audio queue for #%d. Items: %d\n"),
                    GetUserID(i->first), int(i->second->Size()));
        }
    }
}

teamtalk::StreamTypes AudioMuxer::MuxUserAudio()
{
    TTASSERT(!m_usermux_queue.empty());
    TTASSERT(!m_muxed_buffer.empty());

    const int SAMPLES = m_inputformat.samples;
    StreamTypes sts = STREAMTYPE_NONE;
    std::vector<std::unique_ptr<MsgBlock>> audio_blocks;
    auto ii = m_usermux_queue.begin();
    while(ii != m_usermux_queue.end())
    {
        std::unique_ptr<MsgBlock> mb;
        if (ii->second->Get(mb, std::chrono::milliseconds(0)))
        {
            media::AudioFrame frm(mb.get());

            //ensure it's the AudioMuxBlock we're expecting
            auto const ui = m_usermux_progress.find(ii->first);
            if(ui != m_usermux_progress.end() && frm.input_samples > 0)
            {
                MYTRACE_COND(frm.sample_no != ui->second + SAMPLES,
                             TTEXT("Unexpected sample no for #%d streamtype %u. Found sample index %u. Should be %u\n"),
                             GetUserID(ii->first), GetStreamType(ii->first), frm.sample_no, ui->second + SAMPLES);

                if (frm.sample_no != ui->second + SAMPLES)
                {
                    // Sample index is out of sync. Mark stream as ended so
                    // muxer can start over.
                    frm = media::AudioFrame();
                }
            }

            MYTRACE_COND(DEBUG_AUDIOMUXER && ui != m_usermux_progress.end(),
                         TTEXT("Processing/muxing #%d streamtype 0x%x sampleindex: %u\n"),
                         GetUserID(ii->first), GetStreamType(ii->first), frm.sample_no);

            if (frm.input_samples == 0)
            {
                MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("Clearing #%d streamtype 0x%x, sampleindex %u\n"),
                             GetUserID(ii->first), GetStreamType(ii->first), frm.sample_no);

                // remove expected sample-offset for next run
                m_usermux_progress.erase(ii->first);

                //stream ended from user
                if (m_usermux_queue[ii->first]->Size() == 0)
                    m_usermux_queue.erase(ii++);
            }
            else
            {
                MYTRACE_COND(DEBUG_AUDIOMUXER, TTEXT("Adding #%d streamtype 0x%x, sampleindex %u\n"),
                             GetUserID(ii->first), GetStreamType(ii->first), frm.sample_no);
                m_usermux_progress[ii->first] = frm.sample_no;
                audio_blocks.push_back(std::move(mb));
                // 'userdata' contains StreamType. May be STREAMTYPE_NONE if silence frame was
                // submitted (e.g. after stopped-talking)
                sts |= frm.userdata;
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
            audio_blocks.clear();
            return STREAMTYPE_NONE;
        }
    }

    TTASSERT(int(m_muxed_buffer.size()) == m_inputformat.GetTotalSamples());

    if(audio_blocks.empty())
    {
        short const zero = 0;
        m_muxed_buffer.assign(m_muxed_buffer.size(), zero);
    }
    else
    {
        media::AudioFrame frm(audio_blocks[0].get());
        frm.ApplyGain();
        TTASSERT(frm.input_samples);
        TTASSERT(frm.input_samples == m_inputformat.samples);
        TTASSERT(frm.inputfmt == m_inputformat.fmt);
        TTASSERT(int(m_muxed_buffer.size()) == m_inputformat.GetTotalSamples());
        m_muxed_buffer.assign(frm.input_buffer, frm.input_buffer + m_muxed_buffer.size());

        //this is where we mux if there's more than one user
        if(audio_blocks.size()>1)
        {
            for(size_t a=1;a<audio_blocks.size();a++)
            {
                media::AudioFrame mfrm(audio_blocks[a].get());
                mfrm.ApplyGain();
                TTASSERT(mfrm.input_buffer);
                TTASSERT(mfrm.input_samples == m_inputformat.samples);
                for(size_t i=0;i<m_muxed_buffer.size();i++)
                {
                    int const val = static_cast<int>(m_muxed_buffer[i]) + mfrm.input_buffer[i];
                    if(val > 32767)
                        m_muxed_buffer[i] = 32767;
                    else if(val < -32768)
                        m_muxed_buffer[i] = -32768;
                    else
                        m_muxed_buffer[i] = val;
                }
            }
        }

        audio_blocks.clear();
    }

    return sts;
}

void AudioMuxer::WriteAudio(int cb_samples, teamtalk::StreamTypes sts)
{
    media::AudioFrame frame(m_inputformat.fmt, m_muxed_buffer.data(), cb_samples, m_sample_no);
    frame.userdata = sts;
    if (m_muxcallback)
    {
        m_muxcallback(m_streamtypes, frame);
    }

#if defined(ENABLE_SPEEXFILE)
    if(m_speexfile && (m_inputformat.samples != 0))
    {
        int ret = 0;
        for(int i=0;i<cb_samples / m_inputformat.samples && ret >= 0;i++)
        {
            ret = m_speexfile->Encode(&m_muxed_buffer[static_cast<size_t>(i) * m_inputformat.GetTotalSamples()]);
        }
    }
#endif

#if defined(ENABLE_OPUSFILE)
    if(m_opusfile && (m_inputformat.samples != 0))
    {
        int ret = 0;
        for(int i=0;i<cb_samples/m_inputformat.samples && ret >= 0;i++)
            ret = m_opusfile->Encode(&m_muxed_buffer[ static_cast<size_t>(i) * m_inputformat.GetTotalSamples()], m_inputformat.samples, false);
    }
#endif

#if defined(ENABLE_MEDIAFOUNDATION)
    if(m_mp3encoder && m_inputformat.samples)
    {
        m_mp3encoder->ProcessAudioEncoder(frame);
    }
#endif

    if(m_wavefile)
        m_wavefile->AppendSamples(m_muxed_buffer.data(), cb_samples);

    m_sample_no += cb_samples;
}

ChannelAudioMuxer::ChannelAudioMuxer()
= default;

ChannelAudioMuxer::~ChannelAudioMuxer()
{
    assert(m_userchan.empty());
}

bool ChannelAudioMuxer::SaveFile(int channelid, const teamtalk::AudioCodec& codec,
                                 teamtalk::StreamTypes sts,
                                 const tstring& filename,
                                 teamtalk::AudioFileFormat aff)
{
    std::unique_lock<std::recursive_mutex> const g(m_mutex);

    if (m_muxers.contains(channelid))
        return false;

    audiomuxer_t const muxer = std::make_shared<AudioMuxer>(sts);
    bool const ret = muxer->SaveFile(codec, filename, aff);
    if (!ret)
        return false;

    m_muxers[channelid] = muxer;
    return ret;
}

bool ChannelAudioMuxer::CloseFile(int channelid)
{
    audiomuxer_t muxer;
    {
        std::unique_lock<std::recursive_mutex> const g(m_mutex);
        auto mi = m_muxers.find(channelid);
        if (mi != m_muxers.end())
        {
            muxer = mi->second;
            m_muxers.erase(mi);
        }
    }
    return muxer.get() != nullptr;
}

bool ChannelAudioMuxer::AddUser(int userid, teamtalk::StreamType st, int channelid)
{
    std::unique_lock<std::recursive_mutex> const g(m_mutex);
    int const key = GenKey(userid, st);

    assert(m_userchan.find(key) == m_userchan.end());

    if (!m_userchan.contains(key))
    {
        m_userchan[key] = channelid;
        return true;
    }
    return false;
}

bool ChannelAudioMuxer::RemoveUser(int userid, teamtalk::StreamType st)
{
    std::unique_lock<std::recursive_mutex> const g(m_mutex);
    int const key = GenKey(userid, st);
    return m_userchan.erase(key) != 0u;
}

void ChannelAudioMuxer::QueueUserAudio(int userid, teamtalk::StreamType st,
                                       const media::AudioFrame& frm)
{
    audiomuxer_t chanmuxer;
    audiomuxer_t fixedmuxer;
    {
        int const key = GenKey(userid, st);

        std::unique_lock<std::recursive_mutex> const g(m_mutex);
        if (m_userchan.contains(key))
        {
            int const chanid = m_userchan[key];
            
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
