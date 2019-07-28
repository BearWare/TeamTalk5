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

#if !defined(CLIENTCHANNEL_H)
#define CLIENTCHANNEL_H

#include <teamtalk/Channel.h>

namespace teamtalk {
    class ClientUser;
    typedef std::shared_ptr< ClientUser > clientuser_t;
    class ClientChannel;
    typedef std::shared_ptr< ClientChannel > clientchannel_t;
}


#include "ClientUser.h"

namespace teamtalk {

    class ClientChannel : public teamtalk::Channel< ClientChannel, ClientUser >
    {
        typedef teamtalk::Channel< ClientChannel, ClientUser > PARENT;
    public:
        ClientChannel(int channelid);//create a root
        ClientChannel(channel_t& parent, int channelid, 
                      const ACE_TString& name);//create a subchannel
        ~ClientChannel();
    };
}
#endif
