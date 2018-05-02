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

#if !defined(ACCEPTHANDLER_H)
#define ACCEPTHANDLER_H

#include <ace/Acceptor.h>
#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Acceptor.h>

#include <teamtalk/StreamHandler.h>

#if defined(ENABLE_ENCRYPTION)
#include <ace/SSL/SSL_SOCK_Stream.h>
#include <ace/SSL/SSL_SOCK_Acceptor.h>
#endif


#if defined(ENABLE_ENCRYPTION)

class My_SSL_SOCK_Acceptor : public ACE_SOCK_Acceptor
{
public:
  int accept (ACE_SSL_SOCK_Stream &new_stream,
              ACE_Addr *remote_addr = 0,
              ACE_Time_Value *timeout = 0,
              int restart = 1,
              int reset_new_handle = 0) const;
};
#endif

template < typename STREAMHANDLER, typename MYACCEPTOR >
class Acceptor : public ACE_Acceptor< STREAMHANDLER, MYACCEPTOR >
{
    typedef ACE_Acceptor< STREAMHANDLER, MYACCEPTOR > super;

public:
    Acceptor(ACE_Reactor* r = ACE_Reactor::instance())
        : super(r)
        , m_listener(NULL)
    {
    }

    void SetListener(typename STREAMHANDLER::StreamListener_t * lsn)
    {
        m_listener = lsn;
    }

    virtual int activate_svc_handler (STREAMHANDLER* svc_handler)
    {
        svc_handler->SetListener(m_listener);
        int ret = super::activate_svc_handler( svc_handler );
        // if(svc_handler && m_pListener && ret != -1)
        //     m_pListener->NewClient(*svc_handler);

        return ret;

    }

    //virtual int accept_svc_handler (SSLStreamHandler* svc_handler);

private:
    typename STREAMHANDLER::StreamListener_t * m_listener;
};

typedef Acceptor< DefaultStreamHandler, ACE_SOCK_ACCEPTOR > DefaultAcceptor;

#if defined(ENABLE_ENCRYPTION)
typedef Acceptor< CryptStreamHandler, My_SSL_SOCK_Acceptor > CryptAcceptor;
#endif

#endif
