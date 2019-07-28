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

#if !defined(CLIENTUSER_H)
#define CLIENTUSER_H

#include <myace/MyACE.h>
#include <myace/TimerHandler.h>

#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Guard_T.h>

#include <teamtalk/User.h>
#include "DesktopShare.h"
#include "StreamPlayers.h"

#include "ClientChannel.h"

#include <map>
#include <queue>

#define DESKTOPINPUT_QUEUE_MAX_SIZE 100
#define DESKTOPINPUT_MAX_RTX_PACKETS 16

namespace teamtalk {

    struct ClientUserStats
    {
        ACE_INT64 voicepackets_recv;
        ACE_INT64 voicepackets_lost;

        ACE_INT64 vidcappackets_recv;
        ACE_INT64 vidcapframes_recv;
        ACE_INT64 vidcapframes_lost;
        ACE_INT64 vidcapframes_dropped;

        ACE_INT64 mediafile_audiopackets_recv;
        ACE_INT64 mediafile_audiopackets_lost;

        ACE_INT64 mediafile_video_packets_recv;
        ACE_INT64 mediafile_video_frames_recv;
        ACE_INT64 mediafile_video_frames_lost;
        ACE_INT64 mediafile_video_frames_dropped;

        ClientUserStats() 
            : voicepackets_recv(0)
            , voicepackets_lost(0)
            , vidcappackets_recv(0)
            , vidcapframes_recv(0)
            , vidcapframes_lost(0)
            , vidcapframes_dropped(0)
            , mediafile_audiopackets_recv(0)
            , mediafile_audiopackets_lost(0)
            , mediafile_video_packets_recv(0)
            , mediafile_video_frames_recv(0)
            , mediafile_video_frames_lost(0)
            , mediafile_video_frames_dropped(0)
        {}
    };

    typedef std::list<desktoppacket_t> desktop_queue_t;

    class ClientUser : public teamtalk::User
    {
    public:
        ClientUser(int userid, 
                   class ClientNode* clientnode,
                   class ClientListener* listener);
        virtual ~ClientUser();

        void ResetAllStreams();
        void ResetInactiveStreams();
            
        //Timer* methods called by ClientNode's TimerEvent()
        int TimerMonitorVoicePlayback();
        int TimerMonitorAudioFilePlayback();
        int TimerMonitorVideoFilePlayback();
        int TimerDesktopDelayedAck();

        void SetChannel(clientchannel_t& chan);
        clientchannel_t GetChannel() const { return m_channel.lock(); }

        void SetUsername(const ACE_TString& name){ m_username = name; }
        const ACE_TString& GetUsername() const { return m_username; }

        void SetUserType(UserTypes usertype) { m_usertype = usertype; }
        UserTypes GetUserType() const { return m_usertype; }

        void SetUserData(int userdata) { m_userdata = userdata; }
        int GetUserData() const { return m_userdata; }

        void AddVoicePacket(const VoicePacket& audpkt,
                            const struct SoundProperties& sndprop,
                            class VoiceLogger& voice_logger, bool allowrecord);
        void AddAudioFilePacket(const AudioFilePacket& audpkt,
                                const struct SoundProperties& sndprop);
        void AddVideoCapturePacket(const VideoCapturePacket& p,
                                   const ClientChannel& chan);
        void AddVideoFilePacket(const VideoFilePacket& p,
                                const ClientChannel& chan);
        void AddPacket(const DesktopPacket& p, const ClientChannel& chan);
        void GetAckedDesktopPackets(uint8_t& session_id,
                                    uint32_t& upd_time,
                                    std::set<uint16_t>& acked) const;
        void AddPacket(const DesktopCursorPacket& p,
                       const ClientChannel& chan);
        void AddPacket(const DesktopInputPacket& p,
                       const ClientChannel& chan);

        const ClientUserStats& GetStatistics() const { return m_stats; }

        bool IsAudioActive(StreamType stream_type) const;

        void SetPlaybackStoppedDelay(StreamType stream_type, int msec);
        int GetPlaybackStoppedDelay(StreamType stream_type) const;

        void SetVolume(StreamType stream_type, int volume);
        int GetVolume(StreamType stream_type) const;

        void SetMute(StreamType stream_type, bool mute);
        bool IsMute(StreamType stream_type) const;

        void SetPosition(StreamType stream_type, float x, float y, float z);
        void GetPosition(StreamType stream_type, float& x, float& y, float& z) const;

        void SetStereo(StreamType stream_type, bool left, bool right);
        void GetStereo(StreamType stream_type, bool& left, bool& right) const;

        //calls ResetVoicePlayer() and ResetAudioFilePlayer()
        void ResetAudioPlayers(bool reset_snderr);

        void ResetVoicePlayer();
        void ResetAudioFilePlayer();

        ACE_Message_Block* GetVideoCaptureFrame();
        bool GetVideoCaptureCodec(VideoCodec& codec) const;
        void CloseVideoCapturePlayer();

        ACE_Message_Block* GetVideoFileFrame();
        bool GetVideoFileCodec(VideoCodec& codec) const;
        void CloseVideoFilePlayer();

        //ClientUser's desktop session
        bool GetDesktopWindow(DesktopWindow& wnd);
        bool GetDesktopWindow(char* buffer, int length);
        desktop_viewer_t GetDesktopSession() const { return m_desktop; }
        void CloseDesktopSession();
        void ResetDesktopInputRx();
        uint8_t GetNextDesktopInputRxPacketNo() const { return m_desktop_input_rx_pktno; }
        //ClientNode to this user's desktop session
        std::list< desktopinput_pkt_t >& GetDesktopInputTxQueue() { return m_desktop_input_tx; }
        uint8_t NextDesktopInputTxPacket() { return m_desktop_input_tx_pktno++; }
        std::list< desktopinput_pkt_t >& GetDesktopInputRtxQueue() { return m_desktop_input_rtx; }
        void ResetDesktopInputTx();

        void SetLocalSubscriptions(Subscriptions mask);
        Subscriptions GetLocalSubscriptions() const { return m_localsubscriptions; }
        void SetPeerSubscriptions(Subscriptions mask){ m_peersubscriptions = mask; }
        Subscriptions GetPeerSubscriptions() const { return m_peersubscriptions; }

        bool LocalSubscribes(const FieldPacket& packet) const;
        bool PeerSubscribes(const FieldPacket& packet) const;
        bool LocalSubscribes(Subscriptions sub_mask) const
        { return GetLocalSubscriptions() & sub_mask; }

        void SetAudioFolder(const ACE_TString& audfolder) { m_audiofolder = audfolder; }
        const ACE_TString& GetAudioFolder() const { return m_audiofolder; }
        ACE_TString GetAudioFileName() const;
        void SetAudioFileVariables(const ACE_TString& vars) { m_vlog_vars = vars; }
        const ACE_TString& GetAudioFileVariables() const { return m_vlog_vars; }
        void SetAudioFileFormat(AudioFileFormat aff);
        AudioFileFormat GetAudioFileFormat() const;
        ACE_INT32 IncVoiceLogCounter() { return ++m_voicelog_counter; }

        void SetAudioStreamBufferSize(StreamType stream_type, int msec);
        int GetAudioStreamBufferSize(StreamType stream_type) const;


    private:
        audio_player_t LaunchAudioPlayer(const teamtalk::AudioCodec& codec,
                                         const struct SoundProperties& sndprop,
                                         StreamType stream_type);
        bool LaunchVoicePlayer(const teamtalk::AudioCodec& codec,
                               const struct SoundProperties& sndprop);
        bool LaunchAudioFilePlayer(const teamtalk::AudioCodec& codec,
                                   const struct SoundProperties& sndprop);

        void SetDirtyProps();

        ClientNode* m_clientnode;
        ClientListener* m_listener;
        ClientUserStats m_stats;
        ACE_TString m_username;
        UserTypes m_usertype;
        int m_userdata;
        std::weak_ptr< ClientChannel > m_channel;

        //voice playback
        audio_player_t m_voice_player;
        bool m_voice_active;
        int m_voice_buf_msec;

        //video playback
#if defined(ENABLE_VPX)
        webm_player_t m_vidcap_player;
#endif

        //audio file playback
        audio_player_t m_audiofile_player;
        bool m_audiofile_active;
        int m_media_buf_msec;

        //video file playback
#if defined(ENABLE_VPX)
        webm_player_t m_videofile_player;
#endif

        //desktop sharing
        desktop_viewer_t m_desktop;
        desktop_queue_t m_desktop_queue; //queue for desktop packets until session becomes valid
        map_desktoppacket_t m_block_fragments; //fragmented desktop packets
        map_dup_blocks_t m_dup_blocks;
        uint16_t m_desktop_packets_expected;
        std::set<uint16_t> m_acked_desktoppackets;
    
        //desktop input received from user (for ClientNode's desktop session)
        std::list<desktopinput_pkt_t> m_desktop_input_rx;
        uint8_t m_desktop_input_rx_pktno; //packet_no of next expect packet

        //TT instance's tx queue for desktop input to this user
        std::list< desktopinput_pkt_t > m_desktop_input_tx, m_desktop_input_rtx;
        uint8_t m_desktop_input_tx_pktno;

        //sound state
        bool m_snddev_error;
        bool m_snd_duplexmode;

        //gaining audio
        float m_voice_position[3], m_audiofile_position[3];
        int m_voice_volume, m_audiofile_volume;
        bool m_voice_mute, m_audiofile_mute;
        int m_voice_stopped_delay, m_audiofile_stopped_delay;
        int m_voice_gain_level, m_audiofile_gain_level;
        StereoMask m_voice_stereo, m_audiofile_stereo;

        Subscriptions m_localsubscriptions, m_peersubscriptions;
            
        //audio storage
        ACE_TString m_audiofolder;
        ACE_TString m_vlog_vars;
        AudioFileFormat m_aff;
        ACE_UINT32 m_voicelog_counter;
    };
}
#endif
