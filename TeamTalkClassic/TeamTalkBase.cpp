/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#include "StdAfx.h"
#include "TeamTalkBase.h"
#include "resource.h"
#include <algorithm>
#include <assert.h>

extern TTInstance* ttInst;

void InitDefaultAudioCodec(AudioCodec& audiocodec)
{
    audiocodec.nCodec = DEFAULT_AUDIOCODEC;
    switch(DEFAULT_AUDIOCODEC)
    {
    case SPEEX_CODEC :
        audiocodec.speex.nQuality = DEFAULT_SPEEX_QUALITY;
        audiocodec.speex.nBandmode = DEFAULT_SPEEX_BANDMODE;
        audiocodec.speex.nTxIntervalMSec = DEFAULT_SPEEX_DELAY;
        audiocodec.speex.bStereoPlayback = DEFAULT_SPEEX_SIMSTEREO;
        break;
    case SPEEX_VBR_CODEC :
        audiocodec.speex_vbr.nQuality = DEFAULT_SPEEX_VBR_QUALITY;
        audiocodec.speex_vbr.nBandmode = DEFAULT_SPEEX_VBR_BANDMODE;
        audiocodec.speex_vbr.nTxIntervalMSec = DEFAULT_SPEEX_VBR_DELAY;
        audiocodec.speex_vbr.bStereoPlayback = DEFAULT_SPEEX_VBR_SIMSTEREO;
        audiocodec.speex_vbr.nBitRate = DEFAULT_SPEEX_VBR_BITRATE;
        audiocodec.speex_vbr.nMaxBitRate = DEFAULT_SPEEX_VBR_MAXBITRATE;
        audiocodec.speex_vbr.bDTX = DEFAULT_SPEEX_VBR_DTX;
        break;
    case OPUS_CODEC :
        audiocodec.opus.nApplication = DEFAULT_OPUS_APPLICATION;
        audiocodec.opus.nSampleRate = DEFAULT_OPUS_SAMPLERATE;
        audiocodec.opus.nChannels = DEFAULT_OPUS_CHANNELS;
        audiocodec.opus.nTxIntervalMSec = DEFAULT_OPUS_DELAY;
        audiocodec.opus.nComplexity = DEFAULT_OPUS_COMPLEXITY;
        audiocodec.opus.bVBR = DEFAULT_OPUS_VBR;
        audiocodec.opus.bVBRConstraint = DEFAULT_OPUS_VBRCONSTRAINT;
        audiocodec.opus.bDTX = DEFAULT_OPUS_DTX;
        audiocodec.opus.bFEC = DEFAULT_OPUS_FEC;
        audiocodec.opus.nBitRate = DEFAULT_OPUS_BITRATE;
        break;
    default :
        audiocodec.nCodec = NO_CODEC;
        break;
    }
}

channels_t GetSubChannels(int nChannelID, const channels_t& channels, BOOL bRecursive/* = FALSE*/)
{
    channels_t subchannels;
    channels_t::const_iterator ite;
    for(ite = channels.begin(); ite != channels.end(); ite++)
    {
        if(ite->second.nParentID == nChannelID)
        {
            subchannels[ite->first] = ite->second;
            if(bRecursive)
            {
                channels_t subs;
                subs = GetSubChannels(ite->first, channels, bRecursive);
                subchannels.insert(subs.begin(), subs.end());
            }
        }
    }
    return subchannels;
}

channels_t GetParentChannels(int nChannelID, const channels_t& channels)
{
    channels_t parents;
    auto i=channels.find(nChannelID);
    while(i != channels.end() && i->second.nParentID>0)
    {
        parents[i->second.nParentID] = channels.find(i->second.nParentID)->second;
        i = channels.find(i->second.nParentID);
    }
    return parents;
}

int GetRootChannelID(const channels_t& channels)
{
    channels_t::const_iterator ite;
    for(ite = channels.begin(); ite != channels.end(); ite++)
    {
        if(ite->second.nParentID == 0)
            return ite->second.nChannelID;
    }
    return 0;
}

int GetMaxChannelID(const channels_t& channels)
{
    int ret = 0;
    channels_t::const_iterator ite;
    for(ite=channels.begin();ite!=channels.end();ite++)
        ret = max(ite->first, ret);
    return ret;
}

users_t GetChannelUsers(const users_t& users, int nChannelID)
{
    users_t result;
    users_t::const_iterator ite;
    for(ite=users.begin();ite!=users.end();ite++)
    {
        if(ite->second.nChannelID == nChannelID || nChannelID == -1)
            result.insert(*ite);
    }
    return result;
}

transmitusers_t& GetTransmitUsers(const Channel& chan, transmitusers_t& transmitUsers)
{
    for(int i=0;i<TT_TRANSMITUSERS_MAX && chan.transmitUsers[i][0];i++)
    {
        transmitUsers[chan.transmitUsers[i][0]] = chan.transmitUsers[i][1];
    }
    return transmitUsers;
}

BOOL ToggleTransmitUser(Channel& chan, int nUserID, StreamTypes streams)
{
    int* begin = &chan.transmitUsers[0][0];
    int* end = &chan.transmitUsers[0][TT_TRANSMITUSERS_MAX];
    int* p = std::find(begin, end, nUserID);
    if(p == end)
    {
        p = std::find(begin, end, 0);
        if(p == end)
        {
            return FALSE;
        }
        else
        {
            *p = nUserID;
            p[TT_TRANSMITSTREAMTYPE_INDEX] = streams;
        }
    }
    else
    {
        if(p[TT_TRANSMITSTREAMTYPE_INDEX] & streams)
            p[TT_TRANSMITSTREAMTYPE_INDEX] &= ~streams;
        else
            p[TT_TRANSMITSTREAMTYPE_INDEX] |= streams;
    }
    return TRUE;
}

BOOL CanToggleTransmitUsers(int nChannelID)
{
    bool bEnableChan = (TT_GetMyUserRights(ttInst) & USERRIGHT_MODIFY_CHANNELS);
    return TT_IsChannelOperator(ttInst, TT_GetMyUserID(ttInst), nChannelID) ||
          bEnableChan;
}

messages_t GetMessages(int nFromUserID, const messages_t& messages)
{
    messages_t result;
    for(int i=0;i<messages.size();i++)
    {
        if(messages[i].nFromUserID == nFromUserID)
            result.push_back(messages[i]);
    }
    return result;
}

BOOL GetSoundDevice(int nSoundDeviceID, const CString& szDeviceID, SoundDevice& dev)
{
    int count = 25;
    std::vector<SoundDevice> devices(count);
    TT_GetSoundDevices(&devices[0], &count);
    if(count == 25)
    {
        TT_GetSoundDevices(NULL, &count);
        devices.resize(count);
        TT_GetSoundDevices(&devices[0], &count);
    }
    devices.resize(count);
    size_t i;
    for(i=0;i<devices.size() && szDeviceID.GetLength();i++)
    {
        if(devices[i].szDeviceID == szDeviceID)
        {
            dev = devices[i];
            return true;
        }
    }
    for(i=0;i<devices.size();i++)
    {
        if(devices[i].nDeviceID == nSoundDeviceID)
        {
            dev = devices[i];
            return true;
        }
    }
    return false;
}

int RefVolume(double percent)
{
    //82.832*EXP(0.0508*x) - 50 
    if(percent == 0)
        return 0;

    double d = 82.832 * exp(0.0508 * percent) - 50;
    return d;
}

int RefVolumeToPercent(int volume)
{
    if(volume == 0)
        return 0;

    double d = (volume + 50) / 82.832;
    d = log(d) / 0.0508;
    return d + .5;
}

int RefGain(double percent)
{
    if(percent == 0)
        return 0;

    return 82.832 * std::exp(0.0508 * percent) - 50;
}

CString GetVersion(const User& user)
{
    CString szVersion;
    szVersion.Format(_T("%d.%d.%d"), user.uVersion>>16, 
                     ((user.uVersion >> 8) & 0xFF),
                     (user.uVersion) & 0xFF);
    return szVersion;
}

BOOL IsMyselfTalking()
{
    ClientFlags uFlags = TT_GetFlags(ttInst);
    return (uFlags & CLIENT_TX_VOICE) ||
        ((uFlags & CLIENT_SNDINPUT_VOICEACTIVATED) &&
         (uFlags & CLIENT_SNDINPUT_VOICEACTIVE));
}

CString MakeCustomCommand(LPCTSTR szCmd, LPCTSTR szValue)
{
    CString s;
    s.Format(_T("%s\r\n%s"), szCmd, szValue);
    return s;
}

void GetCustomCommand(const CString& szMessage, CStringList& result)
{
    int i = 0;
    while(i >= 0 && i < szMessage.GetLength())
        result.AddTail(szMessage.Tokenize(_T("\r\n"), i));
}

BOOL IsValid(const VideoFormat& capfmt)
{
    return capfmt.nWidth>0 && capfmt.nHeight>0 && capfmt.nFPS_Numerator>0 &&
        capfmt.nFPS_Denominator > 0 && capfmt.picFourCC != FOURCC_NONE;
}
