//
//  Use this file to import your target's public headers that you would like to expose to Swift.
//

#import "../../TeamTalk_DLL/TeamTalk.h"

Channel* getChannel(TTMessage* m);
User* getUser(TTMessage* m);
ServerProperties* getServerProperties(TTMessage* m);
UserAccount* getUserAccount(TTMessage* m);
bool getBoolean(TTMessage* m);
ClientErrorMsg* getClientErrorMsg(TTMessage* m);
TextMessage* getTextMessage(TTMessage* m);
SpeexCodec* getSpeexCodec(AudioCodec* a);
SpeexVBRCodec* getSpeexVBRCodec(AudioCodec* a);
OpusCodec* getOpusCodec(AudioCodec* a);
void setOpusCodec(AudioCodec* a, OpusCodec* o);
void setSpeexCodec(AudioCodec* a, SpeexCodec* s);
void setSpeexVBRCodec(AudioCodec* a, SpeexVBRCodec* s);


void toTTString(const char* str, TTCHAR* buf);
const char* fromTTString(const TTCHAR* buf);
