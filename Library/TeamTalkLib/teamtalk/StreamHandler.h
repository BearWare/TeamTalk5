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

#if !defined(STREAMHANDLER_H)
#define STREAMHANDLER_H

#include <ace/Reactor.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Svc_Handler.h>
#include <ace/Null_Mutex.h>
#include <ace/Reactor_Notification_Strategy.h>

#if defined(ENABLE_ENCRYPTION)
#include <ace/SSL/SSL_SOCK_Stream.h>
#include <ace/SSL/SSL_SOCK_Acceptor.h>
#else
#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Acceptor.h>
#endif

#include <teamtalk/ttassert.h>
#include <myace/MyACE.h>

#include <map>
#include <memory>

template < typename STREAMHANDLER >
class StreamListener
{
public:
    virtual void OnOpened(STREAMHANDLER& streamer) = 0;
    virtual void OnClosed(STREAMHANDLER& streamer) = 0;    //do NOT touch the StreamHandler object after this call (it has already called 'delete this')
    virtual bool OnReceive(STREAMHANDLER& streamer, const char* buff, int len) = 0; //return 'false' to unregister event handler
    virtual bool OnSend(STREAMHANDLER& streamer){ return true; /* return false to unregister event handler */ }
};

#define MSGBUFFERSIZE 0x100000

template < typename ACE_SOCK_STREAM_TYPE >
class StreamHandler : public ACE_Svc_Handler< ACE_SOCK_STREAM_TYPE, ACE_MT_SYNCH >
{
public:
    typedef ACE_Svc_Handler< ACE_SOCK_STREAM_TYPE, ACE_MT_SYNCH > super;
    typedef StreamHandler StreamHandler_t;
    StreamHandler(ACE_Thread_Manager *thr_mgr = 0,
                  ACE_Message_Queue<ACE_MT_SYNCH> *mq = 0,
                  ACE_Reactor *reactor = ACE_Reactor::instance())
    : super(thr_mgr, mq, reactor)
    , m_listener(NULL)
    , sent_(0)
    , recv_(0)
    {
        m_buffer.resize(4096);

        this->msg_queue()->high_water_mark(MSGBUFFERSIZE);
        this->msg_queue()->low_water_mark(MSGBUFFERSIZE);
    }

    virtual ~StreamHandler()
    {
        if(m_listener)
            m_listener->OnClosed(*this);

        ACE_HANDLE hh = this->get_handle();
#if defined (ACE_WIN32)
        MYTRACE(ACE_TEXT("~StreamHandler() %p. Handle: 0x%p, Sent: %u Recv: %u\n"),
                this, hh, ACE_UINT32(sent_), ACE_UINT32(recv_));
#else
        MYTRACE(ACE_TEXT("~StreamHandler() %p, Handle: 0x%x, Sent: %u Recv: %u\n"),
                this, hh, ACE_UINT32(sent_), ACE_UINT32(recv_));
#endif

    }

    void SetListener(StreamListener<StreamHandler>* listener)
    {
        m_listener = listener;
    }

    virtual int open(void* args = 0)
    {
        int ret = super::open(args);
        if(ret >= 0)
        {
            this->peer().enable(ACE_NONBLOCK);

            if(m_listener)
                m_listener->OnOpened(*this);
        }
        return ret;
    }

    //Callback to handle any input received
    virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE)
    {
        //receive the data
        ssize_t ret = this->peer().recv(&m_buffer[0], m_buffer.size());

        switch(ret)
        {
        case -1 :
            if(ACE_OS::last_error() == EWOULDBLOCK)
                return 0;
        case 0 :
            break; //disconnect
        default:
            recv_ += ret;
            if(m_listener)
            {
                if(!m_listener->OnReceive(*this, &m_buffer[0], (int)ret))
                    break;
            }

            return 0;    //continue to listen
        }

        //handle_close will be called afterwards
        return -1;
    }


    //Callback to handle any output received
    virtual int handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE)
    {
        if(m_listener && this->msg_queue()->is_empty())
            m_listener->OnSend(*this);

        ACE_Message_Block* mb = NULL;
        ACE_Time_Value nowait = ACE_Time_Value::zero;
        int ret;
        while((ret = this->getq(mb, &nowait)) >= 0)
        {
            size_t len = mb->length();
            TTASSERT(len>0);
            //MYTRACE("%p --[%s]--\n", this, std::string(mb->rd_ptr(), len).c_str());
            ssize_t send_cnt = this->peer().send(mb->rd_ptr(), mb->length(), &nowait);
            if(send_cnt > 0)
            {
                mb->rd_ptr(send_cnt);
                sent_ += send_cnt;
            }

            if(send_cnt < 0)
            {
                int e = ACE_OS::last_error();
                if (e != EWOULDBLOCK && e != ETIME && e != EINPROGRESS)
                {
                    mb->release();
                    return -1;    //something's wrong so drop the client
                }
            }

            if(mb->length() > 0)
            {
                this->ungetq(mb);
                break;
            }

            mb->release();

            if(this->msg_queue()->is_empty())
            {
                if(m_listener && !m_listener->OnSend(*this))
                {
                    return 0;
                }
            }
        }

        if(this->msg_queue()->is_empty())
            this->reactor()->mask_ops(this, ACE_Event_Handler::WRITE_MASK, ACE_Reactor::CLR_MASK);
        return 0;
    }

    // Called when a timer expires.
    virtual int handle_timeout (const ACE_Time_Value &current_time, const void *act)
    {
        TTASSERT(0);
        return -1;
    }

    virtual int handle_exception (ACE_HANDLE fd = ACE_INVALID_HANDLE)
    {
        TTASSERT(0);
        return -1;
    }

    ACE_INT64 sent_, recv_;

protected:
    StreamListener<StreamHandler>* m_listener;
    std::vector<char> m_buffer;
};

template < typename ACE_SOCK_STREAM_TYPE >
bool CloseStreamHandler(StreamHandler< ACE_SOCK_STREAM_TYPE >& handler)
{
    return handler.peer().close() != -1;
}

class DefaultStreamHandler : public StreamHandler<ACE_SOCK_STREAM>
{
public:
    typedef StreamListener< DefaultStreamHandler::StreamHandler_t > StreamListener_t;

    DefaultStreamHandler(ACE_Thread_Manager *thr_mgr = 0,
                         ACE_Message_Queue<ACE_MT_SYNCH> *mq = 0,
        ACE_Reactor *reactor = ACE_Reactor::instance())
        : StreamHandler<ACE_SOCK_STREAM>(thr_mgr, mq, reactor) { }
};

int QueueStreamData(ACE_Message_Queue_Base& msg_q, 
                    const char* data, int len, ACE_Time_Value* tm = 0);

#if defined(ENABLE_ENCRYPTION)

typedef std::map<ACE_Reactor*, std::shared_ptr<ACE_SSL_Context>> ssl_ctx_t;

class MySSLSockStream : public ACE_SSL_SOCK_Stream
{
public:
    MySSLSockStream(ACE_SSL_Context* ctx = ACE_SSL_Context::instance());
    // Horrifying hack to get around ACE's forced singleton SSL
    // context implementation
    void ReinitSSL(ACE_SSL_Context* ctx);
};

class CryptStreamHandler : public StreamHandler<MySSLSockStream>
{
public:
    typedef StreamHandler<MySSLSockStream> super;
    typedef StreamListener< CryptStreamHandler::StreamHandler_t > StreamListener_t;

    CryptStreamHandler(ACE_Thread_Manager *thr_mgr = 0,
                       ACE_Message_Queue<ACE_MT_SYNCH> *mq = 0,
                       ACE_Reactor *reactor = ACE_Reactor::instance());

    //Callback to handle any input received
    virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);

    //Callback to handle any output received
    virtual int handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE);

    static ACE_SSL_Context* AddSSLContext(ACE_Reactor* r);
    static void RemoveSSLContext(ACE_Reactor* r);

protected:
    int process_ssl(SSL* ssl);

    static ACE_SSL_Context* ssl_context(ACE_Reactor* r);
    static ssl_ctx_t m_contexts;
};

#endif /* ENABLE_ENCRYPTION */

#endif
