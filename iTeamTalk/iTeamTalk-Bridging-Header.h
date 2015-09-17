//
//  Use this file to import your target's public headers that you would like to expose to Swift.
//

#import "../TeamTalk_DLL/TeamTalk.h"

Channel* getChannel(TTMessage* m);
User* getUser(TTMessage* m);
ServerProperties* getServerProperties(TTMessage* m);
UserAccount* getUserAccount(TTMessage* m);
bool getBoolean(TTMessage* m);

void toTTString(const char* str, TTCHAR* buf);
