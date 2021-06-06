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

ttinst InitTeamTalk()
{
    ttinst inst(TT_InitTeamTalkPoll());
    return inst;
}

bool InitSound(TTInstance* ttClient, SoundMode mode /*= DEFAULT*/, INT32 indev, INT32 outdev)
{
    int selindev = indev, seloutdev = outdev;
    if (indev == SOUNDDEVICEID_DEFAULT || outdev == SOUNDDEVICEID_DEFAULT)
    {
        SoundDevice sndindev, sndoutdev;
        if (!GetSoundDevices(sndindev, sndoutdev, indev, outdev))
            return false;
        indev = sndindev.nDeviceID;
        outdev = sndoutdev.nDeviceID;
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
    {
        if (selindev == SOUNDDEVICEID_IGNORE)
            return false;

        selindev |= TT_SOUNDDEVICE_ID_SHARED_FLAG;
        break;
    }
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

bool GetSoundDevices(SoundDevice& insnddev, SoundDevice& outsnddev, INT32 indev/* = SOUNDDEVICEID_DEFAULT*/, INT32 outdev/* = SOUNDDEVICEID_DEFAULT*/)
{
    const int DEVMAX = 100;
    std::vector<SoundDevice> devs(DEVMAX);
    INT32 howmany = INT32(devs.size());
    TT_GetSoundDevices(&devs[0], &howmany);
    devs.resize(howmany);
    if (howmany == DEVMAX)
    {
        TT_GetSoundDevices(&devs[0], &howmany);
    }

    int defaultin, defaultout;
    
    ACE_TCHAR* envindev = ACE_OS::getenv(ACE_TEXT("INPUTDEVICEID"));
    ACE_TCHAR* envoutdev = ACE_OS::getenv(ACE_TEXT("OUTPUTDEVICEID"));
    if (envindev || envoutdev)
    {
        if (envindev)
            defaultin = ACE_OS::atoi(envindev);
        else if (!TT_GetDefaultSoundDevices(indev != SOUNDDEVICEID_IGNORE ? &defaultin : nullptr, nullptr))
            return false;

        if (envoutdev)
            defaultout = ACE_OS::atoi(envoutdev);
        else if (!TT_GetDefaultSoundDevices(nullptr, outdev != SOUNDDEVICEID_IGNORE ? &defaultout : nullptr))
            return false;
    }
    else
    {
        if (!TT_GetDefaultSoundDevices(indev != SOUNDDEVICEID_IGNORE ? &defaultin : nullptr,
                                       outdev != SOUNDDEVICEID_IGNORE ? &defaultout : nullptr))
            return false;
    }
    

    if (indev == SOUNDDEVICEID_DEFAULT)
        indev = defaultin;

    if (outdev == SOUNDDEVICEID_DEFAULT)
        outdev = defaultout;

    if (indev != SOUNDDEVICEID_IGNORE)
    {
        auto dev = std::find_if(devs.begin(), devs.end(), [indev](const SoundDevice& d)
        {
            return d.nDeviceID == indev;
        });
        if (dev == devs.end())
            return false;
        insnddev = *dev;
    }

    if (outdev != SOUNDDEVICEID_IGNORE)
    {
        auto dev = std::find_if(devs.begin(), devs.end(), [outdev] (const SoundDevice& d)
        {
            return d.nDeviceID == outdev;
        });
        if (dev == devs.end())
            return false;
        outsnddev = *dev;
    }
    return true;
}

bool Connect(TTInstance* ttClient, const TTCHAR* hostname, INT32 tcpport, INT32 udpport, TTBOOL encrypted)
{
    if (!TT_Connect(ttClient, hostname, tcpport, udpport, 0, 0, encrypted))
        return false;
    return WaitForEvent(ttClient, CLIENTEVENT_CON_SUCCESS);
}

bool Login(TTInstance* ttClient, const TTCHAR nickname[TT_STRLEN], const TTCHAR* username, const TTCHAR* passwd)
{
    return WaitForCmdSuccess(ttClient, TT_DoLogin(ttClient, nickname, username, passwd));
}

bool JoinRoot(TTInstance* ttClient)
{
    auto chanid = TT_GetRootChannelID(ttClient);
    return WaitForCmdSuccess(ttClient, TT_DoJoinChannelByID(ttClient, chanid, ACE_TEXT("")));
}

AudioCodec MakeDefaultAudioCodec(Codec codec)
{
    AudioCodec result = {};
    result.nCodec = codec;
    switch (codec)
    {
    case SPEEX_CODEC:
        result.speex.nQuality = 4;
        result.speex.nBandmode = 1;
        result.speex.nTxIntervalMSec = 40;
        result.speex.bStereoPlayback = FALSE;
        break;
    case SPEEX_VBR_CODEC:
        result.speex_vbr.nQuality = 4;
        result.speex_vbr.nBandmode = 1;
        result.speex_vbr.nTxIntervalMSec = 40;
        result.speex_vbr.bStereoPlayback = FALSE;
        result.speex_vbr.nBitRate = 0;
        result.speex_vbr.nMaxBitRate = 0;
        result.speex_vbr.bDTX = TRUE;
        break;
    case OPUS_CODEC:
        result.opus.nApplication = OPUS_APPLICATION_VOIP;
        result.opus.nSampleRate = 48000;
        result.opus.nChannels = 1;
        result.opus.nTxIntervalMSec = 40;
        result.opus.nComplexity = 10;
        result.opus.bVBR = TRUE;
        result.opus.bVBRConstraint = FALSE;
        result.opus.bDTX = FALSE;
        result.opus.bFEC = TRUE;
        result.opus.nBitRate = 32000;
        break;
    case NO_CODEC :
    case WEBM_VP8_CODEC :
        break;
    }
    return result;
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
