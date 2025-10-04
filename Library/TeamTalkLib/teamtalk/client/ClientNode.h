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

#if !defined(CLIENTNODE_H)
#define CLIENTNODE_H

#include "ClientNodeBase.h"

#include "AudioContainer.h"
#include "AudioMuxer.h"
#include "AudioThread.h"
#include "Client.h"
#include "ClientChannel.h"
#include "ClientNodeEvent.h"
#include "ClientUser.h"
#include "DesktopShare.h"
#include "FileNode.h"
#include "VideoThread.h"

#include "avstream/AudioInputStreamer.h"
#include "avstream/AudioResampler.h"
#include "avstream/MediaPlayback.h"
#include "avstream/MediaStreamer.h"
#include "avstream/SoundSystem.h"
#include "avstream/VideoCapture.h"
#include "codec/MediaUtil.h"
#include "teamtalk/Commands.h"
#include "teamtalk/Common.h"
#include "teamtalk/PacketHandler.h"
#include "teamtalk/PacketHelper.h"
#include "teamtalk/PacketLayout.h"
#include "teamtalk/StreamHandler.h"

#include <ace/INET_Addr.h>
#include <ace/Message_Block.h>
#include <ace/Message_Queue.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Time_Value.h>
#if defined(ENABLE_ENCRYPTION)
#include <ace/SSL/SSL_Context.h>
#endif

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <vector>

static const auto CLIENT_DESKTOPNAK_TIMEOUT          = ACE_Time_Value(4); //close a desktop session
static const auto CLIENT_QUERY_MTU_INTERVAL          = ACE_Time_Value(0, 500000); //time between MTU query packets
static const auto CLIENT_DESKTOPINPUT_RTX_TIMEOUT    = ACE_Time_Value(1);
static const auto CLIENT_DESKTOPINPUT_ACK_DELAY      = ACE_Time_Value(0, 10000);

constexpr auto MTU_QUERY_RETRY_COUNT = 20; //20 * 500ms = 10 seconds for MTU query (CLIENT_QUERY_MTU_INTERVAL)

#if defined(_DEBUG)

#define GUARD_REACTOR(this_obj)                         \
    guard_t g( this_obj->ReactorLock() );               \
    /*PROFILER_ST(ACE_TEXT("Thread"));*/                \
    this_obj->m_reactorlock_thr_id = ACE_Thread::self()
#else
#define GUARD_REACTOR(this_obj)                         \
    guard_t g(this_obj->ReactorLock())
#endif

namespace teamtalk {

    //ensure DLL compliance
    enum ClientFlag
    {
        CLIENT_CLOSED                    = 0x00000000,

        CLIENT_SNDINPUT_READY            = 0x00000001,
        CLIENT_SNDOUTPUT_READY           = 0x00000002,
        CLIENT_SNDINOUTPUT_DUPLEX        = 0x00000004,
        CLIENT_SNDINPUT_VOICEACTIVATED   = 0x00000008,
        CLIENT_SNDINPUT_VOICEACTIVE      = 0x00000010,
        CLIENT_SNDOUTPUT_MUTE            = 0x00000020,
        CLIENT_SNDOUTPUT_AUTO3DPOSITION  = 0x00000040,

        CLIENT_VIDEOCAPTURE_READY        = 0x00000080,
            
        CLIENT_TX_VOICE                  = 0x00000100,
        CLIENT_TX_VIDEOCAPTURE           = 0x00000200,
        CLIENT_TX_DESKTOP                = 0x00000400,

        CLIENT_DESKTOP_ACTIVE            = 0x00000800,
        CLIENT_MUX_AUDIOFILE             = 0x00001000,

        CLIENT_CONNECTING                = 0x00002000,
        CLIENT_CONNECTED                 = 0x00004000,
        CLIENT_CONNECTION                = CLIENT_CONNECTING | CLIENT_CONNECTED,

        CLIENT_AUTHORIZED                = 0x00008000,

        CLIENT_STREAM_AUDIOFILE          = 0x00010000,
        CLIENT_STREAM_VIDEOFILE          = 0x00020000
    };

    using ClientFlags = ACE_UINT32;

    struct ClientStats
    {
        ACE_INT64 udpbytes_sent = 0;
        ACE_INT64 udpbytes_recv = 0;
        ACE_INT64 voicebytes_sent = 0;
        ACE_INT64 voicebytes_recv = 0;
        ACE_INT64 vidcapbytes_sent = 0;
        ACE_INT64 vidcapbytes_recv = 0;
        ACE_INT64 mediafile_audio_bytes_sent = 0;
        ACE_INT64 mediafile_audio_bytes_recv = 0;
        ACE_INT64 mediafile_video_bytes_sent = 0;
        ACE_INT64 mediafile_video_bytes_recv = 0;
        ACE_INT64 desktopbytes_sent = 0;
        ACE_INT64 desktopbytes_recv = 0;
        ACE_INT32 udpping_time = -1;
        ACE_INT32 tcpping_time = -1;
        //internal use
        ACE_UINT32 tcp_silence_sec = 0;
        ACE_UINT32 udp_silence_sec = 0;
        ACE_Time_Value ping_issue_time;
        bool udp_ping_dirty = true;
        bool tcp_ping_dirty = true;
        int streamcapture_delay_msec = 0;
        ClientStats() = default;
    };

    struct ClientKeepAlive
    {
        // no reply from server after this then report connection lost
        ACE_Time_Value connection_lost_timeout = ACE_Time_Value(180, 0);
        // Defaults to 1/2 of server's user-timeout
        ACE_Time_Value tcp_keepalive_interval;
        // Delay between UDP keepalive packets
        ACE_Time_Value udp_keepalive_interval = ACE_Time_Value(10, 0);
        // UDP keepalive retransmission interval
        ACE_Time_Value udp_keepalive_rtx = ACE_Time_Value(1, 0);
        //Interval between UDP connect packets
        ACE_Time_Value udp_connect_interval = ACE_Time_Value(0, 500000);
        // Time before giving up on udp connect to server
        ACE_Time_Value udp_connect_timeout = ACE_Time_Value(10, 0);
    };

    soundsystem::SoundDeviceFeatures GetSoundDeviceFeatures(const SoundDeviceEffects& effects);

    using filenode_t = std::shared_ptr< class FileNode >;

    class ClientNode
        : public ClientNodeBase
        , public PacketListener
        , public StreamListener<DefaultStreamHandler::StreamHandler_t>
#if defined(ENABLE_ENCRYPTION)
        , public StreamListener<CryptStreamHandler::StreamHandler_t>
#endif
        , public soundsystem::StreamCapture
        , public FileTransferListener
    {
    public:
        ClientNode(const ACE_TString& version, ClientListener* listener);
        ~ClientNode() override;

#if defined(_DEBUG)
        uint32_t m_active_timerid = 0;
#endif
        
        ACE_Recursive_Thread_Mutex& LockSndprop() { return m_sndgrp_lock; }
        VoiceLogger& GetVoiceLogger() override;
        AudioContainer& GetAudioContainer();

        //server properties
        bool GetServerInfo(ServerInfo& info);
        bool GetClientStatistics(ClientStats& stats);

        ClientFlags GetFlags() const { return m_flags; }
        int GetUserID() const override { return m_myuserid; }
        const UserAccount& GetMyUserAccount() const { return m_myuseraccount; }

        clientuser_t GetUser(int userid, bool include_local = false);
        clientuser_t GetUserByUsername(const ACE_TString& username);
        void GetUsers(std::set<int>& userids);
        clientchannel_t GetRootChannel();
        clientchannel_t GetMyChannel();
        int GetChannelID() override;
        clientchannel_t GetChannel(int channelid);
        ACE_TString GetChannelPath(int channelid);
        bool GetChannelProp(int channelid, ChannelProp& prop);

        //SoundSystem
        bool InitSoundInputDevice(int inputdeviceid);
        bool InitSoundOutputDevice(int outputdeviceid);
        bool InitSoundDuplexDevices(int inputdeviceid, int outputdeviceid);
        bool CloseSoundInputDevice();
        bool CloseSoundOutputDevice();
        bool CloseSoundDuplexDevices();
        bool SoundDuplexMode() override;
        bool SetSoundDeviceEffects(const SoundDeviceEffects& effects);
        SoundDeviceEffects GetSoundDeviceEffects();
        const SoundProperties& GetSoundProperties() const { return m_soundprop; }

        bool SetSoundOutputVolume(int volume);
        int GetSoundOutputVolume();

        bool EnableVoiceTransmission(bool enable);
        int GetCurrentVoiceLevel();
        void SetVoiceActivationLevel(int voicelevel);
        int GetVoiceActivationLevel() const;
        bool EnableVoiceActivation(bool enable);
        void SetVoiceActivationStoppedDelay(int msec);
        int GetVoiceActivationStoppedDelay() const;

        bool EnableAutoPositioning(bool enable);
        bool AutoPositionUsers();    //position users in 3D

        bool EnableAudioBlockCallback(int userid, StreamTypes sts,
                                      const media::AudioFormat& outfmt,
                                      bool enable);
        // user provided audio stream that replaces voice input stream
        bool QueueAudioInput(const media::AudioFrame& frm, int streamid);
        
        bool MuteAll(bool muteall);

        bool SetVoiceGainLevel(int gainlevel);
        int GetVoiceGainLevel();

        bool SetSoundPreprocess(const AudioPreprocessor& preprocessor);
        void SetSoundInputTone(StreamTypes streams, int frequency);

        bool StartRecordingMuxedAudioFile(const AudioCodec& codec,
                                          StreamTypes sts,
                                          const ACE_TString& filename,
                                          AudioFileFormat aff);
        bool StartRecordingMuxedAudioFile(int channelid,
                                          StreamTypes sts,
                                          const ACE_TString& filename,
                                          AudioFileFormat aff);
        void StopRecordingMuxedAudioFile();
        void StopRecordingMuxedAudioFile(int channelid);
        
        bool StartMTUQuery();

        //stream media file (DirectShow wrapper)
        bool StartStreamingMediaFile(const ACE_TString& filename,
                                     uint32_t offset, bool paused,
                                     const AudioPreprocessor& preprocessor,
                                     const VideoCodec& vid_codec);
        bool UpdateStreamingMediaFile(uint32_t offset, bool paused,
                                      const AudioPreprocessor& preprocessor,
                                      const VideoCodec& vid_codec);
        void StopStreamingMediaFile(bool killtimer = true);

        // playback local media file
        int InitMediaPlayback(const ACE_TString& filename, uint32_t offset,
                              bool paused, const AudioPreprocessor& preprocessor);
        bool UpdateMediaPlayback(int id, uint32_t offset, bool paused, 
                                 const AudioPreprocessor& preprocessor, bool initial = false);
        bool StopMediaPlayback(int id);

        void MediaPlaybackStatus(int id, const MediaFileProp& mfp, MediaStreamStatus status);
        void MediaPlaybackAudio(int id, const media::AudioFrame& frm);


        //video capture
        bool InitVideoCapture(const ACE_TString& src_id,
                              const media::VideoFormat& cap_format);
        void CloseVideoCapture();

        bool OpenVideoCaptureSession(const VideoCodec& codec);
        void CloseVideoCaptureSession();

        //returns VideoFrame* in ACE_Message_Block*
        ACE_Message_Block* AcquireVideoCaptureFrame();

        //desktop sharing
        //returns -1 on error, 0 no changes, >0 num packets
        int SendDesktopWindow(int width, int height, RGBMode rgb, 
                              DesktopProtocol protocol, 
                              const char* bmp_buf, int bmp_buf_len);
        bool CloseDesktopWindow();
        bool SendDesktopCursor(int x, int y);
        bool SendDesktopInput(int userid,
                              const std::vector<DesktopInput>& inputs);

        bool Connect(bool encrypted, const ACE_TString& hostaddr,
                     u_short tcpport, u_short udpport, const ACE_TString& sysid,
                     const ACE_TString& localaddr,
                     u_short local_tcpport = 0, u_short local_udpport = 0);
        bool Connect(bool encrypted, const ACE_INET_Addr& hosttcpaddr,
                     const ACE_INET_Addr* localtcpaddr);
        void Disconnect();
        ACE_INET_Addr GetLocalAddr();
#if defined(ENABLE_ENCRYPTION)
        ACE_SSL_Context* SetupEncryptionContext();
#endif
        
        //StreamListener
#if defined(ENABLE_ENCRYPTION)
        void OnOpened(CryptStreamHandler::StreamHandler_t& handler) override;
        void OnClosed(CryptStreamHandler::StreamHandler_t& handler) override;
        bool OnReceive(CryptStreamHandler::StreamHandler_t& handler, const char* buff, int len) override;
        bool OnSend(CryptStreamHandler::StreamHandler_t& handler) override;
#endif
        void OnOpened(DefaultStreamHandler::StreamHandler_t& handler) override;
        void OnClosed(DefaultStreamHandler::StreamHandler_t& handler) override;
        bool OnReceive(DefaultStreamHandler::StreamHandler_t& handler, const char* buff, int len) override;
        bool OnSend(DefaultStreamHandler::StreamHandler_t& handler) override;

        //set keep alive timer intervals
        void UpdateKeepAlive(const ClientKeepAlive& keepalive);
        ClientKeepAlive GetKeepAlive();

        //TimerListener - reactor thread
        int TimerEvent(ACE_UINT32 timer_event_id, long userdata) override;

        //Audio encoder callback - separate thread
        void EncodedAudioVoiceFrame(const teamtalk::AudioCodec& codec,
                                    const char* enc_data, int enc_length,
                                    const std::vector<int>& enc_frame_sizes,
                                    const media::AudioFrame& org_frame);

        void EncodedAudioFileFrame(const teamtalk::AudioCodec& codec,
                                   const char* enc_data, int enc_length,
                                   const std::vector<int>& enc_frame_sizes,
                                   const media::AudioFrame& org_frame);
        
        //Video encoder - separate thread
        bool EncodedVideoCaptureFrame(ACE_Message_Block* org_frame,
                                      const char* enc_data, int enc_len,
                                      ACE_UINT32 packet_no,
                                      ACE_UINT32 timestamp);
        bool EncodedVideoFileFrame(ACE_Message_Block* org_frame,
                                   const char* enc_data, int enc_len,
                                   ACE_UINT32 packet_no,
                                   ACE_UINT32 timestamp);

        // SoundSystem listener - separate thread
        void StreamCaptureCb(const soundsystem::InputStreamer& streamer,
                             const short* buffer, int n_samples) override;
        void StreamDuplexCb(const soundsystem::DuplexStreamer& streamer,
                            const short* input_buffer, short* prev_output_buffer, 
                            int n_samples) override;
        soundsystem::SoundDeviceFeatures GetCaptureFeatures() override;
        soundsystem::SoundDeviceFeatures GetDuplexFeatures() override;

        //VideoCapture listener - separate thread
        bool VideoCaptureRGB32Callback(media::VideoFrame& video_frame,
                                       ACE_Message_Block* mb_video);
        bool VideoCaptureEncodeCallback(media::VideoFrame& video_frame,
                                        ACE_Message_Block* mb_video);
        bool VideoCaptureDualCallback(media::VideoFrame& video_frame,
                                      ACE_Message_Block* mb_video);

        //Media stream listener - separate thread
        bool MediaStreamVideoCallback(media::VideoFrame& video_frame,
                                      ACE_Message_Block* mb_video);
        bool MediaStreamAudioCallback(media::AudioFrame& audio_frame,
                                      ACE_Message_Block* mb_audio);
        void MediaStreamStatusCallback(const MediaFileProp& mfp,
                                       MediaStreamStatus status);

        // AudioInputStreamer listener - separate thread
        bool AudioInputCallback(media::AudioFrame& audio_frame,
                                ACE_Message_Block* mb_audio);
        void AudioInputStatusCallback(const AudioInputStatus& ais);

        // AudioMuxer callback - separate thread
        void AudioMuxCallback(teamtalk::StreamTypes sts, const media::AudioFrame& audio_frame);
        // AudioPlayer listener - separate thread
        void AudioUserCallback(int userid, StreamType st,
                               const media::AudioFrame& audio_frame) override;

        // FileNode listener - reactor thread
        void OnFileTransferStatus(const teamtalk::FileTransfer& transfer) override;

        bool GetTransferInfo(int transferid, FileTransfer& transfer);
        bool CancelFileTransfer(int transferid);

        //PacketListener - reactor thread
        void ReceivedPacket(PacketHandler* ph,
                            const char* packet_data, int packet_size, 
                            const ACE_INET_Addr& addr) override;
        void SendPackets() override; //send packets - reactor thread

        bool QueuePacket(FieldPacket* packet) override;

        //Send packet to address
        int SendPacket(const FieldPacket& packet, const ACE_INET_Addr& addr);

        //Issue command (Client->Server)
        int DoPing(bool issue_cmdid);
        int DoLogin(const ACE_TString& nickname, const ACE_TString& username, 
                    const ACE_TString& password, const ACE_TString& clientname);
        int DoLogout();
        int DoJoinChannel(const ChannelProp& chanprop, bool forceexisting);
        int DoLeaveChannel();
        int DoChangeNickname(const ACE_TString& newnick);
        int DoChangeStatus(int statusmode, const ACE_TString& statusmsg);
        int DoTextMessage(const TextMessage& msg);
        int DoKickUser(int userid, int channelid);
        int DoChannelOperator(int userid, int channelid, 
                              const ACE_TString& oppasswd, bool op);
        int DoFileSend(int channelid, const ACE_TString& localfilepath);
        int DoFileRecv(int channelid, const ACE_TString& localfilepath, 
                       const ACE_TString& remotefilename);
        int DoFileDelete(int channelid, const ACE_TString& filename);
        int DoSubscribe(int userid, Subscriptions subscript);
        int DoUnsubscribe(int userid, Subscriptions subscript);
        int DoQueryServerStats();
        int DoQuit();

        int DoMakeChannel(const ChannelProp& chanprop);
        int DoUpdateChannel(const ChannelProp& chanprop);
        int DoRemoveChannel(int channelid);
        int DoMoveUser(int userid, int channelid);
        int DoUpdateServer(const ServerInfo& serverprop);
        int DoBanUser(int userid, const BannedUser& ban);
        int DoUnBanUser(const BannedUser& ban);
        int DoListBans(int chanid, int index, int count);
        int DoListUserAccounts(int index, int count);
        int DoNewUserAccount(const UserAccount& user);
        int DoDeleteUserAccount(const ACE_TString& username);
        int DoSaveConfig();

    private:
        void OnOpened();
        void OnClosed();
        bool OnReceive(const char* buff, int len);
        bool OnSend(ACE_Message_Queue_Base& msgqueue);
        //Calls corresponding Handle* method
        bool ProcessCommand(const ACE_CString& cmdline);
        //Command handlers (Server->Client)
        void HandleWelcome(const mstrings_t& properties);
        void HandleAccepted(const mstrings_t& properties);
        void HandleServerUpdate(const mstrings_t& properties);
        void HandleKeepAlive(const mstrings_t& properties);
        void HandleLoggedIn(const mstrings_t& properties);
        void HandleLoggedOut(const mstrings_t& properties);
        void HandleCmdError(const mstrings_t& properties);
        void HandleAddUser(const mstrings_t& properties);
        void HandleUpdateUser(const mstrings_t& properties);
        void HandleRemoveUser(const mstrings_t& properties);
        void HandleAddChannel(const mstrings_t& properties);
        void HandleUpdateChannel(const mstrings_t& properties);
        void HandleRemoveChannel(const mstrings_t& properties);
        void HandleJoinedChannel(const mstrings_t& properties);
        void HandleLeftChannel(const mstrings_t& properties);
        void HandleAddFile(const mstrings_t& properties);
        void HandleRemoveFile(const mstrings_t& properties);
        void HandleTextMessage(const mstrings_t& properties);
        void HandleKicked(const mstrings_t& properties);
        void HandleBannedUser(const mstrings_t& properties);
        void HandleUserAccount(const mstrings_t& properties);
        void HandleAddUserAccount(const mstrings_t& properties);
        void HandleRemoveUserAccount(const mstrings_t& properties);
        void HandleOk(const mstrings_t& properties);
        void HandleFileAccepted(const mstrings_t& properties);
        void HandleServerStats(const mstrings_t& properties);
        void HandleBeginCmd(const mstrings_t& properties);
        void HandleEndCmd(const mstrings_t& properties);

        int TransmitCommand(const ACE_TString& command, int cmdid);

        void JoinChannel(clientchannel_t& chan);
        void LeftChannel(ClientChannel& chan);

        void LoggedOut();

        void RecreateUdpSocket();

        int TimerOneSecond();
        int TimerUdpKeepAlive();
        int TimerBuildDesktopPackets();
        int TimerDesktopPacketRtx();
        int TimerDesktopNakPacket();
        int TimerQueryMtu(int mtu_index);

        //audio start/stop/update
        void OpenAudioCapture(const AudioCodec& codec);
        void CloseAudioCapture();
        void QueueAudioCapture(media::AudioFrame& audframe);
        void QueueVoiceFrame(media::AudioFrame& audframe,
                             ACE_Message_Block* mb_audio = nullptr);

        void SendVoicePacket(const VoicePacket& packet);
        void SendAudioFilePacket(const AudioFilePacket& packet);

        void ReceivedHelloAckPacket(const HelloPacket& packet,
                                    const ACE_INET_Addr& addr); //called when ACK packet is received from server
        void ReceivedKeepAliveReplyPacket(const KeepAlivePacket& packet,
                                          const ACE_INET_Addr& addr);
        void ReceivedDesktopPacket(ClientUser& user,
                                   const ClientChannel& chan,
                                   const DesktopPacket& desktop_pkt);
        void ReceivedDesktopAckPacket(const DesktopAckPacket& ack_pkt);
        void ReceivedDesktopNakPacket(const DesktopNakPacket& nak_pkt);
        void ReceivedDesktopCursorPacket(const DesktopCursorPacket& csr_pkt);
        void ReceivedDesktopInputPacket(const DesktopInputPacket& csr_pkt);
        void ReceivedDesktopInputAckPacket(const DesktopInputAckPacket& ack_pkt);
        void CloseDesktopSession(bool stop_nak_timer);

        void ResetAudioPlayers();

        // shared sound system instance
        soundsystem::soundsystem_t m_soundsystem;
        std::atomic<ClientFlags> m_flags; //Mask of ClientFlag-enum
        // active sound groups (shared master volume)
        ACE_Recursive_Thread_Mutex m_sndgrp_lock;
        SoundProperties m_soundprop;
        //log voice to files
        voicelogger_t m_voicelogger;
        // audio container for getting raw audio from users
        AudioContainer m_audiocontainer;
        // muxed audio into files
        ChannelAudioMuxer m_channelrecord;
        // muxed audio into stream
        audiomuxer_t m_audiomuxer_stream;
        //TCP connector
        connector_t m_connector;
        DefaultStreamHandler::StreamHandler_t* m_def_stream = nullptr;
#if defined(ENABLE_ENCRYPTION)
        crypt_connector_t m_crypt_connector;
        CryptStreamHandler::StreamHandler_t* m_crypt_stream = nullptr;
#endif
        //TCP send/receive buffer for StreamHandler
        ACE_CString m_recvbuffer, m_sendbuffer;

        //The voice packet receiver
        PacketHandler m_packethandler;

        ServerInfo m_serverinfo;
        ClientStats m_clientstats;
        ClientKeepAlive m_keepalive;

        //channels and users
        using musers_t = std::map<int, clientuser_t>;
        musers_t m_users;

        clientchannel_t m_rootchannel;
        clientchannel_t m_mychannel;
        int m_myuserid = 0;
        UserAccount m_myuseraccount;
        clientuser_t m_local_voicelog;

        //cmdid -> filetransfer
        using filetransfer_q_t = std::map<int, FileTransfer>;
        //file transfer queue
        filetransfer_q_t m_waitingTransfers;
        //active file transfers
        using filenodes_t = std::map<int, filenode_t>;
        filenodes_t m_filetransfers;

        //audio resampler for capture
        audio_resampler_t m_capture_resampler;
        std::vector<short> m_capture_buffer;
        //audio resampler for playback (in duplex mode)
        audio_resampler_t m_playback_resampler;
        std::vector<short> m_playback_buffer;

        //encode voice from sound input
        AudioThread m_voice_thread;
        uint8_t m_voice_stream_id = 0; //0 means not used
        uint16_t m_voice_pkt_counter = 0;
        std::atomic<bool> m_voice_tx_closed{false}; // CLIENT_TX_VOICE was toggled (transmit next packet)

        //encode video from video capture
        vidcap::videocapture_t m_vidcap;
        VideoThread m_vidcap_thread;
        ACE_Message_Queue<ACE_MT_SYNCH> m_local_vidcapframes; //local RGB32 video frames
        uint8_t m_vidcap_stream_id = 0; //0 means not used

        //media streamer to channels
        mediafile_streamer_t m_mediafile_streamer;
        uint8_t m_mediafile_stream_id = 0; //0 means not used

        //encode audio of media file
        AudioThread m_audiofile_thread;
        uint16_t m_audiofile_pkt_counter = 0;

        //encode video of media file
        video_thread_t m_videofile_thread;

        // local playback of media files
        std::map<int, mediaplayback_t> m_mediaplayback_streams;
        uint16_t m_mediaplayback_counter = 0;

        // audio input streamer (replaces voice stream)
        audioinput_streamer_t m_audioinput_voice;

        //desktop session
        desktop_initiator_t m_desktop;
        desktop_transmitter_t m_desktop_tx;
        desktop_nak_tx_t m_desktop_nak_tx;
        uint8_t m_desktop_session_id = 0;

        //UDP packets waiting for transmission
        PacketQueue m_tx_queue;

        //unique IDs for Do* commands
        uint16_t m_cmdid_counter = 0;
        uint16_t m_current_cmdid = 0;

        //local UDP sockets to use (stored in case UDP socket must be recreated)
        ACE_INET_Addr m_localTcpAddr, m_localUdpAddr;

        //query MTU (timestamp -> MTU packet)
        using mtu_packets_t = std::map<uint32_t, ka_mtu_packet_t>;
        mtu_packets_t m_mtu_packets;
        uint16_t m_mtu_data_size = MAX_PAYLOAD_DATA_SIZE, m_mtu_max_payload_size = MAX_PACKET_PAYLOAD_SIZE;

        //the client's version number
        ACE_TString m_version;

        //The listener of the ClientNode instance
        ClientListener* m_listener = nullptr;
    };
} // namespace teamtalk

#endif
