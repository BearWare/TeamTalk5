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

void CryptStreamHandler::AddSSLContext(ACE_Reactor* r, ACE_SSL_Context* c)
{
    m_contexts[r] = c;
}

ACE_SSL_Context* CryptStreamHandler::ssl_context(ACE_Reactor* r)
{
    ACE_SSL_Context* c = ACE_SSL_Context::instance();
    if(m_contexts.find(r) != m_contexts.end())
        c = m_contexts[r];
    return c;
}

ssl_ctx_t CryptStreamHandler::m_contexts;

CryptStreamHandler::CryptStreamHandler(ACE_Thread_Manager *thr_mgr,
                                       ACE_Message_Queue<ACE_MT_SYNCH> *mq,
                                       ACE_Reactor *reactor)
    : super(thr_mgr, mq, reactor)
{
    ACE_SSL_Context* ctx = ssl_context(reactor);
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
        this->reactor()->mask_ops(this, ACE_Event_Handler::WRITE_MASK, ACE_Reactor::ADD_MASK);
        return 1;
    }
    case SSL_ERROR_WANT_WRITE:
    {
        this->reactor()->mask_ops(this, ACE_Event_Handler::WRITE_MASK, ACE_Reactor::ADD_MASK);
        return 0;
    }
    case SSL_ERROR_WANT_READ:
    {
        if(!SSL_want_write (ssl))
            this->reactor()->mask_ops(this, ACE_Event_Handler::WRITE_MASK, ACE_Reactor::CLR_MASK);
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
                this->reactor()->mask_ops(this, ACE_Event_Handler::WRITE_MASK, ACE_Reactor::ADD_MASK);
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
