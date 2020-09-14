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

#if !defined(TTUNITTEST_H)
#define TTUNITTEST_H

#include <TeamTalk.h>

#include <functional>
#include <memory>

#define DEFWAIT 5000

#define SOUNDDEVICEID_DEFAULT -1
#define SOUNDDEVICEID_IGNORE  -2

enum SoundMode
{
    DEFAULT         = 0x0,
    DUPLEX          = 0x1,
    SHARED_INPUT    = 0x2,
};

bool InitSound(TTInstance* ttClient, SoundMode mode = DEFAULT, INT32 indev = SOUNDDEVICEID_DEFAULT, INT32 outdev = SOUNDDEVICEID_DEFAULT);
bool GetSoundDevices(SoundDevice& insnddev, SoundDevice& outsnddev, INT32 indev = SOUNDDEVICEID_DEFAULT, INT32 outdev = SOUNDDEVICEID_DEFAULT);
bool Connect(TTInstance* ttClient, const TTCHAR hostname[TT_STRLEN], INT32 tcpport, INT32 udpport, TTBOOL encrypted = FALSE);
bool Login(TTInstance* ttClient, const TTCHAR nickname[TT_STRLEN], const TTCHAR username[TT_STRLEN], const TTCHAR passwd[TT_STRLEN]);
bool JoinRoot(TTInstance* ttClient);
Channel MakeChannel(TTInstance* ttClient, const TTCHAR* name, int parentid, const AudioCodec& codec);
bool WaitForEvent(TTInstance* ttClient, ClientEvent ttevent, std::function<bool(TTMessage)> pred, TTMessage* outmsg = nullptr, int timeout = DEFWAIT);
bool WaitForEvent(TTInstance* ttClient, ClientEvent ttevent, TTMessage& outmsg, int timeout = DEFWAIT);
bool WaitForEvent(TTInstance* ttClient, ClientEvent ttevent, int timeout = DEFWAIT);
bool WaitForCmdSuccess(TTInstance* ttClient, int cmdid, TTMessage* outmsg = nullptr, int timeout = DEFWAIT);
bool WaitForCmdComplete(TTInstance* ttClient, int cmdid, TTMessage* outmsg = nullptr, int timeout = DEFWAIT);

struct TTInst
{
    TTInst(const TTInst&) = delete;
    void operator=(const TTInst&) = delete;

    TTInstance* ttInst;
    TTInst(TTInstance* ttClient) : ttInst(ttClient) {}
    ~TTInst() { TT_CloseTeamTalk(ttInst); }
};

class ttinst : public std::shared_ptr<TTInst>
{
public:
    ttinst() {}
    ttinst(TTInstance* ttClient) { reset(new TTInst(ttClient)); }
    operator TTInstance*() { return get()->ttInst; }
};

#endif
