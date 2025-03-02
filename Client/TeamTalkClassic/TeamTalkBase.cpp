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

AudioPreprocessor InitDefaultAudioPreprocessor(AudioPreprocessorType preprocessortype)
{
    AudioPreprocessor preprocessor = {};
    preprocessor.nPreprocessor = preprocessortype;
    switch (preprocessor.nPreprocessor)
    {
    case NO_AUDIOPREPROCESSOR:
        break;
    case SPEEXDSP_AUDIOPREPROCESSOR:
        preprocessor.speexdsp.bEnableAGC = DEFAULT_SPEEXDSP_AGC_ENABLE;
        preprocessor.speexdsp.nGainLevel = DEFAULT_SPEEXDSP_AGC_GAINLEVEL;
        preprocessor.speexdsp.nMaxIncDBSec = DEFAULT_SPEEXDSP_AGC_INC_MAXDB;
        preprocessor.speexdsp.nMaxDecDBSec = DEFAULT_SPEEXDSP_AGC_DEC_MAXDB;
        preprocessor.speexdsp.nMaxGainDB = DEFAULT_SPEEXDSP_AGC_GAINMAXDB;
        preprocessor.speexdsp.bEnableDenoise = DEFAULT_SPEEXDSP_DENOISE_ENABLE;
        preprocessor.speexdsp.nMaxNoiseSuppressDB = DEFAULT_SPEEXDSP_DENOISE_SUPPRESS;
        preprocessor.speexdsp.bEnableAGC = DEFAULT_SPEEXDSP_ECHO_ENABLE;
        preprocessor.speexdsp.nEchoSuppress = DEFAULT_SPEEXDSP_ECHO_SUPPRESS;
        preprocessor.speexdsp.nEchoSuppressActive = DEFAULT_SPEEXDSP_ECHO_SUPPRESSACTIVE;
        break;
    case TEAMTALK_AUDIOPREPROCESSOR:
        preprocessor.ttpreprocessor.nGainLevel = SOUND_GAIN_DEFAULT;
        preprocessor.ttpreprocessor.bMuteLeftSpeaker = preprocessor.ttpreprocessor.bMuteRightSpeaker = FALSE;
        break;
    case WEBRTC_AUDIOPREPROCESSOR:
    case WEBRTC_AUDIOPREPROCESSOR_OBSOLETE_R4332 :
        break;
    }
    return preprocessor;
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
    for(ite = channels.begin(); ite != channels.end(); ++ite)
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
    for(ite=channels.begin();ite!=channels.end(); ++ite)
        ret = max(ite->first, ret);
    return ret;
}

users_t GetChannelUsers(const users_t& users, int nChannelID)
{
    users_t result;
    users_t::const_iterator ite;
    for(ite=users.begin();ite!=users.end();++ite)
    {
        if(ite->second.nChannelID == nChannelID || nChannelID == -1)
            result.insert(*ite);
    }
    return result;
}

transmitusers_t& GetTransmitUsers(const Channel& chan, transmitusers_t& transmitUsers)
{
    for(int i=0;i<TT_TRANSMITUSERS_MAX && chan.transmitUsers[i][TT_TRANSMITUSERS_USERID_INDEX];i++)
    {
        transmitUsers[chan.transmitUsers[i][TT_TRANSMITUSERS_USERID_INDEX]] = chan.transmitUsers[i][TT_TRANSMITUSERS_STREAMTYPE_INDEX];
    }
    return transmitUsers;
}

BOOL ToggleTransmitUser(Channel& chan, int nUserID, StreamTypes streams)
{
    int* begin = &chan.transmitUsers[0][TT_TRANSMITUSERS_USERID_INDEX];
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
            p[TT_TRANSMITUSERS_STREAMTYPE_INDEX] = streams;
        }
    }
    else
    {
        if(p[TT_TRANSMITUSERS_STREAMTYPE_INDEX] & streams)
            p[TT_TRANSMITUSERS_STREAMTYPE_INDEX] &= ~streams;
        else
            p[TT_TRANSMITUSERS_STREAMTYPE_INDEX] |= streams;
    }
    return TRUE;
}

BOOL CanToggleTransmitUsers(int nChannelID)
{
    BOOL bEnableChan = (TT_GetMyUserRights(ttInst) & USERRIGHT_MODIFY_CHANNELS);
    return TT_IsChannelOperator(ttInst, TT_GetMyUserID(ttInst), nChannelID) ||
          bEnableChan;
}

messages_t GetMessages(int nFromUserID, const messages_t& messages)
{
    messages_t result;
    for(size_t i=0;i<messages.size();i++)
    {
        if(messages[i].nFromUserID == nFromUserID)
            result.push_back(messages[i]);
    }
    return result;
}

int RefVolume(double percent)
{
    //82.832*EXP(0.0508*x) - 50 
    if(percent == 0)
        return 0;

    double d = 82.832 * exp(0.0508 * percent) - 50;
    return int(d);
}

int RefVolumeToPercent(int volume)
{
    if(volume == 0)
        return 0;

    double d = (volume + 50) / 82.832;
    d = log(d) / 0.0508;
    return int(d + .5);
}

int RefGain(double percent)
{
    if(percent == 0)
        return 0;

    return int(82.832 * exp(0.0508 * percent) - 50);
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

CString UserCacheID(const User& user)
{
    const CString szPostFix = _T(WEBLOGIN_BEARWARE_USERNAMEPOSTFIX);
    if (CString(user.szUsername).Right(szPostFix.GetLength()) == szPostFix)
        return CString(user.szUsername) + _T("|") + CString(user.szClientName);

    return CString();
}


UserCached::UserCached(const User& user)
{
    bValid = !UserCacheID(user).IsEmpty();
    if (!bValid)
        return;

    uSubscriptions = user.uLocalSubscriptions;
    bVoiceMute = (user.uUserState & USERSTATE_MUTE_VOICE) != USERSTATE_NONE;
    bMediaMute = (user.uUserState & USERSTATE_MUTE_MEDIAFILE) != USERSTATE_NONE;
    nVoiceVolume = user.nVolumeVoice;
    nMediaVolume = user.nVolumeMediaFile;
    bVoiceLeftSpeaker = user.stereoPlaybackVoice[0];
    bVoiceRightSpeaker = user.stereoPlaybackVoice[1];
    bMediaLeftSpeaker = user.stereoPlaybackMediaFile[0];
    bMediaRightSpeaker = user.stereoPlaybackMediaFile[1];

    TRACE(_T("Cached ") + UserCacheID(user) + _T("\n"));
}

void UserCached::Sync(TTInstance* ttInst, const User& user)
{
    if (!bValid)
        return;

    TT_SetUserMute(ttInst, user.nUserID, STREAMTYPE_VOICE, bVoiceMute);
    TT_SetUserMute(ttInst, user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO, bMediaMute);
    TT_SetUserVolume(ttInst, user.nUserID, STREAMTYPE_VOICE, nVoiceVolume);
    TT_SetUserVolume(ttInst, user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO, nMediaVolume);
    TT_SetUserStereo(ttInst, user.nUserID, STREAMTYPE_VOICE, bVoiceLeftSpeaker, bVoiceRightSpeaker);
    TT_SetUserStereo(ttInst, user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO, bMediaLeftSpeaker, bMediaRightSpeaker);
    if (uSubscriptions != user.uLocalSubscriptions)
    {
        TT_DoUnsubscribe(ttInst, user.nUserID, user.uLocalSubscriptions ^ uSubscriptions);
        TT_DoSubscribe(ttInst, user.nUserID, uSubscriptions);
    }
    TT_PumpMessage(ttInst, CLIENTEVENT_USER_STATECHANGE, user.nUserID);

    TRACE(_T("Restored ") + UserCacheID(user) + _T("\n"));
}
