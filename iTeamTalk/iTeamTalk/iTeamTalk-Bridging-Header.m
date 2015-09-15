//
//  iTeamTalk-Bridging-Header.m
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 7-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "iTeamTalk-Bridging-Header.h"

Channel* getChannel(TTMessage* m) {
    return &m->channel;
}

ServerProperties* getServerProperties(TTMessage* m) {
    return &m->serverproperties;
}

UserAccount* getUserAccount(TTMessage* m) {
    return &m->useraccount;
}

bool getBoolean(TTMessage* m) {
    return m->bActive;
}
