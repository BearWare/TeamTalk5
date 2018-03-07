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

/*
 * The TeamTalk header and library files are located in 
 * the TeamTalk 5 SDK.
 *
 * To download the TeamTalk 5 SDK go to the GitHub website:
 *
 * https://github.com/BearWare/TeamTalk5
 *
 * Once downloaded copy TeamTalk.h from the downloaded 
 * TeamTalk 5 SDK folder "Library/C-API/TeamTalk_DLL" 
 * to the folder "../../TeamTalk_DLL"
 *
 * Also copy libTeamTalk5.a or libTeamTalk5Pro.a from the 
 * downloaded TeamTalk 5 SDK folder "Library" to the
 * folder ../../TeamTalk_DLL/lib/ios
 */

#import "../../../Library/TeamTalk_DLL/TeamTalk.h"

Channel* getChannel(TTMessage* m);
User* getUser(TTMessage* m);
ServerProperties* getServerProperties(TTMessage* m);
UserAccount* getUserAccount(TTMessage* m);
TTBOOL getTTBOOL(TTMessage* m);
ClientErrorMsg* getClientErrorMsg(TTMessage* m);
TextMessage* getTextMessage(TTMessage* m);
SpeexCodec* getSpeexCodec(AudioCodec* a);
SpeexVBRCodec* getSpeexVBRCodec(AudioCodec* a);
OpusCodec* getOpusCodec(AudioCodec* a);
void setOpusCodec(AudioCodec* a, OpusCodec* o);
void setSpeexCodec(AudioCodec* a, SpeexCodec* s);
void setSpeexVBRCodec(AudioCodec* a, SpeexVBRCodec* s);

typedef struct StringWrap {
    TTCHAR buf[TT_STRLEN];
} StringWrap;

void convertTuple(const char* str, StringWrap* wrap);
TTCHAR* fromStringWrap(StringWrap* wrap);
