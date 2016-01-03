/*
 * Copyright (c) 2005-2016, BearWare.dk
 *
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product
 * documentation is required.
 *
 */

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

TTBOOL getTTBOOL(TTMessage* m) {
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

void convertTuple(const char* str, StringWrap* wrap) {
    strncpy(wrap->buf, str, TT_STRLEN);
}


