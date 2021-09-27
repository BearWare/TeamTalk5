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

#if !defined(VOICELOGGER_H)
#define VOICELOGGER_H

#include "ClientUser.h"

#include <myace/TimerHandler.h>
#include <codec/WaveFile.h>
#include <teamtalk/PacketLayout.h>

#if defined(ENABLE_SPEEX)
#include <codec/SpeexDecoder.h>
#endif

#if defined(ENABLE_OPUS)
#include <codec/OpusDecoder.h>
#endif

#if defined(ENABLE_MEDIAFOUNDATION)
#include <avstream/MFTransform.h>
#endif

#if defined(ENABLE_OGG)
#include <codec/OggFileIO.h>
#endif

#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Message_Queue.h>

#include <map>
#include <memory>

#define DEFAULT_VOICELOG_VARS ACE_TEXT("%Y%m%d-%H%M%S #%userid% %username%")

namespace teamtalk {

    struct VoiceLogFile
    {
        ACE_TString filename;
        int samplerate;
        int channels;
        int duration;
        AudioFileFormat aff;

        VoiceLogFile()
            : samplerate(0)
            , channels(0)
            , duration(0)
            , aff(AFF_NONE) {}
    };

    // Mutex of VoiceLog is ensured by VoiceLogger
    class VoiceLog
    {
    public:
        //// no copying
        VoiceLog(const VoiceLog& log) = delete;

        VoiceLog(int userid, const ACE_TString& filename,
                 const AudioCodec& codec, AudioFileFormat aff,
                 int stream_id, int stoppedtalking_delay);
        ~VoiceLog();

        void AddVoicePacket(const teamtalk::AudioPacket& packet);
        void FlushLog();

        uint32_t GetLatestPacketTime() const;
        ACE_Time_Value GetVoiceEndTime() const;

        VoiceLogFile GetVoiceLogFile();

        bool IsActive() const { return m_active; }

        ACE_TString GetFileName() const { return m_filename; }
        AudioFileFormat GetAFF() const { return m_aff; }

        int GetDuration();
        int GetSampleRate() const;
        int GetChannels() const;
        int GetUserID() const { return m_userid; }
        int GetStreamID() const { return m_streamid; }
        void SetClosing() { m_closing = true; }
    private:
        typedef std::map<int, audiopacket_t, w16_less_comp> mappackets_t;

        int WritePackets(int pktno_cur);
        void WritePacket(int packet_no);
        void WriteAudio(int packet_no);
        void WriteSilence(int msecs);

        ACE_Recursive_Thread_Mutex m_mutex;
        mappackets_t m_mQueuePackets; //packetnum --> packet
        mappackets_t m_mFlushPackets; //packetnum --> packet
        int m_packet_current;
        ACE_Time_Value m_last;
        uint32_t m_packet_timestamp = 0; // timestamp of most recent packet
        int m_tot_msec; // auto close voice log after this timeout
#if defined(ENABLE_SPEEX)
        std::unique_ptr<SpeexDecoder> m_speex;
#endif
#if defined(ENABLE_OPUS)
        std::unique_ptr<OpusDecode> m_opus;
#endif
        wavepcmfile_t m_wavfile;
#if defined(ENABLE_MEDIAFOUNDATION)
        mftransform_t m_mp3transform;
#endif
#if defined(ENABLE_OGG) && defined(ENABLE_SPEEX)
        speexfile_t m_speexfile;
#endif
#if defined(ENABLE_OPUSTOOLS)
        opusfile_t m_opusfile;
#endif
        ACE_TString m_filename;

        teamtalk::AudioCodec m_codec;
        AudioFileFormat m_aff;
        std::vector<short> m_samples_buf;
        bool m_active;
        bool m_closing;
        int m_userid;
        int m_streamid;
    };

    typedef std::shared_ptr< VoiceLog > voicelog_t;

    class VoiceLogListener
    {
    public:
        virtual ~VoiceLogListener() {}
        virtual void OnMediaFileStatus(int userid, MediaFileStatus status, 
                                       const VoiceLogFile& vlog) = 0;
    };

    class VoiceLogger : public TimerListener
                      , private ACE_Task_Base
    {
    public:
        VoiceLogger(VoiceLogListener* listener);
        virtual ~VoiceLogger();

        int TimerEvent(ACE_UINT32 timer_event_id, long userdata);

        void AddVoicePacket(ClientUser& from_user, 
                            const ClientChannel& channel, 
                            const AudioPacket& packet);

        void CancelLog(int userid);
        ACE_TString GetVoiceLogFileName(int userid);

    private:
        int svc (void);
        void BeginLog(ClientUser& user, 
                      const AudioCodec& codec, 
                      int stream_id,
                      const ACE_TString& folderpath);
        bool EndLog(int userid);
        void FlushLogs();
        typedef std::map<int, voicelog_t> mapvlogs_t;
        mapvlogs_t m_mLogs;
        ACE_Recursive_Thread_Mutex m_add_mtx, m_flush_mtx;
        ACE_Reactor m_reactor;
        int m_timerid;
        VoiceLogListener* m_listener;
    };

    typedef std::shared_ptr< VoiceLogger > voicelogger_t;
}
#endif
