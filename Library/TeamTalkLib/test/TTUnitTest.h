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

#include <ace/OS.h> // BOOL, UINT32, etc needed on Windows
#include <TeamTalk.h>

#include <string>

#include <ace/SString.h> // get ACE_TEXT

#include <functional>
#include <memory>

#define DEFWAIT 5000

#define SOUNDDEVICEID_DEFAULT -1
#define SOUNDDEVICEID_IGNORE  -2

extern std::string g_server_ipaddr;
extern const bool GITHUBSKIP;

enum SoundMode
{
    DEFAULT                 = 0x0,
    DUPLEX                  = 0x1,
    SHARED_INPUT            = 0x2,
    SHARED_OUTPUT           = 0x4,
    SHARED_INPUT_OUTPUT     = 0x8,
};

bool InitSound(TTInstance* ttClient, SoundMode mode = DEFAULT, INT32 indev = SOUNDDEVICEID_DEFAULT, INT32 outdev = SOUNDDEVICEID_DEFAULT);
bool GetSoundDevices(SoundDevice& insnddev, SoundDevice& outsnddev, INT32 indev = SOUNDDEVICEID_DEFAULT, INT32 outdev = SOUNDDEVICEID_DEFAULT);
bool Connect(TTInstance* ttClient, INT32 tcpport = 10333, INT32 udpport = 10333, TTBOOL encrypted = FALSE);
bool Connect(TTInstance* ttClient, const TTCHAR* hostname, INT32 tcpport = 10333, INT32 udpport = 10333, TTBOOL encrypted = FALSE);
bool Login(TTInstance* ttClient, const TTCHAR nickname[TT_STRLEN], const TTCHAR* username = ACE_TEXT("guest"), const TTCHAR* passwd = ACE_TEXT("guest"));
bool JoinRoot(TTInstance* ttClient);
AudioCodec MakeDefaultAudioCodec(Codec codec);
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

ttinst InitTeamTalk();

class abptr
{
private:
    TTInstance* m_inst = nullptr;
    AudioBlock* m_ab = nullptr;
    abptr(const abptr&) = delete;
    void operator=(const abptr&) = delete;
public:
    abptr(TTInstance* inst, AudioBlock* ab) : m_inst(inst), m_ab(ab) {}
    ~abptr() { if (m_inst && m_ab) TT_ReleaseUserAudioBlock(m_inst, m_ab); }
    operator AudioBlock*() { return m_ab; }
    AudioBlock* operator->() const { return m_ab; }
    operator bool() const { return m_ab != nullptr; }
};

#endif
