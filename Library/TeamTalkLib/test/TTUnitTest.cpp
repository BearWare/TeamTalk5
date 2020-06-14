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

#include <ace/ACE.h>

#include "TTUnitTest.h"

#include <myace/MyACE.h>

bool InitSound(TTInstance* ttClient, SoundMode mode /*= DEFAULT*/, INT32 indev, INT32 outdev)
{
    int selindev = indev, seloutdev = outdev;
    if (indev == SOUNDDEVICEID_DEFAULT || outdev == SOUNDDEVICEID_DEFAULT)
    {
        if (!TT_GetDefaultSoundDevices(&indev, &outdev))
            return false;
    }

    if (selindev == SOUNDDEVICEID_DEFAULT)
        selindev = indev;
    if (seloutdev == SOUNDDEVICEID_DEFAULT)
        seloutdev = outdev;

    switch (mode)
    {
    case DUPLEX :
        if (selindev == SOUNDDEVICEID_IGNORE || seloutdev == SOUNDDEVICEID_IGNORE)
            return false;

        return TT_InitSoundDuplexDevices(ttClient, selindev, seloutdev);
    case SHARED_INPUT :
        if (selindev == SOUNDDEVICEID_IGNORE)
            return false;

        selindev |= TT_SOUNDDEVICE_ID_SHARED_FLAG;
        break;
    case DEFAULT :
        break;
    }

    TTBOOL success = true;
    if (selindev != SOUNDDEVICEID_IGNORE)
        success &= TT_InitSoundInputDevice(ttClient, selindev);
    if (seloutdev != SOUNDDEVICEID_IGNORE)
        success &= TT_InitSoundOutputDevice(ttClient, seloutdev);

    return success;
}

bool Connect(TTInstance* ttClient, const TTCHAR hostname[TT_STRLEN], INT32 tcpport, INT32 udpport)
{
    if (!TT_Connect(ttClient, hostname, tcpport, udpport, 0, 0, FALSE))
        return false;
    return WaitForEvent(ttClient, CLIENTEVENT_CON_SUCCESS);
}

bool Login(TTInstance* ttClient, const TTCHAR nickname[TT_STRLEN], const TTCHAR username[TT_STRLEN], const TTCHAR passwd[TT_STRLEN])
{
    return WaitForCmdSuccess(ttClient, TT_DoLogin(ttClient, nickname, username, passwd));
}

bool JoinRoot(TTInstance* ttClient)
{
    auto chanid = TT_GetRootChannelID(ttClient);
    return WaitForCmdSuccess(ttClient, TT_DoJoinChannelByID(ttClient, chanid, ACE_TEXT("")));
}

Channel MakeChannel(TTInstance* ttClient, const TTCHAR* name, int parentid, const AudioCodec& codec)
{
    Channel chan = {};
    chan.nParentID = parentid;
    ACE_OS::strsncpy(chan.szName, name, TT_STRLEN);
    chan.audiocodec = codec;
    return chan;
}

bool WaitForEvent(TTInstance* ttClient, ClientEvent ttevent, std::function<bool(TTMessage)> pred, TTMessage* outmsg, int timeout /*= DEFWAIT*/)
{
    TTMessage msg = {};
    auto start = GETTIMESTAMP();
    bool gotmsg;
    do
    {
        INT32 waitMsec = 0;
        gotmsg = TT_GetMessage(ttClient, &msg, &waitMsec);
        if (gotmsg && msg.nClientEvent == ttevent && pred(msg))
        {
            if (outmsg)
                *outmsg = msg;
            
            return true;
        }
    } while (GETTIMESTAMP() <= start + timeout || gotmsg);

    return false;
}

bool WaitForEvent(TTInstance* ttClient, ClientEvent ttevent, TTMessage& outmsg, int timeout /*= DEFWAIT*/)
{
    return WaitForEvent(ttClient, ttevent, [](TTMessage) { return true; }, &outmsg, timeout);
}

bool WaitForEvent(TTInstance* ttClient, ClientEvent ttevent, int timeout)
{
    TTMessage msg;
    return WaitForEvent(ttClient, ttevent, msg, timeout);
}

bool WaitForCmdSuccess(TTInstance* ttClient, int cmdid, TTMessage* outmsg, int timeout /*= DEFWAIT*/)
{
    bool result = WaitForEvent(ttClient, CLIENTEVENT_CMD_SUCCESS, [cmdid](TTMessage msg) {
        return msg.nSource == cmdid;
    }, outmsg, timeout);

    if (result)
        WaitForCmdComplete(ttClient, cmdid, outmsg, timeout);

    return result;
}

bool WaitForCmdComplete(TTInstance* ttClient, int cmdid, TTMessage* outmsg, int timeout /*= DEFWAIT*/)
{
    return WaitForEvent(ttClient, CLIENTEVENT_CMD_PROCESSING, [cmdid](TTMessage msg) {
        return msg.nSource == cmdid && !msg.bActive;
    }, outmsg, timeout);
}
