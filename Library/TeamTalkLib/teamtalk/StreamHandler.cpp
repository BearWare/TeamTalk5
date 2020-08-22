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

#include "StreamHandler.h"

#include <mutex>

int QueueStreamData(ACE_Message_Queue_Base& msg_q, 
                    const char* data, int len, ACE_Time_Value* tm/* = 0*/)
{
    ACE_Message_Block* mb;
    ACE_NEW_RETURN(mb, ACE_Message_Block(len), -1);
    int ret = mb->copy(data, len);
    TTASSERT(ret>=0);
    ret = msg_q.enqueue_tail(mb, tm);
    if(ret<0)
        mb->release();
    return ret;
}

#if defined(ENABLE_ENCRYPTION)

static std::mutex ctxmtx;

ACE_SSL_Context* CryptStreamHandler::AddSSLContext(ACE_Reactor* r)
{
    std::lock_guard<std::mutex> g(ctxmtx);

    TTASSERT(m_contexts.find(r) == m_contexts.end());
    
    m_contexts[r].reset(new ACE_SSL_Context());
    return m_contexts[r].get();
}

void CryptStreamHandler::RemoveSSLContext(ACE_Reactor* r)
{
    std::lock_guard<std::mutex> g(ctxmtx);

    m_contexts.erase(r);
}

ACE_SSL_Context* CryptStreamHandler::ssl_context(ACE_Reactor* r)
{
    std::lock_guard<std::mutex> g(ctxmtx);

    ACE_SSL_Context* c = ACE_SSL_Context::instance();
    if (m_contexts.find(r) != m_contexts.end())
        c = m_contexts[r].get();
    return c;
}

ssl_ctx_t CryptStreamHandler::m_contexts;

MySSLSockStream::MySSLSockStream(ACE_SSL_Context* ctx)
: ACE_SSL_SOCK_Stream(ctx)
{
}

void MySSLSockStream::ReinitSSL(ACE_SSL_Context* ctx)
{
    ::SSL_free (this->ssl_);

    this->ssl_ = ::SSL_new (ctx->context ());
}

/*
 * To ensure we can use multiple SSL contexts we have to map the reactor
 * to a SSL context (see CryptStreamHandler::m_contexts).
 *
 * When ACE_Acceptor call CryptStreamHandler-class (i.e. ACE_Svc_Handler)
 * it calls with an empty contructor and sets reactor afterwards. We have 
 * to handle this case in a bit weird way by checking if reactor already 
 * exists.
 */
CryptStreamHandler::CryptStreamHandler(ACE_Thread_Manager *thr_mgr,
                                       ACE_Message_Queue<ACE_MT_SYNCH> *mq,
                                       ACE_Reactor *reactor)
    : super(thr_mgr, mq, reactor)
{
    if (reactor)
        ssl_reset(reactor);
}

void CryptStreamHandler::reactor(ACE_Reactor *reactor)
{
    // ensure we don't double create SSL context
    auto r = super::reactor();

    super::reactor(reactor);

    if (r == nullptr)
        ssl_reset(reactor);
}

void CryptStreamHandler::ssl_reset(ACE_Reactor *reactor)
{
    ACE_SSL_Context* ctx = ssl_context(reactor);
    peer_.ReinitSSL(ctx);

    SSL_CTX* sslctx = ctx->context();

    SSL* ssl = peer().ssl();
    long opt = SSL_get_options(ssl);
    SSL_clear(ssl);
}

int CryptStreamHandler::handle_input(ACE_HANDLE fd/* = ACE_INVALID_HANDLE*/)
{
    SSL *ssl = peer().ssl();
    if (SSL_is_init_finished(ssl))
    {
        int ret = super::handle_input(fd);
        if (ret == 0 && ::SSL_pending(ssl))
            ret = 1;
        return ret;
    }
    else
        return process_ssl(ssl);
}

int CryptStreamHandler::handle_output(ACE_HANDLE fd/* = ACE_INVALID_HANDLE*/)
{
    SSL *ssl = peer().ssl();
    if (SSL_is_init_finished(ssl))
    {
        int ret = super::handle_output(fd);
        if(ret == 0 && ::SSL_pending(ssl))
            ret = 1;
        return ret;
    }
    else
        return process_ssl(ssl);
}

int CryptStreamHandler::process_ssl(SSL* ssl)
{
    if (!SSL_in_accept_init (ssl))
        SSL_set_accept_state (ssl);

    int status;
    status = SSL_accept(ssl);
    status = SSL_get_error(ssl, status);
    switch (status)
    {
    case SSL_ERROR_NONE:
    {
        super::reactor()->mask_ops(this, ACE_Event_Handler::WRITE_MASK, ACE_Reactor::ADD_MASK);
        return 1;
    }
    case SSL_ERROR_WANT_WRITE:
    {
        super::reactor()->mask_ops(this, ACE_Event_Handler::WRITE_MASK, ACE_Reactor::ADD_MASK);
        return 0;
    }
    case SSL_ERROR_WANT_READ:
    {
        if(!SSL_want_write (ssl))
            super::reactor()->mask_ops(this, ACE_Event_Handler::WRITE_MASK, ACE_Reactor::CLR_MASK);
        return 0;
    }
    case SSL_ERROR_ZERO_RETURN:
    {
        return -1;
    }
    case SSL_ERROR_SYSCALL:
    {
        if (ACE_OS::set_errno_to_last_error () == EWOULDBLOCK && status == -1)
        {
            if (SSL_want_write (ssl))
            {
                super::reactor()->mask_ops(this, ACE_Event_Handler::WRITE_MASK, ACE_Reactor::ADD_MASK);
                return 0;
            }
            else if(SSL_want_read (ssl))
            {
                return 0;
            }
        }
        return -1;            // Doesn't want anything - bail out
    }
    default:
        return -1;
        break;
    }
}

#endif /* ENABLE_ENCRYPTION */
