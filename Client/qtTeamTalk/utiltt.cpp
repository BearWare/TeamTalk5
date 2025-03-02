/*
 * Copyright (C) 2023, Bjørn D. Rasmussen, BearWare.dk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "utiltt.h"

extern TTInstance* ttInst;

bool userCanTx(int userid, StreamTypes stream_type, const Channel& chan)
{
    int i=0;
    while(i<TT_TRANSMITUSERS_MAX && chan.transmitUsers[i][TT_TRANSMITUSERS_USERID_INDEX])
    {
        if(chan.transmitUsers[i][TT_TRANSMITUSERS_USERID_INDEX] == userid && (chan.transmitUsers[i][TT_TRANSMITUSERS_STREAMTYPE_INDEX] & stream_type))
            return (chan.uChannelType & CHANNEL_CLASSROOM) == CHANNEL_CLASSROOM;
        else i++;
    }
    return (chan.uChannelType & CHANNEL_CLASSROOM) == CHANNEL_DEFAULT;
}

bool userCanChanMessage(int userid, const Channel& chan, bool includeFreeForAll /*= false*/)
{
    return userCanTx(userid, STREAMTYPE_CHANNELMSG, chan) || (includeFreeForAll && userCanTx(TT_TRANSMITUSERS_FREEFORALL, STREAMTYPE_CHANNELMSG, chan));
}

bool userCanVoiceTx(int userid, const Channel& chan, bool includeFreeForAll /*= false*/)
{
    return userCanTx(userid, STREAMTYPE_VOICE, chan) || (includeFreeForAll && userCanTx(TT_TRANSMITUSERS_FREEFORALL, STREAMTYPE_VOICE, chan));
}

bool userCanVideoTx(int userid, const Channel& chan, bool includeFreeForAll /*= false*/)
{
    return userCanTx(userid, STREAMTYPE_VIDEOCAPTURE, chan) || (includeFreeForAll && userCanTx(TT_TRANSMITUSERS_FREEFORALL, STREAMTYPE_VIDEOCAPTURE, chan));
}

bool userCanDesktopTx(int userid, const Channel& chan, bool includeFreeForAll /*= false*/)
{
    return userCanTx(userid, STREAMTYPE_DESKTOP, chan) || (includeFreeForAll && userCanTx(TT_TRANSMITUSERS_FREEFORALL, STREAMTYPE_DESKTOP, chan));
}

bool userCanMediaFileTx(int userid, const Channel& chan, bool includeFreeForAll /*= false*/)
{
    return userCanTx(userid, STREAMTYPE_MEDIAFILE, chan) || (includeFreeForAll && userCanTx(TT_TRANSMITUSERS_FREEFORALL, STREAMTYPE_MEDIAFILE, chan));
}

channels_t getSubChannels(int channelid, const channels_t& channels, bool recursive /*= false*/)
{
    channels_t subchannels;
    for(auto ite = channels.begin(); ite != channels.end(); ++ite)
    {
        if(ite.value().nParentID == channelid)
        {
            subchannels[ite.key()] = ite.value();
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
            if(recursive)
                subchannels.unite(getSubChannels(ite.value().nChannelID, channels, recursive));
#else
            subchannels.insert(getSubChannels(ite.value().nChannelID, channels, recursive));
#endif
        }
    }
    return subchannels;
}

channels_t getParentChannels(int channelid, const channels_t& channels)
{
    channels_t parents;
    while(channels[channelid].nParentID>0)
    {
        parents[channels[channelid].nParentID] = channels[channelid];
        channelid = channels[channelid].nParentID;
    }
    return parents;
}

users_t getChannelUsers(int channelid, const users_t& users, const channels_t& channels, bool recursive /* = false */)
{
    users_t result;
    for(auto ite=users.begin();ite!=users.end();ite++)
    {
        if(ite.value().nChannelID == channelid)
            result.insert(ite.key(), ite.value());
    }

    if (recursive)
    {
        channels_t subs = getSubChannels(channelid, channels, true);
        for(auto i=subs.begin();i!=subs.end();++i)
        {
            for (auto& u : getChannelUsers(i.key(), users, channels))
                result.insert(u.nUserID, u);
        }
    }
    return result;
}

bool isFreeForAll(StreamTypes stream_type, const int transmitUsers[][2],
                  int max_userids /*= TT_TRANSMITUSERS_MAX*/)
{
    int i=0;
    while(i<max_userids && transmitUsers[i][TT_TRANSMITUSERS_USERID_INDEX] != 0)
    {
        if(transmitUsers[i][TT_TRANSMITUSERS_USERID_INDEX] == TT_CLASSROOM_FREEFORALL &&
            (transmitUsers[i][TT_TRANSMITUSERS_STREAMTYPE_INDEX] & stream_type))
            return true;
        i++;
    }
    return false;
}

void setTransmitUsers(const QSet<int>& users, INT32* dest_array,
                      INT32 max_elements)
{
    QSet<int>::const_iterator ite = users.begin();
    for(int i=0;i<max_elements;i++)
    {
        if(ite != users.end())
        {
            dest_array[i] = *ite;
            ite++;
        }
        else
            dest_array[i] = 0;
    }
}

void initDefaultAudioCodec(AudioCodec& audiocodec)
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

AudioPreprocessor initDefaultAudioPreprocessor(AudioPreprocessorType preprocessortype)
{
    AudioPreprocessor preprocessor = {};
    preprocessor.nPreprocessor = preprocessortype;
    switch (preprocessor.nPreprocessor)
    {
    case NO_AUDIOPREPROCESSOR :
        break;
    case SPEEXDSP_AUDIOPREPROCESSOR :
        preprocessor.speexdsp.bEnableAGC = DEFAULT_SPEEXDSP_AGC_ENABLE;
        preprocessor.speexdsp.nGainLevel = DEFAULT_SPEEXDSP_AGC_GAINLEVEL;
        preprocessor.speexdsp.nMaxIncDBSec = DEFAULT_SPEEXDSP_AGC_INC_MAXDB;
        preprocessor.speexdsp.nMaxDecDBSec = DEFAULT_SPEEXDSP_AGC_DEC_MAXDB;
        preprocessor.speexdsp.nMaxGainDB = DEFAULT_SPEEXDSP_AGC_GAINMAXDB;
        preprocessor.speexdsp.bEnableDenoise = DEFAULT_SPEEXDSP_DENOISE_ENABLE;
        preprocessor.speexdsp.nMaxNoiseSuppressDB = DEFAULT_SPEEXDSP_DENOISE_SUPPRESS;
        preprocessor.speexdsp.bEnableEchoCancellation = DEFAULT_SPEEXDSP_ECHO_ENABLE;
        preprocessor.speexdsp.nEchoSuppress = DEFAULT_SPEEXDSP_ECHO_SUPPRESS;
        preprocessor.speexdsp.nEchoSuppressActive = DEFAULT_SPEEXDSP_ECHO_SUPPRESSACTIVE;
        break;
    case TEAMTALK_AUDIOPREPROCESSOR :
        preprocessor.ttpreprocessor.nGainLevel = DEFAULT_TEAMTALK_GAINLEVEL;
        preprocessor.ttpreprocessor.bMuteLeftSpeaker = DEFAULT_TEAMTALK_MUTELEFT;
        preprocessor.ttpreprocessor.bMuteRightSpeaker = DEFAULT_TEAMTALK_MUTERIGHT;
        break;
    case WEBRTC_AUDIOPREPROCESSOR_OBSOLETE_R4332 :
        break;
    case WEBRTC_AUDIOPREPROCESSOR :
        preprocessor.webrtc.preamplifier.bEnable = DEFAULT_WEBRTC_PREAMPLIFIER_ENABLE;
        preprocessor.webrtc.preamplifier.fFixedGainFactor = DEFAULT_WEBRTC_PREAMPLIFIER_GAINFACTOR;
        preprocessor.webrtc.gaincontroller2.bEnable = DEFAULT_WEBRTC_GAINCTL_ENABLE;
        preprocessor.webrtc.gaincontroller2.fixeddigital.fGainDB = DEFAULT_WEBRTC_GAINDB;
        preprocessor.webrtc.gaincontroller2.adaptivedigital.bEnable = DEFAULT_WEBRTC_SAT_PROT_ENABLE;
        preprocessor.webrtc.gaincontroller2.adaptivedigital.fHeadRoomDB = DEFAULT_WEBRTC_HEADROOM_DB;
        preprocessor.webrtc.gaincontroller2.adaptivedigital.fInitialGainDB = DEFAULT_WEBRTC_INITIAL_GAIN_DB;
        preprocessor.webrtc.gaincontroller2.adaptivedigital.fMaxGainDB = DEFAULT_WEBRTC_MAXGAIN_DB;
        preprocessor.webrtc.gaincontroller2.adaptivedigital.fMaxGainChangeDBPerSecond = DEFAULT_WEBRTC_MAXGAIN_DBSEC;
        preprocessor.webrtc.gaincontroller2.adaptivedigital.fMaxOutputNoiseLevelDBFS = DEFAULT_WEBRTC_MAX_OUT_NOISE;
        preprocessor.webrtc.noisesuppression.bEnable = DEFAULT_WEBRTC_NOISESUPPRESS_ENABLE;
        preprocessor.webrtc.noisesuppression.nLevel = DEFAULT_WEBRTC_NOISESUPPRESS_LEVEL;
        preprocessor.webrtc.echocanceller.bEnable = DEFAULT_WEBRTC_ECHO_CANCEL_ENABLE;
        break;
    }
    return preprocessor;
}

bool isMyselfTalking()
{
    bool talking = false;
    ClientFlags flags = TT_GetFlags(ttInst);
    talking = (flags & CLIENT_TX_VOICE) ||
              ((flags & CLIENT_SNDINPUT_VOICEACTIVATED) &&
               (flags & CLIENT_SNDINPUT_VOICEACTIVE));
    return talking;
}

bool isMyselfStreaming()
{
    return (TT_GetFlags(ttInst) & (CLIENT_STREAM_AUDIO | CLIENT_STREAM_VIDEO)) != CLIENT_CLOSED;
}

