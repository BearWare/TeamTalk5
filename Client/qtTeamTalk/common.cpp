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

#include "common.h"
#include "settings.h"
#include "appinfo.h"

#include <QDialog>
#include <QStack>
#include <QDebug>

#define DEFAULT_NICKNAME           QT_TRANSLATE_NOOP("MainWindow", "NoName")

extern QSettings* ttSettings;
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

QString makeCustomCommand(const QString& cmd, const QString& value)
{
    return QString("%1\r\n%2").arg(cmd).arg(value);
}

QStringList getCustomCommand(const TextMessage& msg)
{
    return _Q(msg.szMessage).split("\r\n");
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
    case WEBRTC_AUDIOPREPROCESSOR :
        preprocessor.webrtc.preamplifier.bEnable = DEFAULT_WEBRTC_PREAMPLIFIER_ENABLE;
        preprocessor.webrtc.preamplifier.fFixedGainFactor = DEFAULT_WEBRTC_PREAMPLIFIER_GAINFACTOR;
        preprocessor.webrtc.levelestimation.bEnable = DEFAULT_WEBRTC_LEVELESTIMATION_ENABLE;
        preprocessor.webrtc.voicedetection.bEnable = DEFAULT_WEBRTC_VAD_ENABLE;
        preprocessor.webrtc.gaincontroller2.bEnable = DEFAULT_WEBRTC_GAINCTL_ENABLE;
        preprocessor.webrtc.gaincontroller2.fixeddigital.fGainDB = DEFAULT_WEBRTC_GAINDB;
        preprocessor.webrtc.gaincontroller2.adaptivedigital.bEnable = DEFAULT_WEBRTC_SAT_PROT_ENABLE;
        preprocessor.webrtc.gaincontroller2.adaptivedigital.fInitialSaturationMarginDB = DEFAULT_WEBRTC_INIT_SAT_MARGIN_DB;
        preprocessor.webrtc.gaincontroller2.adaptivedigital.fExtraSaturationMarginDB = DEFAULT_WEBRTC_EXTRA_SAT_MARGIN_DB;
        preprocessor.webrtc.gaincontroller2.adaptivedigital.fMaxGainChangeDBPerSecond = DEFAULT_WEBRTC_MAXGAIN_DBSEC;
        preprocessor.webrtc.gaincontroller2.adaptivedigital.fMaxOutputNoiseLevelDBFS = DEFAULT_WEBRTC_MAX_OUT_NOISE;
        preprocessor.webrtc.noisesuppression.bEnable = DEFAULT_WEBRTC_NOISESUPPRESS_ENABLE;
        preprocessor.webrtc.noisesuppression.nLevel = DEFAULT_WEBRTC_NOISESUPPRESS_LEVEL;
        preprocessor.webrtc.echocanceller.bEnable = DEFAULT_WEBRTC_ECHO_CANCEL_ENABLE;
        break;
    }
    return preprocessor;
}

AudioPreprocessor loadAudioPreprocessor(AudioPreprocessorType preprocessortype)
{
    AudioPreprocessor preprocessor = initDefaultAudioPreprocessor(preprocessortype);
    switch (preprocessor.nPreprocessor)
    {
    case NO_AUDIOPREPROCESSOR:
        break;
    case SPEEXDSP_AUDIOPREPROCESSOR:
        preprocessor.speexdsp.bEnableAGC = ttSettings->value(SETTINGS_STREAMMEDIA_SPX_AGC_ENABLE, DEFAULT_SPEEXDSP_AGC_ENABLE).toBool();
        preprocessor.speexdsp.nGainLevel = ttSettings->value(SETTINGS_STREAMMEDIA_SPX_AGC_GAINLEVEL, DEFAULT_SPEEXDSP_AGC_GAINLEVEL).toInt();
        preprocessor.speexdsp.nMaxIncDBSec = ttSettings->value(SETTINGS_STREAMMEDIA_SPX_AGC_INC_MAXDB, DEFAULT_SPEEXDSP_AGC_INC_MAXDB).toInt();
        preprocessor.speexdsp.nMaxDecDBSec = ttSettings->value(SETTINGS_STREAMMEDIA_SPX_AGC_DEC_MAXDB, DEFAULT_SPEEXDSP_AGC_DEC_MAXDB).toInt();
        preprocessor.speexdsp.nMaxGainDB = ttSettings->value(SETTINGS_STREAMMEDIA_SPX_AGC_GAINMAXDB, DEFAULT_SPEEXDSP_AGC_GAINMAXDB).toInt();
        preprocessor.speexdsp.bEnableDenoise = ttSettings->value(SETTINGS_STREAMMEDIA_SPX_DENOISE_ENABLE, DEFAULT_SPEEXDSP_DENOISE_ENABLE).toBool();
        preprocessor.speexdsp.nMaxNoiseSuppressDB = ttSettings->value(SETTINGS_STREAMMEDIA_SPX_DENOISE_SUPPRESS, DEFAULT_SPEEXDSP_DENOISE_SUPPRESS).toInt();
        preprocessor.speexdsp.bEnableEchoCancellation = FALSE; // unusable for streaming
        preprocessor.speexdsp.nEchoSuppress = DEFAULT_SPEEXDSP_ECHO_SUPPRESS;
        preprocessor.speexdsp.nEchoSuppressActive = DEFAULT_SPEEXDSP_ECHO_SUPPRESSACTIVE;
        break;
    case TEAMTALK_AUDIOPREPROCESSOR:
        preprocessor.ttpreprocessor.bMuteLeftSpeaker = ttSettings->value(SETTINGS_STREAMMEDIA_TTAP_MUTELEFT, false).toBool();
        preprocessor.ttpreprocessor.bMuteRightSpeaker = ttSettings->value(SETTINGS_STREAMMEDIA_TTAP_MUTERIGHT, false).toBool();
        preprocessor.ttpreprocessor.nGainLevel = ttSettings->value(SETTINGS_STREAMMEDIA_TTAP_GAINLEVEL, SOUND_GAIN_DEFAULT).toInt();
        break;
    case WEBRTC_AUDIOPREPROCESSOR :
        preprocessor.webrtc.gaincontroller2.bEnable = ttSettings->value(SETTINGS_STREAMMEDIA_WEBRTC_GAINCTL_ENABLE, DEFAULT_WEBRTC_GAINCTL_ENABLE).toBool();
        preprocessor.webrtc.gaincontroller2.fixeddigital.fGainDB = ttSettings->value(SETTINGS_STREAMMEDIA_WEBRTC_GAINDB, DEFAULT_WEBRTC_GAINDB).toFloat();
        preprocessor.webrtc.gaincontroller2.adaptivedigital.bEnable = ttSettings->value(SETTINGS_STREAMMEDIA_WEBRTC_SAT_PROT_ENABLE, DEFAULT_WEBRTC_SAT_PROT_ENABLE).toBool();
        preprocessor.webrtc.gaincontroller2.adaptivedigital.fInitialSaturationMarginDB = ttSettings->value(SETTINGS_STREAMMEDIA_WEBRTC_INIT_SAT_MARGIN_DB, DEFAULT_WEBRTC_INIT_SAT_MARGIN_DB).toFloat();
        preprocessor.webrtc.gaincontroller2.adaptivedigital.fExtraSaturationMarginDB = ttSettings->value(SETTINGS_STREAMMEDIA_WEBRTC_EXTRA_SAT_MARGIN_DB, DEFAULT_WEBRTC_EXTRA_SAT_MARGIN_DB).toFloat();
        preprocessor.webrtc.gaincontroller2.adaptivedigital.fMaxGainChangeDBPerSecond = ttSettings->value(SETTINGS_STREAMMEDIA_WEBRTC_MAXGAIN_DBSEC, DEFAULT_WEBRTC_MAXGAIN_DBSEC).toFloat();
        preprocessor.webrtc.gaincontroller2.adaptivedigital.fMaxOutputNoiseLevelDBFS = ttSettings->value(SETTINGS_STREAMMEDIA_WEBRTC_MAX_OUT_NOISE, DEFAULT_WEBRTC_MAX_OUT_NOISE).toFloat();
        preprocessor.webrtc.noisesuppression.bEnable = ttSettings->value(SETTINGS_STREAMMEDIA_WEBRTC_NOISESUPPRESS_ENABLE, DEFAULT_WEBRTC_NOISESUPPRESS_ENABLE).toBool();
        preprocessor.webrtc.noisesuppression.nLevel = ttSettings->value(SETTINGS_STREAMMEDIA_WEBRTC_NOISESUPPRESS_LEVEL, DEFAULT_WEBRTC_NOISESUPPRESS_LEVEL).toFloat();
        preprocessor.webrtc.echocanceller.bEnable = FALSE; // unusable for streaming
        break;
    }
    return preprocessor;
}

#if defined(Q_OS_WIN32)
bool isComputerIdle(int idle_secs)
{
    LASTINPUTINFO info;
    info.cbSize = sizeof(LASTINPUTINFO);
    if( GetLastInputInfo(&info))
         return (::GetTickCount() - info.dwTime) / 1000 >= (UINT)idle_secs;
    else
        return false;
}
#elif defined(Q_OS_DARWIN)
#include <IOKit/IOKitLib.h>
bool isComputerIdle(int idle_secs)
{
    int64_t os_idle_secs = 0;
    io_iterator_t iter = 0;
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, 
                                     IOServiceMatching("IOHIDSystem"), 
                                     &iter) == KERN_SUCCESS)
    {
        io_registry_entry_t entry = IOIteratorNext(iter);
        if (entry)
        {
            CFMutableDictionaryRef dict = nullptr;
            if (IORegistryEntryCreateCFProperties(entry, &dict, 
                                                  kCFAllocatorDefault, 
                                                  0) == KERN_SUCCESS)
            {
                CFNumberRef obj = static_cast<CFNumberRef>
                (CFDictionaryGetValue(dict, 
                                      CFSTR("HIDIdleTime")));
                if (obj)
                {
                    int64_t nanoseconds = 0;
                    if (CFNumberGetValue(obj, kCFNumberSInt64Type, 
                                         &nanoseconds))
                    {
// Divide by 10^9 to convert from nanoseconds to seconds.
                        os_idle_secs = (nanoseconds >> 30); 
                    }
                }
            }
            IOObjectRelease(entry);
        }
        IOObjectRelease(iter);
    }
    return (int)os_idle_secs > idle_secs;
}
#else
bool isComputerIdle(int /*idle_secs*/)
{
    return false;
}
#endif

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

bool HostEntry::sameHost(const HostEntry& host, bool nickcheck) const
{
    return ipaddr == host.ipaddr &&
           tcpport == host.tcpport &&
           udpport == host.udpport &&
           encrypted == host.encrypted &&
           (username == host.username || (isWebLogin(host.username, true) && isWebLogin(username, false))) &&
           /* password == host.password && */
           (!nickcheck || nickname == host.nickname) &&
           channel == host.channel
           /* && hosts[i].chanpasswd == host.chanpasswd*/;
}

bool HostEntry::sameHostEntry(const HostEntry& host) const
{
    return sameHost(host, false) && host.name == name;
}

void addLatestHost(const HostEntry& host)
{
    QList<HostEntry> hosts;
    HostEntry tmp;
    int index = 0;
    while(getLatestHost(index, tmp))
    {
        hosts.push_back(tmp);
        tmp = HostEntry();
        index++;
    }
    for(int i=0;i<hosts.size();)
    {
        if (hosts[i].sameHostEntry(host))
        {
            hosts.erase(hosts.begin()+i);
        }
        else i++;
    }
    hosts.insert(0, host);
    while(hosts.size()>5)hosts.removeLast();

    for(int i=0;i<hosts.size();i++)
    {
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_HOSTADDR).arg(i), hosts[i].ipaddr);
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_TCPPORT).arg(i), hosts[i].tcpport);
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_UDPPORT).arg(i), hosts[i].udpport);
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_ENCRYPTED).arg(i), hosts[i].encrypted);
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_USERNAME).arg(i), hosts[i].username); 
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_PASSWORD).arg(i), hosts[i].password); 
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_NICKNAME).arg(i), hosts[i].nickname); 
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_CHANNEL).arg(i), hosts[i].channel); 
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_CHANNELPASSWD).arg(i), hosts[i].chanpasswd); 
    }
}

void deleteLatestHost(int index)
{
    QStack<HostEntry> hosts;
    HostEntry tmp;
    int i = 0;
    while(getLatestHost(i, tmp))
    {
        hosts.push(tmp);
        i++;
        tmp = HostEntry();
    }
    
    for(i=0;i<hosts.size();i++)
    {
        ttSettings->remove(QString(SETTINGS_LATESTHOST_HOSTADDR).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_TCPPORT).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_UDPPORT).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_ENCRYPTED).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_USERNAME).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_PASSWORD).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_NICKNAME).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_CHANNEL).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_CHANNELPASSWD).arg(i));
    }

    i=0;
    while(!hosts.isEmpty())
    {
        if(i != index)
            addLatestHost(hosts.pop());
        else
            hosts.pop();
        i++;
    }
}

bool getLatestHost(int index, HostEntry& host)
{
    host.ipaddr = ttSettings->value(QString(SETTINGS_LATESTHOST_HOSTADDR).arg(index)).toString();
    host.tcpport = ttSettings->value(QString(SETTINGS_LATESTHOST_TCPPORT).arg(index)).toInt();
    host.udpport = ttSettings->value(QString(SETTINGS_LATESTHOST_UDPPORT).arg(index)).toInt();
    host.encrypted = ttSettings->value(QString(SETTINGS_LATESTHOST_ENCRYPTED).arg(index), false).toBool();
    host.username = ttSettings->value(QString(SETTINGS_LATESTHOST_USERNAME).arg(index)).toString();
    host.password = ttSettings->value(QString(SETTINGS_LATESTHOST_PASSWORD).arg(index)).toString();
    host.nickname = ttSettings->value(QString(SETTINGS_LATESTHOST_NICKNAME).arg(index)).toString();
    host.channel = ttSettings->value(QString(SETTINGS_LATESTHOST_CHANNEL).arg(index)).toString();
    host.chanpasswd = ttSettings->value(QString(SETTINGS_LATESTHOST_CHANNELPASSWD).arg(index)).toString();
    return host.ipaddr.size();
}

void addServerEntry(const HostEntry& host)
{
    QList<HostEntry> hosts;
    HostEntry tmp;
    int index = 0;
    while(getServerEntry(index, tmp))
    {
        hosts.push_back(tmp);
        tmp = HostEntry();
        index++;
    }
    hosts.append(host);

    for(int i=0;i<hosts.size();i++)
        setServerEntry(i, hosts[i]);
}

void setServerEntry(int index, const HostEntry& host)
{
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_NAME).arg(index), host.name);
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_HOSTADDR).arg(index), host.ipaddr);
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_TCPPORT).arg(index), host.tcpport);  
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_UDPPORT).arg(index), host.udpport);  
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_ENCRYPTED).arg(index), host.encrypted);
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_USERNAME).arg(index), host.username); 
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_PASSWORD).arg(index), host.password); 
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_NICKNAME).arg(index), host.nickname); 
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_CHANNEL).arg(index), host.channel); 
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_CHANNELPASSWD).arg(index), host.chanpasswd); 
}

bool getServerEntry(int index, HostEntry& host)
{
    host.name = ttSettings->value(QString(SETTINGS_SERVERENTRIES_NAME).arg(index)).toString();
    host.ipaddr = ttSettings->value(QString(SETTINGS_SERVERENTRIES_HOSTADDR).arg(index)).toString();
    host.tcpport = ttSettings->value(QString(SETTINGS_SERVERENTRIES_TCPPORT).arg(index)).toInt();
    host.udpport = ttSettings->value(QString(SETTINGS_SERVERENTRIES_UDPPORT).arg(index)).toInt();
    host.encrypted = ttSettings->value(QString(SETTINGS_SERVERENTRIES_ENCRYPTED).arg(index), false).toBool();
    host.username = ttSettings->value(QString(SETTINGS_SERVERENTRIES_USERNAME).arg(index)).toString();
    host.password = ttSettings->value(QString(SETTINGS_SERVERENTRIES_PASSWORD).arg(index)).toString();
    host.nickname = ttSettings->value(QString(SETTINGS_SERVERENTRIES_NICKNAME).arg(index)).toString();
    host.channel = ttSettings->value(QString(SETTINGS_SERVERENTRIES_CHANNEL).arg(index)).toString();
    host.chanpasswd = ttSettings->value(QString(SETTINGS_SERVERENTRIES_CHANNELPASSWD).arg(index)).toString();
    return host.name.size();
}

void deleteServerEntry(const QString& name)
{
    QList<HostEntry> hosts;
    HostEntry tmp;
    int index = 0;
    while(getServerEntry(index, tmp))
    {
        if(tmp.name != name)
            hosts.push_back(tmp);
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_NAME).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_HOSTADDR).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_TCPPORT).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_UDPPORT).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_ENCRYPTED).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_USERNAME).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_PASSWORD).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_NICKNAME).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_CHANNEL).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_CHANNELPASSWD).arg(index));
        index++;
        tmp = HostEntry();
    }

    for(int i=0;i<hosts.size();i++)
        setServerEntry(i, hosts[i]);
}

void processAuthXML(const QDomElement& hostElement, HostEntry& entry)
{
    QDomElement auth = hostElement.firstChildElement("auth");
    if (!auth.isNull())
    {
        QDomElement tmp = auth.firstChildElement("username");
        if (!tmp.isNull())
            entry.username = tmp.text();

        tmp = auth.firstChildElement("password");
        if (!tmp.isNull())
            entry.password = tmp.text();

        tmp = auth.firstChildElement("nickname");
        if (!tmp.isNull())
            entry.nickname = tmp.text();
    }
}

void processJoinXML(const QDomElement& hostElement, HostEntry& entry)
{
    QDomElement join = hostElement.firstChildElement("join");
    if(!join.isNull())
    {
        QDomElement tmp = join.firstChildElement("channel");
        if (!tmp.isNull())
            entry.channel = tmp.text();
        tmp = join.firstChildElement("password");
        if (!tmp.isNull())
            entry.chanpasswd = tmp.text();
    }
}

void processClientSetupXML(const QDomElement& hostElement, HostEntry& entry)
{
    QDomElement client = hostElement.firstChildElement(CLIENTSETUP_TAG);
    if (!client.isNull())
    {
        QDomElement tmp = client.firstChildElement("nickname");
        if (!tmp.isNull())
            entry.nickname = tmp.text();

        tmp = client.firstChildElement("gender");
        if(!tmp.isNull())
        {
            switch (tmp.text().toInt())
            {
            case GENDER_MALE :
                entry.gender = GENDER_MALE;
                break;
            case GENDER_FEMALE :
                entry.gender = GENDER_FEMALE;
                break;
            case GENDER_NEUTRAL :
            default:
                entry.gender = GENDER_NEUTRAL;
                break;
            }
        }
#if defined(Q_OS_WIN32)
        tmp = client.firstChildElement("win-hotkey");
#elif defined(Q_OS_DARWIN)
        tmp = client.firstChildElement("mac-hotkey");
#elif defined(Q_OS_LINUX)
        tmp = client.firstChildElement("x11-hotkey");
#else
#error Unknown OS
#endif
        if(!tmp.isNull())
        {
            tmp = tmp.firstChildElement("key");
            while(!tmp.isNull())
            {
                entry.hotkey.push_back(tmp.text().toInt());
                tmp = tmp.nextSiblingElement("key");
            }
        }
        tmp = client.firstChildElement("voice-activated");
        if(!tmp.isNull())
            entry.voiceact = tmp.text().toInt();
        tmp = client.firstChildElement("videoformat");
        if(!tmp.isNull())
        {
            QDomElement cap = tmp.firstChildElement("width");
            if(!cap.isNull())
                entry.capformat.nWidth = cap.text().toInt();
            cap = tmp.firstChildElement("height");
            if(!cap.isNull())
                entry.capformat.nHeight = cap.text().toInt();

            cap = tmp.firstChildElement("fps-numerator");
            if(!cap.isNull())
                entry.capformat.nFPS_Numerator = cap.text().toInt();
            cap = tmp.firstChildElement("fps-denominator");
            if(!cap.isNull())
                entry.capformat.nFPS_Denominator = cap.text().toInt();

            cap = tmp.firstChildElement("fourcc");
            if(!cap.isNull())
                entry.capformat.picFourCC = (FourCC)cap.text().toInt();
        }
        tmp = client.firstChildElement("videocodec");
        if(!tmp.isNull())
        {
            QDomElement vid = tmp.firstChildElement("codec");
            if(!vid.isNull())
                entry.vidcodec.nCodec = (Codec)vid.text().toInt();
            switch(entry.vidcodec.nCodec)
            {
            case WEBM_VP8_CODEC :
                vid = tmp.firstChildElement("webm-vp8-bitrate");
                if(!vid.isNull())
                {
                    entry.vidcodec.webm_vp8.nRcTargetBitrate = vid.text().toInt();
                    entry.vidcodec.webm_vp8.nEncodeDeadline = DEFAULT_WEBMVP8_DEADLINE;
                }
                break;
            case SPEEX_CODEC :
            case SPEEX_VBR_CODEC :
            case OPUS_CODEC :
            case NO_CODEC :
                break;
            }
        }
    }
}

bool getServerEntry(const QDomElement& hostElement, HostEntry& entry)
{
    Q_ASSERT(hostElement.tagName() == "host");
    bool ok = true;
    QDomElement tmp = hostElement.firstChildElement("name");
    if(!tmp.isNull())
        entry.name = tmp.text();
    else ok = false;

    tmp = hostElement.firstChildElement("address");
    if(!tmp.isNull())
        entry.ipaddr = tmp.text();
    else ok = false;

    tmp = hostElement.firstChildElement("tcpport");
    if(!tmp.isNull())
        entry.tcpport = tmp.text().toInt();
    else ok = false;

    tmp = hostElement.firstChildElement("udpport");
    if(!tmp.isNull())
        entry.udpport = tmp.text().toInt();
    else ok = false;

    tmp = hostElement.firstChildElement("encrypted");
    if(!tmp.isNull())
        entry.encrypted = (tmp.text().toLower() == "true" || tmp.text() == "1");

    processAuthXML(hostElement, entry);
    processJoinXML(hostElement, entry);
    processClientSetupXML(hostElement, entry);

    return ok;
}

void addDesktopAccessEntry(const DesktopAccessEntry& entry)
{
    QVector<DesktopAccessEntry> entries;
    getDesktopAccessList(entries);
    entries.push_back(entry);

    for(int c=0;c<entries.size();c++)
    {
        ttSettings->setValue(QString(SETTINGS_DESKTOPACCESS_HOSTADDR).arg(c),
                             entries[c].ipaddr);
        ttSettings->setValue(QString(SETTINGS_DESKTOPACCESS_TCPPORT).arg(c),
                             entries[c].tcpport);
        for(int i=0;i<entries[c].channels.size();i++)
            ttSettings->setValue(QString(SETTINGS_DESKTOPACCESS_CHANNEL).arg(c).arg(i),
                                 entries[c].channels[i]);
        for(int i=0;i<entries[c].usernames.size();i++)
            ttSettings->setValue(QString(SETTINGS_DESKTOPACCESS_USERNAME).arg(c).arg(i),
                                 entries[c].usernames[i]);
    }
}

void getDesktopAccessList(QVector<DesktopAccessEntry>& entries)
{
    int c = 0;
    while(ttSettings->value(QString(SETTINGS_DESKTOPACCESS_HOSTADDR).arg(c)).toString().size())
    {
        DesktopAccessEntry entry;
        entry.ipaddr = ttSettings->value(QString(SETTINGS_DESKTOPACCESS_HOSTADDR).arg(c)).toString();
        entry.tcpport = ttSettings->value(QString(SETTINGS_DESKTOPACCESS_TCPPORT).arg(c)).toInt();

        int i = 0;
        while(ttSettings->value(QString(SETTINGS_DESKTOPACCESS_CHANNEL).arg(c).arg(i)).toString().size())
        {
            entry.channels.push_back(ttSettings->value(QString(SETTINGS_DESKTOPACCESS_CHANNEL).arg(c).arg(i)).toString());
            i++;
        }
        i = 0;
        while(ttSettings->value(QString(SETTINGS_DESKTOPACCESS_USERNAME).arg(c).arg(i)).toString().size())
        {
            entry.usernames.push_back(ttSettings->value(QString(SETTINGS_DESKTOPACCESS_USERNAME).arg(c).arg(i)).toString());
            i++;
        }
        entries.push_back(entry);
        c++;
    }
}

void getDesktopAccessList(QVector<DesktopAccessEntry>& entries,
                          const QString& ipaddr, int tcpport)
{
    QVector<DesktopAccessEntry> tmp;
    getDesktopAccessList(tmp);
    foreach(DesktopAccessEntry entry, tmp)
        if(entry.ipaddr == ipaddr && tcpport == entry.tcpport)
            entries.push_back(entry);
}

bool hasDesktopAccess(const QVector<DesktopAccessEntry>& entries,
                      const User& user)
{
    QString username = _Q(user.szUsername);
    for(int i=0;i<entries.size();i++)
    {
        if(username.size() && entries[i].usernames.size() &&
            entries[i].usernames.contains(username, Qt::CaseInsensitive))
            return true;

        //channel match must be done with TT-API since path might not 
        //match exactly, i.e. case-insensitive, pre '/' and post '/'
        if(user.nChannelID>0 && entries[i].channels.size())
        {
            for(int j=0;j<entries[i].channels.size();j++)
            {
                int chanid = TT_GetChannelIDFromPath(ttInst,
                                                     _W(entries[i].channels[j]));
                if(user.nChannelID > 0 && chanid == user.nChannelID)
                    return true;
            }
        }
    }
    return false;
}

void deleteDesktopAccessEntries()
{
    QVector<DesktopAccessEntry> entries;
    getDesktopAccessList(entries);

    for(int c=0;c<entries.size();c++)
    {
        ttSettings->remove(QString(SETTINGS_DESKTOPACCESS_HOSTADDR).arg(c));
        ttSettings->remove(QString(SETTINGS_DESKTOPACCESS_TCPPORT).arg(c));

        for(int i=0;i<entries[c].channels.size();i++)
            ttSettings->remove(QString(SETTINGS_DESKTOPACCESS_CHANNEL).arg(c).arg(i));
        for(int i=0;i<entries[c].usernames.size();i++)
            ttSettings->remove(QString(SETTINGS_DESKTOPACCESS_USERNAME).arg(c).arg(i));
    }
}

QString parseXML(const QDomDocument& doc, QString elements)
{
    QDomElement element(doc.documentElement());
    QStringList tokens = elements.split("/");
    while (tokens.size())
    {
        if (element.isNull() || element.nodeName() != tokens[0])
            return QString();

        tokens.removeFirst();
        if (tokens.isEmpty())
            return element.text();

        element = element.firstChildElement(tokens[0]);
    }
    return QString();
}

QString newVersionAvailable(const QDomDocument& updateDoc)
{
    return parseXML(updateDoc, "teamtalk/update/name");
}

QString downloadUpdateURL(const QDomDocument& updateDoc)
{
    return parseXML(updateDoc, "teamtalk/update/download-url");
}

QString newBetaVersionAvailable(const QDomDocument& updateDoc)
{
    return parseXML(updateDoc, "teamtalk/beta/name");
}

QString downloadBetaUpdateURL(const QDomDocument& updateDoc)
{
    return parseXML(updateDoc, "teamtalk/beta/download-url");
}

QString getBearWareRegistrationUrl(const QDomDocument& doc)
{
    return parseXML(doc, "teamtalk/bearware/register-url");
}

bool isWebLogin(const QString& username, bool includeParentLoginName)
{
    if (username.endsWith(WEBLOGIN_BEARWARE_USERNAMEPOSTFIX, Qt::CaseInsensitive))
        return true;

    return includeParentLoginName && username.compare(WEBLOGIN_BEARWARE_USERNAME, Qt::CaseInsensitive) == 0;
}

QString userCacheID(const User& user)
{
    bool restore = ttSettings->value(SETTINGS_GENERAL_RESTOREUSERSETTINGS, SETTINGS_GENERAL_RESTOREUSERSETTINGS_DEFAULT).toBool();

    if (restore && isWebLogin(_Q(user.szUsername), false))
        return QString("%1|%2").arg(_Q(user.szUsername)).arg(_Q(user.szClientName));

    return QString();
}

UserCached::UserCached(const User& user)
{
    valid = !userCacheID(user).isEmpty();
    if (!valid)
        return;

    subscriptions = user.uLocalSubscriptions;
    voiceMute = (user.uUserState & USERSTATE_MUTE_VOICE) != USERSTATE_NONE;
    mediaMute = (user.uUserState & USERSTATE_MUTE_MEDIAFILE) != USERSTATE_NONE;
    voiceVolume = user.nVolumeVoice;
    mediaVolume = user.nVolumeMediaFile;
    voiceLeftSpeaker = user.stereoPlaybackVoice[0];
    voiceRightSpeaker = user.stereoPlaybackVoice[1];
    mediaLeftSpeaker = user.stereoPlaybackMediaFile[0];
    mediaRightSpeaker = user.stereoPlaybackMediaFile[1];

    qDebug() << "Cached " << userCacheID(user);
}

void UserCached::sync(TTInstance* ttInst, const User& user)
{
    if (!valid)
        return;

    TT_SetUserMute(ttInst, user.nUserID, STREAMTYPE_VOICE, voiceMute);
    TT_SetUserMute(ttInst, user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO, mediaMute);
    TT_SetUserVolume(ttInst, user.nUserID, STREAMTYPE_VOICE, voiceVolume);
    TT_SetUserVolume(ttInst, user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO, mediaVolume);
    TT_SetUserStereo(ttInst, user.nUserID, STREAMTYPE_VOICE, voiceLeftSpeaker, voiceRightSpeaker);
    TT_SetUserStereo(ttInst, user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO, mediaLeftSpeaker, mediaRightSpeaker);
    if (subscriptions != user.uLocalSubscriptions)
    {
        TT_DoUnsubscribe(ttInst, user.nUserID, user.uLocalSubscriptions ^ subscriptions);
        TT_DoSubscribe(ttInst, user.nUserID, subscriptions);
    }
    TT_PumpMessage(ttInst, CLIENTEVENT_USER_STATECHANGE, user.nUserID);

    qDebug() << "Restored " << userCacheID(user);
}


QByteArray generateTTFile(const HostEntry& entry)
{
    QDomDocument doc(TTFILE_ROOT);
    QDomProcessingInstruction pi = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" ");
    doc.appendChild(pi);

    QDomElement root = doc.createElement(TTFILE_ROOT);
    root.setAttribute("version", TTFILE_VERSION);
    doc.appendChild(root);

    QDomElement host = doc.createElement("host");

    QDomElement name = doc.createElement("name");
    name.appendChild(doc.createTextNode(entry.name));

    QDomElement address = doc.createElement("address");
    address.appendChild(doc.createTextNode(entry.ipaddr));

    QDomElement tcpport = doc.createElement("tcpport");
    tcpport.appendChild(doc.createTextNode(QString::number(entry.tcpport)));

    QDomElement udpport = doc.createElement("udpport");
    udpport.appendChild(doc.createTextNode(QString::number(entry.udpport)));

    QDomElement encrypted = doc.createElement("encrypted");
    encrypted.appendChild(doc.createTextNode(entry.encrypted?"true":"false"));

    host.appendChild(name);
    host.appendChild(address);
    host.appendChild(tcpport);
    host.appendChild(udpport);
    host.appendChild(encrypted);

    if(entry.username.size())
    {
        QDomElement auth = doc.createElement("auth");

        QDomElement username = doc.createElement("username");
        username.appendChild(doc.createTextNode(entry.username));

        QDomElement password = doc.createElement("password");
        password.appendChild(doc.createTextNode(entry.password));

        QDomElement nickname = doc.createElement("nickname");
        nickname.appendChild(doc.createTextNode(entry.nickname));

        auth.appendChild(username);
        auth.appendChild(password);
        auth.appendChild(nickname);

        host.appendChild(auth);
    }

    if(entry.channel.size())
    {
        QDomElement join = doc.createElement("join");

        QDomElement channel = doc.createElement("channel");
        channel.appendChild(doc.createTextNode(entry.channel));

        QDomElement password = doc.createElement("password");
        password.appendChild(doc.createTextNode(entry.chanpasswd));

        join.appendChild(channel);
        join.appendChild(password);

        host.appendChild(join);
    }

    QDomElement client = doc.createElement(CLIENTSETUP_TAG);

    if(entry.nickname.size())
    {
        QDomElement nickname = doc.createElement("nickname");
        nickname.appendChild(doc.createTextNode(entry.nickname));
        client.appendChild(nickname);
    }
    if(!entry.gender)
    {
        QDomElement gender = doc.createElement("gender");
        gender.appendChild(doc.createTextNode(QString::number(entry.gender)));
        client.appendChild(gender);
    }
    if(entry.hotkey.size())
    {
#if defined(Q_OS_WIN32)
        QDomElement hotkey = doc.createElement("win-hotkey");
#elif defined(Q_OS_DARWIN)
        QDomElement hotkey = doc.createElement("mac-hotkey");
#elif defined(Q_OS_LINUX)
        QDomElement hotkey = doc.createElement("x11-hotkey");
#else
#error Unknown OS
#endif
        foreach(int k, entry.hotkey)
        {
            QDomElement key = doc.createElement("key");
            key.appendChild(doc.createTextNode(QString::number(k)));
            hotkey.appendChild(key);
        }
        client.appendChild(hotkey);
    }
    if(entry.voiceact >= 0)
    {
        QDomElement vox = doc.createElement("voice-activated");
        vox.appendChild(doc.createTextNode(QString::number(entry.voiceact)));
        client.appendChild(vox);
    }
    if(entry.capformat.nWidth)
    {
        QDomElement cap = doc.createElement("videoformat");

        QDomElement newElement = doc.createElement("width");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.nWidth)));
        cap.appendChild(newElement);
        newElement = doc.createElement("height");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.nHeight)));
        cap.appendChild(newElement);

        newElement = doc.createElement("fps-numerator");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.nFPS_Numerator)));
        cap.appendChild(newElement);
        newElement = doc.createElement("fps-denominator");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.nFPS_Denominator)));
        cap.appendChild(newElement);

        newElement = doc.createElement("fourcc");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.picFourCC)));
        cap.appendChild(newElement);

        client.appendChild(cap);
    }
    if(entry.vidcodec.nCodec != NO_CODEC)
    {
        QDomElement vidcodec = doc.createElement("videocodec");

        QDomElement newElement = doc.createElement("codec");
        newElement.appendChild(doc.createTextNode(QString::number(entry.vidcodec.nCodec)));
        vidcodec.appendChild(newElement);
        switch(entry.vidcodec.nCodec)
        {
        case WEBM_VP8_CODEC :
            newElement = doc.createElement("webm-vp8-bitrate");
            newElement.appendChild(doc.createTextNode(QString::number(entry.vidcodec.webm_vp8.nRcTargetBitrate)));
            vidcodec.appendChild(newElement);
            break;
        case SPEEX_CODEC :
        case SPEEX_VBR_CODEC :
        case OPUS_CODEC :
        case NO_CODEC :
            break;
        }
        client.appendChild(vidcodec);
    }

    if(client.hasChildNodes())
        host.appendChild(client);

    root.appendChild(host);

    return doc.toByteArray();
}

bool versionSameOrLater(const QString& check, const QString& against)
{
    if(check == against) return true;

    QStringList chk_tokens = check.split(".");
    QStringList against_tokens = against.split(".");

    QVector<int> vec_chk, vec_against;
    for(int i=0;i<chk_tokens.size();i++)
        vec_chk.push_back(chk_tokens[i].toInt());
    for(int i=0;i<against_tokens.size();i++)
        vec_against.push_back(against_tokens[i].toInt());

    int less = vec_chk.size() < vec_against.size()?vec_chk.size():vec_against.size();
    
    for(int i=0;i<less;i++)
        if(vec_chk[i] < vec_against[i])
            return false;
        else if(vec_chk[i] > vec_against[i])
            return true;

    return true;
}

QString getVersion(const User& user)
{
    return QString("%1.%2.%3")
        .arg(user.uVersion >> 16)
        .arg((user.uVersion >> 8) & 0xFF)
        .arg(user.uVersion & 0xFF);
}

QString limitText(const QString& text)
{
    int len = ttSettings->value(SETTINGS_DISPLAY_MAX_STRING, DEFAULT_MAX_STRING_LENGTH).toInt();
    if(text.size()>len+3)
        return text.left(len) + "...";
    return text;
}

QString getDisplayName(const User& user)
{
    if(ttSettings->value(SETTINGS_DISPLAY_SHOWUSERNAME,
                         SETTINGS_DISPLAY_SHOWUSERNAME_DEFAULT).toBool())
    {
        return limitText(_Q(user.szUsername));
    }

    QString nickname = _Q(user.szNickname);
    if (nickname.isEmpty())
        nickname = QString("%1 - #%2").arg(QCoreApplication::translate("MainWindow", DEFAULT_NICKNAME)).arg(user.nUserID);
    return limitText(nickname);
}

QString generateAudioStorageFilename(AudioFileFormat aff)
{
    QString filename = getDateTimeStamp() + " ";
    filename += QObject::tr("Conference");
    switch(aff)
    {
    case AFF_WAVE_FORMAT :
        filename += ".wav";
        break;
    case AFF_CHANNELCODEC_FORMAT :
        filename += ".ogg";
        break;
    case AFF_MP3_16KBIT_FORMAT :
    case AFF_MP3_32KBIT_FORMAT :
    case AFF_MP3_64KBIT_FORMAT :
    case AFF_MP3_128KBIT_FORMAT :
    case AFF_MP3_256KBIT_FORMAT :
        filename += ".mp3";
        break;
    case AFF_NONE :
        break;
    }
    return filename;
}

QString getDateTimeStamp()
{
    return QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
}

QString generateLogFileName(const QString& name)
{
    static QString invalidPath("?:*\"|<>/\\");

    QString filename = getDateTimeStamp();
    filename += " - " + name;
    for(int i=0;i<filename.size();i++)
    {
        if(invalidPath.contains(filename[i]))
            filename[i] = '_';
    }
    return filename;
}

bool openLogFile(QFile& file, const QString& folder, const QString& name)
{
    if(file.isOpen())
        file.close();

    QString filename = folder + "/";
    filename += generateLogFileName(name);

    file.setFileName(filename);
    return file.open(QFile::WriteOnly | QFile::Append);
}

bool writeLogEntry(QFile& file, const QString& line)
{
    return file.write(QString(line + "\r\n").toUtf8())>0;
}
