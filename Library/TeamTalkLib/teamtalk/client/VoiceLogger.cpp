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

#include <ace/SString.h>
#include <ace/Date_Time.h>
#include <ace/Thread_Manager.h>
#include <ace/FILE_Connector.h>
#include <ace/Sched_Params.h>
#include <ace/FILE_IO.h>

#include <myace/MyACE.h>
#include <myace/TimerHandler.h>
#include <teamtalk/ttassert.h>
#include <teamtalk/Log.h>
#include <teamtalk/CodecCommon.h>

#include "VoiceLogger.h"

enum
{
    TIMER_WRITELOG_ID = 1,
    TIMER_CANCELLOG_ID,
};

#define FLUSH_INTERVAL ACE_Time_Value(3)

using namespace std;
using namespace teamtalk;

VoiceLog::VoiceLog(int userid, const ACE_TString& filename, 
                   const AudioCodec& codec, AudioFileFormat aff,
                   int stream_id, int stoppedtalking_delay)
: m_tot_msec(stoppedtalking_delay)
, m_packet_current(-1)
, m_userid(userid)
, m_codec(codec)
, m_filename(filename)
, m_aff(aff)
, m_streamid(stream_id)
, m_closing(false)
{
    int samplerate = GetAudioCodecSampleRate(m_codec);
    int channels = GetAudioCodecChannels(m_codec);
    int cbsamples = GetAudioCodecCbSamples(m_codec);

    switch(aff)
    {
    case AFF_CHANNELCODEC_FORMAT :
#if defined(ENABLE_OGG)
    {
        bool vbr = true;
        ACE_UNUSED_ARG(vbr);
        switch(m_codec.codec)
        {
        case CODEC_SPEEX :
            vbr = false;
        case CODEC_SPEEX_VBR :
        {
#if defined(ENABLE_SPEEX)
            SpeexFile* spx_file;
            ACE_NEW(spx_file, SpeexFile());
            m_speexfile = speexfile_t(spx_file);
            if(!m_speexfile->Open(filename, GetSpeexBandMode(m_codec), vbr))
            {
                ACE_TString error = ACE_TEXT("Failed to open Speex file ") + filename;
                TT_ERROR(error.c_str());
                m_active = false;
                return;
            }
#endif /* ENABLE_SPEEX */
        }
        break;
#if defined(ENABLE_OPUSTOOLS)
        case CODEC_OPUS :
        {
            OpusFile* opus_file;
            ACE_NEW(opus_file, OpusFile());
            m_opusfile = opusfile_t(opus_file);
            int framesize = GetAudioCodecFrameSize(m_codec);
            if(!m_opusfile->Open(filename, channels, samplerate, framesize))
            {
                ACE_TString error = ACE_TEXT("Failed to open OPUS file ") + filename;
                TT_ERROR(error.c_str());
                m_active = false;
                return;
            }
        }
        break;
#else
        case CODEC_OPUS :
#endif /* ENABLE_OPUSTOOLS */
        case CODEC_NO_CODEC :
        case CODEC_WEBM_VP8 :
            break;
        }
    }
#endif /* ENABLE_OGG */
    break;

    case AFF_MP3_16KBIT_FORMAT :
    case AFF_MP3_32KBIT_FORMAT :
    case AFF_MP3_64KBIT_FORMAT :
    case AFF_MP3_128KBIT_FORMAT :
    case AFF_MP3_256KBIT_FORMAT :
#if defined(ENABLE_MEDIAFOUNDATION)
    {
        int mp3bitrate = AFFToMP3Bitrate(aff);
        m_mp3transform = MFTransform::CreateMP3(media::AudioFormat(samplerate, channels), mp3bitrate, filename.c_str());
        if (!m_mp3transform)
        {
            ACE_TString error = ACE_TEXT("Failed to open file ") + filename;
            TT_ERROR(error.c_str());
            m_active = false;
            m_mp3transform.reset();
            return;
        }
    }
#endif
    break;
    case AFF_WAVE_FORMAT :
    {
        m_wavfile.reset(new WavePCMFile());
        if(!m_wavfile->NewFile(filename.c_str(), samplerate, channels))
        {
            ACE_TString error = ACE_TEXT("Failed to open file ") + filename;
            TT_ERROR(error.c_str());
            m_active = false;
            return;
        }
    }
    break;
    case AFF_NONE:
        assert(0);
        return;
    }

    switch(codec.codec)
    {
    case CODEC_SPEEX :
    case CODEC_SPEEX_VBR :
#if defined(ENABLE_SPEEX)
        m_speex.reset(new SpeexDecoder());
        if(!m_speex->Initialize(GetSpeexBandMode(m_codec)))
            return;
#endif
        break;
    case CODEC_OPUS :
#if defined(ENABLE_OPUS)
        m_opus.reset(new OpusDecode());
        if(!m_opus->Open(codec.opus.samplerate, codec.opus.channels))
            return;
#endif
        break;
    case CODEC_NO_CODEC :
    case CODEC_WEBM_VP8 :
        TTASSERT(0);
        break;
    }

    if (cbsamples > 0)
    {
        m_samples_buf.resize(cbsamples * channels);
        m_active = true;
    }
    MYTRACE(ACE_TEXT("VoiceLog started: %s\n"), this->GetFileName().c_str());
}

VoiceLog::~VoiceLog()
{
    MYTRACE(ACE_TEXT("VoiceLog ended: %s\n"), this->GetFileName().c_str());
}

void VoiceLog::AddVoicePacket(const teamtalk::AudioPacket& packet)
{
    TTASSERT(packet.GetStreamID() == m_streamid);
    TTASSERT(!packet.HasFragments());
    if(packet.GetStreamID() != m_streamid ||
       packet.HasFragments())
        return;

    wguard_t g(m_mutex);

    m_last = ACE_OS::gettimeofday();

    int packet_no = packet.GetPacketNumber();
    bool first = false;
    if (m_packet_current == -1)
    {
        m_packet_current = packet_no;
        first = true;
    }

    if (W16_LT(packet_no, m_packet_current))
    {
        MYTRACE(ACE_TEXT("Skipped delayed packet %d from #%d voice log\n"),
                packet_no, m_userid);
        return;
    }

    if (first || W32_GT(packet.GetTime(), m_packet_timestamp))
        m_packet_timestamp = packet.GetTime();

    m_mQueuePackets[packet_no].reset(new AudioPacket(packet));
}

void VoiceLog::FlushLog()
{
    wguard_t g(m_mutex);
    m_mFlushPackets.insert(m_mQueuePackets.begin(),m_mQueuePackets.end());
    m_mQueuePackets.clear();

    g.release();

    m_packet_current = WritePackets(m_packet_current);
}

int VoiceLog::WritePackets(int pktno_cur)
{
    while(!m_mFlushPackets.empty())
    {
        WritePacket(pktno_cur++);
        pktno_cur &= 0xFFFF;
    }

    return pktno_cur;
}

void VoiceLog::WritePacket(int packet_no)
{
    if(m_aff != AFF_CHANNELCODEC_FORMAT)
    {
        WriteAudio(packet_no);
        return;
    }

    //Write in channel's audio codec format
    //AFF_CHANNELCODEC_FORMAT

    int fpp = GetAudioCodecFramesPerPacket(m_codec);
    vector<uint16_t> frame_sizes;
    uint16_t enc_len = 0;
    const char* enc_data = NULL;

    mappackets_t::iterator ite = m_mFlushPackets.find(packet_no);
    if(ite != m_mFlushPackets.end())
    {
        AudioPacket& packet = *ite->second;
        frame_sizes = GetAudioPacketFrameSizes(packet, m_codec);
        enc_data = packet.GetEncodedAudio(enc_len);
    }
    else
    {
        frame_sizes.resize(fpp);
    }
    
    switch(m_codec.codec)
    {
    case CODEC_SPEEX :
    case CODEC_SPEEX_VBR :
#if defined(ENABLE_OGG) && defined(ENABLE_SPEEX)
        TTASSERT(m_speexfile.get());
        if (m_speexfile)
        {
            int pos = 0;
            for(size_t i=0;i<frame_sizes.size();i++)
            {
                m_speexfile->WriteEncoded(enc_data?&enc_data[pos]:NULL, frame_sizes[i],
                                          m_mFlushPackets.size() == 1 &&
                                          m_closing && i+1 == frame_sizes.size());
                pos += frame_sizes[i];
            }
        }
#endif /* ENABLE_OGG && ENABLE_SPEEX */
        break;
    case CODEC_OPUS :
#if defined(ENABLE_OGG) && defined(ENABLE_OPUSTOOLS)
        TTASSERT(m_opusfile.get());
        if (m_opusfile)
        {
            int pos = 0;
            for(size_t i=0;i<frame_sizes.size();i++)
            {
                m_opusfile->WriteEncoded(enc_data?&enc_data[pos]:NULL, frame_sizes[i],
                                         m_mFlushPackets.size() == 1 &&
                                         m_closing && i+1 == frame_sizes.size());
                pos += frame_sizes[i];
            }
            
        }
#endif /* ENABLE_OGG && ENABLE_OPUSTOOLS */
        break;
    case CODEC_NO_CODEC :
    case CODEC_WEBM_VP8 :
        assert(0);
        break;
    }

    if(ite != m_mFlushPackets.end())
        m_mFlushPackets.erase(ite);
}

void VoiceLog::WriteAudio(int packet_no)
{
    if(GetAudioCodecCbSamples(m_codec)<=0)
        return;

    mappackets_t::iterator ite = m_mFlushPackets.find(packet_no);
    if(ite != m_mFlushPackets.end())
    {
        AudioPacket& packet = *ite->second;
        uint16_t enc_len;
        const char* enc_data = packet.GetEncodedAudio(enc_len);
        switch(m_codec.codec)
        {
        case CODEC_SPEEX :
#if defined(ENABLE_SPEEX)
            if(m_speex.get())
            {
                int frames_per_packet = GetAudioCodecFramesPerPacket(m_codec);
                vector<int> frame_sizes(frames_per_packet, enc_len / frames_per_packet);
                m_speex->DecodeMultiple(enc_data, frame_sizes, &m_samples_buf[0]);
            }
#endif
            break;
        case CODEC_SPEEX_VBR :
#if defined(ENABLE_SPEEX)
            if(m_speex.get())
            {
                vector<uint16_t> frame_sizes = packet.GetEncodedFrameSizes();
                m_speex->DecodeMultiple(enc_data,
                                        ConvertFrameSizes(frame_sizes),
                                        &m_samples_buf[0]);
            }
#endif
            break;
        case CODEC_OPUS :
#if defined(ENABLE_OPUS)
            if(m_opus.get())
            {
                vector<uint16_t> frame_sizes = GetAudioPacketFrameSizes(packet, m_codec);
                int sum_dec = 0;
                int cb_samples = GetAudioCodecCbSamples(m_codec);
                int channels = GetAudioCodecChannels(m_codec);
                int framesize = GetAudioCodecFrameSize(m_codec);
                int decsamples = 0, ret;
                for(size_t i=0;i<frame_sizes.size();i++)
                {
                    ret = m_opus->Decode(&enc_data[sum_dec], frame_sizes[i],
                                         &m_samples_buf[framesize * channels * i],
                                         cb_samples);
                    assert(ret > 0);
                    decsamples += ret;
                    sum_dec += frame_sizes[i];
                }
                assert(decsamples == cb_samples);
            }
#endif
            break;
        default : break;
        }
        m_mFlushPackets.erase(ite);
    }
    else
    {
        switch(m_codec.codec)
        {
        case CODEC_SPEEX :
        case CODEC_SPEEX_VBR :
#if defined(ENABLE_SPEEX)
            if(m_speex.get())
            {
                vector<int> frame_sizes(GetAudioCodecFramesPerPacket(m_codec),
                    0);
                m_speex->DecodeMultiple(NULL, frame_sizes, &m_samples_buf[0]);
            }
#endif
            break;
        case CODEC_OPUS :
#if defined(ENABLE_OPUS)
            if(m_opus.get())
            {
                int fpp = GetAudioCodecFramesPerPacket(m_codec);
                int cb_samples = GetAudioCodecCbSamples(m_codec);
                int channels = GetAudioCodecChannels(m_codec);
                for(int i=0;i<fpp;i++)
                {
                    m_opus->Decode(NULL, 0,
                                   &m_samples_buf[cb_samples*channels*i],
                                   cb_samples);
                }
            }
#endif        
            break;
        default : break;
        }
    }
#if defined(ENABLE_MEDIAFOUNDATION)
    if (m_mp3transform)
    {
        media::AudioFormat fmt = GetAudioCodecAudioFormat(m_codec);
        int samples = GetAudioCodecCbSamples(m_codec);
        m_mp3transform->ProcessAudioEncoder(media::AudioFrame(fmt, &m_samples_buf[0], samples), true);
    }
#endif
    if(m_wavfile)
        m_wavfile->AppendSamples(&m_samples_buf[0], GetAudioCodecCbSamples(m_codec));
}

void VoiceLog::WriteSilence(int msecs)
{
    if(m_samples_buf.empty())
        return;

    m_samples_buf.assign(m_samples_buf.size(), 0);

    int samplerate = GetAudioCodecSampleRate(m_codec);
    int samples = (int)(((msecs / 1000) * samplerate) + ((double)(msecs % 1000)/1000.0) * samplerate);

    media::AudioFormat fmt = GetAudioCodecAudioFormat(m_codec);
    int cbsamples = GetAudioCodecCbSamples(m_codec);

    while(samples > GetAudioCodecCbSamples(m_codec))
    {
#if defined(ENABLE_MEDIAFOUNDATION)
        if(m_mp3transform)
            m_mp3transform->ProcessAudioEncoder(media::AudioFrame(fmt, &m_samples_buf[0], cbsamples), true);
#endif
        if(m_wavfile)
            m_wavfile->AppendSamples(&m_samples_buf[0], GetAudioCodecCbSamples(m_codec));
        samples -= GetAudioCodecCbSamples(m_codec);
    }
    if (samples > 0)
    {
#if defined(ENABLE_MP3)
        if(m_mp3transform)
            m_mp3transform->ProcessAudioEncoder(media::AudioFrame(fmt, &m_samples_buf[0], samples), true);
#endif
        if(m_wavfile)
            m_wavfile->AppendSamples(&m_samples_buf[0], samples);
    }
}

ACE_Time_Value VoiceLog::GetVoiceEndTime() const
{
    return m_last + ToTimeValue(m_tot_msec);
}

uint32_t VoiceLog::GetLatestPacketTime() const
{
    assert(m_packet_current != -1);
    return m_packet_timestamp;
}

VoiceLogFile VoiceLog::GetVoiceLogFile()
{
    VoiceLogFile vlogfile;
    vlogfile.filename = GetFileName();
    vlogfile.samplerate = GetSampleRate();
    vlogfile.channels = GetChannels();
    vlogfile.duration = GetDuration();
    vlogfile.aff = GetAFF();

    return vlogfile;
}

int VoiceLog::GetDuration()
{
    if(m_wavfile && m_wavfile->GetSampleRate()>0)
        return m_wavfile->GetSamplesCount() * 1000 / m_wavfile->GetSampleRate();
    return 0;
}

int VoiceLog::GetSampleRate() const
{
    return GetAudioCodecSampleRate(m_codec);
}

int VoiceLog::GetChannels() const
{
    return GetAudioCodecChannels(m_codec);
}

////////////////////
//  VoiceLogger
////////////////////
VoiceLogger::VoiceLogger(VoiceLogListener* listener)
: m_timerid(-1)
, m_listener(listener)
{
}

VoiceLogger::~VoiceLogger()
{
    if(m_timerid != -1)
        m_reactor.cancel_timer(m_timerid, 0, 0);
    m_reactor.end_reactor_event_loop();
    this->wait();

    MYTRACE(ACE_TEXT("~VoiceLogger()\n"));
}

int VoiceLogger::TimerEvent(ACE_UINT32 timer_event_id, long userdata)
{
    switch(timer_event_id)
    {
    case TIMER_WRITELOG_ID :
        FlushLogs();
        break;
    case TIMER_CANCELLOG_ID :
        EndLog(userdata);
        return -1;
    }
    return 0;
}

void VoiceLogger::BeginLog(ClientUser& from_user, 
                           const AudioCodec& codec, 
                           int stream_id,
                           const ACE_TString& folderpath)
{
    //spawn thread?
    if(this->thr_count() == 0)
    {
        TTASSERT(m_timerid == -1);
        long prio = ACE_Sched_Params::priority_min (ACE_SCHED_FIFO);

        int ret = this->activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED, 
            1, 0, prio);
        TTASSERT(ret>=0);

        TimerHandler* th;
        ACE_NEW(th, TimerHandler(*this, TIMER_WRITELOG_ID));
        m_timerid = m_reactor.schedule_timer(th, 0, FLUSH_INTERVAL, FLUSH_INTERVAL);
        TTASSERT(m_timerid>=0);
    }

    wguard_t g1(m_add_mtx);
    wguard_t g2(m_flush_mtx);

    ACE_Time_Value tv = ACE_OS::gettimeofday();
    ACE_Date_Time dt(tv);
    const int STR_SIZE = 512;
    ACE_TCHAR buf[STR_SIZE];
    ACE_TString username = from_user.GetUsername();
    int userid = from_user.GetUserID();

    ACE_TString var = from_user.GetAudioFileVariables();
    
    if(var.is_empty())
        var = DEFAULT_VOICELOG_VARS;

    replace_all(var, ACE_TEXT("%nickname%"), from_user.GetNickname());
    replace_all(var, ACE_TEXT("%username%"), from_user.GetUsername());
    replace_all(var, ACE_TEXT("%userid%"), i2string(from_user.GetUserID()));
    ACE_UINT32 counter = from_user.IncVoiceLogCounter();
    ACE_TCHAR str_count[100];
    ACE_OS::snprintf(str_count, 100, ACE_TEXT("%09u"), counter);
    replace_all(var, ACE_TEXT("%counter%"), str_count);    

    time_t now = time(NULL);
    struct tm* ttt = localtime(&now);
#ifdef ACE_WIN32
    wcsftime(buf, STR_SIZE-1, var.c_str(), ttt);
#else
    ACE_OS::strftime(buf, STR_SIZE-1, var.c_str(), ttt);
#endif
    buf[STR_SIZE-1] = 0;

    // if(username.length())
    //     ACE_OS::snprintf(buf, STR_SIZE, ACE_TEXT("%d%.2d%.2d-%.2d%.2d%.2d #%.5d %s"), 
    //     dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), userid, username.c_str());
    // else
    //     ACE_OS::snprintf(buf, STR_SIZE, ACE_TEXT("%d%.2d%.2d-%.2d%.2d%.2d #%.5d"), 
    //     dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), userid);

    ACE_TString filename = buf;
    int mp3bitrate = 0;
    AudioFileFormat aff = AFF_WAVE_FORMAT;
    switch(from_user.GetAudioFileFormat())
    {
    case AFF_CHANNELCODEC_FORMAT :
        switch(codec.codec)
        {
        case CODEC_SPEEX :
        case CODEC_SPEEX_VBR :
        case CODEC_OPUS :
            aff = from_user.GetAudioFileFormat();
            filename += ACE_TEXT(".ogg");
            break;
        case CODEC_NO_CODEC :
        case CODEC_WEBM_VP8 :
            break;
        }
        break;
    case AFF_MP3_16KBIT_FORMAT :
        mp3bitrate = 16000;
        break;
    case AFF_MP3_32KBIT_FORMAT :
        mp3bitrate = 32000;
        break;
    case AFF_MP3_64KBIT_FORMAT :
        mp3bitrate = 64000;
        break;
    case AFF_MP3_128KBIT_FORMAT :
        mp3bitrate = 128000;
        break;
    case AFF_MP3_256KBIT_FORMAT :
        mp3bitrate = 256000;
        break;
    case AFF_NONE :
    case AFF_WAVE_FORMAT :
    default :
        aff = AFF_WAVE_FORMAT;
        filename += ACE_TEXT(".wav");
        break;
    }

    if (mp3bitrate)
    {
        aff = from_user.GetAudioFileFormat();
        filename += ACE_TEXT(".mp3");
    }

    ACE_TString filepath = folderpath + ACE_DIRECTORY_SEPARATOR_STR + filename;

    VoiceLog* newlog;
    ACE_NEW(newlog, VoiceLog(from_user.GetUserID(), filepath,
                             codec, aff, stream_id,
                             from_user.GetPlaybackStoppedDelay(STREAMTYPE_VOICE)));
    voicelog_t log (newlog);

    bool active = log->IsActive();
    if(active)
        m_mLogs[from_user.GetUserID()] = log;

    VoiceLogFile vlogfile = log->GetVoiceLogFile();
    log.reset(); //ensure file is not locked

    if(active)
        m_listener->OnMediaFileStatus(from_user.GetUserID(), MFS_STARTED, vlogfile);
    else
        m_listener->OnMediaFileStatus(from_user.GetUserID(), MFS_ERROR, vlogfile);
}

bool VoiceLogger::EndLog(int userid)
{
    wguard_t g1(m_add_mtx);
    wguard_t g2(m_flush_mtx);

    mapvlogs_t::iterator ite = m_mLogs.find(userid);
    if(ite != m_mLogs.end())
    {
        voicelog_t vlog = ite->second;
        vlog->SetClosing();
        vlog->FlushLog();
        m_mLogs.erase(userid);

        VoiceLogFile vlogfile = vlog->GetVoiceLogFile();
        if(vlog->IsActive())
        {
            vlog.reset(); //ensure file is not locked
            m_listener->OnMediaFileStatus(userid, MFS_FINISHED, vlogfile);
            return true;
        }
    }
    return false;
}

void VoiceLogger::AddVoicePacket(ClientUser& from_user, 
                                 const ClientChannel& channel, 
                                 const teamtalk::AudioPacket& packet)
{
    wguard_t g(m_add_mtx);

    TTASSERT(!packet.HasFragments());

    TTASSERT(from_user.GetAudioFolder().length());

    if(channel.GetChannelID() != packet.GetChannel())
    {
        MYTRACE(ACE_TEXT("Dropped packet destined for incorrect channel\n"));
        return;
    }

    mapvlogs_t::iterator ite = m_mLogs.find(from_user.GetUserID());
    if(ite == m_mLogs.end())
    {
        BeginLog(from_user, channel.GetAudioCodec(), packet.GetStreamID(),
            from_user.GetAudioFolder());
    }
    else if(ite->second->GetStreamID() != packet.GetStreamID())
    {
        if (W32_LT(packet.GetTime(), ite->second->GetLatestPacketTime()))
        {
            MYTRACE(ACE_TEXT("Ignored packet %d from #%d. Packet older than latest\n"),
                    packet.GetPacketNumber(), from_user.GetUserID());
            return;
        }

        EndLog(from_user.GetUserID());
        BeginLog(from_user, channel.GetAudioCodec(), packet.GetStreamID(),
                 from_user.GetAudioFolder());
    }
    ite = m_mLogs.find(from_user.GetUserID());
    if(ite != m_mLogs.end() && ite->second->IsActive())
        ite->second->AddVoicePacket(packet);
}

void VoiceLogger::CancelLog(int userid)
{
    TimerHandler* th;
    ACE_NEW(th, TimerHandler(*this, TIMER_CANCELLOG_ID, userid));
    long tid = m_reactor.schedule_timer(th, 0, ACE_Time_Value::zero);
    TTASSERT(tid >= 0);
}

ACE_TString VoiceLogger::GetVoiceLogFileName(int userid)
{
    wguard_t g1(m_add_mtx);

    mapvlogs_t::const_iterator ii = m_mLogs.find(userid);
    if(ii != m_mLogs.end())
        return ii->second->GetFileName();
    return ACE_TString();
}

void VoiceLogger::FlushLogs()
{
    wguard_t g(m_flush_mtx);

    std::vector<int> closeLogs;
    for(mapvlogs_t::iterator ite = m_mLogs.begin();
        ite != m_mLogs.end();ite++)
    {
        ite->second->FlushLog();
        if (ite->second->GetVoiceEndTime() < ACE_OS::gettimeofday())
            closeLogs.push_back(ite->first);
    }
    g.release(); // don't hold lock otherwise lock-order with m_add_mtx can end up wrong

    for(size_t i=0;i<closeLogs.size();i++)
        EndLog(closeLogs[i]);
}

int VoiceLogger::svc (void)
{
    m_reactor.owner (ACE_OS::thr_self ());
    m_reactor.run_reactor_event_loop ();
    return 0;
}
