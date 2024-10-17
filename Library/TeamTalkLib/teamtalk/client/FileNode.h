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

#if !defined(FILENODE_H)
#define FILENODE_H

#include "Client.h"
#include <mystd/MyStd.h>

#include <teamtalk/Commands.h>
#include <myace/TimerHandler.h>

#include <ace/SString.h>

namespace teamtalk {

    class FileTransferListener
    {
    public:
        virtual ~FileTransferListener() {}
        virtual void OnFileTransferStatus(const teamtalk::FileTransfer& transfer) = 0;
    };

    class FileNode 
        : public TimerListener
        , public StreamListener<DefaultStreamHandler::StreamHandler_t>
#if defined(ENABLE_ENCRYPTION)
        , public StreamListener<CryptStreamHandler::StreamHandler_t>
#endif
        , NonCopyable
    {
    public:
        FileNode(ACE_Reactor& reactor, bool encrypted,
                 const ACE_INET_Addr& addr, const ServerProperties& srvprop,
                 const teamtalk::FileTransfer& transfer, 
                 FileTransferListener* listener);
        virtual ~FileNode();

        void BeginTransfer();
        void CancelTransfer();

        const teamtalk::FileTransfer& GetFileTransferInfo() const { return m_transfer; }

        //TimerListener functions
        int TimerEvent(ACE_UINT32 timer_event_id, long userdata) override;

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

        bool IsCompleted() const { return m_completed; }


    protected:
        void OnClosed();
        bool OnReceive(const char* buff, int len);
        bool OnSend(ACE_Message_Queue_Base& msg_queue);
        void InitTransfer();
        void UpdateBytesTransferred();

        void ProcessCommand(const ACE_CString& cmdline);
        void HandleWelcome(const mstrings_t& properties);
        void HandleError(const mstrings_t& properties);
        void HandleFileDeliver(const mstrings_t& properties);
        void HandleFileReady(const mstrings_t& properties); //response when file is ready to be received
        void HandleFileCompleted(const mstrings_t& properties);

        void Connect();
        void Disconnect();

        void DoSendFile();
        void DoRecvFile();
        void DoFileDeliver();


    private:
        void TransmitCommand(const ACE_TString& command);

        void SendFile(ACE_Message_Queue_Base& msg_queue);

        void CloseTransfer();

        ACE_Reactor& m_reactor;
        FileTransferListener* m_listener = nullptr;

        long m_timerid = -1;
        ACE_INET_Addr m_remoteAddr;
        ServerProperties m_srvprop;
        bool m_binarymode = false;
        ACE_CString m_readbuffer, m_sendbuffer;
        std::vector<char> m_filebuffer;

        bool m_pending_complete = false, m_completed = false;

#if defined(ENABLE_ENCRYPTION)
        crypt_connector_t m_crypt_connector;
        CryptStreamHandler::StreamHandler_t* m_crypt_stream = nullptr;
#endif
        connector_t m_connector;
        DefaultStreamHandler::StreamHandler_t* m_def_stream = nullptr;
        teamtalk::FileTransfer m_transfer;
        MyFile m_file;
    };
}

#endif
