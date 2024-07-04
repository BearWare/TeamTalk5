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

#include "common.h"
#include "settings.h"
#include "appinfo.h"

#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QStack>
#include <QTemporaryFile>

#define DEFAULT_NICKNAME           QT_TRANSLATE_NOOP("MainWindow", "NoName")

extern QSettings* ttSettings;
extern TTInstance* ttInst;

QString makeCustomCommand(const QString& cmd, const QString& value)
{
    return QString("%1\r\n%2").arg(cmd).arg(value);
}

QStringList getCustomCommand(const MyTextMessage& msg)
{
    return msg.moreMessage.split("\r\n");
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

QString HostEntry::generateEntryName() const
{
    QString genname;
    if (username.size())
        genname = QString("%1@%2:%3").arg(username).arg(ipaddr).arg(tcpport);
    else if (ipaddr.size())
        genname = QString("%1:%2").arg(ipaddr).arg(tcpport);
    return genname;
}

bool setupEncryption(const HostEntry& host)
{
    if (!host.encrypted)
        return true;

    QTemporaryFile cafile, certfile, keyfile;
    if (!cafile.open() || !cafile.setPermissions(QFileDevice::ReadOwner))
        return false;
    if (!certfile.open() || !certfile.setPermissions(QFileDevice::ReadOwner))
        return false;
    if (!keyfile.open() || !keyfile.setPermissions(QFileDevice::ReadOwner))
        return false;

    QString cafilename, certfilename, keyfilename;
    if (host.encryption.cacertdata.size())
    {
        cafile.write(host.encryption.cacertdata.toUtf8());
        cafilename = QDir::toNativeSeparators(cafile.fileName());
    }
    if (host.encryption.certdata.size())
    {
        certfile.write(host.encryption.certdata.toUtf8());
        certfilename = QDir::toNativeSeparators(certfile.fileName());
    }
    if (host.encryption.privkeydata.size())
    {
        keyfile.write(host.encryption.privkeydata.toUtf8());
        keyfilename = QDir::toNativeSeparators(keyfile.fileName());
    }

    cafile.close();
    certfile.close();
    keyfile.close();

    EncryptionContext context = {};
    context.bVerifyPeer = host.encryption.verifypeer;
    if (!context.bVerifyPeer)
        context.nVerifyDepth = -1;
    COPY_TTSTR(context.szCAFile, cafilename);
    COPY_TTSTR(context.szCertificateFile, certfilename);
    COPY_TTSTR(context.szPrivateKeyFile, keyfilename);

    return TT_SetEncryptionContext(ttInst, &context);
}

void addLatestHost(const HostEntry& host)
{
    QList<HostEntry> hosts;
    HostEntry tmp;
    int index = 0;
    while(getServerEntry(index, tmp, true))
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
        tmp = hosts[i];
        tmp.latesthost = true;
        setServerEntry(i, tmp);
    }
}

void addServerEntry(const HostEntry& host)
{
    QList<HostEntry> hosts;
    HostEntry tmp;
    int index = 0;
    while(getServerEntry(index, tmp, host.latesthost))
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
    ttSettings->setValue(QString((host.latesthost ? SETTINGS_LATESTHOST_NAME : SETTINGS_SERVERENTRIES_NAME)).arg(index), (host.latesthost ? host.generateEntryName() : host.name));
    ttSettings->setValue(QString((host.latesthost ? SETTINGS_LATESTHOST_HOSTADDR : SETTINGS_SERVERENTRIES_HOSTADDR)).arg(index), host.ipaddr);
    ttSettings->setValue(QString((host.latesthost ? SETTINGS_LATESTHOST_TCPPORT : SETTINGS_SERVERENTRIES_TCPPORT)).arg(index), host.tcpport);
    ttSettings->setValue(QString((host.latesthost ? SETTINGS_LATESTHOST_UDPPORT : SETTINGS_SERVERENTRIES_UDPPORT)).arg(index), host.udpport);
    ttSettings->setValue(QString((host.latesthost ? SETTINGS_LATESTHOST_ENCRYPTED : SETTINGS_SERVERENTRIES_ENCRYPTED)).arg(index), host.encrypted);
    ttSettings->setValue(QString((host.latesthost ? SETTINGS_LATESTHOST_ENCRYPTED_CADATA : SETTINGS_SERVERENTRIES_ENCRYPTED_CADATA)).arg(index), host.encryption.cacertdata);
    ttSettings->setValue(QString((host.latesthost ? SETTINGS_LATESTHOST_ENCRYPTED_CERTDATA : SETTINGS_SERVERENTRIES_ENCRYPTED_CERTDATA)).arg(index), host.encryption.certdata);
    ttSettings->setValue(QString((host.latesthost ? SETTINGS_LATESTHOST_ENCRYPTED_KEYDATA : SETTINGS_SERVERENTRIES_ENCRYPTED_KEYDATA)).arg(index), host.encryption.privkeydata);
    ttSettings->setValue(QString((host.latesthost ? SETTINGS_LATESTHOST_ENCRYPTED_VERIFYPEER : SETTINGS_SERVERENTRIES_ENCRYPTED_VERIFYPEER)).arg(index), host.encryption.verifypeer);
    ttSettings->setValue(QString((host.latesthost ? SETTINGS_LATESTHOST_USERNAME : SETTINGS_SERVERENTRIES_USERNAME)).arg(index), host.username);
    ttSettings->setValue(QString((host.latesthost ? SETTINGS_LATESTHOST_PASSWORD : SETTINGS_SERVERENTRIES_PASSWORD)).arg(index), host.password);
    ttSettings->setValue(QString((host.latesthost ? SETTINGS_LATESTHOST_NICKNAME : SETTINGS_SERVERENTRIES_NICKNAME)).arg(index), host.nickname);
    if (!host.latesthost)
        ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_LASTCHANNEL).arg(index), host.lastChan);
    ttSettings->setValue(QString((host.latesthost ? SETTINGS_LATESTHOST_CHANNEL : SETTINGS_SERVERENTRIES_CHANNEL)).arg(index), host.channel);
    ttSettings->setValue(QString((host.latesthost ? SETTINGS_LATESTHOST_CHANNELPASSWD : SETTINGS_SERVERENTRIES_CHANNELPASSWD)).arg(index), host.chanpasswd);
}

bool getServerEntry(int index, HostEntry& host, bool latesthost)
{
    host.name = ttSettings->value(QString((latesthost ? SETTINGS_LATESTHOST_NAME:SETTINGS_SERVERENTRIES_NAME)).arg(index)).toString();
    host.ipaddr = ttSettings->value(QString((latesthost ? SETTINGS_LATESTHOST_HOSTADDR : SETTINGS_SERVERENTRIES_HOSTADDR)).arg(index)).toString();
    host.tcpport = ttSettings->value(QString((latesthost ? SETTINGS_LATESTHOST_TCPPORT : SETTINGS_SERVERENTRIES_TCPPORT)).arg(index)).toInt();
    host.udpport = ttSettings->value(QString((latesthost ? SETTINGS_LATESTHOST_UDPPORT : SETTINGS_SERVERENTRIES_UDPPORT)).arg(index)).toInt();
    host.encrypted = ttSettings->value(QString((latesthost ? SETTINGS_LATESTHOST_ENCRYPTED : SETTINGS_SERVERENTRIES_ENCRYPTED)).arg(index, false)).toBool();
    host.encryption.cacertdata = ttSettings->value(QString((latesthost ? SETTINGS_LATESTHOST_ENCRYPTED_CADATA : SETTINGS_SERVERENTRIES_ENCRYPTED_CADATA)).arg(index)).toString();
    host.encryption.certdata = ttSettings->value(QString((latesthost ? SETTINGS_LATESTHOST_ENCRYPTED_CERTDATA : SETTINGS_SERVERENTRIES_ENCRYPTED_CERTDATA)).arg(index)).toString();
    host.encryption.privkeydata = ttSettings->value(QString((latesthost ? SETTINGS_LATESTHOST_ENCRYPTED_KEYDATA : SETTINGS_SERVERENTRIES_ENCRYPTED_KEYDATA)).arg(index)).toString();
    host.encryption.verifypeer = ttSettings->value(QString((latesthost ? SETTINGS_LATESTHOST_ENCRYPTED_VERIFYPEER : SETTINGS_SERVERENTRIES_ENCRYPTED_VERIFYPEER)).arg(index), HostEntry().encryption.verifypeer).toBool();
    host.username = ttSettings->value(QString((latesthost ? SETTINGS_LATESTHOST_USERNAME : SETTINGS_SERVERENTRIES_USERNAME)).arg(index)).toString();
    host.password = ttSettings->value(QString((latesthost ? SETTINGS_LATESTHOST_PASSWORD : SETTINGS_SERVERENTRIES_PASSWORD)).arg(index)).toString();
    host.nickname = ttSettings->value(QString((latesthost ? SETTINGS_LATESTHOST_NICKNAME : SETTINGS_SERVERENTRIES_NICKNAME)).arg(index)).toString();
    host.channel = ttSettings->value(QString((latesthost ? SETTINGS_LATESTHOST_CHANNEL : SETTINGS_SERVERENTRIES_CHANNEL)).arg(index)).toString();
    host.chanpasswd = ttSettings->value(QString((latesthost ? SETTINGS_LATESTHOST_CHANNELPASSWD : SETTINGS_SERVERENTRIES_CHANNELPASSWD)).arg(index)).toString();
    if (!host.latesthost)
        host.lastChan = ttSettings->value(QString(SETTINGS_SERVERENTRIES_LASTCHANNEL).arg(index)).toBool();
    host.latesthost = latesthost;
    return host.ipaddr.size();
}

void deleteServerEntry(const HostEntry& host)
{
    QList<HostEntry> hosts;
    HostEntry tmp;
    int index = 0;
    while (getServerEntry(index, tmp, host.latesthost))
    {
        if(tmp.name != host.name)
            hosts.push_back(tmp);
        ttSettings->remove(QString((host.latesthost ? SETTINGS_LATESTHOST_NAME : SETTINGS_SERVERENTRIES_NAME)).arg(index));
        ttSettings->remove(QString((host.latesthost ? SETTINGS_LATESTHOST_HOSTADDR : SETTINGS_SERVERENTRIES_HOSTADDR)).arg(index));
        ttSettings->remove(QString((host.latesthost ? SETTINGS_LATESTHOST_TCPPORT : SETTINGS_SERVERENTRIES_TCPPORT)).arg(index));
        ttSettings->remove(QString((host.latesthost ? SETTINGS_LATESTHOST_UDPPORT : SETTINGS_SERVERENTRIES_UDPPORT)).arg(index));
        ttSettings->remove(QString((host.latesthost ? SETTINGS_LATESTHOST_ENCRYPTED : SETTINGS_SERVERENTRIES_ENCRYPTED)).arg(index));
        ttSettings->remove(QString((host.latesthost ? SETTINGS_LATESTHOST_ENCRYPTED_CADATA : SETTINGS_SERVERENTRIES_ENCRYPTED_CADATA)).arg(index));
        ttSettings->remove(QString((host.latesthost ? SETTINGS_LATESTHOST_ENCRYPTED_CERTDATA : SETTINGS_SERVERENTRIES_ENCRYPTED_CERTDATA)).arg(index));
        ttSettings->remove(QString((host.latesthost ? SETTINGS_LATESTHOST_ENCRYPTED_KEYDATA : SETTINGS_SERVERENTRIES_ENCRYPTED_KEYDATA)).arg(index));
        ttSettings->remove(QString((host.latesthost ? SETTINGS_LATESTHOST_ENCRYPTED_VERIFYPEER : SETTINGS_SERVERENTRIES_ENCRYPTED_VERIFYPEER)).arg(index));
        ttSettings->remove(QString((host.latesthost ? SETTINGS_LATESTHOST_USERNAME : SETTINGS_SERVERENTRIES_USERNAME)).arg(index));
        ttSettings->remove(QString((host.latesthost ? SETTINGS_LATESTHOST_PASSWORD : SETTINGS_SERVERENTRIES_PASSWORD)).arg(index));
        ttSettings->remove(QString((host.latesthost ? SETTINGS_LATESTHOST_NICKNAME : SETTINGS_SERVERENTRIES_NICKNAME)).arg(index));
        if (!host.latesthost)
            ttSettings->remove(QString(SETTINGS_SERVERENTRIES_LASTCHANNEL).arg(index));
        ttSettings->remove(QString((host.latesthost ? SETTINGS_LATESTHOST_CHANNEL : SETTINGS_SERVERENTRIES_CHANNEL)).arg(index));
        ttSettings->remove(QString((host.latesthost ? SETTINGS_LATESTHOST_CHANNELPASSWD : SETTINGS_SERVERENTRIES_CHANNELPASSWD)).arg(index));
        index++;
        tmp = HostEntry();
    }

    for(int i=0;i<hosts.size();i++)
        setServerEntry(i, hosts[i]);
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

void saveAudioPreprocessor(const AudioPreprocessor& preprocessor)
{
    ttSettings->setValue(SETTINGS_STREAMMEDIA_AUDIOPREPROCESSOR, preprocessor.nPreprocessor);
    switch (preprocessor.nPreprocessor)
    {
    case NO_AUDIOPREPROCESSOR:
        break;
    case SPEEXDSP_AUDIOPREPROCESSOR:
        ttSettings->setValue(SETTINGS_STREAMMEDIA_SPX_AGC_ENABLE, preprocessor.speexdsp.bEnableAGC);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_SPX_AGC_GAINLEVEL, preprocessor.speexdsp.nGainLevel);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_SPX_AGC_INC_MAXDB, preprocessor.speexdsp.nMaxIncDBSec);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_SPX_AGC_DEC_MAXDB, preprocessor.speexdsp.nMaxDecDBSec);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_SPX_AGC_GAINMAXDB, preprocessor.speexdsp.nMaxGainDB);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_SPX_DENOISE_ENABLE, preprocessor.speexdsp.bEnableDenoise);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_SPX_DENOISE_SUPPRESS, preprocessor.speexdsp.nMaxNoiseSuppressDB);
        break;
    case TEAMTALK_AUDIOPREPROCESSOR:
        ttSettings->setValue(SETTINGS_STREAMMEDIA_TTAP_MUTELEFT, preprocessor.ttpreprocessor.bMuteLeftSpeaker);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_TTAP_MUTERIGHT, preprocessor.ttpreprocessor.bMuteRightSpeaker);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_TTAP_GAINLEVEL, preprocessor.ttpreprocessor.nGainLevel);
        break;
    case WEBRTC_AUDIOPREPROCESSOR :
        ttSettings->setValue(SETTINGS_STREAMMEDIA_WEBRTC_GAINCTL_ENABLE, preprocessor.webrtc.gaincontroller2.bEnable);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_WEBRTC_GAINDB, preprocessor.webrtc.gaincontroller2.fixeddigital.fGainDB);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_WEBRTC_SAT_PROT_ENABLE, preprocessor.webrtc.gaincontroller2.adaptivedigital.bEnable );
        ttSettings->setValue(SETTINGS_STREAMMEDIA_WEBRTC_INIT_SAT_MARGIN_DB, preprocessor.webrtc.gaincontroller2.adaptivedigital.fInitialSaturationMarginDB);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_WEBRTC_EXTRA_SAT_MARGIN_DB, preprocessor.webrtc.gaincontroller2.adaptivedigital.fExtraSaturationMarginDB);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_WEBRTC_MAXGAIN_DBSEC, preprocessor.webrtc.gaincontroller2.adaptivedigital.fMaxGainChangeDBPerSecond);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_WEBRTC_MAX_OUT_NOISE, preprocessor.webrtc.gaincontroller2.adaptivedigital.fMaxOutputNoiseLevelDBFS);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_WEBRTC_NOISESUPPRESS_ENABLE, preprocessor.webrtc.noisesuppression.bEnable);
        ttSettings->setValue(SETTINGS_STREAMMEDIA_WEBRTC_NOISESUPPRESS_LEVEL, preprocessor.webrtc.noisesuppression.nLevel);
        break;
    }
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
    int len = ttSettings->value(SETTINGS_DISPLAY_MAX_STRING, SETTINGS_DISPLAY_MAX_STRING_DEFAULT).toInt();
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
    case AFF_MP3_256KBIT_FORMAT:
    case AFF_MP3_320KBIT_FORMAT :
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
    bool ret =  file.write(QString(line + "\r\n").toUtf8()) > 0;
    file.flush();
    return ret;
}
