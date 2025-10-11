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

#include "TTUnitTest.h"

#include "mystd/MyStd.h"

std::string g_server_ipaddr = "127.0.0.1";
const bool GITHUBSKIP = ACE_OS::getenv(ACE_TEXT("GITHUBSKIP")) ? ACE_OS::atoi(ACE_OS::getenv(ACE_TEXT("GITHUBSKIP"))) == 1 : false;

TTInstPtr InitTeamTalk()
{
    TTInstPtr inst(TT_InitTeamTalkPoll());
    return inst;
}

bool InitSound(TTInstance* ttClient, SoundMode mode /*= DEFAULT*/, INT32 indev, INT32 outdev)
{
    int selindev = indev, seloutdev = outdev;
    int orgindev = indev, orgoutdev = outdev;
    if (indev == SOUNDDEVICEID_DEFAULT || outdev == SOUNDDEVICEID_DEFAULT)
    {
        SoundDevice sndindev, sndoutdev;
        if (!GetSoundDevices(sndindev, sndoutdev, indev, outdev))
            return false;
        indev = sndindev.nDeviceID;
        outdev = sndoutdev.nDeviceID;
    }

    if (selindev == SOUNDDEVICEID_DEFAULT)
    {
        if (indev == SOUNDDEVICEID_IGNORE)
            return false;
        selindev = indev;
    }
    if (seloutdev == SOUNDDEVICEID_DEFAULT)
    {
        if (outdev == SOUNDDEVICEID_IGNORE)
            return false;
        seloutdev = outdev;
    }

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
    case SHARED_OUTPUT :
    {
        if (seloutdev == SOUNDDEVICEID_IGNORE)
            return false;

        seloutdev |= TT_SOUNDDEVICE_ID_SHARED_FLAG;
        break;
    }
    case SHARED_INPUT_OUTPUT :
    {
        if (selindev == SOUNDDEVICEID_IGNORE)
            return false;
        if (seloutdev == SOUNDDEVICEID_IGNORE)
            return false;

        selindev |= TT_SOUNDDEVICE_ID_SHARED_FLAG;
        seloutdev |= TT_SOUNDDEVICE_ID_SHARED_FLAG;
        break;
    }
    case DEFAULT :
#if defined(__ANDROID__)
        // Android only supports a single recorder and a limited
        // number of players, so we by default switch to shared
        // mode on Android.
        if (orgindev == SOUNDDEVICEID_DEFAULT)
            selindev |= TT_SOUNDDEVICE_ID_SHARED_FLAG;
        if (orgoutdev == SOUNDDEVICEID_DEFAULT)
            seloutdev |= TT_SOUNDDEVICE_ID_SHARED_FLAG;
#endif
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

    insnddev.nDeviceID = SOUNDDEVICEID_IGNORE;
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

    outsnddev.nDeviceID = SOUNDDEVICEID_IGNORE;
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

bool Connect(TTInstance* ttClient, INT32 tcpport, INT32 udpport, TTBOOL encrypted)
{
#if defined(WIN32)
    std::wstring wserver_ip = std::wstring(g_server_ipaddr.begin(), g_server_ipaddr.end());
    const TTCHAR* hostname = reinterpret_cast<const TTCHAR*>(wserver_ip.c_str());
#else
    const TTCHAR* hostname = reinterpret_cast<const TTCHAR*>(g_server_ipaddr.c_str());
#endif
    return Connect(ttClient, hostname, udpport, tcpport, encrypted);
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
