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

User* getUser(TTMessage* m) {
    return &m->user;
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

ClientErrorMsg* getClientErrorMsg(TTMessage* m){
    return &m->clienterrormsg;
}

TextMessage* getTextMessage(TTMessage* m) {
    return &m->textmessage;
}

SpeexCodec* getSpeexCodec(AudioCodec* a) {
    return &a->speex;
}

SpeexVBRCodec* getSpeexVBRCodec(AudioCodec* a) {
    return &a->speex_vbr;
}

OpusCodec* getOpusCodec(AudioCodec* a) {
    return &a->opus;
}

void setOpusCodec(AudioCodec* a, OpusCodec* o) {
    a->nCodec = OPUS_CODEC;
    a->opus = *o;
}

void setSpeexCodec(AudioCodec* a, SpeexCodec* s) {
    a->nCodec = SPEEX_CODEC;
    a->speex = *s;
}

void setSpeexVBRCodec(AudioCodec* a, SpeexVBRCodec* s) {
    a->nCodec = SPEEX_VBR_CODEC;
    a->speex_vbr = *s;
}

void toTTString(const char* str, TTCHAR* buf) {
    if(str) {
        strncpy(buf, str, TT_STRLEN);
    }
}

const char* fromTTString(const TTCHAR* buf) {
    return buf;
}
