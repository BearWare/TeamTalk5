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

#if !defined(SERVER_H)
#define SERVER_H

#include <teamtalk/Common.h>

namespace teamtalk {

    struct ServerSettings : public ServerProperties
    {
        ACE_TString filesroot; //files root directory            
        std::vector<ACE_INET_Addr> tcpaddrs;
        std::vector<ACE_INET_Addr> udpaddrs;
        int rxloss = 0, txloss = 0;

        ServerSettings()
        {
            autosave = false;
            maxusers = MAX_USERS;
            diskquota = 0;
            maxdiskusage = 0;
            usertimeout = USER_TIMEOUT;
        }
    };

}

#endif
