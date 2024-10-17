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

#if !defined(CLIENT_H)
#define CLIENT_H

#include <teamtalk/Common.h>

#include <ace/Connector.h> 
#include <ace/INET_Addr.h>

#if defined(ENABLE_ENCRYPTION)
#include <ace/SSL/SSL_SOCK_Connector.h>
#else
#include <ace/SOCK_Connector.h>
#endif

#include <teamtalk/StreamHandler.h>

namespace teamtalk {

    struct ServerInfo : public ServerProperties
    {
        ACE_TString protocol;
        int packetprotocol = 0;
        ACE_TString motd_raw;
        std::vector<ACE_INET_Addr> hostaddrs;
        ACE_INET_Addr udpaddr; // same as hostaddrs[0] but port number may be different
        ACE_TString accesstoken;
        ServerInfo() { }
    };

    typedef ACE_Connector<DefaultStreamHandler::StreamHandler_t, ACE_SOCK_CONNECTOR> connector_t;

#if defined(ENABLE_ENCRYPTION)
    template < typename STREAMHANDLER, typename PEER_CONNECTOR>
    class SSL_Connector : public ACE_Connector< STREAMHANDLER, PEER_CONNECTOR >
    {
        typedef ACE_Connector< STREAMHANDLER, PEER_CONNECTOR > super;

    public:
        SSL_Connector(ACE_Reactor* r, int flags = 0)
            : super(r, flags)
        {
        }

        ~SSL_Connector()
        {
            MYTRACE(ACE_TEXT("~Connector()\n"));
        }

        /* copy-paste from ACE_Connector::initialize_svc_handler except SSL handling */
        void initialize_svc_handler (ACE_HANDLE handle,
                                     STREAMHANDLER *svc_handler) override
        {
            // Try to find out if the reactor uses event associations for the
            // handles it waits on. If so we need to reset it.
            bool const reset_new_handle =
                    this->reactor ()->uses_event_associations ();

            if (reset_new_handle)
                this->connector().reset_new_handle (handle);

            // Transfer ownership of the ACE_HANDLE to the SVC_HANDLER.
            svc_handler->set_handle (handle);

            typename PEER_CONNECTOR::PEER_ADDR raddr;

            // Check to see if we're connected (takes peer().peer() which is the SSL socket).
            if (svc_handler->peer ().peer().get_remote_addr (raddr) != -1)
            {
                this->activate_svc_handler (svc_handler);
            }
            else // Somethings gone wrong, so close down...
            {
#if defined (ACE_WIN32)
                // Win32 (at least prior to Windows 2000) has a timing problem.
                // If you check to see if the connection has completed too fast,
                // it will fail - so wait 35 milliseconds to let it catch up.
                ACE_Time_Value tv (0, ACE_NON_BLOCKING_BUG_DELAY);
                ACE_OS::sleep (tv);
                if (svc_handler->peer ().get_remote_addr (raddr) != -1)
                    this->activate_svc_handler (svc_handler);
                else // do the svc handler close below...
#endif /* ACE_WIN32 */
                    svc_handler->close (NORMAL_CLOSE_OPERATION);
            }
        }

        int activate_svc_handler(STREAMHANDLER* svc_handler) override
        {
            int ret = super::activate_svc_handler(svc_handler);
            this->reactor()->mask_ops(svc_handler, ACE_Event_Handler::WRITE_MASK, ACE_Reactor::ADD_MASK);
            return ret;
        }
    };

    class My_SSL_SOCK_Connector : public ACE_SOCK_Connector
    {
    public:
        int connect(ACE_SSL_SOCK_Stream& new_stream,
            const ACE_Addr& remote_sap,
            const ACE_Time_Value* timeout = 0,
            const ACE_Addr& local_sap = ACE_Addr::sap_any,
            int reuse_addr = 0,
            int flags = 0,
            int perms = 0,
            int protocol = 0)
        {
            ACE_SOCK_Stream temp_stream;
            int ret = ACE_SOCK_Connector::connect(temp_stream, remote_sap, timeout, local_sap, reuse_addr, flags, perms, protocol);

            /* SSL_set_fd() causes ENOENT on Ubuntu 22 so we need to store previous errno.
             * Windows and macOS does not have this issue. */
            int lasterrno = ACE_OS::last_error();
            new_stream.set_handle(temp_stream.get_handle());
            temp_stream.set_handle(ACE_INVALID_HANDLE);
            ACE_OS::last_error(lasterrno);

            return ret;
        }
    };

    typedef SSL_Connector<CryptStreamHandler::StreamHandler_t, My_SSL_SOCK_Connector> crypt_connector_t;
#endif

}
#endif
