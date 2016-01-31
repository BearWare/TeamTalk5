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

#import "../../TeamTalk_DLL/TeamTalk.h"

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

