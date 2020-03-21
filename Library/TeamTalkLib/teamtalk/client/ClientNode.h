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

#include "Client.h"
#include "ClientChannel.h"
#include "ClientUser.h"
#include "AudioThread.h"
#include "FileNode.h"
#include "VideoThread.h"
#include "VoiceLogger.h"
#include "AudioMuxer.h"
#include "AudioContainer.h"
#include "DesktopShare.h"
#include <myace/TimerHandler.h>
#include <myace/MyACE.h>
#include <teamtalk/StreamHandler.h>
#include <teamtalk/Common.h>
#include <teamtalk/PacketHandler.h>
#include <avstream/VideoCapture.h>
#include <avstream/MediaPlayback.h>

#include <avstream/MediaStreamer.h>
#include <avstream/AudioInputStreamer.h>

// ACE
#include <ace/Reactor.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Thread_Mutex.h>
#include <ace/INET_Addr.h>
#include <ace/Time_Value.h>
#include <ace/FILE_Connector.h>
#include <ace/Connector.h> 
#include <ace/SString.h>

#if defined(ENABLE_ENCRYPTION)
#include <ace/SSL/SSL_SOCK_Connector.h>
#else
#include <ace/SOCK_Connector.h>
#endif

#include <atomic>

#define CLIENT_DESKTOPNAK_TIMEOUT           ACE_Time_Value(4) //close a desktop session
#define CLIENT_QUERY_MTU_INTERVAL           ACE_Time_Value(0, 500000) //time between MTU query packets
#define CLIENT_DESKTOPINPUT_RTX_TIMEOUT     ACE_Time_Value(1)
#define CLIENT_DESKTOPINPUT_ACK_DELAY       ACE_Time_Value(0, 10000)

#define MTU_QUERY_RETRY_COUNT 20 //20 * 500ms = 10 seconds for MTU query (CLIENT_QUERY_MTU_INTERVAL)

#define SOUNDDEVICE_IGNORE_ID -1

#ifdef _DEBUG
#define ASSERT_REACTOR_LOCKED(this_obj)                         \
    TTASSERT(this_obj->m_reactor_thr_id == ACE_Thread::self())
#define GUARD_REACTOR(this_obj)                         \
    guard_t g( this_obj->reactor_lock() );              \
    /*PROFILER_ST(ACE_TEXT("Thread"));*/                \
    this_obj->m_reactor_thr_id = ACE_Thread::self()
#else
#define ASSERT_REACTOR_LOCKED(...)     (void)0
#define GUARD_REACTOR(this_obj)                 \
    guard_t g(this_obj->reactor_lock())
#endif

enum ClientTimer
{
    TIMER_ONE_SECOND_ID                     = 1, //timer for checking things every second
    TIMER_TCPKEEPALIVE_ID                   = 2,
    TIMER_UDPCONNECT_ID                     = 3, //connect to server with UDP
    TIMER_UDPKEEPALIVE_ID                   = 4,
    TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID      = 8,
    TIMER_DESKTOPNAKPACKET_TIMEOUT_ID       = 9,
    TIMER_BUILD_DESKTOPPACKETS_ID           = 10,
    TIMER_QUERY_MTU_ID                      = 11,
    TIMER_STOP_AUDIOINPUT                   = 12,

    //User instance timers (termination not handled by ClientNode::StopTimer())
    USER_TIMER_MASK                         = 0x8000,

    USER_TIMER_VOICE_PLAYBACK_ID            = USER_TIMER_MASK + 2,
    USER_TIMER_MEDIAFILE_AUDIO_PLAYBACK_ID  = USER_TIMER_MASK + 3,
    USER_TIMER_MEDIAFILE_VIDEO_PLAYBACK_ID  = USER_TIMER_MASK + 4,
    USER_TIMER_DESKTOPACKPACKET_ID          = USER_TIMER_MASK + 5,
    USER_TIMER_STOP_STREAM_MEDIAFILE_ID     = USER_TIMER_MASK + 6,
    USER_TIMER_DESKTOPINPUT_RTX_ID          = USER_TIMER_MASK + 7,
    USER_TIMER_DESKTOPINPUT_ACK_ID          = USER_TIMER_MASK + 8,
    USER_TIMER_REMOVE_FILETRANSFER_ID       = USER_TIMER_MASK + 9,
    USER_TIMER_UPDATE_USER                  = USER_TIMER_MASK + 10,
    USER_TIMER_REMOVE_LOCALPLAYBACK         = USER_TIMER_MASK + 11
};

#define TIMERID_MASK            0xFFFF
#define USER_TIMER_USERID_MASK  0xFFFF0000
#define USER_TIMER_USERID_SHIFT 16

#define USER_TIMERID(timerid, userid) ((userid << USER_TIMER_USERID_SHIFT) | timerid)
#define TIMER_USERID(timerid) ((timerid >> USER_TIMER_USERID_SHIFT) & 0xFFFF)

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

    typedef ACE_UINT32 ClientFlags;

    struct ClientStats
    {
        ACE_INT64 udpbytes_sent;
        ACE_INT64 udpbytes_recv;
        ACE_INT64 voicebytes_sent;
        ACE_INT64 voicebytes_recv;
        ACE_INT64 vidcapbytes_sent;
        ACE_INT64 vidcapbytes_recv;
        ACE_INT64 mediafile_audio_bytes_sent;
        ACE_INT64 mediafile_audio_bytes_recv;
        ACE_INT64 mediafile_video_bytes_sent;
        ACE_INT64 mediafile_video_bytes_recv;
        ACE_INT64 desktopbytes_sent;
        ACE_INT64 desktopbytes_recv;
        ACE_INT32 udpping_time;
        ACE_INT32 tcpping_time;
        //internal use
        ACE_UINT32 tcp_silence_sec;
        ACE_UINT32 udp_silence_sec;
        ACE_Time_Value ping_issue_time;
        bool udp_ping_dirty;
        bool tcp_ping_dirty;
        ClientStats()
        {
            udpbytes_sent = udpbytes_recv = 
                voicebytes_sent = voicebytes_recv = 
                vidcapbytes_sent = vidcapbytes_recv = 
                desktopbytes_sent = desktopbytes_recv = 
                mediafile_audio_bytes_sent = mediafile_audio_bytes_recv = 
                mediafile_video_bytes_sent = mediafile_video_bytes_recv = 0;
            udpping_time = tcpping_time = -1;
            tcp_silence_sec = udp_silence_sec = 0;
            tcp_ping_dirty = udp_ping_dirty = true;
        }
    };

    struct ClientKeepAlive
    {
        // no reply for three minutes, consider server dead
        ACE_Time_Value connection_lost = ACE_Time_Value(180, 0);
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

    struct SoundProperties
    {
        int inputdeviceid;
        int outputdeviceid;
        //sound group for current instance
        int soundgroupid;
        //AGC and denoise settings
        SpeexDSP speexdsp;
        //gain input audio
        int gainlevel;
        //dereverb
        bool dereverb;
        //count transmitted samples
        ACE_UINT32 samples_transmitted;
        //total samples recorded
        ACE_UINT32 samples_recorded;

        SoundProperties()
        {
            inputdeviceid = outputdeviceid = SOUNDDEVICE_IGNORE_ID;
            soundgroupid = 0;
            gainlevel = GAIN_NORMAL;
            dereverb = true;
            samples_transmitted = 0;
            samples_recorded = 0;
        }
    };

    class EventSuspender
    {
    public:
        virtual void SuspendEventHandling() = 0;
        virtual void ResumeEventHandling() = 0;
    };

    //forward decl.
    class ClientListener;
    typedef std::shared_ptr< class FileNode > filenode_t;

    class ClientNode
        : public ACE_Task<ACE_MT_SYNCH>
        , public PacketListener
        , public StreamListener<DefaultStreamHandler::StreamHandler_t>
#if defined(ENABLE_ENCRYPTION)
        , public StreamListener<CryptStreamHandler::StreamHandler_t>
#endif
        , public TimerListener
        , public soundsystem::StreamCapture
        , public soundsystem::StreamDuplex
        , public FileTransferListener
        , public EventSuspender
    {
    public:
        ClientNode(const ACE_TString& version, ClientListener* listener);
        virtual ~ClientNode();

        int svc(void);

        void SuspendEventHandling();
        void ResumeEventHandling();

        ACE_Lock& reactor_lock();
#if defined(_DEBUG)
        ACE_thread_t m_reactor_thr_id;
        ACE_UINT32 m_active_timerid;
        ACE_Semaphore m_reactor_wait;
#endif
        ACE_Recursive_Thread_Mutex& lock_sndprop() { return m_sndgrp_lock; }
        ACE_Recursive_Thread_Mutex& lock_timers() { return m_timers_lock; }
        VoiceLogger& voicelogger();
        AudioContainer& audiocontainer();

        //server properties
        bool GetServerInfo(ServerInfo& info);
        bool GetClientStatistics(ClientStats& stats);

        ClientFlags GetFlags() const { return m_flags; }
        int GetUserID() const { return m_myuserid; }
        const UserAccount& GetMyUserAccount() const { return m_myuseraccount; }

        clientuser_t GetUser(int userid, bool include_local = false);
        clientuser_t GetUserByUsername(const ACE_TString& username);
        void GetUsers(std::set<int>& userids);
        clientchannel_t GetRootChannel();
        clientchannel_t GetMyChannel();
        int GetChannelID();
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
        const SoundProperties& GetSoundProperties() const { return m_soundprop; }

        bool SetSoundOutputVolume(int volume);
        int GetSoundOutputVolume();

        bool EnableVoiceTransmission(bool enable);
        int GetCurrentVoiceLevel();
        void SetVoiceActivationLevel(int voicelevel);
        int GetVoiceActivationLevel();
        bool EnableVoiceActivation(bool enable);
        void SetVoiceActivationStoppedDelay(int msec);
        int GetVoiceActivationStoppedDelay();

        bool EnableAutoPositioning(bool enable);
        bool AutoPositionUsers();    //position users in 3D

        bool EnableAudioBlockCallback(int userid, StreamType stream_type,
                                      bool enable);
        // user provided audio stream that replaces voice input stream
        bool QueueAudioInput(const media::AudioFrame& frm, int streamid);
        
        bool MuteAll(bool muteall);

        void SetVoiceGainLevel(int gainlevel);
        int GetVoiceGainLevel();

        bool SetSoundPreprocess(const SpeexDSP& speexdsp);
        void SetSoundInputTone(StreamTypes streams, int frequency);

        bool StartRecordingMuxedAudioFile(const AudioCodec& codec, 
                                          const ACE_TString& filename,
                                          AudioFileFormat aff);
        bool StartRecordingMuxedAudioFile(int channelid,
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
        void StopStreamingMediaFile();

        // playback local media file
        int InitMediaPlayback(const ACE_TString& filename, uint32_t offset,
                              bool paused, const AudioPreprocessor& preprocessor);
        bool UpdateMediaPlayback(int id, uint32_t offset, bool paused, 
                                 const AudioPreprocessor& preprocessor, bool initial = false);
        bool StopMediaPlayback(int id);

        void MediaPlaybackStatus(int id, const MediaFileProp& mfp, MediaStreamStatus status);

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
        
        //StreamListener
#if defined(ENABLE_ENCRYPTION)
        void OnOpened(CryptStreamHandler::StreamHandler_t& handler);
        void OnClosed(CryptStreamHandler::StreamHandler_t& handler);
        bool OnReceive(CryptStreamHandler::StreamHandler_t& handler, const char* buff, int len);
        bool OnSend(CryptStreamHandler::StreamHandler_t& handler);
#endif
        void OnOpened(DefaultStreamHandler::StreamHandler_t& handler);
        void OnClosed(DefaultStreamHandler::StreamHandler_t& handler);
        bool OnReceive(DefaultStreamHandler::StreamHandler_t& handler, const char* buff, int len);
        bool OnSend(DefaultStreamHandler::StreamHandler_t& handler);

        //set keep alive timer intervals
        void UpdateKeepAlive(const ClientKeepAlive& keepalive);
        ClientKeepAlive GetKeepAlive();

        //Start timer which is handled and terminated outside ClientNode
        long StartUserTimer(uint16_t timer_id, uint16_t userid, 
                            long userdata, const ACE_Time_Value& delay, 
                            const ACE_Time_Value& interval = ACE_Time_Value::zero);
        bool StopUserTimer(uint16_t timer_id, uint16_t userid);
        bool TimerExists(ACE_UINT32 timer_id);
        bool TimerExists(ACE_UINT32 timer_id, int userid);
        //TimerListener - reactor thread
        int TimerEvent(ACE_UINT32 timer_event_id, long userdata);

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
                             const short* buffer, int n_samples);
        void StreamDuplexEchoCb(const soundsystem::DuplexStreamer& streamer,
                                const short* input_buffer, 
                                const short* prev_output_buffer, 
                                int n_samples);

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
        void AudioMuxCallback(const media::AudioFrame& audio_frame);
        // AudioPlayer listener - separate thread
        void AudioUserCallback(int userid, StreamType st,
                               const media::AudioFrame& audio_frame);

        // FileNode listener - reactor thread
        void OnFileTransferStatus(const teamtalk::FileTransfer& transfer);

        bool GetTransferInfo(int transferid, FileTransfer& transfer);
        bool CancelFileTransfer(int transferid);

        //PacketListener - reactor thread
        void ReceivedPacket(PacketHandler* ph,
                            const char* packet_data, int packet_size, 
                            const ACE_INET_Addr& addr);
        void SendPackets(); //send packets - reactor thread

        bool QueuePacket(FieldPacket* packet);

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

        //Start/stop timers handled by ClientNode
        long StartTimer(ACE_UINT32 timer_id, long userdata, 
                        const ACE_Time_Value& delay, 
                        const ACE_Time_Value& interval = ACE_Time_Value::zero);
        bool StopTimer(ACE_UINT32 timer_id);
        //remove timer from timer set (without stopping it)
        void ClearTimer(ACE_UINT32 timer_id);

        int Timer_OneSecond();
        int Timer_UdpKeepAlive();
        int Timer_BuildDesktopPackets();
        int Timer_DesktopPacketRTX();
        int Timer_DesktopNAKPacket();
        int Timer_QueryMTU(int mtu_index);

        //audio start/stop/update
        void OpenAudioCapture(const AudioCodec& codec);
        void CloseAudioCapture();
        void QueueVoiceFrame(media::AudioFrame& audframe);

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
        //the reactor associated with this client instance
        ACE_Reactor m_reactor;
        std::atomic<ClientFlags> m_flags; //Mask of ClientFlag-enum
        //set of timers currently in use. Protected by lock_timers().
        timer_handlers_t m_timers;
        ACE_Recursive_Thread_Mutex m_timers_lock; //mutexes must be the last to be destroyed
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
        DefaultStreamHandler::StreamHandler_t* m_def_stream;
#if defined(ENABLE_ENCRYPTION)
        crypt_connector_t m_crypt_connector;
        CryptStreamHandler::StreamHandler_t* m_crypt_stream;
#endif
        //TCP send/receive buffer for StreamHandler
        ACE_CString m_recvbuffer, m_sendbuffer;

        //The voice packet receiver
        PacketHandler m_packethandler;

        ServerInfo m_serverinfo;
        ClientStats m_clientstats;
        ClientKeepAlive m_keepalive;

        //channels and users
        typedef std::map<int, clientuser_t> musers_t;
        musers_t m_users;

        clientchannel_t m_rootchannel;
        clientchannel_t m_mychannel;
        int m_myuserid;
        UserAccount m_myuseraccount;
        clientuser_t m_local_voicelog;

        //cmdid -> filetransfer
        typedef std::map<int, FileTransfer> filetransfer_q_t;
        //file transfer queue
        filetransfer_q_t m_waitingTransfers;
        //active file transfers
        typedef std::map<int, filenode_t> filenodes_t;
        filenodes_t m_filetransfers;

        //audio resampler for capture
        audio_resampler_t m_capture_resampler;
        std::vector<short> m_capture_buffer;
        //audio resampler for playback (in duplex mode)
        audio_resampler_t m_playback_resampler;
        std::vector<short> m_playback_buffer;

        //encode voice from sound input
        AudioThread m_voice_thread;
        uint8_t m_voice_stream_id; //0 means not used
        uint16_t m_voice_pkt_counter;
        std::atomic<bool> m_voice_tx_closed{false}; // CLIENT_TX_VOICE was toggled (transmit next packet)

        //encode video from video capture
        vidcap::videocapture_t m_vidcap;
        VideoThread m_vidcap_thread;
        ACE_Message_Queue<ACE_MT_SYNCH> m_local_vidcapframes; //local RGB32 video frames
        uint8_t m_vidcap_stream_id; //0 means not used

        //media streamer to channels
        mediafile_streamer_t m_mediafile_streamer;
        uint8_t m_mediafile_stream_id; //0 means not used

        //encode audio of media file
        AudioThread m_audiofile_thread;
        uint16_t m_audiofile_pkt_counter;

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
        uint8_t m_desktop_session_id;

        //UDP packets waiting for transmission
        PacketQueue m_tx_queue;

        //unique IDs for Do* commands
        uint16_t m_cmdid_counter; 
        uint16_t m_current_cmdid; 

        //local UDP sockets to use (stored in case UDP socket must be recreated)
        ACE_INET_Addr m_localTcpAddr, m_localUdpAddr;

        //query MTU (timestamp -> MTU packet)
        typedef std::map<uint32_t, ka_mtu_packet_t> mtu_packets_t;
        mtu_packets_t m_mtu_packets;
        uint16_t m_mtu_data_size, m_mtu_max_payload_size;

        //the client's version number
        ACE_TString m_version;

        //The listener of the ClientNode instance
        ClientListener* m_listener;
    };

    class ClientListener 
        : public VoiceLogListener //VoiceLogger
    {
    public:
        virtual ~ClientListener() {}

        virtual void RegisterEventSuspender(EventSuspender* suspender) = 0;

        virtual void OnConnectSuccess() = 0;
        virtual void OnConnectFailed() = 0;
        virtual void OnConnectionLost() = 0;

        virtual void OnAccepted(int myuserid, const teamtalk::UserAccount& account) = 0;
        virtual void OnLoggedOut() = 0;

        virtual void OnUserLoggedIn(const teamtalk::ClientUser& user) = 0;
        virtual void OnUserLoggedOut(const teamtalk::ClientUser& user) = 0;
        virtual void OnUserUpdate(const teamtalk::ClientUser& user) = 0;

        virtual void OnUserJoinChannel(const teamtalk::ClientUser& user,
                                       const teamtalk::ClientChannel& chan) = 0;
        virtual void OnUserLeftChannel(const teamtalk::ClientUser& user,
                                       const teamtalk::ClientChannel& chan) = 0;

        virtual void OnAddChannel(const ClientChannel& chan) = 0;
        virtual void OnUpdateChannel(const teamtalk::ClientChannel& chan) = 0;
        virtual void OnRemoveChannel(const teamtalk::ClientChannel& chan) = 0;

        virtual void OnJoinedChannel(int channelid) = 0;
        virtual void OnLeftChannel(int channelid) = 0;

        virtual void OnAddFile(const teamtalk::ClientChannel& chan,
                               const teamtalk::RemoteFile& file) = 0;
        virtual void OnRemoveFile(const teamtalk::ClientChannel& chan,
                                  const teamtalk::RemoteFile& file) = 0;

        virtual void OnUserAccount(const teamtalk::UserAccount& account) = 0;
        virtual void OnBannedUser(const teamtalk::BannedUser& banuser) = 0;

        virtual void OnTextMessage(const teamtalk::TextMessage& textmsg) = 0;

        virtual void OnKicked(const teamtalk::clientuser_t& user, int channelid) = 0;
        virtual void OnServerUpdate(const ServerInfo& serverinfo) = 0;
        virtual void OnServerStatistics(const ServerStats& serverstats) = 0;

        virtual void OnFileTransferStatus(const teamtalk::FileTransfer& transfer) = 0;

        virtual void OnCommandError(int cmdid, int err_num, const ACE_TString& msg) = 0;
        virtual void OnCommandSuccess(int cmdid) = 0;
        virtual void OnCommandProcessing(int cmdid, bool begin_end) = 0;

        virtual void OnInternalError(int err_num, const ACE_TString& msg) = 0;

        virtual void OnVoiceActivated(bool enabled) = 0;

        virtual void OnUserStateChange(const teamtalk::ClientUser& user) = 0;
        virtual void OnUserVideoCaptureFrame(int userid, int stream_id) = 0;
        virtual void OnUserMediaFileVideoFrame(int userid, int stream_id) = 0;

        virtual void OnDesktopTransferUpdate(int session_id, int remain_bytes) = 0;

        virtual void OnUserDesktopWindow(int userid, int session_id) = 0;
        virtual void OnUserDesktopCursor(int src_userid, const teamtalk::DesktopInput& input) = 0;
        virtual void OnUserDesktopInput(int src_userid, const teamtalk::DesktopInput& input) = 0;

        virtual void OnChannelStreamMediaFile(const MediaFileProp& mfp,
                                              MediaFileStatus status) = 0;

        virtual void OnLocalMediaFilePlayback(int sessionid, const MediaFileProp& mfp,
                                              MediaFileStatus status) = 0;

        virtual void OnAudioInputStatus(int voicestreamid, const AudioInputStatus& progress) = 0;

        virtual void OnUserAudioBlock(int userid, StreamType stream_type) = 0;

        virtual void OnMTUQueryComplete(int payload_size) = 0;
    };
}

#endif
