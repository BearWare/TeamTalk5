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

#if !defined(SERVERNODE_H)
#define SERVERNODE_H

#include "Server.h"

// ACE
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Guard_T.h>
#include <ace/Acceptor.h>

#include <teamtalk/Commands.h>
#include <teamtalk/PacketHandler.h>

#include <myace/TimerHandler.h>
#include <myace/MyACE.h>

#include "AcceptHandler.h"
#include "ServerChannel.h"

// STL
#include <map>
#include <string>
#include <vector>

#if defined(ENABLE_TEAMTALKPRO)
#define DEFAULT_TCPPORT 10443
#define DEFAULT_UDPPORT 10443
#else
#define DEFAULT_TCPPORT 10333
#define DEFAULT_UDPPORT 10333
#endif

#define SERVER_USERID 0

#define SERVER_KEEPALIVE_DELAY 1  //keep alive delay (secs). Checks
                                  //whether some users are dead

#define GUARD_OBJ_NAME(name, this_obj, lock)            \
    guard_t name(lock);                                 \
    (this_obj)->m_reactor_thr_id = ACE_Thread::self()

#define GUARD_OBJ_REACQUIRE(name, this_obj)             \
    (name).acquire();                                   \
    (this_obj)->m_reactor_thr_id = ACE_Thread::self()

#define GUARD_OBJ_RELEASE(name, this_obj)               \
    (this_obj)->m_reactor_thr_id = ACE_thread_t();      \
    (name).release()

#define GUARD_OBJ(this_obj, lock)    GUARD_OBJ_NAME(g, this_obj, lock)

#ifdef _DEBUG
#define ASSERT_REACTOR_LOCKED(this_obj)                         \
    TTASSERT(this_obj->m_reactor_thr_id == ACE_Thread::self())
#else
#define ASSERT_REACTOR_LOCKED(...)     (void)0
#endif

typedef std::map<ACE_TString, std::vector<ACE_TString> > mapipchan_t; //ip-address -> channelpaths
typedef std::map<ACE_TString, std::vector<ACE_Time_Value> > mapiptime_t; //map ip to login time (used for checking login attempts)

namespace teamtalk {

    enum ServerTimer
    {
        TIMER_ONE_SECOND_ID                     = 1,
        TIMER_DESKTOPACKPACKET_ID               = 2,
        TIMER_DESKTOPPACKET_RTX_TIMEOUT_ID      = 3,
        TIMER_START_DESKTOPTX_ID                = 4,
        TIMER_CLOSE_DESKTOPSESSION_ID           = 5,
        TIMER_COMMAND_RESUME                    = 6
    };

    class ServerNodeListener;
    class ServerUser;
    class ServerChannel;

    union timer_userdata
    {
        struct
        {
            ACE_UINT16 src_userid;
            ACE_UINT16 dest_userid;
        };
        long userdata;
        operator long() const  { return userdata; }
    };

    class ServerNode 
        : public TimerListener
        , public PacketListener
#if defined(ENABLE_ENCRYPTION)
        , public CryptStreamHandler::StreamListener_t
#endif
        , public DefaultStreamHandler::StreamListener_t
    {
        // prevent copying
        ServerNode(const ServerNode& sn);
        const ServerNode& operator = (const ServerNode& sn);

    public:
        ServerNode(const ACE_TString& version,
                   ACE_Reactor* timerReactor,
                   ACE_Reactor* tcpReactor, 
                   ACE_Reactor* udpReactor, 
                   ServerNodeListener* listener = NULL);

        virtual ~ServerNode();

        ACE_Lock& lock();
        ACE_thread_t m_reactor_thr_id;

        int GetNewUserID();
        serveruser_t GetUser(int userid, const ServerUser* caller);

        ACE_Time_Value GetUptime() const;
        serverchannel_t& GetRootChannel();
        void CheckKeepAlive();
        int GetAuthUserCount();
        int GetActiveFileTransfers(int& uploads, int& downloads);
        bool IsEncrypted() const;
        bool LoginsExceeded(const ServerUser& user);

        //send udp packet
        int SendPacket(const FieldPacket& packet, const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
        int SendPacket(const FieldPacket& packet, const ServerUser& user);
        int SendPackets(const FieldPacket& packet, const ServerChannel::users_t& users);

        //UDP packet handling functions
        void ReceivedPacket(PacketHandler* ph,
                            const char* packet_data, int packet_size, 
                            const ACE_INET_Addr& remoteaddr);
        void ReceivedHelloPacket(ServerUser& user, const HelloPacket& packet, 
                                 const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
        void ReceivedKeepAlivePacket(ServerUser& user, const KeepAlivePacket& packet, 
                                     const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);

        //broadcast either an audio packet or a crypt audio packet
        void ReceivedVoicePacket(ServerUser& user, 
                                 const FieldPacket& packet, 
                                 const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
        void ReceivedAudioFilePacket(ServerUser& user, 
                                     const FieldPacket& packet, 
                                     const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
        void ReceivedVideoCapturePacket(ServerUser& user, 
                                 const FieldPacket& packet, 
                                 const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
        void ReceivedVideoFilePacket(ServerUser& user, 
                                     const FieldPacket& packet, 
                                     const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);

#ifdef ENABLE_ENCRYPTION
        void ReceivedDesktopPacket(ServerUser& user, 
                                   const CryptDesktopPacket& crypt_pkt, 
                                   const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
#endif
        void ReceivedDesktopPacket(ServerUser& user, 
                                   const DesktopPacket& packet, 
                                   const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
#ifdef ENABLE_ENCRYPTION
        void ReceivedDesktopAckPacket(ServerUser& user, 
                                      const CryptDesktopAckPacket& crypt_pkt, 
                                      const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
#endif
        void ReceivedDesktopAckPacket(ServerUser& user, 
                                      const DesktopAckPacket& packet, 
                                      const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
#ifdef ENABLE_ENCRYPTION
        void ReceivedDesktopNakPacket(ServerUser& user, 
                                      const CryptDesktopNakPacket& crypt_pkt, 
                                      const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
#endif
        void ReceivedDesktopNakPacket(ServerUser& user, 
                                      const DesktopNakPacket& packet, 
                                      const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
#ifdef ENABLE_ENCRYPTION
        void ReceivedDesktopCursorPacket(ServerUser& user, 
                                         const CryptDesktopCursorPacket& crypt_pkt, 
                                         const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
#endif
        void ReceivedDesktopCursorPacket(ServerUser& user, 
                                         const DesktopCursorPacket& packet, 
                                         const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
#ifdef ENABLE_ENCRYPTION
        void ReceivedDesktopInputPacket(ServerUser& user, 
                                        const CryptDesktopInputPacket& crypt_pkt, 
                                        const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
#endif
        void ReceivedDesktopInputPacket(ServerUser& user, 
                                        const DesktopInputPacket& packet, 
                                        const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
#ifdef ENABLE_ENCRYPTION
        void ReceivedDesktopInputAckPacket(ServerUser& user, 
                                           const CryptDesktopInputAckPacket& crypt_pkt, 
                                           const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);
#endif
        void ReceivedDesktopInputAckPacket(ServerUser& user, 
                                           const DesktopInputAckPacket& packet, 
                                           const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr);

        //server properties
        void SetServerProperties(const ServerSettings& srvprop);
        const ServerSettings& GetServerProperties() const;
        const ServerStats& GetServerStats() const;
        ACE_TString GetMessageOfTheDay(int ignore_userid = 0);
        bool SetFileSharing(const ACE_TString& rootdir);
        ACE_INT64 GetDiskUsage();
        ACE_TString GetFilesRoot() const { return m_properties.filesroot; }
        bool IsAutoSaving();
        void SetAutoSaving(bool autosave);
        int GetChannelID(const ACE_TString& chanpath);
        bool GetChannelProp(int channelid, ChannelProp& prop);
        serverchannel_t GetChannel(int channelid) const;

        //register callback from TCP reactor
        ACE_Event_Handler* RegisterStreamCallback(ACE_HANDLE h);

#if defined(ENABLE_ENCRYPTION)
        void OnOpened(CryptStreamHandler::StreamHandler_t& streamer);
        void OnClosed(CryptStreamHandler::StreamHandler_t& streamer);
        bool OnReceive(CryptStreamHandler::StreamHandler_t& streamer, const char* buff, int len);
        bool OnSend(CryptStreamHandler::StreamHandler_t& streamer);
#endif
        void OnOpened(DefaultStreamHandler::StreamHandler_t& streamer);
        void OnClosed(DefaultStreamHandler::StreamHandler_t& streamer);
        bool OnReceive(DefaultStreamHandler::StreamHandler_t& streamer, const char* buff, int len);
        bool OnSend(DefaultStreamHandler::StreamHandler_t& streamer);

        //launch server
        bool StartServer(bool encrypted, const ACE_TString& sysid);
        void StopServer(bool docallback = true);

        int StartTimer(ServerTimer timer, timer_userdata userdata, 
                       const ACE_Time_Value& delay, 
                       const ACE_Time_Value& interval = ACE_Time_Value::zero);

        //timer callback
        int TimerEvent(ACE_UINT32 timer_event_id, long userdata);

        //get currently connected admins
        const ServerChannel::users_t& GetAdministrators();
        //get all admins except those in excludeChannel
        ServerChannel::users_t GetAdministrators(const ServerChannel& excludeChannel);
        //get all authorized users
        ServerChannel::users_t GetAuthorizedUsers(bool excludeAdmins = false);
        //get users who should be notified of users changing channels
        ServerChannel::users_t GetNotificationUsers(const ServerChannel& excludeChannel);
        //get users who should be notified of user changes
        ServerChannel::users_t GetNotificationUsers();

        //////////////////////////////////////////
        /// User performing action
        //////////////////////////////////////////
        ErrorMsg UserLogin(int userid, const ACE_TString& username, 
                           const ACE_TString& passwd);
        ErrorMsg UserLogout(int userid); //both admins and users go through here.
        ErrorMsg UserJoinChannel(int userid, const ChannelProp& chanprop);
        //User is exiting channel. Check whether we should clean up
        ErrorMsg UserLeaveChannel(int userid, int channelid = 0);
        ErrorMsg UserChangeNickname(int userid, const ACE_TString& newnick);
        ErrorMsg UserChangeStatus(int userid, int mode, const ACE_TString& status);
        ErrorMsg UserUpdate(int userid);
        ErrorMsg UserMove(int userid, int moveuserid, int channelid);
        void UserDisconnected(int userid);
        ErrorMsg UserOpDeOp(int userid, int channelid, //op user in a channel
                            const ACE_TString& oppasswd, int op_userid, bool op); 
        ErrorMsg UserKick(int userid, int kick_userid, int chanid, bool force_kick);
        ErrorMsg UserBan(int userid, int ban_userid, BannedUser ban);
        ErrorMsg UserUnBan(int userid, const BannedUser& ban);
        ErrorMsg UserListServerBans(int userid, int chanid, int index, int count);
        ErrorMsg UserListUserAccounts(int userid, int index, int count);
        ErrorMsg UserNewUserAccount(int userid, const UserAccount& regusr);
        ErrorMsg UserDeleteUserAccount(int userid, const ACE_TString& username);
        ErrorMsg UserTextMessage(const TextMessage& msg);

        //transfer id will be set if successful
        ErrorMsg UserRegFileTransfer(FileTransfer& transfer);
        ErrorMsg UserBeginFileTransfer(int transferid, FileTransfer& transfer, 
                                       ACE_FILE_IO& file);
        ErrorMsg UserEndFileTransfer(int transferid);
        ErrorMsg UserDeleteFile(int userid, int channelid, const ACE_TString& filename);

        //user subscriptions to audio and channel/user text messages
        ErrorMsg UserSubscribe(int userid, int subuserid, Subscriptions subscrip);
        ErrorMsg UserUnsubscribe(int userid, int subuserid, Subscriptions subscrip);

        ErrorMsg UserUpdateChannel(int userid, const ChannelProp& chanprop);
        ErrorMsg UserUpdateServer(int userid, const ServerSettings& properties);
        ErrorMsg UserSaveServerConfig(int userid);

        ErrorMsg UpdateServer(const ServerSettings& properties);

        ErrorMsg MakeChannel(const ChannelProp& chanprop, const ServerUser* user = NULL);
        ErrorMsg UpdateChannel(const ChannelProp& chanprop, const ServerUser* user = NULL);
        ErrorMsg RemoveChannel(int channelid, const ServerUser* user = NULL);

        //add a file to a channel's folder
        ErrorMsg AddFileToChannel(const RemoteFile& remotefile);
        //remove a file from a channel's folder
        ErrorMsg RemoveFileFromChannel(const ACE_TString& filename, int channelid);

        ErrorMsg AddBannedUserToChannel(const BannedUser& ban);

        ErrorMsg SendTextMessage(const TextMessage& msg);

    private:
        void OnOpened(ACE_HANDLE h, serveruser_t& user);
        void OnClosed(ACE_HANDLE h);
        bool OnReceive(ACE_HANDLE h, const char* buff, int len);
        //notify users of channel update
        void UpdateChannel(const ServerChannel& chan);
        void UpdateChannel(const ServerChannel& chan, const ServerChannel::users_t& users);
        void CleanChannels(serverchannel_t& channel);
        void UpdateSoloTransmitChannels();
        //update the number of times a user has tried to login unsuccessfully
        void IncLoginAttempt(const ServerUser& user);
        //get the destination channel of a packet
        serverchannel_t GetPacketChannel(ServerUser& user,
                                         const FieldPacket& packet,
                                         const ACE_INET_Addr& remoteaddr,
                                         const ACE_INET_Addr& localaddr);
        //get destination IP-addresses and users of packet
        ServerChannel::users_t GetPacketDestinations(const ServerUser& user,
                                                     const ServerChannel& channel,
                                                     const FieldPacket& packet,
                                                     Subscriptions subscrip_check,
                                                     Subscriptions intercept_check);
        //send desktop ack packet (client desktop -> server)
        bool SendDesktopAckPacket(int userid);
        //process desktop transmitter (server -> client)
        bool RetransmitDesktopPackets(int src_userid, int dest_userid);
        //start or resume desktop transmitter (server -> client)
        bool StartDesktopTransmitter(const ServerUser& src_user,
                                     ServerUser& dest_user,
                                     const ServerChannel& chan);
        //stop desktop transmitter  (server -> client)
        void StopDesktopTransmitter(const ServerUser& src_user,
                                    ServerUser& dest_user,
                                    bool start_nak_timer);

        //all connected users
        typedef std::map<int, serveruser_t> mapusers_t;
        mapusers_t m_mUsers; //all users
        ServerChannel::users_t m_admins; //only admins (admin cache for speed up)

        //failed login attempts
        mapiptime_t m_failedlogins;
        // last login (ip->time)
        std::map<ACE_TString, ACE_Time_Value> m_logindelay;
        
        //user id incrementer
        int m_userid_counter;
        //acceptor for listening for clients
#if defined(ENABLE_ENCRYPTION)
        typedef std::shared_ptr<CryptAcceptor> cryptacceptor_t;
        std::vector<cryptacceptor_t> m_crypt_acceptors;
#endif
        typedef std::shared_ptr<DefaultAcceptor> defaultacceptor_t;
        std::vector<defaultacceptor_t> m_def_acceptors;

        std::map<ACE_HANDLE, serveruser_t> m_streamhandles;

        //socket for udp traffic
        typedef std::shared_ptr<PacketHandler> packethandler_t;
        std::vector<packethandler_t> m_packethandlers;
        
        //mutex for clients
        ACE_Recursive_Thread_Mutex m_sendmutex;
        //the channels
        serverchannel_t m_rootchannel;

        //registered file transfers
        typedef std::map<int, FileTransfer> filetransfers_t;
        filetransfers_t m_filetransfers;

        //one second timer id
        long m_onesec_timerid;
        //ack desktop data received (user id -> timer id)
        typedef std::map<int, long> user_desktopack_timers_t;
        user_desktopack_timers_t m_desktop_ack_timers;
        //RTX timer for desktop transmitters (dest/src user id -> timer id)
        typedef std::map<long, long> user_desktoppacket_rtx_t;
        user_desktoppacket_rtx_t m_desktop_rtx_timers;

        //set of user's who must be updated
        std::set<int> m_updUserIPs;

        //file transfer id and file id counters
        int m_filetx_id_counter, m_file_id_counter;

        std::map<ACE_thread_t, ACE_Reactor*> m_reactors;
        ACE_Reactor* m_timer_reactor, *m_tcp_reactor, *m_udp_reactor;

        //server stats
        ServerStats m_stats;
        //listener for changes
        ServerNodeListener* m_srvguard;
        //server's properties
        ServerSettings m_properties;
    };

    class ServerNodeListener
    {
    public:
        virtual ~ServerNodeListener() {}

        /* begin logging functions */
        virtual void OnUserConnected(const ServerUser& user) = 0;
        virtual void OnUserLogin(const ServerUser& user) = 0;
        virtual void OnUserAuthFailed(const ServerUser& user, const ACE_TString& username) = 0;
        virtual void OnUserLoginBanned(const ServerUser& user) = 0;
        virtual void OnUserLoggedOut(const ServerUser& user) = 0;
        virtual void OnUserDisconnected(const ServerUser& user) = 0;
        virtual void OnUserDropped(const ServerUser& user) = 0;
        virtual void OnUserKicked(const ServerUser& kickee, const ServerUser* kicker, const ServerChannel* channel) = 0;
        virtual void OnUserBanned(const ServerUser& banee, const ServerUser& banner) = 0;
        virtual void OnUserBanned(const ACE_TString& ipaddr, const ServerUser& banner) = 0;
        virtual void OnUserBanned(const ServerUser& banner, const BannedUser& ban) = 0;
        virtual void OnUserUnbanned(const ServerUser& user, const BannedUser& ban) = 0;
        virtual void OnUserUpdated(const ServerUser& user) = 0;
        virtual void OnUserJoinChannel(const ServerUser& user, const ServerChannel& channel) = 0;
        virtual void OnUserLeaveChannel(const ServerUser& user, const ServerChannel& channel) = 0;
        virtual void OnUserMoved(const ServerUser& mover, const ServerUser& movee) = 0;
        virtual void OnUserMessage(const ServerUser& from, const ServerUser& to, const teamtalk::TextMessage& msg) = 0;
        virtual void OnChannelMessage(const ServerUser& from, const ServerChannel& channel, const teamtalk::TextMessage& msg) = 0;
        virtual void OnBroadcastMessage(const ServerUser& from, const teamtalk::TextMessage& msg) = 0;
        virtual void OnCustomMessage(const ServerUser& from, const ServerUser& to, const teamtalk::TextMessage& msg) = 0;

        virtual void OnChannelCreated(const ServerChannel& channel, const ServerUser* user = NULL) = 0;
        virtual void OnChannelUpdated(const ServerChannel& channel, const ServerUser* user = NULL) = 0;
        virtual void OnChannelRemoved(const ServerChannel& channel, const ServerUser* user = NULL) = 0;

        virtual void OnFileUploaded(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file) = 0;
        virtual void OnFileDownloaded(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file) = 0;
        virtual void OnFileDeleted(const ServerUser& user, const ServerChannel& chan, const RemoteFile& file) = 0;

        virtual void OnServerUpdated(const ServerUser& user, const ServerSettings& srvprop) = 0;
        virtual void OnSaveConfiguration(ServerNode& servernode, const ServerUser* user = NULL) = 0;

        virtual void OnShutdown(const ServerStats& stats) = 0;
        /* end logging functions */

        virtual ErrorMsg AuthenticateUser(ServerNode* servernode, ServerUser& user, 
                                          UserAccount& useraccount) = 0; //user-type is set, therefore not const
        virtual ErrorMsg JoinChannel(const ServerUser& user, const ServerChannel& chan) = 0;
        virtual ErrorMsg GetUserAccount(const ServerUser& user,
                                        UserAccount& useraccount) = 0;
        virtual ErrorMsg GetRegUsers(const ServerUser& user, useraccounts_t& users) = 0;
        virtual ErrorMsg AddRegUser(const ServerUser& user, const UserAccount& useraccount) = 0;
        virtual ErrorMsg DeleteRegUser(const ServerUser& user, const ACE_TString& username) = 0;

        virtual ErrorMsg AddUserBan(const ServerUser& banner, const ServerUser& banee, BanTypes bantype) = 0;
        virtual ErrorMsg AddUserBan(const ServerUser& banner, const BannedUser& ban) = 0;
        virtual ErrorMsg RemoveUserBan(const ServerUser& user, const BannedUser& ban) = 0;
        virtual ErrorMsg GetUserBans(const ServerUser& user, std::vector<BannedUser>& bans) = 0;

        virtual ErrorMsg ChangeNickname(const ServerUser& user, const ACE_TString& newnick) = 0;
        virtual ErrorMsg ChangeStatus(const ServerUser& user, int mode, const ACE_TString& status) = 0;
    };
}
#endif
