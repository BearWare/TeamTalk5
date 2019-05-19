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

#if !defined(SERVERUSER_H)
#define SERVERUSER_H

#include "Server.h"

#include <ace/Bound_Ptr.h> 
#include <ace/Null_Mutex.h> 
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/FILE_IO.h>

#include <teamtalk/User.h>
#include <teamtalk/StreamHandler.h>
#include "ServerChannel.h"
#include "DesktopCache.h"

namespace teamtalk { 

    class ServerNode;
    class ServerUser;
    class ServerChannel;
    struct ServerProperties;
    struct ClosedDesktopSession
    {
        uint8_t session_id;
        uint32_t update_id;
    };
        
    typedef ACE_Strong_Bound_Ptr< ServerChannel, ACE_Null_Mutex > serverchannel_t;
    typedef ACE_Strong_Bound_Ptr< ServerUser, ACE_Null_Mutex > serveruser_t;

    class ServerUser : public User
    {

    private:
        // prevent copying
        ServerUser(const ServerUser& user);
        const ServerUser& operator = (const ServerUser& user);

#define FILEBUFFERSIZE 0x10000

#if FILEBUFFERSIZE > MSGBUFFERSIZE
#error "File buffer cannot be bigger than message queue"
#endif

        struct LocalFileTransfer
        {
            bool inbound;
            int transferid;
            ACE_FILE_IO file;
            ACE_OFF_T filesize;
            bool active;
            std::vector<char> readbuffer;
            LocalFileTransfer() : inbound(0), transferid(0), filesize(0), active(false)
            {
                readbuffer.resize(FILEBUFFERSIZE);
            }
        };

    public:
        ServerUser(int userid, ServerNode& servernode, ACE_HANDLE h);
        virtual ~ServerUser();

        ACE_HANDLE GetStreamHandle() const { return m_stream_handle; }
        ACE_HANDLE ResetStreamHandle() { 
            ACE_HANDLE h = m_stream_handle;
            m_stream_handle = ACE_INVALID_HANDLE;
            return h;
        }
        bool ReceiveData(const char* data, int len);
        bool SendData(ACE_Message_Queue_Base& msg_queue);

        ACE_TString GetAccessToken() const { return KeyToHexString(m_accesstoken, sizeof(m_accesstoken)); }
        bool IsAuthorized() const { return m_account.usertype & (USERTYPE_ADMIN | USERTYPE_DEFAULT); }
        void SetUserAccount(const UserAccount& account) { m_account = account; }
        const UserAccount& GetUserAccount() const { return m_account; }
        const ACE_TString& GetUsername() const { return m_account.username; }
        UserRights GetUserRights() const { return m_account.userrights; }
        UserTypes GetUserType() const { return m_account.usertype; }
        int GetUserData() const { return m_account.userdata; }
        const ACE_TString& GetInitialChannel() const { return m_account.init_channel; }

        void SetUdpAddress(const ACE_INET_Addr& remoteaddr, const ACE_INET_Addr& localaddr){ m_udpaddr = remoteaddr; m_localudpaddr = localaddr; }
        const ACE_INET_Addr& GetUdpAddress() const { return m_udpaddr; }
        const ACE_INET_Addr& GetLocalUdpAddress() const { return m_localudpaddr; }

        void SetStreamProtocol(const ACE_TString& protocol){m_stream_protocol=protocol;}
        const ACE_TString& GetStreamProtocol() const { return m_stream_protocol; }

        int GetLastKeepAlive() const { return m_nLastKeepAlive; }
        void SetLastKeepAlive(int lasttime){ m_nLastKeepAlive = lasttime; }

        void SetChannel(serverchannel_t& channel){ m_channel = channel; }
        serverchannel_t GetChannel() const { return m_channel.strong(); }
        
        BannedUser GetBan(BanTypes bantype = BANTYPE_NONE, const ACE_TString& chanpath = ACE_TEXT("")) const;

        //user specific subscriptions
        void AddSubscriptions(const ServerUser& user, Subscriptions subscribe);
        void ClearSubscriptions(const ServerUser& user, Subscriptions subscribe);
        Subscriptions GetSubscriptions(const ServerUser& user) const;
        void ClearUserSubscription(const ServerUser& user);

        int GetFileTransferID() const { return m_filetransfer.get() ? m_filetransfer->transferid : 0; }

        ACE_Time_Value GetDuration() const;

        void ForwardChannels(const serverchannel_t& root, bool encrypted);
        void ForwardUsers(const serverchannel_t& channel, bool recursive);
        void ForwardFiles(const serverchannel_t& root, bool recursive);

        //server --> client commands
        void DoWelcome(const ServerSettings& properties);
        void DoServerUpdate(const ServerSettings& properties);
        //also stores user account
        void DoAccepted(const UserAccount& useraccount);
        void DoLoggedOut();

        void DoLoggedIn(const ServerUser& user);
        void DoLoggedOut(const ServerUser& user);

        void DoAddUser(const ServerUser& user, const ServerChannel& channel);
        void DoUpdateUser(const ServerUser& user);
        void DoRemoveUser(const ServerUser& user, const ServerChannel& channel);

        void DoAddChannel(const ServerChannel& channel, bool encrypted);
        void DoUpdateChannel(const ServerChannel& channel, bool encrypted);
        void DoRemoveChannel(const ServerChannel& channel);
        void DoJoinedChannel(const ServerChannel& channel, bool encrypted);
        void DoLeftChannel(const ServerChannel& channel);

        void DoTextMessage(const ServerUser& fromuser, const TextMessage& msg);
        void DoTextMessage(const TextMessage& msg);
        void DoKicked(int kicker_userid, bool channel_kick);
        void DoError(ErrorMsg cmderr);
        void DoPingReply();
        void DoShowBan(const BannedUser& ban);
        void DoShowUserAccount(const UserAccount& user);

        void DoFileDeliver(const FileTransfer& transfer);
        void DoFileAccepted(const FileTransfer& transfer);
        void DoFileCompleted();
        void DoFileReady();
        void DoAddFile(const RemoteFile& file);
        void DoRemoveFile(const ACE_TString& filename, const ServerChannel& channel);
        void DoServerStats();

        void DoOk();
        void DoQuit();

        //desktop packets processing from this user
        bool AddDesktopPacket(const DesktopPacket& packet);
        const desktoppackets_t& GetDesktopSessionQueue() const { return m_desktop_queue; }
        const desktop_cache_t& GetDesktopSession() const { return m_desktop_cache; }
        void CloseDesktopSession();

        //desktop packets being transmitted to this user
        desktop_transmitter_t StartDesktopTransmitter(const ServerUser& src_user,
                                                      const ServerChannel& channel,
                                                      const DesktopCache& desktop);
        desktop_transmitter_t ResumeDesktopTransmitter(const ServerUser& src_user,
                                                       const ServerChannel& channel,
                                                       const DesktopCache& desktop);
        desktop_transmitter_t GetDesktopTransmitter(int src_userid) const;
        void CloseDesktopTransmitter(int src_userid, bool store_closed_session);
        bool GetClosedDesktopSession(int src_userid, ClosedDesktopSession& session) const;
        bool ClosePendingDesktopTerminate(int src_userid);

        bool ProcessCommandQueue(bool clearsuspended);
    private:
        enum CmdProcessing
        {
            CMD_ABORT, CMD_SUSPENDED, CMD_DONE
        };
        CmdProcessing ProcessCommand(const ACE_CString& cmdline, bool was_suspended);
        ErrorMsg HandleCommand(const ACE_TString& cmd, const mstrings_t& properties);
        //handling of client --> server commands
        ErrorMsg HandleLogin(const mstrings_t& properties);
        ErrorMsg HandleLogout(const mstrings_t& properties);

        ErrorMsg HandleChangeNick(const mstrings_t& properties);
        ErrorMsg HandleChangeStatus(const mstrings_t& properties);
        ErrorMsg HandleMessage(const mstrings_t& properties);
        ErrorMsg HandleKeepAlive(const mstrings_t& properties);
        ErrorMsg HandleKick(const mstrings_t& properties);
        ErrorMsg HandleMoveUser(const mstrings_t& properties); 
        ErrorMsg HandleChannelOp(const mstrings_t& properties);

        ErrorMsg HandleJoinChannel(const mstrings_t& properties);
        ErrorMsg HandleLeaveChannel(const mstrings_t& properties);
        ErrorMsg HandleMakeChannel(const mstrings_t& properties); 
        ErrorMsg HandleUpdateChannel(const mstrings_t& properties);
        ErrorMsg HandleRemoveChannel(const mstrings_t& properties);

        ErrorMsg HandleUpdateServer(const mstrings_t& properties);
        ErrorMsg HandleSaveConfig(const mstrings_t& properties);
        ErrorMsg HandleListUserAccounts(const mstrings_t& properties);
        ErrorMsg HandleNewUserAccount(const mstrings_t& properties);
        ErrorMsg HandleDeleteUserAccount(const mstrings_t& properties);
        ErrorMsg HandleUserBan(const mstrings_t& properties);
        ErrorMsg HandleUserUnban(const mstrings_t& properties);
        ErrorMsg HandleListServerBans(const mstrings_t& properties);

        ErrorMsg HandleRegSendFile(const mstrings_t& properties);
        ErrorMsg HandleRegRecvFile(const mstrings_t& properties);
        ErrorMsg HandleSendFile(const mstrings_t& properties);    //client to server file, switches to binary mode
        ErrorMsg HandleRecvFile(const mstrings_t& properties);    //server to client file
        ErrorMsg HandleFileDeliver(const mstrings_t& properties);    //initiates send to client
        ErrorMsg HandleDeleteFile(const mstrings_t& properties);

        ErrorMsg HandleSubscribe(const mstrings_t& properties);
        ErrorMsg HandleUnsubscribe(const mstrings_t& properties);

        ErrorMsg HandleQueryStats(const mstrings_t& properties);
        //binary mode handler for file transfers
        void HandleBinaryFileWrite(const char* buff, int len, bool& bContinue);

        void DoBeginCmd(int cmdID);
        void DoEndCmd(int cmdID);

        void TransmitCommand(const ACE_TString& cmd);
        void SendFile(ACE_Message_Queue_Base& msg_queue);
        void CloseTransfer();

        //success call to HandleLogin
        UserAccount m_account;

        ACE_INET_Addr m_udpaddr, m_localudpaddr;

        ACE_TString m_stream_protocol;
        ServerNode& m_servernode;
        ACE_HANDLE m_stream_handle;
        uint8_t m_accesstoken[CRYPTKEY_SIZE];
            
        int m_nLastKeepAlive;
        ACE_Weak_Bound_Ptr< ServerChannel, ACE_Null_Mutex > m_channel;
        ACE_Time_Value m_LogonTime;

        //commands received so far
        ACE_CString m_recvbuf, m_sendbuf;
        bool m_cmdsuspended;

        //file transfer variables
        std::unique_ptr<LocalFileTransfer> m_filetransfer;

        std::set<ACE_UINT32, w32_less_comp> m_floodcmd;

        //desktop session, from this user
        desktop_cache_t m_desktop_cache;
        desktoppackets_t m_desktop_queue;
        //desktop session, to this user, src-user -> desktop transmitter
        typedef std::map<int, desktop_transmitter_t> user_desktoptx_t;
        user_desktoptx_t m_user_desktop_tx;
        //closed desktop session which are being NAK'ed as ended sessions
        typedef std::map<int, ClosedDesktopSession> closed_desktops_t;
        closed_desktops_t m_closed_desktops;

        //userid -> subscription.
        typedef std::map<int, Subscriptions> usersubscriptions_t;
        usersubscriptions_t m_usersubscriptions;
    };
}
#endif
