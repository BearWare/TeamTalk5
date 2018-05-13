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
        int packetprotocol;
        ACE_TString motd_raw;
        std::vector<ACE_INET_Addr> hostaddrs;
        ACE_INET_Addr udpaddr; // same as hostaddrs[0] but port number may be different
        ServerInfo()
        {
            packetprotocol = 0;
        }
    };

    typedef ACE_Connector<DefaultStreamHandler::StreamHandler_t, ACE_SOCK_CONNECTOR> connector_t;
#if defined(ENABLE_ENCRYPTION)
    typedef ACE_Connector<CryptStreamHandler::StreamHandler_t, ACE_SSL_SOCK_Connector> crypt_connector_t;
#endif
    
}
#endif
