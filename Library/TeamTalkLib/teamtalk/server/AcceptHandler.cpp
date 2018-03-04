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

#include "AcceptHandler.h"

#if defined(ENABLE_ENCRYPTION)
int My_SSL_SOCK_Acceptor::accept (ACE_SSL_SOCK_Stream &new_stream,
              ACE_Addr *remote_addr/* = 0*/,
              ACE_Time_Value *timeout/* = 0*/,
              int restart/* = 1*/,
              int reset_new_handle/* = 0*/) const
{
    ACE_SOCK_Stream temp_stream;
    if(ACE_SOCK_Acceptor::accept(temp_stream, remote_addr, timeout, restart, reset_new_handle) == -1)
        return -1;

    new_stream.set_handle (temp_stream.get_handle ());
    temp_stream.set_handle (ACE_INVALID_HANDLE);
    return 0;
}
#endif


//
//int Acceptor::accept_svc_handler (SSLStreamHandler* svc_handler)
//{
//    int reset_new_handle = this->reactor ()->uses_event_associations ();
//
//    ACE_Time_Value tm = ACE_Time_Value(0,0);
//    if (this->acceptor ().accept (svc_handler->peer (), // stream
//        0, // remote address
//        &tm, // timeout
//        1, // restart
//        reset_new_handle  // reset new handler
//        ) == -1)
//    {
//        int x = ACE_OS::last_error();
//        // Ensure that errno is preserved in case the svc_handler
//        // close() method resets it
//        ACE_Errno_Guard error(errno);
//
//        // Close down handler to avoid memory leaks.
//        svc_handler->close (0);
//
//        return -1;
//    }
//    else
//        return 0;
//}
