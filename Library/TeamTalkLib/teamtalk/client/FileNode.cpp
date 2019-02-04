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

#include "FileNode.h"
#include <myace/MyACE.h>
#include <teamtalk/ttassert.h>
#include <ace/FILE_Connector.h>

using namespace std;
using namespace teamtalk;

#define FILEBUFFERSIZE 0x10000

FileNode::FileNode(ACE_Reactor& reactor, bool encrypted,
                   const ACE_INET_Addr& addr, const ServerProperties& srvprop,
                   const teamtalk::FileTransfer& transfer, 
                   FileTransferListener* listener)
: m_reactor(reactor)
, m_listener(listener)
, m_timerid(-1)
, m_binarymode(false)
, m_remoteAddr(addr)
, m_srvprop(srvprop)
, m_completed(false)
, m_pending_complete(false)
, m_transfer(transfer)
#if defined(ENABLE_ENCRYPTION)
, m_crypt_connector(&reactor)
, m_crypt_stream(NULL)
#endif
, m_connector(&reactor)
, m_def_stream(NULL)
{
    m_filebuffer.resize(FILEBUFFERSIZE);

#if defined(ENABLE_ENCRYPTION)
    if(encrypted)
    {
        ACE_NEW(m_crypt_stream, CryptStreamHandler(0, 0, &m_reactor));
        m_crypt_stream->msg_queue()->high_water_mark(FILEBUFFERSIZE);
        m_crypt_stream->msg_queue()->low_water_mark(FILEBUFFERSIZE);
    }
    else
#endif
    {
        ACE_NEW(m_def_stream, DefaultStreamHandler(0, 0, &m_reactor));
        m_def_stream->msg_queue()->high_water_mark(FILEBUFFERSIZE);
        m_def_stream->msg_queue()->low_water_mark(FILEBUFFERSIZE);
    }
}

FileNode::~FileNode()
{
    CancelTransfer();

#if defined(ENABLE_ENCRYPTION)
    if(m_crypt_stream)
        m_crypt_stream->SetListener(NULL);
#endif
    if(m_def_stream)
        m_def_stream->SetListener(NULL);
    MYTRACE(ACE_TEXT("~FileNode()\n"));
}

void FileNode::BeginTransfer()
{
    m_timerid = m_reactor.schedule_timer(new TimerHandler(*this, 1), 0, ACE_Time_Value(0,1));
    TTASSERT(m_timerid>=0);
}

void FileNode::CancelTransfer()
{
    int ret = 0;
    Disconnect();

    if(m_timerid>=0)
        m_reactor.cancel_timer(m_timerid, 0, 0);
    m_timerid = -1;

    if(m_file.get_handle() != ACE_INVALID_HANDLE)
    {
        m_file.close();
    }
}

void FileNode::InitTransfer()
{
    int ret;
    TTASSERT(m_file.get_handle() == ACE_INVALID_HANDLE);

    ACE_FILE_Connector con;
    if(m_transfer.inbound)
    {
#ifdef WIN32
        ret = con.connect(m_file, ACE_FILE_Addr(m_transfer.localfile.c_str()),
                          0, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT | O_TRUNC, 
                          FILE_SHARE_READ | FILE_SHARE_WRITE);
#else
        ret = con.connect(m_file, ACE_FILE_Addr(m_transfer.localfile.c_str()),
                          0, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT | O_TRUNC);
#endif
        if(ret<0)
        {
            if(m_listener)
            {
                m_transfer.status = FILETRANSFER_ERROR;
                m_listener->OnFileTransferStatus(m_transfer);
                m_listener = NULL;
            }
        }
        else
        {
            Connect();
        }
    }
    else
    {
#ifdef WIN32
        ret = con.connect(m_file, ACE_FILE_Addr(m_transfer.localfile.c_str()), 
            0, ACE_Addr::sap_any, 0, O_RDONLY, FILE_SHARE_READ);
#else
        ret = con.connect(m_file, ACE_FILE_Addr(m_transfer.localfile.c_str()), 
            0, ACE_Addr::sap_any, 0, O_RDONLY);
#endif
        if(ret<0)
        {
            if(m_listener)
            {
                m_transfer.status = FILETRANSFER_ERROR;
                m_listener->OnFileTransferStatus(m_transfer);
                m_listener = NULL;
            }
        }
        else
        {
            Connect();
        }
    }
}

void FileNode::UpdateBytesTransferred()
{
    if(m_binarymode)
    {
        if(m_transfer.inbound)
            m_transfer.transferred = m_file.tell();
        else
        {
#if defined(ENABLE_ENCRYPTION)
            if(m_crypt_stream)
                return;
            else
#endif
            {
                if(m_def_stream)
                    m_transfer.transferred = m_def_stream->sent_;
                else
                    return;
            }
        }
    }
    if(m_completed || m_pending_complete)
        m_transfer.transferred = m_transfer.filesize;
}


void FileNode::Connect()
{
    int ret;
#if defined(ENABLE_ENCRYPTION)
    if(m_crypt_stream)
    {
        m_crypt_stream->SetListener(this);

        //ACE only supports OpenSSL on blocking sockets
        ACE_Synch_Options options(ACE_Synch_Options::USE_TIMEOUT, ACE_Time_Value(10));
        ret = m_crypt_connector.connect(m_crypt_stream, m_remoteAddr, options);
    }
    else
#endif
    {
        TTASSERT(m_def_stream);
        m_def_stream->SetListener(this);

        ACE_Synch_Options options(ACE_Synch_Options::USE_REACTOR, ACE_Time_Value(30,0));
        ret = m_connector.connect(m_def_stream, m_remoteAddr, options);
    }
    if(ret == -1 && ACE_OS::last_error() != EWOULDBLOCK)
    {
        if(m_listener)
        {
            m_transfer.status = FILETRANSFER_ERROR;
            m_listener->OnFileTransferStatus(m_transfer);
            m_listener = NULL;
        }
    }
}

void FileNode::Disconnect()
{
    ACE_HANDLE h = ACE_INVALID_HANDLE;
#if defined(ENABLE_ENCRYPTION)
    if(m_crypt_stream)
    {
        //NOTE: same in ClientNode::Disconnect()

        m_crypt_stream->SetListener(NULL);
        h = m_crypt_stream->get_handle();

        m_crypt_connector.cancel(m_crypt_stream);
        m_crypt_stream->close();

        m_crypt_stream = NULL;
    }
#endif

    if(m_def_stream)
    {
        //NOTE: same in ClientNode::Disconnect()

        m_def_stream->SetListener(NULL);
        h = m_def_stream->get_handle();
        //if we're in the process of connecting, then cancel() which
        //doesn't destroy the handler. We destroy it by calling close 
        //manually
        m_connector.cancel(m_def_stream);
        m_def_stream->close();

        m_def_stream = NULL;
    }
    TTASSERT(m_reactor.find_handler(h) == NULL);
}

void FileNode::TransmitCommand(const ACE_TString& command)
{
    MYTRACE(ACE_TEXT("FILENODE: %s"), command.c_str());
    int ret = -1;
    ACE_Time_Value tm = ACE_Time_Value::zero;
#if defined(UNICODE)
    ACE_CString data = UnicodeToUtf8(command.c_str());
#else
    const ACE_CString& data = command;
#endif

    bool empty = m_sendbuffer.empty();
    m_sendbuffer += data;
#if defined(ENABLE_ENCRYPTION)
    if (m_crypt_stream && empty)
    {
        ret = m_reactor.register_handler(m_crypt_stream, ACE_Event_Handler::WRITE_MASK);
        TTASSERT(ret >= 0);
    }
#endif
    if (m_def_stream && empty)
    {
        ret = m_reactor.register_handler(m_def_stream, ACE_Event_Handler::WRITE_MASK);
        TTASSERT(ret >= 0);
    }
}

int FileNode::TimerEvent(ACE_UINT32 timer_event_id, long userdata)
{
    m_timerid = -1;
    InitTransfer();
    return -1;
}

#if defined(ENABLE_ENCRYPTION)
void FileNode::OnOpened(CryptStreamHandler::StreamHandler_t& handler)
{
    MYTRACE(ACE_TEXT("FILENODE: Connected successfully on TCP\n") );
    int size = FILEBUFFERSIZE, optlen = sizeof(size);
    int ret;

    ret = handler.peer().set_option(SOL_SOCKET, SO_SNDBUF, (char*)&size, optlen);
    TTASSERT(ret == 0);
    ret = handler.peer().set_option(SOL_SOCKET, SO_RCVBUF, (char*)&size, optlen);
    TTASSERT(ret == 0);
}
#endif /* ENABLE_ENCRYPTION */

void FileNode::OnOpened(DefaultStreamHandler::StreamHandler_t& handler)
{
    MYTRACE(ACE_TEXT("FILENODE: Connected successfully on TCP\n") );
    int size = FILEBUFFERSIZE, optlen = sizeof(size);
    int ret;

    ret = handler.peer().set_option(SOL_SOCKET, SO_SNDBUF, (char*)&size, optlen);
    TTASSERT(ret == 0);
    ret = handler.peer().set_option(SOL_SOCKET, SO_RCVBUF, (char*)&size, optlen);
    TTASSERT(ret == 0);
}

void FileNode::OnClosed()
{
#if defined(ENABLE_ENCRYPTION)
    m_crypt_stream = NULL;
#endif
    m_def_stream = NULL;

    if(m_listener && !m_pending_complete)
    {
        m_transfer.status = FILETRANSFER_ERROR;
        m_listener->OnFileTransferStatus(m_transfer);
    }
    Disconnect();
}

#if defined(ENABLE_ENCRYPTION)
void FileNode::OnClosed(CryptStreamHandler::StreamHandler_t& handler)
{
    //reactor deletes stream instance
    m_crypt_stream = NULL;
    OnClosed();
}
#endif

void FileNode::OnClosed(DefaultStreamHandler::StreamHandler_t& handler)
{
    m_def_stream = NULL;
    OnClosed();
}

bool FileNode::OnReceive(const char* buff, int len)
{
    if(m_binarymode)
    {
        TTASSERT(m_transfer.inbound);
        TTASSERT(m_file.get_handle() != ACE_INVALID_HANDLE);
        ACE_INT64 writebytes = len;
        if(m_file.tell() + len > m_transfer.filesize)
            writebytes = m_transfer.filesize - m_file.tell();
        ssize_t ret = m_file.send_n(buff, (size_t)writebytes);

        UpdateBytesTransferred();

        if(m_file.tell() == m_transfer.filesize)
        {
            CloseTransfer();
            if(m_listener)
            {
                m_transfer.status = FILETRANSFER_FINISHED;
                m_listener->OnFileTransferStatus(m_transfer);
                m_completed = true;
                m_listener = NULL;
            }
        }
    }
    else
    {
        m_readbuffer.append(buff, len);
        ACE_CString cmd;
        ACE_CString remain;
        while(GetCmdLine(m_readbuffer, cmd, remain))
        {
            ProcessCommand(cmd);
            m_readbuffer = remain;
        }
    }
    return true;
}

#if defined(ENABLE_ENCRYPTION)
bool FileNode::OnReceive(CryptStreamHandler::StreamHandler_t& handler, const char* buff, int len)
{
    return OnReceive(buff, len);
}
#endif

bool FileNode::OnReceive(DefaultStreamHandler::StreamHandler_t& handler, const char* buff, int len)
{
    return OnReceive(buff, len);
}

bool FileNode::OnSend(ACE_Message_Queue_Base& msg_queue)
{
    if(m_binarymode && m_transfer.inbound == false)
    {
        if(m_file.tell() < m_transfer.filesize)
        {
            SendFile(msg_queue);
            return true;
        }

        m_binarymode = false;
    }

    if(m_sendbuffer.length())
    {
        ACE_Time_Value tm = ACE_Time_Value::zero;
        if (QueueStreamData(msg_queue, m_sendbuffer.c_str(), 
                            (int)m_sendbuffer.length(), &tm) < 0)
        {
            TTASSERT(0);
            return false;
        }
        m_sendbuffer.clear();
    }

    return true;
}

#if defined(ENABLE_ENCRYPTION)
bool FileNode::OnSend(CryptStreamHandler::StreamHandler_t& handler)
{
    return OnSend(*handler.msg_queue());
}
#endif

bool FileNode::OnSend(DefaultStreamHandler::StreamHandler_t& handler)
{
    return OnSend(*handler.msg_queue());
}

void FileNode::DoRecvFile()
{
    ACE_TString cmd;
    cmd = CLIENT_RECVFILE;
    AppendProperty(TT_TRANSFERID, m_transfer.transferid, cmd);
    cmd += EOL;

    TransmitCommand(cmd);
}

void FileNode::DoSendFile()
{
    ACE_TString cmd;
    cmd = CLIENT_SENDFILE;
    AppendProperty(TT_TRANSFERID, m_transfer.transferid, cmd);
    cmd += EOL;

    TransmitCommand(cmd);
}

void FileNode::DoFileDeliver()
{
    ACE_TString cmd;
    cmd = CLIENT_DELIVERFILE;
    cmd += EOL;

    TransmitCommand(cmd);
}

void FileNode::ProcessCommand(const ACE_CString& cmdline)
{
    ACE_CString tmp_cmd;
    if(!GetCmd(cmdline, tmp_cmd))
        return;

#if defined(UNICODE)
    ACE_TString cmd = Utf8ToUnicode(tmp_cmd.c_str(), (int)tmp_cmd.length());
    ACE_TString command = Utf8ToUnicode(cmdline.c_str(), (int)cmdline.length());
#else
    ACE_TString cmd = tmp_cmd;
    const ACE_TString& command = cmdline;
#endif

    MYTRACE(ACE_TEXT("SERVER: %s"), command.c_str());

    mstrings_t properties;
    if(ExtractProperties(command, properties)<0)
        return;

    if(stringcmpnocase(cmd, SERVER_ERROR))
        HandleError(properties);
    else if(stringcmpnocase(cmd, m_srvprop.systemid))
        HandleWelcome(properties);
    else if(stringcmpnocase(cmd, SERVER_FILE_DELIVER))
        HandleFileDeliver(properties);
    else if(stringcmpnocase(cmd, SERVER_FILE_READY))
        HandleFileReady(properties);
    else if(stringcmpnocase(cmd, SERVER_FILE_COMPLETED))
        HandleFileCompleted(properties);
}

void FileNode::HandleWelcome(const mstrings_t& properties)
{
    if(m_transfer.inbound)
        DoRecvFile();
    else
        DoSendFile();
}

void FileNode::HandleError(const mstrings_t& properties)
{
    int nErrorNum = 0;
    GetProperty(properties, TT_ERRORNUM, nErrorNum);

    if(m_listener)
    {
        m_transfer.status = FILETRANSFER_ERROR;
        m_listener->OnFileTransferStatus(m_transfer);
        m_listener = NULL;
    }
}

void FileNode::HandleFileDeliver(const mstrings_t& properties)
{
    ACE_INT64 filesize = 0;
    int transid = 0;
    GetProperty(properties, TT_FILESIZE, filesize);
    TTASSERT(filesize == m_transfer.filesize);

    m_binarymode = true;
    
#if defined(ENABLE_ENCRYPTION)
    if(m_crypt_stream)
        m_crypt_stream->sent_ = 0;
#endif
    if(m_def_stream)
        m_def_stream->sent_ = 0;

    if(m_listener)
    {
        m_transfer.status = FILETRANSFER_ACTIVE;
        m_listener->OnFileTransferStatus(m_transfer);
    }

#if defined(ENABLE_ENCRYPTION)
    if(m_crypt_stream)
    {
        int ret = m_reactor.register_handler(m_crypt_stream, ACE_Event_Handler::WRITE_MASK);
        TTASSERT(ret >= 0);
    }
#endif
    if(m_def_stream)
    {
        int ret = m_reactor.register_handler(m_def_stream, ACE_Event_Handler::WRITE_MASK);
        TTASSERT(ret >= 0);
    }
}

void FileNode::HandleFileReady(const mstrings_t& properties) //response when file is ready to be received
{
    m_file.truncate(0); //delete the contents of the file if any
    int transferid = 0;
    GetProperty(properties, TT_TRANSFERID, transferid);
    TTASSERT(m_transfer.transferid == transferid);
    GetProperty(properties, TT_FILESIZE, m_transfer.filesize);

    if(m_listener)
    {
        m_transfer.status = FILETRANSFER_ACTIVE;
        m_listener->OnFileTransferStatus(m_transfer);
    }
    DoFileDeliver();
    m_binarymode = true;
}

void FileNode::HandleFileCompleted(const mstrings_t& properties)
{
    TTASSERT(m_binarymode == false);
    CloseTransfer();
    m_completed = true;
    if(m_listener)
    {
        m_transfer.status = FILETRANSFER_FINISHED;
        m_listener->OnFileTransferStatus(m_transfer);
        m_listener = NULL;
    }
}

void FileNode::SendFile(ACE_Message_Queue_Base& msg_queue)
{
    ssize_t ret = 0;
    ssize_t bytes = 0;
    TTASSERT(m_file.get_handle() != ACE_INVALID_HANDLE);
    TTASSERT(m_binarymode);

    while(true/*streamhandler_.msg_queue()->message_count()<10*/)
    {
        bytes = m_file.recv(&m_filebuffer[0], m_filebuffer.size());
        TTASSERT(ret>=0);

        if(bytes>0)
        {
            ACE_Time_Value tm = ACE_Time_Value::zero;
            ret = QueueStreamData(msg_queue, &m_filebuffer[0], (int)bytes, &tm);
            if(ret<0)
            {
                m_file.seek(m_file.tell()-bytes, SEEK_SET);    //rewind since we didn't send
                break;
            }
            else if(m_file.tell() >= m_transfer.filesize)
            {
                //we have sent everything now wait for HandleFileCompleted()
                m_pending_complete = true;
                break;
            }
        }
        else
            break;
    }

    UpdateBytesTransferred();
}

void FileNode::CloseTransfer()
{
    UpdateBytesTransferred();

    m_binarymode = false;
    if(m_file.get_handle() != ACE_INVALID_HANDLE)
        m_file.close();
}
