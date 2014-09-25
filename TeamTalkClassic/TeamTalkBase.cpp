/*
 * Copyright (c) 2005-2014, BearWare.dk
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

#include "StdAfx.h"
#include "TeamTalkBase.h"
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
        audiocodec.speex.nMSecPerPacket = DEFAULT_SPEEX_DELAY;
        audiocodec.speex.bStereoPlayback = DEFAULT_SPEEX_SIMSTEREO;
        break;
    case SPEEX_VBR_CODEC :
        audiocodec.speex_vbr.nQuality = DEFAULT_SPEEX_VBR_QUALITY;
        audiocodec.speex_vbr.nBandmode = DEFAULT_SPEEX_VBR_BANDMODE;
        audiocodec.speex_vbr.nMSecPerPacket = DEFAULT_SPEEX_VBR_DELAY;
        audiocodec.speex_vbr.bStereoPlayback = DEFAULT_SPEEX_VBR_SIMSTEREO;
        audiocodec.speex_vbr.nBitRate = DEFAULT_SPEEX_VBR_BITRATE;
        audiocodec.speex_vbr.nMaxBitRate = DEFAULT_SPEEX_VBR_MAXBITRATE;
        audiocodec.speex_vbr.bDTX = DEFAULT_SPEEX_VBR_DTX;
        break;
    case OPUS_CODEC :
        audiocodec.opus.nApplication = DEFAULT_OPUS_APPLICATION;
        audiocodec.opus.nSampleRate = DEFAULT_OPUS_SAMPLERATE;
        audiocodec.opus.nChannels = DEFAULT_OPUS_CHANNELS;
        audiocodec.opus.nMSecPerPacket = DEFAULT_OPUS_DELAY;
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

channels_t GetSubChannels(int nChannelID, const channels_t& channels)
{
  channels_t subchannels;
  channels_t::const_iterator ite;
  for(ite = channels.begin(); ite != channels.end(); ite++)
  {
    if(ite->second.nParentID == nChannelID)
      subchannels[ite->first] = ite->second;
  }
  return subchannels;
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

users_t GetChannelUsers(int nChannelID, const users_t& users)
{
  users_t result;
  users_t::const_iterator ite;
  for(ite=users.begin();ite!=users.end();ite++)
  {
    if(ite->second.nChannelID == nChannelID)
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
            return FALSE;
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
    return TT_IsChannelOperator(ttInst, TT_GetMyUserID(ttInst), nChannelID) ||
           (TT_GetMyUserType(ttInst) & USERTYPE_ADMIN);
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

BOOL GetSoundDevice(int nSoundDeviceID, SoundDevice& dev)
{
    int count = 25;
    std::vector<SoundDevice> devices(count);
    TT_GetSoundDevices(&devices[0], &count);
    if(count == 25)
    {
        devices.resize(count);
        TT_GetSoundDevices(&devices[0], &count);
    }
    size_t i;
    for(i=0;i<devices.size();i++)
    {
        if(devices[i].nDeviceID == nSoundDeviceID)
        {
            dev = devices[i];
            break;
        }
    }
    return i < devices.size();
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
